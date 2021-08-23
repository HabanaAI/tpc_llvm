//===---- TPCMovCoalescer.cpp - Coalesce constant movs -----===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//
//
// This pass removes duplicate constant moves inside a basic block
//
//===----------------------------------------------------------------------===//


#include "TPCInstrInfo.h"
#include "TPCSubtarget.h"
#include "MCTargetDesc/TPCMCTargetDesc.h"
#include "llvm/ADT/SmallSet.h"
#include "llvm/ADT/SmallVector.h"
#include "llvm/ADT/Statistic.h"
#include "llvm/ADT/StringRef.h"
#include "llvm/CodeGen/MachineBasicBlock.h"
#include "llvm/CodeGen/MachineDominators.h"
#include "llvm/CodeGen/MachineFunction.h"
#include "llvm/CodeGen/MachineFunctionPass.h"
#include "llvm/CodeGen/MachineInstr.h"
#include "llvm/CodeGen/MachineInstrBuilder.h"
#include "llvm/CodeGen/MachineLoopInfo.h"
#include "llvm/CodeGen/MachineOperand.h"
#include "llvm/CodeGen/MachineRegisterInfo.h"
#include "llvm/IR/Constants.h"
#include "llvm/IR/DebugLoc.h"
#include "llvm/InitializePasses.h"
#include "llvm/Pass.h"
#include "llvm/Support/CommandLine.h"
#include "llvm/Support/Debug.h"
#include "llvm/Support/ErrorHandling.h"
#include "llvm/Support/MathExtras.h"
#include "llvm/Support/raw_ostream.h"
#include "llvm/Transforms/Utils/LoopSimplify.h"
#include "llvm/Transforms/Scalar.h"
#include "llvm/CodeGen/TargetRegisterInfo.h"
#include <cassert>
#include <cstdint>
#include <cstdlib>
#include <iterator>
#include <map>
#include <set>
#include <utility>
#include <vector>
using namespace llvm;

#define DEBUG_TYPE "tpc-mc"

static cl::opt<bool>
    EnableCoalescer("enable-mov-coalescer", cl::Hidden,
                 cl::desc("Enable move coalescer"), cl::init(true));

namespace llvm {
  FunctionPass *createTPCMovCoalescer();
  void initializeTPCMovCoalescerPass(PassRegistry&);
} // end namespace llvm


typedef std::map<unsigned, SmallVector<MachineInstr*, 4> > MovMap;

class TPCMovCoalescer : public MachineFunctionPass {
  MachineLoopInfo *MLI;
  MachineRegisterInfo* MRI;
  const TPCInstrInfo * TII;

  MovMap ConstMap[TPCII::OpType::Max + 1];

public:
  static char ID;
  TPCMovCoalescer() : MachineFunctionPass(ID), MLI(nullptr), TII(nullptr) {
    initializeTPCMovCoalescerPass(*PassRegistry::getPassRegistry());
  }
  bool runOnMachineFunction(MachineFunction &MF) override;
  bool findInnermostLoop(MachineLoop* L);
  bool coalesce(MachineLoop* L);

  void getAnalysisUsage(AnalysisUsage &AU) const override {
    AU.addRequired<MachineLoopInfo>();
    MachineFunctionPass::getAnalysisUsage(AU);
  }
};

INITIALIZE_PASS_BEGIN(TPCMovCoalescer, "movcoalescer",
                      "TPC Mov Coalescer", false, false)
INITIALIZE_PASS_DEPENDENCY(MachineLoopInfo)
INITIALIZE_PASS_END(TPCMovCoalescer, "movcoalescer",
                    "TPC Mov Coalescer", false, false)

FunctionPass *llvm::createTPCMovCoalescer() {
  return new TPCMovCoalescer();
}

char TPCMovCoalescer::ID = 0;

bool TPCMovCoalescer::runOnMachineFunction(MachineFunction &MF) {
  if (!EnableCoalescer) {
    return false;
  }
  MLI = &getAnalysis<MachineLoopInfo>();
  MRI = &MF.getRegInfo();
  TII = MF.getSubtarget<TPCSubtarget>().getInstrInfo();

  for (unsigned i = 0; i < TPCII::OpType::Max + 1; ++i) {
    ConstMap[i] = MovMap();
  }

  bool Changed = false;
  for (auto &L : *MLI) {
    Changed |= findInnermostLoop(L);
  }

  return Changed;
}

bool TPCMovCoalescer::findInnermostLoop(MachineLoop* L) {
  if (L->begin() == L->end()) {
    for (unsigned i = 0; i < TPCII::OpType::Max + 1; ++i) {
      ConstMap[i].clear();
    }
    return coalesce(L);
  }

  bool Changed = false;
  for (MachineLoop::iterator I = L->begin(), E = L->end(); I != E; ++I) {
    Changed |= findInnermostLoop(*I);
  }
  return Changed;
}

bool TPCMovCoalescer::coalesce(MachineLoop *L) {
  if (L->getHeader() != L->getLoopLatch()) {
    return false;
  }

  bool Changed = false;

  for (MachineInstr& MI : L->getHeader()->instrs()) {
    if (MI.getOpcode() == TPC::MOVvip || MI.getOpcode() == TPC::MOV_ld_vip) {
      unsigned Const = MI.getOperand(1).getImm();
      unsigned Type = MI.getOperand(2).getImm();
      ConstMap[Type][Const].push_back(&MI);
    }
  }

  unsigned RemovedInsts = 0;
  for (unsigned i = 0; i < TPCII::OpType::Max + 1; ++i) {
    for (auto ConstPair : ConstMap[i]) {
      if (ConstPair.second.size() > 1) {
        LLVM_DEBUG(RemovedInsts += ConstPair.second.size() - 1);
        unsigned SharedReg = ConstPair.second.front()->getOperand(0).getReg();
        for (unsigned j = 1; j < ConstPair.second.size(); ++j) {
          MRI->replaceRegWith(ConstPair.second[j]->getOperand(0).getReg(), SharedReg);
          ConstPair.second[j]->removeFromParent();
        }
        Changed = true;
      }
    }
  }
  LLVM_DEBUG(dbgs() << "Colescer removed " << RemovedInsts << " instructions\n");
  return Changed;
}
