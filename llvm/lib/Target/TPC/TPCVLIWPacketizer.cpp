//===- TPCVLIWPacketizer.cpp ----------------------------------------------===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//
//
//
//
//===----------------------------------------------------------------------===//

#include "TPCRegisterInfo.h"
#include "TPCSubtarget.h"
#include "TPCTargetMachine.h"
#include "MCTargetDesc/TPCMCTargetDesc.h"
#include "MCTargetDesc/TPCMCInstrInfo.h"
#include "TPC.h"
#include "TPCVLIWPacketizer.h"
#include "llvm/InitializePasses.h"
#include "llvm/Analysis/AliasAnalysis.h"
#include "llvm/CodeGen/MachineDominators.h"
#include "llvm/CodeGen/MachineFunctionPass.h"
#include "llvm/CodeGen/MachineLoopInfo.h"
#include "llvm/CodeGen/MachineRegisterInfo.h"
#include "llvm/CodeGen/Passes.h"
#include "llvm/CodeGen/MachineInstrBuilder.h"
#include "llvm/Support/CommandLine.h"
#include "llvm/Support/Debug.h"
#include "TPCInstrInfo.h"

using namespace llvm;

#define DEBUG_TYPE "packets"

static cl::opt<bool> DisablePacketizer("disable-tpc-packetizer", cl::Hidden,
  cl::ZeroOrMore, cl::init(false),
  cl::desc("Disable TPC packetizer pass"));

static cl::opt<bool> TPCSinglePackets("tpc-single-packets", cl::Hidden,
  cl::ZeroOrMore, cl::init(false),
  cl::desc("TPC debug mode (single instr in a VLIW)"));

extern cl::opt<bool> ScheduleInlineAsm;

namespace llvm {
  FunctionPass *createTPCPacketizer();
  void initializeTPCPacketizerPass(PassRegistry&);
}


namespace {
  class TPCPacketizer : public MachineFunctionPass {
  public:
    static char ID;
    TPCPacketizer() : MachineFunctionPass(ID) {
      initializeTPCPacketizerPass(*PassRegistry::getPassRegistry());
      HII = nullptr;
      HRI = nullptr;
    }

    void getAnalysisUsage(AnalysisUsage &AU) const override {
      AU.setPreservesCFG();
      AU.addRequired<AAResultsWrapperPass>();
      AU.addRequired<MachineBranchProbabilityInfo>();
      AU.addRequired<MachineDominatorTree>();
      AU.addRequired<MachineLoopInfo>();
      AU.addPreserved<MachineDominatorTree>();
      AU.addPreserved<MachineLoopInfo>();
      MachineFunctionPass::getAnalysisUsage(AU);
    }
    StringRef getPassName() const override { return "TPC Packetizer"; }
    bool runOnMachineFunction(MachineFunction &Fn) override;
    MachineFunctionProperties getRequiredProperties() const override {
      return MachineFunctionProperties().set(
          MachineFunctionProperties::Property::NoVRegs);
    }

  private:
    const TPCInstrInfo *HII;
    const TPCRegisterInfo *HRI;
  };

  char TPCPacketizer::ID = 0;
}

INITIALIZE_PASS_BEGIN(TPCPacketizer, "tpc-packets", "TPC Packetizer",
                      false, false)
INITIALIZE_PASS_DEPENDENCY(MachineDominatorTree)
INITIALIZE_PASS_DEPENDENCY(MachineBranchProbabilityInfo)
INITIALIZE_PASS_DEPENDENCY(MachineLoopInfo)
INITIALIZE_PASS_DEPENDENCY(AAResultsWrapperPass)
INITIALIZE_PASS_END(TPCPacketizer, "tpc-packets", "TPC Packetizer",
                    false, false)

TPCPacketizerList::TPCPacketizerList(MachineFunction &MF,
      MachineLoopInfo &MLI, AAResults *AA,
      const MachineBranchProbabilityInfo *MBPI)
    : VLIWPacketizerList(MF, MLI, AA), MBPI(MBPI), MLI(&MLI) {
  HII = MF.getSubtarget<TPCSubtarget>().getInstrInfo();
  HRI = MF.getSubtarget<TPCSubtarget>().getRegisterInfo();
  Dependence = false;
  PacketNum = 0;

  addMutation(std::make_unique<TPCSubtarget::TPCDAGMutation>());
}

