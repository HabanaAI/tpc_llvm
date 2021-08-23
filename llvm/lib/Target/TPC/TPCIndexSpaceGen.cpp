//===- TPCIndexSpace.cpp --- TPC INDEX SPACE ------------------------------===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//
//===----------------------------------------------------------------------===//

#include "TPCIndexSpaceGen.h"
#include "llvm/Support/Debug.h"

#define INVALID_SCEV 9999
#define INVALID_STRIDE 999

static cl::opt<bool> IndexSpaceWarning("index-space-warn", cl::init(false),
                                       cl::Hidden);

static cl::opt<bool> IndexSpaceMLIR("index-space-mlir", cl::init(false),
                                       cl::Hidden);
static cl::opt<bool>
    EmitIndexFactors("emit-index-factors",
                     cl::desc("Enable index space generation."), cl::init(true),
                     cl::ZeroOrMore, cl::Hidden);

char TPCIndexGen::ID = 0;
INITIALIZE_PASS_BEGIN(TPCIndexGen, PassName, PassDescription, false, false)
INITIALIZE_PASS_END(TPCIndexGen, PassName, PassDescription, false, false)
FunctionPass *llvm::createTPCIndexGen() { return new TPCIndexGen(); }

void TPCIndexGen::findTensorLoops(Function &F, bool LoopFlag = 0) {
  // Iterating through LI gives top level loops.
  for (Loop *TopLevelLoop : *LI) {
    for (Loop *L : depth_first(TopLevelLoop)) {
      bool HasInt5 = false;

      for (PHINode &PHI : L->getHeader()->phis()) {
        if (PHI.getType() == Int5Ty) {
          HasInt5 = true;
          break;
        }
      }
      // TODO : Include all loops
      if (HasInt5 || LoopFlag)
        TensorLoops.push_back(L);
    }
  }
}

llvm::GlobalVariable *processIndexSpace(Module *M, StringRef Name,
                                        std::string TestString, Type *Int8Ty) {
  std::vector<Constant *> GVConst;
  ArrayType *ATy;
  for (auto const &C : TestString) {
    GVConst.push_back(ConstantInt::get(Int8Ty, C));
  }
  ATy = ArrayType::get(Int8Ty, GVConst.size());
  return (new llvm::GlobalVariable(*M, ATy, false, GlobalValue::ExternalLinkage,
                                   ConstantArray::get(ATy, GVConst), Name,
                                   nullptr));
}

unsigned fetchIndexFromOperand(unsigned Operand) {
  unsigned Count = 0;
  while (Operand) {
    Operand = Operand >> 1;
    ++Count;
  }
  if (Count)
    return (Count - 1);
  else
    return Count;
}

bool TPCIndexGen::runOnFunction(Function &F) {
  bool InnerUpdatesOnly = 0, ZeroLoopCount = 0;
  if (!EmitIndexFactors)
    return false;

  SE = &getAnalysis<ScalarEvolutionWrapperPass>().getSE();
  LI = &getAnalysis<LoopInfoWrapperPass>().getLoopInfo();
  Int5Ty = VectorType::get(Type::getInt32Ty(F.getContext()), 5);

  findTensorLoops(F);

  if (TensorLoops.size() == 0 || TensorLoops.size() == 1 ||
      TensorLoops.size() == 2) {
    if (!TensorLoops.size())
      ZeroLoopCount = 1;
    InnerUpdatesOnly = 1;
    findTensorLoops(F, InnerUpdatesOnly);
  }
  TensorAccessAnalysis TAA(TensorLoops, SE, LI);
  if (!ZeroLoopCount)
    InnerUpdatesOnly = 0;
  TAA.prepareLdStLData(F);
  TAA.prepareStLData(F);
  TAA.processLoadStoreLocalInfo(F);
  TAA.processPragmaInfo(F);
  TAA.computeTensorInfo(F, InnerUpdatesOnly);
  if(!IndexSpaceMLIR)
    TAA.dumpIntoASM();
  else
    TAA.dumpIntoASM_MLIR();
  if (TAA.getIndexSpaceBool()) {
    LLVM_DEBUG(dbgs() << "Caught an exception");
    return false;
  }
  // Dump data into a separate asm section
  // const_cast is needed to adhere to GlobalVariable constructor
  llvm::Module *M = const_cast<llvm::Module *>(F.getFunction().getParent());
  StringRef Name = "SCEVCost From TPC Index";
  Type *Int8Ty = llvm::Type::getInt8Ty(F.getFunction().getContext());
  std::string IndexMapString = " SCEVBEGIN IndexSpace:";
  TAA.updateIndSpaceCoords();
  if(!IndexSpaceMLIR){
    TAA.analyseGenAddr();
    TAA.updateAddMask();
    TAA.padGCCustom();
  }
  for (const auto &Input : TAA.getInputVector())
    IndexMapString += Input.Name;
  for (const auto &Output : TAA.getOutputVector())
    IndexMapString += Output.Name;
  for (const auto &Aux : TAA.getAuxVector())
    IndexMapString += Aux.Name;
  for (const auto &GC : TAA.getGCCustomVec())
    IndexMapString += GC.Name;
  IndexMapString += " #SCEVEND";
  if (F.getMetadata("reduction_or_norm_axes")) {
    IndexMapString += " ReductionNormAxes- IDs : { ";
    for (const auto &ID : TAA.getRedNormTensorVec())
      IndexMapString += std::to_string(ID) + " ";
    IndexMapString += "}";
    IndexMapString += " Axes : { ";
    for (const auto &Axis : TAA.getRedNormTensorAxes())
      IndexMapString += Axis + " ";
    IndexMapString += "}";
  }
  if (F.getMetadata("index_space_b")) {
    IndexMapString += " Index Space-B : ";
    std::string StartEndBStr = "";
    int PrevID = -1;
    for (auto Iter : TAA.getStartBEndBCoords()) {
      if (PrevID == -1) {
        PrevID = Iter.first;
        StartEndBStr += "[" + std::to_string(PrevID) + "] {";
        StartEndBStr +=
            std::get<1>(Iter.second) + ":" + std::get<2>(Iter.second) + " ";
      } else if (Iter.first == PrevID) {
        StartEndBStr +=
            std::get<1>(Iter.second) + ":" + std::get<2>(Iter.second) + " ";
      } else if (PrevID != -1) {
        IndexMapString += StartEndBStr + " }";
        StartEndBStr = "";
        PrevID = Iter.first;
        StartEndBStr += "[" + std::to_string(PrevID) + "] {";
        StartEndBStr +=
            std::get<1>(Iter.second) + ":" + std::get<2>(Iter.second) + " ";
      }
    }
    IndexMapString += StartEndBStr + " }";
  }

  llvm::GlobalVariable *SCEVGV =
      processIndexSpace(M, Name, IndexMapString, Int8Ty);
  // LLVM-1766: In case index space analysis is returned with GC_CUSTOM. MLIR expects the compilation to be failed.
  if(IndexSpaceMLIR && IndexMapString.find("GC_CUSTOM") != std::string::npos){
        errs() << "GC_CUSTOM is not supported by MLIR path !\n";
        exit(1);
  }

  SCEVGV->setSection(".IndexMap");

  if (TAA.BailoutReason.empty())
    TAA.BailoutReason = "No bailouts required";
  LLVM_DEBUG({
    llvm::GlobalVariable *Bailout =
        processIndexSpace(M, "Bailout Reason : ", TAA.BailoutReason, Int8Ty);
    Bailout->setSection(".BailoutGCCUSTOM");
  });

  // Dump unroll informartion.
  MDNode *MD = F.getMetadata("unroll_info");
  if (!MD)
    return false;
  std::string UnrollInfoStr = "";
  for (Metadata *Arg : MD->operands()) {
    if (auto *MDStr = dyn_cast<MDString>(Arg)) {
      auto Str = MDStr->getString();
      UnrollInfoStr += Str.str();
      UnrollInfoStr += "; ";
    }
  }

  SmallVector<Constant *, 64> UnrollInfoVec;
  for (auto const &C : UnrollInfoStr)
    UnrollInfoVec.push_back(ConstantInt::get(Int8Ty, C));
  ArrayType *ATy = ArrayType::get(Int8Ty, UnrollInfoVec.size());
  auto *UnrollInfoGV = new llvm::GlobalVariable(
      *M, ATy, false, GlobalValue::ExternalLinkage,
      ConstantArray::get(ATy, UnrollInfoVec), "Loop nest unroll Info", nullptr);
  UnrollInfoGV->setSection(".UnrollInfo");
  return false;
}

void TensorAccessAnalysis::padGCCustom() {
  bool FallBack = false;
  int TensorID = -1;
  for (const auto &CurrInput : Input) {
    auto IndexMapString = CurrInput.Name;
    if (IndexMapString.find("GC_CUSTOM") != std::string::npos) {
      FallBack = true;
      break;
    }
  }
  for (const auto &CurrOutput : Output) {
    auto IndexMapString = CurrOutput.Name;
    if (IndexMapString.find("GC_CUSTOM") != std::string::npos) {
      FallBack = true;
      break;
    }
  }
  if (FallBack) {
    for (auto &TensorInfo : Input) {
      auto IndexMapString = TensorInfo.Name;
      if (IndexMapString.find("GC_CUSTOM") == std::string::npos) {
        if (TensorInfo.Order >= FakeTensorIdPad) { // FakeTensorId
          std::string Temp =
              "x" + std::to_string(TensorInfo.Order - FakeTensorIdPad);
          TensorInfo.Name = "[" + Temp + "].";
        } else
          TensorInfo.Name = "[" + std::to_string(TensorInfo.Order) + "].";
        TensorInfo.Name += "{ GC_CUSTOM";
        // prepare TensorName
        for (unsigned i = 1; i < TensorInfo.IndexFactor.capacity(); i++)
          TensorInfo.Name += ", GC_CUSTOM";
        TensorInfo.Name += " }";
      }
    }
    for (auto &TensorInfo : Output) {
      auto IndexMapString = TensorInfo.Name;
      if (IndexMapString.find("GC_CUSTOM") == std::string::npos) {
        if (TensorInfo.Order >= FakeTensorIdPad) { // FakeTensorId
          std::string Temp =
              "x" + std::to_string(TensorInfo.Order - FakeTensorIdPad);
          TensorInfo.Name = "[" + Temp + "].";
        } else
          TensorInfo.Name = "[" + std::to_string(TensorInfo.Order) + "].";
        TensorInfo.Name += "{ GC_CUSTOM";
        // prepare TensorName
        for (unsigned i = 1; i < TensorInfo.IndexFactor.capacity(); i++)
          TensorInfo.Name += ", GC_CUSTOM";
        TensorInfo.Name += " }";
      }
    }
  }
}

