//===- TPCoptimization.cpp --- IR pipelined--------------------------------===//
//
//
//                     The LLVM Compiler Infrastructure:
//
//              2019 - This pass is a property of Habana labs
//
//
//===----------------------------------------------------------------------===//
//   TPC-IR pipelined optimization
//
//   This pass is a pipeline optimization. The pass accepts code with unroll
//   pragma the equal or greater to 1. The Unroll factor is calculated according
//   to reuse patterns that are in the inner loop.
//
//   The pass works on three Basic blocks described by two loops. The inner loop
//   as shown in the example LOOP1.2 and the outer loop as LOOP1. The pass
//   extracts loads from the inner loop to the other loop and then duplicates
//   them after all stores in the inner loop. Since loads instructions lead
//   stores instruction with a gap of one iteration. The pass dupllicats
//   instructions between load and store and stores to an exit point.
//
//   For now, the pass supports the following scenarios:
//   1) load and store of the same class.
//   2) The number of loads and stores are equal and greater than 1.
//   3) Unroll factor between 1 to 4.
//
//  This pass transform the code in the following way:
//
//  begin                    begin
//  LOOP1:                   LOOP1:          *prolog*
//      LOOP1.2:                in = load_type
//        in = load_type        LOOP1.2: *inner loop*
//        Inst1(in)                 Inst1(in)
//        inst2,inst3      ==>      inst2,inst3
//        res = inst4               res = inst4
//        store(res)                store(res)
//  LOOP1.EXIT                      in = load_type
//  ##end                    LOOP1.EXIT:     *epilog*
//  Into                     Inst1(in)
//                           inst2,inst3
//                           res = inst4
//                           store(res)
//                           end
//
//===----------------------------------------------------------------------===//

#include "TPCTargetMachine.h"
#include "llvm/InitializePasses.h"
#include "llvm/Analysis/AssumptionCache.h"
#include "llvm/Analysis/MemoryBuiltins.h"
#include "llvm/Analysis/ScalarEvolution.h"
#include "llvm/IR/IntrinsicsTPC.h"
#include "llvm/Transforms/Utils/LoopUtils.h"
#include "llvm/Transforms/Utils/UnrollLoop.h"
using namespace llvm;

#define PassName "tpcopt"
#define PassDescription "Create pipeline optimization in the level of the IR"
#define DEBUG_TYPE PassName
#include <iostream>

static cl::opt<bool> tpcoptFlag("unroll-pipelined",cl::init(true), cl::Hidden);
static cl::opt<bool> tpcoptDebug("TPCOPT_DEBUG",cl::init(false), cl::Hidden);


using namespace std;
class TpcLoopOpt : public LoopPass {
public:
    static char ID;
    TpcLoopOpt() : LoopPass(ID) {
      initializeTpcLoopOptPass(*PassRegistry::getPassRegistry());
    }

    void getAnalysisUsage(AnalysisUsage &AU) const override {
      AU.addPreserved<ScalarEvolutionWrapperPass>();
      getLoopAnalysisUsage(AU);
    }

private:
    Loop *WorkingLoop = nullptr;
    Instruction *InsertElement2 = nullptr;
    Instruction *Add32 = nullptr;
    std::vector<Instruction *> LoadPtrList;
    std::vector<Instruction *> StorePtrList;
    SmallVector<Instruction *, 16> CloneInstruction;
    SmallVector<Instruction *, 16> DuplicateInstruction;
    bool Diagnostic = false;
    std::vector<Instruction *> InstructionBufferArray;
    Intrinsic::ID StoreId = Intrinsic::not_intrinsic;
    Intrinsic::ID LoadId = Intrinsic::not_intrinsic;
    unsigned CycleSize = 0;
    unsigned UnrollSize = 0;
    Instruction *InducationPhi = nullptr;
    llvm::ConstantInt *InductionSize = nullptr;
    llvm::ConstantInt *IRFLocation = nullptr;
    // Pass condition functions
    bool runOnLoop(Loop *Lp, LPPassManager &LPM) override;
    bool loadAndSet(Intrinsic::ID intrinId);
    bool unrollPragmaValue(const Loop *L);
    bool createCopyInstruction();
    bool patternCheck(Loop *L);

