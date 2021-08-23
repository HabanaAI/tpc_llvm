//===- TPCIndexSpace.cpp --- TPC INDEX SPACE ------------------------------===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//
//===----------------------------------------------------------------------===//

#include "TPCIndexSpace.h"
#include "llvm/IR/IntrinsicsTPC.h"

char TPCIndexMap::ID = 0;
INITIALIZE_PASS_BEGIN(TPCIndexMap, PassName, PassDescription, false, false)
INITIALIZE_PASS_END(TPCIndexMap, PassName, PassDescription, false, false)
FunctionPass *llvm::createTPCIndexMap() { return new TPCIndexMap(); }

class LoadToStorIRFEV {
  Instruction *Load;
  Instruction *Store;
  Value *LoadIRF[5];
  Value *StoreIRF[5];

public:
  LoadToStorIRFEV(Instruction *load, Instruction *store, LoopInfo *LII)
      : Load(load), Store(store) {}
  void computeIndexMap(Function &F, ScalarEvolution *SE, LoopInfo *LI);
  int findIRF(Value *begin, Value *IRF[]);
};



bool TPCIndexMap::runOnFunction(Function &F) {
  p_func = &F;
  p_SE = &getAnalysis<ScalarEvolutionWrapperPass>().getSE();
  p_LI = &getAnalysis<LoopInfoWrapperPass>().getLoopInfo();
  collectDataLoop(F, p_SE, p_LI);
  sort();
  print_data();
  return false;
}

void TPCIndexMap::sort() {
  LoopData loopTemp[5];
  int size = 0;
  for (unsigned long i = 0; i < m_loopInfoVec.size(); i++) {
    if (m_loopInfoVec[i].is_Valid()) {
      loopTemp[m_loopInfoVec[i].get_DIM()] = m_loopInfoVec[i];
      size++;
    }
  }
  for (int i = 0; i < size; i++)
      m_loopInfoVec[i] = loopTemp[i];
}

static vector<Constant *> createAString(std::string input, Type *Int8Ty) {
  vector<Constant *> Init;
  for (unsigned long i = 0; i < input.size(); i++) {
    Init.push_back(ConstantInt::get(Int8Ty, input[i]));
  }
  return Init;
}

void TPCIndexMap::print_data() {
  std::string index_space = "IndexSpace:";
  StringRef Name = "IndexSpace";
  // llvm::dbgs() << "IndexSpace : {";
  for (auto run : m_loopInfoVec) {
    if (run.is_Valid()) {
      // llvm::dbgs() << run.get_STEP() << ",";
      index_space += std::to_string(run.get_STEP()) + ",";
    }
  }
  // llvm::dbgs() << "}\n";
  index_space += "#";
  Type *Int8Ty = llvm::Type::getInt8Ty(p_func->getContext());
  vector<Constant *> Init = createAString(index_space, Int8Ty);
  ArrayType *ATy = ArrayType::get(Int8Ty, Init.size());
  llvm::GlobalVariable *GV0 = new llvm::GlobalVariable(
      *p_func->getParent(), ATy, true, GlobalValue::ExternalLinkage,
      ConstantArray::get(ATy, Init), Name);
  GV0->setSection(".SCEV");
}

template <typename T> static vector<T *> get_list_of(BasicBlock *BB) {
  // Retrun a vector of all element correspond to type <T>
  // indise the basicblock <BB>.
  vector<T *> list;
  for (BasicBlock::iterator II = (*BB).begin(); II != (*BB).end(); II++)
    if (auto Intrin = dyn_cast<T>(II))
      list.push_back(Intrin);
  return list;
}

static vector<Instruction *>
extractIntrinFromList(vector<IntrinsicInst *> intrinsicList,
                      Intrinsic::ID intrinId) {
  vector<Instruction *> selectInst;
  for (auto run : intrinsicList) {
    if (run->getIntrinsicID() == intrinId)
      selectInst.push_back(run);
  }
  return selectInst;
}