void TensorAccessAnalysis::updateAddMask() {
  for (auto CurrTensor : FallBackVec) {
    auto TensorInfo = getTensorInfo(CurrTensor);
    // change Tensor Access Pattern
    if (TensorTypeMap.find(CurrTensor) == TensorTypeMap.end())
      continue;
    if (TensorInfo.Order == INVALID_SCEV)
      continue;
    if (CurrTensor >= FakeTensorIdPad) { // FakeTensorId
      std::string Temp = "x" + std::to_string(CurrTensor - FakeTensorIdPad);
      TensorInfo.Name = "[" + Temp + "].";
    } else
      TensorInfo.Name = "[" + std::to_string(CurrTensor) + "].";
    if (BailoutReason.empty())
      BailoutReason = "Add mask has pred dependency";
    std::string IndexStr = "{ GC_CUSTOM";
    // prepare TensorName
    for (unsigned i = 1; i < TensorInfo.IndexFactor.capacity(); i++)
      IndexStr += ", GC_CUSTOM";
    IndexStr += " }";
    if (TensorInfo.Type == TensorType::Input) {
      TensorInfo.Name += "[Input].";
      TensorInfo.Name += IndexStr;
      Input.push_back(TensorInfo);
    }
    if (TensorInfo.Type == TensorType::Output) {
      TensorInfo.Name += "[Output].";
      TensorInfo.Name += IndexStr;
      Output.push_back(TensorInfo);
    }
  }
}

void TensorAccessAnalysis::analyseGenAddr() {
  for (auto Iter : GenAddrMap) {
    unsigned CurrTensor = Iter.first;
    Instruction *I = Iter.second;
    bool IntrinsUsage = true;
    const auto &TempUse = I->use_begin();
    if (TempUse == I->use_end())
      continue;
    User *TempUser = TempUse->getUser();
    if (auto *InstrPtrTemp = dyn_cast<IntrinsicInst>(TempUser)) {
      if (InstrPtrTemp->getIntrinsicID() != Intrinsic::tpc_ld_g)
        continue;
    }
    std::vector<Instruction *> PreventCycle;
    while (IntrinsUsage) {
      IntrinsUsage = false;
      const auto &GenAddrUse = I->use_begin();
      if (GenAddrUse == I->use_end())
        break;
      User *GenAddrUser = GenAddrUse->getUser();
      if (auto *ICmp = dyn_cast<ICmpInst>(GenAddrUser)) {
        // change Tensor Access Pattern
        if (TensorTypeMap.find(CurrTensor) == TensorTypeMap.end())
          break;
        auto Tensor1Info = getTensorInfo(CurrTensor);
        if (Tensor1Info.Order == INVALID_SCEV)
          break;
        if (CurrTensor >= FakeTensorIdPad) { // FakeTensorId
          std::string Temp = "x" + std::to_string(CurrTensor - FakeTensorIdPad);
          Tensor1Info.Name = "[" + Temp + "].";
        } else
          Tensor1Info.Name = "[" + std::to_string(CurrTensor) + "].";
        std::string IndexStr = "{ GC_CUSTOM";
        if (BailoutReason.empty())
          BailoutReason = "Gen Addr Determines Loop Bounds";
        // prepare TensorName
        for (unsigned i = 1; i < Tensor1Info.IndexFactor.capacity(); i++)
          IndexStr += ", GC_CUSTOM";
        IndexStr += " }";
        if (Tensor1Info.Type == TensorType::Input) {
          Tensor1Info.Name += "[Input].";
          Tensor1Info.Name += IndexStr;
          Input.push_back(Tensor1Info);
        }
        if (Tensor1Info.Type == TensorType::Output) {
          Tensor1Info.Name += "[Output].";
          Tensor1Info.Name += IndexStr;
          Output.push_back(Tensor1Info);
        }
        break;
      } else if (auto *InstrPtr = dyn_cast<Instruction>(GenAddrUser)) {
        if (InstrPtr->getParent() != I->getParent()) {
          bool FindOneUse = false;
          for (const Use &IUse : I->uses()) {
            User *IUser = IUse.getUser();
            if (InstrPtr = dyn_cast<Instruction>(IUser)) {
              if (InstrPtr->getParent() == I->getParent()) {
                I = InstrPtr;
                if (std::find(PreventCycle.begin(), PreventCycle.end(),
                              InstrPtr) != PreventCycle.end())
                  break;
                else
                  PreventCycle.push_back(InstrPtr);
                IntrinsUsage = true;
                FindOneUse = true;
                break;
              } else
                PreventCycle.push_back(InstrPtr);
            }
          }
          if (!FindOneUse)
            IntrinsUsage = false;
        } else {
          if (std::find(PreventCycle.begin(), PreventCycle.end(), InstrPtr) !=
              PreventCycle.end())
            break;
          PreventCycle.push_back(InstrPtr);
          I = InstrPtr;
          IntrinsUsage = true;
        }
      }
    }
  }
}

TensorInfo TensorAccessAnalysis::getTensorInfo(unsigned TensorId) {
  auto Type1 = TensorTypeMap.at(TensorId);
  if (Type1 == TensorType::Input) {
    for (auto InputIt = Input.begin(); InputIt != Input.end(); ++InputIt) {
      TensorInfo Temp = *InputIt;
      if (Temp.Order == TensorId) {
        if (!Temp.IndexFactor.size())
          continue;
        Input.erase(InputIt);
        return Temp;
      }
    }
    TensorInfo *Temp = new TensorInfo;
    Temp->Order = INVALID_SCEV;
    return *Temp;
  } else if (Type1 == TensorType::Output) {
    for (auto OutputIt = Output.begin(); OutputIt != Output.end(); ++OutputIt) {
      TensorInfo Temp = *OutputIt;
      if (Temp.Order == TensorId) {
        Output.erase(OutputIt);
        return Temp;
      }
    }
    TensorInfo *Temp = new TensorInfo;
    Temp->Order = INVALID_SCEV;
    return *Temp;
  }
}

void TensorAccessAnalysis::updateIndSpaceCoords() {
  for (auto Iter = CopyIndSpace.begin(); Iter != CopyIndSpace.end(); Iter++) {
    auto Tensor1Info = getTensorInfo(Iter->first);
    auto Tensor2Info = getTensorInfo(Iter->second);
    if (Tensor1Info.Order == INVALID_SCEV || Tensor2Info.Order == INVALID_SCEV)
      continue;
    for (int i = 0; i < Tensor1Info.IndexFactor.size(); i++) {
      if (Tensor1Info.IndexFactor[i] > Tensor2Info.IndexFactor[i])
        Tensor2Info.IndexFactor[i] = Tensor1Info.IndexFactor[i];
      else
        Tensor1Info.IndexFactor[i] = Tensor2Info.IndexFactor[i];
    }

    Tensor1Info.Name = "[" + std::to_string(Iter->first) + "].";
    std::string IndexStr = "";
    auto Formula = Tensor1Info.umap.find(0);
    if (Formula != Tensor1Info.umap.end()) {
      IndexStr = "{ " + Formula->second + "*";
      IndexStr += std::to_string(Tensor1Info.IndexFactor[0]);
    } else
      IndexStr = "{ " + std::to_string(Tensor1Info.IndexFactor[0]);
    for (unsigned i = 1; i < Tensor1Info.IndexFactor.size(); i++) {
      Formula = Tensor1Info.umap.find(i);
      if (Formula != Tensor1Info.umap.end()) {
        IndexStr += ", " + Formula->second + "*";
        IndexStr += std::to_string(Tensor1Info.IndexFactor[i]);
      } else
        IndexStr += ", " + std::to_string(Tensor1Info.IndexFactor[i]);
    }

    IndexStr += " }";
    if (Tensor1Info.Type == TensorType::Input) {
      Tensor1Info.Name += "[Input].";
      Tensor1Info.Name += IndexStr;
      Input.push_back(Tensor1Info);
    }
    if (Tensor1Info.Type == TensorType::Output) {
      Tensor1Info.Name += "[Output].";
      Tensor1Info.Name += IndexStr;
      Output.push_back(Tensor1Info);
    }

    Tensor2Info.Name = "[" + std::to_string(Iter->second) + "].";
    Formula = Tensor2Info.umap.find(0);
    if (Formula != Tensor2Info.umap.end()) {
      IndexStr = "{ " + Formula->second + "*";
      IndexStr += std::to_string(Tensor2Info.IndexFactor[0]);
    } else
      IndexStr = "{ " + std::to_string(Tensor2Info.IndexFactor[0]);

    for (unsigned i = 1; i < Tensor2Info.IndexFactor.size(); i++) {
      Formula = Tensor2Info.umap.find(i);
      if (Formula != Tensor2Info.umap.end()) {
        IndexStr += ", " + Formula->second + "*";
        IndexStr += std::to_string(Tensor2Info.IndexFactor[i]);
      } else
        IndexStr += ", " + std::to_string(Tensor2Info.IndexFactor[i]);
    }

    IndexStr += " }";
    if (Tensor2Info.Type == TensorType::Input) {
      Tensor2Info.Name += "[Input].";
      Tensor2Info.Name += IndexStr;
      Input.push_back(Tensor2Info);
    }
    if (Tensor2Info.Type == TensorType::Output) {
      Tensor2Info.Name += "[Output].";
      Tensor2Info.Name += IndexStr;
      Output.push_back(Tensor2Info);
    }
  }
}

int64_t TensorAccessAnalysis::getAddMaskStepVar(IntrinsicInst *Instr) {
  // check if operand 0 is a constant Vec
  if (auto *ConstantVec = dyn_cast<ConstantDataVector>(Instr->getOperand(0))) {
    if (auto *ConstantVal = ConstantVec->getSplatValue()) {
      if (auto ElementVal = dyn_cast<ConstantInt>(ConstantVal)->getSExtValue())
        return ElementVal;
    }
  } else if (auto *ConstPtr = dyn_cast<ConstantInt>(Instr->getOperand(1))) {
    int ConstStride = ConstPtr->getSExtValue();
    return ConstStride;
  }
  return INVALID_STRIDE; // Bail out and continue with loop pointer
}