    // Pass transform functions
    bool reorder();
    Instruction *prologue(BasicBlock *outerBB, BasicBlock *innerBB);
    SmallVector<Instruction *, 4> inner(BasicBlock *BB0, BasicBlock *BB1,
                                        Instruction *lastAddLoadBB0);
    void cleanUp(BasicBlock *BB2, SmallVector<Instruction *, 4> PhiInst);
    Instruction *incInstruction(BasicBlock *to, BasicBlock *from,
                                Intrinsic::ID idIntrin, Instruction *input,
                                Instruction *firstElment = nullptr,
                                Instruction *moveAfter = nullptr);
    // Support pass functions
    Instruction *creatCloneIntrinsics(Instruction *after, Instruction *pointer,
                                      bool dontUpdate);

    void fixPhiInstruction(BasicBlock *BB);
};

INITIALIZE_PASS_BEGIN(TpcLoopOpt, PassName, PassDescription, false, false)
  INITIALIZE_PASS_DEPENDENCY(LoopPass)
INITIALIZE_PASS_END(TpcLoopOpt, PassName, PassDescription, false, false)

char TpcLoopOpt::ID = 0;

LoopPass *llvm::createTpcLoopOptPass() { return new TpcLoopOpt(); }

static MDNode *GetUnrollMetadataForLoop(const Loop *L, StringRef Name) {
  if (MDNode *LoopID = L->getLoopID())
    return GetUnrollMetadata(LoopID, Name);
  return nullptr;
}

bool TpcLoopOpt::unrollPragmaValue(const Loop *L) {
  // Function check if the pragam unroll was deploy in the code before the loop.
  // unroll > 1 || unroll == 1
  // TODO: In the future we can add new pragam "piplined"
  if (GetUnrollMetadataForLoop(L, "llvm.loop.unroll.disable") ||
      GetUnrollMetadataForLoop(L, "llvm.loop.unroll.count"))
    return false;
  return true;
}

template <typename T, unsigned N>
static SmallVector<T *, N> get_list_of(BasicBlock *const *BB) {
  // Retrun a vector of all element correspond to type <T>
  // indise the basicblock <BB>.
  SmallVector<T *, N> list;
  for (BasicBlock::iterator II = (*BB)->begin(); II != (*BB)->end(); II++)
    if (auto Intrin = dyn_cast<T>(II))
      list.push_back(Intrin);
  return list;
}

static bool check_if_debug(Loop *lp)
{
  BasicBlock* head = lp->getHeader();
  Module *M = head->getModule();
  NamedMDNode *CUs = M->getNamedMetadata("llvm.dbg.cu");
  return CUs != nullptr;
}

bool TpcLoopOpt::runOnLoop(Loop *lp, LPPassManager &LPM) {
  // Virtual function: RunOnLoop function checks if the condiatios are stasfied
  // If yes continu to reorder.
  // Else return false.
  if(!tpcoptFlag)
    return false;
  // incorrect work was found with -g
  if (check_if_debug(lp)) {
    return false;
  }
  WorkingLoop = lp;
  // Check pragma unroll.
  if (unrollPragmaValue(lp))
    return false;
  // The loop is the inner loop with one control flow.
  if (lp->getNumBlocks() > 1)
    return false;

  // Clear the lists
  LoadPtrList.clear();
  StorePtrList.clear();

  // The pattern of the loop correspond to the file's comment. If there is no
  // induction return false.
  if (patternCheck(lp)) {
    // Find the induction variable and the correlate array IRF location.
    InductionDescriptor ID;
    vector<BasicBlock *> basicBlockVec = WorkingLoop->getBlocksVector();
    SmallVector<PHINode *, 8> phi = get_list_of<PHINode, 8>(&basicBlockVec[0]);
    auto *SE = &getAnalysis<ScalarEvolutionWrapperPass>().getSE();

    if (InductionDescriptor::isInductionPHI(phi[0], WorkingLoop, SE, ID)) {
      InductionSize = ConstantInt::get(
              Type::getInt32Ty(phi[0]->getContext()),
              ID.getConstIntStepValue()->getSExtValue() / UnrollSize);
      for (auto users : phi[0]->users()) {
        if (auto insertElement = dyn_cast<InsertElementInst>(users)) {
          for (auto serachLoadUse : insertElement->users())
            if (serachLoadUse == LoadPtrList[0]) {
              InducationPhi = phi[0];
              auto CI = dyn_cast<ConstantInt>(insertElement->getOperand(2));
              IRFLocation =
                      ConstantInt::get(Type::getInt32Ty(phi[0]->getContext()),
                                       CI->getSExtValue() + 1);
              break;
            }
        }
        if (IRFLocation != nullptr)
          break;
      }
    } else
      return false;
    // Check if IRF insert element is exist.
    if (IRFLocation == nullptr)
      return false;
    // If so do the transformation.
    return reorder();
  }
  return false;
}

