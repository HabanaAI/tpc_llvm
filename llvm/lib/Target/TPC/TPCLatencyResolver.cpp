//===- TPCLatencyResolver.cpp ---- Latency Resolver for TPC ---------------===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//
//
//===----------------------------------------------------------------------===//

#include "TPCInstrInfo.h"
#include "TPCSubtarget.h"
#include "MCTargetDesc/TPCMCTargetDesc.h"
#include "TPCMachineScheduler.h"
#include "llvm/InitializePasses.h"
#include "llvm/CodeGen/MachineLoopInfo.h"
#include "llvm/CodeGen/MachineScheduler.h"
#include "llvm/CodeGen/ScheduleDAGMutation.h"
#include "llvm/CodeGen/MachineRegisterInfo.h"
#include "llvm/CodeGen/MachineInstr.h"
#include "llvm/CodeGen/MachineInstrBuilder.h"
#include "llvm/CodeGen/TargetRegisterInfo.h"
#include "llvm/Support/Debug.h"

using namespace llvm;

#define DEBUG_TYPE "tpclatresolver"

#define TPC_MAX_LATENCY 7

static cl::opt<int> TPCNumDebugNops("tpc-debug-nops", cl::Hidden,
  cl::ZeroOrMore, cl::init(0),
  cl::desc("TPC debug mode (insert unconditional NOPs before every instr)"));

static cl::opt<bool> DisableDelaySlotUtilization("tpc-disable-delay-slot-utilization", cl::Hidden,
  cl::ZeroOrMore, cl::init(false),
  cl::desc("Disable loop delay slot utilization"));

// LLVM-425 ld_tnsr cannot be scheduled at 4 cycle after st_tnsr
static cl::opt<bool> EnableLdTnsrWorkaround("tpc-ld-tnsr-workaround", cl::Hidden,
              cl::init(true),
              cl::desc("Ensure ld_tnsr is not scheduled at 4 cycle after st_tnsr"));

namespace llvm {
  FunctionPass *createTPCLatencyResolver();
  void initializeTPCLatencyResolverPass(PassRegistry&);
}

namespace {

class TPCLatencyResolver : public MachineFunctionPass {

public:
  static char ID;
  TPCLatencyResolver() : MachineFunctionPass(ID) {
    MLI = nullptr;
    MF = nullptr;
    ItinData = nullptr;
    TII = nullptr;
    TRI = nullptr;
  };
  void getAnalysisUsage(AnalysisUsage &AU) const override;
  bool runOnMachineFunction(MachineFunction &Fn) override;

private:
  MachineLoopInfo          * MLI;
  MachineFunction          * MF;
  const InstrItineraryData * ItinData;
  const TargetInstrInfo    * TII;
  const TargetRegisterInfo * TRI;

  int  getRequiredDistance(const MachineInstr& DefMI, const MachineInstr& UseMI ,int curDistance);
  int  getBBCycles(MachineBasicBlock* MBB, MachineBasicBlock* Succ, bool ignore_loop_instr);
  void insertDebugNops(MachineBasicBlock& MBB, int numPreNops);
  void fixSmallLoops(MachineBasicBlock& MBB);
  void fillLoopDelaySlot(MachineBasicBlock& MBB);
  void ensureLoopEndAndJmpLatency(MachineBasicBlock& MBB);
  bool resolveBlockLatency(MachineBasicBlock& MBB, bool check_only = false, bool ignore_loop_instr = false);
  bool resolveFunctionLatency(MachineFunction &MF);
  bool resolveCrossBlockLatency(MachineBasicBlock& MBB,
                                bool topdown = false,
                                bool deep = false,
                                bool check_only = false,
                                bool ignore_loop_instr = false);
  bool resolveCrossBlockLatency(MachineBasicBlock& MBB,
                                MachineBasicBlock* PredMBB,
                                bool topdown,
                                bool deep,
                                int distance,
                                bool check_only,
                                bool ignore_loop_instr,
				std::vector<const MachineBasicBlock*> ProcessedBB);
  int getRequiredBBDistance(MachineBasicBlock* PBB,
				MachineBasicBlock* SBB,
				bool ignore_loop_instr);

  bool isLoopTaken(const MachineInstr *MI);
};

}

char TPCLatencyResolver::ID = 0;

INITIALIZE_PASS_BEGIN(TPCLatencyResolver, "tpclatresolver", "TPC Latency Resolver", false, false)
  INITIALIZE_PASS_DEPENDENCY(MachineDominatorTree)
  INITIALIZE_PASS_DEPENDENCY(MachineBranchProbabilityInfo)
  INITIALIZE_PASS_DEPENDENCY(MachineLoopInfo)
  INITIALIZE_PASS_DEPENDENCY(MachineScheduler)
INITIALIZE_PASS_END(TPCLatencyResolver, "tpclatresolver", "TPC Latency Resolver", false, false)


namespace llvm {
FunctionPass* createTPCLatencyResolver() {
  return new TPCLatencyResolver();
}
}

//
// isHaltInstr: returns true if MI is a HALT instruction
//
static bool isHaltInstr(const MachineInstr* MI) {
    if (MI->isBundle()) {
      const MachineBasicBlock* MBB = MI->getParent();
      MachineBasicBlock::const_instr_iterator MII = MI->getIterator();
      for (++MII; MII != MBB->instr_end() && MII->isInsideBundle(); ++MII) {
        const MachineInstr& BMI = *MII;
	if (isHaltInstr(&BMI))
	  return true;
      }
    }
    else {
	if ((MI->getOpcode() == TPC::HALTs) || (MI->getOpcode() == TPC::HALTv)) {
	  return true;
	}
    }
    return false;
}

//
// isNopInstr: returns true if MI is a full NOP instruction
//
static bool isNopInstr(const MachineInstr *MI) {
  if (MI->isBundle()) {
    const MachineBasicBlock* MBB = MI->getParent();
    MachineBasicBlock::const_instr_iterator MII = MI->getIterator();
    for (++MII; MII != MBB->instr_end() && MII->isInsideBundle(); ++MII) {
      const MachineInstr& BMI = *MII;
      if (!isNopInstr(&BMI)) {
        return false;
      }
    }
    return true;
  }
  else {
    if (MI->getOpcode() == TPC::NOPv  ||
        MI->getOpcode() == TPC::NOPs  ||
        MI->getOpcode() == TPC::NOPld ||
        MI->getOpcode() == TPC::NOPst) {
      return true;
    }
  }
  return false;
}

//
// isJmpInstr: returns true if MI is a JMP instruction, or if it is a bundle instruction.
// containing JMP. 
//
static bool isJmpInstr(const MachineInstr *MI) {
  if (MI->isBundle()) {
    const MachineBasicBlock* MBB1 = MI->getParent();
    MachineBasicBlock::const_instr_iterator MII = MI->getIterator();
    for (++MII; MII != MBB1->instr_end() && MII->isInsideBundle(); ++MII) {
      const MachineInstr& BMI = *MII;
      if (isJmpInstr(&BMI))
        return true;
    }
    return false;
  }

  return MI->getOpcode() == TPC::JMPR || MI->getOpcode() == TPC::JMPR_u;
}

//
// isStartLoopBlock: returns true MBB is a block that starts HW loop. 
//
static bool isStartLoopBlock(MachineBasicBlock& MBB) {
  MachineInstr& MI = *(--(MBB.end()));
  if (TPCII::isLoopInst(MI.getDesc()) && (MI.getOpcode() != TPC::LOOPEND)) {
    return true;
  }
  return false;
}

//
// getJmpDest: returns BB, which is a destination of JMP instruction MI. 
//
static MachineBasicBlock* getJmpDest(const MachineInstr *MI) {
      if (MI->isBundle()) {
	const MachineBasicBlock* MBB1 = MI->getParent();
	MachineBasicBlock::const_instr_iterator MII = MI->getIterator();
	for (++MII; MII != MBB1->instr_end() && MII->isInsideBundle(); ++MII) {
          const MachineInstr& BMI = *MII;
          if (isJmpInstr(&BMI) && BMI.getOperand(0).isMBB()) {
            return BMI.getOperand(0).getMBB();
          }
	}
      }
      else {
        if (isJmpInstr(MI) && MI->getOperand(0).isMBB()) {
          return MI->getOperand(0).getMBB();
	}
      }
      return nullptr;
}

