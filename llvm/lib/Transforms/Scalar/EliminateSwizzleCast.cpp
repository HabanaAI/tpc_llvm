//===---------------------------- EliminateSwizzleCast.cpp ----------------===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//
// This file implements a pass to replace "fpext/fptrunc" or
// 'sitofp/fptosi' or 'uitofp/fptoui' having post dominator relationship with
// corresponding *tpc.convert* intrinsics without swizzling.
//===----------------------------------------------------------------------===//

#include "llvm/Transforms/Scalar/EliminateSwizzleCast.h"
#include "llvm/Analysis/DDG.h"
#include "llvm/IR/CallSite.h"
#include "llvm/IR/Function.h"
#include "llvm/IR/Instructions.h"
#include "llvm/IR/IntrinsicInst.h"
#include "llvm/IR/IntrinsicsTPC.h"
#include "llvm/IR/PassManager.h"
#include "llvm/InitializePasses.h"
#include "llvm/Pass.h"
#include "llvm/Passes/PassBuilder.h"
#include "llvm/Support/Debug.h"
#include "llvm/Transforms/Scalar.h"
#include "llvm/Transforms/Utils/TPCIntrinsicUtils.h"
#include <unordered_map>

#define DEBUG_TYPE "cast-swizzle-opt"
#include <iostream>

using namespace llvm;

namespace {

using NodeSetType = SetVector<const DDGNode *>;
using NodeSetVectorMap = DenseMap<const DDGNode *, NodeSetType>;
using NodeToBoolMapType = DenseMap<const DDGNode *, bool>;
using InstPairType = std::pair<Instruction *, Instruction *>;

std::unordered_multimap<unsigned, unsigned>
    marker({{Instruction::FPTrunc, Instruction::FPExt},
            {Instruction::FPExt, Instruction::FPTrunc},
            {Instruction::SIToFP, Instruction::FPToSI},
            {Instruction::FPToSI, Instruction::SIToFP},
            {Instruction::UIToFP, Instruction::FPToUI},
            {Instruction::FPToUI, Instruction::UIToFP},
            {Instruction::FPTrunc, Instruction::Call}});
class EliminateSwizzleCast {

public:
  // Entry point for analysis and transformation.
  void processDDG(const DataDependenceGraph &G);
  bool CheckLegal(unsigned int PrevStratCode, unsigned int PrevEndCode,
                  unsigned int StartCode, unsigned int EndCode);

private:
  unsigned int StartOpCode;
  unsigned int EndOpCode;
  NodeSetVectorMap PostDomSet;
  NodeSetVectorMap FinalPostDomSet;
  std::vector<NodeSetType> ForwardPaths;
  std::vector<NodeSetType> ReversePaths;
  NodeToBoolMapType NodeVisitedMap;

  // 'EndOpCode' setter in case where multiple ending operation is possible for
  // a given start operation.
  void setEndOpCode(unsigned OpCode) {
    EndOpCode = OpCode;
  }
  // Compute all forward paths forllowing Def-Use chain having correlative casts
  // 'fpext/fptrunc' or 'sitofp/fptosi' or 'uitofp/fptoui'.
  void computeAllFWPath(const DataDependenceGraph &G);

  // Computes all backward paths forllowing Use-Def chain having correlative
  // casts 'fpext/fptrunc' or 'sitofp/fptosi' or 'uitofp/fptoui'. \p
  // IsStorePackSpl is to indicate trivial store-pack pattern.
  void computeAllBWPath(const DataDependenceGraph &G,
                        bool IsStorePackSpl = false);

  // Recursively follow Def-Use or Use-Def edges starting from node \p Src and
  // ending at node containing instruction with path ending operation \p
  // EndOpCode.\p IsBackward is to chose direction of traversal.
  void dfsAllPath(const DDGNode *Src, unsigned int StartOpCode,
                  unsigned int EndOpCode, NodeSetType &Path,
                  bool BackwardSearch = false);

  bool CheckFinalList(const Instruction *I);
  // Look through the computed post dominator map to check if the intended
  // tranformation is legal.The transformation is legal only if all backward
  // path contains cast and which is post dominated by its correlative cast.
  bool isLegalToTransform(void);

  bool isNodeVisited(const DDGNode *N);

  // Replace instruction \p InstrToReplace with coresponding intrinsic
  void replaceInstWithIntrinsic(Instruction *InstrToReplace);

  // Process each path and do the required transformation if applicable.
  void applyTransform(void);

  // Iterate over forward path and build a map from DDG node to a set of DDG
  // node being post dominated.Returns true, if the map is built.
  bool buildPostDomSet(void);

  // Prints DOT format graph using post dominator map
  void printPostDomTree(NodeSetVectorMap PostDomSet);