bool TPCPacketizer::runOnMachineFunction(MachineFunction &MF) {
  if (DisablePacketizer || skipFunction(MF.getFunction()))
    return false;

  HII = MF.getSubtarget<TPCSubtarget>().getInstrInfo();
  HRI = MF.getSubtarget<TPCSubtarget>().getRegisterInfo();
  auto &MLI = getAnalysis<MachineLoopInfo>();
  auto *AA = &getAnalysis<AAResultsWrapperPass>().getAAResults();
  auto *MBPI = &getAnalysis<MachineBranchProbabilityInfo>();

  // Instantiate the packetizer.
  TPCPacketizerList Packetizer(MF, MLI, AA, MBPI);

  // DFA state table should not be empty.
  assert(Packetizer.getResourceTracker() && "Empty DFA table!");

  if (Packetizer.getResourceTracker()->getInstrItins()->isEmpty()) {
    return false;
  }

  //
  // Loop over all basic blocks and remove KILL pseudo-instructions
  // These instructions confuse the dependence analysis. Consider:
  // D0 = ...   (Insn 0)
  // R0 = KILL R0, D0 (Insn 1)
  // R0 = ... (Insn 2)
  // Here, Insn 1 will result in the dependence graph not emitting an output
  // dependence between Insn 0 and Insn 2. This can lead to incorrect
  // packetization
  //
  for (auto &MB : MF) {
    auto End = MB.end();
    auto MI = MB.begin();
    while (MI != End) {
      auto NextI = std::next(MI);
      if (MI->isKill() || MI->getOpcode() == TPC::IMPLICIT_DEF) {
        MB.erase(MI);
        End = MB.end();
      }
      MI = NextI;
    }
  }

  // Loop over all of the basic blocks.
  for (auto &MB : MF) {
    Packetizer.PacketNum = 0;
    Packetizer.PacketizeMIs(&MB, MB.begin(), MB.end());
  }

  Packetizer.unpacketizeSoloInstrs(MF);
  return true;
}


// Initialize packetizer flags.
void TPCPacketizerList::initPacketizerState() {
  Dependence = false;
}

// Ignore bundling of pseudo instructions.
bool TPCPacketizerList::ignorePseudoInstruction(const MachineInstr &MI,
                                                    const MachineBasicBlock *) {
  if (MI.isDebugValue())
    return true;

  if (MI.isCFIInstruction())
    return false;

  // We must print out inline assembly.
  if (MI.isInlineAsm())
    return false;

  if (MI.isImplicitDef())
    return false;

  // We check if MI has any functional units mapped to it. If it doesn't,
  // we ignore the instruction.
  const MCInstrDesc& TID = MI.getDesc();

  auto *IS = ResourceTracker->getInstrItins()->beginStage(TID.getSchedClass());
  unsigned FuncUnits = IS->getUnits();
  return !FuncUnits;
}

bool TPCPacketizerList::isSoloInstruction(const MachineInstr &MI) {
  if (MI.isInlineAsm() /*&& !ScheduleInlineAsm*/) {
    return true;
  }
  if (MI.getOpcode() == TPC::LOOPEND) {
    return true;
  }
  switch (TPCII::getSlotOpCode(MI.getDesc())) {
  case TPCII::spuLOOP:
    return TPCII::getInstrType(MI.getDesc()) == TPCII::TypeLOOP;
  default:
    return false;
  }
}

static MachineBasicBlock::iterator moveInstrOut(MachineInstr &MI,
      MachineBasicBlock::iterator BundleIt, bool Before) {
  MachineBasicBlock::instr_iterator InsertPt;
  if (Before)
    InsertPt = BundleIt.getInstrIterator();
  else
    InsertPt = std::next(BundleIt).getInstrIterator();

  MachineBasicBlock &B = *MI.getParent();
  // The instruction should at least be bundled with the preceding instruction
  // (there will always be one, i.e. BUNDLE, if nothing else).
  assert(MI.isBundledWithPred());
  if (MI.isBundledWithSucc()) {
    MI.clearFlag(MachineInstr::BundledSucc);
    MI.clearFlag(MachineInstr::BundledPred);
  } else {
    // If it's not bundled with the successor (i.e. it is the last one
    // in the bundle), then we can simply unbundle it from the predecessor,
    // which will take care of updating the predecessor's flag.
    MI.unbundleFromPred();
  }
  B.splice(InsertPt, &B, MI.getIterator());

  // Get the size of the bundle without asserting.
  MachineBasicBlock::const_instr_iterator I = BundleIt.getInstrIterator();
  MachineBasicBlock::const_instr_iterator E = B.instr_end();
  unsigned Size = 0;
  for (++I; I != E && I->isBundledWithPred(); ++I)
    ++Size;

  // If there are still two or more instructions, then there is nothing
  // else to be done.
  if (Size > 1)
    return BundleIt;

  // Otherwise, extract the single instruction out and delete the bundle.
  MachineBasicBlock::iterator NextIt = std::next(BundleIt);
  MachineInstr &SingleI = *BundleIt->getNextNode();
  SingleI.unbundleFromPred();
  assert(!SingleI.isBundledWithSucc());
  BundleIt->eraseFromParent();
  return NextIt;
}