bool TpcLoopOpt::loadAndSet(Intrinsic::ID intrinId) {
  // todo: it not necessary that load and store will equal to each other
  switch (intrinId) {
    default:
      return false;
    case Intrinsic::tpc_ld_tnsr:
      StoreId = Intrinsic::tpc_st_tnsr;
      break;
  }
  if (LoadId == Intrinsic::not_intrinsic)
    LoadId = intrinId;
  else
    return LoadId == intrinId;
  return true;
}

static vector<Instruction *>
extractIntrinFromList(SmallVector<IntrinsicInst *, 16> intrinsicList,
                      Intrinsic::ID intrinId) {
  vector<Instruction *> selectInst;
  for (unsigned j = 0; j < intrinsicList.size(); j++) {
    if (intrinsicList[j]->getIntrinsicID() == intrinId)
      selectInst.push_back(intrinsicList[j]);
  }
  return selectInst;
}

bool TpcLoopOpt::createCopyInstruction() {
  // The function prepares a list of instruction between load and store.
  // The count indicate the number of instruction between the load and store.
  unsigned count = 0;
  for (unsigned i = 0; i < LoadPtrList.size(); i++) {
    Instruction *ptr = LoadPtrList[i]->getNextNode();
    while (ptr != StorePtrList[i]) {
      InstructionBufferArray.push_back(ptr);
      ptr = ptr->getNextNode();
      count++;
    }
    if (CycleSize == 0)
      CycleSize = count;
    else {
      // Check that each load store gap is the same.
      if (CycleSize != count)
        return false;
    }
    count = 0;
  }
  return true;
}

bool TpcLoopOpt::patternCheck(Loop *L) {
  auto *BB = L->block_begin();
  SmallVector<IntrinsicInst *, 16> intrinList =
          get_list_of<IntrinsicInst, 16>(BB);
  if (intrinList.size() < 2)
    return false;
  bool flag = false;
  Value *ptr = nullptr, *ptr2 = nullptr;
  // Build the load and store instruction list.
  for (auto *runner : intrinList) {
    if (loadAndSet(runner->getIntrinsicID())) {
      if (!flag) {
        flag = !flag;
        ptr = dyn_cast<Instruction>(runner)->getOperand(0);
        LoadPtrList.push_back(runner);
      } else
        return false;
    } else if (((runner)->getIntrinsicID() == StoreId)) {
      if (flag) {
        flag = !flag;
        ptr2 = dyn_cast<Instruction>(runner)->getOperand(0);
        StorePtrList.push_back(runner);
        if (dyn_cast<Instruction>(ptr) == dyn_cast<Instruction>(ptr2))
          Diagnostic = true;
      } else
        return false;
    }
  }
  // If there is no store complete the load instruction return false
  if (flag)
    return false;

  if (!createCopyInstruction())
    return false;
  // Since we checking match between load and store and only one type of load.
  // We can compute the UnrollSize.
  UnrollSize = LoadPtrList.size();

  // For now only support up to three instruction gaps.
  if (CycleSize > 4 || CycleSize == 0 || UnrollSize > 4 || UnrollSize == 0)
    return false;

  BB = L->getParentLoop()->block_begin();

  // For now the outer loop need to be empty from load.
  return extractIntrinFromList(get_list_of<IntrinsicInst, 16>(BB), LoadId)
          .empty();
}