  // Prints DOT format path
  void printPath(bool IsPrintUseDef = false);
};

static void printGraph(const DataDependenceGraph &G, bool isPrintUseDef);

bool EliminateSwizzleCast::CheckLegal(unsigned int PrevStratCode,
                                      unsigned int PrevEndCode,
                                      unsigned int StartCode,
                                      unsigned int EndCode) {
  if ((StartCode == Instruction::FPTrunc) &&
      (EndOpCode == Instruction::FPExt) &&
      (PrevStratCode == Instruction::FPExt) &&
      (PrevEndCode == Instruction::FPTrunc)) {
    return true;
  } else if ((StartCode == Instruction::SIToFP) &&
             (EndOpCode == Instruction::FPToSI) &&
             (PrevStratCode == Instruction::FPToSI) &&
             (PrevEndCode == Instruction::SIToFP)) {
    return true;
  } else if ((StartCode == Instruction::Instruction::UIToFP) &&
             (EndOpCode == Instruction::FPToUI) &&
             (PrevStratCode == Instruction::FPToUI) &&
             (PrevEndCode == Instruction::UIToFP)) {
    return true;
  } else {
    return false;
  }
}

// Analyze all paths of DDG. Each DDG node consists of single or multiple
// instruction.A path is defined as a set of DDG nodes starting and ending with
// correlative casts "fpext/fptrunc" or 'sitofp/fptosi' or 'uitofp/fptoui' and
// transform the correlative casts with the corresponding intrinsics, if found
// legal for transformation. For example,
//                                      fptrunc --> t2
//                                      /
//        t0 --> fpext --> t1 ---> add
//                                      \
//                                        fptrunc --> t3
void EliminateSwizzleCast::processDDG(const DataDependenceGraph &G) {

  bool flag = false;
  unsigned int PrevStartCode = 0;
  unsigned int PrevEndCode = 0;
  // Find the start and end operation of the path which will be used as
  // indicator for all forward and backward path computation.
  for (auto entry : marker) {
    StartOpCode = entry.first;
    EndOpCode = entry.second;
    LLVM_DEBUG(dbgs() << "StartOpCode =" << StartOpCode << "\n");
    LLVM_DEBUG(dbgs() << "EndOpCode =" << EndOpCode << "\n");

    if (EndOpCode != Instruction::Call) {
      if (flag &&
          CheckLegal(PrevStartCode, PrevEndCode, StartOpCode, EndOpCode)) {
        flag = false;
        PrevStartCode = 0;
        PrevEndCode = 0;
        continue;
      } else {
        PrevStartCode = 0;
        PrevEndCode = 0;
        flag = false;
      }
    }

    // Compute all forward paths forllowing Def-Use chain having correlative
    // casts 'fpext/fptrunc' or 'sitofp/fptosi' or 'uitofp/fptoui'.
    computeAllFWPath(G);

    // Computes all backward paths forllowing Use-Def chain having correlative
    // casts 'fpext/fptrunc' or 'sitofp/fptosi' or 'uitofp/fptoui'.
    computeAllBWPath(G);

    // Prints computed forward/backward paths for debugging
#ifndef NDEBUG
  printPath();
  printPath(true);
#endif

  // Iterate over the computed forward/backward paths and build a post-dominator
  // map.Early exit, if no correlative casts post-dom relation found.
  if (!buildPostDomSet()) {
      LLVM_DEBUG(dbgs() << "No post-dom relation found\n");
      PostDomSet.clear();
      ReversePaths.clear();
      ForwardPaths.clear();
      continue;
  }

  // This is to handle trivial store-pack pattern where a fptrun is fed to
  // subsequent store.tnsr.
  if (!isLegalToTransform()) {
    LLVM_DEBUG(dbgs() << "Special Case Try !!\n");
    // Clear all data structure and explicitly set the end marker for store-pack
    // pattern path.
    PostDomSet.clear();
    ReversePaths.clear();
    ForwardPaths.clear();
    printPath();
    printPath(true);
    continue;
  }

#ifndef NDEBUG
  printPostDomTree(PostDomSet);
  // printGraph(G, true);
#endif
  if (!PostDomSet.empty() && EndOpCode != Instruction::Call) {
    PrevStartCode = StartOpCode;
    PrevEndCode = EndOpCode;
    flag = true;
  }
  // Copy the paths
  for (auto it : PostDomSet) {
    const DDGNode *PDomNode = it.getFirst();
    FinalPostDomSet[PDomNode] = it.second;
  }
  // Clear all data structure
  PostDomSet.clear();
  ReversePaths.clear();
  ForwardPaths.clear();
  }
  // Do transformation only when its not empty
  if (!FinalPostDomSet.empty()) {
    // Query post dominator map and do the transformation if applicable
#ifndef NDEBUG
  printPostDomTree(FinalPostDomSet);
#endif
    applyTransform();
  }
}

// Returns true if node \p N is already visited.
bool EliminateSwizzleCast::isNodeVisited(const DDGNode *N) {
  NodeToBoolMapType::iterator V = NodeVisitedMap.find(N);
  if (V != NodeVisitedMap.end())
    return V->second;

  return false;
}

// Returns true if \p I is tpc_st_tnsr intrinsic.
static bool isStoreIntrinsic(const Instruction *I) {
  LLVM_DEBUG(dbgs() << "Checking for StoreIntrinsic\n");
  const auto *Intrin = dyn_cast<IntrinsicInst>(I);
  if (!Intrin)
    return false;
  if (Intrin->getIntrinsicID() != llvm::Intrinsic::tpc_st_tnsr)
    return false;
  return true;
}

// Returns true if \p I is tpc_ld_g intrinsic.
static bool isLoadIntrinsic(const Instruction *I) {
  LLVM_DEBUG(dbgs() << "Checking for LoadIntrinsic\n");
  const auto *Intrin = dyn_cast<IntrinsicInst>(I);
  if (!Intrin)
    return false;
  if (Intrin->getIntrinsicID() == llvm::Intrinsic::tpc_ld_g)
    return true;
  return false;
}

// Returns true if \p I is tpc_fptosi_swch intrinsic.
static bool isFpToSiIntrinsic(const Instruction *I) {
  LLVM_DEBUG(dbgs() << "Checking for fptosi Intrinsic\n");
  const auto *Intrin = dyn_cast<IntrinsicInst>(I);
  if (!Intrin)
    return false;
  if (Intrin->getIntrinsicID() != llvm::Intrinsic::tpc_fptosi_swch)
    return false;
  return true;
}

// Returns true if \p I is tpc_fptoui_swch intrinsic.
static bool isFpToUiIntrinsic(const Instruction *I) {
  LLVM_DEBUG(dbgs() << "Checking for fptoui Intrinsic\n");
  const auto *Intrin = dyn_cast<IntrinsicInst>(I);
  if (!Intrin)
    return false;
  if (Intrin->getIntrinsicID() != llvm::Intrinsic::tpc_fptoui_swch)
    return false;
  return true;
}

// Returns true if \p I is tpc_fpext_swch intrinsic.
static bool isFpExtIntrinsic(const Instruction *I) {
  LLVM_DEBUG(dbgs() << "Checking for tpc_fpext_swch Intrinsic\n");
  const auto *Intrin = dyn_cast<IntrinsicInst>(I);
  if (!Intrin)
    return false;
  if (Intrin->getIntrinsicID() != llvm::Intrinsic::tpc_fpext_swch)
    return false;
  return true;
}

// Returns true if \p I is tpc_fptrunc_swch intrinsic.
static bool isFpTruncIntrinsic(const Instruction *I) {
  LLVM_DEBUG(dbgs() << "Checking for tpc_fptrunc_swch Intrinsic\n");
  const auto *Intrin = dyn_cast<IntrinsicInst>(I);
  if (!Intrin)
    return false;
  if (Intrin->getIntrinsicID() != llvm::Intrinsic::tpc_fptrunc_swch)
    return false;
  return true;
}

// Returns true if \p I is tpc_sitofp_swch intrinsic.
static bool isSiToFpIntrinsic(const Instruction *I) {
  LLVM_DEBUG(dbgs() << "Checking for tpc_sitofp_swch Intrinsic\n");
  const auto *Intrin = dyn_cast<IntrinsicInst>(I);
  if (!Intrin)
    return false;
  if (Intrin->getIntrinsicID() != llvm::Intrinsic::tpc_sitofp_swch)
    return false;
  return true;
}

// Returns true if \p I is tpc_uitofp_swch intrinsic.
static bool isUiToFpIntrinsic(const Instruction *I) {
  LLVM_DEBUG(dbgs() << "Checking for tpc_uitofp_swch Intrinsic\n");
  const auto *Intrin = dyn_cast<IntrinsicInst>(I);
  if (!Intrin)
    return false;
  if (Intrin->getIntrinsicID() != llvm::Intrinsic::tpc_uitofp_swch)
    return false;
  return true;
}

// Returns the llvm instruction or intrinsic which could be part of the pattern
// we are looking for.Also sets the \p OpCode corresponding to the found
// instruction.
static Instruction *getIntrestingInstruction(Instruction *I,
                                             unsigned int *OpCode) {
  if (I->getOpcode() == Instruction::FPExt || isFpExtIntrinsic(I)) {
    if (OpCode)
      *OpCode = Instruction::FPExt;
    return I;
  }
  if (I->getOpcode() == Instruction::FPTrunc || isFpTruncIntrinsic(I)) {
    if (OpCode)
      *OpCode = Instruction::FPTrunc;
    return I;
  }
  if (I->getOpcode() == Instruction::SIToFP || isSiToFpIntrinsic(I)) {
    if (OpCode)
      *OpCode = Instruction::SIToFP;
    return I;
  }
  if (I->getOpcode() == Instruction::FPToSI || isFpToSiIntrinsic(I)) {
    if (OpCode)
      *OpCode = Instruction::FPToSI;
    return I;
  }
  if (I->getOpcode() == Instruction::UIToFP || isUiToFpIntrinsic(I)) {
    if (OpCode)
      *OpCode = Instruction::UIToFP;
    return I;
  }
  if (I->getOpcode() == Instruction::FPToUI || isFpToUiIntrinsic(I)) {
    if (OpCode)
      *OpCode = Instruction::FPToUI;
    return I;
  }
  if (I->getOpcode() == Instruction::Call && isStoreIntrinsic(I)) {
    LLVM_DEBUG(dbgs() << "Found Store Intrinsic Instruction\n");
    if (OpCode)
      *OpCode = Instruction::Call;
    return I;
  }
  if (isLoadIntrinsic(I)) {
    LLVM_DEBUG(dbgs() << "Found Load Intrinsic Instruction\n");
    if (OpCode) {
      *OpCode = Instruction::Call;
    }
    return I;
  }
  return nullptr;
}

// Returns the cast instruction if found in node \p N, else returns nullptr.
// Also sets the \p OpCode corresponding to the found instruction.
static Instruction *getIntrestingInst(const DDGNode *N, unsigned int *OpCode) {
  for (Instruction *I : cast<const SimpleDDGNode>(*N).getInstructions()) {
    return getIntrestingInstruction(I, OpCode);
  }
  return nullptr;
}

// Returns the opcode of the cast operation this node \p N contains otherwise
// returns 0.
static unsigned int getIntrestingOpCode(const DDGNode *N) {
  unsigned int OpCode = 0;
  getIntrestingInst(N, &OpCode);
  return OpCode;
}

static bool IsLoadIntrin(const DDGNode *N) {
  for (Instruction *I : cast<const SimpleDDGNode>(*N).getInstructions()) {
    if (auto *TpcIntrins = dyn_cast<IntrinsicInst>(I)) {
      Intrinsic::ID Inid = TpcIntrins->getIntrinsicID();
      if (Inid == Intrinsic::tpc_ld_g) {
        return true;
      }
    }
  }
  return false;
}

static bool isStoreIntrin(const DDGNode *N) {
  for (Instruction *I : cast<const SimpleDDGNode>(*N).getInstructions()) {
    if (auto *TpcIntrins = dyn_cast<IntrinsicInst>(I)) {
      Intrinsic::ID Inid = TpcIntrins->getIntrinsicID();
      if (Inid == Intrinsic::tpc_st_tnsr) {
        return true;
      }
    }
  }
  return false;
}

// Computes all forward paths forllowing Def-Use chain having correlative casts
// such as 'fpext/fptrunc' or 'sitofp/fptosi'.
void EliminateSwizzleCast::computeAllFWPath(const DataDependenceGraph &G) {
  LLVM_DEBUG(dbgs() << " Computing forward path \n");

  // Iterate over all nodes of DDG, check in each DDG node for the existence of
  // 'fpext/fptrunc' or 'sitofp/fptosi' or 'uitofp/fptoui' instruction. Idea is
  // to get start and end instruction of the path as one of 'fpext/fptrunc' or
  // 'sitofp/fptosi' or 'uitofp/fptoui'.
  for (const DDGNode *N : G) {
    if (!isa<SimpleDDGNode>(*N))
      continue;

    unsigned int OpCode = getIntrestingOpCode(N);

    if (OpCode != StartOpCode) {
      LLVM_DEBUG(dbgs() << "Correlative cast not found: CONTINUE "
                        << "\n");
      continue;
    }

    // Having found the start and end instruction, get the full forward path.
    for (auto &E : N->getEdges()) {
      // Skip use-def edges for forward path.
      if (!E->isDefUse())
        continue;
      NodeSetType Path;
      dfsAllPath(N, StartOpCode, EndOpCode, Path, false);
      //NodeVisitedMap.clear();
      Path.clear();
    }
    NodeVisitedMap.clear();
  }
}

// Recursively follow Def-Use or Use-Def edges starting from node \p Src and
// ending at node containing instruction with path ending operation \p
// EndOpCode.\p IsBackward is to chose direction of traversal.
void EliminateSwizzleCast::dfsAllPath(const DDGNode *Src,
                                      unsigned int StartOpCode,
                                      unsigned int EndOpCode, NodeSetType &Path,
                                      bool BackwardSearch) {
  NodeVisitedMap[Src] = true;
  Path.insert(Src);

  Instruction *I = cast<const SimpleDDGNode>(*Src).getFirstInstruction();
  if (I) {
    LLVM_DEBUG(dbgs() << " dfsAllPath: Src Instruction - " << *I << "\n");
    LLVM_DEBUG(dbgs() << " OpCode is: " << I->getOpcode() << "\n");
    LLVM_DEBUG(dbgs() << " StartOpCode is: " << StartOpCode << "\n");
    LLVM_DEBUG(dbgs() << " EndOpCode is: " << EndOpCode << "\n");
  }

  // Check if backward search & instruction is ld_g
  IntrinsicInst *Intrins = nullptr;
  bool ScalarBroadcast = false;
  bool LoadOperation = false;
  bool FpExt = false;
  if ((Intrins = dyn_cast<IntrinsicInst>(I))) {
    // Check if this is ld_g
    Intrinsic::ID Inid = Intrins->getIntrinsicID();
    if (Inid == Intrinsic::tpc_ld_g) {
      // Check if the operand 0 is scaler
      if (auto *TpcIntrins = dyn_cast<IntrinsicInst>(Intrins->getOperand(0))) {
        Intrinsic::ID TpcInid = TpcIntrins->getIntrinsicID();
        if (TpcInid == Intrinsic::tpc_gen_addr) {
          ScalarBroadcast = true;
        }
      }
    } else if (Inid == Intrinsic::tpc_ld_tnsr) {
      // This also should be the termination point
      LoadOperation = true;
    } else if (Inid == Intrinsic::tpc_fpext_swch &&
               (StartOpCode == Instruction::Call ||
                EndOpCode == Instruction::Call)) {
      /*Store pack cannot be done if there is a FPExt vector in between*/
      FpExt = true;
    }
  } else if (I && I->getOpcode() == Instruction::FPExt &&
             (StartOpCode == Instruction::Call ||
              EndOpCode == Instruction::Call)) {
    FpExt = true;
  }

  // Need to prevent store-pack, if fpext is found between fptrunc and store.
  if (FpExt && (StartOpCode == Instruction::FPTrunc ||
                EndOpCode == Instruction::FPTrunc)) {
    LLVM_DEBUG(
        dbgs()
        << "dfsAllPath: Intervening Fpext found in fptunc to store-pack \n");
    return;
  }

  // For path starting with FPTrunc, call to store-pack intrinsic could be other
  // ending operation apart from FPext.
  bool EndsWithStore = (StartOpCode == Instruction::FPTrunc &&
                        EndOpCode == Instruction::Call && isStoreIntrinsic(I));

  // Search ending checks for forward and backward path identification.
  unsigned int OpCode = 0;
  Instruction *TmpInst = getIntrestingInstruction(I, &OpCode);
  if ((TmpInst && OpCode == EndOpCode && !isStoreIntrinsic(I)) ||
      EndsWithStore ||
      (I->getOpcode() == Instruction::PHI && // End check for backward path
       StartOpCode != Instruction::Call) ||
      ScalarBroadcast || LoadOperation || FpExt) {

    // If the path ends with call to store-pack intrinsic, set the 'EndOpcode'
    // to Call operation.
    if (EndsWithStore)
      setEndOpCode(Instruction::Call);

    if (!BackwardSearch)
      ForwardPaths.push_back(Path);
    else
      ReversePaths.push_back(Path);
    LLVM_DEBUG(dbgs() << "dfsAllPath: Path Found, push and clear... \n");
    return;
  }

  for (auto &E : Src->getEdges()) {
    if (!BackwardSearch) {
      if (!E->isDefUse())
        continue;
    } else {
      if (E->isDefUse())
        continue;
    }
    if (E->isMemoryDependence()) {
      continue;
    }
    const DDGNode &Nbr = E->getTargetNode();
    if (!isa<SimpleDDGNode>(Nbr))
      continue;

    bool Visited = isNodeVisited(&Nbr);
    if (!Visited) {
      LLVM_DEBUG(dbgs() << "Recursing down the path.. \n");
      dfsAllPath(&Nbr, StartOpCode, EndOpCode, Path, BackwardSearch);
      if (!Path.empty())
        Path.pop_back();
    }
  }
}

// Computes all backward paths forllowing Use-Def chain having correlative casts
// 'fpext/fptrunc' or 'sitofp/fptosi'.
void EliminateSwizzleCast::computeAllBWPath(const DataDependenceGraph &G,
                                            bool IsStorePackSpl) {
  LLVM_DEBUG(dbgs() << " Computing backward path \n");

  // Iterate over all nodes of DDG, check in each DDG node for the existence of
  // 'fpext/fptrun' or 'sitofp/fptosi' or 'uitofp/fptoui' instruction. Idea is
  // to get start and end instruction of the path.
  for (const DDGNode *N : G) {

    if (!isa<SimpleDDGNode>(*N))
      continue;

    unsigned int OpCode = getIntrestingOpCode(N);

    // Keep looking if intresting instruction is not found.
    if (OpCode != EndOpCode) {
      LLVM_DEBUG(dbgs() << "Correlative cast not found CONTINUE "
                        << "\n");
        continue;
    }

    // Skip def-use edges for backward path.
    for (auto &E : N->getEdges()) {
      if (E->isDefUse())
        continue;

      // Handle trivial Store-pack pattern path, continue if not special store
      // pack pattern.
      if (IsStorePackSpl && (StartOpCode == Instruction::FPTrunc) &&
          (EndOpCode == Instruction::Call)) {
        LLVM_DEBUG(dbgs() << "StorePack trivial Path CHECK \n");
        Instruction *I = cast<const SimpleDDGNode>(*N).getFirstInstruction();
        const DDGNode &Nbr = E->getTargetNode();

        // Instruction *TmpInst = getIntrestingInst(&Nbr, &TempOpCode);
        unsigned int TempOpCode = getIntrestingOpCode(&Nbr);
        if (isStoreIntrinsic(I) && TempOpCode == Instruction::FPTrunc) {
          NodeSetType TmpPath;
          TmpPath.insert(N);
          TmpPath.insert(&Nbr);
          ReversePaths.push_back(TmpPath);
          LLVM_DEBUG(dbgs() << "StorePack trivial Path Found \n");
          return;
        }
      }

      // Having found the start and end, try to get the backward path.
      // For backward path, end of path is indicated by 'StartOpCode'
      NodeSetType Path;
      dfsAllPath(N, EndOpCode, StartOpCode, Path, true);
      Path.clear();
    }
    NodeVisitedMap.clear();
  }
}

// Iterate over forward path and build a map from DDG node to a set of DDG node
// being post dominated.
bool EliminateSwizzleCast::buildPostDomSet(void) {
  LLVM_DEBUG(dbgs() << " Building post dominator map"
                    << "\n");

  // For zero forward path, no need to build postdom.
  if (ForwardPaths.empty())
    return false;
  bool IsMapAvailable = false;
  for (auto &Path : ReversePaths) {
    const DDGNode *FN = Path.front();
#ifndef NDEBUG
    for (const Instruction *I :
         cast<const SimpleDDGNode>(*FN).getInstructions()) {
      LLVM_DEBUG(dbgs() << *I);
      LLVM_DEBUG(dbgs() << "\n");
    }
    const DDGNode *BN = Path.back();
    for (const Instruction *I :
         cast<const SimpleDDGNode>(*BN).getInstructions()) {
      LLVM_DEBUG(dbgs() << *I);
      LLVM_DEBUG(dbgs() << "\n");
    }
#endif
    PostDomSet[FN].insert(Path.back());
    IsMapAvailable = true;
  }
  return IsMapAvailable;
}

bool EliminateSwizzleCast::CheckFinalList(const Instruction *I1) {
  for (auto IT2 : FinalPostDomSet) {
    const DDGNode *PDomNode2 = IT2.getFirst();
    for (const Instruction *I2 :
         cast<const SimpleDDGNode>(*PDomNode2).getInstructions()) {
      if (I1 == I2) {
        LLVM_DEBUG(dbgs() << "Found the Match Optimization not possible\n");
        return false;
      }
    }
  }
  return true;
}

bool EliminateSwizzleCast::isLegalToTransform(void) {
  LLVM_DEBUG(dbgs() << "Legality check\n");
  if (PostDomSet.empty()) {
    return false;
  }

  // Post dominator nodes must have only path ending operation, i.e
  // 'EndOpCode' while the dominated node can only have path starting
  // operation i.e 'StartOpCode'.
  for (auto IT : PostDomSet) {
    const DDGNode *PDomNode = IT.getFirst();
    unsigned int OpEnd = 0;

    // Handle fptrunc/store-pack pattern, when it overlaps with fprunc/fpext
    // pattern. Basically multiple use of fptrunc feeding to st.tnsr needs to be
    // prevented from getting transformed into store-pack if no valid fptrunc-fpext
    // postdom relation exist.
    if (isStoreIntrin(PDomNode)) {
      // Get the fptrunc feeding into st.tnsr.
      unsigned TmpOpCode;
      Instruction *SI = getIntrestingInst(PDomNode, &TmpOpCode);
      CallInst *CI = cast<CallInst>(SI);
      CallSite CS(CI);
      Value *CastVal = CS.getArgument(2);
      LLVM_DEBUG(dbgs() << "isLegalToTransform:  Cast instruction:  "
                        << *CastVal << "\n");
      CallInst *Cast = dyn_cast<CallInst>(CastVal);

      // Inside fptrunc-fpext path, Ensure store-pack pattern transform only for
      // valid fptrunc-fext postdom relation.
      if (Cast && !Cast->hasOneUse()) {
        LLVM_DEBUG(
            dbgs() << "isLegalToTransform: Multiple use of fptrunc found\n");
        unsigned DNCount = 0;
        for (auto ITT : PostDomSet) {
          for (const auto *DomNode : ITT.getSecond()) {
            Instruction *Inst = getIntrestingInst(DomNode, &TmpOpCode);
            if (Cast == Inst) {
              DNCount++;
            }
          }
        }

        // Valid fptrunc-fpext postdom not found, so prevent fptrunc/store-pack
        // transform.
        if (DNCount == 1) {
          LLVM_DEBUG(
              dbgs()
              << "isLegalToTransform: Prevent fptrunc/store-pack transform\n");
          PostDomSet.erase(PDomNode);
          continue;
        }
      }
    }

    // Handle unsupported 'convert' ops by preventing the transformation.
    Instruction *I = getIntrestingInst(PDomNode, &OpEnd);
    VectorType *VTy = dyn_cast<VectorType>(I->getType());
    if (VTy) {
      Type *ElementTy = VTy->getElementType();
      unsigned Num = VTy->getVectorNumElements();
      if ((ElementTy->isIntegerTy() || ElementTy->isFloatTy()) && Num == 256) {
        PostDomSet.erase(PDomNode);
        continue;
      }
    }

    if (OpEnd != EndOpCode && !(IsLoadIntrin(PDomNode))) {
      for (auto ItBegin : PostDomSet) {
        if (ItBegin.getFirst() == PDomNode) {
          PostDomSet.erase(PDomNode);
        }
      }
      continue;
    }

    for (const auto *DomNode : IT.second) {
      LLVM_DEBUG(dbgs() << "Legality check 3\n");
      unsigned OpStart = getIntrestingOpCode(DomNode);
      // if both start and End are load and store instruction simply drop them
      if ((OpStart != StartOpCode && !(IsLoadIntrin(DomNode))) ||
          (OpEnd == Instruction::Call && OpStart == Instruction::Call)) {
        for (auto ItBegin : PostDomSet) {
          if (ItBegin.getFirst() == PDomNode) {
            PostDomSet.erase(PDomNode);
          }
        }
      }
    }
  }

  /*We need to check any of the instruction in PostDomSet is not already part
    of the FinalPostDomSet*/
  for (auto IT1 : PostDomSet) {
    const DDGNode *PDomNode1 = IT1.getFirst();
    for (const Instruction *I1 :
         cast<const SimpleDDGNode>(*PDomNode1).getInstructions()) {
      /*We need to check if this instruction is part of FinalPostDomSet*/
      if (!CheckFinalList(I1)) {
        return false;
      }
    }
    /*Check for Second part*/
    for (const auto *DomNode : IT1.second) {
      for (const Instruction *I1 :
           cast<const SimpleDDGNode>(*DomNode).getInstructions()) {
        if (!CheckFinalList(I1)) {
          return false;
        }
      }
    }
  }
  return true;
}

static void collectInstToReplace(NodeSetVectorMap &PostDomSet,
                                 DenseSet<Instruction *> &WorkingSet) {
  Instruction *I;
  for (auto IT : PostDomSet) {
    const DDGNode *PDomNode = IT.getFirst();
    I = getIntrestingInst(PDomNode, nullptr);
    assert(I && "PDomNode must have instruction");

    WorkingSet.insert(I);
    LLVM_DEBUG(dbgs() << "Post Dominator Instruction: " << *I << "\n");
    for (const auto *DomNode : IT.second) {
      I = getIntrestingInst(DomNode, nullptr);
      assert(I && "DomNode must have instruction");
      WorkingSet.insert(I);
      LLVM_DEBUG(dbgs() << "Dominated Instruction: " << *I << "\n");
    }
  }
}

// Process each path and do the required transformation if applicable.
void EliminateSwizzleCast::applyTransform(void) {
  LLVM_DEBUG(dbgs() << "Try to transform\n");

  // Iterate over post dominator map and do the transformation
  DenseSet<Instruction *> WorkingSet;
  collectInstToReplace(FinalPostDomSet, WorkingSet);

  for (auto Inst : WorkingSet) {
    LLVM_DEBUG(dbgs() << "Replacing Instruction: " << *Inst << "\n");
    replaceInstWithIntrinsic(Inst);
  }
}

// Sets the modified switch using the specified \p Switch to this call
// instruction.If no switch is supplied, returns the switch from the call
// instruction.
static int getOrSetSwitch(CallSite CS, int ArgNum, int Switch = -1) {
  int SW = -1;
  Value *SwitchVal = CS.getArgument(ArgNum);
  ConstantInt *CI = dyn_cast<llvm::ConstantInt>(SwitchVal);
  if (!CI) {
    LLVM_DEBUG(dbgs() << "Switch not a constant !!\n");
    return SW;
  }

  SW = CI->getSExtValue();
  if (Switch == -1)
    return SW;

  // Update the switch.
  SW = SW | Switch;
  SwitchVal = ConstantInt::get(CI->getType(), SW, true);
  CS.setArgument(ArgNum, SwitchVal);

  // Return the modified switch
  return SW;
}

// Replace instruction \p InstrToReplace with coresponding intrinsic
void EliminateSwizzleCast::replaceInstWithIntrinsic(
    Instruction *InstrToReplace) {
  Value *IntrinsicCall = nullptr;
  if (InstrToReplace->getOpcode() == Instruction::FPExt ||
      InstrToReplace->getOpcode() == Instruction::FPTrunc ||
      InstrToReplace->getOpcode() == Instruction::SIToFP ||
      InstrToReplace->getOpcode() == Instruction::FPToSI ||
      InstrToReplace->getOpcode() == Instruction::UIToFP ||
      InstrToReplace->getOpcode() == Instruction::FPToUI) {
    LLVM_DEBUG(dbgs() << "Found cast instruction to be replaced\n");
    IntrinsicCall = createConvertIntrinsic(InstrToReplace, 0);
  } else if (isFpExtIntrinsic(InstrToReplace) ||
             isFpTruncIntrinsic(InstrToReplace) ||
             isSiToFpIntrinsic(InstrToReplace) ||
             isUiToFpIntrinsic(InstrToReplace) ||
             isFpToUiIntrinsic(InstrToReplace) ||
             isFpToSiIntrinsic(InstrToReplace)) {
    LLVM_DEBUG(dbgs() << "Found intrinsic to be replaced\n");
    CallInst *CI = cast<CallInst>(InstrToReplace);
    CallSite CS(CI);
    // Get the switch from the intrinsic instruction to be applied while
    // creating the convert intrinsic.
    int Switch = getOrSetSwitch(CS, 1);
    LLVM_DEBUG(dbgs() << "getOrSetSwitch(CS, 1): " << Switch << "\n");
    IntrinsicCall = createConvertIntrinsic(InstrToReplace, Switch);
  }
  if (IntrinsicCall) {
    InstrToReplace->replaceAllUsesWith(IntrinsicCall);
    InstrToReplace->eraseFromParent();
  }

  if (InstrToReplace->getOpcode() == Instruction::Call &&
      isStoreIntrinsic(InstrToReplace)) {
    LLVM_DEBUG(dbgs() << "Modifying Store pack switch\n");
    CallInst *CI = cast<CallInst>(InstrToReplace);
    CallSite CS(CI);
    getOrSetSwitch(CS, 3, 1 << 2);
  }
}

// Define node attributes for DOT graph printing
static void defineNode(const DDGNode *N) {
  LLVM_DEBUG(dbgs() << "\n");
  LLVM_DEBUG(dbgs() << "Node" << N);
  LLVM_DEBUG(dbgs() << " [shape=\"box\", label=\"");
  if (N->getKind() == DDGNode::NodeKind::Root)
    LLVM_DEBUG(dbgs() << "START");
  if (N->getKind() == DDGNode::NodeKind::Exit)
    LLVM_DEBUG(dbgs() << "END");
  for (const Instruction *I : cast<const SimpleDDGNode>(*N).getInstructions()) {
    LLVM_DEBUG(dbgs() << *I);
  }
  LLVM_DEBUG(dbgs() << "\""
                    << "];\n");
}

// Print DOT edges
static void printEdge(const DDGNode *N, bool isPrintUseDef = false) {
  LLVM_DEBUG(dbgs() << "\n");
  for (auto &E : N->getEdges()) {
    if (E->isDefUse()) {
      if (isPrintUseDef)
        continue;
    } else {
      if (!isPrintUseDef)
        continue;
    }

    LLVM_DEBUG(dbgs() << "Node" << N);
    LLVM_DEBUG(dbgs() << " -> ");
    const DDGNode &TN = E->getTargetNode();
    if (isa<RootDDGNode>(TN) || isa<ExitDDGNode>(TN)) {
      LLVM_DEBUG(dbgs() << "Node" << &TN << "\n");
      continue;
    }
    if (!isa<SimpleDDGNode>(TN))
      continue;
    LLVM_DEBUG(dbgs() << "Node" << &TN << "\n");
  }
}

// Prints DOT format edges using post dominator map
static void printPDomEdge(const DDGNode *Src, NodeSetType &NS) {
  for (const DDGNode *TN : NS) {
    defineNode(TN);
    LLVM_DEBUG(dbgs() << "Node" << Src);
    LLVM_DEBUG(dbgs() << " -> ");
    LLVM_DEBUG(dbgs() << "Node" << TN << "\n");
  }
}

// Prints DOT format graph using post dominator map
void EliminateSwizzleCast::printPostDomTree(NodeSetVectorMap PostDomSet) {
  LLVM_DEBUG(dbgs() << "Printing printPostDomTree: "
                    << "\n");
  LLVM_DEBUG(dbgs() << "Digraph {"
                    << "\n");
  for (auto IT : PostDomSet) {
    defineNode(IT.first);
    printPDomEdge(IT.first, IT.second);
  }
  LLVM_DEBUG(dbgs() << "}"
                    << "\n");
}

// Prints DOT format path
void EliminateSwizzleCast::printPath(bool isPrintUseDef) {
  if (!isPrintUseDef) {
    LLVM_DEBUG(dbgs() << "Printing Def-Use path: " << ForwardPaths.size()
                      << "\n");
    for (auto &Path : ForwardPaths) {
      LLVM_DEBUG(dbgs() << "Digraph {"
                        << "\n");
      for (const DDGNode *N : Path) {
        if (!isa<SimpleDDGNode>(*N))
          continue;
        defineNode(N);
        printEdge(N, isPrintUseDef);
      }
      LLVM_DEBUG(dbgs() << "}"
                        << "\n");
    }
  } else {
    LLVM_DEBUG(dbgs() << "Printing Use-Def path: " << ReversePaths.size()
                      << "\n");
    for (auto &Path : ReversePaths) {
      LLVM_DEBUG(dbgs() << "Digraph {"
                        << "\n");
      for (const DDGNode *N : Path) {
        if (!isa<SimpleDDGNode>(*N))
          continue;
        defineNode(N);
        printEdge(N, isPrintUseDef);
      }
      LLVM_DEBUG(dbgs() << "}"
                        << "\n");
    }
  }
}

// Prints DOT format graph using DataDependenceGraph \p G
static void printGraph(const DataDependenceGraph &G, bool isPrintUseDef) {
  LLVM_DEBUG(dbgs() << "Printing Graph"
                    << "\n");
  LLVM_DEBUG(dbgs() << "Digraph {"
                    << "\n");
  for (const DDGNode *N : G) {
    if (!isa<SimpleDDGNode>(*N))
      continue;
    defineNode(N);
    printEdge(N, isPrintUseDef);
  }
  LLVM_DEBUG(dbgs() << "}"
                    << "\n");
}

class EliminateSwizzleCastLegacyPass : public FunctionPass {
public:
  static char ID; // Pass ID, replacement for typeid
  EliminateSwizzleCastLegacyPass() : FunctionPass(ID) {
    initializeEliminateSwizzleCastLegacyPassPass(
        *PassRegistry::getPassRegistry());
  }