void TensorAccessAnalysis::processPragmaInfo(Function &F) {
  bool prevOperandString = false;
  int DimIndex = 0;

  // Begin Reduction/Norm Axes Evaluation
  if (MDNode *PragmaIdxAxis = F.getMetadata("reduction_or_norm_axes")) {
    for (unsigned i = 0; i < PragmaIdxAxis->getNumOperands(); ++i) {
      if (auto *TensorIDPtr =
              dyn_cast<ValueAsMetadata>(PragmaIdxAxis->getOperand(i))) {
        auto *TensorIDPtr2 = TensorIDPtr->getValue();
        if (auto TensorID = dyn_cast<ConstantInt>(TensorIDPtr2)) {
          auto TensorIDVal = TensorID->getZExtValue();
          TensorIDVecReduction.push_back(TensorIDVal);
        }
      }
      if (auto *IndSpacePtr = dyn_cast<MDString>(PragmaIdxAxis->getOperand(i)))
        ReductionOrNormAxes.push_back(IndSpacePtr->getString());
    }
  }
  // End Reduction/Norm Axes Evaluation

  // Start b End b begin
  if (MDNode *PragmaIdxSpace = F.getMetadata("index_space_b")) {
    SmallVector<unsigned, 8> TensorIDVec;
    for (unsigned i = 0; i < PragmaIdxSpace->getNumOperands();) {
      if (auto *TensorIDPtr =
              dyn_cast<ValueAsMetadata>(PragmaIdxSpace->getOperand(i))) {
        if (prevOperandString) {
          TensorIDVec.clear();
          prevOperandString = false;
          DimIndex = 0;
        }
        auto *TensorIDPtr2 = TensorIDPtr->getValue();
        if (auto TensorID = dyn_cast<ConstantInt>(TensorIDPtr2)) {
          auto TensorIDVal = TensorID->getZExtValue();
          TensorIDVec.push_back(TensorIDVal);
          ++i;
        }
      }
      auto *StartBPtr = dyn_cast<MDString>(PragmaIdxSpace->getOperand(i));
      auto *EndBPtr = dyn_cast<MDString>(PragmaIdxSpace->getOperand(i + 1));
      if (StartBPtr && EndBPtr) {
        auto StringRepStartB = StartBPtr->getString();
        auto StringRepEndB = EndBPtr->getString();
        for (auto TensorID : TensorIDVec)
          StartBEndBCoords.insert(
              {TensorID,
               std::make_tuple(DimIndex, StringRepStartB, StringRepEndB)});
        ++DimIndex;
        prevOperandString = true;
        i += 2;
      }
    }
  }
  // Start b End b end

  if (MDNode *PragmaIdxSpace = F.getMetadata("index_space")) {
    SmallVector<unsigned, 8> TensorIDVec;
    for (unsigned i = 0; i < PragmaIdxSpace->getNumOperands(); ++i) {
      if (auto *TensorIDPtr =
              dyn_cast<ValueAsMetadata>(PragmaIdxSpace->getOperand(i))) {
        if (prevOperandString) {
          TensorIDVec.clear();
          prevOperandString = false;
          DimIndex = 0;
        }
        auto *TensorIDPtr2 = TensorIDPtr->getValue();
        if (auto TensorID = dyn_cast<ConstantInt>(TensorIDPtr2)) {
          auto TensorIDVal = TensorID->getZExtValue();
          TensorIDVec.push_back(TensorIDVal);
          PragmaTensors.push_back(TensorIDVal);
        }
      }
      if (auto *IndSpacePtr =
              dyn_cast<MDString>(PragmaIdxSpace->getOperand(i))) {
        auto StringRep = IndSpacePtr->getString();
        long long IntegerRep;
        bool Flag = getAsSignedInteger(StringRep, 10, IntegerRep);
        if (!Flag) {
          for (auto Iter : TensorIDVec)
            updateCoordFactor(Iter, DimIndex, IntegerRep, "", nullptr);
        } else { // Handle formula here
          for (auto Iter : TensorIDVec)
            updateCoordFactor(Iter, DimIndex, 1, StringRep, nullptr);
        }
        ++DimIndex;
        prevOperandString = true;
      }
    }
  }
}

void TensorAccessAnalysis::prepareLdStLData(Function &F) {
  Attribute SubArchAttr = F.getFnAttribute("target-cpu");
  auto SubArchStr = SubArchAttr.getValueAsString();
  if (ArchTensorsBase.find(SubArchStr) == ArchTensorsBase.end())
    return;
  unsigned ArchBase = ArchTensorsBase[SubArchStr];
  unsigned Offset = ArchBase + TensorSizeStrideArrOffset;
  for (unsigned i = 0; i < 16; i++) {
    unsigned RangeStart = Offset + i * ArchTensorsDescSize[SubArchStr];
    ArchLdStLVec.push_back(RangeStart);
  }
}

void TensorAccessAnalysis::prepareStLData(Function &F) {
  for (auto &BB : F)
    for (auto &II : BB)
      if (auto *Intrins = dyn_cast<IntrinsicInst>(&II)) {
        Intrinsic::ID Inid = Intrins->getIntrinsicID();
        if (is_st_l_tnsr(Inid)) {
          if (auto *Num = dyn_cast<ConstantInt>(Intrins->getOperand(0))) {
            unsigned NumVal = Num->getZExtValue();
            if (NumVal >= ArchLdStLVec.front() &&
                NumVal <= ArchLdStLVec.back()) {
              unsigned TensorId = std::lower_bound(ArchLdStLVec.begin(),
                                                   ArchLdStLVec.end(), NumVal) -
                                  ArchLdStLVec.begin() - 1;
              if (TensorId == -1)
                TensorId = 0;
              StLTensors.insert((TensorId));
              if (BailoutReason.empty())
                BailoutReason = "Reshape in one of the Tensors";
            }
          }
        }
      }
}

void TensorAccessAnalysis::processLoadStoreLocalInfo(Function &F) {
  for (auto &BB : F)
    for (auto &II : BB)
      if (auto *Intrins = dyn_cast<IntrinsicInst>(&II)) {
        Intrinsic::ID Inid = Intrins->getIntrinsicID();
        if (is_ld_l_tnsr(Inid)) {
          if (auto *Num = dyn_cast<ConstantInt>(Intrins->getOperand(0))) {
            for (const Use &LdTensorUse : Intrins->uses()) {
              User *LdTensorUser = LdTensorUse.getUser();
              if (auto *InstrPtr = dyn_cast<IntrinsicInst>(LdTensorUser)) {
                Inid = InstrPtr->getIntrinsicID();
                unsigned NumVal = Num->getZExtValue();
                if (is_st_l_tnsr(Inid) && NumVal >= ArchLdStLVec.front() &&
                    NumVal <= ArchLdStLVec.back()) {
                  int TensorId = std::lower_bound(ArchLdStLVec.begin(),
                                                  ArchLdStLVec.end(), NumVal) -
                                 ArchLdStLVec.begin() - 1;
                  if (TensorId == -1)
                    TensorId = 0;
                  LdStLTensors.insert(TensorId);
                }
              }
            }
          }
        }
      }
}

Value *getNextInstruction(Instruction *Ptr) {
  Value *InitTensor = nullptr;
  if (dyn_cast<InsertElementInst>(Ptr)) {
    InitTensor = Ptr->getOperand(0);
    if (!(dyn_cast<Instruction>(InitTensor))) {
      InitTensor = Ptr->getOperand(1);
    }
    return InitTensor;
  } else if (dyn_cast<ShuffleVectorInst>(Ptr)) {
    InitTensor = Ptr->getOperand(0);
    if (!(dyn_cast<Instruction>(InitTensor))) {
      InitTensor = Ptr->getOperand(1);
    }
    return InitTensor;
  } else if (auto *Intrins = dyn_cast<IntrinsicInst>(Ptr)) {
    Intrinsic::ID Inid = Intrins->getIntrinsicID();
    if (Inid == Intrinsic::tpc_add_mask || Inid == Intrinsic::tpc_add) {
      InitTensor = Ptr->getOperand(0);
      if (!(dyn_cast<Instruction>(InitTensor))) {
        InitTensor = Ptr->getOperand(1);
      }
    } else
      InitTensor = Ptr->getOperand(0);
  } else
    InitTensor = Ptr->getOperand(0);
  return InitTensor;
}

Loop *TensorAccessAnalysis::getLoopPointerForOperand(Instruction *InsertInst) {
  auto InitialVal = InsertInst->getOperand(1);
  while (InitialVal) {
    if (Instruction *InstPtr = dyn_cast<Instruction>(InitialVal)) {
      if (InstPtr->getOpcode() != Instruction::PHI) {
        InitialVal = InstPtr->getOperand(0);
        continue;
      } else {
        auto PHIPtr = dyn_cast<PHINode>(InstPtr);
        return (LI->getLoopFor(PHIPtr->getParent()));
      }
    } else {
      return nullptr;
    }
  }
  return nullptr;
}

void TensorAccessAnalysis::processSetIndexNode(int TensorId, Loop *CurrLoop,
                                               PHINode *PHI, Function &F,
                                               IntrinsicInst *SetIndexInst) {

  int IndexShl =
      dyn_cast<llvm::ConstantInt>(SetIndexInst->getOperand(1))->getZExtValue();
  unsigned Index = fetchIndexFromOperand(IndexShl);
  std::string Formula = getFormula(SetIndexInst, F);
  if (auto *StepVar = dyn_cast<Instruction>(SetIndexInst->getOperand(2))) {
    if (StepVar->getOpcode() != Instruction::PHI) {
      // Attempt at fetching proper index stride
      int64_t DiffStep = setIndexStepVar(StepVar, CurrLoop);
      if (DiffStep != INVALID_SCEV) {
        DiffCoords.insert(
            {TensorId, std::make_tuple(Index, DiffStep, "", nullptr, PHI)});
        prepareLoopIvCoordMap(TensorId, nullptr, Index, Formula);
        processNestedInstructions(SetIndexInst, CurrLoop, PHI, TensorId, false,
                                  F);
        return;
      }
    }
    // Now check whether set index depends on the loop induction variable
    if (LI->isLoopHeader(PHI->getParent())) {
      if (auto *StepVar = dyn_cast<Instruction>(SetIndexInst->getOperand(2)))
        prepareLoopIvCoordMap(TensorId, CurrLoop, Index, Formula);
    }
  }
}

