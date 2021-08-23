//===- TPC_LICM.cpp --- IR LICM--------------------------------------------===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
// Author: Michael Zuckerman
//===----------------------------------------------------------------------===//
//   This pass implement a simple LICM for moving a list of known internists.
//   How does this pass work:
//   1) The TPC_LICM search for intrinsic from a list of intrinsics
//      (LicmIntrinsics).
//   2) If exist: find related instruction parameters.(searchSource)
//   3) Move all listed instruction to the first basic block
//===----------------------------------------------------------------------===//

#include "llvm/Analysis/LoopPass.h"
#include "llvm/IR/Constants.h"
#include "llvm/IR/Instructions.h"
#include "llvm/IR/IntrinsicInst.h"
#include "llvm/IR/IntrinsicsTPC.h"
#include "llvm/InitializePasses.h"
#include "llvm/Pass.h"
#include "llvm/Support/CommandLine.h"
#include "llvm/Transforms/Scalar.h"
#include <vector>

using namespace llvm;

static const char PassDescription[] = "TPC-LICM pass.";
static const char PassName[] = "tpc-licm";

#define DEBUG_TYPE "tpc-licm"
static cl::opt<bool>
    TPCLICM("tpc-licm-pass",
                     cl::desc("tpc-licm pass"),
                     cl::init(true), cl::Hidden);

namespace {
class TPCLicmPass : public LoopPass {
public:
  static char ID;
  StringRef getPassName() const override { return PassDescription; }
  TPCLicmPass() : LoopPass(ID) {
    initializeTPCLicmPassPass(*PassRegistry::getPassRegistry());
    LicmIntrinsics.push_back(Intrinsic::tpc_lookup);
    LicmIntrinsics.push_back(Intrinsic::tpc_lookup_1c);
    LicmIntrinsics.push_back(Intrinsic::tpc_lookup_2c);
    LicmIntrinsics.push_back(Intrinsic::tpc_lookup_c0);
    LicmIntrinsics.push_back(Intrinsic::tpc_lookup_c1c2);
    sourceIntrinsics.push_back(Intrinsic::read_register);
  }
  bool runOnLoop(Loop *Lp, LPPassManager &LPM) override;
  void getAnalysisUsage(AnalysisUsage &AU) const override {
    AU.setPreservesCFG();
    AU.addRequired<LoopInfoWrapperPass>();
  }
  void searchSource(Instruction *II, BasicBlock *BB,
                    std::vector<Instruction *> &chin);

private:
  std::vector<Intrinsic::ID> LicmIntrinsics;
  std::vector<Intrinsic::ID> sourceIntrinsics;
  IntrinsicInst *isLICMIntrinsic(Instruction *II);
};
} // namespace

INITIALIZE_PASS(TPCLicmPass, PassName, PassDescription, false, false)
char TPCLicmPass::ID = 0;
Pass *llvm::createTPCLICMPass() { return new TPCLicmPass(); }

/// @brief isLICMIntrinsic
/// \param II IR instruction
/// \return check if LicmIntrinsics contains II, if true return pointer to
///         IntrinsicInst else null.
IntrinsicInst *TPCLicmPass::isLICMIntrinsic(Instruction *II) {
  if (auto val = dyn_cast<IntrinsicInst>(II)) {
    for (auto element : LicmIntrinsics) {
      if (val->getIntrinsicID() == element)
        return val;
    }
  }
  return NULL;
}

/// The function search all instruction chin def to move together with the
/// first instruction.
/// \param II
/// \param BB the scope of check
/// \param chin a reference parameter contain a list of instruction to move.
void TPCLicmPass::searchSource(Instruction *II, BasicBlock *BB,
                               std::vector<Instruction *> &chin) {
  for (unsigned i = 0; i < II->getNumOperands(); i++) {
    if (auto intrin = dyn_cast<IntrinsicInst>(II)) {
      if (std::find(std::begin(sourceIntrinsics), std::end(sourceIntrinsics),
                    intrin->getIntrinsicID()) != std::end(sourceIntrinsics)) {
        chin.push_back(II);
        return;
      }
    } /// If instruction lead to previous Basic block exit
      else if (II->getParent() != BB) {
      return;
      /// If reach to a PHI node
    } else if (II->getOpcode() == Instruction::PHI) {
      return;
      /// Else continue to search
    } else if (auto sourceII = dyn_cast<Instruction>(II->getOperand(i))) {
      searchSource(sourceII, BB, chin);
      /// Only with return with relevant instruction push it to chin
      if (chin.size() > 0)
        chin.push_back(II);
    }
  }
  return;
}

///
/// \param Lp Current loop
/// \param LPM LPM pass manger
/// \return true if runOnLoop end with transformation or not.
bool TPCLicmPass::runOnLoop(Loop *Lp, LPPassManager &LPM) {

  if(!TPCLICM)
    return false;

  bool change = false;
  Loop *parentLoop = Lp->getParentLoop();
  BasicBlock *BB = NULL;
  std::vector<BasicBlock *> basicBlockVec = Lp->getBlocksVector();
  std::vector<Instruction *> movIns;

  /// Check on which basic block to work.
  if (parentLoop) {
    BB = parentLoop->getHeader();
  } else {
    BB = Lp->getLoopPredecessor();
  }

  /// If there is no upper basic block, exit.
  if (!BB)
    return false;
  /// Fins the last instruction and move the movIns before it
  auto BBp = BB->end();
  BBp--;

  for (auto BB : basicBlockVec) {
    for (BasicBlock::iterator II = BB->begin(); II != BB->end(); II++) {
      if (auto IntrinsicsTpc = isLICMIntrinsic(&*II)) {
        bool isConst = true;
        /// Check if all intrinsics parameters are constant
        for (unsigned i = 1; i < IntrinsicsTpc->getNumOperands(); i++) {
          isConst &=
              dyn_cast<llvm::Constant>(IntrinsicsTpc->getOperand(i)) != NULL;
        }
        if (!isConst)
          continue;
        /// Only the first operand can not be constant
        Instruction *inst =
            dyn_cast<llvm::Instruction>(IntrinsicsTpc->getOperand(0));
        std::vector<Instruction *> chin;
        searchSource(inst, BB, chin);
        if (chin.size() == 0 || !inst)
          continue;
        for (auto instInChin : chin) {
          if (std::find(std::begin(movIns), std::end(movIns), instInChin) ==
              std::end(movIns)) {
            movIns.push_back(instInChin);
          }
        }
        movIns.push_back(IntrinsicsTpc);
      }
    }
    /// Move all instruction above the last instruction in the previous basic
    /// block.
    for (auto IIMov : movIns) {
      IIMov->moveBefore(dyn_cast<Instruction>(&*BBp));
      change = true;
    }
    movIns.clear();
  }
  return change;
}