#ifndef NDEBUG
//
// printMI: debug print of MI instruction. If MI is a bundle prints all instructions
// in that bundle (excluding NOPs)
//
static void printMI(MachineInstr *MI) {
  if (MI->isBundle()) {
    const MachineBasicBlock* MBB = MI->getParent();
    MachineBasicBlock::const_instr_iterator MII = MI->getIterator();
    for (++MII; MII != MBB->instr_end() && MII->isInsideBundle(); ++MII) {
      const MachineInstr& DefMI = *MII;
      if (!isNopInstr(&DefMI)) {
        dbgs() << "      " << DefMI;
      }
    }
  }
  else {
    dbgs() << "    " << *MI;
  }
}
#endif

static bool isUsingSrcD(const MachineInstr *MI, const InstrItineraryData * ItinData) {
  unsigned idx = MI->getDesc().getSchedClass();

  if (idx == TPC::Sched::IIC_VectorComplexOp) {
    return true;
  }

  return false;
}

void TPCLatencyResolver::getAnalysisUsage(AnalysisUsage &AU) const {
  AU.setPreservesCFG();
  AU.addRequired<MachineLoopInfo>();
  MachineFunctionPass::getAnalysisUsage(AU);
}

bool TPCLatencyResolver::runOnMachineFunction(MachineFunction &Fn) {
  MF = &Fn;
  TII = Fn.getSubtarget().getInstrInfo();
  ItinData = Fn.getSubtarget().getInstrItineraryData();
  TRI = Fn.getSubtarget().getRegisterInfo();
  MLI = &getAnalysis<MachineLoopInfo>();
  TRI = Fn.getSubtarget().getRegisterInfo();

  LLVM_DEBUG(dbgs() << "\n\n*** TPC Latency Resolver\n\n");

  for (auto &MBB : Fn) {
    LLVM_DEBUG(dbgs() << "Processing BB#" << MBB.getNumber() << "\n");
    if (MBB.empty()) {
      continue;
    }
    if (TPCNumDebugNops) {
      insertDebugNops(MBB, TPCNumDebugNops);
    }
    else {
      resolveBlockLatency(MBB);
      fixSmallLoops(MBB);
    }
  }

  if (TPCNumDebugNops) {
    return true;
  } else
    resolveFunctionLatency(Fn);

  for (auto &MBB : Fn) {
    if (!MBB.empty()) {
      resolveCrossBlockLatency(MBB);
    }
  }

  for (auto &MBB : Fn) {
    if (!MBB.empty()) {
      resolveCrossBlockLatency(MBB, false, true);
    }
  }

  for (auto &MBB : Fn) {
    if (!MBB.empty()) {
      fillLoopDelaySlot(MBB);
    }
  }

  for (auto &MBB : Fn) {
    if (!MBB.empty()) {
      ensureLoopEndAndJmpLatency(MBB);
    }
  }
  return true;
}

static bool getLoopTakenFromMetadata(const MDNode* LoopMD) {
  // First operand should refer to the loop id itself.
  assert(LoopMD->getNumOperands() > 0 && "requires at least one operand");

  MDNode *MD = nullptr;

  for (unsigned i = 1, e = LoopMD->getNumOperands(); i < e; ++i) {
    MD = dyn_cast<MDNode>(LoopMD->getOperand(i));
    if (!MD)
      continue;

    MDString *S = dyn_cast<MDString>(MD->getOperand(0));
    if (!S)
      continue;

    if (S->getString().equals("llvm.loop.taken")) {
      assert(MD->getNumOperands() == 2 &&
             "Loop taken hint metadata should have two operands.");
      unsigned Count =
        mdconst::extract<ConstantInt>(MD->getOperand(1))->getZExtValue();
      return Count != 0;
    }
  }

  return false;
}

static bool ignoreInstr(MachineInstr *MI) {
  if (MI->isDebugValue())
    return true;
  if (MI->getOpcode() == TargetOpcode::IMPLICIT_DEF)
    return true;
  return false;
}

bool TPCLatencyResolver::isLoopTaken(const MachineInstr *MI) {
  assert(MI->getOpcode() == TPC::LOOPEND);

  if (MF->getSubtarget<TPCSubtarget>().getTargetLowering()->getTargetMachine().Options.AllLoopsTaken) {
    return true;
  }
  
  const MDNode* LoopMD = nullptr;

  if(MI->getOperand(MI->getNumOperands() - 5).isMetadata()) {
    LoopMD = MI->getOperand(MI->getNumOperands() - 5).getMetadata();
  }

  // Get info from pragma
  if (LoopMD)
    return getLoopTakenFromMetadata(LoopMD);
  return false;
}

static bool isFloatData(TPCII::OpType X) {
  switch (X) {
  case TPCII::OpType::BF16:
  case TPCII::OpType::FP32:
    return true;
  default:
    return false;
  }
}

static bool HasLookup(const MachineInstr &MI) {
  const MachineBasicBlock &MBB = *MI.getParent();
  MachineBasicBlock::const_instr_iterator MII = MI.getIterator();
  for (++MII; MII != MBB.instr_end() && MII->isInsideBundle(); ++MII) {
    const MachineInstr &BMI = *MII;
    const MCInstrDesc &BMID = BMI.getDesc();

    if (TPCII::isLoadInst(BMID) && TPCII::isLookup(BMID))
      return true;
  }

  return false;
}

// TODO: When a loop instruction will implicit-def only one iterator
// register, use Reg argument.
static bool HasLoopIter(const MachineInstr& MI/*, Register Reg*/) {
  assert(!MI.isBundle());
  for (unsigned i = 0; i < MI.getNumOperands(); ++i) {
    const MachineOperand &MO = MI.getOperand(i);
    if (!MO.isReg())
      continue;

    Register Reg = MO.getReg();
    if (Reg == TPC::S32 || Reg == TPC::S33 || Reg == TPC::S34 ||
        Reg == TPC::S35)
      return true;
  }

  return false;
}

static MachineBasicBlock * GetPrevNonEmptyMBB(const MachineBasicBlock *MBB) {
  assert(MBB);
  MachineBasicBlock *LayoutPred = nullptr;
  for(MachineBasicBlock *PredMBB : MBB->predecessors()) {
    if (PredMBB && PredMBB->isLayoutSuccessor(MBB))
      LayoutPred = PredMBB;
  }

  if (!LayoutPred)
    return LayoutPred;
  else if (!LayoutPred->empty())
    return LayoutPred;
  else
    return GetPrevNonEmptyMBB(LayoutPred);
}

