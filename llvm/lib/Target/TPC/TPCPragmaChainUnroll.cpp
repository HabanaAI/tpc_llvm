//===- TPCPragmaChainUnroll.cpp - In-order unroll for pragma loops --------===//
//
// `#pragma loop_unroll(N)` (llvm.loop.machine.unroll.count N) asks for the
// backend unroller, TPCSoftwareLoopPipelining, which gives each copy of the
// body its own version of every carried value. That is right for linear
// induction variables and coordinates, and it refuses a loop that carries
// anything else (a running max, a sum, a value kept from the previous
// iteration) with "machine_unroll(N) is not applied to the loop".
//
// Such a loop is unrolled here instead, by N and in order (copy k reads copy
// k-1's value), by LLVM's loop unroller: its pragma becomes
// llvm.loop.unroll.count N. Loops that carry only what the backend unroller
// takes keep their pragma for it.
//
// Habana's compiler refuses these loops as well, with the same warning, except
// where its code keeps the value in local memory: in GaudiInfer's
// gated_norm_quant_bf16 it unrolls the 8-head loop carrying a running max by
// 4, and the fork, which keeps the max in a register and did not unroll, took
// twice as long.
//
//===----------------------------------------------------------------------===//

#include "TPCTargetMachine.h"
#include "llvm/ADT/Statistic.h"
#include "llvm/Analysis/IVDescriptors.h"
#include "llvm/Analysis/LoopInfo.h"
#include "llvm/Analysis/ScalarEvolution.h"
#include "llvm/IR/Constants.h"
#include "llvm/IR/Instructions.h"
#include "llvm/IR/Metadata.h"
#include "llvm/Support/CommandLine.h"
#include "llvm/Support/Debug.h"
#include "llvm/Transforms/Utils/LoopUtils.h"

using namespace llvm;

#define DEBUG_TYPE "tpc-pragma-chain-unroll"

STATISTIC(NumLoops, "Pragma loops unrolled in order by the IR unroller");

static cl::opt<bool> EnablePragmaChainUnroll(
    "tpc-pragma-chain-unroll", cl::Hidden, cl::init(true),
    cl::desc("Unroll a loop_unroll pragma loop that carries a value other "
             "than an induction variable in order, in IR, where the backend "
             "unroller would refuse it"));

/// Whether the backend unroller takes this carried value: a 5-lane int
/// coordinate, an induction variable SCEV knows, or one stepped by a constant
/// (an f32 one too, as the backend unroller handles).
static bool isTakenByBackendUnroller(PHINode &Phi, Loop &L,
                                     ScalarEvolution &SE) {
  if (auto *VT = dyn_cast<FixedVectorType>(Phi.getType()))
    if (VT->getNumElements() == 5 && VT->getElementType()->isIntegerTy(32))
      return true;
  InductionDescriptor ID;
  if (InductionDescriptor::isInductionPHI(&Phi, &L, &SE, ID))
    return true;
  BasicBlock *Latch = L.getLoopLatch();
  if (!Latch)
    return false;
  auto *Next = dyn_cast<BinaryOperator>(Phi.getIncomingValueForBlock(Latch));
  if (!Next)
    return false;
  switch (Next->getOpcode()) {
  case Instruction::Add:
  case Instruction::FAdd:
    return (Next->getOperand(0) == &Phi && isa<Constant>(Next->getOperand(1))) ||
           (Next->getOperand(1) == &Phi && isa<Constant>(Next->getOperand(0)));
  case Instruction::Sub:
  case Instruction::FSub:
    return Next->getOperand(0) == &Phi && isa<Constant>(Next->getOperand(1));
  default:
    return false;
  }
}

PreservedAnalyses TPCPragmaChainUnrollPass::run(Loop &L, LoopAnalysisManager &,
                                                LoopStandardAnalysisResults &AR,
                                                LPMUpdater &) {
  if (!EnablePragmaChainUnroll || !L.isInnermost() || L.getNumBlocks() != 1)
    return PreservedAnalyses::all();
  MDNode *Hint = findOptionMDForLoop(&L, "llvm.loop.machine.unroll.count");
  if (!Hint || Hint->getNumOperands() != 2)
    return PreservedAnalyses::all();
  auto *CountC = mdconst::dyn_extract<ConstantInt>(Hint->getOperand(1));
  if (!CountC || CountC->getZExtValue() < 2)
    return PreservedAnalyses::all();
  const bool CarriesChain = any_of(L.getHeader()->phis(), [&](PHINode &Phi) {
    return !isTakenByBackendUnroller(Phi, L, AR.SE);
  });
  if (!CarriesChain)
    return PreservedAnalyses::all();

  LLVM_DEBUG(dbgs() << "Unroll in order by " << CountC->getZExtValue() << ": "
                    << L);
  LLVMContext &Ctx = L.getHeader()->getContext();
  SmallVector<Metadata *, 4> MDs = {nullptr};
  if (MDNode *LoopID = L.getLoopID())
    for (unsigned I = 1, E = LoopID->getNumOperands(); I < E; ++I) {
      Metadata *Op = LoopID->getOperand(I).get();
      if (Op == Hint)
        continue;
      MDs.push_back(Op);
    }
  MDs.push_back(MDNode::get(
      Ctx, {MDString::get(Ctx, "llvm.loop.unroll.count"),
            ConstantAsMetadata::get(ConstantInt::get(
                Type::getInt32Ty(Ctx), CountC->getZExtValue()))}));
  MDNode *NewLoopID = MDNode::getDistinct(Ctx, MDs);
  NewLoopID->replaceOperandWith(0, NewLoopID);
  L.setLoopID(NewLoopID);
  ++NumLoops;
  // Only the loop's metadata changed.
  return PreservedAnalyses::all();
}
