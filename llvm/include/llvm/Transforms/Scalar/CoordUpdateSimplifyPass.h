//====------------------------ CoordUpdateSimplifyPass.h ------------------===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//
//
//===----------------------------------------------------------------------===//

#ifndef LLVM_TRANSFORMS_SCALAR_COORDUPDATESIMPLIFYPASS_H
#define LLVM_TRANSFORMS_SCALAR_COORDUPDATESIMPLIFYPASS_H

#include "llvm/Analysis/LoopAnalysisManager.h"
#include "llvm/IR/PassManager.h"

namespace llvm {

class CoordUpdateSimplifyPass : PassInfoMixin<CoordUpdateSimplifyPass> {
public:
  explicit CoordUpdateSimplifyPass() {}
};

} // end namespace llvm

#endif
