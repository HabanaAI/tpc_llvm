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

  BitVector DeadDefs(TPC::NUM_TARGET_REGS);
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
  const TPCSubtarget &Subtarget = MF.getSubtarget<TPCSubtarget>();
  
  // Immediate sharing
  bool hasImmI = (HII->instHasImm(I));
  bool hasImmJ = (HII->instHasImm(J));
  bool hasImmField = (HII->instHasImmField(I) || HII->instHasImmField(J));
  if (hasImmI && hasImmJ && !hasImmField) {
    uint64_t immI = HII->getInstImm(I);
    uint64_t immJ = HII->getInstImm(J);
    if (immI != immJ) {
      bool immHasSpecialEncoding = Subtarget.hasShortImm() &&
        (!useImmSlotForImm(I, immI) || !useImmSlotForImm(J, immJ));
      if (!immHasSpecialEncoding) {
        LLVM_DEBUG(dbgs() << "Imm field dependency between " << I << " and " << J << "\n");
        return true;
      }
    }
  }

  // LD/ST predicate sharing
  // Doron1 has separate fields in the VLIW for LD and ST predicates,
  // so it is allowed to schedule predicated LD and ST in the same VLIW.
  if (!Subtarget.hasDoron1ISA()) {
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
  }

  // THREAD_SYNC must be scheduled alone in a bundle [GAUDI-2468]
  if (I.getOpcode() == TPC::THREAD_SYNC || J.getOpcode() == TPC::THREAD_SYNC) {
    LLVM_DEBUG(dbgs() << "THREAD_SYNC must be alone (" << I << " and " << J << ")\n");
    return true;
  }

  // 1.3.4. General Restrictions
  // CACHE FLUSH/INVALIDATE or ASO with Evict and LD_G cannot be scheduled in the same VLIW instruction
  //
  // For Gaudi2, this restriction looks as follows:
  //   CACHE FLUSH and LD_G/PREFETCH cannot be scheduled in the same VLIW instruction.
  //   CACHE INVALIDATE.D/CACHE INVALIDATE.RST_D_PREF and LD_G/PREFETCH cannot be
  //   scheduled in the same VLIW instruction.
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
    if (Subtarget.hasGaudi2ISA()) {
      if (TPCII::isLoadInst(I.getDesc()) && TPCII::getSlotOpCode(I.getDesc()) == TPCII::PREFETCH) {
        restrict1 = true;
      }
      else if (TPCII::isLoadInst(J.getDesc()) && TPCII::getSlotOpCode(J.getDesc()) == TPCII::PREFETCH) {
        r_restrict1 = true;
      }
    }
    if (restrict1) {
      switch (J.getOpcode()) {
        case TPC::CACHE_FLUSH:
        case TPC::CACHE_INVALIDATE:
          // For Gaudi2 CACHE_INVALIDATE, we should have checked the 'switch' value
          // to be either D or RST_D_PREF. However, there are some more restrictions
          // for CACHE_INVALIDATE with other switches, so let's restrict the whole instruction.
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
          // For Gaudi2 CACHE_INVALIDATE, we should have checked the 'switch' value
          // to be either D or RST_D_PREF. However, there are some more restrictions
          // for CACHE_INVALIDATE with other switches, so let's restrict the whole instruction.
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

  // It is not allowed to schedule a CACHE_INVALIDATE.LU/RST_LU together
  // with LOOKUP* in the same VLIW.
  if (Subtarget.hasGaudi2ISA() &&
      ((TPCII::isStoreInst(I.getDesc()) && sopcI == TPCII::CACHE_INVALIDATE &&
        (I.getOperand(0).getImm() & TPCII::SW_LU ||
         I.getOperand(0).getImm() & TPCII::SW_RST_LU) &&
        TPCII::isLookup(J.getDesc()))
       ||
       (TPCII::isStoreInst(J.getDesc()) && sopcJ == TPCII::CACHE_INVALIDATE &&
        (J.getOperand(0).getImm() & TPCII::SW_LU ||
         J.getOperand(0).getImm() & TPCII::SW_RST_LU) &&
        TPCII::isLookup(I.getDesc())))) {
    LLVM_DEBUG(dbgs() << "CACHE_INVALIDATE.LU/RST_LU and LOOKUP* "
                         "dependency between " << I << " and " << J << "\n");
    return true;
  }

  //It is not allowed to schedule a ST_TNSR/ST_TNSR_SQZ/ST_TNSR_S with RMW
  //in the same VLIW with the following VPU FP8_143 operations:
  //  MAC/MUL/ADD/SUB/MADD
  //  NEARBYINT
  //  CONVERT TO/FROM FP8_143
  //  EXTRACT_EXP
  //  FORM_FP_NUMBER
  if (Subtarget.hasGaudi2ISA()) {
    auto IsVpuWithFp8_143 = [](const MachineInstr &Inst) -> bool {
      const MCInstrDesc &InstDesc = Inst.getDesc();

      if (TPCII::isVPUInst(InstDesc)) {
        unsigned SlotOp = TPCII::getSlotOpCode(InstDesc);

        switch (SlotOp) {
        case TPCII::vpuMAC:
        case TPCII::vpuMUL:
        case TPCII::vpuADD:
        case TPCII::vpuSUB:
        case TPCII::vpuMADD:
        case TPCII::vpuNEARBYINT:
        case TPCII::vpuEXTRACT_EXP:
        case TPCII::vpuFORM_FP_NUM:
          return getOpType(Inst) == TPCII::OpType::FP8_143;
        case TPCII::vpuCONVERT:
          return getOpType(Inst) == TPCII::OpType::FP8_143 ||
                 getSwitches(Inst) == TPCII::SW_TO_FP8_143;
        default:
          return false;
        }
      } else
        return false;
    };

    auto IsStTnsrWithRMW = [](const MachineInstr &Inst) -> bool {
      const MCInstrDesc &InstDesc = Inst.getDesc();
      unsigned SlotOp = TPCII::getSlotOpCode(InstDesc);
      if (TPCII::isStoreInst(InstDesc) &&
          (SlotOp == TPCII::ST_TNSR ||
           SlotOp == TPCII::ST_TNSR_HIGH ||
           SlotOp == TPCII::ST_TNSR_LOW ||
           SlotOp == TPCII::ST_TNSR_S ||
           SlotOp == TPCII::ST_TNSR_SQZ) &&
          (getSwitches(Inst) & TPCII::SW_RMW_SEL))
        return true;
      else
        return false;
    };

    if ((IsStTnsrWithRMW(I) && IsVpuWithFp8_143(J)) ||
        (IsStTnsrWithRMW(J) && IsVpuWithFp8_143(I))) {
      LLVM_DEBUG(dbgs() << "ST_TNSR/*ST_TNSR_SQZ/ST_TNSR_S* with RMW and some"
                           " VPU FP8_143 instructions dependency between "
                 << I << " and " << J << "\n");
      return true;
    }
  }

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
  if (!Subtarget.hasGoyaISA()) {
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
  //   - LD_L_V* (Dali/GaudiB)
  //   - VPU instruction
  //   - EVENT with SLOT=VPU (Gaudi2+)
  //   - LD_L_V with ADDR_CALC (Doron1+)
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
  // In Gaudi/Goya2 this restriction can be mitigated, because we added a separate field
  // (LD_VLM_ADDR) for LD_L_V.
  //
  // Therefore in Gaudi/Goya2 the restriction holds only for MOV S->V and VPU using SRF.
  {
    const TPCInstrInfo * TII = Subtarget.getInstrInfo();
    
    const auto &SRFOrSPRFRestriction = [&TII, &Subtarget]
        (const MachineInstr &Inst) {
      const MCInstrDesc &Desc = Inst.getDesc();
      unsigned Opcode = TPCII::getSlotOpCode(Desc);
      
      if (TPCII::isLoadInst(Desc) && Opcode == TPCII::ldMOV &&
          TII->isScalarToVector(Inst))
        return true;
      else if (TPCII::isVPUInst(Desc) && TII->hasSRFOrSPRFOperands(Inst))
        return true;
      else if ((Subtarget.hasGoyaISA() || Subtarget.hasGaudiBISA()) &&
               TPCII::isLoadInst(Inst.getDesc()) &&
               (Opcode == TPCII::LD_L_V || Opcode == TPCII::LD_L_V_LOW ||
                Opcode == TPCII::LD_L_V_HIGH) &&
               TII->hasSRFOrSPRFOperands(Inst))
        return true;
      else if (Subtarget.hasGen4Plus() && TPCII::isEvent(Desc) &&
               Inst.getOperand(1).getImm() == 1)
        return true;
      else if (Subtarget.hasDoron1() && TPCII::isLoadInst(Inst.getDesc()) &&
               Opcode == TPCII::LD_L_V &&
               (getSwitches(Inst) & TPCII::SW_ADDR_CALC))
        return true;
      else
        return false;
    };
    
    if ((SRFOrSPRFRestriction(I) && TII->hasSRFOrSPRFOperands(J)) ||
        (SRFOrSPRFRestriction(J) && TII->hasSRFOrSPRFOperands(I))) {
      LLVM_DEBUG(dbgs() << "SRF/SPRF dependency between " << I <<
                 " and " << J << "\n");
      return true;
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

  if (Subtarget.hasGaudi2ISA()) {
    // When LD_TNSR/LD_TNSR_LOW/LD_TNSR_HIGH is issued on STORE slot,
    // then LOAD slot in the same VLIW is allowed to use only scalar LD_G or NOP opcodes.
    bool isScalarLD_G = false;
    if ((TPCII::isLoadInst(J.getDesc()) && (sopcJ == TPCII::LD_G))) {
        assert(J.getOperand(0).isReg());
        unsigned reg = J.getOperand(0).getReg();
        if (TPC::SRFRegClass.contains(reg)) {
          isScalarLD_G = true;
        }
    }
    if ((TPCII::isStoreInst(I.getDesc()) && (sopcI >= 17 && sopcI <= 19)) && // LD_TNSR*
        !isScalarLD_G) {
      return true;
    }
    if ((TPCII::isLoadInst(I.getDesc()) && (sopcI == TPCII::LD_G))) {
        assert(I.getOperand(0).isReg());
        unsigned reg = I.getOperand(0).getReg();
        if (TPC::SRFRegClass.contains(reg)) {
          isScalarLD_G = true;
      }
    }
    if ((TPCII::isStoreInst(J.getDesc()) && (sopcJ >= 17 && sopcJ <= 19)) && // LD_TNSR*
        !isScalarLD_G) {
      return true;
    }
    // It is not allowed to schedule an ASO together with:
    //    LD_TNSR*
    //      OR
    //    LD_G with VRF destination
    //      OR
    //    LOOKUP* (PRM 0.59)
    // in the same VLIW
    if (I.getOpcode() == TPC::ASO) {
      if ((TPCII::isStoreInst(J.getDesc()) || TPCII::isLoadInst(J.getDesc())) &&
          (sopcJ >= 17 && sopcJ <= 19)) { // LD_TNSR*
        return true;
      }
      if (TPCII::isLoadInst(J.getDesc()) && (sopcJ == TPCII::LD_G)) {
        assert(J.getOperand(0).isReg());
        unsigned reg = J.getOperand(0).getReg();
        if (TPC::VRFRegClass.contains(reg)) {
          return true;
        }
      }
      if (TPCII::isLookup(J.getDesc())) {
        return true;
      }
    }
    if (J.getOpcode() == TPC::ASO) {
      if ((TPCII::isStoreInst(I.getDesc()) || TPCII::isLoadInst(I.getDesc())) &&
          (sopcI >= 17 && sopcI <= 19)) { // LD_TNSR*
        return true;
      }
      if (TPCII::isLoadInst(I.getDesc()) && (sopcI == TPCII::LD_G)) {
        assert(I.getOperand(0).isReg());
        unsigned reg = I.getOperand(0).getReg();
        if (TPC::VRFRegClass.contains(reg)) {
          return true;
        }
      }
      if (TPCII::isLookup(I.getDesc())) {
        return true;
      }
    }

    // It is not allowed to schedule anything to store slot if VPU use store_src_c
    if (TPCII::getSrcCIsStoreSrcC(I.getDesc()) && TPCII::isStoreInst(J.getDesc()))
      return true;
    else if (TPCII::getSrcCIsStoreSrcC(J.getDesc()) && TPCII::isStoreInst(I.getDesc()))
      return true;
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
       sopcJ == TPCII::ST_TNSR_S || sopcJ == TPCII::ST_TNSR_SQZ ||
       sopcJ == TPCII::stLD_TNSR || sopcJ == TPCII::stLD_TNSR_HIGH ||
       sopcJ == TPCII::stLD_TNSR_LOW))
    return true;

  if (JIsMulWithIRF && TPCII::isStoreInst(I.getDesc()) &&
      (sopcI == TPCII::stSET_INDX || sopcI == TPCII::stPRMT_INDX ||
       sopcI == TPCII::stGEN_ADDR || sopcI == TPCII::ST_TNSR ||
       sopcI == TPCII::ST_TNSR_HIGH || sopcI == TPCII::ST_TNSR_LOW ||
       sopcI == TPCII::ST_TNSR_S || sopcI == TPCII::ST_TNSR_SQZ ||
       sopcI == TPCII::stLD_TNSR || sopcI == TPCII::stLD_TNSR_HIGH ||
       sopcI == TPCII::stLD_TNSR_LOW))
    return true;

  //It is not allowed to encode in the same VLIW the following combination:
  //  - MADD with SRF on SRC_C on VPU slot
  //  - ST_TNSR_S with IMM on STORE slot
  // Gaudi2
  const auto &VPUMADDWithSRFOnSrcC = [](const MachineInstr &Inst) {
    if (!TPCII::isVPUInst(Inst.getDesc()))
      return false;
    if (TPCII::getSlotOpCode(Inst.getDesc()) != TPCII::vpuMADD)
      return false;

    // SrcC
    const MachineOperand &SrcC = Inst.getOperand(3);
    if (!SrcC.isReg())
      return false;
    if (TPC::SRFRegClass.contains(SrcC.getReg()))
      return true;

    return false;
  };

  const auto &StSTTnsrSWithImm = [](const MachineInstr &Inst){
    if (!TPCII::isStoreInst(Inst.getDesc()))
      return false;
    if (TPCII::getSlotOpCode(Inst.getDesc()) != TPCII::ST_TNSR_S)
      return false;

    if(Inst.getOperand(2).isImm())
      return true;

    return false;
  };

  if (Subtarget.hasGaudi2ISA() &&
      ((VPUMADDWithSRFOnSrcC(I) && StSTTnsrSWithImm(J)) ||
       (VPUMADDWithSRFOnSrcC(J) && StSTTnsrSWithImm(I))))
    return true;
  

  if (Subtarget.hasDoron1ISA()) {
    // If compiler scheduled FCLASS on LOAD slot, it is not allowed to schedule
    // FCLASS/CALC_FP_SPECIAL/ FORM_FP_NUMBER/ EXTRACT_EXP on VPU slot in the
    // same VLIW.
    // If compiler scheduled CALC_FP_SPECIAL on LOAD slot, it is not allowed
    // to schedule FCLASS/CALC_FP_SPECIAL/ FORM_FP_NUMBER/ EXTRACT_EXP on VPU
    // slot in the same VLIW.
    const auto &IsLdVpuRestriction = [](const MachineInstr &LoadMI,
                                        const MachineInstr &VpuMI) {
      unsigned LoadSlotOpcode = TPCII::getSlotOpCode(LoadMI.getDesc());
      unsigned VpuSlotOpcode = TPCII::getSlotOpCode(VpuMI.getDesc());
      if (TPCII::isLoadInst(LoadMI.getDesc()) &&
          TPCII::isVPUInst(VpuMI.getDesc()) &&
          (LoadSlotOpcode == TPCII::LD_FCLASS ||
           LoadSlotOpcode ==TPCII::LD_CALC_FP_SPECIAL) &&
          (VpuSlotOpcode == TPCII::vpuFCLASS ||
           VpuSlotOpcode == TPCII::vpuCALC_FP_SPECIAL ||
           VpuSlotOpcode == TPCII::vpuFORM_FP_NUM ||
           VpuSlotOpcode == TPCII::vpuEXTRACT_EXP)) {
        return true;
      }
      
      return false;
    };
    
    // AUTO_INC_DIM switch can be set either on LOAD slot OR on STORE slot,
    // but not on both slots in the same VLIW.
    // Don't check slots.
    // When setting AUTO_INC_DIM, the SPU slot cannot contain an instruction
    // with an IRF destination
    const MachineRegisterInfo &MRI = MF.getRegInfo();
    const auto &IsAutoIncRestriction = [&Subtarget, &MRI]
        (const MachineInstr &AutoIncMI, const MachineInstr &OtherMI) {
      
      if ((TPCII::isLdTnsr(AutoIncMI.getDesc(), Subtarget.hasDoron1()) ||
           TPCII::isStTnsr(AutoIncMI.getDesc())) &&
          (getSwitches(AutoIncMI) & TPCII::SW_AUTO_INC_DIM)) {
        if ((TPCII::isLdTnsr(OtherMI.getDesc(), Subtarget.hasDoron1()) ||
             TPCII::isStTnsr(OtherMI.getDesc())) &&
            (getSwitches(OtherMI) & TPCII::SW_AUTO_INC_DIM))
          return true;
        
        if (TPCII::isSPUInst(OtherMI.getDesc())) {
          for (unsigned I = 0; I < OtherMI.getNumOperands(); ++I) {
            const MachineOperand &MO = OtherMI.getOperand(I);
            if (MO.isDef() &&
                MRI.getRegClass(MO.getReg()) == &TPC::IRFRegClass)
              return true;
          }
        }
      }
      
      return false;
    };
    
    if (IsLdVpuRestriction(I, J) || IsLdVpuRestriction(J, I))
      return true;
    
    if (IsAutoIncRestriction(I, J) || IsAutoIncRestriction(J, I))
      return true;
    
  }

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
        // Two instructions that are anti-dependent can share a bundle,
        // since in most such cases all operands are read before any
        // modifications take place.
        // There are certain anti-dependencies that cannot be ignored.
        // Specifically, starting from Gaudi2, there may be zero latency
        // between two dependent instructios, such as VPRF producer and
        // store that VPRF via ST_L_V. Threfore, the following two
        // anti-dependent instructions can't be put into the same bundle:
        // 
        //   ST_L_V %VP1 ...   ; SUJ
        //   %VP1 = MOV ...    ; SUI
        // 
        const InstrItineraryData * ItinData = MF.getSubtarget<TPCSubtarget>().getInstrItineraryData();
        for (unsigned i = 0; i < I.getNumOperands(); ++i) {
          MachineOperand Op = I.getOperand(i);
          if (Op.isReg() && Op.isDef()) {
            Register R = Op.getReg();
            int idx = J.findRegisterUseOperandIdx(R, false, HRI);
            if (idx != -1) {
              int lat = TII->getOperandLatency(ItinData, I, i, J, idx);
              if (lat == 0) {
                LLVM_DEBUG(dbgs() << "Failed due to ANTI dependency (zero latency) with " << J << "\n");
                return false;
              }
            }
          }
        }
        // Skip over remaining anti-dependences.
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
