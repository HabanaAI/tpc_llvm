//===---------------------------- LoopSWPPass.h ---------------------------===//
//
//
//                     The LLVM Compiler Infrastructure:
//
//              2020 - This pass is a property of Habana labs
//
//
//===----------------------------------------------------------------------===//

#ifndef LLVM_TRANSFORMS_SCALAR_LOOPSWPPASS_H
#define LLVM_TRANSFORMS_SCALAR_LOOPSWPPASS_H

#include "llvm/Analysis/LoopAnalysisManager.h"
#include "llvm/IR/PassManager.h"

namespace llvm {

class LoopSWPPass : PassInfoMixin<LoopSWPPass> {
public:
  explicit LoopSWPPass() {}
};

} // end namespace llvm

#endif