// Check if FirstI modifies a register that SecondI reads.
static bool hasWriteToReadDep(const MachineInstr &FirstI,
                              const MachineInstr &SecondI,
                              const TargetRegisterInfo *TRI) {
  for (auto &MO : FirstI.operands()) {
    if (!MO.isReg() || !MO.isDef())
      continue;
    unsigned R = MO.getReg();
    if (SecondI.readsRegister(R, TRI))
      return true;
  }
  return false;
}

void TPCPacketizerList::unpacketizeSoloInstrs(MachineFunction &MF) {
  for (auto &B : MF) {
    MachineBasicBlock::iterator BundleIt;
    MachineBasicBlock::instr_iterator NextI;
    for (auto I = B.instr_begin(), E = B.instr_end(); I != E; I = NextI) {
      NextI = std::next(I);
      MachineInstr &MI = *I;
      if (MI.isBundle())
        BundleIt = I;
      if (!MI.isInsideBundle())
        continue;
      bool InsertBeforeBundle;
      if (MI.isInlineAsm())
        InsertBeforeBundle = !hasWriteToReadDep(MI, *BundleIt, HRI);
      else if (MI.isDebugValue())
        InsertBeforeBundle = true;
      else
        continue;

      BundleIt = moveInstrOut(MI, BundleIt, InsertBeforeBundle);
    }
  }
}

static bool isControlFlow(const MachineInstr &MI) {
  return MI.getDesc().isTerminator();
}

#if 0
// Currently not used.
// Return true when ConsMI uses a register defined by ProdMI.
static bool isDependent(const MachineInstr &ProdMI, const MachineInstr &ConsMI) {
  if (!ProdMI.getOperand(0).isReg())
    return false;
  unsigned DstReg = ProdMI.getOperand(0).getReg();

  for (auto &Op : ConsMI.operands())
    if (Op.isReg() && Op.isUse() && Op.getReg() == DstReg)
      // The MIs depend on each other.
      return true;

  return false;
}
#endif

bool TPCPacketizerList::hasDeadDependence(const MachineInstr &I,
                                          const MachineInstr &J) {
  // The dependence graph may not include edges between dead definitions,
  // so without extra checks, we could end up packetizing two instruction
  // defining the same (dead) register.

  if (HII->isPredicated(I) || HII->isPredicated(J))
    return false;

  BitVector DeadDefs(256); // TODO: need to put a named constant here
  for (auto &MO : I.operands()) {
    if (!MO.isReg() || !MO.isDef() || !MO.isDead())
      continue;
    DeadDefs[MO.getReg()] = true;
  }

  for (auto &MO : J.operands()) {
    if (!MO.isReg() || !MO.isDef() || !MO.isDead())
      continue;
    unsigned R = MO.getReg();
    if (DeadDefs[R]) {
      LLVM_DEBUG(dbgs() << "Dead dependency between " << I << " and " << J << "\n");
      return true;
    }
  }
  return false;
}

bool TPCPacketizerList::hasControlDependence(const MachineInstr &I,
                                             const MachineInstr &J) {

  // Two control flow instructions cannot go in the same packet.
  if (isControlFlow(I) && isControlFlow(J)) {
     LLVM_DEBUG(dbgs() << "Ctrl dependency between " << I << " and " << J << "\n");
     return true;
  }
  return false;
}

