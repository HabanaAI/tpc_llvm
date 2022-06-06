//===---- TPCSelectorPreshaper.cpp ----------------------------------------===//
//
//===----------------------------------------------------------------------===//
//
//
//===----------------------------------------------------------------------===//

#include "llvm/Analysis/TargetLibraryInfo.h"
#include "llvm/IR/Instructions.h"
#include "llvm/IR/IntrinsicInst.h"
#include "llvm/IR/IntrinsicsTPC.h"
#include "llvm/IR/IRBuilder.h"
#include "llvm/IR/PatternMatch.h"
#include "llvm/Analysis/VectorUtils.h"
#include "llvm/Pass.h"
#include "llvm/Transforms/Utils/LowerMemIntrinsics.h"
#include "TPCTargetMachine.h"
#include "llvm/IR/DebugInfoMetadata.h"

using namespace llvm;

namespace llvm {
FunctionPass *createTPCSelectorPreshaper();
void initializeTPCSelectorPreshaperLegacyPassPass(PassRegistry&);
}

static const char PassDescription[] = "TPC selector preshaper";
static const char PassName[] = "tpc-preshape";

static cl::opt<bool>
EnableNearbyintWorkaround("tpc-nearbyint-workaround",
  cl::Hidden,
  cl::init(true));


#define DEBUG_TYPE "tpc-movopt"

namespace {
class TPCSelectorPreshaperLegacyPass : public FunctionPass {
  Function *F = nullptr;
  unsigned NumTransformed = 0;

  SmallVector<Value *, 16> WorkList;
public:
  static char ID;

  StringRef getPassName() const override { return PassDescription; }

  TPCSelectorPreshaperLegacyPass() : FunctionPass(ID) {
    initializeTPCSelectorPreshaperLegacyPassPass(*PassRegistry::getPassRegistry());
  }

  bool runOnFunction(Function &F) override;
  Value *replace(Value *I, FixedVectorType *DType);
};
}

char TPCSelectorPreshaperLegacyPass::ID = 0;

INITIALIZE_PASS_BEGIN(TPCSelectorPreshaperLegacyPass, PassName, PassDescription, false, false)
INITIALIZE_PASS_END(TPCSelectorPreshaperLegacyPass, PassName, PassDescription, false, false)


FunctionPass *llvm::createTPCSelectorPreshaper() {
  return new TPCSelectorPreshaperLegacyPass();
}

// return true if shaffle is unsupported and must be transformed
// shuffle is concat of 2 vec ({1,2} {3,4})
// first is 1st half of result vector
// second is 2nd half vector
// {0,1,2,3} -> returns first- 3,  sec-1, < 0 if mask is undefined
// result is vector second part of second vector + 2half of 1st vector
static bool analyze_shuffle_mask(ArrayRef<int> mask , int *first,
                                 int *second) {
  int MaskNumElts = mask.size();
  if ((MaskNumElts & 0x7) != 0) {// size must have 4 parts
    return false;
  }
  int half_size = MaskNumElts / 2;
  int starts[4];
  int i;
  for (i = 0; i < 4; i++) {
    starts[i] = half_size*i;
  }
  ArrayRef<int> Indices = mask;
  // to find for first
  bool found = false;
  for (i = 0; i < 4; i++) {
    if (Indices[0] == starts[i]) {
      *first = i;
      found = true;
      break;
    }
  }
  if (!found) {
    if (Indices[0] >= 0) {
      return false;
    }
    else { // if half mask is undef
      for (i = 0; i < half_size; i++) {
        if (Indices[i] >= 0) {
          return false;
        }
      }
      // 1sthalfmask is undef
      *first = -1;
    }
  }
  if (*first >= 0) {
    // To check linearity of 1st half 
    for (i = 0; i < half_size; i++) {
      if (Indices[i] != i + *first*half_size) {
        return false;
      }
    }
  }
  found = false;
  // to find for second
  for (i = 0; i < 4; i++) {
    if (Indices[half_size] == starts[i]) {
      *second = i;
      found = true;
      break;
    }
  }
  if (!found) {
    return false;
  }
  // To check linearity of 2nd half 
  for (i = half_size; i < MaskNumElts; i++) {
    if (Indices[i] != i - half_size + *second*half_size) {
      return false;
    }
  }
  return true;
}


static bool shallExpandIntrinsic(Instruction &I) {
  if (isa<IntrinsicInst>(I))
    if (auto *Memset = dyn_cast<MemSetInst>(&I))
      if (!isa<Constant>(Memset->getLength()))
        return true;
  return false;
}


static void expandVectorMemSetAsLoop(MemSetInst *Memset) {
  const int VectorSize = 256;

  BasicBlock *OrigBB = Memset->getParent();
  Function *F = OrigBB->getParent();
  Value *CopyLen = Memset->getLength();
  Value *DstAddr = Memset->getRawDest();
  Value *SetValue = Memset->getValue();
  Type *TypeOfCopyLen = CopyLen->getType();

  // Create vector to fill with.
  IRBuilder<> PheHeaderBuilder(Memset);
  SetValue = PheHeaderBuilder.CreateVectorSplat(VectorSize, SetValue);
  // Length must be divided by 256.
  CopyLen = PheHeaderBuilder.CreateUDiv(CopyLen, ConstantInt::get(TypeOfCopyLen, VectorSize));

  // The code below is borrowed from createMemSetLoop in LowerMemIntrinsics.cpp

  BasicBlock *NewBB = OrigBB->splitBasicBlock(Memset, "split");
  BasicBlock *LoopBB = BasicBlock::Create(F->getContext(), "loadstoreloop", F, NewBB);

  IRBuilder<> Builder(OrigBB->getTerminator());

  // Cast pointer to the type of value getting stored
  unsigned dstAS = cast<PointerType>(DstAddr->getType())->getAddressSpace();
  DstAddr = Builder.CreateBitCast(DstAddr, PointerType::get(SetValue->getType(), dstAS));

  Builder.CreateCondBr(
    Builder.CreateICmpEQ(ConstantInt::get(TypeOfCopyLen, 0), CopyLen), NewBB,
    LoopBB);
  OrigBB->getTerminator()->eraseFromParent();

  IRBuilder<> LoopBuilder(LoopBB);
  PHINode *LoopIndex = LoopBuilder.CreatePHI(TypeOfCopyLen, 0);
  LoopIndex->addIncoming(ConstantInt::get(TypeOfCopyLen, 0), OrigBB);

  LoopBuilder.CreateStore(
    SetValue,
    LoopBuilder.CreateInBoundsGEP(SetValue->getType(), DstAddr, LoopIndex),
    Memset->isVolatile());

  Value *NewIndex =
    LoopBuilder.CreateAdd(LoopIndex, ConstantInt::get(TypeOfCopyLen, 1));
  LoopIndex->addIncoming(NewIndex, LoopBB);

  LoopBuilder.CreateCondBr(LoopBuilder.CreateICmpULT(NewIndex, CopyLen), LoopBB,
                           NewBB);
}


