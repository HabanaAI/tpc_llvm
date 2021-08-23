//===- TPCLoopData.h ------------------------------------------------------===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//
//                                 LoopData
// LoopData extract inforamtion on the Loop by asking SCEV on the Loop SCALAR. 
//===----------------------------------------------------------------------===//

#ifndef LLVM_TPCLOOPDATA_CPP_H
#define LLVM_TPCLOOPDATA_CPP_H
#include "TPCTargetMachine.h"
#include "SCEVParser.h"
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


class LoopData {
public:
  LoopData(){};
  LoopData(Loop *L, ScalarEvolution *SE, bool costModel = false);
  std::pair<Instruction *, Instruction *> getCmpValue();
  unsigned get_STEP() { return m_STEP; }
  unsigned get_DIM() { return m_DIM; }
  const SCEV *getLoopSCEV() { return p_LoopSCEV; }
  bool getSCEVStatus() { return m_SCEVNotValid; }
  bool is_Valid() { return m_Valid; }

private:
  Module *p_MD;
  Loop *p_LH;
  Loop *p_Prev;
  BasicBlock *p_Nested;
  BasicBlock *p_Latch;
  ScalarEvolution *p_SEL;
  Instruction *p_Inducation = nullptr;
  unsigned m_STEP = 0;
  unsigned m_DIM = 0;
  bool m_Valid = false;
  bool m_SCEVNotValid = true;
  unsigned m_backendUnroll = 1;
  const SCEV *p_LoopSCEV;

  const SCEV *tryFindSCEV();
  void findNumberOfIterations(BasicBlock *BB);
  std::pair<IntrinsicInst *, IntrinsicInst *> findOffsetAndSizeIntrinsics();
  const SCEV *relaxSCEV(const SCEV *EV, vector<const Instruction *> index,
                        string name);
};

#endif // LLVM_TPCLOOPDATA_CPP_H
