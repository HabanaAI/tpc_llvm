//===- TPC_LICM.cpp --- IR LICM--------------------------------------------===//
//
//
//                     The LLVM Compiler Infrastructure:
//
//              2020 - This pass is a property of Habana labs
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

#include "llvm/Transforms/Scalar/TPCLICM.h"
#include "llvm/Analysis/ScalarEvolution.h"
#include "llvm/IR/Constants.h"
#include "llvm/IR/Instructions.h"
#include "llvm/IR/IntrinsicInst.h"
#include "llvm/IR/IntrinsicsTPC.h"
#include "llvm/Support/CommandLine.h"
#include "llvm/Transforms/Scalar.h"
#include <vector>

using namespace llvm;


#define DEBUG_TYPE "tpc-licm"
static cl::opt<bool>
    TPCLICM("tpc-licm-pass",
                     cl::desc("tpc-licm pass"),
                     cl::init(true), cl::Hidden);

namespace {
class TPCLicmImpl {
public:
  TPCLicmImpl() {
    LicmIntrinsics.push_back(Intrinsic::tpc_lookup);
    LicmIntrinsics.push_back(Intrinsic::tpc_lookup_1c);
    LicmIntrinsics.push_back(Intrinsic::tpc_lookup_2c);
    LicmIntrinsics.push_back(Intrinsic::tpc_lookup_c0);
    LicmIntrinsics.push_back(Intrinsic::tpc_lookup_c1c2);
    sourceIntrinsics.push_back(Intrinsic::read_register);
  }
  bool run(Loop *Lp);
  void searchSource(Instruction *II, BasicBlock *BB,
                    std::vector<Instruction *> &chin);

private:
  std::vector<Intrinsic::ID> LicmIntrinsics;
  std::vector<Intrinsic::ID> sourceIntrinsics;
  IntrinsicInst *isLICMIntrinsic(Instruction *II);
};

} // namespace

/// @brief isLICMIntrinsic
/// \param II IR instruction
/// \return check if LicmIntrinsics contains II, if true return pointer to
///         IntrinsicInst else null.
IntrinsicInst *TPCLicmImpl::isLICMIntrinsic(Instruction *II) {
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
void TPCLicmImpl::searchSource(Instruction *II, BasicBlock *BB,
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
/// \return true if runOnLoop end with transformation or not.
bool TPCLicmImpl::run(Loop *Lp) {

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
        if (!inst)
          continue;
        std::vector<Instruction *> chin;
        searchSource(inst, BB, chin);
        if (chin.size() == 0)
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

PreservedAnalyses TPCLICMPass::run(Loop &L, LoopAnalysisManager &AM,
                                   LoopStandardAnalysisResults &AR,
                                   LPMUpdater &U) {
  if (!TPCLicmImpl().run(&L))
    return PreservedAnalyses::all();

  // Instructions were hoisted out of L, so the cached loop dispositions are
  // stale.
  AR.SE.forgetLoopDispositions();

  // Preserve the CFG (instructions are only moved).
  PreservedAnalyses PA = getLoopPassPreservedAnalyses();
  PA.preserveSet<CFGAnalyses>();
  return PA;
}