static SmallVector<Instruction *, 4>
extractIntrinFromList(SmallVector<Instruction *, 16> instList,
                      Intrinsic::ID intrinId) {
  SmallVector<Instruction *, 4> selectInst;
  for (unsigned j = 0; j < instList.size(); j++)
    if (auto val = dyn_cast<IntrinsicInst>(instList[j])) {
      if (val->getIntrinsicID() == intrinId)
        selectInst.push_back(instList[j]);
    }
  return selectInst;
}

Instruction *TpcLoopOpt::incInstruction(BasicBlock *to, BasicBlock *from,
                                        Intrinsic::ID idIntrin,
                                        Instruction *input,
                                        Instruction *firstElment,
                                        Instruction *moveAfter) {
  // incInstruction creates add instruction for each use of the load and store.
  // The function return the last load's/store's ADD.
  IRBuilder<> builder(to);
  SmallVector<Instruction *, 16> instList = get_list_of<Instruction, 16>(&from);
  // Get all load/store instruction for increment the IRF.
  SmallVector<Instruction *, 4> intrinsicList =
          extractIntrinFromList(get_list_of<Instruction, 16>(&to), idIntrin);
  LLVMContext &C = from->getContext();
  Type *Int5Ty = FixedVectorType::get(Type::getInt32Ty(C), 5);
  //Value *undef = UndefValue::get(Int5Ty);
  Instruction *add = input;
  Function *func = Intrinsic::getDeclaration(to->getModule(),
      Intrinsic::tpc_add, { Int5Ty, Type::getInt32Ty(C), Int5Ty, Type::getInt1Ty(C) });

  assert(instList.size() > 2 && "number of instruction smaller then two");
  if (!moveAfter)
    moveAfter = instList[instList.size() - 2];

  for (auto *intrin = intrinsicList.begin(), *end = intrinsicList.end();
       intrin != end; ++intrin) {
    // Create the add instruction where the func is intrinsics InductionSize
    // is the loop inducation and the IRF location is location inside the IRF
    // array is.
    add = builder.CreateCall(func, {InductionSize,
                                    add,
                                    ConstantInt::get(Type::getInt8Ty(C),
                                                     TPCII::OpType::INT32),
                                    IRFLocation,
                                    add,
                                    ConstantInt::get(Type::getInt1Ty(C), 1),
                                    ConstantInt::get(Type::getInt1Ty(C), 0) });
    (*intrin)->moveAfter(moveAfter);
    (*intrin)->setOperand(0, input);
    add->moveAfter(*intrin);
    moveAfter = add;
    input = add;
    if (firstElment) {
      firstElment->insertBefore(*intrin);
      firstElment = nullptr;
    }
  }
  return add;
}

Instruction *TpcLoopOpt::prologue(BasicBlock *outerBB, BasicBlock *innerBB) {
  // The prologue function extracts the inner loop's load to the outer loop.
  // Return the last add.

  // for.body: -> OuterBB                 // for.body: -> OuterBB
  // %h = Outer var indeucation           // %h = Outer var indeucation
  // %IRF[3] = %h                 ==>     // %0 = IRF
  // %br condition inner outer            // %temp = add unroll factor to temp
                                          // %1 = load call(%0)
                                          // %2 = add %0,1
                                          // %3 = load call(%2)
                                          // %4 = add %2,1
                                          // Continue ass unroll factor
                                          // %br condition inner outer

  // Extract the insertElement for the load and store elements.
  // The inserElement saves the current stat of the IRF will prepares the gap between
  // store's and load's IRF.
  Instruction *insertElement1 =
          dyn_cast<Instruction>(LoadPtrList[0]->getOperand(0))->clone();
  InsertElement2 =
          dyn_cast<Instruction>(StorePtrList[0]->getOperand(0))->clone();
  insertElement1->setOperand(
          0, dyn_cast<Instruction>(insertElement1->getOperand(0))->getOperand(1));
  InsertElement2->setOperand(
          0, dyn_cast<Instruction>(InsertElement2->getOperand(0))->getOperand(1));
  insertElement1->setOperand(1, InducationPhi->getOperand(1));
  InsertElement2->setOperand(1, InducationPhi->getOperand(1));

  Instruction *lastAdd =
          incInstruction(innerBB, outerBB, LoadId, insertElement1, insertElement1);
  InsertElement2->insertAfter(insertElement1);
  IRBuilder<> builder(outerBB);
  Add32 = cast<Instruction>(builder.CreateAdd(
          InsertElement2->getOperand(1),
          ConstantInt::get(builder.getInt32Ty(), LoadPtrList.size())));
  Add32->moveAfter(InsertElement2);
  return lastAdd;
}