void TensorAccessAnalysis::processInsertElementNode(Instruction *InsertInst,
                                                    Loop *CurrLoop,
                                                    PHINode *PHI,
                                                    unsigned TensorId) {
  if (dyn_cast<llvm::ConstantInt>(InsertInst->getOperand(2))) {
    int index =
        dyn_cast<llvm::ConstantInt>(InsertInst->getOperand(2))->getZExtValue();
    if (LI->isLoopHeader(PHI->getParent())) {
      Loop *ParentLoop = getLoopPointerForOperand(InsertInst);
      if (ParentLoop) {
        if (auto *StrideArg =
                dyn_cast<Instruction>(InsertInst->getOperand(1))) {
          // check if the operand 1 is itself a binary instruction
          if (auto BO = dyn_cast<BinaryOperator>(StrideArg)) {
            if (dyn_cast<llvm::ConstantInt>(BO->getOperand(1))) {
              int Val = dyn_cast<llvm::ConstantInt>(BO->getOperand(1))
                            ->getZExtValue();
              if (BO->getOpcode() == Instruction::Add) {
                unsigned UnrollCount = 0;
                if (getScevInfo(ParentLoop)) {
                  UnrollCount =
                      getScevInfo(ParentLoop)->getLoopUnrollCount(ParentLoop);
                }
                if (UnrollCount)
                  Val = Val * UnrollCount;
                // We have this block for only negative updates
                if (Val < 0) {
                  UpdateDiffCoords(TensorId, index, Val, "", CurrLoop, PHI);
                  prepareLoopIvCoordMap(TensorId, nullptr, index);
                  return;
                } else
                  return;
              } else if (BO->getOpcode() == Instruction::Or) {
                // Or is present for strength reduction only
                // Ignore the updates here and acknowledge only
                // the loop stride increment
                return;
              } else if (BO->getOpcode() == Instruction::Shl) {
                auto ShiftVal =
                    dyn_cast<ConstantInt>(BO->getOperand(1))->getZExtValue();
                UpdateDiffCoords(
                    TensorId, index,
                    (getScevInfo(CurrLoop)->getSCEVStep() << ShiftVal), "",
                    CurrLoop, PHI);
                prepareLoopIvCoordMap(TensorId, nullptr, index);
                return;
              }
            } else
              updateCoordFactor(TensorId, index, INVALID_SCEV, "", nullptr);
          }
          for (const Use &StrideArgUse : StrideArg->uses()) {
            User *StrideArgUser = StrideArgUse.getUser();
            if (auto *InstrPtr = dyn_cast<Instruction>(StrideArgUser)) {
              if (auto BO = dyn_cast<BinaryOperator>(StrideArgUser)) {
                if (!dyn_cast<llvm::ConstantInt>(BO->getOperand(1))) {
                  updateCoordFactor(TensorId, index, INVALID_SCEV, "", nullptr);
                  continue;
                }
                int Val = dyn_cast<llvm::ConstantInt>(BO->getOperand(1))
                              ->getZExtValue();
                if (BO->getOpcode() == Instruction::Add) {
                  unsigned UnrollCount = 0;
                  if(getScevInfo(ParentLoop)){
                  UnrollCount =
                      getScevInfo(ParentLoop)->getLoopUnrollCount(ParentLoop);
                  }
                  if (UnrollCount)
                    Val = Val * UnrollCount;
                  DiffCoords.insert({TensorId, std::make_tuple(index, Val, "",
                                                               nullptr, PHI)});
                  prepareLoopIvCoordMap(TensorId, nullptr, index);
                  return;
                }
              }
            }
          }
          // Let's check if operand 1 is Phi node then use this as SCEV
          if (Instruction *Phi = dyn_cast<PHINode>(InsertInst->getOperand(1))) {
            if (Phi == CurrLoop->getInductionVariable(*SE))
              prepareLoopIvCoordMap(TensorId, ParentLoop, index);
            return;
          }
        }
        // failsafe
        updateCoordFactor(TensorId, index, INVALID_SCEV, "", nullptr);
      } else {
        if (auto *StrideArg =
                dyn_cast<Instruction>(InsertInst->getOperand(1))) {
          for (const Use &StrideArgUse : StrideArg->uses()) {
            User *StrideArgUser = StrideArgUse.getUser();
            if (auto *InstrPtr = dyn_cast<Instruction>(StrideArgUser)) {
              if (auto BO = dyn_cast<BinaryOperator>(StrideArgUser)) {
                if (!dyn_cast<llvm::ConstantInt>(BO->getOperand(1))) {
                  continue;
                }
                int Val = dyn_cast<llvm::ConstantInt>(BO->getOperand(1))
                              ->getZExtValue();
                if (BO->getOpcode() == Instruction::Add) {
                  unsigned UnrollCount = 0;
                  if (getScevInfo(ParentLoop)) {
                    UnrollCount =
                        getScevInfo(ParentLoop)->getLoopUnrollCount(ParentLoop);
                  }
                  if (UnrollCount)
                    Val = Val * UnrollCount;
                  DiffCoords.insert({TensorId, std::make_tuple(index, Val, "",
                                                               nullptr, PHI)});
                  prepareLoopIvCoordMap(TensorId, nullptr, index);
                  return;
                }
              }
            }
          }
        }
      }
    }
  }
}

void TensorAccessAnalysis::innerLoopUpdates(Instruction *T, int TensorId) {
  Value *begin = T->getOperand(0);
  auto *InsertInst = dyn_cast<InsertElementInst>(begin);
  while (InsertInst && InsertInst->getOpcode() == Instruction::InsertElement) {
    int index =
        dyn_cast<llvm::ConstantInt>(InsertInst->getOperand(2))->getZExtValue();
    if (Instruction *Phi = dyn_cast<PHINode>(InsertInst->getOperand(1))) {
      Loop *CurrLoop = LI->getLoopFor(Phi->getParent());
      Optional<Loop::LoopBounds> LB = CurrLoop->getBounds(*SE);
      PHINode *LoopPHI = CurrLoop->getInductionVariable(*SE);
      if (LB && (LoopPHI != nullptr)) {
        Value &Initial = LB->getInitialIVValue();
        Value &Final = LB->getFinalIVValue();
        if (dyn_cast<llvm::ExtractElementInst>(&Initial)) {
          updateCoordFactor(TensorId, index, INVALID_SCEV, "", nullptr);
        } else
          prepareLoopIvCoordMap(TensorId, CurrLoop, index);
      } else
        prepareLoopIvCoordMap(TensorId, CurrLoop, index);
    } else
      updateCoordFactor(TensorId, index, INVALID_SCEV, "", nullptr);
    begin = getNextInstruction(InsertInst);
    InsertInst = dyn_cast<InsertElementInst>(begin);
  }
}

bool TensorAccessAnalysis::compDepthCheckforNextBlock(int Currdepth,
                                                      Value *InVal,
                                                      BasicBlock *RefBB) {
  bool done = true; // TODO: Cleanup this
  // We need to check the Phi and if that instrcution is pointing to
  // deeper loop ignore that
  if (auto *Phi = dyn_cast<PHINode>(InVal)) {
    auto InBB = Phi->getParent();
    if (!LI->getLoopFor(InBB)) {
      return true;
    } else {
      Loop *InLoop = LI->getLoopFor(InBB);
      int InDepth = InLoop->getLoopDepth();
      if (InDepth <= Currdepth) {
        return false;
      } else {
        return true;
      }
    }
  } else {
    // Not a Phi Node just get the Parent of this instruction
    if (auto *inst = dyn_cast<Instruction>(InVal)) {
      auto InBB = inst->getParent();
      if (!LI->getLoopFor(InBB)) {
        done = true;
      } else {
        Loop *InLoop = LI->getLoopFor(InBB);
        int InDepth = InLoop->getLoopDepth();
        if (InDepth > Currdepth) {
        } else {
          done = false;
        }
      }
    }
  }
  return done;
}

int64_t TensorAccessAnalysis::setIndexStepVar(Instruction *StepVar,
                                              Loop *CurrLoop) {
  if (auto BO = dyn_cast<BinaryOperator>(StepVar)) {
    const auto LoopVar = CurrLoop->getInductionVariable(*SE);
    if (LoopVar == StepVar->getOperand(0)) {
      if (BO->getOpcode() == Instruction::SDiv) {
        auto DivDenominator =
            dyn_cast<ConstantInt>(BO->getOperand(1))->getZExtValue();
        return (getScevInfo(CurrLoop)->getSCEVStep() / DivDenominator);
      } else if (BO->getOpcode() == Instruction::Shl) {
        auto ShiftVal =
            dyn_cast<ConstantInt>(BO->getOperand(1))->getZExtValue();
        return (getScevInfo(CurrLoop)->getSCEVStep() << ShiftVal);
      }
    }
    if (BO->getOpcode() == Instruction::Add) {
      if (dyn_cast<ConstantInt>(BO->getOperand(1)))
        return dyn_cast<ConstantInt>(BO->getOperand(1))->getZExtValue();
    }
  }
  return INVALID_SCEV; // Bail out and continue with loop pointer
}

Loop *TensorAccessAnalysis::compareStepValuesForAddMask(PHINode *Phi,
                                                        Loop *CurrLoop) {
  int64_t BBLoopStep;
  if (!LI->getLoopFor(Phi->getParent()) ||
      !getScevInfo(LI->getLoopFor(Phi->getParent())))
    return CurrLoop;
  if (!getScevInfo(CurrLoop))
    return LI->getLoopFor(Phi->getParent());
  auto CurrLoopStep = getScevInfo(CurrLoop)->getSCEVStep();
  BBLoopStep = getScevInfo(LI->getLoopFor(Phi->getParent()))->getSCEVStep();
  if (CurrLoopStep != BBLoopStep)
    return LI->getLoopFor(Phi->getParent());
  else
    return CurrLoop;
}

std::string getPredType(ICmpInst::Predicate Pred) {
  std::string type = "";
  if (Pred == CmpInst::ICMP_SLT) {
    type = "_slt";
  }
  return type;
}

std::string getLDLOperand(Intrinsic::ID inid) {
  std::string type = "";
  if (inid == llvm::Intrinsic::tpc_ld_l) {
    type = "(ld_l(";
  }
  return type;
}