static void expandScalarMemSetAsLoop(MemSetInst *Memset) {
  const int ScalarSize = 4;

  BasicBlock *OrigBB = Memset->getParent();
  Function *F = OrigBB->getParent();
  Value *CopyLen = Memset->getLength();
  Value *DstAddr = Memset->getRawDest();
  Value *SetValue = Memset->getValue();
  Type *TypeOfCopyLen = CopyLen->getType();

  // Create word to fill with.
  IRBuilder<> PheHeaderBuilder(Memset);
  if (ConstantInt *C = dyn_cast<ConstantInt>(SetValue)) {
    unsigned ByteVal = C->getLimitedValue();
    ByteVal &= 0x00FF;
    ByteVal |= (ByteVal << 8);
    ByteVal |= (ByteVal << 16);
    SetValue = ConstantInt::get(Type::getInt32Ty(F->getContext()), ByteVal);
  } else {
    SetValue = PheHeaderBuilder.CreateAnd(SetValue, ConstantInt::get(SetValue->getType(), 0x00FF));
    Value *Byte1 = PheHeaderBuilder.CreateShl(SetValue, 8);
    SetValue = PheHeaderBuilder.CreateOr(SetValue, Byte1);
    Value *Bytes2_3 = PheHeaderBuilder.CreateShl(SetValue, 16);
    SetValue = PheHeaderBuilder.CreateOr(SetValue, Bytes2_3);
  }

  // Length must be divided by 4.
  CopyLen = PheHeaderBuilder.CreateUDiv(CopyLen, ConstantInt::get(TypeOfCopyLen, ScalarSize));

  // The code below is borrowed from createMemSetLoop in LowerMemIntrinsics.cpp

  BasicBlock *NewBB =
    OrigBB->splitBasicBlock(Memset, "split");
  BasicBlock *LoopBB
    = BasicBlock::Create(F->getContext(), "loadstoreloop", F, NewBB);

  IRBuilder<> Builder(OrigBB->getTerminator());

  // Cast pointer to the type of value getting stored
  unsigned dstAS = cast<PointerType>(DstAddr->getType())->getAddressSpace();
  DstAddr = Builder.CreateBitCast(DstAddr,
                                  PointerType::get(Type::getInt32Ty(F->getContext()), dstAS));

  Builder.CreateCondBr(
    Builder.CreateICmpEQ(ConstantInt::get(TypeOfCopyLen, 0), CopyLen), NewBB,
    LoopBB);
  OrigBB->getTerminator()->eraseFromParent();

  IRBuilder<> LoopBuilder(LoopBB);
  PHINode *LoopIndex = LoopBuilder.CreatePHI(TypeOfCopyLen, 0);
  LoopIndex->addIncoming(ConstantInt::get(TypeOfCopyLen, 0), OrigBB);

  LoopBuilder.CreateStore(
    SetValue,
    LoopBuilder.CreateInBoundsGEP(SetValue->getType(), DstAddr, LoopIndex),
    Memset->isVolatile());

  Value *NewIndex =
    LoopBuilder.CreateAdd(LoopIndex, ConstantInt::get(TypeOfCopyLen, 1));
  LoopIndex->addIncoming(NewIndex, LoopBB);

  LoopBuilder.CreateCondBr(LoopBuilder.CreateICmpULT(NewIndex, CopyLen), LoopBB,
                           NewBB);
}


static unsigned expandIntrinsics(SmallVectorImpl<Instruction *> &IntrinsicsToExpand) {
  unsigned Cnt = 0;
  for (Instruction *I : IntrinsicsToExpand) {
    if (auto *Memset = dyn_cast<MemSetInst>(I)) {
      if (Memset->getDestAddressSpace() == 2)
        expandVectorMemSetAsLoop(Memset);
      else
        expandScalarMemSetAsLoop(Memset);
      Memset->eraseFromParent();
      ++Cnt;
    }
  }
  return Cnt;
}

static bool user_is_zero_mask_shuffle(Instruction &I)
{
  if (I.getNumUses() == 1) {
    bool all_zero = true;
    auto *subuser = I.user_back();
    if (auto *shafi = dyn_cast<ShuffleVectorInst>(subuser)) {
      ArrayRef<int> Indices = shafi->getShuffleMask();
      //analyze if all mask is 0
      for (int i = 0, e = Indices.size(); i < e; i++) {
        if (Indices[i] != 0) {
          all_zero = false;
          break;
        }
      }
      return all_zero;
    } 
    else {
      return user_is_zero_mask_shuffle(*subuser);
    }
  }
  return false;
}

static Value* build_op(Instruction &I, Value*opnd1, Value*opnd2)
{
  IRBuilder<> Builder(&I);
  if (I.getOpcode() == Instruction::Add) {
    return Builder.CreateAdd(opnd1, opnd2);
  }
  if (I.getOpcode() == Instruction::FMul) {
    return Builder.CreateFMul(opnd1, opnd2);
  }
  if (I.getOpcode() == Instruction::Mul) {
    return Builder.CreateMul(opnd1, opnd2);
  }
  if (I.getOpcode() == Instruction::Shl) {
    return Builder.CreateShl(opnd1, opnd2);
  }
  if (I.getOpcode() == Instruction::LShr) {
    return Builder.CreateShl(opnd1, opnd2);
  }
  if (I.getOpcode() == Instruction::And) {
    return Builder.CreateShl(opnd1, opnd2);
  }

  return nullptr;
}

static bool MaskIs0(ArrayRef<int> Mask) {
  for (int i = 0, NumMaskElts = Mask.size(); i < NumMaskElts; ++i) {
    if (Mask[i] != 0) {
      return false;
    }
  }
  return true;
}

static Value *consider_shuffle(ShuffleVectorInst *shuffl) {
  Value *o01 = nullptr;
  if (!MaskIs0(shuffl->getShuffleMask())) {
    return nullptr;
  }
  Value *shufop = shuffl->getOperand(0);
  auto inse = dyn_cast<InsertElementInst>(shufop);
  if (inse) {
    o01 = inse->getOperand(1);
  } else if (auto nextshuff = dyn_cast<ShuffleVectorInst>(shufop)) {
    o01 = consider_shuffle(nextshuff);
  }
  return o01;
}

static void unite_shuffle(ShuffleVectorInst *shufinstr) {
  for (auto Usr : shufinstr->users()) {
    if (auto shaf_us = dyn_cast<ShuffleVectorInst>(Usr)) {
      ArrayRef<int> mask1 = shufinstr->getShuffleMask();
      ArrayRef<int> mask2 = shaf_us->getShuffleMask();
      Value *op1_1 = shufinstr->getOperand(1);
      Value *op1_2 = shaf_us->getOperand(1);
      if (mask1 == mask2 && op1_1 == op1_2) {
        if (shaf_us->getOperand(0) == shufinstr) {
          shaf_us->replaceAllUsesWith(shufinstr);
        }
      }
    }
  }
}