Instruction *TpcLoopOpt::creatCloneIntrinsics(Instruction *after,
                                              Instruction *pointer,
                                              bool dontUpdate = true) {
  pointer->clone()->insertAfter(after);
  if (dontUpdate)
    CloneInstruction.push_back(pointer);
  after = after->getNextNode();
  if (dontUpdate) {
    DuplicateInstruction.push_back(after);
    for (unsigned op = 0; op < after->getNumOperands(); op++) {
      if (auto *a = dyn_cast<Instruction>(after->getOperand(op))) {
        for (unsigned i = 0; i < CloneInstruction.size(); i++) {
          if (CloneInstruction[i] == a) {
            after->setOperand(op, DuplicateInstruction[i]);
            break;
          }
        }
      }
    }
  }
  return after;
}

static PHINode *insetPhiInst(Instruction *first, Instruction *second,
                             BasicBlock *firstBB, BasicBlock *secondBB,
                             IRBuilder<> &builder, Type *ptrType, int number,
                             Instruction *after) {
  PHINode *phiLoad = builder.CreatePHI(FixedVectorType::get(ptrType, number), 2);
  phiLoad->addIncoming(first, firstBB);
  phiLoad->addIncoming(second, secondBB);
  phiLoad->moveAfter(after);
  return phiLoad;
}

static SmallVector<Instruction *, 4>
createPhiInstruction(vector<Instruction *> *selectItrin,
                     vector<BasicBlock *> basicBlockLoc, IRBuilder<> &builder,
                     Instruction *after) {
  SmallVector<Instruction *, 4> result;
  PHINode *phiLoad;
  for (unsigned i = 0; i < selectItrin[0].size(); i++) {
    int number = cast<FixedVectorType>(selectItrin[0][i]->getType())->getNumElements();
    Type *ptrType = cast<FixedVectorType>(selectItrin[0][i]->getType())->getElementType();
    phiLoad =
            insetPhiInst(selectItrin[0][i], selectItrin[1][i], basicBlockLoc[0],
                         basicBlockLoc[1], builder, ptrType, number, after);
    result.push_back(phiLoad);
    after = after->getNextNode();
  }
  return result;
}

static SmallVector<Instruction *, 4>
phiFromTwoBasicBlock(Intrinsic::ID intrinId, Instruction *after,
                     BasicBlock *firstBB, BasicBlock *secondBB,
                     BasicBlock *firstBBLoc, BasicBlock *secondBBLoc) {
  // The function phiFromTwoBasicBlock creates Phi instruction from to
  // instruction locate in the tow BasicBlockLoc var.
  IRBuilder<> builder(secondBB);
  SmallVector<IntrinsicInst *, 16> itrin[2] = {
          get_list_of<IntrinsicInst, 16>(&firstBB),
          get_list_of<IntrinsicInst, 16>(&secondBB)};
  vector<Instruction *> selectItrin[2];
  for (int i = 0; i < 2; i++)
    selectItrin[i] = extractIntrinFromList(itrin[i], intrinId);
  assert(selectItrin[0].size() == selectItrin[1].size() &&
         "To many confuse instruction");
  return createPhiInstruction(selectItrin, {firstBBLoc, secondBBLoc}, builder,
                              after);
}

