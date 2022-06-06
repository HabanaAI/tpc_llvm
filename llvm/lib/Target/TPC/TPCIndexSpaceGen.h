//===- TPCIndexSpaceGen.h --- TPC INDEX SPACE ----------------------------===//
//
//
//                     The LLVM Compiler Infrastructure:
//
//              2020 - This pass is a property of Habana labs
//
//
//===----------------------------------------------------------------------===//
//===----------------------------------------------------------------------===//
#ifndef LLVM_TPC_INDEX_SPACE_GEN_CPP_H
#define LLVM_TPC_INDEX_SPACE_GEN_CPP_H

#include "TPCTargetMachine.h"
#include "llvm/ADT/StringRef.h"
#include "llvm/Analysis/AssumptionCache.h"
#include "llvm/Analysis/LoopInfo.h"
#include "llvm/Analysis/MemoryBuiltins.h"
#include "llvm/Analysis/ScalarEvolution.h"
#include "llvm/Analysis/ScalarEvolutionExpressions.h"
#include "llvm/IR/Dominators.h"
#include "llvm/IR/Function.h"
#include "llvm/IR/InstIterator.h"
#include "llvm/IR/IntrinsicInst.h"
#include "llvm/IR/IntrinsicsTPC.h"
#include "llvm/Transforms/Utils/LoopUtils.h"
#include "llvm/Transforms/Utils/UnrollLoop.h"
#include <unordered_map>
#include <unordered_set>

using namespace llvm;

#define PassName "TPCIndexGen"
#define PassDescription "Generate index factors per tensor for the kernel."
#define DEBUG_TYPE PassName

enum class TensorType { Input=0, Output };

struct TensorInfo {
  std::string Name;
  TensorType Type;
  unsigned Order;
  SmallVector<int, 5> IndexFactor;
  std::unordered_map<int, std::string> umap;
  unsigned AccessGranularity; /*64, 128, 256*/
};

class SCEVInfo {
public:
  SCEVInfo(Loop *L, LoopInfo *LI, ScalarEvolution *SE) : L(L), LI(LI), SE(SE) {
    processSCEVInfo();
    if (!SCEVPtr)
      LLVM_DEBUG(dbgs() << "Induction Variable could not be computed"
                        << "\n");
  }

  int64_t getSCEVStep() const;

  PHINode *getLoopInductionVar() const;

  std::string getSCEVLoopName() const;

  unsigned getLoopUnrollCount(Loop *L) const;

  Loop *getLoopPtr() const;

  void processSCEVExpression(); // TODO : Define per usage

  void dump() const;

private:
  void processSCEVInfo();
  Loop *L;
  LoopInfo *LI;
  ScalarEvolution *SE;
  const SCEV *SCEVPtr = nullptr;
  PHINode *LoopIndVar = nullptr;
};

// Utility for creating storage with SCEVInfo type

struct SCEVInfoHasher {
  size_t operator()(const SCEVInfo &obj) const noexcept {
    return std::hash<Loop *>()(obj.getLoopPtr());
  }
};

struct SCEVInfoComparator {
  bool operator()(const SCEVInfo &obj1, const SCEVInfo &obj2) const {
    if (obj1.getLoopPtr() == obj2.getLoopPtr())
      return true;
    return false;
  }
};

class TensorAccessAnalysis {
public:
  TensorAccessAnalysis(SmallVector<Loop *, 8> &TensorLoopsArg,
                       ScalarEvolution *SEArg, LoopInfo *LIArg)
      : TensorLoops(TensorLoopsArg), SE(SEArg), LI(LIArg), HasLoadAndStore(false) {
    for (Loop *L : TensorLoops)
      LoopSCEVInfo.emplace_back(L, LI, SE);
    FakeTensorId = 0;

    ArchTensorsBase["gaudi"] = 0x400U;
    ArchTensorsBase["gaudib"] = 0x400U;
    ArchTensorsBase["goya"] = 0x400U;
    ArchTensorsBase["greco"] = 0x0U;
    ArchTensorsBase["gaudi2"] = 0x0U;
    TensorSizeStrideArrOffset = 0x10U;

    ArchTensorsDescSize["gaudi"] = 0x38U;
    ArchTensorsDescSize["gaudib"] = 0x38U;
    ArchTensorsDescSize["goya"] = 0x4cU;
    ArchTensorsDescSize["greco"] = 0x38U;
    ArchTensorsDescSize["gaudi2"] = 0x50U;

    ConfigStartOffset["goya"] = 0x40C;
    ConfigStartOffset["gaudi"] = 0x40C;
    ConfigStartOffset["greco"] = 0xC;
    ConfigStartOffset["gaudi2"] = 0xC;

    ConfigOffset["goya"] = 0x4C;
    ConfigOffset["gaudi"] = 0x38;
    ConfigOffset["greco"] = 0x38;
    ConfigOffset["gaudi2"] = 0x50;
  }