static void unvect_instr(Instruction &I, LLVMContext &Ctx)
{
  IRBuilder<> Builder(&I);
  auto op1 = I.getOperand(1);
  auto op0 = I.getOperand(0);
  Type* t1 = op1->getType();
  IntegerType *I32Type = Type::getInt32Ty(Ctx);

  // need to eliminate vector fadd
  auto *ins0 = dyn_cast<InsertElementInst>(op0);
  auto *ins1 = dyn_cast<InsertElementInst>(op1);
  Value* o01 = nullptr;
  Value* o02 = nullptr;
  Value* o11 = nullptr;
  Value* o12 = nullptr;
  uint64_t Val0, Val1;
  Value* sum1 = nullptr;
  bool all_zero_shuffle = user_is_zero_mask_shuffle(I);
  if (ins0) {
    o01 = ins0->getOperand(1);
    o02 = ins0->getOperand(2);
    if (!o01->getType()->isVectorTy() && dyn_cast<ConstantInt>(o02)) {
      Val0 = cast<ConstantInt>(o02)->getLimitedValue();
      if (Val0 != 0) {
        o01 = nullptr;
      }
    }
  }
  else if (auto shuffl = dyn_cast<ShuffleVectorInst>(op0)) { //consider shuffle
    o01 = consider_shuffle(shuffl);
  }
  else {
    Constant* cv = dyn_cast<Constant>(op0);
    FixedVectorType *cvt = cast<FixedVectorType>(op0->getType());
    if (cv&&cvt) {
      unsigned int veclen = cvt->getNumElements();
      // check if it is irregular vector {v,u,u.....}
      if (isa<ConstantVector>(cv) || isa<ConstantDataVector>(cv)) {
        Constant* first = cv->getAggregateElement(0U);
        bool elements_are_not_equal = false;
        if (!all_zero_shuffle) {
          for (unsigned i = 1; i < veclen; i++) {
            Constant* current = cv->getAggregateElement(i);
            if (current != first && !isa<UndefValue>(current)) { // undef -1 accepatble
              elements_are_not_equal = true;
              break;
            }
          }
        }
        if (!elements_are_not_equal) {
          o01 = first;
        }
      }
      else if (cv->isZeroValue()) {
        o01 = cv->getAggregateElement(0U);
      }
    }
    // compare with below for ins1
  }
  if (ins1) {
    o11 = ins1->getOperand(1);
    o12 = ins1->getOperand(2);
    if (!o11->getType()->isVectorTy() && dyn_cast<ConstantInt>(o12)) {
      Val1 = cast<ConstantInt>(o12)->getLimitedValue();
      if (Val1 != 0) {
        o11 = nullptr;
      }
      else if (o01) {
        sum1 = build_op(I, o01, o11);
      }
    }
  }
  else if (auto shuffl = dyn_cast<ShuffleVectorInst>(op1)) { //consider shuffle
      if (MaskIs0(shuffl->getShuffleMask())) {
      auto inse = dyn_cast<InsertElementInst>(shuffl->getOperand(0));
      if (inse && o01) {
        sum1 = build_op(I, o01, inse->getOperand(1));
      }
    }
  }
  else {
    Constant* cv = dyn_cast<Constant>(op1);
    FixedVectorType* cvt = cast<FixedVectorType>(op1->getType());
    if (cv&&cvt) {
      unsigned int veclen = cvt->getNumElements();
      // check if it is irregular vector {v,u,u.....}
      if (isa<ConstantVector>(cv) || isa<ConstantDataVector>(cv)) {
        Constant* first = cv->getAggregateElement(0U);
        bool elements_are_not_equal = false;
        if (!all_zero_shuffle) {
          for (unsigned i = 1; i < veclen; i++) {
            Constant* current = cv->getAggregateElement(i);
            if (current != first && !isa<UndefValue>(current)) {
              elements_are_not_equal = true;
              break;
            }
          }
        }
        if (!elements_are_not_equal && o01) {
          sum1 = build_op(I, o01, first);
        }
      } else if (cv->isZeroValue()&& o01) {
        sum1 = build_op(I, o01, cv->getAggregateElement(0U));
      }

    }
  }/// ins1== nullptr, not insert el 
  if (sum1) {
    Value* v2 = UndefValue::get(t1);
    Type* current_t = t1;
    int veclen = cast<FixedVectorType>(current_t)->getNumElements();
    Value* inse = Builder.CreateInsertElement(v2, sum1, ConstantInt::get(I32Type, 0));
    if (veclen == 64) current_t = FixedVectorType::get(I32Type, 64);
    else if (veclen == 128) current_t = FixedVectorType::get(I32Type, 128);
    else if (veclen == 256) current_t = FixedVectorType::get(I32Type, 256);
    else llvm_unreachable("bad vector length");
    Constant * Zeromask = ConstantInt::get(current_t, 0);
    Value* insshf = Builder.CreateShuffleVector(inse, v2, Zeromask);
    if (insshf) {
      I.replaceAllUsesWith(insshf);
      I.eraseFromParent();
      unite_shuffle(cast<ShuffleVectorInst> (insshf));
    }
  }
}

