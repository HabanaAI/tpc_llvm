//===- TPCHwWaGeneral.cpp--------General pass for transformation-----------===//
//
//
//                     The LLVM Compiler Infrastructure:
//
//              2020 - This pass is a property of Habana labs
//
// author: Michael Zuckerman
//         mzuckerman@habana.ai
//===----------------------------------------------------------------------===//
// This pass create a work around for hardware issue.
// In this pass you can find the following transformations:
// A) CALC_FP_SPECIAL.FP16 DEST, SRC1,SRC2, functionid, val
//    For functionid == {POW|DIV}:
//         1) CONVERT.BF16 TO_FP16 DEST1, PRED = SP0, 0
//         2) SEL DEST2,DEST,DEST,DEST1 PRED=SP0,0
//    else:
//         1) CMP.FP16 MASK_ZERO VPRF1, SRC1, 0xbd
//         2) CONVERT.BF16 TO_FP16 DEST, PRED = VPRF1,-1
// B) If kernel doesn't include a lookup instruction the compiler most adds a
//    CHACH_INVALIDATED
//===----------------------------------------------------------------------===//
#include "MCTargetDesc/TPCMCInstrInfo.h"
#include "MCTargetDesc/TPCMCTargetDesc.h"
#include "TPCInstrInfo.h"
#include "TPCSubtarget.h"
#include "TPCTargetMachine.h"
#include "llvm/ADT/SmallSet.h"
#include "llvm/CodeGen/MachineInstrBuilder.h"
#include "llvm/CodeGen/MachineRegisterInfo.h"

#include <unordered_set>

#define DEBUG_TYPE "hwwaGeneral"

using namespace llvm;

namespace llvm {
FunctionPass *createTPCHWWAGeneral();
void initializeTPCHWWAGeneralPass(PassRegistry &);
} // namespace llvm

//
// Option to enable/disable ASH restriction for GRECO architecture.
//
static cl::opt<bool> EnableHwwaAshZeroScale("tpc-hwwa-ash-zero-scale", cl::ZeroOrMore, cl::init(false));
static cl::opt<bool> EnableHwwaAshAndScale("tpc-hwwa-ash-and-scale", cl::ZeroOrMore, cl::init(true));

static const char PassDescription[] =
    "TPC Hardware Workarounds pass (general pass)";
static const char PassName[] = "tpc-hwwa-general-workaround";

namespace {
class TPCHWWAGeneral : public MachineFunctionPass {
private:
  MachineInstr *produceCONVERTVVm(MachineFunction &Func, unsigned convertReg,
                                  MachineInstr *MI,
                                  unsigned reg_a,int polarity);
  MachineInstr *produceSEL_EQ(MachineFunction &Func, MachineInstr *MI,
                              unsigned SRC_A, unsigned SRC_B,
                              unsigned SRC_C, unsigned SRC_D, unsigned income);
  MachineInstr *produceCMP_EQvip(MachineFunction &Func, MachineInstr *MI,
                                 unsigned reg_a, int imm,
                                 int maskZero);

  void updateRegister(MachineInstr *OldMI, MachineInstr *MI);

  bool lookupWorkAround(MachineFunction &Func);
  
  bool goya2HwWA(MachineFunction &Func);

  bool gen4PlusHwWA(MachineFunction &Func);
  
  bool ForceLFSRDefs(MachineFunction &Func);
  
  bool RemoveSQZCopy(MachineFunction &Func);
  
  bool ProcessLookupBV32(MachineFunction &Func);

public:
  static char ID;

  StringRef getPassName() const override { return PassDescription; }

  TPCHWWAGeneral() : MachineFunctionPass(ID) {
    initializeTPCHWWAGeneralPass(*PassRegistry::getPassRegistry());
  }

  bool runOnMachineFunction(MachineFunction &MF) override;
};
} // namespace

char TPCHWWAGeneral::ID = 0;

INITIALIZE_PASS(TPCHWWAGeneral, PassName, PassDescription, false, false)

FunctionPass *llvm::createTPCHWWAGeneral() { return new TPCHWWAGeneral(); }

void TPCHWWAGeneral::updateRegister(MachineInstr *OldMI, MachineInstr *MI) {
  for (MachineBasicBlock::iterator miFrom = MI->getNextNode();
       miFrom != MI->getParent()->end(); miFrom++) {
    for (MachineOperand &MO : miFrom->uses()) {
      if (MO.isReg()) {
        if (MO.getReg() == OldMI->getOperand(0).getReg()) {
          MO.setReg(MI->getOperand(0).getReg());
        }
      }
    }
  }
}
/*!
 *
 * @param Func        machine function
 * @param MI        Current machineInstr
 * @param reg_a     register to compare
 * @param imm       immediate  to compare
 * @param maskZero  maskZero switch
 * @return New MachineInstr after MI
 */