  void processPragmaInfo(Function &F);
  void prepareStLData(Function &F);
  void prepareLdStLData(Function &F);
  void processLoadStoreLocalInfo(Function &F);
  void computeTensorInfo(Function &F, bool Update);
  void getloadstoreIntrins(Function &F);
  bool compDepthCheckforNextBlock(int Currdepth, Value *InVal,
                                  BasicBlock *RefBB);
  void processSetIndexNode(int TensorId, Loop *CurrLoop, PHINode *PHI,
                           Function &F, IntrinsicInst *SetIndexInst);
  std::string getFormula(IntrinsicInst *Intrins, Function &F);
  const std::vector<TensorInfo> &getInputVector() { return TensorInfos[unsigned(TensorType::Input)]; }
  const std::vector<TensorInfo> &getOutputVector() { return TensorInfos[unsigned(TensorType::Output)]; }
  std::string BailoutReason;
  bool BailOut = false;
  void updateIndSpaceCoords();
  const SmallVector<unsigned, 16> &getRedNormTensorVec() {
    return TensorIDVecReduction;
  }
  const SmallVector<std::string, 8> &getRedNormTensorAxes() {
    return ReductionOrNormAxes;
  }
  const std::multimap<int, std::tuple<int, std::string, std::string>> &
  getStartBEndBCoords() {
    return StartBEndBCoords;
  }
  void resetFakeTensorId() { FakeTensorId = 0; }
  bool getIndexSpaceBool() { return IncorrectIndexSpace; }
  void updateCoordFactor(unsigned int TensorID, unsigned int DimIndex,
                         int DimFactor, std::string = "", Loop * = nullptr);
  void dumpIntoASM();
  void dumpIntoASM_MLIR();
  void processPHIUses(int TensorId, PHINode *PHI, Loop *CurrLoop, Function &F);
  void checkRuntimeInfo(int TensorId, std::vector<Value *> TraceToPHI);
  void analyseGenAddr();
  void padGCCustom();
  void updateAddMask();
  void sortVectorsByID();
  void addToProcessedPHI(PHINode *PHI) { VisitedPhiNodes.push_back(PHI); }
  bool iterateProcessedPHI(PHINode *PHI) {
    for (auto Iter : VisitedPhiNodes) {
      if (Iter == PHI)
        return true;
    }
    return false;
  }
  bool iterateProcessedNodes(int TensorId, Loop *CurrLoop) {
    auto DiffCBegin = DiffCoords.lower_bound(TensorId);
    auto DiffCEnd = DiffCoords.upper_bound(TensorId);
    while (DiffCBegin != DiffCEnd) {
      if (TensorId == DiffCBegin->first &&
          CurrLoop == std::get<3>(DiffCBegin->second)) {
        // Loop ptr already exists, we need to update the counter
        return true;
      }
      DiffCBegin++;
    }
    return false;
  }

private:
  DenseMap<StringRef, unsigned> ConfigStartOffset;
  DenseMap<StringRef, unsigned> ConfigOffset;
  DenseMap<StringRef, unsigned> ArchTensorsBase;
  DenseMap<StringRef, unsigned> ArchTensorsDescSize;
  SmallVector<unsigned, 16> ArchLdStLVec;
  unsigned TensorSizeStrideArrOffset;

  SmallVector<Loop *, 8> &TensorLoops;
  ScalarEvolution *SE;
  LoopInfo *LI;
  SmallVector<SCEVInfo, 8> LoopSCEVInfo;
  bool HasLoadAndStore;

  SCEVInfo *getScevInfo(Loop *L) {
    for (unsigned i = 0; i < LoopSCEVInfo.size(); i++) {
      if (LoopSCEVInfo[i].getLoopPtr() == L)
        return &LoopSCEVInfo[i];
    }
    return nullptr;
  }

  SmallDenseSet<Instruction *> UniqueTensors;
  std::vector<Instruction *> Tensors;
  std::map<int, TensorType> TensorTypeMap;
  std::unordered_map<int, std::unordered_set<Loop *>> TensorLoopMapInfo;
  std::unordered_map<int, std::vector<std::tuple<Loop *, int, std::string>>>
      TensorCordInfoMap;
  int FakeTensorId, FakeTensorIdPad = int('x');
  bool IncorrectIndexSpace = false;