//
// getRequiredDistance ()
// Returns the distance (in cycles) between two bundle instructions, which is required
// due to latency rules and other HW restrictions.
//
int TPCLatencyResolver::getRequiredDistance(const MachineInstr& DefMI,
                                            const MachineInstr& UseMI,
                                            int curDistance) {
  int alatency = 0;
  std::vector<const MachineInstr*> DefInstrs;
  std::vector<const MachineInstr*> UseInstrs;

  // Needed for EnableLdTnsrGaudiWorkaround
  bool defHasStTnsr = false;
  bool useHasLdTnsr = false;
  bool needLdTnsrWorkaround = false;

  if (curDistance <= 4 && EnableLdTnsrWorkaround) {
    needLdTnsrWorkaround = true;
  }

  // Collect all instructions from DefMI bundle
  //
  DefInstrs.clear();
  if (DefMI.isBundle()) {
    const MachineBasicBlock* MBB = DefMI.getParent();
    MachineBasicBlock::const_instr_iterator MII = DefMI.getIterator();
    for (++MII; MII != MBB->instr_end() && MII->isInsideBundle(); ++MII) {
      const MachineInstr& BMI = *MII;
      DefInstrs.push_back(&BMI);
    }
  }
  else {
    DefInstrs.push_back(&DefMI);
  }
  
  // Collect all instructions from UseMI bundle
  //
  UseInstrs.clear();
  if (UseMI.isBundle()) {
    const MachineBasicBlock* MBB = UseMI.getParent();
    MachineBasicBlock::const_instr_iterator MII = UseMI.getIterator();
    for (++MII; MII != MBB->instr_end() && MII->isInsideBundle(); ++MII) {
      const MachineInstr& BMI = *MII;
      UseInstrs.push_back(&BMI);
    }
  } else {
    UseInstrs.push_back(&UseMI);
  }

  // Check for latencies
  //
  for (auto DMI : DefInstrs) {
    unsigned DReg;
    unsigned DefIdx;

    for (unsigned i = 0, e = DMI->getNumOperands(); i != e; ++i) {
      const MachineOperand &MO = DMI->getOperand(i);
      if (!MO.isReg()) continue;
      if (!MO.isDef()) continue;
      DReg = MO.getReg();
      DefIdx = i;

      for (auto UMI : UseInstrs) {
        unsigned UReg;
        unsigned UseIdx;
        for (unsigned j = 0, e = UMI->getNumOperands(); j != e; ++j) {
          const MachineOperand &MOU = UMI->getOperand(j);
          if (!MOU.isReg()) continue;
          if (!MOU.isUse()) continue;
          UReg = MOU.getReg();
          UseIdx = j;
          if (UReg == DReg ||
              TRI->isSubRegister(DReg, UReg) ||
              TRI->isSubRegister(UReg, DReg)) {
            int lat = TII->getOperandLatency(ItinData, *DMI, DefIdx, *UMI, UseIdx);
            alatency = std::max(alatency, lat);
          }
        }
      }
    }
  }

  // Check for PRM restrictions
  //
  for (auto DMI : DefInstrs) {
    for (auto UMI : UseInstrs) {
      unsigned opcDef = TPCII::getSlotOpCode(DMI->getDesc());
      unsigned opcUse = TPCII::getSlotOpCode(UMI->getDesc());

      if (needLdTnsrWorkaround) {
        if (TPCII::isLoadInst(UMI->getDesc()) &&
            (opcUse == TPCII::LD_TNSR ||
             opcUse == TPCII::LD_TNSR_LOW ||
             opcUse == TPCII::LD_TNSR_HIGH)) {
          useHasLdTnsr = true;
        }
        if (TPCII::isStoreInst(DMI->getDesc()) &&
            (opcDef == TPCII::ST_TNSR ||
             opcDef == TPCII::ST_TNSR_LOW ||
             opcDef == TPCII::ST_TNSR_HIGH)) {
          defHasStTnsr = true;
        }
      }

      // One cycle After LOOKUP_C1C2, LOOKUP_C0 and LOOKUP, a VPU instruction
      // which uses SRC_D isn't allowed to scheduled.
      // TODO: check for SrcD here
      //
      if (TPCII::isVPUInst(UMI->getDesc()) && !isNopInstr(UMI)) {
        bool srcD = isUsingSrcD(UMI, ItinData);
        if (srcD && TPCII::isLookupC(DMI->getDesc())) {
          alatency = std::max(alatency, 2);
        }
      }

      // One cycles After LOOKUP_C0C2, LOOKUP_C1 and LOOKUP, the LOAD issue slot must
      // contain an NOP instruction.
      //
      if (TPCII::isLoadInst(UMI->getDesc()) && !isNopInstr(UMI)) {
        if (TPCII::isLookupC(DMI->getDesc())) {
          alatency = std::max(alatency, 2);
        }
      }

      // One cycle After LOOKUP_C1C2, LOOKUP_C0 and LOOKUP,
      // ST_L_V isn't allowed to be scheduled.
      //
      if (TPCII::isStoreInst(UMI->getDesc()) &&
          opcUse == TPCII::ST_L_V) {
        if (TPCII::isLookupC(DMI->getDesc())) {
          alatency = std::max(alatency, 2);
        }
      }

      // After LOOKUP_1C, LOOKUP_2C and LOOKUP,
      // the STORE issue slot must not contain a LD_TNSR* instruction.
      //
      if (TPCII::isStoreInst(UMI->getDesc()) &&
          (opcUse == TPCII::stLD_TNSR ||
           opcUse == TPCII::stLD_TNSR_LOW ||
           opcUse == TPCII::stLD_TNSR_HIGH)) {
        if (TPCII::isLookup(DMI->getDesc())) {
          alatency = std::max(alatency, 2);
        }
      }


      // At least 1 instruction should exist between CACHE_INVALIDATE and LOOKUP*.
      //
      if (TPCII::isStoreInst(UMI->getDesc()) &&
          opcUse == TPCII::CACHE_INVALIDATE) {
        if (TPCII::isLookupC(DMI->getDesc())) {
          alatency = std::max(alatency, 2);
        }
      }
      if (TPCII::isStoreInst(DMI->getDesc()) &&
          opcDef == TPCII::CACHE_INVALIDATE) {
        if (TPCII::isLookupC(UMI->getDesc())) {
          alatency = std::max(alatency, 2);
        }
      }

      // 1 Insturction after JMP*, a NOP must exists in all issue slots, besides VPU issue slot.
      //
      if (!TPCII::isVPUInst(UMI->getDesc()) && !isNopInstr(UMI)) {
        if (isJmpInstr(DMI)) {
          MachineBasicBlock * DestMBB = getJmpDest(DMI);
          const MachineBasicBlock * UseMBB = UMI->getParent();
          const MachineBasicBlock * DefMBB = DMI->getParent();
          if ((DestMBB->getNumber() == UseMBB->getNumber()) ||
              ((UseMBB->getNumber() != DefMBB->getNumber()+1) &&
               (UseMBB->getNumber() != DefMBB->getNumber()))) {
            // JMP taken
            alatency = std::max(alatency, 0);
          }
          else {
            // JMP not taken
            alatency = std::max(alatency, 2);
          }
        }
      }

      // 2 Instruction after JMP* or LOOP delay slot, a CONVERT* on SPU issue slot is not allowed to be issued.
      //
      if (TPCII::isSPUInst(UMI->getDesc()) &&
          (opcUse == 22 || /* TPCII::CONVERT        */
           opcUse == 23 || /* TPCII::CONVERT_INT32  */
           opcUse == 24 || /* TPCII::CONVERT_UINT32 */
           opcUse == 28)   /* TPCII::CONVERT_UINT16 */
      ) {
        if (isJmpInstr(DMI)) {
          alatency = std::max(alatency, 3);
        }
      }

      // 2 Instruction after JMP* or LOOP delay slot, a ASO with SPU_ASO_OP is not allowed to be issued.
      // TODO: check for SPU_ASO_OP
      //
      if (TPCII::isStoreInst(UMI->getDesc()) &&
          opcUse == TPCII::ASO
      ) {
        if (isJmpInstr(DMI)) {
          alatency = std::max(alatency, 3);
        }
      }

      // At least one instruction should separate a VLIW instruction which
      // contain LD_TNSR_LOW and a VLIW instruction which contain
      // LD_TNSR_HIGH, regardless which one is first
      //
      if (TPCII::isLoadInst(DMI->getDesc()) && TPCII::isLoadInst(UMI->getDesc()) &&
          ((opcDef == TPCII::LD_TNSR_LOW &&
            opcUse == TPCII::LD_TNSR_HIGH) ||
          ((opcDef == TPCII::LD_TNSR_HIGH &&
            opcUse == TPCII::LD_TNSR_LOW)))
        ) {
          alatency = std::max(alatency, 2);
      }

      // VLIW instruction which contains LD_TNSR_LOW cannot be followed with LD_TNSR
      //
      if (TPCII::isLoadInst(DMI->getDesc()) && TPCII::isLoadInst(UMI->getDesc()) &&
          (opcDef == TPCII::LD_TNSR_LOW &&  // LD_TNSR_LOW
           opcUse == TPCII::LD_TNSR)    // LD_TNSR
        ) {
          alatency = std::max(alatency, 2);
      }

      // Gaudi (Gen2): The following cycle after MAC/MUL BF16/FP32, it is not allowed
      //               to schedule an instruction which writes to the same destination
      //               (or destinations) as the MAC/MUL
      //
      if (!MF->getSubtarget<TPCSubtarget>().hasGoyaISA()) {
        const MCInstrDesc &DefMCID = DMI->getDesc();
        const MCInstrDesc &UseMCID = UMI->getDesc();
        bool isDefRestrict = false;
        if ((TPCII::isVPUInst(DefMCID) && (opcDef == TPCII::vpuMAC || opcDef == TPCII::vpuMUL)) ||
            (TPCII::isSPUInst(DefMCID) && (opcDef == TPCII::spuMAC || opcDef == TPCII::spuMUL))) {
          isDefRestrict = isFloatData(getOpType(DefMI));
        }
        if (isDefRestrict &&
            ((TPCII::isVPUInst(UseMCID) && (opcUse == TPCII::vpuMAC || opcUse == TPCII::vpuMUL)) ||
             (TPCII::isSPUInst(UseMCID) && (opcUse == TPCII::spuMAC || opcUse == TPCII::spuMUL)) ||
             (TPCII::isVPUInst(UseMCID) && (opcUse == TPCII::vpuADD || opcUse == TPCII::vpuSUB)) ||
             (TPCII::isSPUInst(UseMCID) && (opcUse == TPCII::spuADD || opcUse == TPCII::spuSUB)) ||
             (TPCII::isVPUInst(UseMCID) && (opcUse == TPCII::vpuMADD))))
        {
          // Check if the "use" instr writes to the same destination as "def"
          const MachineOperand &DMO = DMI->getOperand(0);
          assert(DMO.isReg());
          unsigned Dreg = DMO.getReg();
          if (UMI->getNumOperands() > 0) {
            const MachineOperand &UMO = UMI->getOperand(0);
            if (UMO.isReg()) {
              unsigned Ureg = UMO.getReg();
              if (Ureg == Dreg) { // restriction
                alatency = std::max(alatency, 2);
              }
            }
          }
        }
      }

      // At least one instruction should separate a VLIW instruction which contain
      // LD_TNSR_LOW and a VLIW instruction which contain LD_TNSR_HIGH, regardless
      // which one is 1st
      //
      {
        bool DefIsLow  = false;
        bool DefIsHigh = false;
        bool UseIsLow  = false;
        bool UseIsHigh = false;
        if (TPCII::isLoadInst(DMI->getDesc())) {
          DefIsLow  = (opcDef == TPCII::LD_TNSR_LOW);
          DefIsHigh = (opcDef == TPCII::LD_TNSR_HIGH);
        }
        else if (TPCII::isStoreInst(DMI->getDesc())) {
          DefIsLow  = (opcDef == TPCII::stLD_TNSR_LOW);
          DefIsHigh = (opcDef == TPCII::stLD_TNSR_HIGH);
        }
        if (TPCII::isLoadInst(UMI->getDesc())) {
          UseIsLow  = (opcUse == TPCII::LD_TNSR_LOW);
          UseIsHigh = (opcUse == TPCII::LD_TNSR_HIGH);
        }
        else if (TPCII::isStoreInst(UMI->getDesc())) {
          UseIsLow  = (opcUse == TPCII::stLD_TNSR_LOW);
          UseIsHigh = (opcUse == TPCII::stLD_TNSR_HIGH);
        }
        if ((DefIsLow && UseIsHigh) || (DefIsHigh && UseIsLow)) {
          alatency = std::max(alatency, 2);
        }
      }

      // 4 Instructions before LOOP instruction, ST_L_V/LD_L_V/LD.MOV (VRF/VPFR)/VPU*
      // cannot use LOOP iterator as Source.
      //
      // On the 6 Last instructions of a LOOP, ST_L_V/LD_L_V/LD.MOV (VRF/VPRF)/VPU*
      // cannot use LOOP Iterator as Source
      //
      if (TPCII::isLoopInst(UMI->getDesc())) {
        if ((TPCII::isStoreInst(DMI->getDesc()) && opcDef == TPCII::ST_L_V) ||
            (TPCII::isLoadInst(DMI->getDesc())  && opcDef == TPCII::LD_L) ||
            (TPCII::isLoadInst(DMI->getDesc())  && opcDef == TPCII::ldMOV) ||
            (TPCII::isVPUInst(DMI->getDesc())   && !isNopInstr(DMI))
            ) {
          bool use_loop_iterator = HasLoopIter(*DMI);
          if (use_loop_iterator) {
            if (UMI->getOpcode() != TPC::LOOPEND) { // LOOP
              alatency = std::max(alatency, 6);
            }
            else { // LOOPEND
              alatency = std::max(alatency, 8);
            }
          }
        }

        // Loop iterator can’t be used as source in the 4 instructions before a
        // LOOP on VPU SLOT
        if (!isNopInstr(DMI) &&
            TPCII::isVPUInst(DMI->getDesc())) {
          if (HasLoopIter(*DMI))
            alatency = std::max(alatency, 5);
        }

        // Loop iterator can’t be used as source in the last 6 instructions
        // inside a LOOP on VPU SLOT
        if (UMI->getOpcode() == TPC::LOOPEND &&
            !isNopInstr(DMI) &&
            TPCII::isVPUInst(DMI->getDesc())) {
          if (HasLoopIter(*DMI))
            // With LOOPEND pseudo instruction
            alatency = std::max(alatency, 8);
          }
      }

      const TPCSubtarget &Subtarget = MF->getSubtarget<TPCSubtarget>();

      // At least 6 instructions must separate between 2 consecutive
      // UDIV_4STEP instructions.
      if (Subtarget.hasGaudiISA())
        if (TPCII::isSPUInst(DMI->getDesc()) &&
            opcDef == TPCII::spuUDIV_4STEP &&
            TPCII::isSPUInst(UMI->getDesc()) &&
            opcUse == TPCII::spuUDIV_4STEP) {
          alatency = std::max(alatency, 7);
        }

      // MMIO configuration restrictions:
      // * CSR_MODE configuration has a latency of 3 cycles (i.e. 2
      //   instructions must separate the configuration of CSR_MODE and a
      //   consumer of CSR_MODE).
      // * A few MMIO configurations done by ST_L have latency of 2 cycles:
      //   KERNEL_TENSOR_*_DIM_*_STRIDE, CFG_BASE_ADDRESS_HIGH,
      //   CFG_SUBTRACT_VALUE, KERNEL.KERNEL_CONFIG.ASO_EVICT_L0 (i.e. 1
      //   instruction must separate such a configuration and the consumer
      //   of this configuration).
      // * The rest of MMIO configurations done by ST_L have latency of 1 cycle
      //   (i.e. can be consumed by the following instruction).
      //
      // From letter
      // CSR_MODE includes both ROUND_CSR and CONV_ROUND_CSR ports?
      // Yes. It includes also CONV_ROUND_CSR. I will write it explicitly in
      // PRM.
      //
      // What are exact consumers of CSR_MODE - CONVERT, NEARBYINT?
      // The consumers of ROUND_CSR are all the Floating-Point arithmetic
      // instructions: MAC, MUL, ADD, SUB, MADD. The consumers of
      // CONV_ROUND_CSR are all the CONVERT* and NEARBYINT instructions
      // which use DEFAULT rounding mode.
      //
      // What instructions are consumers of other specific configurations?
      // * The consumers of all tensor descriptor configurations
      //   (KERNEL_TENSOR_*) are GEN_ADDR, LD_TNSR* and ST_TNSR*.
      // * The consumer of CFG_SUBTRACT_VALUE is LD_L/ST_L with MMIO.
      // * The consumers of LFSR_POLYNOM are MOV which reads from V40 and
      //   CONVERT* with SR rounding mode on VPU.
      // * The consumers of SPE_LFSR_POLYNOM are MOV which reads from S40 and
      //   CONVERT* with SR rounding mode on SPU.
      // * The consumer of KERNEL.KERNEL_CONFIG.ASO_EVICT_L0 is ASO.
      // * The consumers of KERNEL.KERNEL_CONFIG.IRF_32BIT_COMPATIBILITY are
      //   all the instructions that use IRFs, as well as GEN_ADDR, LD_TNSR*
      //   and ST_TNSR*.

      // TODO: think how to move it to general code for TPCSubtarget.cpp.
      if (Subtarget.hasGen2Plus() &&
          TPCII::isStoreInst(DMI->getDesc()) &&
          TPCII::getSlotOpCode(DMI->getDesc()) == TPCII::ST_L &&
          DMI->getOperand(
            DMI->getNumOperands() - 3).getImm() & TPCII::SW_MMIO) {
        unsigned Opcode = TPCII::getSlotOpCode(UMI->getDesc());
        unsigned NumOperands = UMI->getNumOperands();
        const MCInstrDesc &Desc = UMI->getDesc();

        if ((TPCII::isSPUInst(Desc) &&
            (Opcode == TPCII::spuADD ||
             Opcode == TPCII::spuMAC ||
             Opcode == TPCII::spuMUL ||
             Opcode == TPCII::spuSUB)) ||
            (TPCII::isVPUInst(Desc) &&
             (Opcode == TPCII::vpuADD ||
              Opcode == TPCII::vpuMAC ||
              Opcode == TPCII::vpuMADD ||
              Opcode == TPCII::vpuMUL ||
              Opcode == TPCII::vpuSUB))) {
          TPCII::OpType Type = getOpType(*UMI);
          switch (Type) {
          case TPCII::OpType::FP32:
          case TPCII::OpType::BF16:
            alatency = std::max(alatency, 3);
            break;
          default:
            break;
          }
        }
        else if ((TPCII::isSPUInst(UMI->getDesc()) &&
                  (Opcode == TPCII::spuCONVERT ||
                   Opcode == TPCII::spuCONVERT_INT8 ||
                   Opcode == TPCII::spuCONVERT_INT16 ||
                   Opcode == TPCII::spuCONVERT_INT32 ||
                   Opcode == TPCII::spuCONVERT_INT64 ||
                   Opcode == TPCII::spuCONVERT_UINT8 ||
                   Opcode == TPCII::spuCONVERT_UINT16 ||
                   Opcode == TPCII::spuCONVERT_UINT32)) ||
                 (TPCII::isVPUInst(UMI->getDesc()) &&
                  (Opcode == TPCII::vpuCONVERT ||
                   Opcode == TPCII::vpuCONVERT_INT8 ||
                   Opcode == TPCII::vpuCONVERT_INT16 ||
                   Opcode == TPCII::vpuCONVERT_INT32 ||
                   Opcode == TPCII::vpuCONVERT_UINT8 ||
                   Opcode == TPCII::vpuCONVERT_UINT16 ||
                   Opcode == TPCII::vpuCONVERT_UINT32))) {
          unsigned RoundMode =
              UMI->getOperand(NumOperands - 4).getImm() & TPCII::SW_GROUP_RM;
          if (RoundMode == TPCII::SW_CSR)
            alatency = std::max(alatency, 3);
          else if (RoundMode == TPCII::SW_SR)
            alatency = std::max(alatency, 2);
        }
        else if ((TPCII::isSPUInst(UMI->getDesc()) &&
                  Opcode == TPCII::spuNEARBYINT) ||
                 (TPCII::isVPUInst(UMI->getDesc()) &&
                  Opcode == TPCII::vpuNEARBYINT)) {
          unsigned RoundMode =
              UMI->getOperand(NumOperands - 4).getImm() & TPCII::SW_GROUP_RM;
          if (RoundMode == TPCII::SW_CSR)
            alatency = std::max(alatency, 3);
        }
        else if ((TPCII::isLoadInst(Desc) &&
                  (Opcode == TPCII::ldGEN_ADDR ||
                   Opcode == TPCII::LD_TNSR ||
                   Opcode == TPCII::LD_TNSR_HIGH ||
                   Opcode == TPCII::LD_TNSR_LOW)) ||
                 (TPCII::isStoreInst(Desc) &&
                  (Opcode == TPCII::stGEN_ADDR ||
                   Opcode == TPCII::stLD_TNSR ||
                   Opcode == TPCII::stLD_TNSR_HIGH ||
                   Opcode == TPCII::stLD_TNSR_LOW ||
                   Opcode == TPCII::ST_TNSR ||
                   Opcode == TPCII::ST_TNSR_HIGH ||
                   Opcode == TPCII::ST_TNSR_LOW))) {
          alatency = std::max(alatency, 2);
        }
        else if ((TPCII::isLoadInst(Desc) && Opcode == TPCII::LD_L)) {
          bool isMMIO =
              UMI->getOperand(NumOperands - 4).getImm() & TPCII::SW_MMIO;
          if (isMMIO)
            alatency = std::max(alatency, 2);
        }
        else if (TPCII::isStoreInst(Desc) && Opcode == TPCII::ST_L) {
          bool isMMIO =
              UMI->getOperand(NumOperands - 3).getImm() & TPCII::SW_MMIO;
          if (isMMIO)
            alatency = std::max(alatency, 2);
        }
        else if ((TPCII::isSPUInst(Desc) &&
                  (UMI->getOpcode() == TPC::ReadSLFSR ||
                   UMI->getOpcode() == TPC::ReadSLFSRNC)) ||
                 (TPCII::isVPUInst(Desc) &&
                  (UMI->getOpcode() == TPC::ReadLFSR ||
                   UMI->getOpcode() == TPC::ReadLFSRNC))) {
          alatency = std::max(alatency, 2);
        }
        else if (TPCII::isStoreInst(Desc) && Opcode == TPCII::ASO) {
          alatency = std::max(alatency, 2);
        }
      }
    }
  }

  if (DefMI.getOpcode() == TPC::LOOPEND && UseMI.getOpcode() == TPC::LOOPEND) {
    alatency = isLoopTaken(&DefMI) ? 2 : 4;
  }

  // The LOOKUP* instruction cannot be the last instruction in a loop block.
  if (HasLookup(DefMI) && UseMI.getOpcode() == TPC::LOOPEND) {
    alatency = std::max(alatency, 2);
  }
  
  // JMPR/A inside loop must be at least 6 instructions before the Loop Label
  //
  if (isJmpInstr(&DefMI) && UseMI.getOpcode() == TPC::LOOPEND) {
    alatency = 7;
  }

  if (needLdTnsrWorkaround && defHasStTnsr && useHasLdTnsr) {
    if (alatency == 4 || curDistance == 4) {
      alatency = 5;
      //dbgs() << "Def: "; printMI((MachineInstr*)&DefMI);
      //dbgs() << "Use: "; printMI((MachineInstr*)&UseMI);
      //dbgs() << "-------\n";
    }
  }

  return alatency;
}

