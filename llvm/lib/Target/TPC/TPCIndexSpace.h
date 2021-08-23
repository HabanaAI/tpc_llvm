//===- TPCIndexSpace.cpp --- TPC INDEX SPACE ------------------------------===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//
//===----------------------------------------------------------------------===//
#ifndef LLVM_TPCINDEXSPACE_CPP_H
#define LLVM_TPCINDEXSPACE_CPP_H

#include "SCEVParser.h"
#include "TPCTargetMachine.h"
#include "llvm/Analysis/AssumptionCache.h"
#include "llvm/Analysis/LoopInfo.h"
#include "llvm/Analysis/MemoryBuiltins.h"
#include "llvm/Analysis/ScalarEvolution.h"
#include "llvm/Analysis/ScalarEvolutionExpander.h"
#include "llvm/Analysis/ScalarEvolutionExpressions.h"
#include "llvm/IR/Function.h"
#include "llvm/IR/InstIterator.h"
#include "llvm/Transforms/Utils/LoopUtils.h"
#include "llvm/Transforms/Utils/UnrollLoop.h"
#include "TPCLoopData.h"

using namespace llvm;

#define PassName "TPCIndexMap"
#define PassDescription "Create pipeline optimization in the level of the IR"
#define DEBUG_TYPE PassName
#include <iostream>

using namespace std;

class TPCIndexMap : public FunctionPass {
  // Pass identification, replacement for typeid
public:
  static char ID;

  TPCIndexMap() : FunctionPass(ID) {
    initializeTPCIndexMapPass(*PassRegistry::getPassRegistry());
  }

  void getAnalysisUsage(AnalysisUsage &AU) const override {
    AU.setPreservesCFG();
    AU.addPreserved<ScalarEvolutionWrapperPass>();
    getLoopAnalysisUsage(AU);
  }

private:
  vector<Instruction *> m_load;
  vector<Instruction *> m_store;
  vector<LoopData> m_loopInfoVec;
  vector<Loop *> m_loopHeader;
  ScalarEvolution *p_SE;
  Function *p_func;
  LoopInfo *p_LI;
  bool runOnFunction(Function &F) override;
  void collectDataLoop(Function &F, ScalarEvolution *SE, LoopInfo *LI);
  void sort();
  void print_data();
};




#endif //LLVM_TPCINDEXSPACE_CPP_H