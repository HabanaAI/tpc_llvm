#ifndef LLVM_TRANSFORMS_SCALAR_ITERATIONINTERLEAVE_H
#define LLVM_TRANSFORMS_SCALAR_ITERATIONINTERLEAVE_H

#include "llvm/IR/PassManager.h"

namespace llvm {

class Function;

class IterationInterleavePass : public PassInfoMixin<IterationInterleavePass> {
public:
  PreservedAnalyses run(Function &F, FunctionAnalysisManager &AM);
};

} // end namespace llvm

#endif // LLVM_TRANSFORMS_SCALAR_ITERATIONINTERLEAVE_H