void TPCIndexMap::collectDataLoop(Function &F, ScalarEvolution *SE,
                                  LoopInfo *LI) {

  for (auto &Bb : F.getBasicBlockList()) {
    if (LI->isLoopHeader(&Bb)) {
      Loop *Lp = LI->getLoopFor(&Bb);
      LoopData L(Lp, SE);
      m_loopInfoVec.push_back(L);
    }
  }

  vector<IntrinsicInst *> IntrinsicsList;
  vector<Instruction *> listOfLoad;
  vector<Instruction *> listOfStore;
  vector<LoadToStorIRFEV> Load2Store;
  vector<Instruction *> temp;
  unsigned long intrinInd = 0;
  // todo: return the type of the intrinsics and is_load , is_Store
  vector<std::pair<Intrinsic::ID, Intrinsic::ID>> intrinSics = {
      {Intrinsic::tpc_ld_tnsr, Intrinsic::tpc_st_tnsr},

  };
  do {
    for (auto &Bb : F.getBasicBlockList()) {

      listOfLoad = extractIntrinFromList(get_list_of<IntrinsicInst>((&Bb)),
                                         intrinSics[intrinInd].first);
      for (auto &runer : listOfLoad)
          m_load.push_back(runer);
      listOfStore = extractIntrinFromList(get_list_of<IntrinsicInst>((&Bb)),
                                          intrinSics[intrinInd].second);
      for (auto &runer : listOfStore)
          m_store.push_back(runer);
    }
    temp = m_load.size() > m_store.size() ? m_store : m_load;
    intrinInd++;
  } while (temp.size() == 0 && intrinInd < intrinSics.size());

  if (m_load.size() != m_store.size()) {
    // TODO: Check what append with diffenet size of load and store
    dbgs() << "This is not support it different size of load and store\n";
  }
  for (unsigned long i = 0; i < temp.size(); i++) {
    // Todo: Connect the load to store instruction
    Load2Store.push_back(LoadToStorIRFEV(m_load[i], m_store[i], LI));
  }
  if (m_load.size() < 1 || m_store.size() < 1) {
    // TODO: Support all load and size
    dbgs() << "We only support load and store of tpc_v_f32_ld_tnsr_i\n";
    return;
  }
  Load2Store[0].computeIndexMap(F, SE, LI);
}

vector<Value *> InsertInstruction;

int LoadToStorIRFEV::findIRF(Value *begin, Value *IRF[]) {
  // Todo: find all dimanations via all phis.
  bool check[5] = {false, false, false, false, false};
  while (begin) {
    InsertInstruction.push_back(begin);
    if (Instruction *ptr = dyn_cast<Instruction>(begin)) {
      if (ptr->getOpcode() == Instruction::PHI)
        dbgs() << "we don't support direct Indexing\n";
      if (ptr->getOpcode() != Instruction::InsertElement)
        break;
      int index =
          dyn_cast<llvm::ConstantInt>(ptr->getOperand(2))->getZExtValue();
      IRF[index] = ptr->getOperand(1);
      check[index] = true;
      begin = ptr->getOperand(0);
    } else
      break;
  }
  if (!check[0])
    return 0;
  for (int i = 1; i < 5; i++) {
    if (!check[i]) {
      for (int j = i + 1; j < 5; j++)
        if (check[j])
          return 0;
      return i;
    }
  }
  return 5;
}

void LoadToStorIRFEV::computeIndexMap(Function &F, ScalarEvolution *SE,
                                      LoopInfo *LI) {
  Value *IRFLoad = Load->getOperand(0);
  Value *IRFStore = Store->getOperand(0);
  int IRFLengthLoad = findIRF(IRFLoad, LoadIRF);
  int IRFLengthStore = findIRF(IRFStore, StoreIRF);
  if (IRFLengthLoad == IRFLengthStore) {
    for (int i = 0; i < IRFLengthStore; i++) {
      SCEVParser scevRunerLoad(SE->getSCEV(LoadIRF[i]), SE, F.getParent());
        scevRunerLoad.parseExpr(i, 'L');
      SCEVParser scevRunerStore(SE->getSCEV(StoreIRF[i]), SE, F.getParent());
        scevRunerStore.parseExpr(i, 'S');
    }
  }
}
