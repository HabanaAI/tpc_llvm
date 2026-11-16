//===- LoopTaken.cpp - Honor loop_taken pragma -------------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
//
// Pass to honor loop_taken pragma. Currently it generates assume intrinsic
// indicating a guranteed entry to the particular loop, this helps to keep
// the CFG simple for later passes like Unroll And Jam.
// In the future this pass can serve as the place to honor loop_taken in other
// ways too.
//===----------------------------------------------------------------------===//

#include "llvm/Transforms/Scalar/LoopTaken.h"
#include "llvm/ADT/PriorityWorklist.h"
#include "llvm/ADT/Statistic.h"
#include "llvm/Analysis/AssumptionCache.h"
#include "llvm/Analysis/LoopInfo.h"
#include "llvm/Analysis/ScalarEvolution.h"
#include "llvm/IR/BasicBlock.h"
#include "llvm/IR/CFG.h"
#include "llvm/IR/Function.h"
#include "llvm/IR/Instructions.h"
#include "llvm/IR/IntrinsicInst.h"
#include "llvm/IR/LLVMContext.h"
#include "llvm/IR/Module.h"
#include "llvm/Transforms/Scalar.h"
#include "llvm/Transforms/Scalar/LoopPassManager.h"
#include "llvm/Transforms/Utils.h"
#include "llvm/Transforms/Utils/Local.h"
#include "llvm/Transforms/Utils/LoopUtils.h"
#include <list>
using namespace llvm;

#define DEBUG_TYPE "looptaken"

static bool insertAssume(Loop *const L, ScalarEvolution &SE,
                         Instruction *InsertP) {
  LLVM_DEBUG(dbgs() << "insertAssume: \n");
  std::optional<Loop::LoopBounds> LB = L->getBounds(SE);
  PHINode *p = L->getInductionVariable(SE);
  if (LB && (p != nullptr)) {
    Value &Initial = LB->getInitialIVValue();
    Value &Final = LB->getFinalIVValue();

    llvm::CmpInst::Predicate Pred =
        (LB->getDirection() == Loop::LoopBounds::Direction::Increasing)
            ? ICmpInst::ICMP_SGT
            : ICmpInst::ICMP_SLT;

    Function *AssumeIntrinsic = Intrinsic::getOrInsertDeclaration(
        L->getHeader()->getModule(), Intrinsic::assume);
    ICmpInst *Cmp = new ICmpInst(Pred, &Final, &Initial);
    Cmp->insertBefore(InsertP);
    CallInst *CI = CallInst::Create(AssumeIntrinsic, {Cmp});
    CI->insertAfter(Cmp);
    LLVM_DEBUG(dbgs() << "builtin assume inserted succesfully!\n");
    return true;
  }
  LLVM_DEBUG(dbgs() << "builtin assume not inserted!\n");
  return false;
}

static bool processLoopTaken(Loop *const L, ScalarEvolution &SE,
                             Instruction *InsertP) {
  LLVM_DEBUG(dbgs() << "processLoopTaken: \n");
  bool Ret = false;
  BasicBlock *Latch = L->getLoopLatch();
  if (Latch == nullptr)
    return Ret;

  MDNode *LoopMD = Latch->getTerminator()->getMetadata(LLVMContext::MD_loop);

  if (LoopMD == nullptr)
    return Ret;

  MDNode *MD = nullptr;
  for (unsigned i = 1, e = LoopMD->getNumOperands(); i < e; ++i) {
    MD = dyn_cast<MDNode>(LoopMD->getOperand(i));
    if (!MD)
      continue;
    MDString *S = dyn_cast<MDString>(MD->getOperand(0));
    if (!S)
      continue;

    if (S->getString() == "llvm.loop.taken") {
      assert(MD->getNumOperands() == 2 &&
             "Taken hint metadata should have two operands.");

      if ((Ret = insertAssume(L, SE, InsertP))) {
        break;
      }
    }
  }
  return Ret;
}

// Returns true if an assume was inserted. \p SimplifiedLoops is set to true if
// simplifyLoop() changed the IR.
static bool runImpl(Function &F, LoopInfo &LI, ScalarEvolution &SE,
                    DominatorTree &DT, AssumptionCache &AC,
                    bool &SimplifiedLoops) {
  BasicBlock &BB = F.getEntryBlock();
  Instruction *InsertP = &(BB.back());
  for (auto &L : LI) {
    SimplifiedLoops |= simplifyLoop(L, &DT, &LI, &SE, &AC, nullptr, true);
  }
  bool Ret = false;
  SmallPriorityWorklist<Loop *, 4> Worklist;
  // Need to see if reverse(LI) is required, for now commented to resolve
  // conflict for llvm merge12
  // appendLoopsToWorklist(reverse(LI), Worklist);
  appendLoopsToWorklist(LI, Worklist);
  while (!Worklist.empty()) {
    Loop *L = Worklist.pop_back_val();
    // simplifyLoop() cannot give every loop a preheader, for example one
    // entered from an indirectbr.
    BasicBlock *Preheader = L->getLoopPreheader();
    if (!Preheader)
      continue;
    InsertP = &Preheader->back();
    Ret |= processLoopTaken(L, SE, InsertP);
  }
  return Ret;
}

PreservedAnalyses LoopTakenPass::run(Function &F, FunctionAnalysisManager &AM) {
  LLVM_DEBUG(dbgs() << "loop_taken runOnFunction.\n");
  if (F.isDeclaration())
    return PreservedAnalyses::all();
  auto &LI = AM.getResult<LoopAnalysis>(F);
  auto &SE = AM.getResult<ScalarEvolutionAnalysis>(F);
  auto &DT = AM.getResult<DominatorTreeAnalysis>(F);
  auto &AC = AM.getResult<AssumptionAnalysis>(F);

  bool SimplifiedLoops = false;
  bool Changed = runImpl(F, LI, SE, DT, AC, SimplifiedLoops);
  if (!Changed && !SimplifiedLoops)
    return PreservedAnalyses::all();
  return PreservedAnalyses::none();
}
