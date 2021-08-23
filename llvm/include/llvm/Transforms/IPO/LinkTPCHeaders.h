//===-- LinkTPCHeaders.h - Link TPC Headers ---------===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//
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

// List of header files.
#include "llvm/Transforms/IPO/TPCHeaders/Gaudi/CosF32.h"
#include "llvm/Transforms/IPO/TPCHeaders/Gaudi/ExpF32.h"
#include "llvm/Transforms/IPO/TPCHeaders/Gaudi/LogF32.h"
#include "llvm/Transforms/IPO/TPCHeaders/Gaudi/RSqrtF32.h"
#include "llvm/Transforms/IPO/TPCHeaders/Gaudi/ReciprocalF32.h"
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

#include "llvm/Transforms/IPO/TPCHeaders/Goya/ReciprocalF32.h"

namespace llvm {
struct LinkTPCHeadersPass : public PassInfoMixin<LinkTPCHeadersPass> {
  PreservedAnalyses run(Module &M, ModuleAnalysisManager &AM);
};

/// Create a legacy pass manager instance of a pass to link
/// TPC headers.
Pass *createLinkTPCHeadersLegacyPass();
} // namespace llvm

#endif // LLVM_TRANSFORMS_IPO_LINKTPCHEADERS_H