MachineInstr *TPCHWWAGeneral::produceCMP_EQvip(MachineFunction &Func, 
                                               MachineInstr *MI, unsigned reg_a,
                                               int imm, int maskZero = 0) {
  MachineFunction *MF = &Func;
  MachineRegisterInfo *MRI = &MF->getRegInfo();
  const TargetInstrInfo *TII = MF->getSubtarget().getInstrInfo();

  unsigned v_reg1 = MRI->createVirtualRegister(&TPC::VPRFRegClass);
  unsigned v_reg2 = MRI->createVirtualRegister(&TPC::VRFRegClass);
  MachineBasicBlock *MBB = MI->getParent();
  MachineInstr *CMP_EQvip =
      MF->CreateMachineInstr(TII->get(TPC::CMP_EQvip), MI->getDebugLoc(), true);
  BuildMI(*MBB, MI, MI->getDebugLoc(), TII->get(TargetOpcode::IMPLICIT_DEF),
          v_reg2);
  CMP_EQvip->addOperand(*MF, MachineOperand::CreateReg(v_reg1, true)); // dest
  CMP_EQvip->addOperand(*MF,
                        MachineOperand::CreateReg(reg_a, false)); // source1
  CMP_EQvip->addOperand(*MF,
                        MachineOperand::CreateImm(imm)); // source2 hard compare
  CMP_EQvip->addOperand(
      *MF, MachineOperand::CreateImm(TPCII::OpType::FP16)); // CompareType
  CMP_EQvip->addOperand(*MF, MachineOperand::CreateImm(maskZero)); // switch
  CMP_EQvip->addOperand(*MF,
                        MachineOperand::CreateReg(v_reg2, false)); // income
  CMP_EQvip->addOperand(
      *MF, MachineOperand::CreateReg(TPC::SPRF_TRUE, false)); // predicate
  CMP_EQvip->addOperand(*MF, MachineOperand::CreateImm(0));
  MBB->insertAfter(MI, CMP_EQvip);
  return CMP_EQvip;
}

/*!
 *
 * @param Func        machine function
 * @param convertReg  source1
 * @param MI          Current machineInstr
 * @param predicate   predicate register
 * @return New MachineInstr after MI
 */
MachineInstr * TPCHWWAGeneral::produceCONVERTVVm(MachineFunction &Func,
                                  unsigned convertReg,
                                  MachineInstr *MI,
                                  unsigned predicate,int polarity) {
  MachineFunction *MF = &Func;
  MachineRegisterInfo *MRI = &MF->getRegInfo();
  const TargetInstrInfo *TII = MF->getSubtarget().getInstrInfo();

  unsigned v_reg1 = MRI->createVirtualRegister(&TPC::VRFRegClass);
  MachineBasicBlock *MBB = MI->getParent();
  const MCInstrDesc &MID = predicate == TPC::SPRF_TRUE ? TII->get(TPC::CONVERTvvp)
                                                : TII->get(TPC::CONVERTvvm);
  MachineInstr *CONVERTvvp =
      MF->CreateMachineInstr(MID, MI->getDebugLoc(), true);
  CONVERTvvp->addOperand(*MF, MachineOperand::CreateReg(v_reg1, true)); // dest
  CONVERTvvp->addOperand(
      *MF, MachineOperand::CreateReg(convertReg, false)); // source
  CONVERTvvp->addOperand(
      *MF, MachineOperand::CreateImm(TPCII::OpType::BF16)); // From_type
  CONVERTvvp->addOperand(
      *MF,
      MachineOperand::CreateImm(TPCII::SW_TO_FP16 | TPCII::SW_LANE_0 | TPCII::SW_SINGLE_LANE_SRCB)); // switch
  CONVERTvvp->addOperand(
      *MF, MachineOperand::CreateReg(convertReg, false)); // income
  CONVERTvvp->addOperand(
      *MF, MachineOperand::CreateReg(predicate, false)); // Predicate
  CONVERTvvp->addOperand(*MF, MachineOperand::CreateImm(polarity));
  MBB->insertAfter(MI, CONVERTvvp);
  return CONVERTvvp;
}

/*!
 * \param Func machine function
 * \param MI Current machineInstr 
 * \param SRC_A
 * \param SRC_B
 * \param SRC_C
 * \param SRC_D
 * \param income 
 * \return new SEL_EQvvvp instruction 
 */
MachineInstr *TPCHWWAGeneral::produceSEL_EQ(MachineFunction &Func,
                                            MachineInstr *MI, unsigned SRC_A,
                                            unsigned SRC_B, unsigned SRC_C,
                                            unsigned SRC_D, unsigned income) {
  MachineFunction *MF = &Func;
  MachineRegisterInfo *MRI = &MF->getRegInfo();
  const TargetInstrInfo *TII = MF->getSubtarget().getInstrInfo();
  
  unsigned v_reg1 = MRI->createVirtualRegister(&TPC::VRFRegClass);
  MachineBasicBlock *MBB = MI->getParent();
  MachineInstr *SEL_EQ = MF->CreateMachineInstr(TII->get(TPC::SEL_EQvvvvp),
                                                MI->getDebugLoc(), true);
  SEL_EQ->addOperand(*MF, MachineOperand::CreateReg(v_reg1, true)); // Result
  SEL_EQ->addOperand(*MF, MachineOperand::CreateReg(SRC_A, false)); // SRC_A
  SEL_EQ->addOperand(*MF, MachineOperand::CreateReg(SRC_B, false)); // SRC_B
  SEL_EQ->addOperand(*MF, MachineOperand::CreateReg(SRC_C, false)); // SRC_C
  SEL_EQ->addOperand(*MF, MachineOperand::CreateReg(SRC_D, false)); // SRC_D
  SEL_EQ->addOperand(*MF, MachineOperand::CreateImm(TPCII::OpType::FP16)); //data type
  SEL_EQ->addOperand(*MF, MachineOperand::CreateImm(0));
  SEL_EQ->addOperand(*MF, MachineOperand::CreateReg(income, false)); // Income
  SEL_EQ->addOperand(*MF, MachineOperand::CreateReg(TPC::SPRF_TRUE, false)); // predicate 
  SEL_EQ->addOperand(*MF, MachineOperand::CreateImm(0)); // polarity 
  MBB->insertAfter(MI, SEL_EQ);
  return SEL_EQ;
}
/*!
 * In case lookup is not part of the kernel add CACHE_INVALIDATE instruction
 * before the halt instruction. In case there is a change return true.
 */
