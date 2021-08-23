//===- TPCInstrCompress.cpp ---- Instruction compression for TPC ---------------===//
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
  TII = Fn.getSubtarget().getInstrInfo();
  ItinData = Fn.getSubtarget().getInstrItineraryData();
  TRI = Fn.getSubtarget().getRegisterInfo();
  MLI = &getAnalysis<MachineLoopInfo>();
  TRI = Fn.getSubtarget().getRegisterInfo();

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


static bool maybeCompressInstr(MachineInstr *MI, bool doCompress) {
  if (!MI->isBundle()) {
    if (isNopInstr(MI)) {
      if (doCompress) {
        // Create a bundle with NOPv & NOPs
        MachineBasicBlock* MBB = MI->getParent();
        MachineFunction &MF = *MBB->getParent();
        const TargetSubtargetInfo &STI = MF.getSubtarget();
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
  const MachineBasicBlock* MBB = MI->getParent();
  MachineBasicBlock::const_instr_iterator MII = MI->getIterator();
  for (++MII; MII != MBB->instr_end() && MII->isInsideBundle(); ++MII) {
    const MachineInstr& BMI = *MII;

    // Check for cross-slot instructions
    // Do not use TPCMCInstrInfo interface function (commented out below) for now
    // because it does not check for all possible opcodes (the list of the opcodes
    // is huge currently because we still have to support old formats)
    // if (TPCMCInstrInfo::isVpuInstrWithSrcCD(BMI.getOpcode())) {
    if (isVpuInstrWithSrcCD(BMI))
      return false;
    if (isSrcCIsStoreSrcC(BMI))
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

  auto PredBBs = MBB.predecessors();
  for (MachineBasicBlock::const_pred_iterator PredBB = PredBBs.begin();
       (PredBB != PredBBs.end()) && !IsDestination; ++PredBB) {
    if ((*PredBB)->getFirstTerminator() != (*PredBB)->end())
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

    if (maybeCompressInstr(&MI, false) && !isLoopEndN && !IsDestination) {
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