bool TPCPacketizerList::hasTPCSpecificDependence(const MachineInstr &I,
                                                 const MachineInstr &J) {
  // Immediate sharing
  bool hasImmI = (HII->instHasImm(I));
  bool hasImmJ = (HII->instHasImm(J));
  bool hasImmField = (HII->instHasImmField(I) || HII->instHasImmField(J));
  if (hasImmI && hasImmJ && !hasImmField) {
    uint64_t immI = HII->getInstImm(I);
    uint64_t immJ = HII->getInstImm(J);
    if (immI != immJ) {
      LLVM_DEBUG(dbgs() << "Imm field dependency between " << I << " and " << J << "\n");
      return true;
    }
  }

  // LD/ST predicate sharing
  unsigned pI = 0;
  unsigned pJ = 0;
  unsigned ppI = 0;
  unsigned ppJ = 0;
  bool ldstI = (HII->isLDSTInstrWithPredicate(I, pI, ppI));
  bool ldstJ = (HII->isLDSTInstrWithPredicate(J, pJ, ppJ));
  if (ldstI && ldstJ) {
    if ((pI != pJ) || (ppI != ppJ)) {
      LLVM_DEBUG(dbgs() << "Predicate dependency between " << I << " and " << J << "\n");
      return true;
    }
  }

  // 1.3.4. General Restrictions
  // CACHE FLUSH/INVALIDATE or ASO with Evict and LD_G cannot be scheduled in the same VLIW instruction
  //
  {
    bool restrict1 = false;
    bool restrict2 = false;
    bool r_restrict1 = false;
    bool r_restrict2 = false;
    if (TPCII::isLoadInst(I.getDesc()) && TPCII::getSlotOpCode(I.getDesc()) == TPCII::LD_G) {
      restrict1 = true;
    }
    else  if (TPCII::isLoadInst(J.getDesc()) && TPCII::getSlotOpCode(J.getDesc()) == TPCII::LD_G) {
      r_restrict1 = true;
    }
    if (restrict1) {
      switch (J.getOpcode()) {
        case TPC::CACHE_FLUSH:
        case TPC::CACHE_INVALIDATE:
        case TPC::ASO:
          restrict2 = true;
          break;
        default:;
      }
    }
    if (r_restrict1) {
      switch (I.getOpcode()) {
        case TPC::CACHE_FLUSH:
        case TPC::CACHE_INVALIDATE:
        case TPC::ASO:
          r_restrict2 = true;
          break;
        default:;
      }
    }

    if ((restrict1 && restrict2) || (r_restrict1 && r_restrict2)) {
      LLVM_DEBUG(dbgs() << "CACHE and LD_G dependency between " << I << " and " << J << "\n");
      return true;
    }
  }

  unsigned sopcI = TPCII::getSlotOpCode(I.getDesc());
  unsigned sopcJ = TPCII::getSlotOpCode(J.getDesc());

  // From PRM:
  // LOAD and STORE issue slots share the same resource (spill RAM), and cannot
  // access it simultaneously. On the LOAD issue slot, the LD_L* and LOOKUP*
  // instructions access the spill RAM. On the STORE issue slot, all ST_L*
  // instructions access this SRAM. The compiler should avoid scheduling both
  // the stated instruction on the LOAD issue slot and the stated insruction
  // on the STORE issue slot in the same VLIW instruction.
  if (TPCII::isLookupC(I.getDesc()) ||
      (TPCII::isLoadInst(I.getDesc()) && sopcI == TPCII::LOOKUP) ||
      (TPCII::isLoadInst(I.getDesc()) && (sopcI >= 11 && sopcI <= 16))   // LD_L*
  ) {
    if (TPCII::isStoreInst(J.getDesc()) && (sopcJ >= TPCII::ST_L && sopcJ <= TPCII::ST_L_V_HIGH)) {
      return true;
    }
  }
  if (TPCII::isLookupC(J.getDesc()) ||
      (TPCII::isLoadInst(J.getDesc()) && sopcJ == TPCII::LOOKUP) ||
      (TPCII::isLoadInst(J.getDesc()) && (sopcJ >= 11 && sopcJ <= 16))   // LD_L*
  ) {
    if (TPCII::isStoreInst(I.getDesc()) && (sopcI >= TPCII::ST_L && sopcI <= TPCII::ST_L_V_HIGH)) {
      return true;
    }
  }

  // 1.3.4. General Restrictions
  // All generations: ST_G and LD_G cannot be scheduled in the same VLIW instruction
  if (TPCII::isLoadInst(I.getDesc()) && TPCII::getSlotOpCode(I.getDesc()) == TPCII::LD_G &&
      TPCII::isStoreInst(J.getDesc()) && TPCII::getSlotOpCode(J.getDesc()) == TPCII::ST_G) {
      return true;
  }
  if (TPCII::isLoadInst(J.getDesc()) && TPCII::getSlotOpCode(J.getDesc()) == TPCII::LD_G &&
      TPCII::isStoreInst(I.getDesc()) && TPCII::getSlotOpCode(I.getDesc()) == TPCII::ST_G) {
      return true;
  }
  // All except Gen1 (Dali) ST_G and LD_G/PREFETCH cannot be scheduled in the same VLIW instruction
  if (!MF.getSubtarget<TPCSubtarget>().hasGoyaISA()) {
    if (TPCII::isLoadInst(I.getDesc()) && TPCII::getSlotOpCode(I.getDesc()) == TPCII::PREFETCH &&
        TPCII::isStoreInst(J.getDesc()) && TPCII::getSlotOpCode(J.getDesc()) == TPCII::ST_G) {
        return true;
    }
    if (TPCII::isLoadInst(J.getDesc()) && TPCII::getSlotOpCode(J.getDesc()) == TPCII::PREFETCH &&
        TPCII::isStoreInst(I.getDesc()) && TPCII::getSlotOpCode(I.getDesc()) == TPCII::ST_G) {
        return true;
    }
  }
  
  // 1.3.4. General Restrictions
  // Assertion 1: The maximum number of SRF or SPRF sources allowed
  // in 1 VLIW instruction which includes the following is 1:
  //   - MOV to V or VP
  //   - LD_L_V* (only for Dali)
  //   - VPU instruction
  //
  // Hilla Ben Yaacov wrote on 11/03/2020:
  //
  // Let me explain the HW mechanism:
  // The instructions are decoded in SPU, and then written to an Instruction-Queue for the VPU.
  // The instructions in the Instruction-Queue have a slightly different format
  // (see sheet Vector Pipe Instruction Encoding  in the ISA excel).
  //
  // In addition to the regular fields like VPU_OPCODE, LOAD_OPCODE etc.,
  // there is some meta-data coming as well.
  // You can see there the field LOAD_VPU_EMBEDDED_S.
  // This field (referred to as LD_VPU_EMBEDDED_S in other sheets of the ISA excel)
  // is used for transferring the required SRF/SPRF value to the vector pipe.
  //
  // In Goya, you can see that all 3 instructions are using the same field
  // LD_L_V, MOV from SRF/SPRF to VRF/VPRF, and VPU with SRF (you can see it on the
  // right hand side of the excel sheet).
  //
  // In Gaudi this restriction can be mitigated, because we added a separate field
  // (LD_VLM_ADDR) for LD_L_V.
  //
  // Therefore in Gaudi the restriction holds only for MOV S->V and VPU using SRF.

  bool ldlv_I = (TPCII::isLoadInst(I.getDesc()) &&
                (sopcI == TPCII::LD_L_V || sopcI == TPCII::LD_L_V_LOW || sopcI == TPCII::LD_L_V_HIGH));
  bool ldlv_J = (TPCII::isLoadInst(J.getDesc()) &&
                (sopcJ == TPCII::LD_L_V || sopcJ == TPCII::LD_L_V_LOW || sopcJ == TPCII::LD_L_V_HIGH));
  bool isIMovSToV = (TPCII::isLoadInst(I.getDesc()) &&
                    (sopcI == TPCII::ldMOV) && HII->isScalarToVector(I));
  bool isJMovSToV = (TPCII::isLoadInst(J.getDesc()) &&
                    (sopcJ == TPCII::ldMOV) && HII->isScalarToVector(J));
  if (MF.getSubtarget<TPCSubtarget>().hasGaudiISA()) {
    if (isIMovSToV || (TPCII::isVPUInst(I.getDesc()) && HII->hasSRFOrSPRFOperands(I))) {
      if (isJMovSToV || (TPCII::isVPUInst(J.getDesc()) && HII->hasSRFOrSPRFOperands(J))) {
        LLVM_DEBUG(dbgs() << "SRF/SPRF dependency between " << I << " and " << J << "\n");
        return true;
      }
    }
  }
  else { // Dali
    if (isIMovSToV || ldlv_I || (TPCII::isVPUInst(I.getDesc()) && HII->hasSRFOrSPRFOperands(I))) {
      if (isJMovSToV || ldlv_J || (TPCII::isVPUInst(J.getDesc()) && HII->hasSRFOrSPRFOperands(J))) {
        LLVM_DEBUG(dbgs() << "SRF/SPRF dependency between " << I << " and " << J << "\n");
        return true;
      }
    }
  }

  // 1.3.4. General Restrictions
  // Assertion 1: If a VPU instruction accepts an SRF as input :
  //    - LD_L_V must not be scheduled in the same VLIW instruction.
  //    - MOV from SRF to V or VP must not be scheduled in LOAD slot in the same VLIW
  //      instruction.
  if (HII->isVPUInstrWithSRF(I)) {
    if (TPCII::isLoadInst(J.getDesc()) && (sopcJ == 14 || sopcJ == 15 || sopcJ == 16)) { // LD_L_V
      return true;
    }
    if (HII->isMovSRFtoVInstr(J)) {
      return true;
    }
  }
  if (HII->isVPUInstrWithSRF(J)) {
    if (TPCII::isLoadInst(I.getDesc()) && (sopcI == 14 || sopcI == 15 || sopcI == 16)) { // LD_L_V
      return true;
    }
    if (HII->isMovSRFtoVInstr(I)) {
      return true;
    }
  }

  // MUL IRF,* on an SPU issue slot and SET_INDX/PRMT_INDX/GEN_ADDR/ST_TNSR*
  // on a Store issue slot can not be scheduled together in the same VLIW
  // instruction.
  auto HasIRFReg = [](const MachineInstr &MI) -> bool {
    for (unsigned i = 0; i < MI.getNumOperands(); ++i) {
      MachineOperand MO = MI.getOperand(i);
      if (MO.isReg() && TPC::IRFRegClass.contains(MO.getReg()))
        return true;
    }
    return false;
  };

  bool IIsMulWithIRF = false;
  bool JIsMulWithIRF = false;

  if (TPCII::isSPUInst(I.getDesc()) &&
      TPCII::getSlotOpCode(I.getDesc()) == TPCII::spuMUL) {
    IIsMulWithIRF = HasIRFReg(I);
  }
  if (TPCII::isSPUInst(J.getDesc()) &&
      TPCII::getSlotOpCode(J.getDesc()) == TPCII::spuMUL) {
    JIsMulWithIRF = HasIRFReg(J);
  }

  if (IIsMulWithIRF && TPCII::isStoreInst(J.getDesc()) &&
      (sopcJ == TPCII::stSET_INDX || sopcJ == TPCII::stPRMT_INDX ||
       sopcJ == TPCII::stGEN_ADDR || sopcJ == TPCII::ST_TNSR ||
       sopcJ == TPCII::ST_TNSR_HIGH || sopcJ == TPCII::ST_TNSR_LOW ||
       sopcJ == TPCII::stLD_TNSR || sopcJ == TPCII::stLD_TNSR_HIGH ||
       sopcJ == TPCII::stLD_TNSR_LOW))
    return true;

  if (JIsMulWithIRF && TPCII::isStoreInst(I.getDesc()) &&
      (sopcI == TPCII::stSET_INDX || sopcI == TPCII::stPRMT_INDX ||
       sopcI == TPCII::stGEN_ADDR || sopcI == TPCII::ST_TNSR ||
       sopcI == TPCII::ST_TNSR_HIGH || sopcI == TPCII::ST_TNSR_LOW ||
       sopcI == TPCII::stLD_TNSR || sopcI == TPCII::stLD_TNSR_HIGH ||
       sopcI == TPCII::stLD_TNSR_LOW))
    return true;

  return false;
}