bool TPCHWWAGeneral::lookupWorkAround(MachineFunction &Func) {
  MachineFunction *MF = &Func;
  const TargetInstrInfo *TII = MF->getSubtarget().getInstrInfo();
  bool LookupPresent = false;
  for (MachineBasicBlock &MBB : *MF) {
    for (MachineBasicBlock::iterator mi = MBB.begin(), me = MBB.end(); mi != me; mi++) {
      LookupPresent |= (TPCII::isLookup((*mi).getDesc()) || TPCII::isLookupC((*mi).getDesc()));
    }
  }

  if (!LookupPresent) {
    const auto &Features = Func.getSubtarget().getFeatureBits();
    auto firstInst = MF->getBlockNumbered(0)->instr_begin();
    MachineInstr *cacheIns = MF->CreateMachineInstr(TII->get(TPC::CACHE_INVALIDATE), firstInst->getDebugLoc(), true);
    if(Features[TPC::FeatureGen4Plus])
      cacheIns->addOperand(*MF, MachineOperand::CreateImm(TPCII::SW_LU));
    else
      cacheIns->addOperand(*MF, MachineOperand::CreateImm(0));
    cacheIns->addOperand(*MF, MachineOperand::CreateReg(TPC::SPRF_TRUE, false));
    cacheIns->addOperand(*MF, MachineOperand::CreateImm(0));
    MachineBasicBlock *MBB = firstInst->getParent();
    MBB->insert(firstInst, cacheIns);
    return true;
  }
  return false;
}

/*!

 * This is the main function to run over the MachineFucntion. In this function
 * we iterate over all blocks and instructions inside a machine function.
 * The function finds the desirable instruction to replace with a new sequence.
 * @param Func
 * @return  return boolean indicate of replacement
 */
bool TPCHWWAGeneral::runOnMachineFunction(MachineFunction &Func) {
  bool Status = false;
  const auto &Features = Func.getSubtarget().getFeatureBits();
  if (!Features[TPC::FeatureDoron1]) {
    Status |= lookupWorkAround(Func);
  }
  if (Features[TPC::FeatureGreco])
    Status |= goya2HwWA(Func);
  if (Features[TPC::FeatureGen4Plus]) {
    Status |= gen4PlusHwWA(Func);
    Status |= RemoveSQZCopy(Func);
  }
  if (Features[TPC::FeatureDoron1]) {
    Status |= ForceLFSRDefs(Func);
    Status |= ProcessLookupBV32(Func);
  }
  return Status;
}

bool TPCHWWAGeneral::gen4PlusHwWA(MachineFunction &Func) {
  MachineFunction *MF = &Func;
  const TargetInstrInfo *TII = MF->getSubtarget().getInstrInfo();
  bool RestrictionReleased = false;
  for (MachineBasicBlock &MBB : *MF) {
    for (MachineBasicBlock::iterator mi = MBB.begin(), me = MBB.end(); mi != me; mi++) {
      MCInstrDesc Desc = (*mi).getDesc();
      auto OpCode = TPCII::getSlotOpCode(Desc);
      MachineInstr *MI = &*mi;
      if (TPCII::isLoadInst(Desc)) {
        if ((OpCode == TPCII::LOOKUP_1C) || (OpCode == TPCII::LOOKUP_2C)) {
          if (MI->getOperand(3).getImm() & TPCII::SW_X2_ARITHMETIC) {
            // GAUDI2_0012: Before using LOOKUP.X2, must do CACHE_INVALIDATE.LU
            // Problem:     Tags might be wrong during transition from normal LOOKUP to LOOKUP.X2
            // Implication: Can't do LOOKUP.X2 after LOOKUP
            // Workaround:  Perform CACHE_INVALIDATE.LU before using LOOKUP.X2
            RestrictionReleased = true;
            MachineInstr *cacheIns = MF->CreateMachineInstr(TII->get(TPC::CACHE_INVALIDATE), MI->getDebugLoc(), true);
            cacheIns->addOperand(*MF, MachineOperand::CreateImm(TPCII::SW_LU));
            cacheIns->addOperand(*MF, MachineOperand::CreateReg(TPC::SPRF_TRUE, false));
            cacheIns->addOperand(*MF, MachineOperand::CreateImm(0));
            MBB.insert(MI, cacheIns);
          }
        }
      } else if (TPCII::isStoreInst(Desc)) {
        if (OpCode == TPCII::CACHE_INVALIDATE && MI->getOperand(0).isImm() && (MI->getOperand(0).getImm() & TPCII::SW_RST_LU)) {
          // GAUDI2_0010 : CACHE_INVALIDATE.RST_LU is not resetting LUT$ pLRU
          // Problem     : Can't reset LUT$ pLRU using simple CACHE_INVALIDATE.RST_LU
          // Workaround  : Perform the dummy lookup after the CACHE_INVALIDATE.RST_LU instruction
          MachineRegisterInfo *MRI = &MF->getRegInfo();
          Register Income = MRI->createVirtualRegister(&TPC::VRFRegClass);
          BuildMI(MBB, MI, MI->getDebugLoc(), TII->get(TargetOpcode::IMPLICIT_DEF), Income);
          Register Src = MRI->createVirtualRegister(&TPC::VRFRegClass);
          BuildMI(MBB, MI, MI->getDebugLoc(), TII->get(TargetOpcode::IMPLICIT_DEF), Src);
          MachineInstr *dummyLookup = MF->CreateMachineInstr(TII->get(TPC::LOOKUPnodce), MI->getDebugLoc(), true);
          dummyLookup->addOperand(*MF, MachineOperand::CreateReg(TPC::LFSR, true));  // Destination.
          dummyLookup->addOperand(*MF, MachineOperand::CreateReg(Src,  false));   // Source #1.
          dummyLookup->addOperand(*MF, MachineOperand::CreateImm(0));                // Source #2.
          dummyLookup->addOperand(*MF, MachineOperand::CreateImm(0));                // Switch.
          dummyLookup->addOperand(*MF, MachineOperand::CreateReg(Income, false));  // Income.
          dummyLookup->addOperand(*MF, MachineOperand::CreateReg(TPC::SPRF_TRUE, false));  // Predicate.
          dummyLookup->addOperand(*MF, MachineOperand::CreateImm(0));                // Polarity.
          MBB.insertAfter(MI, dummyLookup);
          RestrictionReleased = true;
        }
      } 
    }
  }

  return RestrictionReleased;
}