void TpcLoopOpt::fixPhiInstruction(BasicBlock *BB) {
  SmallVector<Instruction *, 16> instrucitonList =
          get_list_of<Instruction, 16>(&BB);
  SmallVector<PHINode *, 4> PhiInst = get_list_of<PHINode, 4>(&BB);
  if (!PhiInst.empty()) {
    for (auto start = instrucitonList.begin() + PhiInst.size(),
                 end = instrucitonList.end();
         start != end; start++) {
      for (unsigned op = 0; op < (*start)->getNumOperands(); op++) {
        if (auto *a = dyn_cast<Instruction>((*start)->getOperand(op))) {
          for (unsigned i = 0; i < PhiInst.size(); i++) {
            for (unsigned j = 0; j < PhiInst[i]->getNumOperands() - 1; j++) {
              if (PhiInst[i]->getOperand(j) == a &&
                  PhiInst[i]->getParent() != a->getParent()) {
                (*start)->setOperand(op, PhiInst[i]);
                break;
              }
            }
          }
        }
      }
    }
  }
}

void splitCoordinate(vector<Instruction *> load, vector<Instruction *> store) {
  // splitCoordinate insert  new coordinate when a coordinate is shared by
  // both load and store
  IRBuilder<> builder(load[0]->getParent());
  unsigned i = 0;
  for (auto runOnLoad : load) {
    auto *cord = dyn_cast<Instruction>(runOnLoad->getOperand(0));
    cord->clone()->insertAfter(cord);
    cord = cord->getNextNode();
    store[i]->setOperand(0, cord);
    i++;
  }
}

SmallVector<Instruction *, 4> TpcLoopOpt::inner(BasicBlock *BB0,
                                                BasicBlock *BB1,
                                                Instruction *lastAddLoadBB0) {
  //      LOOP1.2:                LOOP1.2: *inner loop*
  //        in = load_type            Inst1(in0)
  //        Inst1(in)                 inst2,inst3
  //        inst2,inst3      ==>      res = inst4
  //        res = inst4               Inst1(in1)
  //        store(res)                inst2,inst3
  //  LOOP1.EXIT                      res = inst4
  //                                  in0 = load_type(IRF[L].1)
  //                                  IRF[L].1++
  //                                  in1 = load_type(IRF[L].1)
  //                                  IRF[L].1++
  //                                  store(res,IRF[L].2)
  //                                  IRF[L].2++
  //                                  store(res,IRF[L].2)
  //                                  IRF[L].2++
  //                                  ... repeat as unroll factor

  // Transform the inner loop into pipelined version return a list phi instructions.

  // Search for the last intrinsics.
  auto *after = (Instruction *)get_list_of<IntrinsicInst, 16>(&BB1).back();
  // Copy the load instruction to the location point after.
  for (auto load : LoadPtrList)
    after = creatCloneIntrinsics(after, load, false);

  // For each clone of store and load create inc the IRF.
  Instruction *lastAddLoadBB1 =
          incInstruction(BB1, BB1, LoadId, lastAddLoadBB0, nullptr, after);
  Instruction *lastAddStoreBB1 = incInstruction(
          BB1, BB1, StoreId, dyn_cast<Instruction>(StorePtrList[0]->getOperand(0)),
          nullptr, lastAddLoadBB1);

  // Creates phi instruction for all load.
  BasicBlock *preHeader = WorkingLoop->getLoopPreheader();
  SmallVector<Instruction *, 4> phiInst =
          phiFromTwoBasicBlock(LoadId, (&*(*WorkingLoop->block_begin())->begin()),
                               BB0, BB1, preHeader, BB1);

  IRBuilder<> builder(BB1);
  // Create phi for laod from the outer loop.
  PHINode *phiLoad = insetPhiInst(
          lastAddLoadBB0, lastAddLoadBB1, preHeader, lastAddLoadBB1->getParent(),
          builder, Type::getInt32Ty(lastAddLoadBB1->getContext()), 5,
          phiInst.back());

  SmallVector<IntrinsicInst *, 16> intrinList =
          get_list_of<IntrinsicInst, 16>(&BB1);

  LoadPtrList = extractIntrinFromList(intrinList, LoadId);
  LoadPtrList[0]->setOperand(0, phiLoad);
  LoadPtrList[0]->getNextNode()->setOperand(1, phiLoad);

  // also set operand 4 ('income' operand) to be the same as operand 1
  LoadPtrList[0]->getNextNode()->setOperand(4, phiLoad);

  StorePtrList = extractIntrinFromList(intrinList, StoreId);
  SmallVector<Instruction *, 16> inInstVec = get_list_of<Instruction, 16>(&BB0);

  // Create a phi instruction for the store instruction that come from inner loop and outer loop.
  phiLoad = insetPhiInst(InsertElement2, lastAddStoreBB1, preHeader,
                         lastAddStoreBB1->getParent(), builder,
                         Type::getInt32Ty(lastAddStoreBB1->getContext()), 5,
                         phiInst.back()->getNextNode());

  StorePtrList[0]->setOperand(0, phiLoad);
  StorePtrList[0]->getNextNode()->setOperand(1, phiLoad);

  // also set operand 4 ('income' operand) to be the same as operand 1
  StorePtrList[0]->getNextNode()->setOperand(4, phiLoad);

  // Set the gap between the load IRF and the store IRF.
  get_list_of<Instruction, 16>(&BB1)[0]->setOperand(1, Add32);
  return phiInst;
}

