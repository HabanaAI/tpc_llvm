// ===------------------ CoordUpdateSimplifyPass.h ------------------===//
//
//
//                     The LLVM Compiler Infrastructure:
//
//              2020 - This pass is a property of Habana labs
//
//
// ===------------------ CoordUpdateSimplifyPass.h ------------------===//

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