bool TPCSelectorPreshaperLegacyPass::runOnFunction(Function &Func) {
  if (skipFunction(Func))
    return false;
  F = &Func;
  LLVMContext &Ctx = Func.getContext();
  Module *Mod = Func.getParent();
  NumTransformed = 0;
  const unsigned AS = 1; // scalar address space
  SmallVector<Instruction *, 8> IntrinsicsToExpand;
  IntegerType *I32Type = Type::getInt32Ty(Ctx);
  IntegerType *I64Type = Type::getInt64Ty(Ctx);
  IntegerType *I16Type = Type::getInt16Ty(Ctx);
  PointerType *I32PtrType = Type::getInt32PtrTy(Ctx, AS /* scalar address space */);
  FixedVectorType* Int64Type = FixedVectorType::get(I32Type, 64);
  FixedVectorType* Int128Type = FixedVectorType::get(I32Type, 128);
  FixedVectorType *Short128Type = FixedVectorType::get(I16Type, 128);
  FixedVectorType *Int5Type = FixedVectorType::get(I32Type, 5);
 
  for (auto BBIt = Func.begin(), BBEnd = Func.end(); BBIt != BBEnd;) {
    BasicBlock &BB = *BBIt;
    ++BBIt;
    for (auto It = BB.begin(), E = BB.end(); It != E; ) {
      Instruction &I = *It;
      ++It;
      // Find intrinsics that we do not lower in selector.
      if (shallExpandIntrinsic(I)) {
        IntrinsicsToExpand.push_back(&I);
        continue;
      }

      // If IR contans a bitcast from i2048 to a valid TPC vector type, try
      // replacing i2048 with corresponding vector type. For example:
      //
      //     %8 = bitcast i2048 %.in to <64 x i32>
      //
      if (auto *BC = dyn_cast<BitCastInst>(&I)) {
        Type *DT = BC->getSrcTy();
        if (auto IT = dyn_cast<IntegerType>(DT)) {
          if (IT->getBitWidth() == 2048) {
            Value *NewV = replace(BC->getOperand(0), cast<FixedVectorType>(BC->getDestTy()));
            BC->replaceAllUsesWith(NewV);
          }
        }
      }

      // Stores of i64 are inserted by InstCombiner when it optimizes memset.
      // Split such stores into pairs of i32 stores.
      if (auto *Store = dyn_cast<StoreInst>(&I)) {
        auto type = Store->getValueOperand()->getType();
        if (FixedVectorType *vt = dyn_cast<FixedVectorType>(type)) {
          unsigned bsz = vt->getPrimitiveSizeInBits() / 8;
          unsigned alStore = Store->getAlignment();
          if (alStore < bsz && bsz >= 256) {
            Store->setAlignment(Align(256));
          }
        }

        if (Store->getValueOperand()->getType() == I64Type) {

          if (dyn_cast<ConstantInt>(Store->getValueOperand())) {
            uint64_t Val = cast<ConstantInt>(Store->getValueOperand())->getLimitedValue();
            IRBuilder<> Builder(Store);
            auto *V1 = ConstantInt::get(I32Type, Val & 0x0FFFFFFFFUL);
            auto Addr1 = Builder.CreateBitCast(Store->getPointerOperand(), I32PtrType);
            Builder.CreateAlignedStore(V1, Addr1, Align(std::min(Store->getAlignment(), 4U)), Store->isVolatile());
            auto Ndx = ConstantInt::get(I32Type, 1);
            auto Addr2 = Builder.CreateGEP(Addr1, Ndx);
            auto *V2 = ConstantInt::get(I32Type, Val >> 32);
            Builder.CreateAlignedStore(V2, Addr2, Align(std::min(Store->getAlignment(), 4U)), Store->isVolatile());
            Store->eraseFromParent();
          }
          else {
            auto Val = Store->getValueOperand();
            // We suppose that we got this inst from memcpy
            auto Load = cast<LoadInst>(Val);

            auto InitialAddr = Load->getPointerOperand();
            IRBuilder<> LDBuilder(Load);

            // Changing i64 load to two i32 loads
            auto LoadAddrHi = LDBuilder.CreateBitCast(InitialAddr, I32PtrType);
            auto LoadHi = LDBuilder.CreateAlignedLoad(LoadAddrHi,
              Align(std::min(Load->getAlignment(), 4U)),
              Load->isVolatile());
            auto Ndx = ConstantInt::get(I32Type, 1);
            auto LoadAddrLo = LDBuilder.CreateGEP(LoadAddrHi, Ndx);
            auto LoadLo = LDBuilder.CreateAlignedLoad(LoadAddrLo,
              Align(std::min(Load->getAlignment(), 4U)),
              Load->isVolatile());

            auto StoreAddr = Store->getPointerOperand();
            IRBuilder<> STBuilder(Store);

            // Changing i64 store to two i32 stores

            auto StoreAddrHi = STBuilder.CreateBitCast(StoreAddr, I32PtrType);
            STBuilder.CreateAlignedStore(LoadHi, StoreAddrHi,
              Align(std::min(Store->getAlignment(), 4U)), Store->isVolatile());
            auto StoreAddrLo = STBuilder.CreateGEP(StoreAddrHi, Ndx);
            STBuilder.CreateAlignedStore(LoadLo, StoreAddrLo,
              Align(std::min(Store->getAlignment(), 4U)), Store->isVolatile());
            for (auto Usr : Load->users()) {
              if (auto *BitCast = dyn_cast<BitCastInst>(Usr)) {
                Type *DT = BitCast->getSrcTy();
                Type *TT = BitCast->getDestTy();
                FixedVectorType *vt = cast<FixedVectorType>(TT);
                if (DT == I64Type && vt->getNumElements() == 2 &&
                    vt->getElementType() == I32Type) {
                  Value* UndefVal = UndefValue::get(vt);
                  Constant *C0 = ConstantInt::get(I32Type, 0);
                  Constant *C1 = ConstantInt::get(I32Type, 1);
                  Value *V2 =
                      STBuilder.CreateInsertElement(UndefVal, LoadHi, C0);
                  V2 = STBuilder.CreateInsertElement(V2, LoadLo, C1);
                  BitCast->replaceAllUsesWith(V2);
                }
              }
              else {
                auto *mayStore = dyn_cast<StoreInst>(Usr);
                if (!mayStore)
                    llvm_unreachable("unexpectable use of i64 type");
              }
            }
            Store->eraseFromParent();
            // Do not erase Load,BitCase here, will be removed after DCE 
          }
        }
      }
      // llvm Early CSE separate InsertElementInst and ShuffleVectorInst
      // moving Shuffle in other Basic Black
      // after all DAG cannot process it and caused to wrong BUILD_VECTOR {t, undef, undef....
      // this code fix it returning shuffle from other BB into its native place
      // just after InsertElementInst
      else if (auto *Load = dyn_cast<LoadInst>(&I)) {
        auto type = Load->getType();
        if (FixedVectorType *vt = dyn_cast<FixedVectorType>(type)) {
          unsigned bsz = vt->getPrimitiveSizeInBits()/8;
          unsigned alLo = Load->getAlignment();
          if (alLo < bsz && bsz >= 256) {
            Load->setAlignment(Align(256));
          }
        }
      }
      else if (auto *inse = dyn_cast<InsertElementInst>(&I)) {
        IRBuilder<> Builder(inse);
        auto opnd1 = I.getOperand(0);
        auto type1 = opnd1->getType();
        if (FixedVectorType* vt = cast<FixedVectorType>(type1)) {
          if (vt->getNumElements() >= 64) {
            for (auto Usr : inse->users()) {
              if (auto shuffl = dyn_cast<ShuffleVectorInst>(Usr)) {
                auto bbsh = shuffl->getParent();
                if (bbsh != &BB) {
                  shuffl->moveAfter(inse);
                }
              }
              else {  // user is not shuffle
                int nop = Usr->getNumOperands();
                if (nop > 1) {
                  // need to seek irregular constant vector
                  for (int iop = 0; iop < nop; iop++) {
                    Value* opnd = Usr->getOperand(iop);
                    Constant* cv = dyn_cast<Constant>(opnd);
                    FixedVectorType *cvt = cast<FixedVectorType>(opnd->getType());
                    if (cv&&cvt) {
                      unsigned int veclen = cvt->getNumElements();
                      // check if it is irregular vector {v,u,u.....}
                      if (isa<ConstantVector>(cv) || isa<ConstantDataVector>(cv)) {
                        Constant* first = cv->getAggregateElement(0U);
                        bool elements_are_not_equal = false;
                        for (unsigned i = 1; i < veclen; i++) {
                          if (cv->getAggregateElement(i) != first) {
                            elements_are_not_equal = true;
                            break;
                          }
                        }
                        if (elements_are_not_equal) {
                          // need replace with uniform vector (first, first,....)
                          SmallVector<Constant*, 64> Result;
                          for (unsigned int i = 0; i < veclen; i++) {
                            Result.push_back(first);
                          }
                          Value* newK = ConstantVector::get(Result);
                          Usr->setOperand(iop, newK);
                          // now need remove undef in shaffle mask
                          for (auto UsrUsr : Usr->users()) {
                            if (auto shuffl = dyn_cast<ShuffleVectorInst>(UsrUsr)) {
                              ArrayRef<int> mask = shuffl->getShuffleMask();
                              int msksize =  mask.size();
                             
                              bool undef=false,  other = false;
                              for (int i = 0; i < msksize; i++) {
                                int elm = shuffl->getMaskValue(i);
                                if (elm < 0) undef = true;
                                else other = true;
                              }
                              if (!other && undef) {
                                Constant * newzeromask = nullptr;
                                if (msksize == 64) {
                                  newzeromask = ConstantInt::get(Int64Type, 0);
                                }
                                else if (msksize == 128){
                                  newzeromask = ConstantInt::get(Int128Type, 0);
                                }
                                if (newzeromask) {
                                  shuffl->setOperand(2, newzeromask);
                                }
                              }
                            }
                          }
                        }
                      }
                    }
                  }
                }
              }
            }
          }
        }
      }
      else if (auto *shafi = dyn_cast<ShuffleVectorInst>(&I)) {
        ArrayRef<int> mask = shafi->getShuffleMask();
        int first, second;
        unsigned half1, half2;
        auto opnd0 = shafi->getOperand(0);
        FixedVectorType *T0 = cast<FixedVectorType>(opnd0->getType());
        auto Tel = T0->getElementType();
        unsigned int sosize = T0->getNumElements();
        unsigned msksize = mask.size();
        // special case extract from int5 and splat
        IRBuilder<> Builder(shafi);
        if (msksize == 128 && sosize == 10 && Tel == I16Type) {
          // mask be identical
          bool mask_identical = true;
          for (unsigned int i = 1; i < msksize; i++) {
            if (mask[i] != mask[i-1]) {
              mask_identical = false;
              break;
            }
          }
          if (mask_identical) {
            unsigned mask_val = mask[0];
            auto opi5 = Builder.CreateBitCast(opnd0, Int5Type);
            unsigned indint5 = mask_val / 2;
            assert(mask_val % 2 == 0); // user expects that coordinates really short
            auto exc = Builder.CreateExtractElement(opi5, ConstantInt::get(I32Type, indint5));
            auto ii16 = Builder.CreateTrunc(exc, I16Type);
            Constant *Zeromask = ConstantInt::get(Int128Type, 0);
            Value *v2 = UndefValue::get(Short128Type);
            Value *v1 = Builder.CreateInsertElement(v2, ii16, (uint64_t)0);
            Value *insshf = Builder.CreateShuffleVector(v1, v2, Zeromask);
            shafi->replaceAllUsesWith(insshf);
            shafi->eraseFromParent();
          } else {
            llvm_unreachable("Unhandled shuffle mask for short irf");
          }
        }
        else if (msksize >= sosize && // no need to analyze extract
          analyze_shuffle_mask(mask, &first, &second)) {
          unsigned concat1, concat2 = second >> 1;
          if (first < 0) {// 1st half is undef
            concat1 = concat2;
            half2 = second & 1;
            half1 = half2 ^ 1;
          }
          else {
            concat1 = first >> 1;
            if (concat1 == concat2) {
              // no need transorm instruction
              continue;
            }
            half1 = first & 1;
            half2 = second & 1;
          }
          auto opnd1 = shafi->getOperand(concat1);
          auto opnd2 = shafi->getOperand(concat2);
          // need to extract half1 from opnd1
          unsigned MaskNumElts = mask.size();
          unsigned half_size = MaskNumElts / 2;
          Value * shaf1, *shaf2, *shaf_last;
          if (!half1) {
            auto loc_mask = createSequentialMask(0, half_size, 0);
            Value* v1 = opnd1;
            VectorType *VecTy = dyn_cast<VectorType>(v1->getType());
            Value* v2 = UndefValue::get(VecTy);
            shaf1 = Builder.CreateShuffleVector(v1, v2, loc_mask);
          }
          else {
            auto loc_mask = createSequentialMask(half_size, half_size, 0);
            Value* v1 = opnd2;
            VectorType *VecTy = dyn_cast<VectorType>(v1->getType());
            Value* v2 = UndefValue::get(VecTy);
            shaf1 = (Builder.CreateShuffleVector(v1, v2, loc_mask));
          }
          if (!half2) {
            auto loc_mask = createSequentialMask(0, half_size, 0);
            Value* v1 = opnd2;
            VectorType *VecTy = dyn_cast<VectorType>(v1->getType());
            Value* v2 = UndefValue::get(VecTy);
            shaf2 = Builder.CreateShuffleVector(v1, v2, loc_mask);
          }
          else {
            auto loc_mask = createSequentialMask(half_size, half_size, 0);
            Value* v1 = opnd2;
            VectorType *VecTy = dyn_cast<VectorType>(v1->getType());
            Value* v2 = UndefValue::get(VecTy);
            shaf2 = Builder.CreateShuffleVector(v1, v2, loc_mask);
          }
          if (shaf1 && shaf2) {
            auto loc_mask = createSequentialMask(0, MaskNumElts, 0);
            shaf_last = Builder.CreateShuffleVector(shaf1, shaf2, loc_mask);
            if (shaf_last) {
              shafi->replaceAllUsesWith(shaf_last);
              shafi->eraseFromParent();
            }
          }
        }
      } // end of shuffle
      else if (auto *sitofp = dyn_cast<SIToFPInst>(&I)) {
        Type* ty = sitofp->getType();
        auto opnd1 = I.getOperand(0);
        auto type_from = opnd1->getType();
        if (type_from->isIntegerTy(32) && ty->isBFloatTy()) {
          IRBuilder<> Builder(sitofp);
          Intrinsic::ID intrinId = Intrinsic::tpc_convert;
          auto ExtF = Intrinsic::getDeclaration(Mod, intrinId, { ty, type_from, IntegerType::get(F->getContext(), 1) });
          auto NewIns = Builder.CreateCall(ExtF,
          { opnd1, //src1
           ConstantInt::get(Type::getInt8Ty(Ctx), 2),//op_type = INT32
           ConstantInt::get(Type::getInt32Ty(Ctx), 1 << 12 | 1 << 8), //target_type = BF16 | lane_sel = 1
           UndefValue::get(ty),
           ConstantInt::get(Type::getInt1Ty(BB.getContext()), 1),
           ConstantInt::get(Type::getInt1Ty(BB.getContext()), 0) });
          sitofp->replaceAllUsesWith(NewIns);
          sitofp->eraseFromParent();
        }
      }
      else if (I.getOpcode() == Instruction::FAdd) {
        IRBuilder<> Builder(&I);
        auto op1 = I.getOperand(1);
        auto op0 = I.getOperand(0);
        Type* t1 = op1->getType();
        Type* t0 = op1->getType();
        if (t0 != t1) continue;
        if (t1->isVectorTy()) {
          Type *Elt = cast<FixedVectorType>(t1)->getElementType();
          unsigned eltsize = Elt->getScalarSizeInBits();
          unsigned vecsize = cast<FixedVectorType>(t1)->getNumElements();
          if (eltsize == 32 && vecsize == 128) { // not supported in BE, need to lower
            auto  mask1 = createSequentialMask(0, 64, 0);
            auto  mask2 = createSequentialMask(64, 64, 0);
            Value* v2 = UndefValue::get(t1);
            Value* novo11 = Builder.CreateShuffleVector(op0, v2, mask1);
            Value* novo21 = Builder.CreateShuffleVector(op0, v2, mask2);
            Value* novo12 = Builder.CreateShuffleVector(op1, v2, mask1);
            Value* novo22 = Builder.CreateShuffleVector(op1, v2, mask2);
            Value* sum1 = Builder.CreateFAdd(novo11, novo12);
            Value* sum2 = Builder.CreateFAdd(novo21, novo22);
            auto maskconcat = createSequentialMask(0, 128, 0);
            Value* concatv = Builder.CreateShuffleVector(sum1, sum2, maskconcat);
            if (concatv) {
              I.replaceAllUsesWith(concatv);
              I.eraseFromParent();
            }
          }
          else if (eltsize == 32 && vecsize == 64) {
            unvect_instr(I, Ctx);
          }
          else if (eltsize == 16 && vecsize == 128) {
            unvect_instr(I, Ctx);
          }
        }
      }
      else if (I.getOpcode() == Instruction::Sub) {
        IRBuilder<> Builder(&I);
        auto opnd0 = I.getOperand(0);
        auto opnd1 = I.getOperand(1);
        Type* t0 = opnd0->getType();
        Type* t1 = opnd1->getType();
        if (t0->isVectorTy() || t1->isVectorTy()) {
          if (user_is_zero_mask_shuffle(I)) {
            Constant *CI = ConstantInt::get(Type::getInt32Ty(Ctx), 0);
            Value *e0 = opnd0, *e1 = opnd1;
            VectorType *VecType = nullptr;
            Value *UndefVal = nullptr;
            if (t0->isVectorTy()) {
              auto *insel = dyn_cast<InsertElementInst>(opnd0);
              if (insel) {
                e0 = insel->getOperand(1);
              } else {
                e0 = Builder.CreateExtractElement(opnd0, CI);
              }
              VecType = cast<VectorType>(t0);
            }
            if (t1->isVectorTy()) {
              auto *insel = dyn_cast<InsertElementInst>(opnd1);
              if (insel) {
                e1 = insel->getOperand(1);
              } else {
                e1 = Builder.CreateExtractElement(opnd1, CI);
              }
              VecType = cast<VectorType>(t1);
            }
            Value *scalar_sub = Builder.CreateNSWSub(e0, e1);
            UndefVal = UndefValue::get(VecType);
            Value *new_vec_sub =
                Builder.CreateInsertElement(UndefVal, scalar_sub, CI);
            if (new_vec_sub) {
              I.replaceAllUsesWith(new_vec_sub);
              I.eraseFromParent();
              continue;
            }
          }
        }
      }
      else if (I.getOpcode() == Instruction::FMul) {
        IRBuilder<> Builder(&I);
        auto opnd0 = I.getOperand(0);
        auto opnd1 = I.getOperand(1);
        Type* t0 = opnd0->getType();
        Type* t1 = opnd1->getType();
        if (t0 == t1 && t1->isVectorTy()){ 
          FixedVectorType *vt = cast<FixedVectorType>(t1);       
          int eltsize = vt->getElementType()->getScalarSizeInBits();
          int vecsize = vt->getNumElements();
          if (eltsize == 32 && vecsize == 128) {
            auto  mask1 = createSequentialMask(0, 64, 0);
            auto mask2 = createSequentialMask(64, 64, 0);
            Value* v2 = UndefValue::get(t1);
            Value* novo11 = Builder.CreateShuffleVector(opnd0, v2, mask1);
            Value* novo21 = Builder.CreateShuffleVector(opnd0, v2, mask2);
            Value* novo12 = Builder.CreateShuffleVector(opnd1, v2, mask1);
            Value* novo22 = Builder.CreateShuffleVector(opnd1, v2, mask2);
            Value* mul1 = Builder.CreateFMul(novo11, novo12);
            Value* mul2 = Builder.CreateFMul(novo21, novo22);
            auto maskconcat = createSequentialMask(0, 128, 0);
            Value* concatv = Builder.CreateShuffleVector(mul1, mul2, maskconcat);
            if (concatv) {
              I.replaceAllUsesWith(concatv);
              I.eraseFromParent();
            }
          }
          else if (eltsize == 32 && vecsize == 64) {
            unvect_instr(I, Ctx);
          }
          else if (eltsize == 16 && vecsize == 128) {
            unvect_instr(I, Ctx);
          }
        }
        else if (t0->isVectorTy() || t1->isVectorTy()) {
          if (I.getNumUses() == 1) {
            auto *fmuser = I.user_back();
            if (auto *shafi = dyn_cast<ShuffleVectorInst>(fmuser)) {
              ArrayRef<int> mask = shafi->getShuffleMask();
              int msksize = mask.size();
              ArrayRef<int> Indices = mask;
              //analyze if all mask is 0
              bool all_zero = true;
              for (int i = 0; i < msksize; i++) {
                if (Indices[i] != 0) {
                  all_zero = false;
                  break;
                }
              }
              if (all_zero) {
                Constant *CI = ConstantInt::get(Type::getInt32Ty(Ctx), 0);
                Value *e0=opnd0, *e1=opnd1;
                VectorType*  VecType = nullptr;
                Value *UndefVal = nullptr;
                if (t0->isVectorTy()) {
                  auto *insel = dyn_cast<InsertElementInst>(opnd0);
                  if (insel) {
                    e0 = insel->getOperand(1);
                  }
                  else {
                    e0 = Builder.CreateExtractElement(opnd0, CI);
                    VecType = cast<VectorType>(t0);
                  }
                }
                if (t1->isVectorTy()) {
                  e1 = Builder.CreateExtractElement(opnd1, CI);
                  VecType = cast<VectorType>(t1);
                }
                Value* scalar_mul = Builder.CreateFMul(e0, e1);
                UndefVal = UndefValue::get(VecType);
                Value* new_vec_mul = Builder.CreateInsertElement(UndefVal, scalar_mul, CI);
                if (new_vec_mul) {
                  I.replaceAllUsesWith(new_vec_mul);
                  I.eraseFromParent();
                }
              }
            }
          }
        }
      }
      else if (I.getOpcode() == Instruction::Mul
        || I.getOpcode() == Instruction::Shl 
        || I.getOpcode() == Instruction::LShr 
        || I.getOpcode() == Instruction::And
        || I.getOpcode() == Instruction::Add
        ) {
        IRBuilder<> Builder(&I);
        auto opnd0 = I.getOperand(0);
        auto opnd1 = I.getOperand(1);
        Type* t0 = opnd0->getType();
        Type* t1 = opnd1->getType();
        if (t0 == t1 && t1->isVectorTy()) {
          int eltsize = cast<FixedVectorType>(t1)->getElementType()->getScalarSizeInBits();
          int vecsize = cast<FixedVectorType>(t1)->getNumElements();
          if (eltsize == 32 && vecsize == 64) {
            unvect_instr(I, Ctx);
          }
          else if (eltsize == 16 && vecsize == 128) {
            unvect_instr(I, Ctx);
          } 
          else if (eltsize == 32 && vecsize == 128) { // not supported in BE, need to lower
            auto mask1 = createSequentialMask(0, 64, 0);
            auto mask2 = createSequentialMask(64, 64, 0);
            Value *v2 = UndefValue::get(t1);
            Value *novo11 = Builder.CreateShuffleVector(opnd0, v2, mask1);
            Value *novo21 = Builder.CreateShuffleVector(opnd0, v2, mask2);
            Value *novo12 = Builder.CreateShuffleVector(opnd1, v2, mask1);
            Value *novo22 = Builder.CreateShuffleVector(opnd1, v2, mask2);
            Value *sum1, *sum2;
            if (I.getOpcode() == Instruction::FSub) {
              sum1 = Builder.CreateFSub(novo11, novo12);
              sum2 = Builder.CreateFSub(novo21, novo22);
            } else {
              llvm_unreachable((I.dump(),"Has not happened yet, do similar"));
            }
            auto maskconcat = createSequentialMask(0, 128, 0);
            Value *concatv =
                Builder.CreateShuffleVector(sum1, sum2, maskconcat);
            if (concatv) {
              I.replaceAllUsesWith(concatv);
              I.eraseFromParent();
            }
          } else if (eltsize == 32 &&
                     vecsize == 256) { // not supported in BE, need to lower
            auto mask1 = createSequentialMask(0, 64, 0);
            auto mask2 = createSequentialMask(64, 64, 0);
            auto mask3 = createSequentialMask(128, 64, 0);
            auto mask4 = createSequentialMask(192, 64, 0);
            Value *v2 = UndefValue::get(t1);
            Value *novo11 = Builder.CreateShuffleVector(opnd0, v2, mask1);
            Value *novo21 = Builder.CreateShuffleVector(opnd0, v2, mask2);
            Value *novo31 = Builder.CreateShuffleVector(opnd0, v2, mask3);
            Value *novo41 = Builder.CreateShuffleVector(opnd0, v2, mask4);

            Value *novo12 = Builder.CreateShuffleVector(opnd1, v2, mask1);
            Value *novo22 = Builder.CreateShuffleVector(opnd1, v2, mask2);
            Value *novo32 = Builder.CreateShuffleVector(opnd1, v2, mask3);
            Value *novo42 = Builder.CreateShuffleVector(opnd1, v2, mask4);
            Value *sum1, *sum2,*sum3,*sum4;
            if (I.getOpcode() == Instruction::FSub) {
              sum1 = Builder.CreateFSub(novo11, novo12);
              sum2 = Builder.CreateFSub(novo21, novo22);
              sum3 = Builder.CreateFSub(novo31, novo32);
              sum4 = Builder.CreateFSub(novo41, novo42);
            } else {
              llvm_unreachable((I.dump(), "Has not happened yet, do similar"));
            }
            auto maskconcat = createSequentialMask(0, 128, 0);
            Value *concatv12 =
                Builder.CreateShuffleVector(sum1, sum2, maskconcat);
            Value *concatv34 =
                Builder.CreateShuffleVector(sum3, sum4, maskconcat);
            auto maskconcat256 = createSequentialMask(0, 256, 0);
            Value *concatv =
                Builder.CreateShuffleVector(concatv12, concatv34, maskconcat256);
            if (concatv) {
              I.replaceAllUsesWith(concatv);
              I.eraseFromParent();
            }
          }
        }
      }
      else if (const IntrinsicInst* intrins = dyn_cast<IntrinsicInst>(&I)) {
        IRBuilder<> Builder(&I);
        Intrinsic::ID inid = intrins->getIntrinsicID();
 
        if (inid == Intrinsic::tpc_nearbyint) {
          if (!EnableNearbyintWorkaround) {
            continue;
          }

          AttributeList Fa = Func.getAttributes();
          if (Fa.hasFnAttribute("target-cpu")) {
            auto as= Fa.begin();
            Attribute  tac = as->getAttribute("target-cpu");
            auto savl = tac.getValueAsString();
            if (savl != "goya") {
              continue;
            }
          }
          Value* NearVal= I.getOperand(0);
          Type* TNear = NearVal->getType();
          Value* switch_opnd = I.getOperand(2);
          Value* income = I.getOperand(3);
          Value* predic_opnd = I.getOperand(4);
          Type* Tpredic = predic_opnd->getType();
          Value* polarity = I.getOperand(5);
          ConstantInt* ci = cast<ConstantInt>(switch_opnd);
          int swval = ci->getLimitedValue() & 0xf;
          if (swval != 0 && swval != 5) {
            continue;
          }
          ConstantInt* cp = cast<ConstantInt>(polarity);
          int poval = cp->getLimitedValue() & 0x1;
          Value* ExtrExp;
          Type* TExp; // Type for exp , must be int of same size TNear
          int tsize = TNear->getPrimitiveSizeInBits();
          if (tsize == 32) {
            TExp = Type::getInt32Ty(Ctx);
          }
          else if (tsize == 16) {
            TExp = Type::getInt32Ty(Ctx);
          }
          else if (TNear == FixedVectorType::get(Type::getFloatTy(Ctx), 64)) {
            TExp = FixedVectorType::get(Type::getInt32Ty(Ctx), 64);
          }
          else if (TNear == FixedVectorType::get(Type::getBFloatTy(Ctx), 128)) {
            TExp = FixedVectorType::get(Type::getInt16Ty(Ctx), 128);
          }
          else if (TNear == FixedVectorType::get(Type::getHalfTy(Ctx), 128)) {
            TExp = FixedVectorType::get(Type::getInt16Ty(Ctx), 128);
          }
          else {
            llvm_unreachable("bad type");
          }
          Function *ExtF = Intrinsic::getDeclaration(F->getParent()
            , Intrinsic::tpc_extract_exp,
            {TExp,TNear,Type::getInt1Ty(Ctx) });
          ExtrExp = Builder.CreateCall(ExtF,
          { NearVal,
            ConstantInt::get(Type::getInt8Ty(Ctx), 0),
            ConstantInt::get(Type::getInt32Ty(Ctx), 0),
            UndefValue::get(TExp),
            ConstantInt::get(Type::getInt1Ty(Ctx), 1),
            ConstantInt::get(Type::getInt1Ty(Ctx), 0)
          });

          Constant *Neg32 = ConstantInt::get(TExp, -32);
          Value* cmp_32;
          Value* NewPred;
          /// round mode extraction
          ExtF = Intrinsic::getDeclaration(F->getParent(), Intrinsic::tpc_ld_l, Type::getInt32Ty(Ctx));
          Value* roundmode = Builder.CreateCall(ExtF,
          { ConstantInt::get(Type::getInt32Ty(Ctx), 2044),
            ConstantInt::get(Type::getInt32Ty(Ctx), 1),
            UndefValue::get(Type::getInt32Ty(Ctx)),
            ConstantInt::get(Type::getInt1Ty(Ctx), 1),
            ConstantInt::get(Type::getInt1Ty(Ctx), 0)
          });
          Value* cmp0 = Builder.CreateICmpNE(roundmode, ConstantInt::get(Type::getInt32Ty(Ctx), 0));

          if (TNear->isVectorTy()) {
            ExtF = Intrinsic::getDeclaration(F->getParent()
              , Intrinsic::tpc_cmp_neq,
              { FixedVectorType::get(Type::getInt1Ty(Ctx), 256),TExp,TExp,Type::getInt1Ty(Ctx) });
            cmp_32 = Builder.CreateCall(ExtF,
            { ExtrExp,Neg32,
              ConstantInt::get(Type::getInt8Ty(Ctx), 2),
              ConstantInt::get(Type::getInt32Ty(Ctx), 0),
              UndefValue::get(FixedVectorType::get(Type::getInt1Ty(Ctx), 256)),
              ConstantInt::get(Type::getInt1Ty(Ctx), 1),
              ConstantInt::get(Type::getInt1Ty(Ctx), 0)
            });
            NewPred = cmp_32;
            if (swval == 5) {
              // need to broadcast cmp0
              cmp0 = Builder.CreateVectorSplat(256, cmp0);
              NewPred = Builder.CreateOr(cmp0, cmp_32);
            }
            if (poval == 1) {
              predic_opnd = Builder.CreateNot(predic_opnd);
              I.setOperand(5, ConstantInt::get(Type::getInt1Ty(Ctx), 0));
            }
            if (!Tpredic->isVectorTy()) {
              predic_opnd = Builder.CreateVectorSplat(256, predic_opnd);
            }

            NewPred = Builder.CreateAnd(predic_opnd, NewPred);
            if (!isa<UndefValue>(income)) {
              auto cincome = dyn_cast<Constant>(income);
              if (cincome) {
                if (!cincome->isZeroValue()) {
                  // incompatible with predicate
                  if (!dyn_cast<ConstantInt>(predic_opnd)) {
                    llvm_unreachable("income with rt predicate");
                  }
                }
              }
            }
            // neeed to set zero into income
            I.setOperand(3, ConstantFP::get(TNear, 0.0));
            I.setOperand(4, NewPred);
            // need to reconfigure call, as pred become vector
            ExtF = Intrinsic::getDeclaration(F->getParent(), Intrinsic::tpc_nearbyint,
            { TNear,TNear,predic_opnd->getType() }
            );
            Value* newnear = Builder.CreateCall(ExtF,
            { I.getOperand(0),
              I.getOperand(1),
              I.getOperand(2),
              I.getOperand(3),
              I.getOperand(4),
              I.getOperand(5),
            });

            I.replaceAllUsesWith(newnear);
            I.eraseFromParent();
          }
          else {
            NewPred = Builder.CreateICmpNE(ExtrExp, Neg32);
            if (swval == 5) {
              NewPred = Builder.CreateOr(cmp0, NewPred);
            }
            if (poval == 1) {
              predic_opnd = Builder.CreateNot(predic_opnd);
              I.setOperand(5, ConstantInt::get(Type::getInt1Ty(Ctx), 0));
            }
            NewPred = Builder.CreateAnd(predic_opnd, NewPred);

            if (!isa<UndefValue>(income)) {
              auto cincome = dyn_cast<Constant>(income);
              if (cincome) {
                if (!cincome->isZeroValue()) {
                  // incompatible with predicate
                  if (!dyn_cast<ConstantInt>(predic_opnd)) {
                    llvm_unreachable("income with rt predicate");
                  }
                }
              }
            }
            // neeed to set zero into income
            I.setOperand(3, ConstantFP::get(TNear, 0.0));
            I.setOperand(4, NewPred);
          }
        }
        else if (inid == Intrinsic::dbg_value) {
          unsigned Numop = intrins->getNumOperands();
          if (Numop >= 3) {
            auto opnd2 = intrins->getOperand(2);
            MetadataAsValue *md2 = dyn_cast<MetadataAsValue>(opnd2);
            Metadata *MD = md2->getMetadata();
            if (const DIExpression *Expr = dyn_cast<DIExpression>(MD)) {
              
              if (Expr->getNumElements() > 0 && Expr->getElement(0) == dwarf::DW_OP_LLVM_fragment) {
                assert(Expr->getNumElements()==3);
                unsigned left = Expr->getElement(1);
                unsigned bsz = Expr->getElement(2);
                auto opnd0 = intrins->getOperand(0);
                Type *t0 = opnd0->getType();
                unsigned szb0 = t0->getScalarSizeInBits();
                if (left + bsz >= szb0) { //incorrect instr
                  I.eraseFromParent();
                }
              }
            }
          }
        }
      }
    }
  }
  NumTransformed += expandIntrinsics(IntrinsicsToExpand);
  return NumTransformed > 0;
}

