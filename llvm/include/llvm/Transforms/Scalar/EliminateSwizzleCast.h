//===---------------------------- EliminateSwizzleCast.h ------------------===//
//
//
//                     The LLVM Compiler Infrastructure:
//
//              2020 - This pass is a property of Habana labs
//
// Author : Md Shahid
// Email  : aashahid@habana.ai
//
//===----------------------------------------------------------------------===//

#ifndef LLVM_TRANSFORMS_SCALAR_CASTSWIZZLE_H
#define LLVM_TRANSFORMS_SCALAR_CASTSWIZZLE_H

#include "llvm/IR/PassManager.h"

namespace llvm {

class FunctionPass;

class EliminateSwizzleCastPass : PassInfoMixin<EliminateSwizzleCastPass> {
public:
  explicit EliminateSwizzleCastPass() {}
};

/// Create a legacy pass manager instance of the pass
FunctionPass *createEliminateSwizzleCastLegacyPass();

} // end namespace llvm

#endif
