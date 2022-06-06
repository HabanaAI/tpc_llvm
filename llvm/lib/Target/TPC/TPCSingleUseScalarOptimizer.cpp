//===---- TPCSingleUseScalarOptimizer.cpp ----------------------------------------===//
//
//===----------------------------------------------------------------------===//
//
//
//===----------------------------------------------------------------------===//

#include "llvm/Analysis/TargetLibraryInfo.h"
#include "llvm/IR/Instructions.h"
#include "llvm/IR/IntrinsicInst.h"
#include "llvm/IR/IntrinsicsTPC.h"
#include "llvm/IR/IRBuilder.h"
#include "llvm/IR/PatternMatch.h"
#include "llvm/Analysis/VectorUtils.h"
#include "llvm/Pass.h"
#include "llvm/Transforms/Utils/LowerMemIntrinsics.h"
#include "TPCTargetMachine.h"
using namespace llvm;

namespace llvm {
FunctionPass *createTPCSingleUseScalarOptimizer();
void initializeTPCSingleUseScalarOptimizerPassPass(PassRegistry&);
}

static const char PassDescription[] = "TPC single scalar preshaper";
static const char PassName[] = "tpc-single-use-scalar-opt";

#define DEBUG_TYPE "tpc-singleuse-scalaropt"

namespace {
class TPCSingleUseScalarOptimizerPass : public FunctionPass {
public:
  static char ID;

  StringRef getPassName() const override { return PassDescription; }

  TPCSingleUseScalarOptimizerPass() : FunctionPass(ID) {
    initializeTPCSingleUseScalarOptimizerPassPass(*PassRegistry::getPassRegistry());
  }

  bool runOnFunction(Function &F) override;
};
}

char TPCSingleUseScalarOptimizerPass::ID = 0;

INITIALIZE_PASS_BEGIN(TPCSingleUseScalarOptimizerPass, PassName, PassDescription, false, false)
INITIALIZE_PASS_END(TPCSingleUseScalarOptimizerPass, PassName, PassDescription, false, false)


FunctionPass *llvm::createTPCSingleUseScalarOptimizer() {
  return new TPCSingleUseScalarOptimizerPass();
}

bool TPCSingleUseScalarOptimizerPass::runOnFunction(Function &Func) {
  if (skipFunction(Func))
    return false;

  LLVM_DEBUG(dbgs() << "**** SingleUseScalarOptimizer Pass\n");
  bool Changed = false;

  for (auto &BB : Func) {
    for (auto &I : BB) {
      if (const IntrinsicInst* intrins = dyn_cast<IntrinsicInst>(&I)) {
        Intrinsic::ID inid = intrins->getIntrinsicID();
        if (inid == Intrinsic::tpc_convert_int) {
          if (I.hasOneUse()) {
            Instruction *C = cast<Instruction>(*I.user_begin());
            if (auto *inse = dyn_cast<InsertElementInst>(C)) {
              if (inse->hasOneUse()) {
                Instruction *D = cast<Instruction>(*C->user_begin());
                if (auto *shuffl = dyn_cast<ShuffleVectorInst>(D)) {
                  if (shuffl->hasOneUse()) {
                    for (auto &U : D->uses()) {
                      Instruction *UI = cast<Instruction>(U.getUser());
                      if (IntrinsicInst* intrins1 = dyn_cast<IntrinsicInst>(UI)) {
                        Intrinsic::ID inid1 = intrins1->getIntrinsicID();
                        if (inid1 == Intrinsic::tpc_mac &&
                            UI->getParent() != &BB) {
                          C->moveBefore(UI);
                          D->moveBefore(UI);
                          Changed = true;
                        }
                      }
                    }
                  }
                }
              }
            }
          }
        }
      }
    }
  }
  return Changed;
}
