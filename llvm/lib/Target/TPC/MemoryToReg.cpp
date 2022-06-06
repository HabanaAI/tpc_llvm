//===- MemoryToReg.cpp --- Move values from memory to temporaries ---------===//
//
//===----------------------------------------------------------------------===//
//
// This is limited version of mem2reg pass. It is necessary for -O0 mode,
// as values that keep global pointers cannot be stored in memory.
//
//===----------------------------------------------------------------------===//

#include "TPCTargetMachine.h"
#include "llvm/Transforms/Utils/Mem2Reg.h"
#include "llvm/ADT/Statistic.h"
#include "llvm/Analysis/AssumptionCache.h"
#include "llvm/IR/BasicBlock.h"
#include "llvm/IR/Dominators.h"
#include "llvm/IR/Function.h"
#include "llvm/IR/Instructions.h"
#include "llvm/IR/PassManager.h"
#include "llvm/InitializePasses.h"
#include "llvm/Pass.h"
#include "llvm/Support/Casting.h"
#include "llvm/Transforms/Utils.h"
#include "llvm/Transforms/Utils/PromoteMemToReg.h"
#include <vector>

using namespace llvm;

#define PassName        "tpcmem2reg"
#define PassDescription "Promote memory to registers (TPC)"
#define DEBUG_TYPE      PassName

STATISTIC(NumPromoted, "Number of alloca's promoted (TPC)");

static bool shouldMoveToReg(AllocaInst *AI) {
  auto *APT = cast<PointerType>(AI->getType());
  Type *AT = APT->getPointerElementType();
  if (auto *PT = dyn_cast<PointerType>(AT))
    if (PT->getAddressSpace() == 3)
      return true;
  return false;
}

static bool promoteMemoryToRegister(Function &F, DominatorTree &DT,
                                    AssumptionCache &AC) {
  std::vector<AllocaInst *> Allocas;
  BasicBlock &BB = F.getEntryBlock(); // Get the entry node for the function
  bool Changed = false;

  while (true) {
    Allocas.clear();

    // Find allocas that are safe to promote, by looking at all instructions in
    // the entry node
    for (BasicBlock::iterator I = BB.begin(), E = --BB.end(); I != E; ++I)
      if (AllocaInst *AI = dyn_cast<AllocaInst>(I)) // Is it an alloca?
        if (isAllocaPromotable(AI) && shouldMoveToReg(AI))
          Allocas.push_back(AI);

    if (Allocas.empty())
      break;

    PromoteMemToReg(Allocas, DT, &AC);
    NumPromoted += Allocas.size();
    Changed = true;
  }
  return Changed;
}


namespace {
struct PromoteMemoryToRegs : public FunctionPass {
  // Pass identification, replacement for typeid
  static char ID;

  PromoteMemoryToRegs() : FunctionPass(ID) {
    initializePromoteMemoryToRegsPass(*PassRegistry::getPassRegistry());
  }

  // runOnFunction - To run this pass, first we calculate the alloca
  // instructions that are safe for promotion, then we promote each one.
  bool runOnFunction(Function &F) override {
    DominatorTree &DT = getAnalysis<DominatorTreeWrapperPass>().getDomTree();
    AssumptionCache &AC =
      getAnalysis<AssumptionCacheTracker>().getAssumptionCache(F);
    return promoteMemoryToRegister(F, DT, AC);
  }

  void getAnalysisUsage(AnalysisUsage &AU) const override {
    AU.addRequired<AssumptionCacheTracker>();
    AU.addRequired<DominatorTreeWrapperPass>();
    AU.setPreservesCFG();
  }
};

}

char PromoteMemoryToRegs::ID = 0;

INITIALIZE_PASS_BEGIN(PromoteMemoryToRegs, PassName, PassDescription,
                      false, false)
INITIALIZE_PASS_DEPENDENCY(AssumptionCacheTracker)
INITIALIZE_PASS_DEPENDENCY(DominatorTreeWrapperPass)
INITIALIZE_PASS_END(PromoteMemoryToRegs, PassName, PassDescription,
                    false, false)

  // createPromoteMemoryToRegister - Provide an entry point to create this pass.
FunctionPass *llvm::createPromoteMemoryToRegsPass() {
  return new PromoteMemoryToRegs();
}