//
// resolveBlockLatency ()
// Checks distance requirements between instructions in one basic block.
// Inserts NOPs between instructions if distance requirements are not met.
//
bool TPCLatencyResolver::resolveBlockLatency(MachineBasicBlock& MBB, bool check_only, bool ignore_loop_instr) {
  MachineBasicBlock::iterator BegBB = MBB.begin();
  MachineBasicBlock::iterator EndBB = MBB.end();
  int Cycle = 0;
  int numPreNops;
  bool changed = false;

  if (check_only) {
    LLVM_DEBUG(dbgs() << "Check latencies in BB#" << MBB.getNumber() << "\n");
  }
  else {
    LLVM_DEBUG(dbgs() << "Resolve latencies in BB#" << MBB.getNumber() << "\n");
  }

  for (MachineBasicBlock::iterator I = BegBB, E = EndBB; I != E; Cycle++) {
    MachineInstr &MI = *I;
    ++I;
    if (ignoreInstr(&MI)) {
      Cycle--;
      continue;
    }
    MachineBasicBlock::iterator MIIt = MI;
    if (MIIt != BegBB) {
      MIIt--;
      int defCycle = Cycle - 1;
      if (ignore_loop_instr && TPCII::isLoopInst(MI.getDesc()) && (MI.getOpcode() != TPC::LOOPEND)) {
        Cycle--;
      }

      int defCycleSaved = defCycle;
      do {
        numPreNops = 0;
        for (MachineBasicBlock::iterator J = MIIt; defCycle >= 0; defCycle--) {
          MachineInstr &defMI = *J;
          --J;
          if (ignoreInstr(&defMI)) {
            defCycle++;
            continue;
          }
          int lat = getRequiredDistance(defMI, MI, Cycle - defCycle);
          if (lat > (Cycle - defCycle)) {
            int numNops = lat - (Cycle - defCycle);
            LLVM_DEBUG(dbgs() << "   NOP(" << numNops << ") needed before MI(" << Cycle << ")\n"; printMI(&MI));
            LLVM_DEBUG(dbgs() << "   latency(" << lat << ") with MI(" << defCycle << ")\n"; printMI(&defMI));
            numPreNops = std::max(numPreNops, numNops);
          }
        }
        if (numPreNops) {
          if (!check_only) {
            LLVM_DEBUG(dbgs() << "   NOP(" << numPreNops << ") inserted before MI(" << Cycle << ")\n"; printMI(&MI));
            for (int j = 0; j < numPreNops; ++j) {
        	TII->insertNoop(MBB, MachineBasicBlock::iterator(MI));
        	Cycle++;
            }
          } else {
            Cycle++;
          }
          changed = true;

          // Go back and check if nops are still needed for the current instruction.
          // This may happen if some instructions require the distance not equal to
          // an exact number of cycles (for example, see EnableLdTnsrGaudiWorkaround)
          defCycle = defCycleSaved;
        }
      } while (numPreNops > 0);
    }

    if (isHaltInstr(&MI)) {
      //
      // PRM_0.999; 1.3.3: At least two instruction must exist between
      // a JMP* instruction and a HALT instruction.
      // We do not look for a JMP here - we just insert 2 NOPs before the HALT
      //
      if (Cycle < 2) {
        TII->insertNoop(MBB, MachineBasicBlock::iterator(MI));
        TII->insertNoop(MBB, MachineBasicBlock::iterator(MI));
        LLVM_DEBUG(dbgs() << "   NOP(" << 2 << ") inserted before HALT\n");
      }

      //
      // PRM 0.9992: After HALT, 3 VLIW instructions which are NOPs should appear
      //
      TII->insertNoop(MBB, MBB.end());
      TII->insertNoop(MBB, MBB.end());
      TII->insertNoop(MBB, MBB.end());
      LLVM_DEBUG(dbgs() << "   NOP(" << 3 << ") inserted after HALT\n");
    }

    // Special case when there're no real instructions in the Latch with two or more
    // predecessors
    if (MI.getOpcode() == TPC::LOOPEND) {
      if (Cycle == 0 && MBB.pred_size() > 1) {
        TII->insertNoop(MBB, MBB.front());
        LLVM_DEBUG(dbgs() << "   NOP(" << 1 << ") inserted before LOOPEND in empty block\n");
      }
    }
  }

  return changed;
}