// SUI is the current instruction that is outside of the current packet.
// SUJ is the current instruction inside the current packet against which that
// SUI will be packetized.
bool TPCPacketizerList::isLegalToPacketizeTogether(SUnit *SUI, SUnit *SUJ) {
  assert(SUI->getInstr() && SUJ->getInstr());
  MachineInstr &I = *SUI->getInstr();
  MachineInstr &J = *SUJ->getInstr();

  if (TPCSinglePackets) {
    return false;
  }

  LLVM_DEBUG(dbgs() << "Trying " << I << "\n");

  if (I.getOpcode() == TPC::NOPv) {
    LLVM_DEBUG(dbgs() << "Failed: NOP" << "\n");
    return false;
  }
  if (J.getOpcode() == TPC::NOPv) {
    LLVM_DEBUG(dbgs() << "Failed: NOP" << "\n");
    return false;
  }

  // TODO: We currently do not allow JMPR to bundle with any instr:
  // this is because it breaks kernel tests.
  if (I.isTerminator()) {
    LLVM_DEBUG(dbgs() << "Failed: Terminator" << "\n");
    return false;
  }

  //MachineBasicBlock::iterator II = I.getIterator();

  // Solo instructions cannot go in the packet.
  assert(!isSoloInstruction(I) && "Unexpected solo instr!");

  if (SUI == SUJ) {
    LLVM_DEBUG(dbgs() << "Failed because the slot is already occupied by" << J << "\n");
    return false;
  }

  Dependence = hasDeadDependence(I, J) || hasControlDependence(I, J);
  if (Dependence) {
    LLVM_DEBUG(dbgs() << "Failed due to dead dependency with " << J << "\n");
    return false;
  }

  Dependence = hasTPCSpecificDependence(I, J);
  if (Dependence) {
    LLVM_DEBUG(dbgs() << "Failed due to TPC dependency with " << J << "\n");
    return false;
  }

  if (SUJ->isSucc(SUI)) {
    for (unsigned i = 0, e = SUJ->Succs.size(); i < e; ++i) {
      const SDep &Dep = SUJ->Succs[i];
      if (Dep.getSUnit() != SUI) {
        continue;
      }
      if (Dep.getKind() == SDep::Anti) {
        continue;
      }
      if (Dep.getKind() == SDep::Output) {
        // Zero latency means that operation writes to different parts of vector
        if (Dep.getLatency() != 0 &&
            I.getOperand(0).getReg() == J.getOperand(0).getReg()) {
          LLVM_DEBUG(dbgs() << "Failed due to OUT dependency with " << J << "\n");
          return false;
        }
      }
      if (Dep.getKind() == SDep::Order) {
        LLVM_DEBUG(dbgs() << "Failed due to ORDER dependency with " << J << "\n");
        return false;
      }
      if (Dep.getKind() == SDep::Data) {
        LLVM_DEBUG(dbgs() << "Failed due to DATA dependency with " << J << "\n");
        return false;
      }
    }
  }

#if 0
  // Do not packetize the instruction if it causes NOP insertion
  // before the bundle due to latency with its predecessor
  for (unsigned i = 0, e = SUI->Preds.size(); i < e; ++i) {
    const SDep &Dep = SUI->Preds[i];
    if (Dep.getSUnit() == SUJ) {
      continue;
    }
    if (Dep.getKind() == SDep::Data) {
      if ((PacketNum - Dep.getSUnit()->TopReadyCycle) < Dep.getLatency()) {
        LLVM_DEBUG(dbgs() << "Failed due to Latency: " << I << "\n");
        LLVM_DEBUG(dbgs() << "Dep: " << *(Dep.getSUnit()->getInstr()));
        LLVM_DEBUG(dbgs() << "   DepLatency: " << Dep.getLatency() << "\n");
        LLVM_DEBUG(dbgs() << "   DepCycle  : "   << Dep.getSUnit()->TopReadyCycle << "\n");
        LLVM_DEBUG(dbgs() << "   CurCycle  : "   << PacketNum << "\n");
        return false;
      }
    }
  }
#endif

  return true;
}

