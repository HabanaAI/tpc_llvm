//===---------------------------- EliminateSwizzleCast.h ------------------===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//
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