  Loop* getLoopPointerForOperand(Instruction *InsertInst);
  Loop *compareStepValuesForAddMask(PHINode *Phi, Loop *CurrLoop);
  void PrepareUnrollLoopIvCoordMap(Instruction *Inst, int TensorId);
  void processInsertElementNode(Instruction *InsertInst, Loop *CurrLoop,
                                PHINode *PHIPtr, unsigned TensorId);
  bool hasRuntimeDep(Value *V, std::string Indent="");
  void processNestedInstructions(Instruction *RootInst, Loop *CurrLoop,
                                 PHINode *PHI, int TensorId, bool CopyPHI,
                                 Function &F);
  void innerLoopUpdates(Instruction *T, int TensorId);
  TensorInfo getTensorInfo(unsigned TensorId);
  int64_t getAddMaskStepVar(IntrinsicInst *Instr);
  int64_t setIndexStepVar(Instruction *StepVar, Loop *CurrLoop);
  std::multimap<int, std::tuple<int, int, std::string, Loop *, PHINode *>>
      DiffCoords;
  std::multimap<int, std::tuple<int, std::string, std::string>>
      StartBEndBCoords;
  std::map<unsigned, unsigned> CopyIndSpace;
  std::map<unsigned, Instruction *> GenAddrMap;
  SmallVector<PHINode *, 8> VisitedPhiNodes;
  SmallVector<unsigned, 8> PragmaTensors;
  SmallDenseSet<unsigned> LdStLTensors;
  SmallDenseSet<unsigned> StLTensors;
  SmallDenseSet<unsigned> FallBackVec;
  SmallVector<unsigned, 16> TensorIDVecReduction;
  SmallVector<std::string, 8> ReductionOrNormAxes;
  DenseSet<Value *> RuntimeDepValues;
  DenseSet<Instruction *> RuntimeDepAnalyzedPHIs;

  void UpdateDiffCoords(int TensorId, int Index, int StrideVal,
                        std::string Formula, Loop *CurrLoop, PHINode *PHI) {
    auto DiffCBegin = DiffCoords.lower_bound(TensorId);
    auto DiffCEnd = DiffCoords.upper_bound(TensorId);
    bool Update = false;
    while (DiffCBegin != DiffCEnd) {
      if (TensorId == DiffCBegin->first &&
          Index == std::get<0>(DiffCBegin->second) &&
          CurrLoop == std::get<3>(DiffCBegin->second) &&
          PHI == std::get<4>(DiffCBegin->second)) {
        // Loop ptr already exists, we need to update the counter
        auto UpdatedVal = std::get<1>(DiffCBegin->second) + StrideVal;
        DiffCoords.erase(DiffCBegin);
        DiffCoords.insert({TensorId, std::make_tuple(Index, UpdatedVal, Formula,
                                                     CurrLoop, PHI)});
        Update = true;
        break;
      }
      DiffCBegin++;
    }
    if (!Update) {
      DiffCoords.insert({TensorId, std::make_tuple(Index, StrideVal, Formula,
                                                   CurrLoop, PHI)});
    }
  }

  bool is_ld_l_tnsr(Intrinsic::ID InId) {
    if (InId == Intrinsic::tpc_ld_l) {
      return true;
    }
    return false;
  }

  bool is_st_l_tnsr(Intrinsic::ID InId) {
    if (InId == Intrinsic::tpc_st_l) {
      return true;
    }
    return false;
  }

  bool is_gen_addr(Intrinsic::ID InId) {
    return (InId == Intrinsic::tpc_gen_addr);
  }

  bool is_ld_tnsr(Intrinsic::ID InId) {
    return (InId == Intrinsic::tpc_ld_tnsr ||
            InId == Intrinsic::tpc_ld_tnsr_high ||
            InId == Intrinsic::tpc_ld_tnsr_low ||
            InId == Intrinsic::tpc_ld_tnsr_partial ||
            InId == Intrinsic::tpc_gen_addr);
  }

  bool is_st_tnsr(Intrinsic::ID InId) {
    return (InId == Intrinsic::tpc_st_tnsr ||
            InId == Intrinsic::tpc_st_tnsr_high ||
            InId == Intrinsic::tpc_st_tnsr_low ||
            InId == Intrinsic::tpc_st_tnsr_low_rmw ||
            InId == Intrinsic::tpc_st_tnsr_partial ||
            InId == Intrinsic::tpc_st_tnsr_partial_rmw ||
            InId == Intrinsic::tpc_st_tnsr_rmw ||
            InId == Intrinsic::tpc_st_tnsr_sqz);
  }