bool TPCPacketizerList::isLegalToPruneDependencies(SUnit *SUI, SUnit *SUJ) {
  assert(SUI->getInstr() && SUJ->getInstr());
  return false;
}

MachineBasicBlock::iterator
TPCPacketizerList::addToPacket(MachineInstr &MI) {
  MachineBasicBlock::iterator MII = MI.getIterator();
  assert(ResourceTracker->canReserveResources(MI));
  ResourceTracker->reserveResources(MI);
  CurrentPacketMIs.push_back(&MI);

  // Save current packet num (cycle) into the SU
  SUnit *SU = MIToSUnit[&MI];
  if (SU) {
    SU->TopReadyCycle = PacketNum;
  }
  return MII;
}

void TPCPacketizerList::addNops(MachineBasicBlock *MBB, MachineBasicBlock::iterator MI) {
  assert(!CurrentPacketMIs.empty());

  bool LD_Slot = false,
       SPU_Slot = false,
       VPU_Slot = false,
       ST_Slot = false;
  const DebugLoc &DL = CurrentPacketMIs.front()->getDebugLoc();

  for (auto I : CurrentPacketMIs) {
    const MCInstrDesc& MCI = I->getDesc();

    if (TPCII::getSlotOpCode(MCI) == TPCII::spuHALT) {
      assert(TPCII::isSPUInst(MCI));
      assert(!LD_Slot && !SPU_Slot && !VPU_Slot && !ST_Slot);
      assert(CurrentPacketMIs.size() == 1);
      auto NMI = MF.CreateMachineInstr(HII->get(TPC::HALTv), DL, true);
      MBB->insert(MI, NMI);
      addToPacket(*NMI);
      NMI = MF.CreateMachineInstr(HII->get(TPC::NOPld), DL, true);
      MBB->insert(MI, NMI);
      addToPacket(*NMI);
      NMI = MF.CreateMachineInstr(HII->get(TPC::NOPst), DL, true);
      MBB->insert(MI, NMI);
      addToPacket(*NMI);
      return;
    }

    if (TPCII::isLoopInst(MCI)) {
      assert(!LD_Slot && !SPU_Slot && !VPU_Slot && !ST_Slot);
      assert(CurrentPacketMIs.size() == 1);
      return;
    }

    if (TPCII::isLoadInst(MCI)) {
      assert(!LD_Slot && "Invalid packet");
      LD_Slot = true;
    }
    else if (TPCII::isSPUInst(MCI)) {
      assert(!SPU_Slot && "Invalid packet");
      SPU_Slot = true;
    }
    else if (TPCII::isVPUInst(MCI)) {
      assert(!VPU_Slot && "Invalid packet");
      VPU_Slot = true;
    }
    else if (TPCII::isStoreInst(MCI)) {
      assert(!ST_Slot && "Invalid packet");
      ST_Slot = true;
    }
  }

  if (LD_Slot && SPU_Slot && VPU_Slot && ST_Slot) {
    assert(CurrentPacketMIs.size() == 4);
    return;
  }

  if (!LD_Slot) {
      auto NMI = MF.CreateMachineInstr(HII->get(TPC::NOPld), DL, true);
      MBB->insert(MI, NMI);
      addToPacket(*NMI);
  }
  if (!SPU_Slot) {
      auto NMI = MF.CreateMachineInstr(HII->get(TPC::NOPs), DL, true);
      MBB->insert(MI, NMI);
      addToPacket(*NMI);
  }
  if (!VPU_Slot) {
      auto NMI = MF.CreateMachineInstr(HII->get(TPC::NOPv), DL, true);
      MBB->insert(MI, NMI);
      addToPacket(*NMI);
  }
  if (!ST_Slot) {
      auto NMI = MF.CreateMachineInstr(HII->get(TPC::NOPst), DL, true);
      MBB->insert(MI, NMI);
      addToPacket(*NMI);
  }
  assert(CurrentPacketMIs.size() <= 4);
}