Value *TPCSelectorPreshaperLegacyPass::replace(Value *V, FixedVectorType *DType) {
  if (V->getType() == DType)
    return V;

  if (auto *BC = dyn_cast<BitCastInst>(V)) {
    Value *Src = BC->getOperand(0);
    if (Src->getType() == DType)
      return Src;
    ++NumTransformed;
    IRBuilder<> Builder(BC);
    return Builder.CreateBitCast(Src, DType);
  }

  if (auto *PHI = dyn_cast<PHINode>(V)) {
    IRBuilder<> Builder(PHI);
    unsigned NIncome = PHI->getNumOperands();
    ++NumTransformed;
    PHINode *NewPhi = Builder.CreatePHI(DType, NIncome);
    for (unsigned I = 0; I != NIncome; ++I) {
      Value *Op = PHI->getIncomingValue(I);
      Value *NewOp = replace(Op, DType);
      NewPhi->addIncoming(NewOp, PHI->getIncomingBlock(I));
    }
    return NewPhi;
  }

  if (auto *EEI = dyn_cast<ExtractElementInst>(V)) {
    Value *Src = EEI->getOperand(0);
    ConstantInt *Ndx = cast<ConstantInt>(EEI->getOperand(1));
    auto SVT = cast<FixedVectorType>(Src->getType());
    unsigned Factor = SVT->getPrimitiveSizeInBits().getFixedSize() / DType->getPrimitiveSizeInBits().getFixedSize();
    unsigned NewVSize = Factor * DType->getNumElements();
    auto *NewDVT = FixedVectorType::get(cast<VectorType>(DType)->getElementType(), NewVSize);
    Value *NewSrc = replace(Src, NewDVT);
    IRBuilder<> Builder(EEI);
    SmallVector<int, 64> Mask;
    unsigned DVSize = cast<FixedVectorType>(DType)->getNumElements();
    Mask.reserve(DVSize);
    unsigned Offset = Ndx->getLimitedValue() * DVSize;
    for (unsigned I = 0; I < DVSize; ++I, ++Offset)
      Mask.push_back(Offset);
    return Builder.CreateShuffleVector(NewSrc, UndefValue::get(NewDVT), Mask);
  }

  llvm_unreachable("Unhandled value");
}