// The following sequence is not allowed:
// st_l_v <addX>
// st_l_v <addY>
// nop (optional)
// ld_l_v <addX>
// ld_l_v <addY>
// LOOKUP
static bool IsUnluckyLookup(MachineInstr& Instr) {
  const unsigned STATE_ST_L_V_2 = 5;
  const unsigned STATE_ST_L_V_1 = 4;
  const unsigned STATE_NOP = 3;
  const unsigned STATE_LD_L_V_2 = 2;
  const unsigned STATE_LD_L_V_1 = 1;
  const unsigned STATE_START = 0;

  assert(HasLookup(Instr) && "Must call only for Lookup instruction");

  MachineBasicBlock::reverse_iterator CurrentMII(Instr); ++CurrentMII;
  MachineBasicBlock *CurrentBB = Instr.getParent();
  unsigned State = STATE_START;

  for (;;) {
    if (CurrentMII == CurrentBB->rend()) {
      bool HasPrevious = false;
      for (MachineBasicBlock *Pred : CurrentBB->predecessors()) {
        if (Pred && Pred->isLayoutSuccessor(CurrentBB)) {
            CurrentBB = Pred;
            CurrentMII = Pred->rbegin();
            HasPrevious = true;
            break;
        }
      }

      if (!HasPrevious)
        break;

      if (CurrentBB->empty())
        break;
    }

    MachineInstr &CurrentMI = *CurrentMII;
    if (!CurrentMI.isBundle() && !isNopInstr(&CurrentMI))
      break;

    bool HasST_L_V = false;
    bool HasLD_L_V = false;
    MachineBasicBlock::const_instr_iterator BMII = CurrentMI.getIterator();
    for (++BMII;
         BMII != CurrentBB->instr_end() && BMII->isInsideBundle();
         ++BMII) {
      const MachineInstr &BMI = *BMII;
      const MCInstrDesc &BMID = BMI.getDesc();
      const unsigned SlotOpcode = TPCII::getSlotOpCode(BMID);

      if (TPCII::isStoreInst(BMID) &&
          (SlotOpcode == TPCII::ST_L_V ||
           SlotOpcode == TPCII::ST_L_V_HIGH ||
           SlotOpcode == TPCII::ST_L_V_LOW))
        HasST_L_V = true;
      else if (TPCII::isLoadInst(BMID) &&
               (SlotOpcode == TPCII::LD_L_V ||
                SlotOpcode == TPCII::LD_L_V_HIGH ||
                SlotOpcode == TPCII::LD_L_V_LOW))
        HasLD_L_V = true;
    }

    if (State == STATE_START && HasLD_L_V)
      State = STATE_LD_L_V_1;
    else if (State == STATE_LD_L_V_1 && HasLD_L_V)
      State = STATE_LD_L_V_2;
    else if (State == STATE_LD_L_V_2 && isNopInstr(&CurrentMI))
      State = STATE_NOP;
    else if ((State == STATE_NOP || State == STATE_LD_L_V_2) &&
             HasST_L_V)
      State = STATE_ST_L_V_1;
    else if (State == STATE_ST_L_V_1 && HasST_L_V) {
      State = STATE_ST_L_V_2;
      break;
    } else {
      State = STATE_START;
      break;
    }

    ++CurrentMII;
  }

  return State == STATE_ST_L_V_2;
}


