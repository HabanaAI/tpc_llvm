/*****************************************************************************
*  Copyright (C) 2021 HabanaLabs, Ltd.
*  All Rights Reserved.
*
*  Unauthorized copying of this file, via any medium is strictly prohibited.
*  Proprietary and confidential.
*
*  Authors:
*   Anand Kandomi     <akodnani@habana.ai>
*   Michael Zuckerman <mzuckerman@habana.ai>
****************************************************************************/
///
/// \file
/// Links TPC headers like cast_helpers, special function headers etc..
///
//===----------------------------------------------------------------------===//

#ifndef LLVM_TRANSFORMS_IPO_LINKTPCHEADERS_H
#define LLVM_TRANSFORMS_IPO_LINKTPCHEADERS_H

#include "llvm/ADT/StringRef.h"
#include "llvm/IR/Module.h"
#include "llvm/IR/PassManager.h"

// Lits of Gadui2 kernel
#ifndef _WIN32
#include "llvm/Transforms/IPO/TPCHeaders/Gadui2/gaudi2LTOBF16.h"
#include "llvm/Transforms/IPO/TPCHeaders/Gadui2/gaudi2LTO.h"
#endif
#include "llvm/Transforms/IPO/TPCHeaders/Gadui2/rsqrt_f16.h"
#include "llvm/Transforms/IPO/TPCHeaders/Gadui2/sqrt_f16.h"

// List of header files.
#include "llvm/Transforms/IPO/TPCHeaders/Gaudi/CosF32.h"
#include "llvm/Transforms/IPO/TPCHeaders/Gaudi/ExpF32.h"
#include "llvm/Transforms/IPO/TPCHeaders/Gaudi/LogF32.h"
#include "llvm/Transforms/IPO/TPCHeaders/Gaudi/RSqrtF32.h"
#ifndef _WIN32
#include "llvm/Transforms/IPO/TPCHeaders/Gaudi/ReciprocalF32.h"
#endif
#include "llvm/Transforms/IPO/TPCHeaders/Gaudi/SinF32.h"
#include "llvm/Transforms/IPO/TPCHeaders/Gaudi/SqrtF32.h"
#include "llvm/Transforms/IPO/TPCHeaders/Gaudi/TanhF32.h"

#include "llvm/Transforms/IPO/TPCHeaders/Gaudi/CosBF16.h"
#include "llvm/Transforms/IPO/TPCHeaders/Gaudi/ExpBF16.h"
#include "llvm/Transforms/IPO/TPCHeaders/Gaudi/LogBF16.h"
#include "llvm/Transforms/IPO/TPCHeaders/Gaudi/RSqrtBF16.h"
#include "llvm/Transforms/IPO/TPCHeaders/Gaudi/ReciprocalBF16.h"
#include "llvm/Transforms/IPO/TPCHeaders/Gaudi/SinBF16.h"
#include "llvm/Transforms/IPO/TPCHeaders/Gaudi/SqrtBF16.h"
#include "llvm/Transforms/IPO/TPCHeaders/Gaudi/TanhBF16.h"

// Reduction.
#include "llvm/Transforms/IPO/TPCHeaders/Gaudi/ReduceAddBF16.h"
#include "llvm/Transforms/IPO/TPCHeaders/Gaudi/ReduceAddF32.h"
#include "llvm/Transforms/IPO/TPCHeaders/Gaudi/ReduceAddI32.h"
#include "llvm/Transforms/IPO/TPCHeaders/Gaudi/ReduceMaxBF16.h"
#include "llvm/Transforms/IPO/TPCHeaders/Gaudi/ReduceMaxF32.h"
#include "llvm/Transforms/IPO/TPCHeaders/Gaudi/ReduceMaxI32.h"
#include "llvm/Transforms/IPO/TPCHeaders/Gaudi/ReduceMinBF16.h"
#include "llvm/Transforms/IPO/TPCHeaders/Gaudi/ReduceMinF32.h"
#include "llvm/Transforms/IPO/TPCHeaders/Gaudi/ReduceMulF32.h"
#include "llvm/Transforms/IPO/TPCHeaders/Gaudi/ReduceMaxI16.h"
#include "llvm/Transforms/IPO/TPCHeaders/Gaudi/ReduceMaxI8.h"
#include "llvm/Transforms/IPO/TPCHeaders/Gaudi/ReduceMaxU8.h"
#include "llvm/Transforms/IPO/TPCHeaders/Gaudi/ReduceMinI16.h"
#include "llvm/Transforms/IPO/TPCHeaders/Gaudi/ReduceMinI8.h"
#include "llvm/Transforms/IPO/TPCHeaders/Gaudi/ReduceMinU8.h"

#include "llvm/Transforms/IPO/TPCHeaders/Goya2/ReduceMaxF16.h"
#include "llvm/Transforms/IPO/TPCHeaders/Goya2/ReduceMinF16.h"

#ifndef _WIN32
#include "llvm/Transforms/IPO/TPCHeaders/Goya/ReciprocalF32.h"
#endif
#include "llvm/Transforms/IPO/TPCHeaders/Gaudi/TruncI32I8.h"
#include "llvm/Transforms/IPO/TPCHeaders/Gaudi/Swizzle2xF32.h"
#include "llvm/Transforms/IPO/TPCHeaders/Gaudi/Swizzle2xBF16.h"
#include "llvm/Transforms/IPO/TPCHeaders/Gaudi/Swizzle4xF32.h"

namespace llvm {
  typedef void* LibHandle;
  typedef void* FnHandle;

struct LinkTPCHeadersPass : public PassInfoMixin<LinkTPCHeadersPass> {
  enum class ArchTy {
    Goya = 1,
    Gaudi,
    GaudiB,
    Greco,
    Gaudi2,
    Doron1,
  };

  PreservedAnalyses run(Module &M, ModuleAnalysisManager &AM);
};

#ifndef _WIN32
LibHandle LoadLib(const char* path);
FnHandle GetLibFunction(LibHandle handle, const char* name);
#endif

/// Create a legacy pass manager instance of a pass to link
/// TPC headers.
Pass *createLinkTPCHeadersLegacyPass();
} // namespace llvm

#endif // LLVM_TRANSFORMS_IPO_LINKTPCHEADERS_H