  bool is_ld_st_tnsr(Intrinsic::ID InId) {
    return is_ld_tnsr(InId) || is_st_tnsr(InId);
  }

  int GetValId(Value *val) {
    if ((dyn_cast<llvm::ConstantInt>(val)) &&
        val->getValueID() == Value::ConstantIntVal) {
      return dyn_cast<llvm::ConstantInt>(val)->getZExtValue();
    } else
      return -1;
    }

  std::vector<unsigned> ld_id;
  std::vector<unsigned> st_id;
  std::vector<unsigned> Tensor_ids;
  int GetTensorId(Value *val) {
    if (val->getValueID() == Value::ConstantIntVal) {
      if (dyn_cast<llvm::ConstantInt>(val))
        return dyn_cast<llvm::ConstantInt>(val)->getZExtValue();
      else
        return -1;
    }
    return (FakeTensorIdPad + FakeTensorId++);
  }

  void prepareTensorLoopMap(int TensorId, Loop *L) {
    if (TensorLoopMapInfo.find(TensorId) == TensorLoopMapInfo.end()) {
      TensorLoopMapInfo[TensorId].insert(L);
      return;
    }
    TensorLoopMapInfo.at(TensorId).insert(L);
  }

  void classifyTensorType(int TensorId, Instruction *I) {
    auto *intrins = dyn_cast<IntrinsicInst>(I);
    if (!intrins) return;

    Intrinsic::ID inid = intrins->getIntrinsicID();
    assert(is_ld_st_tnsr(inid) &&
           "Invalid Intrinsic used for Tensor classification");

    auto TType = (is_ld_tnsr(inid) ? TensorType::Input : TensorType::Output);
    auto It = TensorTypeMap.find(TensorId);

    // if the Tensor is visited for the first time
    if (It == TensorTypeMap.end()) {
      TensorTypeMap[TensorId] = TType;
      if (is_gen_addr(inid))
        GenAddrMap.insert(std::make_pair(TensorId, I));
      return;
    }

    // Tensor is already classified earlier
    if (It->second != TType) {
      HasLoadAndStore = true;
      return;
    }

    // update GenAddrMap
    if (inid != Intrinsic::tpc_gen_addr)
      return;
    // first visit
    if (GenAddrMap.find(TensorId) == GenAddrMap.end()) {
      GenAddrMap.insert(std::make_pair(TensorId, I));
      return;
    }
    // compare loop depths for gen_addr entry
    if (auto CurrParent = LI->getLoopFor(I->getParent())) {
      auto PrevParent =
        LI->getLoopFor(GenAddrMap.at(TensorId)->getParent());
      if (!PrevParent ||
          (PrevParent->getLoopDepth() < CurrParent->getLoopDepth())) {
        GenAddrMap.erase(TensorId);
        GenAddrMap.insert(std::make_pair(TensorId, I));
      }
    }

    return;
  }

  void appendTensorIdString(unsigned TensorId, TensorInfo &TnsrInfo);
  void appendTensorIndexString(TensorInfo &TnsrInfo, bool FillGCCustom=true);
  void prepareTensorName(unsigned TensorId, TensorInfo &TnsrInfo, bool FillGCCustom=true);
  void prepareTensorName(unsigned TensorId, TensorInfo &TnsrInfo, const std::string IndexStr);