bool TPCLatencyResolver::resolveFunctionLatency(MachineFunction &MF) {

  bool IsChanged = false;

  return IsChanged;

  for (MachineBasicBlock &MBB : MF) {
    for(MachineBasicBlock::reverse_iterator MII = MBB.rbegin();
        MII != MBB.rend();
        ++MII) {
      MachineInstr &MI = *MII;

      if (HasLookup(MI) && IsUnluckyLookup(MI)) {
        TII->insertNoop(MBB, MachineBasicBlock::iterator(MI));
        IsChanged = true;
      }
    }
  }

  return IsChanged;
}

//
// getRequiredBBDistance ()
// Returns the distance (in cycles) required between the end of one BB and the start of
// other BB.
//
int TPCLatencyResolver::getRequiredBBDistance(MachineBasicBlock* PBB,
                                              MachineBasicBlock* SBB,
                                              bool ignore_loop_instr)
{
  int DefCycle;
  int UseCycle;
  int dist = 0;

  UseCycle = 0;
  for (MachineBasicBlock::iterator I = SBB->begin(), E = SBB->end(); I != E;) {
    MachineInstr &UseMI = *I;
    ++I;
    if (ignoreInstr(&UseMI)) {
      continue;
    }
    UseCycle++;

    if (ignore_loop_instr && TPCII::isLoopInst(UseMI.getDesc()) && (UseMI.getOpcode() != TPC::LOOPEND)) {
      UseCycle--;
    }
    if (UseCycle > TPC_MAX_LATENCY) {
      // Reached max latency, no need to process more instructions.
      break;
    }

    DefCycle = 0;
    for (MachineBasicBlock::iterator J = PBB->end(), PE = PBB->begin(); J != PE;) {
      --J;
      MachineInstr &DefMI = *J;
      if (ignoreInstr(&DefMI)) {
        continue;
      }
      if (DefMI.getOpcode() != TPC::LOOPEND) {
        DefCycle++;
      }

      if (isJmpInstr(&DefMI)) {
          MachineBasicBlock * DestMBB = getJmpDest(&DefMI);
          if (DestMBB && DestMBB->getNumber() == SBB->getNumber()) {
            DefCycle = 0;
          }
      }

      if (ignore_loop_instr && TPCII::isLoopInst(DefMI.getDesc()) && (DefMI.getOpcode() != TPC::LOOPEND)) {
        DefCycle--;
      }
      if ((UseCycle + DefCycle) > TPC_MAX_LATENCY) {
        // Reached max latency, no need to process more instructions.
        break;
      }
      int lat;
      if (DefMI.getOpcode() == TPC::LOOPEND && UseMI.getOpcode() != TPC::LOOPEND) {
        // do not consider LOOPEND pseudo instruction as it is really a noop.
        lat = 0;
      }
      else {
        lat = getRequiredDistance(DefMI, UseMI, UseCycle + DefCycle - 1);
      }

      if (lat > (UseCycle + DefCycle - 1)) {
        int numNops = lat - (UseCycle + DefCycle - 1);
        LLVM_DEBUG(dbgs() << "   Cycles(" << numNops << ") needed before MI(BB#"
                     << SBB->getNumber() << ":" << UseCycle << ")\n"; printMI(&UseMI));
        LLVM_DEBUG(dbgs() << "   latency(" << lat << ") with MI(BB#"
                     << PBB->getNumber() << ":" << DefCycle << ")\n"; printMI(&DefMI));
        dist = std::max(dist, numNops);
      }
    }
  }
  LLVM_DEBUG(dbgs() << "   Distance(" << dist << ") needed between BB#"
               << PBB->getNumber() << " and BB#" << SBB->getNumber() << "\n");
  return dist;
}

//
// getBBCycles ()
// Returns the number of VLIW instructions in MBB.
//
int TPCLatencyResolver::getBBCycles(MachineBasicBlock* MBB, MachineBasicBlock* Succ, bool ignore_loop_instr)
{
  int Cycle = 0;
  for (MachineBasicBlock::iterator J = MBB->end(), PE = MBB->begin(); J != PE;) {
    --J;
    MachineInstr &DefMI = *J;
    if (ignoreInstr(&DefMI)) {
      continue;
    }
    if (DefMI.getOpcode() != TPC::LOOPEND) {
      Cycle++;
    }
    if (ignore_loop_instr && TPCII::isLoopInst(DefMI.getDesc()) && (DefMI.getOpcode() != TPC::LOOPEND)) {
      Cycle--;
    }
    if (Succ && isJmpInstr(&DefMI)) {
      //
      // Consider the following dependency on V18:
      //
      // .BB0_10:
      // 	MOV	%V32, %V18
      // 	...
      // 	MOV_DUAL_GROUP.F %V18, %V18, ...
      // .BB0_11:
      // 	cmp_less.i32 %SP1, %S8, %S10, %SP0
      // 	nop
      // 	jmpr .LBB0_10, %SP1
      // 	nop
      // 	jmpr .LBB0_7
      // .BB0_12:
      // 	...
      // getBBCycles (.BB0_11) will return 5 cycles, but as predecessor of .BB0_10 it has only 3 cycles
      // due to jmpr .LBB0_10, %SP1.
      // Ideally, the block .BB0_11 should be split into two blocks, like this:
      // 	...
      // .BB0_11:
      // 	cmp_less.i32 %SP1, %S8, %S10, %SP0
      // 	nop
      // 	jmpr .LBB0_10, %SP1
      // .BB0_11_1:
      // 	nop
      // 	jmpr .LBB0_7
      // .BB0_12:
      //
      MachineBasicBlock * DestMBB = getJmpDest(&DefMI);
      if (DestMBB && DestMBB->getNumber() == Succ->getNumber()) {
        break;
      }
    }
    if (Cycle > TPC_MAX_LATENCY) {
      // Reached max latency, no need to process more instructions.
      break;
    }
  }
  return Cycle;
}

