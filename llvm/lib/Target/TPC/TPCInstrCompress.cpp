//===- TPCInstrCompress.cpp ---- Instruction compression for TPC ---------------===//
//
//===----------------------------------------------------------------------===//
//
//===----------------------------------------------------------------------===//

#include "TPCInstrInfo.h"
#include "TPCSubtarget.h"
#include "MCTargetDesc/TPCMCTargetDesc.h"
#include "TPCMachineScheduler.h"
#include "TPCTargetMachine.h"
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

#define DEBUG_TYPE "tpcinstcompress"

static cl::opt<bool> TPCInstCompress("tpc-compress", cl::Hidden,
  cl::ZeroOrMore, cl::init(true),
  cl::desc("Enable compressed instruction format"));

static cl::opt<bool> TPCCompressJumps("tpc-compress-jumps", cl::Hidden,
  cl::ZeroOrMore, cl::init(false),
  cl::desc("Allow compression of jump destinations"));

namespace llvm {
  FunctionPass *createTPCInstrCompress();
  void initializeTPCInstrCompressPass(PassRegistry&);
}

namespace {

class TPCInstrCompress : public MachineFunctionPass {

public:
  static char ID;
  TPCInstrCompress() : MachineFunctionPass(ID) {
    MLI = nullptr;
    MF = nullptr;
    ItinData = nullptr;
    TII = nullptr;
    TRI = nullptr;
  };
  void getAnalysisUsage(AnalysisUsage &AU) const override;
  bool runOnMachineFunction(MachineFunction &Fn) override;
  void processBB(MachineBasicBlock& MBB);

private:
  MachineLoopInfo          * MLI;
  MachineFunction          * MF;
  const InstrItineraryData * ItinData;
  const TargetInstrInfo    * TII;
  const TargetRegisterInfo * TRI;
  const TPCSubtarget       * Subtarget;
  MachineInstr *prevMI;
};

}

char TPCInstrCompress::ID = 0;

INITIALIZE_PASS_BEGIN(TPCInstrCompress, "tpcinstcompress", "TPC Inst Compression", false, false)
  INITIALIZE_PASS_DEPENDENCY(MachineDominatorTree)
  INITIALIZE_PASS_DEPENDENCY(MachineBranchProbabilityInfo)
  INITIALIZE_PASS_DEPENDENCY(MachineLoopInfo)
  INITIALIZE_PASS_DEPENDENCY(MachineScheduler)
INITIALIZE_PASS_END(TPCInstrCompress, "tpcinstcompress", "TPC Inst Compression", false, false)


namespace llvm {
FunctionPass* createTPCInstrCompress() {
  return new TPCInstrCompress();
}
}

void TPCInstrCompress::getAnalysisUsage(AnalysisUsage &AU) const {
  AU.setPreservesCFG();
  AU.addRequired<MachineLoopInfo>();
  MachineFunctionPass::getAnalysisUsage(AU);
}

bool TPCInstrCompress::runOnMachineFunction(MachineFunction &Fn) {
  MF = &Fn;
  Subtarget = &MF->getSubtarget<TPCSubtarget>();
  TII = Subtarget->getInstrInfo();
  ItinData = Subtarget->getInstrItineraryData();
  TRI = Subtarget->getRegisterInfo();
  MLI = &getAnalysis<MachineLoopInfo>();
  TRI = Subtarget->getRegisterInfo();

  if (!TPCInstCompress ||
      !Subtarget->hasCompress() ||
      !Subtarget->getTargetLowering()->getTargetMachine().Options.CompressInstructions)
    return false;

  LLVM_DEBUG(dbgs() << "\n*** TPC Inst Compression\n");

  prevMI = nullptr;

  for (auto &MBB : Fn) {
    LLVM_DEBUG(dbgs() << "Processing BB#" << MBB.getNumber() << "\n");
    if (MBB.empty()) {
      continue;
    }
    processBB(MBB);
  }
  return true;
}