bool TPCHWWAGeneral::ForceLFSRDefs(MachineFunction &Func) {
  bool IsChanged = false;
  
  for (MachineBasicBlock &MBB : Func) {
    std::unordered_set<unsigned> DefsHWReg;
    for (MachineInstr &MI : MBB) {
      // Uses
      for (unsigned I = 0; I < MI.getNumOperands(); ++I) {
        MachineOperand &MO = MI.getOperand(I);
        if (!MO.isReg())
          continue;
        if (!MO.isUse())
          continue;
        
        Register Reg = MO.getReg();
        // A undefined physical register
        if ((Reg == TPC::SPU_LFSR ||
             Reg == TPC::SPU_LFSR_RO ||
             Reg == TPC::VPU_LFSR ||
             Reg == TPC::VPU_LFSR_RO) &&
            DefsHWReg.find(Reg) == DefsHWReg.end()) {
          if (!MBB.isLiveIn(Reg)) {
            MBB.addLiveIn(Reg);
            IsChanged = true;
          }
          DefsHWReg.insert(Reg);
        }
      }
      
      // Defs
      for (unsigned I = 0; I < MI.getNumOperands(); ++I) {
        MachineOperand &MO = MI.getOperand(I);
        if (!MO.isReg())
          continue;
        if (!MO.isDef())
          continue;
        
        Register Reg = MO.getReg();
        if (Reg == TPC::SPU_LFSR ||
            Reg == TPC::VPU_LFSR)
          DefsHWReg.insert(Reg);
      }
    }
  }
  return IsChanged;
}