bool TPCLatencyResolver::resolveCrossBlockLatency(MachineBasicBlock& MBB,
                                                  MachineBasicBlock* PredMBB,
                                                  bool topdown,
                                                  bool deep,
                                                  int  distance,
                                                  bool check_only,
                                                  bool ignore_loop_instr,
                                                  std::vector<const MachineBasicBlock*> ProcessedBB)
{
  int lat = 0;
  int NumCycles = 0;
  int NumPreNops = 0;
  bool changed = false;
  MachineBasicBlock::pred_iterator BEG;
  MachineBasicBlock::pred_iterator END;
  
  if (topdown) {
    LLVM_DEBUG(dbgs() << "Checking Successors of BB#" << PredMBB->getNumber() << "\n");
    BEG = PredMBB->succ_begin();
    END = PredMBB->succ_end();
  }
  else {
    LLVM_DEBUG(dbgs() << "Checking predecessors of BB#" << PredMBB->getNumber() << "\n");
    BEG = PredMBB->pred_begin();
    END = PredMBB->pred_end();
  }
  LLVM_DEBUG(dbgs() << "  (dist = " << distance << ")\n");
  ProcessedBB.push_back(PredMBB);

  for (MachineBasicBlock::pred_iterator PI = BEG, PE = END; PI != PE; ++PI) {
    MachineBasicBlock *Pred = *PI;
    MachineBasicBlock *PredBB;
    MachineBasicBlock *SuccBB;
    //DEBUG(dbgs() << " - checking BB#" << Pred->getNumber() << "\n");

    if (topdown) {
      PredBB = &MBB;
      SuccBB = Pred;
    }
    else {
      PredBB = Pred;
      SuccBB = &MBB;
    }
    
    if (!Pred->empty()) {
      do {
        lat = getRequiredBBDistance(PredBB, SuccBB, ignore_loop_instr);
        NumPreNops = lat - distance;

        // Insert NOPs
        if (NumPreNops > 0) {
          // If NOPs needed between loop start and loop body then it is better
          // to insert them before the LOOP instr so that the body isn't impacted.
          if (isStartLoopBlock(*PredBB)) {
            if (!check_only) {
              LLVM_DEBUG(dbgs() << "     NOP(" << NumPreNops << ") inserted at end of BB#" << PredBB->getNumber() << "\n");
              for (int j = 0; j < NumPreNops; ++j) {
                TII->insertNoop(*PredBB, --(PredBB->end()));
              }
            } else {
              LLVM_DEBUG(dbgs() << "   NOP(" << NumPreNops << ") would be inserted at end of BB#" << PredBB->getNumber() << "\n");
            }
          } else {
            if (!check_only) {
              LLVM_DEBUG(dbgs() << "   NOP(" << NumPreNops << ") inserted at BB#" << SuccBB->getNumber() << " start\n");
              for (int j = 0; j < NumPreNops; ++j) {
                TII->insertNoop(*SuccBB, SuccBB->front());
              }
            } else {
              LLVM_DEBUG(dbgs() << "   NOP(" << NumPreNops << ") would be inserted at BB#" << SuccBB->getNumber() << " start\n");
            }
          }
          changed = true;
        }
      } while (NumPreNops > 0 && !check_only);
    }
    if (deep) {
      NumCycles = distance + getBBCycles(Pred, &MBB, ignore_loop_instr);
      if (NumCycles < TPC_MAX_LATENCY) {
        //DEBUG(dbgs() << " - small BB#" << Pred->getNumber() << "("<< NumCycles << ")\n");
        bool alreadyProcessed = false;
        for (auto BB : ProcessedBB) {
          if (BB->getNumber() == Pred->getNumber()) {
            alreadyProcessed = true;
            break;
          }
        }
        if (!alreadyProcessed) {
          changed |= resolveCrossBlockLatency(MBB, Pred, topdown, deep, NumCycles, check_only, ignore_loop_instr, ProcessedBB);
        }
      }
    }
  }
  return changed;
}

//
// resolveCrossBlockLatency: resolve latencies between current block and its predecessors.
// Inserts NOPs at the beginnig of current block.
//
bool TPCLatencyResolver::resolveCrossBlockLatency(MachineBasicBlock& MBB,
                                                  bool topdown,
                                                  bool deep,
                                                  bool check_only,
                                                  bool ignore_loop_instr)
{
  std::vector<const MachineBasicBlock*> ProcessedBB;

  LLVM_DEBUG(dbgs() << "Resolve Cross Block latencies for BB#" << MBB.getNumber() << "\n");

  return resolveCrossBlockLatency(MBB, &MBB, topdown, deep, 0, check_only, ignore_loop_instr, ProcessedBB);
}

//
// Restriction: JMPA and JMPR destinations must be at least three instructions before
// the last instruction pointed to by PC_OFFSSET.
//
void TPCLatencyResolver::ensureLoopEndAndJmpLatency(MachineBasicBlock& MBB)
{
  MachineBasicBlock::iterator BegBB = MBB.begin();
  MachineBasicBlock::iterator EndBB = MBB.end();
  MachineBasicBlock* JmpDestBB = nullptr;
  unsigned destBlkSize = 0;
  unsigned pcNum = 0;

  LLVM_DEBUG(dbgs() << " *** Fixing loop end and jmp (BB_" << MBB.getNumber() << ")\n");

  for (MachineBasicBlock::iterator I = BegBB, E = EndBB; I != E;) {
    MachineInstr &MI = *I;

    if (isJmpInstr(&MI)) {
      // Found a JMP - get the destination block of that JMP
      JmpDestBB = getJmpDest(&MI);
      assert(JmpDestBB && "NULL destination for JMP.");
      LLVM_DEBUG(dbgs() << "    - JMP to BB_" << JmpDestBB->getNumber() << "\n");
      destBlkSize = 0;
      // Calculate the number of instructions in the dest block
      if (!JmpDestBB->empty()) {
        for (MachineBasicBlock::iterator J = JmpDestBB->begin(), JBE = JmpDestBB->end(); J != JBE;) {
          destBlkSize++;
          MachineInstr &bMI = *J;
          if (bMI.getOpcode() == TPC::LOOPEND) {
            LLVM_DEBUG(dbgs() << " * LOOPEND at(" << destBlkSize << ") in BB_" << JmpDestBB->getNumber() << "\n");
            destBlkSize--;
            if (destBlkSize < 4) {
              int pnops = 4 - destBlkSize;
              for (int i=0; i<pnops; i++) {
                TII->insertNoop(*JmpDestBB, MachineBasicBlock::iterator(bMI));
                destBlkSize++;
              }
              LLVM_DEBUG(dbgs() << "     NOP(" << pnops << ") inserted in BB#" << JmpDestBB->getNumber() << " (JMP)\n");
              break;
            }
          }
          ++J;
        }
      }
      LLVM_DEBUG(dbgs() << "    - InstNum(" << destBlkSize << ") in BB_" << JmpDestBB->getNumber() << "\n");
      // If the number of instructions in the dest block > 3 then we have enough distance for
      // any LOOPEND in subsequent blocks.
      if (destBlkSize > 3) {
        ++I;
        continue;
      }
      // Look for the next block after the JMP dest, and check if there is a LOOPEND there.
      MachineBasicBlock* JmpDestNextBB;
      MachineFunction::iterator mbbit(JmpDestBB);
      if (++mbbit == MF->end()) {
        JmpDestNextBB = nullptr;
        LLVM_DEBUG(dbgs() << "   * next BB_" << "NULL" << "\n");
      }
      else {
        JmpDestNextBB = &*mbbit;
        LLVM_DEBUG(dbgs() << "   * next BB_" << JmpDestNextBB->getNumber() << "\n");
      }
      if (JmpDestNextBB) {
        pcNum = 0;
        for (MachineBasicBlock::iterator J = JmpDestNextBB->begin(), JBE = JmpDestNextBB->end(); J != JBE;) {
          MachineInstr &nMI = *J;
          pcNum++;
          if (nMI.getOpcode() == TPC::LOOPEND) {
            LLVM_DEBUG(dbgs() << " * LOOPEND at(" << pcNum << ") in BB_" << JmpDestNextBB->getNumber() << "\n");
            if ((destBlkSize + pcNum - 2) < 3) {
              int pnops = 5 - destBlkSize - pcNum;
              for (int i=0; i<pnops; i++) {
                TII->insertNoop(*JmpDestBB, JmpDestBB->end());
              }
              LLVM_DEBUG(dbgs() << "     NOP(" << pnops << ") inserted at end of BB#" << JmpDestBB->getNumber() << " (JMP)\n");
            }
          }
          ++J;
        }
      }
    }
   ++I;
  }
}

