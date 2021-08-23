//===- TPCHwWaGeneral.cpp--------General pass for transformation-----------===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
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

#define DEBUG_TYPE "hwwaGeneral"

using namespace llvm;

namespace llvm {
FunctionPass *createTPCHWWAGeneral();
void initializeTPCHWWAGeneralPass(PassRegistry &);
} // namespace llvm

//
// Option to enable/disable ASH restriction.
//
static cl::opt<bool> EnableHwwaAshZeroScale("tpc-hwwa-ash-zero-scale", cl::ZeroOrMore, cl::init(false));
static cl::opt<bool> EnableHwwaAshAndScale("tpc-hwwa-ash-and-scale", cl::ZeroOrMore, cl::init(true));

static const char PassDescription[] =
    "TPC Hardware Workarounds pass (general pass)";
static const char PassName[] = "tpc-hwwa-general-workaround";

namespace {
class TPCHWWAGeneral : public MachineFunctionPass {
private:
  unsigned NumReplaced = 0;
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
      *MF, MachineOperand::CreateReg(TPC::SP0, false)); // predicate
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
  const MCInstrDesc &MID = predicate == TPC::SP0 ? TII->get(TPC::CONVERTvvp)
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
  SEL_EQ->addOperand(*MF, MachineOperand::CreateReg(TPC::SP0, false)); // predicate 
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
    auto firstInst = MF->getBlockNumbered(0)->instr_begin();
    MachineInstr *cacheIns = MF->CreateMachineInstr(TII->get(TPC::CACHE_INVALIDATE), firstInst->getDebugLoc(), true);
    cacheIns->addOperand(*MF, MachineOperand::CreateImm(0));
    cacheIns->addOperand(*MF, MachineOperand::CreateReg(TPC::SP0, false));
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
  bool Status = lookupWorkAround(Func);
  return Status;
}