bool TPCHWWAGeneral::goya2HwWA(MachineFunction &Func) {
  MachineFunction *MF = &Func;
  bool Status = false;
  for (MachineBasicBlock &MBB : *MF) {
    std::map<Register, Register> mapForAshWA;
    for (MachineBasicBlock::iterator mi = MBB.begin(), me = MBB.end();
         mi != me;) {
      MachineInstr *MI = &*mi;
      mi++;
      auto OpCode = MI->getOpcode();
      if (OpCode == TPC::CALC_FP_SPECIALvvp || OpCode == TPC::CALC_FP_SPECIALvvm) {
        if (MI->getOperand(3).getImm() == TPCII::OpType::FP16) {
          unsigned functionId = MI->getOperand(4).getImm();
          MachineInstr *CMP_EQvip, *SEL, *CONV;
          unsigned calc_incom = 0, conv_result = 0;
          unsigned calc_result = MI->getOperand(0).getReg();
          switch (functionId) {
          case TPCII::SW_DIV:
          case TPCII::SW_POW:
            CONV = produceCONVERTVVm(Func, MI->getOperand(0).getReg(), MI, TPC::SPRF_TRUE, 0);
            updateRegister(MI, CONV);
            calc_incom = MI->getOperand(5).getReg();
            conv_result = CONV->getOperand(0).getReg();
            SEL = produceSEL_EQ(Func, CONV, calc_result, calc_incom, calc_incom, conv_result, conv_result);
            updateRegister(CONV, SEL);
            Status = true;
            break;
          default:
            CMP_EQvip = produceCMP_EQvip(Func, MI, MI->getOperand(1).getReg(), 0xbd, 1);
            unsigned cmp_result = CMP_EQvip->getOperand(0).getReg();
            CONV = produceCONVERTVVm(Func, calc_result, CMP_EQvip, cmp_result, 1);
            updateRegister(MI, CONV);
            Status = true;
            break;
          }
        }
      } else {
        MCInstrDesc Desc = (*MI).getDesc();
        auto opc = TPCII::getSlotOpCode(Desc);
        if (!TPCII::isVPUInst(Desc)) {
          continue;
        }

        if (opc == TPCII::vpuASH && (MI->getOperand(4).getImm() & TPCII::SW_RHAZ_RS)) {
          if (!EnableHwwaAshZeroScale)
            continue;

          MachineRegisterInfo *MRI = &MF->getRegInfo();
          auto parentMulInstr = MRI->getVRegDef(MI->getOperand(1).getReg());
          if (TPCII::getSlotOpCode(parentMulInstr->getDesc()) == TPCII::vpuMUL && (parentMulInstr->getOperand(4).getImm() & TPCII::SW_RND32_KEEP_RS)) {
            // Restriction below is relevant only for INT32/UINT32 OpTypes.
            assert (MI->getOperand(3).getImm() == TPCII::OpType::INT32 || MI->getOperand(3).getImm() == TPCII::OpType::UINT32);

            // Do not use ASH with RHAZ_RS when the shift value is 0.
            // Instead, it is possible to do MUL with KEEP_RS_FOR_ADD and
            // then use ADD in order to do the correct rounding without any shift.
            const TargetInstrInfo *TII = MF->getSubtarget().getInstrInfo();
            llvm::MachineOperand shift = MI->getOperand(2);
            if (shift.isReg()) {
              // We need to change the next instructions
              // D = MUL.KEEP_RS X, Y
              // R = ASH.RHAZ_RS D, Z
              //
              // to the following set of instructions
              //
              // D1 = MUL.KEEP_RS X, Y
              // R1 = ASH.RHAZ_RS D1, Z
              // D2 = MUL.KEEP_RS_FOR_ADD X, Y
              // R2 = ADD D2.V1, D2.V2
              // R  = SEL_EQ Z, 0, R2, R1

              MachineBasicBlock::iterator next = MI;
              MachineInstr *nextInstrAfterAsh = &*(++next);

              // Keep predication of original MUL to preserve semantics. So copy original income.
              MachineInstrBuilder copyOfMul = BuildMI(MBB, nextInstrAfterAsh, parentMulInstr->getDebugLoc(), TII->get(TargetOpcode::COPY), MRI->createVirtualRegister(&TPC::DRFRegClass));
              copyOfMul.addReg(parentMulInstr->getOperand(5).getReg());

              // Add new instruction : D2 = MUL.KEEP_RS_FOR_ADD X, Y.
              MachineInstrBuilder mulInstr = BuildMI(MBB, nextInstrAfterAsh, parentMulInstr->getDebugLoc(), parentMulInstr->getDesc(), MRI->createVirtualRegister(&TPC::DRFRegClass));
              mulInstr.addReg(parentMulInstr->getOperand(1).getReg());
              if (parentMulInstr->getOperand(2).isImm())
                mulInstr.addImm(parentMulInstr->getOperand(2).getImm());
              else
                mulInstr.addReg(parentMulInstr->getOperand(2).getReg());

              // Insert all necessary operands for MUL.
              mulInstr.addImm(parentMulInstr->getOperand(3).getImm());
              mulInstr.addImm(MachineOperand::CreateImm(TPCII::SW_RND32_KEEP_RS_FOR_ADD).getImm());
              mulInstr.addReg(copyOfMul->getOperand(0).getReg());
              mulInstr.addReg(parentMulInstr->getOperand(6).getReg());
              mulInstr.addImm(parentMulInstr->getOperand(7).getImm());

              // Add new instruction : R2 = ADD D2.V1, D2.V2.
              Register addPredicate = MachineOperand::CreateReg(TPC::SPRF_TRUE, false).getReg();
              MachineInstrBuilder addInstr = BuildMI(MBB, nextInstrAfterAsh, MI->getDebugLoc(), TII->get(TPC::ADDvvp), MRI->createVirtualRegister(&TPC::VRFRegClass));

              // Create an implicit income for ADD.
              Register addIncome = MRI->createVirtualRegister(&TPC::VRFRegClass);
              BuildMI(MBB, nextInstrAfterAsh, MI->getDebugLoc(), TII->get(TargetOpcode::IMPLICIT_DEF), addIncome);

              // Insert all necessary operands for ADD.
              addInstr.addReg(mulInstr->getOperand(0).getReg(), 0, TPC::sub_0); // Src #1.
              addInstr.addReg(mulInstr->getOperand(0).getReg(), 0, TPC::sub_1); // Src #2.
              addInstr.addImm(mulInstr->getOperand(3).getImm());                // Data type.
              addInstr.addImm(MachineOperand::CreateImm(0).getImm());           // Switch.
              addInstr.addReg(addIncome);                                       // Income.
              addInstr.addReg(addPredicate);                                    // Predicate.
              addInstr.addImm(MachineOperand::CreateImm(0).getImm());           // Polarity.

              // Create a copy for the shift operand of original ASH instruction.
              MachineInstrBuilder movOfAsh;
              Register shiftVector = shift.getReg();
              std::map<Register, Register>::iterator it = mapForAshWA.find(shiftVector);
              if (it == mapForAshWA.end()) {
                if (TPC::SRFRegClass.hasSubClassEq(MRI->getRegClass(shiftVector))) {
                  // Is the shift a scalar?
                  Register movIncome = MRI->createVirtualRegister(&TPC::VRFRegClass);
                  BuildMI(MBB, nextInstrAfterAsh, MI->getDebugLoc(), TII->get(TargetOpcode::IMPLICIT_DEF), movIncome);
                  movOfAsh = BuildMI(MBB, nextInstrAfterAsh, MI->getDebugLoc(), TII->get(TPC::MOVvsp), MRI->createVirtualRegister(&TPC::VRFRegClass));
                  movOfAsh.addReg(shift.getReg());                                      // Src.
                  movOfAsh.addImm(MI->getOperand(3).getImm());                          // Data type.
                  movOfAsh.addImm(MachineOperand::CreateImm(0).getImm());               // Switch.
                  movOfAsh.addReg(movIncome);                                           // Income.
                  movOfAsh.addReg(MachineOperand::CreateReg(TPC::SPRF_TRUE, false).getReg()); // Predicate.
                  movOfAsh.addImm(MachineOperand::CreateImm(0).getImm());               // Polarity.

                  shiftVector = movOfAsh->getOperand(0).getReg();
                  mapForAshWA.insert(std::pair <Register, Register>(shift.getReg(), shiftVector));
                } else {
                  mapForAshWA.insert(std::pair <Register, Register>(shiftVector, shiftVector));
                }
              } else {
                shiftVector = it->second;
              }

              if (it == mapForAshWA.end()) {
                if (EnableHwwaAshAndScale) {
                  // Add new instruction : shift & 0xFF
                  Register andIncome = MRI->createVirtualRegister(&TPC::VRFRegClass);
                  BuildMI(MBB, nextInstrAfterAsh, MI->getDebugLoc(), TII->get(TargetOpcode::IMPLICIT_DEF), andIncome);
                  MachineInstrBuilder andInstr = BuildMI(MBB, nextInstrAfterAsh, MI->getDebugLoc(), TII->get(TPC::ANDvip), MRI->createVirtualRegister(&TPC::VRFRegClass));
                  andInstr.addReg(shiftVector);                                         // Operand #1.
                  andInstr.addImm(MachineOperand::CreateImm(0xFF).getImm());            // Operand #2.
                  andInstr.addImm(MI->getOperand(3).getImm());                          // Data type.
                  andInstr.addImm(MachineOperand::CreateImm(0).getImm());               // Switch.
                  andInstr.addReg(andIncome);                                           // Income.
                  andInstr.addReg(MachineOperand::CreateReg(TPC::SPRF_TRUE, false).getReg()); // Predicate.
                  andInstr.addImm(MachineOperand::CreateImm(0).getImm());               // Polarity.

                  shiftVector = andInstr->getOperand(0).getReg();
                  mapForAshWA.insert(std::pair <Register, Register>(shift.getReg(), shiftVector));
                }
              }

              // Define the type of predicate (scalar - false, vector - true).
              bool ashVPred = false;
              switch (MI->getOpcode()) {
              case TPC::ASHvspRhaz:
              case TPC::ASHvvpRhaz:
                break;
              case TPC::ASHvvmRhaz:
              case TPC::ASHvsmRhaz:
                ashVPred = true;
                break;
              }

              // Add new instruction : R = SEL_EQ Z, 0, R2, R1.
              MachineInstrBuilder selEqInstr;
              if (ashVPred)
                selEqInstr= BuildMI(MBB, nextInstrAfterAsh, MI->getDebugLoc(), TII->get(TPC::SEL_EQvivvm), MI->getOperand(0).getReg());
              else
                selEqInstr= BuildMI(MBB, nextInstrAfterAsh, MI->getDebugLoc(), TII->get(TPC::SEL_EQvivvp), MI->getOperand(0).getReg());

              // Insert all necessary operands for SEL_EQ.
              Register ashInstrDest = MRI->createVirtualRegister(&TPC::VRFRegClass);
              selEqInstr.addReg(shiftVector);                           // Op #1  - aka shift from ASH but moved & modified possible.
              selEqInstr.addImm(MachineOperand::CreateImm(0).getImm()); // Op #2  - immediate 0.
              selEqInstr.addReg(addInstr->getOperand(0).getReg());      // Op #3  - destination of ADD instruction.
              selEqInstr.addReg(ashInstrDest);                          // Op #4  - destination of ASH instruction.
              selEqInstr.addImm(addInstr->getOperand(3).getImm());      // Data type.
              selEqInstr.addImm(MachineOperand::CreateImm(0).getImm()); // Switch - immediate 0.
              selEqInstr.addReg(MI->getOperand(6).getReg());            // Income - income of ASH instruction.
              selEqInstr.addReg(MI->getOperand(7).getReg());            // Pred.  - predicate of ASH instruction.
              selEqInstr.addImm(MI->getOperand(8).getImm());            // Polar. - polarity of ASH instruction.

              // Change original ASH instruction.
              // It should be w/o predicate because of its predicate is used with SEL_EQ instruction for now.
              MI->getOperand(0).setReg(ashInstrDest);
              Register ashIncome = MRI->createVirtualRegister(&TPC::VRFRegClass);
              BuildMI(MBB, nextInstrAfterAsh, MI->getDebugLoc(), TII->get(TargetOpcode::IMPLICIT_DEF), ashIncome);
              MI->getOperand(6).setReg(ashIncome);
              if (ashVPred)
                MI->getOperand(7).setReg(MachineOperand::CreateReg(TPC::VPRF_TRUE, false).getReg());
              else
                MI->getOperand(7).setReg(MachineOperand::CreateReg(TPC::SPRF_TRUE, false).getReg());

              MI->getOperand(8).setImm(MachineOperand::CreateImm(0).getImm());

              Status = true;
            } else if (shift.isImm() && shift.getImm() == 0) {
              // We need to change the next instructions
              // D1 = MUL.KEEP_RS X, Y
              // R  = ASH.RHAZ_RS D, 0
              //
              // to the following set of instructions
              //
              // D1 = MUL.KEEP_RS X, Y     // maybe unused, DCE will care.
              // D2 = MUL.KEEP_RS_FOR_ADD X, Y
              // R1 = ADD D2.V1, D2.V2

              // Add new instruction : D2 = MUL.KEEP_RS_FOR_ADD X, Y.
              MachineInstrBuilder copyOfMul = BuildMI(MBB, MI, parentMulInstr->getDebugLoc(), TII->get(TargetOpcode::COPY), MRI->createVirtualRegister(&TPC::DRFRegClass));
              copyOfMul.addReg(parentMulInstr->getOperand(5).getReg());

              MachineInstrBuilder mulInstr = BuildMI(MBB, MI, parentMulInstr->getDebugLoc(), parentMulInstr->getDesc(), MRI->createVirtualRegister(&TPC::DRFRegClass));
              mulInstr.addReg(parentMulInstr->getOperand(1).getReg());
              if (parentMulInstr->getOperand(2).isImm())
                mulInstr.addImm(parentMulInstr->getOperand(2).getImm());
              else
                mulInstr.addReg(parentMulInstr->getOperand(2).getReg());

              mulInstr.addImm(parentMulInstr->getOperand(3).getImm());
              mulInstr.addImm(MachineOperand::CreateImm(TPCII::SW_RND32_KEEP_RS_FOR_ADD).getImm());
              mulInstr.addReg(copyOfMul->getOperand(0).getReg());
              mulInstr.addReg(parentMulInstr->getOperand(6).getReg());
              mulInstr.addImm(parentMulInstr->getOperand(7).getImm());

              // Add new instruction : R = ADD D2.V1, D2.V2.
              MachineInstrBuilder addInstr = BuildMI(MBB, MI, MI->getDebugLoc(), TII->get(TPC::ADDvvp), MI->getOperand(0).getReg());
              addInstr.addReg(mulInstr->getOperand(0).getReg(), 0, TPC::sub_0);
              addInstr.addReg(mulInstr->getOperand(0).getReg(), 0, TPC::sub_1);
              addInstr.addImm(mulInstr->getOperand(3).getImm());
              addInstr.addImm(MachineOperand::CreateImm(0).getImm()); // Switch.
              addInstr.addReg(MI->getOperand(6).getReg()); // ASH income.
              addInstr.addReg(MI->getOperand(7).getReg()); // ASH predicate.
              addInstr.addImm(MI->getOperand(8).getImm()); // ASH polarity.

              MBB.remove(MI);
              Status = true;
            }
          }
        }
      }
    }
  }
  return Status;
}