void TPCLatencyResolver::fixSmallLoops(MachineBasicBlock& MBB) {
  MachineBasicBlock::iterator BegBB = MBB.begin();
  MachineBasicBlock::iterator EndBB = MBB.end();
  unsigned InstrNum = 0;

  MachineLoop * L = MLI->getLoopFor(&MBB);

  if (!L) return;

  if (L->isLoopLatch(&MBB)) {
    MachineBasicBlock * header = L->getHeader();
    assert(header);

    if (header->getNumber() != MBB.getNumber()) {
      return;
    }

    LLVM_DEBUG(dbgs() << "Fixing loop instr count in BB#" << MBB.getNumber() << "\n");

    // Loop over last 8 instructions in current block
    //
    for (MachineBasicBlock::iterator I = BegBB, E = EndBB; I != E;) {
      if (InstrNum > 8) {
        break;
      }
      MachineInstr &MI = *I;
      if (ignoreInstr(&MI)) {
        ++I;
        continue;
      }
      InstrNum++;

      if (MI.getOpcode() == TPC::LOOPEND && InstrNum < 8) {
        int pnops = 7 - InstrNum;
        for (int i=0; i<pnops; i++) {
          TII->insertNoop(MBB, MachineBasicBlock::iterator(MI));
        }
        LLVM_DEBUG(dbgs() << "     NOP(" << pnops << ") inserted at end of BB#" << MBB.getNumber() << " (small loop)\n");
        break;
      }
      ++I;
    }
  }
}

static MachineBasicBlock *GetNextNonEmptyMBB(const MachineBasicBlock &MBB) {
  MachineBasicBlock *NextMBB = nullptr;
  for (MachineBasicBlock *Next : MBB.successors()) {
    if (Next && MBB.isLayoutSuccessor(Next)) {
      NextMBB = Next;
      break;
    }
  }

  if (!NextMBB)
    return NextMBB;
  else if (!NextMBB->empty())
    return NextMBB;
  else
    return GetNextNonEmptyMBB(*NextMBB);
}

void TPCLatencyResolver::fillLoopDelaySlot(MachineBasicBlock& MBB) {
  MachineInstr * LoopMI = nullptr;
  MachineInstr * CandMI = nullptr;
  MachineBasicBlock::iterator LI;
  MachineBasicBlock::const_instr_iterator MII;

  
  for (MachineBasicBlock::iterator I = MBB.end(), E = MBB.begin(); I != E;) {
    --I;
    MachineInstr &MI = *I;
    
    if (TPCII::isLoopInst(MI.getDesc()) && (MI.getOpcode() != TPC::LOOPEND)) {
      LLVM_DEBUG(dbgs() << "Filling Loop Delay Slot in BB#" << MBB.getNumber() << "\n");
      LoopMI = &*I;
      if (I != MBB.begin()) {
        --I;
        CandMI = &*I;
        if (isNopInstr(CandMI)) {
          CandMI = nullptr;
          break;
        }
        LI = I;
        LLVM_DEBUG(dbgs() << "     Candidate:\n"; printMI(CandMI));
      }
    }
  }
  if (!LoopMI) {
    return;
  }
  if (DisableDelaySlotUtilization) {
    CandMI = nullptr;
  }

  if (!CandMI) {
    goto fill_delay_slot;
  }
  

  // LOOP delay slot can't contain a JMP* or a LOOP* instructions
  //
  if (TPCII::isLoopInst(CandMI->getDesc()) || isJmpInstr(CandMI)) {
    CandMI = nullptr;
    goto fill_delay_slot;
  }

  MII = CandMI->getIterator();
  for (++MII; MII != MBB.instr_end() && MII->isInsideBundle(); ++MII) {
    // PRM 0.9992 LOOP delay slot isn't allowed to contain LD_G/LD_L to SPRF.
    //
    if (MF->getSubtarget<TPCSubtarget>().hasGoyaISA()) {
      if (MII->getOpcode() == TPC::LD_Lpsp ||
          MII->getOpcode() == TPC::LD_Lpip) {
        CandMI = nullptr;
        goto fill_delay_slot;
      }
      if (MII->getOpcode() == TPC::LD_Gpap) {
        CandMI = nullptr;
        goto fill_delay_slot;
      }
    }

    if (!CandMI) {
      goto fill_delay_slot;
    }
  }

  // Check that candidate does not define any reg used in LOOP inst
  //
  for (unsigned i = 0, e = CandMI->getNumOperands(); i != e; ++i) {
    const MachineOperand &MO = CandMI->getOperand(i);
    if (!MO.isReg()) continue;
    unsigned DR = MO.getReg();
    for (unsigned j = 0, e = LoopMI->getNumOperands(); j != e; ++j) {
      const MachineOperand &MO1 = LoopMI->getOperand(j);
      if (!MO1.isReg()) continue;
      unsigned UR = MO1.getReg();
      if (UR == DR ||
          TRI->isSubRegister(DR, UR) ||
          TRI->isSubRegister(UR, DR)) {
        LLVM_DEBUG(dbgs() << "     Candidate defines regs used in LOOP instr\n");
        CandMI = nullptr;
        goto fill_delay_slot;
      }
    }
  }

  if (resolveBlockLatency(MBB, true, true)) {
    CandMI = nullptr;
    goto fill_delay_slot;
  }
  if (resolveCrossBlockLatency(MBB, true, true, true, true)) {
    CandMI = nullptr;
    goto fill_delay_slot;
  }
  if (resolveCrossBlockLatency(MBB, false, true, true, true)) {
    CandMI = nullptr;
    goto fill_delay_slot;
  }

fill_delay_slot:
  if (!CandMI) {
    // Fill delay slot with a NOP
    //
    LLVM_DEBUG(dbgs() << "     NOP(" << 1 << ") after: "  << *LoopMI);
    LLVM_DEBUG(dbgs() << "        (LOOP delay slot)\n");
    TII->insertNoop(MBB, MBB.end());
  }
  else {
    // Moving the candidate into delay slot
    //
    LLVM_DEBUG(dbgs() << "     OK to use MI in delay slot:\n"; printMI(CandMI));
    MBB.splice(MBB.end(), &MBB, LI);
  }
}

void TPCLatencyResolver::insertDebugNops(MachineBasicBlock& MBB, int numPreNops) {
  MachineBasicBlock::iterator BegBB = MBB.begin();
  MachineBasicBlock::iterator EndBB = MBB.end();

  if (numPreNops == 0) {
    return;
  }
  LLVM_DEBUG(dbgs() << "Insert debug NOPs in BB#" << MBB.getNumber() << "\n");

  for (MachineBasicBlock::iterator I = BegBB, E = EndBB; I != E;) {
    MachineInstr &MI = *I;
    ++I;
    if (!MI.isPseudo() || TPCII::isLoopInst(MI.getDesc())) {
      for (int j = 0; j < numPreNops; ++j) {
        TII->insertNoop(MBB, MachineBasicBlock::iterator(MI));
      }
    }

    //
    // Make sure there are 3 NOPs after HALT instruction
    // PRM 0.9992: After HALT, 3 VLIW instructions which are NOPs should appear
    //
    if (isHaltInstr(&MI)) {
      TII->insertNoop(MBB, MBB.end());
      TII->insertNoop(MBB, MBB.end());
      TII->insertNoop(MBB, MBB.end());
    }
  }
}