//
// isNopInstr: returns true if MI is a full NOP instruction
//
static bool isNopInstr(const MachineInstr *MI) {
  if (MI->isBundle()) {
    const MachineBasicBlock* MBB = MI->getParent();
    MachineBasicBlock::const_instr_iterator MII = MI->getIterator();
    for (++MII; MII != MBB->instr_end() && MII->isInsideBundle(); ++MII) {
      const MachineInstr &BMI = *MII;
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

static bool isJmpInstr(const MachineInstr &MI,
                       const MachineBasicBlock *&DestMBB) {
  if (MI.isBundle()) {
     const MachineBasicBlock* MBB = MI.getParent();
     MachineBasicBlock::const_instr_iterator MII = MI.getIterator();
      for (++MII; MII != MBB->instr_end() && MII->isInsideBundle(); ++MII) {
        const MachineInstr &BMI = *MII;
        if (isJmpInstr(BMI, DestMBB)) {
          return true;
        }
      }
      return false;
  } else {
    switch (MI.getOpcode()) {
      case TPC::JMPA:
      case TPC::JMPAr:
      case TPC::JMPR:
      case TPC::JMPR_u:
      case TPC::JMPRr:
        DestMBB = MI.getOperand(0).getMBB();
        assert(DestMBB);
        return true;
      default:
        return false;
    }
  }
}

static void rmOpcodeFromBundle(MachineInstr *MI, unsigned opcode) {
  const MachineBasicBlock* MBB = MI->getParent();
  MachineBasicBlock::instr_iterator MII = MI->getIterator();
  for (++MII; MII != MBB->instr_end() && MII->isInsideBundle(); ++MII) {
    MachineInstr& BMI = *MII;

    if (BMI.getOpcode() == opcode) {
      MII->eraseFromBundle();
      return;
    }
  }
}

static bool isVpuInstrWithSrcCD(const MachineInstr &MI) {
  const MCInstrDesc &MC = MI.getDesc();
  if (!TPCII::isVPUInst(MC))
    return false;
  if (TPCII::getHasSrcC(MC) || TPCII::getHasSrcD(MC))
    return true;

  return false;
}

static bool isSrcCIsStoreSrcC(const MachineInstr &MI) {
  const MCInstrDesc &MC = MI.getDesc();
  if (!TPCII::isVPUInst(MC))
    return false;
  if (TPCII::getSrcCIsStoreSrcC(MC))
    return true;

  return false;
}

static bool isVPUInstWithVPUExtSwitch(const MachineInstr &MI) {
  const MCInstrDesc &Desc = MI.getDesc();
  if (!TPCII::isVPUInst(Desc))
    return false;
  
  unsigned SlotOpcode = TPCII::getSlotOpCode(Desc);
  if (SlotOpcode == TPCII::vpuMAC ||
      SlotOpcode == TPCII::vpuMADD) {
    unsigned SwitchVal = getSwitches(MI);
    if (SwitchVal & TPCII::SW_ZP ||
        SwitchVal & TPCII::SW_NEG_ZP)
      return true;
  }
  
  return false;
}


static bool maybeCompressInstr(MachineInstr *MI, bool doCompress) {
  MachineBasicBlock* MBB = MI->getParent();
  MachineFunction &MF = *MBB->getParent();
  const TargetSubtargetInfo &STI = MF.getSubtarget();
  if (!MI->isBundle()) {
    if (isNopInstr(MI)) {
      if (doCompress) {
        // Create a bundle with NOPv & NOPs
        const TargetInstrInfo *TII = STI.getInstrInfo();
        MIBundleBuilder Bundler(*MBB,  MI);
        Bundler.append(BuildMI(MF, DebugLoc(), TII->get(TPC::NOPld)));
        Bundler.append(BuildMI(MF, DebugLoc(), TII->get(TPC::NOPst)));
        finalizeBundle(*MBB, Bundler.begin());
        MI->eraseFromParent();
      }
      return true;
    }
    return false;
  }
  bool hasVPU = false;
  bool hasSPU = false;
  bool hasLD = false;
  bool hasST = false;
  MachineBasicBlock::const_instr_iterator MII = MI->getIterator();
  for (++MII; MII != MBB->instr_end() && MII->isInsideBundle(); ++MII) {
    const MachineInstr& BMI = *MII;

    // Check for cross-slot instructions
    if (isVpuInstrWithSrcCD(BMI))
      return false;
    if (isSrcCIsStoreSrcC(BMI))
      return false;
    if (STI.hasFeature(TPC::FeatureDoron1) && isVPUInstWithVPUExtSwitch(BMI))
      return false;

    // Instructions that can be compressed (2 instructions in a single VLIW):
    // Instructions which are not JMPR/JMPA
    if (BMI.isTerminator()) {
      return false;
    }

    if (!isNopInstr(&BMI)) {
      if (TPCII::isVPUInst(BMI.getDesc()))
        hasVPU = true;
      else if (TPCII::isSPUInst(BMI.getDesc()))
        hasSPU = true;
      else if (TPCII::isLoadInst(BMI.getDesc()))
        hasLD = true;
      else if (TPCII::isStoreInst(BMI.getDesc()))
        hasST = true;
    }
  }
  if ((hasVPU || hasSPU) && (hasLD || hasST)) {
      // Cannot compress
      return false;
  }
  
  //
  // TODO: check for other extra bits?
  //


  //
  // Now we know that the bundle instruction is compressible, so we remove
  // two NOPs from this bundle leaving only two other instructions there.
  // The code emitter will know to compress a bundle with only two instructions.
  //
  if (doCompress) {
    if (hasVPU || hasSPU) {
      rmOpcodeFromBundle(MI, TPC::NOPld);
      rmOpcodeFromBundle(MI, TPC::NOPst);
    }
    else {
      rmOpcodeFromBundle(MI, TPC::NOPs);
      rmOpcodeFromBundle(MI, TPC::NOPv);
    }
  }
  
  return true;
}

void TPCInstrCompress::processBB(MachineBasicBlock& MBB) {
  MachineBasicBlock::iterator BegBB = MBB.begin();
  MachineBasicBlock::iterator EndBB = MBB.end();

  bool IsDestination = false;
  const MachineBasicBlock *FirstMBB =
    llvm::GraphTraits<MachineFunction *>::getEntryNode(MF);
  bool IsFirstBB = MBB.getNumber() == FirstMBB->getNumber();
  
  auto PredBBs = MBB.predecessors();
  for (MachineBasicBlock::const_pred_iterator PredBB = PredBBs.begin();
       (PredBB != PredBBs.end()) && !IsDestination; ++PredBB) {
    // These functions doesn't work correctly. I don't know why.
    // MachineBasicBlock::getFirstTerminator()
    // MachineBasicBlock::getFirstInstrTerminator()
    // Because look over whole block.
    for (MachineBasicBlock::reverse_iterator Inst = (*PredBB)->rbegin();
         Inst != (*PredBB)->rend() && !IsDestination; ++Inst) {
      const MachineBasicBlock *DestMBB = nullptr;
      if(isJmpInstr(*Inst, DestMBB) &&
         MBB.getNumber() == DestMBB->getNumber())
        IsDestination = true;
    }
  }

  if (!TPCCompressJumps) {
    prevMI = nullptr;
  }

  bool IsDelaySlot = false;
  for (MachineBasicBlock::iterator I = BegBB, E = EndBB; I != E; ) {
    MachineInstr &MI = *I;
    ++I;

    if(MI.isDebugInstr())
      continue;

    // Instructions that can be compressed (2 instructions in a single VLIW):
    // Instructions which are not having index EndLoop, EndLoop-1, EndLoop-2, EndLoop-3
    // (where EndLoop is the index of the last instruction of the LOOP)
    bool isLoopEndN = false;
    if (I != E) {
      int num = 0;
      for (MachineBasicBlock::iterator X = I; X != E; ++X) {
        if(X != E) {
          if((*X).getOpcode() == TPC::LOOPEND) {
            isLoopEndN = true;
            break;
          }
          if (!(*X).isDebugInstr()) {
            num++;
            if (num >= 4)
              break;
          }
        }
      }
    }
    
    const MachineBasicBlock *DestMBB = nullptr;
    if (isJmpInstr(MI, DestMBB)) {
      if (Subtarget->hasDoron1()) {
        prevMI = nullptr;
        IsDelaySlot = true;
        continue;
      }
    }
    
    if (IsDelaySlot) {
      IsDelaySlot = false;
      continue;
    }

    //Instructions that can be compressed (2 instructions in a single VLIW):
    //  - Instructions which are not the first instruction of the kernel.
    bool IsFirstInst = IsFirstBB && &MI == &(*BegBB);
    if (maybeCompressInstr(&MI, false) && !isLoopEndN &&
        !IsDestination && !(IsFirstInst && Subtarget->hasDoron1())) {
      if (prevMI) {
        // Compress both, MI and prevMI
        maybeCompressInstr(prevMI, true);
        maybeCompressInstr(&MI, true);
        prevMI = nullptr;
      }
      else {
        prevMI = &MI;
      }
    } else {
      IsDestination = false;
      prevMI = nullptr;
    }
  }
}