std::string TensorAccessAnalysis::getFormula(IntrinsicInst *Intrins,
                                             Function &F) {
  std::string formula = "";
  if (Instruction *form_inst =
          (dyn_cast<Instruction>(Intrins->getOperand(4)))) {
    // Get polarity here
    int i = 0;
    if (auto *polarity = dyn_cast<llvm::ConstantInt>(Intrins->getOperand(5))) {
      int polarity_val = polarity->getZExtValue();
      for (auto arg = F.arg_begin(); arg != F.arg_end(); ++arg) {
        if (auto *val = (dyn_cast<Value>(arg))) {
          for (unsigned j = 0; j < form_inst->getNumOperands(); j++) {
            Value *runtime = form_inst->getOperand(j);
            if (runtime == val && polarity_val == 1) {
              formula = "not(%" + std::to_string(i) + ")";
              return formula;
            } else if (runtime == val) {
              // No need of formula if polarity is zero
              return formula;
            }
          }
        }
        i++;
      }
    }
    // We are here means runtime arg not from function
    if ((formula.length() == 0) &&
        (form_inst->getOpcode() == Instruction::ICmp)) {
      int cmp_index = -1;
      int intrins_index = -1;
      formula = "cmp";
      ICmpInst *CI = dyn_cast<ICmpInst>(form_inst);
      ICmpInst::Predicate Pred = CI->getPredicate();
      std::string type = getPredType(Pred);
      if (type.length() > 0) {
        formula += type;
      }
      for (unsigned j = 0; j < CI->getNumOperands(); j++) {
        if (auto *intrins = dyn_cast<IntrinsicInst>(CI->getOperand(j))) {
          Intrinsic::ID inid = intrins->getIntrinsicID();
          std::string type = getLDLOperand(inid);
          if (type.length() > 0) {
            formula += type;
            if (auto *intrins_int =
                    dyn_cast<llvm::ConstantInt>(intrins->getOperand(0))) {
              intrins_index = intrins_int->getZExtValue();
              formula += std::to_string(intrins_index) + "),";
            }
          }
        } else if (auto *cmp_int =
                       dyn_cast<llvm::ConstantInt>(CI->getOperand(j))) {
          cmp_index = cmp_int->getZExtValue();
          formula += std::to_string(cmp_index) + ")";
        }
      }
    }
  }
  return formula;
}

void TensorAccessAnalysis::dumpIntoASM_MLIR() {
  // Prepare TensorInfo
  for (auto &TensorId : TensorTypeMap) {
    if (TensorCordInfoMap.find(TensorId.first) == TensorCordInfoMap.end()) {
      TensorInfo TnsrInfo;
      TnsrInfo.Type = TensorTypeMap.at(TensorId.first);
      TnsrInfo.Order = TensorId.first;
      std::string IndexStr = "{ 0";
      for (unsigned i = 1; i < TnsrInfo.IndexFactor.capacity(); i++)
        IndexStr += ", " + std::to_string(0);

      IndexStr += " }";
      // prepare TensorName
      if (TensorId.first >= FakeTensorIdPad) { // FakeTensorId
        std::string Temp =
            "x" + std::to_string(TensorId.first - FakeTensorIdPad);
        TnsrInfo.Name = "[" + Temp + "].";
      } else
        TnsrInfo.Name = "[" + std::to_string(TensorId.first) + "].";
      if (TnsrInfo.Type == TensorType::Input) {
        TnsrInfo.Name += "[Input].";
        TnsrInfo.Name += IndexStr;
        Input.push_back(TnsrInfo);
      }
      if (TnsrInfo.Type == TensorType::Output) {
        TnsrInfo.Name += "[Output].";
        TnsrInfo.Name += IndexStr;
        Output.push_back(TnsrInfo);
      }
    }
  }

  for (auto Tlinfo : TensorCordInfoMap) {
    TensorInfo TnsrInfo;
    if (TensorTypeMap.find(Tlinfo.first) == TensorTypeMap.end() &&
        (std::find(PragmaTensors.begin(), PragmaTensors.end(), Tlinfo.first) !=
         PragmaTensors.end())) {
      auto WarningStr = "Warning : Pragma defined for an unused Tensor. "
                        "Skipping pragma notation for ID " +
                        std::to_string(Tlinfo.first) + "\n";
      errs() << WarningStr;
      continue;
    }
    TnsrInfo.Type = TensorTypeMap.at(Tlinfo.first);
    TnsrInfo.Order = Tlinfo.first;
    for (unsigned i = 0; i < TnsrInfo.IndexFactor.capacity(); i++)
      TnsrInfo.IndexFactor.emplace_back(0);
    for (auto Linfo : Tlinfo.second) { // Loop over vector per TensorId
      Loop *Loopit = std::get<0>(Linfo);
      int indexit = std::get<1>(Linfo);
      auto SCEVInfo = getScevInfo(Loopit); // Fetch the required loop pointer
      // Populate stride
      if (SCEVInfo) {
        TnsrInfo.IndexFactor[indexit] = SCEVInfo->getSCEVStep();
        std::string formula = std::get<2>(Linfo);
        if (formula.length() > 0) {
          // push the formula
          TnsrInfo.umap.insert({indexit, formula});
        }
      } else {
        auto DiffCBegin = DiffCoords.lower_bound(Tlinfo.first);
        auto DiffCEnd = DiffCoords.upper_bound(Tlinfo.first);
        int MaxVal = -1;
        std::string Formula = "";
        while (DiffCBegin != DiffCEnd) {
          int Diffindex = std::get<0>(DiffCBegin->second);
          int DiffVal = std::get<1>(DiffCBegin->second);
          if (Diffindex == indexit) {
            if (DiffVal > MaxVal) {
              MaxVal = DiffVal;
              Formula = std::get<2>(DiffCBegin->second);
            }
          }
          DiffCBegin++;
        }
        if (MaxVal >= 0) { // To bar any invalid access to TnsrInfo
          TnsrInfo.IndexFactor[indexit] = MaxVal;
          if (Formula.length() > 0)
            TnsrInfo.umap.insert({indexit, Formula});
        }
      }
      // Check if the Tensor ID is manipulated zero it out
      for (auto i = Tensor_ids.begin(); i != Tensor_ids.end(); ++i) {
        if (*i == TnsrInfo.Order) {
          for (unsigned i = 0; i < TnsrInfo.IndexFactor.size(); i++) {
            TnsrInfo.IndexFactor[i] = 0;
          }
        }
      }
    }

    // prepare IndexString
    std::string IndexStr = "";
    std::string formula = "";
    std::string warn = "";
    bool InvalidScev = false;
    for (unsigned i = 0; i < TnsrInfo.IndexFactor.size(); i++) {
      if (TnsrInfo.IndexFactor[i] == INVALID_SCEV) {
        InvalidScev = true;
        break;
      }
    }
    // Making the Whole tensor zero if Invalid scev found, this
    // is temp work around and will be removed later
    if (InvalidScev) {
      for (unsigned i = 0; i < TnsrInfo.IndexFactor.size(); i++) {
        TnsrInfo.IndexFactor[i] = 0;
      }
    }
    warn = "Warning: Auto Index Space could not be generated"
           " for Tensor ID: " +
           std::to_string(Tlinfo.first);
    std::unordered_map<int, std::string>::const_iterator got =
        TnsrInfo.umap.find(0);
    if (TnsrInfo.IndexFactor[0] == INVALID_SCEV) {
      InvalidScev = true;
      warn += " Coord: " + std::to_string(0);
      TnsrInfo.IndexFactor[0] = 0;
    }
    if (got != TnsrInfo.umap.end()) {
      formula = got->second;
      if (formula.length() > 0) {
        IndexStr = "{ " + formula + "*";
        IndexStr += std::to_string(TnsrInfo.IndexFactor[0]);
      } else {
        IndexStr = "{ " + std::to_string(TnsrInfo.IndexFactor[0]);
      }
    } else {
      IndexStr = "{ " + std::to_string(TnsrInfo.IndexFactor[0]);
    }
    for (unsigned i = 1; i < TnsrInfo.IndexFactor.size(); i++) {
      got = TnsrInfo.umap.find(i);
      if (TnsrInfo.IndexFactor[i] == INVALID_SCEV) {
        InvalidScev = true;
        warn += " Coord: " + std::to_string(i);
        TnsrInfo.IndexFactor[i] = 0;
      }
      if (got != TnsrInfo.umap.end()) {
        formula = got->second;
        if (formula.length() > 0) {
          IndexStr += ", " + formula + "*";
          IndexStr += std::to_string(TnsrInfo.IndexFactor[i]);
        } else {
          IndexStr += ", " + std::to_string(TnsrInfo.IndexFactor[i]);
        }
      } else {
        IndexStr += ", " + std::to_string(TnsrInfo.IndexFactor[i]);
      }
    }

    warn += " Setting it zero, but Please use Pragma"
            " for better performance\n";
    if (InvalidScev && IndexSpaceWarning)
      errs() << warn;

    IndexStr += " }";
    // prepare TensorName
    if (TnsrInfo.Order >= FakeTensorIdPad) { // FakeTensorId
      std::string Temp = "x" + std::to_string(TnsrInfo.Order - FakeTensorIdPad);
      TnsrInfo.Name = "[" + Temp + "].";
    } else
      TnsrInfo.Name = "[" + std::to_string(TnsrInfo.Order) + "].";
    if (TnsrInfo.Type == TensorType::Input) {
      TnsrInfo.Name += "[Input].";
      TnsrInfo.Name += IndexStr;
      Input.push_back(TnsrInfo);
    }
    if (TnsrInfo.Type == TensorType::Output) {
      TnsrInfo.Name += "[Output].";
      TnsrInfo.Name += IndexStr;
      Output.push_back(TnsrInfo);
    }
    if (TnsrInfo.Type == TensorType::Aux) {
      TnsrInfo.Name += "[Aux].";
      TnsrInfo.Name += IndexStr;
      Aux.push_back(TnsrInfo);
    }
    LLVM_DEBUG(dbgs() << "\n" << TnsrInfo.Name);
  }
}