static MachineBasicBlock::iterator ReplacePhiToSQZPhi(MachineInstr &OldPhi,
                                                      Register NewDstReg) {
  MachineFunction &MF = *OldPhi.getMF();
  MachineRegisterInfo &MRI = MF.getRegInfo();
  const TargetInstrInfo *TII = MF.getSubtarget().getInstrInfo();
  
  const auto CreateCopyToSQZ = [&MF, &MRI, &TII](const Register &Reg,
                                                   const DebugLoc& Loc) {
    Register SQZReg = MRI.createVirtualRegister(&TPC::HWSqzCntrRegClass);
    return BuildMI(MF, Loc, TII->get(TargetOpcode::COPY), SQZReg)
    .addReg(Reg)
    .getInstr();
  };
  
  // Get Info from Old PHI
  const DebugLoc &PhiLoc = OldPhi.getDebugLoc();
  MachineBasicBlock &PhiBlock = *OldPhi.getParent();
  
  SmallVector<MachineOperand, 4> Operands;
  
  for (MachineOperand &MO : OldPhi.uses()) {
    if (MO.isReg()) {
      Register Reg = MO.getReg();
      MachineInstr &RegDefInst = *MRI.def_instructions(Reg).begin();
      MachineInstr *CopyInst = CreateCopyToSQZ(Reg, PhiLoc);
      RegDefInst.getParent()->insertAfter(RegDefInst.getIterator(), CopyInst);
      Operands.push_back(MachineOperand::CreateReg(
                           CopyInst->getOperand(0).getReg(), false));
    } else if (MO.isMBB()) {
      Operands.push_back(MachineOperand::CreateMBB(MO.getMBB()));
    } else {
      llvm_unreachable("A unexpected Phi type of operand");
    }
  }
  
  // Build new Phi
  BuildMI(PhiBlock, OldPhi.getIterator(), PhiLoc,
          TII->get(TargetOpcode::PHI), NewDstReg)
    .add(Operands);
  
  // Remove old phi
  return PhiBlock.erase(OldPhi.getIterator());
}