  bool runOnFunction(Function &F) override;

  void getAnalysisUsage(AnalysisUsage &AU) const override {
    AU.addRequired<AAResultsWrapperPass>();
    AU.addRequired<ScalarEvolutionWrapperPass>();
    AU.addRequired<LoopInfoWrapperPass>();
  }
};

bool EliminateSwizzleCastLegacyPass::runOnFunction(Function &F) {
  if (skipFunction(F))
    return false;

  auto &AA = getAnalysis<AAResultsWrapperPass>().getAAResults();
  auto &SE = getAnalysis<ScalarEvolutionWrapperPass>().getSE();
  auto &LI = getAnalysis<LoopInfoWrapperPass>().getLoopInfo();

  // Construct Data Dependence Graph(DDG)
  DependenceInfo DI(&F, &AA, &SE, &LI);
  DataDependenceGraph G(F, DI);

  // Analyze DDG and do the transformation if applicable.
  EliminateSwizzleCast CS;

  CS.processDDG(G);
  return false;
}
} // namespace

char EliminateSwizzleCastLegacyPass::ID = 0;
INITIALIZE_PASS_BEGIN(EliminateSwizzleCastLegacyPass, "cast-swizzle-opt",
                      "Eliminate swizzle ops", false, false)
INITIALIZE_PASS_DEPENDENCY(AAResultsWrapperPass)
INITIALIZE_PASS_DEPENDENCY(ScalarEvolutionWrapperPass)
INITIALIZE_PASS_DEPENDENCY(LoopInfoWrapperPass)
INITIALIZE_PASS_END(EliminateSwizzleCastLegacyPass, "cast-swizzle-opt",
                    "Eliminate swizzle ops", false, false)

FunctionPass *llvm::createEliminateSwizzleCastLegacyPass() {
  return new EliminateSwizzleCastLegacyPass();
}
