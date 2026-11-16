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
#include "llvm/Transforms/Scalar/LoopPassManager.h"

namespace llvm {

class Loop;

/// Replaces the insertelement-based coord update of an innermost single-block
/// loop with a tpc.add.mask increment of the coord.
class CoordUpdateSimplifyPass : public PassInfoMixin<CoordUpdateSimplifyPass> {
public:
  PreservedAnalyses run(Loop &L, LoopAnalysisManager &AM,
                        LoopStandardAnalysisResults &AR, LPMUpdater &U);
  // Like the legacy pass, which never calls skipLoop(), run on optnone
  // functions too.
  static bool isRequired() { return true; }
};

} // end namespace llvm

#endif