void TPCPacketizerList::endPacket(MachineBasicBlock *MBB,
                                  MachineBasicBlock::iterator MI) {
  if (!CurrentPacketMIs.empty())
    addNops(MBB, MI);
  VLIWPacketizerList::endPacket(MBB, MI);
  LLVM_DEBUG(dbgs() << "** Packet Num: "   << PacketNum << "\n\n");
  PacketNum++;
}

int TPCPacketizerList::produceLatencyHazard(const MachineInstr &I) {
  SUnit *SUI = MIToSUnit[const_cast<MachineInstr *>(&I)];
  for (unsigned i = 0, e = SUI->Preds.size(); i < e; ++i) {
    const SDep &Dep = SUI->Preds[i];
    if (Dep.getKind() == SDep::Data) {
      int nopsNeeded = Dep.getLatency() - (PacketNum - Dep.getSUnit()->TopReadyCycle);
      if (nopsNeeded > 0) {
        LLVM_DEBUG(dbgs() << "Latency Hazard: " << I << "\n");
        LLVM_DEBUG(dbgs() << "Dep: " << *(Dep.getSUnit()->getInstr()));
        LLVM_DEBUG(dbgs() << "   DepLatency: " << Dep.getLatency() << "\n");
        LLVM_DEBUG(dbgs() << "   DepCycle  : "   << Dep.getSUnit()->TopReadyCycle << "\n");
        LLVM_DEBUG(dbgs() << "   CurCycle  : "   << PacketNum << "\n");
        return nopsNeeded;
      }
    }
  }
  return 0;
}

bool TPCPacketizerList::shouldAddToPacket(const MachineInstr &MI) {
  //return true;
  int curProduceNops = produceLatencyHazard(MI);
  if(CurrentPacketMIs.empty()) {
    return (curProduceNops == 0);
  }
  if(curProduceNops) {
    int nopsAlreadyProduced = 0;
    for (auto I : CurrentPacketMIs) {
      nopsAlreadyProduced = std::max(nopsAlreadyProduced, produceLatencyHazard(*I));
    }
    return (curProduceNops <= nopsAlreadyProduced);
  }
  return true;


  return !produceLatencyHazard(MI);
}


//===----------------------------------------------------------------------===//
//                         Public Constructor Functions
//===----------------------------------------------------------------------===//

FunctionPass *llvm::createTPCPacketizer() {
  return new TPCPacketizer();
}