void TpcLoopOpt::cleanUp(BasicBlock *BB2,
                         SmallVector<Instruction *, 4> PhiInst) {
  //  LOOP1.EXIT               in = load_type
  //  ##end                    LOOP1.EXIT:     *epilog*
  //                  =>       Inst1(in)
  //                           inst2,inst3
  //                           res = inst4
  //                           store(res,IRF[L].2)
  //                           IRF[L].2++
  //                           Inst1(in1)
  //                           inst2,inst3
  //                           res = inst4
  //                           store(res,IRF[L].2)
  //                           IRF[L].2++
  //                           ... repeat as unroll factor
  //                           end
  //

  Instruction *after;
  SmallVector<Instruction *, 16> intrinListBB2 =
          get_list_of<Instruction, 16>(&BB2);
  intrinListBB2[0]->setOperand(1, StorePtrList.back()->getNextNode());
  intrinListBB2[0]->setOperand(0, Add32->getPrevNode());

  if (Diagnostic)
    after = intrinListBB2[0];
  else
    after = intrinListBB2[1];

  IRBuilder<> builder(BB2);
  for (unsigned i = 0; i < PhiInst.size(); i++) {
    auto *first = dyn_cast<Instruction>(PhiInst[i]->getOperand(0));
    auto *second = dyn_cast<Instruction>(PhiInst[i]->getOperand(1));
    after = insetPhiInst(first, second, first->getParent(),
                         WorkingLoop->getExitBlock(), builder,
                         cast<FixedVectorType>(PhiInst[i]->getType())->getElementType(),
                         cast<FixedVectorType>(PhiInst[i]->getType())->getNumElements(), after);
  }

  // Duplicate the instruction between the load and the store to the cleanup block.
  for (unsigned i = 0; i < InstructionBufferArray.size() / UnrollSize; i++)
    for (unsigned j = 0; j < UnrollSize; j++)
      after = creatCloneIntrinsics(after,
                                   InstructionBufferArray[i + CycleSize * j]);

  // Duplicate the store to the cleanup block.
  for (auto store : StorePtrList)
    after = creatCloneIntrinsics(after, store);

  incInstruction(BB2, BB2, StoreId, intrinListBB2[0], nullptr, after);
}

bool TpcLoopOpt::reorder() {
  if(tpcoptDebug)
    llvm::dbgs() << "kernel pipelined \n";

  Loop *L = WorkingLoop->getParentLoop();

  vector<BasicBlock *> basicBlockVec = L->getBlocksVector();

  if (Diagnostic)
    splitCoordinate(LoadPtrList, StorePtrList);

  // Prologue
  Instruction *lastAddLoadBB0 = prologue(basicBlockVec[0], basicBlockVec[1]);

  // inner loop
  SmallVector<Instruction *, 4> phiInst =
          inner(basicBlockVec[0], basicBlockVec[1], lastAddLoadBB0);

  // Epilogue
  cleanUp(basicBlockVec[2], phiInst);

  // Fix all phi instruction to point to the correct instructions.
  for (unsigned i = 0; i < 3; i++)
    fixPhiInstruction(basicBlockVec[i]);

  return true;
}