  void prepareLoopIvCoordMap(int TensorId, Loop *L, int index,
                             std::string formula = "") {
    if (TensorCordInfoMap.find(TensorId) == TensorCordInfoMap.end()) {
      TensorCordInfoMap[TensorId].push_back(std::make_tuple(L, index, formula));
      return;
    }
    std::vector<std::tuple<Loop *, int, std::string>>::iterator it =
        TensorCordInfoMap.at(TensorId).begin();
    // update
    for (; it < TensorCordInfoMap.at(TensorId).end(); ++it) {
      Loop *Loopit = std::get<0>(*it);
      int indexit = std::get<1>(*it);
      std::string formulait = std::get<2>(*it);
      if (Loopit == nullptr || L == nullptr) {
        if (index == indexit) {
          // Index is same but give priority to Formula
          if (formula.length() > 0) {
            // Before updating the existing entries in TensorCordInfoMap
            // check whether the new loop pointer has a valid SCEVInfo
            // Mostly for ignoring prologue loops
            if (!getScevInfo(L))
              return;
            TensorCordInfoMap[TensorId].erase(it);
            TensorCordInfoMap.at(TensorId).push_back(
                std::make_tuple(L, index, formula));
            return;
          }
          if (std::get<2>(*it).length() > 0) {
            return;
          }

          int64_t LoopStep1 = -1;
          int64_t LoopStep2 = -1, DiffCoordsStep = 0;
          //  Fetch required data from DiffCoords map
          auto DiffCBegin = DiffCoords.lower_bound(TensorId);
          auto DiffCEnd = DiffCoords.upper_bound(TensorId);
          int MaxVal = -1;
          while (DiffCBegin != DiffCEnd) {
            int Diffindex = std::get<0>(DiffCBegin->second);
            int DiffVal = std::get<1>(DiffCBegin->second);
            if (Diffindex == indexit) {
              if (DiffVal > MaxVal) {
                MaxVal = DiffVal;
              }
            }
            DiffCBegin++;
          }
          if (MaxVal >= 0) {
            DiffCoordsStep = MaxVal;
          }
          // index is match we need to compare the Scev
          if (Loopit != nullptr && L == nullptr) {
            if(getScevInfo(Loopit)){
            LoopStep1 = getScevInfo(Loopit)->getSCEVStep();
            }
            LoopStep2 = DiffCoordsStep;
          } else if (Loopit == nullptr && L != nullptr) {
            LoopStep1 = DiffCoordsStep;
            if (getScevInfo(L))
              LoopStep2 = getScevInfo(L)->getSCEVStep();
            // else 2 cases srise
            // either there was a constant update
            // or the loop cannot be processed altogether
            else
              continue;
          }
          if (LoopStep1 < LoopStep2) {
            // Simple remove the null Loop and add the new loop with this index
            TensorCordInfoMap[TensorId].erase(it);
            TensorCordInfoMap.at(TensorId).push_back(
                std::make_tuple(L, index, formula));
            return;
          } else {
            return;
          }
        }
        continue;
      }
      if (indexit == index) {
        // We need to get Max out of the 2 conflicting updates
        if (!getScevInfo(Loopit) || !getScevInfo(L))
          return;
        if (formula.length() > 0) {
          TensorCordInfoMap[TensorId].erase(it);
          TensorCordInfoMap.at(TensorId).push_back(
              std::make_tuple(L, index, formula));
          return;
        }
        if (std::get<2>(*it).length() > 0) {
          return;
        }
        int64_t LoopStep1 = getScevInfo(Loopit)->getSCEVStep();
        int64_t LoopStep2 = getScevInfo(L)->getSCEVStep();
        int64_t MaxLoopSteps = std::max(LoopStep1, LoopStep2);
        if (MaxLoopSteps != LoopStep1) {
          TensorCordInfoMap[TensorId].erase(it);
          TensorCordInfoMap.at(TensorId).push_back(
              std::make_tuple(L, index, formula));
        }
        return;
      } else {
        if (Loopit->getLoopDepth() == L->getLoopDepth()) {
          // Tensor & Loop is same and Index is not same
          continue;
        }
      }
    }
    // New
    if (it == TensorCordInfoMap.at(TensorId).end()) {
      TensorCordInfoMap.at(TensorId).push_back(
          std::make_tuple(L, index, formula));
    }
  }

  std::vector<TensorInfo> TensorInfos[2];
  std::string TensorTypeString[2] = { "Input", "Output" };
};

class TPCIndexGen : public FunctionPass {
public:
  static char ID;

  TPCIndexGen() : FunctionPass(ID) {
    initializeTPCIndexGenPass(*PassRegistry::getPassRegistry());
  }

  void getAnalysisUsage(AnalysisUsage &AU) const override {
    AU.setPreservesCFG();
    AU.addRequired<DominatorTreeWrapperPass>();
    AU.addPreserved<DominatorTreeWrapperPass>();
    AU.addRequired<LoopInfoWrapperPass>();
    AU.addPreserved<LoopInfoWrapperPass>();
    AU.addRequired<ScalarEvolutionWrapperPass>();
    AU.addPreserved<ScalarEvolutionWrapperPass>();
  }

private:
  bool runOnFunction(Function &F) override;
  void findTensorLoops(Function &F, bool LoopFlag);

  ScalarEvolution *SE;
  LoopInfo *LI;
  SmallVector<Loop *, 8> TensorLoops;
  VectorType *Int5Ty;
};

#endif // LLVM_TPC_INDEX_SPACE_GEN_CPP_H