void TensorAccessAnalysis::dumpIntoASM() {
  // Prepare TensorInfo
  if (StLTensors.size()) {
    for (auto &TensorId : TensorTypeMap) {
      TensorInfo TnsrInfo;
      TnsrInfo.Type = TensorTypeMap.at(TensorId.first);
      TnsrInfo.Order = TensorId.first;
      std::string IndexStr = "{ GC_CUSTOM";
      // prepare TensorName
      for (unsigned i = 1; i < TnsrInfo.IndexFactor.capacity(); i++)
        IndexStr += ", GC_CUSTOM";
      IndexStr += " }";
      if (TensorId.first >= FakeTensorIdPad) { // FakeTensorId
        std::string Temp =
            "x" + std::to_string(TensorId.first - FakeTensorIdPad);
        TnsrInfo.Name = "[" + Temp + "].";
      } else
        TnsrInfo.Name = "[" + std::to_string(TensorId.first) + "].";
      if (TnsrInfo.Type == TensorType::Input) {
        TnsrInfo.Name += "[Input].";
        TnsrInfo.Name += IndexStr;
        Input.push_back(TnsrInfo);
      }
      if (TnsrInfo.Type == TensorType::Output) {
        TnsrInfo.Name += "[Output].";
        TnsrInfo.Name += IndexStr;
        Output.push_back(TnsrInfo);
      }
    }
  } else {
    for (auto &TensorId : TensorTypeMap) {
      if (TensorCordInfoMap.find(TensorId.first) == TensorCordInfoMap.end()) {
        TensorInfo TnsrInfo;
        TnsrInfo.Type = TensorTypeMap.at(TensorId.first);
        TnsrInfo.Order = TensorId.first;
        std::string IndexStr;
        if (LdStLTensors.find(TensorId.first) != LdStLTensors.end()) {
          if (BailoutReason.empty())
            BailoutReason = "GetDimSize followed by SetDimSize present";
          IndexStr = "{ GC_CUSTOM";
          // prepare TensorName
          for (unsigned i = 1; i < TnsrInfo.IndexFactor.capacity(); i++)
            IndexStr += ", GC_CUSTOM";
          IndexStr += " }";
        } else {
          IndexStr = "{ 0";
          // prepare TensorName
          for (unsigned i = 1; i < TnsrInfo.IndexFactor.capacity(); i++)
            IndexStr += ", 0";
          IndexStr += " }";
        }
        // prepare TensorName
        if (TensorId.first >= FakeTensorIdPad) { // FakeTensorId
          std::string Temp =
              "x" + std::to_string(TensorId.first - FakeTensorIdPad);
          TnsrInfo.Name = "[" + Temp + "].";
        } else
          TnsrInfo.Name = "[" + std::to_string(TensorId.first) + "].";
        if (TnsrInfo.Type == TensorType::Input) {
          TnsrInfo.Name += "[Input].";
          TnsrInfo.Name += IndexStr;
          Input.push_back(TnsrInfo);
        }
        if (TnsrInfo.Type == TensorType::Output) {
          TnsrInfo.Name += "[Output].";
          TnsrInfo.Name += IndexStr;
          Output.push_back(TnsrInfo);
        }
      }
    }
    // continue normal processing of asm
    for (auto Tlinfo : TensorCordInfoMap) {
      TensorInfo TnsrInfo;
      if (TensorTypeMap.find(Tlinfo.first) == TensorTypeMap.end() &&
          (std::find(PragmaTensors.begin(), PragmaTensors.end(),
                     Tlinfo.first) != PragmaTensors.end())) {
        auto WarningStr = "Warning : Pragma defined for an unused Tensor. "
                          "Skipping pragma notation for ID " +
                          std::to_string(Tlinfo.first) + "\n";
        errs() << WarningStr;
        continue;
      }
      TnsrInfo.Type = TensorTypeMap.at(Tlinfo.first);
      TnsrInfo.Order = Tlinfo.first;
      for (unsigned i = 0; i < TnsrInfo.IndexFactor.capacity(); i++)
        TnsrInfo.IndexFactor.emplace_back(0);
      for (auto Linfo : Tlinfo.second) { // Loop over vector per TensorId
        Loop *Loopit = std::get<0>(Linfo);
        int indexit = std::get<1>(Linfo);
        auto SCEVInfo = getScevInfo(Loopit); // Fetch the required loop pointer
        // Populate stride
        if (SCEVInfo) {
          TnsrInfo.IndexFactor[indexit] = SCEVInfo->getSCEVStep();
          std::string formula = std::get<2>(Linfo);
          if (formula.length() > 0) {
            // push the formula
            TnsrInfo.umap.insert({indexit, formula});
          }
        } else {
          auto DiffCBegin = DiffCoords.lower_bound(Tlinfo.first);
          auto DiffCEnd = DiffCoords.upper_bound(Tlinfo.first);
          int MaxVal = -1;
          std::string Formula = "";
          while (DiffCBegin != DiffCEnd) {
            int Diffindex = std::get<0>(DiffCBegin->second);
            int DiffVal = std::get<1>(DiffCBegin->second);
            if (Diffindex == indexit) {
              if (DiffVal > MaxVal) {
                MaxVal = DiffVal;
                Formula = std::get<2>(DiffCBegin->second);
              }
            }
            DiffCBegin++;
          }
          if (MaxVal >= 0) { // To bar any invalid access to TnsrInfo
            TnsrInfo.IndexFactor[indexit] = MaxVal;
            if (Formula.length() > 0)
              TnsrInfo.umap.insert({indexit, Formula});
          }
        }
        // Check if the Tensor ID is manipulated zero it out
        for (auto i = Tensor_ids.begin(); i != Tensor_ids.end(); ++i) {
          if (*i == TnsrInfo.Order) {
            for (unsigned i = 0; i < TnsrInfo.IndexFactor.size(); i++) {
              TnsrInfo.IndexFactor[i] = 0;
            }
          }
        }
      }

      // prepare IndexString
      std::string IndexStr = "";
      std::string formula = "";
      std::string warn = "";
      bool InvalidScev = false;
      for (unsigned i = 0; i < TnsrInfo.IndexFactor.size(); i++) {
        if (TnsrInfo.IndexFactor[i] == INVALID_SCEV) {
          InvalidScev = true;
          break;
        }
      }
      // Making the Whole tensor zero if Invalid scev found, this
      // is temp work around and will be removed later
      if (InvalidScev) {
        for (unsigned i = 0; i < TnsrInfo.IndexFactor.size(); i++) {
          TnsrInfo.IndexFactor[i] = 0;
        }
      }
      warn = "Warning: Auto Index Space could not be generated"
             " for Tensor ID: " +
             std::to_string(Tlinfo.first);
      std::unordered_map<int, std::string>::const_iterator got =
          TnsrInfo.umap.find(0);
      if (TnsrInfo.IndexFactor[0] == INVALID_SCEV) {
        InvalidScev = true;
        warn += " Coord: " + std::to_string(0);
        TnsrInfo.IndexFactor[0] = 0;
      }
      if (got != TnsrInfo.umap.end()) {
        formula = got->second;
        if (formula.length() > 0) {
          IndexStr = "{ " + formula + "*";
          IndexStr += std::to_string(TnsrInfo.IndexFactor[0]);
        } else {
          IndexStr = "{ " + std::to_string(TnsrInfo.IndexFactor[0]);
        }
      } else {
        IndexStr = "{ " + std::to_string(TnsrInfo.IndexFactor[0]);
      }
      for (unsigned i = 1; i < TnsrInfo.IndexFactor.size(); i++) {
        got = TnsrInfo.umap.find(i);
        if (TnsrInfo.IndexFactor[i] == INVALID_SCEV) {
          InvalidScev = true;
          warn += " Coord: " + std::to_string(i);
          TnsrInfo.IndexFactor[i] = 0;
        }
        if (got != TnsrInfo.umap.end()) {
          formula = got->second;
          if (formula.length() > 0) {
            IndexStr += ", " + formula + "*";
            IndexStr += std::to_string(TnsrInfo.IndexFactor[i]);
          } else {
            IndexStr += ", " + std::to_string(TnsrInfo.IndexFactor[i]);
          }
        } else {
          IndexStr += ", " + std::to_string(TnsrInfo.IndexFactor[i]);
        }
      }

      warn += " Setting it zero, but Please use Pragma"
              " for better performance\n";

      if (InvalidScev && IndexSpaceWarning)
        errs() << warn;

      if (InvalidScev) {
        if (BailoutReason.empty())
          BailoutReason =
              "Invalid Scev present (Runtime args in coord manipulation)";
        IndexStr = "{ GC_CUSTOM";
        // prepare TensorName
        for (unsigned i = 1; i < TnsrInfo.IndexFactor.capacity(); i++)
          IndexStr += ", GC_CUSTOM";
      }

      IndexStr += " }";
      // prepare TensorName
      if (TnsrInfo.Order >= FakeTensorIdPad) { // FakeTensorId
        std::string Temp =
            "x" + std::to_string(TnsrInfo.Order - FakeTensorIdPad);
        TnsrInfo.Name = "[" + Temp + "].";
      } else
        TnsrInfo.Name = "[" + std::to_string(TnsrInfo.Order) + "].";
      if (TnsrInfo.Type == TensorType::Input) {
        TnsrInfo.Name += "[Input].";
        TnsrInfo.Name += IndexStr;
        Input.push_back(TnsrInfo);
      }
      if (TnsrInfo.Type == TensorType::Output) {
        TnsrInfo.Name += "[Output].";
        TnsrInfo.Name += IndexStr;
        Output.push_back(TnsrInfo);
      }
      if (TnsrInfo.Type == TensorType::Aux) {
        TnsrInfo.Name += "[Aux].";
        TnsrInfo.Name += IndexStr;
        Aux.push_back(TnsrInfo);
      }
      LLVM_DEBUG(dbgs() << "\n" << TnsrInfo.Name);
    }
  }
}

