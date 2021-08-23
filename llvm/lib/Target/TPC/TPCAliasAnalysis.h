//===- TPCAliasAnalysis --------------------------------------*- C++ -*-===//
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

#ifndef LLVM_LIB_TARGET_TPC_TPCALIASANALYSIS_H
#define LLVM_LIB_TARGET_TPC_TPCALIASANALYSIS_H

#include "TPC.h"
#include "llvm/ADT/Triple.h"
#include "llvm/Analysis/AliasAnalysis.h"
#include "llvm/IR/Function.h"
#include "llvm/IR/Module.h"
#include "llvm/Pass.h"
#include "llvm/Support/TargetRegistry.h"
#include "llvm/CodeGen/Passes.h"
#include "llvm/CodeGen/TargetPassConfig.h"
#include "llvm/IR/LegacyPassManager.h"
#include <algorithm>
#include <memory>

namespace llvm {

ImmutablePass  * createTPCAAWrapperPass();
void initializeTPCAAWrapperPassPass(PassRegistry&);


class DataLayout;
class MDNode;
class MemoryLocation;

/// A simple AA result that uses TBAA metadata to answer queries.
class TPCAAResult : public AAResultBase<TPCAAResult> {
  friend AAResultBase<TPCAAResult>;

public:
  explicit TPCAAResult() : AAResultBase() {}
  TPCAAResult(TPCAAResult &&Arg)
      : AAResultBase(std::move(Arg)){}

  bool invalidate(Function &, const PreservedAnalyses &) { return false; }

  AliasResult alias(const MemoryLocation &LocA, const MemoryLocation &LocB, AAQueryInfo &AAQI);
  bool pointsToConstantMemory(const MemoryLocation &Loc, AAQueryInfo &QInfo, bool OrLocal);

private:
  AliasResult getAliasResult(unsigned AS1, unsigned AS2) const;
};

/// Analysis pass providing a never-invalidated alias analysis result.
class TPCAA : public AnalysisInfoMixin<TPCAA> {
  friend AnalysisInfoMixin<TPCAA>;

  static char PassID;

public:
  using Result = TPCAAResult;

  TPCAAResult run(Function &F, AnalysisManager<Function> &AM) {
    return TPCAAResult();
  }
};

/// Legacy wrapper pass to provide the TPCAAResult object.
class TPCAAWrapperPass : public ImmutablePass {
  std::unique_ptr<TPCAAResult> Result;

public:
  static char ID;

  TPCAAWrapperPass() : ImmutablePass(ID) {
    initializeTPCAAWrapperPassPass(*PassRegistry::getPassRegistry());
  }

  TPCAAResult &getResult() { return *Result; }
  const TPCAAResult &getResult() const { return *Result; }

  bool doInitialization(Module &M) override {
    Result.reset(new TPCAAResult());
    return false;
  }

  bool doFinalization(Module &M) override {
    Result.reset();
    return false;
  }

  void getAnalysisUsage(AnalysisUsage &AU) const override;
};

} // end namespace llvm

#endif // LLVM_LIB_TARGET_TPC_TPCALIASANALYSIS_H
