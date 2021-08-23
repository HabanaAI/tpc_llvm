//===- TPCAliasAnalysis ------------------------------------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
/// \file
/// This is the TPC address space based alias analysis pass.
//===----------------------------------------------------------------------===//

#include "TPCAliasAnalysis.h"
#include "TPC.h"
#include "llvm/ADT/Triple.h"
#include "llvm/Analysis/AliasAnalysis.h"
#include "llvm/Analysis/MemoryLocation.h"
#include "llvm/Analysis/ValueTracking.h"
#include "llvm/IR/Argument.h"
#include "llvm/IR/Attributes.h"
#include "llvm/IR/CallingConv.h"
#include "llvm/IR/Function.h"
#include "llvm/IR/GlobalVariable.h"
#include "llvm/IR/Type.h"
#include "llvm/IR/Value.h"
#include "llvm/Pass.h"
#include "llvm/Support/Casting.h"
#include "llvm/Support/ErrorHandling.h"
#include <cassert>

using namespace llvm;

#define DEBUG_TYPE "tpc-aa"

// Register this pass...
char TPCAAWrapperPass::ID = 0;

INITIALIZE_PASS(TPCAAWrapperPass, "TPC-aa",
                "TPC Address space based Alias Analysis", false, true)

ImmutablePass *llvm::createTPCAAWrapperPass() {
  return new TPCAAWrapperPass();
}

void TPCAAWrapperPass::getAnalysisUsage(AnalysisUsage &AU) const {
  AU.setPreservesAll();
}

AliasResult TPCAAResult::getAliasResult(unsigned AS1,
                                        unsigned AS2) const {
  if (AS1 == 0 || AS2 == 0) {
    // unknown addrspace
    return MayAlias;
  }
  return AS1 == AS2 ? MayAlias : NoAlias;
}

AliasResult TPCAAResult::alias(const MemoryLocation &LocA,
                               const MemoryLocation &LocB, AAQueryInfo &AAQI) {
  unsigned asA = LocA.Ptr->getType()->getPointerAddressSpace();
  unsigned asB = LocB.Ptr->getType()->getPointerAddressSpace();

  AliasResult Result = getAliasResult(asA, asB);
  if (Result == NoAlias) return Result;
  if (asA != asB && asA != 0 && asB != 0) return NoAlias;

  // Forward the query to the next alias analysis.
  return AAResultBase::alias(LocA, LocB, AAQI);
}

bool TPCAAResult::pointsToConstantMemory(const MemoryLocation &Loc,
                                         AAQueryInfo &QInfo, bool OrLocal) {
  return AAResultBase::pointsToConstantMemory(Loc, QInfo, OrLocal);
}