void TensorAccessAnalysis::processPHIUses(int TensorId, PHINode *PHI,
                                          Loop *CurrLoop, Function &F) {
  User *PrevInstr = nullptr;
  for (const Use &PHIUse : PHI->uses()) {
    User *PHIUser = PHIUse.getUser();
    if (PrevInstr && PHIUser == PrevInstr)
      continue;
    PrevInstr = PHIUser;
    if (auto *InsetInst = dyn_cast<InsertElementInst>(PHIUser)) {
      bool CheckIfNodeProcessed = iterateProcessedNodes(TensorId, CurrLoop);
      if (iterateProcessedPHI(PHI) && CheckIfNodeProcessed)
        continue;
      if (!dyn_cast<ConstantInt>(InsetInst->getOperand(1))) {
        processInsertElementNode(InsetInst, CurrLoop, PHI, TensorId);
      }
      processNestedInstructions(InsetInst, CurrLoop, PHI, TensorId, false, F);
    } else if (auto *ShuffVecInst = dyn_cast<ShuffleVectorInst>(PHIUser)) {
      processNestedInstructions(ShuffVecInst, CurrLoop, PHI, TensorId, false, F);
      continue;
    } else if (auto *Intrins = dyn_cast<IntrinsicInst>(PHIUser)) {
      Intrinsic::ID Inid = Intrins->getIntrinsicID();
      if (Inid == Intrinsic::tpc_ld_tnsr) {
        // copy detected, need to copy index space
        int srcTensorId = GetTensorId(Intrins->getOperand(1));
        if (TensorId != srcTensorId)
          CopyIndSpace.insert(std::make_pair(TensorId, srcTensorId));
      }
      if (Inid != Intrinsic::tpc_set_indx && Inid != Intrinsic::tpc_add_mask &&
          Inid != Intrinsic::tpc_add)
        continue;
      int IndexShl = -1;
      std::string Formula = "";
      if (Inid == Intrinsic::tpc_add) {
        continue;
      } else if (Inid == Intrinsic::tpc_add_mask) {
        if (dyn_cast<llvm::ConstantInt>(Intrins->getOperand(2))) {
          if (dyn_cast<Instruction>(Intrins->getOperand(6))) {
            if (!dyn_cast<ICmpInst>(Intrins->getOperand(6)))
              FallBackVec.insert(TensorId);
            else {
              if (auto *Icmp = dyn_cast<ICmpInst>(Intrins->getOperand(6)))
                if (!dyn_cast<ConstantInt>(Icmp->getOperand(1)))
                  FallBackVec.insert(TensorId);
            }
          }
          IndexShl = dyn_cast<llvm::ConstantInt>(Intrins->getOperand(2))
                         ->getZExtValue();
          CurrLoop = compareStepValuesForAddMask(PHI, CurrLoop);
          int64_t AddMaskStep = getAddMaskStepVar(Intrins);
          bool CheckIfNodeProcessed = iterateProcessedNodes(TensorId, CurrLoop);
          if (AddMaskStep != INVALID_STRIDE) {
            bool CopyPHI = false;
            if (iterateProcessedPHI(PHI)) {
              // Check for tensor ID in diffcoords
              if (CheckIfNodeProcessed)
                continue;
              else
                CopyPHI = true;
            }
            unsigned UnrollCount = 0;
            if (getScevInfo(CurrLoop)) {
              UnrollCount = getScevInfo(CurrLoop)->getLoopUnrollCount(CurrLoop);
            }
            if (UnrollCount)
              AddMaskStep = AddMaskStep * UnrollCount;
            unsigned Index = fetchIndexFromOperand(IndexShl);
            UpdateDiffCoords(TensorId, Index, AddMaskStep, "", CurrLoop, PHI);
            prepareLoopIvCoordMap(TensorId, nullptr, Index);
            processNestedInstructions(Intrins, CurrLoop, PHI, TensorId, CopyPHI,
                                      F);
            continue;
          }
        } else {
          IncorrectIndexSpace = true;
          return;
        }
      } else if (Inid == Intrinsic::tpc_set_indx) {
        if (dyn_cast<llvm::ConstantInt>(Intrins->getOperand(1)))
          processSetIndexNode(TensorId, CurrLoop, PHI, F, Intrins);
        else {
          IncorrectIndexSpace = true;
          return;
        }
      }

      // This is to counter the notation of set_index and
      // add_mask
      unsigned Index = fetchIndexFromOperand(IndexShl);

      if (Inid == Intrinsic::tpc_add_mask &&
          LI->isLoopHeader(PHI->getParent())) {
        prepareLoopIvCoordMap(TensorId, CurrLoop, Index, "");
      }
      if (Inid == Intrinsic::tpc_add_mask || Inid == Intrinsic::tpc_set_indx)
        processNestedInstructions(Intrins, CurrLoop, PHI, TensorId, false, F);
    }
  }
}

void TensorAccessAnalysis::getloadstoreIntrins(Function &F) {
  StringRef SubArchName = "";
  Attribute SubArchAttr = F.getFnAttribute("target-cpu");
  auto SubArchStr = SubArchAttr.getValueAsString();
  for (auto &BB : F)
    for (auto &II : BB)
      if (auto *Intrins = dyn_cast<IntrinsicInst>(&II)) {
        Intrinsic::ID Inid = Intrins->getIntrinsicID();
        if (is_ld_l_tnsr(Inid)) {
          int val_id = GetValId(Intrins->getOperand(0));
          if (val_id < 0)
            continue;
          ld_id.push_back(val_id);
        } else if (is_st_l_tnsr(Inid)) {
          int val_id = GetValId(Intrins->getOperand(0));
          if (val_id < 0)
            continue;
          st_id.push_back(val_id);
        }
      }
  // Store the Valid tensors
  if (SubArchStr.size() > 0) {
    SubArchName = SubArchStr;
    if (ConfigStartOffset.find(SubArchName) == ConfigStartOffset.end()) {
      return;
    }
    for (auto i = ld_id.begin(); i != ld_id.end(); ++i) {
      int ld_id = *i;
      for (auto j = st_id.begin(); j != st_id.end(); ++j) {
        int st_id = *j;
        if (ld_id == st_id) {
          // for the Match once we need to have exact division
          int id_offset = ld_id - ConfigStartOffset[SubArchName];
          if (id_offset % ConfigOffset[SubArchName] == 0) {
            int TensorID = id_offset / ConfigOffset[SubArchName];
            Tensor_ids.push_back(TensorID);
          }
        }
      }
    }
  }
}

bool checkIndexSpaceOffset(Instruction *InstrPtr) {
  if (InstrPtr->getOpcode() == Instruction::ExtractElement) {
    auto Arg0 = InstrPtr->getOperand(0);
    if (auto *Intrinsic = dyn_cast<IntrinsicInst>(Arg0)) {
      auto ID = Intrinsic->getIntrinsicID();
      if (ID == Intrinsic::tpc_get_index_space_offset) {
        return true;
      }
    }
  }
  return false;
}

void TensorAccessAnalysis::checkRuntimeInfo(int TensorId,
                                            std::vector<Value *> TraceToPHI) {
  for (auto &Iter : TraceToPHI) {
    if (auto *Instr = dyn_cast<InsertElementInst>(Iter)) {
      auto StrideArg = Instr->getOperand(1);
      if (auto BO = dyn_cast<BinaryOperator>(StrideArg)) {
        if (!dyn_cast<llvm::ConstantInt>(BO->getOperand(1))) {
          auto TempPtr = dyn_cast<Instruction>(BO->getOperand(1));
          if (TempPtr && checkIndexSpaceOffset(TempPtr))
            continue;
          StLTensors.insert(TensorId);
          if (BailoutReason.empty())
            BailoutReason = "Loop bounds have runtime dependency";
        }
      }
    }
  }
}

void TensorAccessAnalysis::computeTensorInfo(Function &F, bool Update) {
  // prepare ld/st tensor list
  resetFakeTensorId();
  getloadstoreIntrins(F);
  for (auto &BB : F)
    for (auto &II : BB)
      if (auto *Intrins = dyn_cast<IntrinsicInst>(&II)) {
        Intrinsic::ID Inid = Intrins->getIntrinsicID();
        if (is_ld_st_tnsr(Inid)) {
          int TensorId = GetTensorId(Intrins->getOperand(1));
          assert(TensorId >= 0 && "Tensor ID must be positive.");
          classifyTensorType(TensorId, Intrins);
          if (UniqueTensors.find(Intrins) == UniqueTensors.end()) {
            Tensors.push_back(Intrins);
            UniqueTensors.insert(Intrins);
          }
        }
      }
  // Prepare TensorID Coordnate Map
  resetFakeTensorId();
  for (auto T : Tensors) {
    LLVM_DEBUG(dbgs() << "IndexAnalysis for Tensor "; T->dump());
    int TensorId = GetTensorId(T->getOperand(1));
    if (std::find(PragmaTensors.begin(), PragmaTensors.end(), TensorId) !=
        PragmaTensors.end())
      continue;
    if (std::find(LdStLTensors.begin(), LdStLTensors.end(), TensorId) !=
        LdStLTensors.end())
      continue;
    if (std::find(StLTensors.begin(), StLTensors.end(), TensorId) !=
        StLTensors.end())
      continue;

    assert(TensorId >= 0 && "Tensor ID must be positive.");
    if (Update == 1) {
      innerLoopUpdates(T, TensorId);
      continue;
    }

    Value *InitTensor = T->getOperand(0);
    Value *InitTensorBackup = T;

    std::vector<Value *> TraceToPHI;
    // collect all Invariant Loads/Stores
    if (!(dyn_cast<Instruction>(InitTensor))) {
      continue;
    }

    // Use-Def Ld/St Tensor
    while (InitTensor) {
      LLVM_DEBUG(dbgs() << "IndexAnalysis Use-Def Inst "; InitTensor->dump());
      Loop *CurrLoop = nullptr;
      if (Instruction *InstructionPtr = dyn_cast<Instruction>(InitTensor)) {
        // Backtrack for loop induction variable
        if (InstructionPtr->getOpcode() != Instruction::PHI) {
          TraceToPHI.push_back(InstructionPtr);
          InitTensor = getNextInstruction(InstructionPtr);
          continue;
        }
        checkRuntimeInfo(TensorId, TraceToPHI);
        TraceToPHI = {};
        // Captured the loop variable phi node
        auto *PHI = dyn_cast<PHINode>(InstructionPtr);
        CurrLoop = LI->getLoopFor(PHI->getParent());
        // Loop through the Uses and prepare Tensor Info
        processPHIUses(TensorId, PHI, CurrLoop, F);
        // move to next BB if any
        bool PHIUseRemaining = false;
        for (unsigned i = 0; i < PHI->getNumIncomingValues(); ++i) {
          Loop *IncomingLoop = nullptr;
          auto BB = PHI->getIncomingBlock(i);
          IncomingLoop = LI->getLoopFor(BB);
          if (!CurrLoop)
            continue;
          if (IncomingLoop == CurrLoop->getParentLoop()) {
            InitTensorBackup = InitTensor = PHI->getIncomingValue(i);
            PHIUseRemaining = true;
            break;
          } else {
            // We are either same of less depth
            int Currdepth = CurrLoop->getLoopDepth();
            int Incomingdepth = IncomingLoop->getLoopDepth();
            if (Currdepth == Incomingdepth) {
              Value *InVal = PHI->getIncomingValue(i);
              if (!compDepthCheckforNextBlock(Currdepth, InVal, BB)) {
                if (InitTensor != nullptr && PHIUseRemaining != true) {
                  InitTensorBackup = InitTensor = PHI->getIncomingValue(i);
                  PHIUseRemaining = true;
                }
              } else if (auto *LCSSAPhi = dyn_cast<PHINode>(InVal)) {
                if (LCSSAPhi->getNumIncomingValues() == 1) {
                  InitTensorBackup = InitTensor = PHI->getIncomingValue(0);
                  PHIUseRemaining = true;
                }
              }
            }
            if (BB == PHI->getParent())
              continue;
            // We only take the incoming node and traverse till we find the
            // phi
            // TraverseTillPhiNode begin
            Value *IncomingEdge = PHI->getIncomingValue(i);
            while (IncomingEdge) {
              LLVM_DEBUG(dbgs() << "IndexAnalysis Use-Def Inst ";
                         IncomingEdge->dump());
              Loop *BaseLoop = nullptr;
              if (Instruction *InstructionPtr2 =
                      dyn_cast<Instruction>(IncomingEdge)) {
                // Backtrack for loop induction variable
                if (InstructionPtr2->getOpcode() != Instruction::PHI) {
                  IncomingEdge = getNextInstruction(InstructionPtr2);
                  continue;
                }
                auto *PHIPtr = dyn_cast<PHINode>(InstructionPtr2);
                // Trace back to first basic block
                BaseLoop = LI->getLoopFor(PHIPtr->getParent());
                processPHIUses(TensorId, PHIPtr, BaseLoop, F);
                break;
              } else // If not an instruction, no need of processing
                break;
            }
            // TraverseTillPhiNode end
          }
        }
        if (!PHIUseRemaining)
          break;
      } else {
        break;
      }
    }
  }
}