bool TPCHWWAGeneral::RemoveSQZCopy(MachineFunction &Func) {
  MachineRegisterInfo &MRI = Func.getRegInfo();
  
  bool HasChanges;
  bool IsChanged = false;
  do {
    HasChanges = false;
    for (MachineBasicBlock &MBB : Func) {
      for (MachineBasicBlock::iterator MBI = MBB.begin(); MBI != MBB.end(); ++MBI) {
        MachineInstr &MI = *MBI;
        if (!MI.isCopy())
          continue;
        
        MachineOperand &DstMO = MI.getOperand(0);
        MachineOperand &SrcMO = MI.getOperand(1);

        Register DstReg = DstMO.getReg();
        Register SrcReg = SrcMO.getReg();
        
        if (!DstReg.isVirtual())
          continue;
        if (!SrcReg.isVirtual())
          continue;

        if (MRI.getRegClass(DstReg) == &TPC::HWSqzCntrRegClass &&
            MRI.getRegClass(SrcReg) == &TPC::HWSqzCntrRegClass) {
          SmallVector<MachineOperand *, 2> SwitchedMO;
          for (MachineOperand &MO : MRI.use_operands(DstReg))
            SwitchedMO.push_back(&MO);
          for (MachineOperand *MO : SwitchedMO)
            MO->setReg(SrcReg);
          MBI = MBB.erase(MBI);
          HasChanges = true;
        } else if (MRI.getRegClass(DstReg) == &TPC::HWSqzCntrRegClass &&
                   MRI.getRegClass(SrcReg) == &TPC::SRFRegClass) {
          MachineInstr &SrcDefInst = *MRI.def_instructions(SrcReg).begin();
          if (SrcDefInst.isPHI()) {
            bool IsBadPhi = false;
            bool HasPhiUser = false;
            for (MachineInstr &MI : MRI.use_instructions(SrcReg)) {
              if (MI.isPHI()) {
                HasPhiUser = true;
              } else if (!MI.isCopy() && !MI.isDebugInstr()) {
                assert(false && "Can not replace phi with non copy users");
                IsBadPhi = true;
              }
            }
            if (!IsBadPhi && !HasPhiUser) {
              SmallVector<MachineOperand *, 2> SwitchedMO;
              for (MachineOperand &MO : MRI.use_operands(SrcReg))
                SwitchedMO.push_back(&MO);
              for (MachineOperand *MO : SwitchedMO)
                MO->setReg(DstReg);
              ReplacePhiToSQZPhi(SrcDefInst, DstReg);
              MBI = MBB.erase(MBI);
              HasChanges = true;
            }
          } else if (SrcDefInst.isCopy()) {
            bool IsBadCopy = false;
            bool HasPhiUser = false;
            for (MachineInstr &MI : MRI.use_instructions(SrcReg))
              if (MI.isPHI()) {
                HasPhiUser = true;
              } else if (!MI.isCopy() && !MI.isDebugInstr()) {
                assert(false && "Can not replace copy with non copy users");
                IsBadCopy = true;
              }
            
            if (!IsBadCopy && !HasPhiUser) {
              SmallVector<MachineOperand *, 2> SwitchedMO;
              for (MachineOperand &MO : MRI.use_operands(SrcReg))
                SwitchedMO.push_back(&MO);
              for (MachineOperand *MO : SwitchedMO)
                MO->setReg(DstReg);
              SrcDefInst.getOperand(0).setReg(DstReg);
              MBI = MBB.erase(MBI);
              HasChanges = true;
            }
          }
          // If SrcDefInst is copy, the next branch process this copy.
        } else if (MRI.getRegClass(DstReg) == &TPC::SRFRegClass &&
                   MRI.getRegClass(SrcReg) == &TPC::HWSqzCntrRegClass) {
          bool IsBadCopy = false;
          bool HasPhiUser = false;
          for (MachineInstr &MI : MRI.use_instructions(DstReg)) {
            if (MI.isPHI()) {
              HasPhiUser = true;
            } else if (!MI.isCopy() && !MI.isDebugInstr()) {
              assert(false && "Can not replace copy with non copy users");
              IsBadCopy = true;
            }
          }
          if (!IsBadCopy && !HasPhiUser) {
            SmallVector<MachineOperand *, 2> SwitchedMO;
            for (MachineOperand &MO : MRI.use_operands(DstReg))
              SwitchedMO.push_back(&MO);
            for (MachineOperand *MO : SwitchedMO)
              MO->setReg(SrcReg);
            MBI = MBB.erase(MBI);
            HasChanges = true;
          }
        } else if (MRI.getRegClass(DstReg) == &TPC::HWSqzCntrRegClass ||
                   MRI.getRegClass(SrcReg) == &TPC::HWSqzCntrRegClass) {
          assert(false && "Unexpected types of registers from copy");
        }
      }
    }
    IsChanged = IsChanged || HasChanges;
  } while (HasChanges);
  
  return IsChanged;
}

// LOOKUP with BV32 can’t use the same VRF as both source and destination.
bool TPCHWWAGeneral::ProcessLookupBV32(MachineFunction &Func) {
  assert(Func.getSubtarget<TPCSubtarget>().hasDoron1());
  
  bool Changed = false;
  
  for (auto &MBB : Func) {
    for (auto &MI : MBB) {
      const MCInstrDesc &Desc = MI.getDesc();
      unsigned SlotOpcode = TPCII::getSlotOpCode(Desc);
      if ((SlotOpcode == TPCII::LOOKUP ||
           SlotOpcode == TPCII::LOOKUP_2C ||
           SlotOpcode == TPCII::LOOKUP_1C) &&
          TPCII::isLoadInst(Desc)) {
        unsigned SwitchVal = getSwitches(MI);
        unsigned DtType = SwitchVal & TPCII::SW_LOOKUP_G3;
        
        if (DtType == 0) {
          MI.getOperand(0).setIsEarlyClobber();
          Changed = true;
        }
      }
    }
  }
  
  return Changed;
}