void SCEVInfo::processSCEVInfo() {
  if (!L->getLoopLatch() || !L->getLoopPredecessor()) {
    return;
  }
  InductionDescriptor ID;
  for (PHINode &PHI : L->getHeader()->phis()) {
    PHINode *PhiPtr = &(PHI);
    if (!InductionDescriptor::isInductionPHI(PhiPtr, L, SE, ID))
      continue;
    const SCEVAddRecExpr *AddRec =
        dyn_cast<SCEVAddRecExpr>(SE->getSCEV(PhiPtr));
    if (!AddRec || !AddRec->isAffine())
      continue;
    const SCEV *Step = AddRec->getStepRecurrence(*SE);
    if (!isa<SCEVConstant>(Step))
      continue;
    LoopIndVar = PhiPtr;
    break;
  }
  if (LoopIndVar)
    SCEVPtr = SE->getSCEV(LoopIndVar);
}

// Return the loop step if a valid SCEV Ptr is encountered
// Return INT_MAX otherwise

int64_t SCEVInfo::getSCEVStep() const {
  if (!SCEVPtr) {
    LLVM_DEBUG(dbgs() << "Not a valid SCEV Node"
                      << "\n");
    return INVALID_SCEV;
  }
  const SCEVAddRecExpr *AddRec = dyn_cast<SCEVAddRecExpr>(SCEVPtr);
  if (!AddRec || !AddRec->isAffine())
      return INT_MAX;
  const SCEV *Step = AddRec->getStepRecurrence(*SE);
  unsigned UnrollCount = getLoopUnrollCount(L);
  if (UnrollCount)
    return (UnrollCount *
            std::abs(cast<SCEVConstant>(Step)->getValue()->getSExtValue()));
  else
    return (std::abs(cast<SCEVConstant>(Step)->getValue()->getSExtValue()));
}

PHINode *SCEVInfo::getLoopInductionVar() const { return LoopIndVar; }

Loop *SCEVInfo::getLoopPtr() const { return L; }

std::string SCEVInfo::getSCEVLoopName() const {
  if (L)
    return L->getName().str();
  else
    return "";
}

unsigned SCEVInfo::getLoopUnrollCount(Loop *L) const {
  MDNode *LoopMD = L->getLoopID();
  if (!LoopMD)
    return 0;
  assert(LoopMD->getNumOperands() > 0 && "requires at least one operand");
  MDNode *MDPtr = nullptr;

  for (unsigned i = 1; i < LoopMD->getNumOperands(); ++i) {
    MDPtr = dyn_cast<MDNode>(LoopMD->getOperand(i));
    if (!MDPtr)
      continue;
    MDString *S = dyn_cast<MDString>(MDPtr->getOperand(0));
    if (!S)
      continue;
    if (S->getString().equals("llvm.loop.machine.unroll.count")) {
      assert(MDPtr->getNumOperands() == 2 &&
             "Unroll hint metadata should have two operands");
      unsigned Count =
          mdconst::extract<ConstantInt>(MDPtr->getOperand(1))->getZExtValue();
      assert(Count >= 1 && "Unroll count must be positive.");
      return Count;
    }
  }
  return 0;
}

void TensorAccessAnalysis::processNestedInstructions(Instruction *RootInst,
                                                     Loop *CurrLoop,
                                                     PHINode *PHI, int TensorId,
                                                     bool CopyPHI,
                                                     Function &F) {
  bool Nested_Ins = true;
  bool CheckIfNodeProcessed = iterateProcessedNodes(TensorId, CurrLoop);
  if (iterateProcessedPHI(PHI)) {
    // Check for tensor ID in diffcoords
    if (CheckIfNodeProcessed && !CopyPHI)
      return;
  } else
    addToProcessedPHI(PHI);
  while (Nested_Ins) {
    Nested_Ins = false;
    for (const Use &RootInstUse : RootInst->uses()) {
      User *RootInstUser = RootInstUse.getUser();
      if (auto *InsertUserCast = dyn_cast<InsertElementInst>(RootInstUser)) {
        processInsertElementNode(InsertUserCast, CurrLoop, PHI, TensorId);
        RootInst = InsertUserCast;
        Nested_Ins = true;
        break;
      } else if (auto *Intrins = dyn_cast<IntrinsicInst>(RootInstUser)) {
        Intrinsic::ID Inid = Intrins->getIntrinsicID();
        int index = -1;
        if (Inid == Intrinsic::tpc_add_mask ||
            Inid == Intrinsic::tpc_set_indx) {
          if (Inid == Intrinsic::tpc_add_mask) {

            int OperandIndex = 2;
            if (dyn_cast<llvm::ConstantInt>(
                    Intrins->getOperand(OperandIndex))) {
              if (dyn_cast<Instruction>(Intrins->getOperand(6))) {
                if (!dyn_cast<ICmpInst>(Intrins->getOperand(6)))
                  FallBackVec.insert(TensorId);
                else {
                  if (auto *Icmp = dyn_cast<ICmpInst>(Intrins->getOperand(6)))
                    if (!dyn_cast<ConstantInt>(Icmp->getOperand(1)))
                      FallBackVec.insert(TensorId);
                }
              }
              index =
                  dyn_cast<llvm::ConstantInt>(Intrins->getOperand(OperandIndex))
                      ->getZExtValue();
              CurrLoop = compareStepValuesForAddMask(PHI, CurrLoop);
              int64_t AddMaskStep = getAddMaskStepVar(Intrins);
              if (AddMaskStep != INVALID_STRIDE) {
                unsigned Count = fetchIndexFromOperand(index);
                unsigned UnrollCount = 0;
                if (getScevInfo(CurrLoop)) {
                  UnrollCount =
                      getScevInfo(CurrLoop)->getLoopUnrollCount(CurrLoop);
                }
                if (UnrollCount)
                  AddMaskStep = AddMaskStep * UnrollCount;
                UpdateDiffCoords(TensorId, Count, AddMaskStep, "", CurrLoop,
                                 PHI);
                if ((1 << Count) < index) {
                  auto Remaining = fetchIndexFromOperand(index - (1 << Count));
                  UpdateDiffCoords(TensorId, Remaining, AddMaskStep, "",
                                   CurrLoop, PHI);
                }
                prepareLoopIvCoordMap(TensorId, nullptr, Count);
                RootInst = Intrins;
                Nested_Ins = true;
                break;
              }
            } else {
              IncorrectIndexSpace = true;
              return;
            }

            if (dyn_cast<llvm::ConstantInt>(Intrins->getOperand(2))) {
              index = dyn_cast<llvm::ConstantInt>(Intrins->getOperand(2))
                          ->getZExtValue();
            }
          } else if (Inid == Intrinsic::tpc_set_indx) {
            processSetIndexNode(TensorId, CurrLoop, PHI, F, Intrins);
            RootInst = Intrins;
            Nested_Ins = true;
            continue;
          }
          RootInst = Intrins;
          Nested_Ins = true;
          unsigned Count = fetchIndexFromOperand(index);
          prepareLoopIvCoordMap(TensorId, CurrLoop, Count);
          break;
        }
      } else if (auto *PHIPtr = dyn_cast<PHINode>(RootInstUser)) {
        // check depth. If the edge traces to some other loop depth, dont
        // process
        Loop *LoopPtr1 = LI->getLoopFor(PHIPtr->getParent());
        Loop *LoopPtr2 = LI->getLoopFor(PHI->getParent());
        if (LoopPtr1 && LoopPtr2) {
          int InDepth1 = LoopPtr1->getLoopDepth();
          int InDepth2 = LoopPtr2->getLoopDepth();
          if (!iterateProcessedPHI(PHIPtr) && InDepth1 == InDepth2) {
            RootInst = PHIPtr;
            Nested_Ins = true;
            addToProcessedPHI(PHIPtr);
            PHI = PHIPtr;
            break;
          }
        }
      }
    }
  }
}

void SCEVInfo::dump() const {
  if (!SCEVPtr) {
    llvm::dbgs() << "Not a valid SCEV Node"
                 << "\n";
    return;
  }
  llvm::dbgs() << "Loop Name --> " << L->getName().str() << "\n";
  llvm::dbgs() << "Loop Depth --> " << L->getLoopDepth() << "\n";
  llvm::dbgs() << "ScevValue --> "
               << "\n";
  SCEVPtr->print(llvm::dbgs());
  llvm::dbgs() << "\n";
  llvm::dbgs() << "ScevStep --> "
               << "\n";
  llvm::dbgs() << getSCEVStep();
  llvm::dbgs() << "\n";
}

void TensorAccessAnalysis::updateCoordFactor(unsigned int TensorID,
                                             unsigned int DimIndex,
                                             int DimFactor,
                                             std::string DimFormula, Loop *L) {
  if (L != nullptr) {
    prepareLoopIvCoordMap(TensorID, L, DimIndex, DimFormula);
  } else {
    // No loops available just update the coords
    DiffCoords.insert({TensorID, std::make_tuple(DimIndex, DimFactor,
                                                 DimFormula, L, nullptr)});
    prepareLoopIvCoordMap(TensorID, nullptr, DimIndex, DimFormula);
  }
}
