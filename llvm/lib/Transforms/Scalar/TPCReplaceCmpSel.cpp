//===-TPCOptUtils.cpp ----------------------------------------------------===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//
//
//===----------------------------------------------------------------------===//
#include "llvm/ADT/DenseMap.h"
#include "llvm/Analysis/VectorUtils.h"
#include "llvm/IR/Function.h"
#include "llvm/IR/IRBuilder.h"
#include "llvm/IR/IntrinsicsTPC.h"
#include "llvm/IR/LLVMContext.h"
#include "llvm/IR/Module.h"
#include "llvm/InitializePasses.h"
#include "llvm/Pass.h"
#include "llvm/Support/Debug.h"
#include "llvm/Transforms/Scalar.h"
#include "llvm/Transforms/Utils.h"
#include "llvm/Transforms/Utils/Local.h"
#include <cassert>
#include <vector>

using namespace llvm;

static const char PassDescription[] =
    "TPC replace compare and select with select intrinsic.";
static const char PassName[] = "tpc-replace-cmp-sel";

#define DEBUG_TYPE "tpc-replace-cmp-sel"

#define INVALID_ID 0xFFFFFFFF

namespace {
class TPCReplaceCmpSelPass : public FunctionPass {
public:
  static char ID;
  StringRef getPassName() const override { return PassDescription; }
  TPCReplaceCmpSelPass() : FunctionPass(ID) {
    initializeTPCReplaceCmpSelPassPass(*PassRegistry::getPassRegistry());
    IntrinsicMap[CmpInst::Predicate::FCMP_OEQ] = Intrinsic::tpc_sel_eq;
    IntrinsicMap[CmpInst::Predicate::FCMP_OGT] = Intrinsic::tpc_sel_grt;
    IntrinsicMap[CmpInst::Predicate::FCMP_OGE] = Intrinsic::tpc_sel_geq;
    IntrinsicMap[CmpInst::Predicate::FCMP_OLT] = Intrinsic::tpc_sel_less;
    IntrinsicMap[CmpInst::Predicate::FCMP_OLE] = Intrinsic::tpc_sel_leq;
    IntrinsicMap[CmpInst::Predicate::FCMP_ONE] = Intrinsic::tpc_sel_neq;
    IntrinsicMap[CmpInst::Predicate::FCMP_UEQ] = Intrinsic::tpc_sel_eq;
    IntrinsicMap[CmpInst::Predicate::FCMP_UGT] = Intrinsic::tpc_sel_grt;
    IntrinsicMap[CmpInst::Predicate::FCMP_UGE] = Intrinsic::tpc_sel_geq;
    IntrinsicMap[CmpInst::Predicate::FCMP_ULT] = Intrinsic::tpc_sel_less;
    IntrinsicMap[CmpInst::Predicate::FCMP_ULE] = Intrinsic::tpc_sel_leq;
    IntrinsicMap[CmpInst::Predicate::FCMP_UNE] = Intrinsic::tpc_sel_neq;

    IntrinsicMap[CmpInst::Predicate::ICMP_EQ] = Intrinsic::tpc_sel_eq;
    IntrinsicMap[CmpInst::Predicate::ICMP_UGT] = Intrinsic::tpc_sel_grt;
    IntrinsicMap[CmpInst::Predicate::ICMP_UGE] = Intrinsic::tpc_sel_geq;
    IntrinsicMap[CmpInst::Predicate::ICMP_ULT] = Intrinsic::tpc_sel_less;
    IntrinsicMap[CmpInst::Predicate::ICMP_ULE] = Intrinsic::tpc_sel_leq;
    IntrinsicMap[CmpInst::Predicate::ICMP_NE] = Intrinsic::tpc_sel_neq;
    IntrinsicMap[CmpInst::Predicate::ICMP_SGT] = Intrinsic::tpc_sel_grt;
    IntrinsicMap[CmpInst::Predicate::ICMP_SGE] = Intrinsic::tpc_sel_geq;
    IntrinsicMap[CmpInst::Predicate::ICMP_SLT] = Intrinsic::tpc_sel_less;
    IntrinsicMap[CmpInst::Predicate::ICMP_SLE] = Intrinsic::tpc_sel_leq;

    // Create a type map.
    // Limitation: LLVM does not distinguish between unsigned
    // and signed integers. Hence the encoding of unsigned for
    // select intrinsics can't be put here.
    // Need to figure out a way.
    TypeMap[Type::FloatTyID] = 0;
    TypeMap[Type::BFloat16ID] = 1;
    TypeMap[Type::HalfTyID] = 11;
    // i1 Type
    TypeMap[Type::IntegerTyID] = 6;
    // i4 Type
    TypeMap[4 * Type::IntegerTyID] = 9;
    // i8 Type
    TypeMap[8 * Type::IntegerTyID] = 4;
    // i16 Type
    TypeMap[16 * Type::IntegerTyID] = 7;
    // i32 Type
    TypeMap[32 * Type::IntegerTyID] = 2;
    // i64 Type
    TypeMap[64 * Type::IntegerTyID] = 14;
  }

  bool runOnFunction(Function &F) override;
  bool replaceCmpSel(Instruction &I);
  bool replaceSelWithSel2(std::pair<Instruction *, Instruction *> Sel);
  unsigned getElementID(Value *);
  bool replaceOrSel(SelectInst &SelInst);

private:
  SmallDenseSet<Instruction *> DeleteList;
  std::list<Instruction *> SelList;
  std::pair<Instruction *, Instruction *> sels;
  DenseMap<unsigned, Intrinsic::ID> IntrinsicMap;
  DenseMap<unsigned, unsigned> TypeMap;
};
} // namespace

INITIALIZE_PASS(TPCReplaceCmpSelPass, PassName, PassDescription, false, false)
char TPCReplaceCmpSelPass::ID = 0;
FunctionPass *llvm::createTPCReplaceCmpSelPass() {
  return new TPCReplaceCmpSelPass();
}

static std::string getIntrinsicName(Intrinsic::ID IDNum, FunctionType *FType) {
  SmallVector<Intrinsic::IITDescriptor, 8> Table;
  Intrinsic::getIntrinsicInfoTableEntries(IDNum, Table);
  ArrayRef<Intrinsic::IITDescriptor> TableRef = Table;
  (void)TableRef;
  SmallVector<Type *, 4> ArgTys;
  Intrinsic::matchIntrinsicSignature(FType, TableRef, ArgTys);
  return Intrinsic::getName(IDNum, ArgTys);
}

// Returns true, if the \p I is OR instruction.
static bool isOrInst(Instruction &I) {
  auto *OrI = dyn_cast<BinaryOperator>(&I);
  if (!OrI || OrI->getOpcode() != Instruction::Or) {
    LLVM_DEBUG(dbgs() << "Not an Or\n");
    return false;
  }
  return true;
}

// A wrapper to create Sel_* intrinsic call instruction.
static CallInst *CreateSelIntrinsic(IRBuilder<> &Builder, CmpInst *CmpI,
                                    Value *TrueVal, Value *FalseVal,
                                    Type *CmpTy, Type *ValTy,
                                    unsigned int IntrinsicID,
                                    unsigned int ElementTyID) {
  auto &Context = CmpI->getParent()->getContext();
  SmallVector<Type *, 8> Types{CmpTy,
                               CmpTy,
                               ValTy,
                               ValTy,
                               IntegerType::get(Context, 8),
                               IntegerType::get(Context, 32),
                               ValTy,
                               llvm::Type::getInt1Ty(Context),
                               llvm::Type::getInt1Ty(Context)};

  FunctionType *FType = FunctionType::get(ValTy, Types, false);
  Function *Intrinsic = cast<Function>(
      CmpI->getModule()
          ->getOrInsertFunction(getIntrinsicName(IntrinsicID, FType), FType)
          .getCallee());
  auto *Call = Builder.CreateCall(
      Intrinsic,
      {CmpI->getOperand(0), CmpI->getOperand(1), TrueVal, FalseVal,
       llvm::ConstantInt::get(IntegerType::get(Context, 8), ElementTyID),
       llvm::ConstantInt::get(IntegerType::get(Context, 32), 0),
       UndefValue::get(ValTy),
       llvm::ConstantInt::get(Type::getInt1Ty(Context), 1),
       llvm::ConstantInt::getFalse(Context)});
  return Call;
}

// This api tries to pattern match "select + or" and replace with a call to
// sel_* intrinsic.
bool TPCReplaceCmpSelPass::replaceOrSel(SelectInst &SelInst) {
  Value *CV = SelInst.getCondition();
  Instruction *OrInst = dyn_cast<Instruction>(CV);
  // Early exit if not an or instruction.
  if (!OrInst || !isOrInst(*OrInst))
    return false;

  // Try to match CMPs feeding into OR.
  Value *TV = SelInst.getTrueValue();
  Value *FV = SelInst.getFalseValue();

  Value *OrOp0 = OrInst->getOperand(0);
  Value *OrOp1 = OrInst->getOperand(1);

  CmpInst *CmpI0 = dyn_cast<CmpInst>(OrOp0);
  CmpInst *CmpI1 = dyn_cast<CmpInst>(OrOp1);
  if (!CmpI0 || !OrOp1)
    return false;

  auto Ty = SelInst.getOperand(1)->getType();
  auto CmpTy = CmpI0->getOperand(0)->getType();

  // Check supported predicate
  auto PredKind = CmpI0->getPredicate();
  auto It = IntrinsicMap.find(PredKind);
  if (It == IntrinsicMap.end()) {
    LLVM_DEBUG(dbgs() << "sel_* intrinsic does not support this predicate\n");
    return false;
  }
  // Get intrinsic ID for supported predicate
  auto ID = It->second;

  // We only need this transformation for vector types.
  // Exclude co-ordinate vector type.
  auto &Context = SelInst.getParent()->getContext();
  auto CoorType = VectorType::get(IntegerType::get(Context, 32), 5);
  if (!Ty->isVectorTy() || !CmpTy->isVectorTy() || Ty == CoorType ||
      CmpTy == CoorType) {
    LLVM_DEBUG(dbgs() << "Vector type check failed\n");
    return false;
  }

  // If the type is not supported for sel_* intrinsic, then return.
  VectorType *VType = dyn_cast<VectorType>(Ty);
  assert(VType != nullptr && "Type should be a vector.");
  Type *ElementTy = VType->getElementType();
  unsigned TyID = ElementTy->getTypeID();
  if (ElementTy->isIntegerTy())
    TyID = ElementTy->getScalarSizeInBits() * TyID;

  auto TyIt = TypeMap.find(TyID);
  if (TyIt == TypeMap.end()) {
    LLVM_DEBUG(dbgs() << "Unsupported type for sel_* intrinsic\n");
    return false;
  }
  auto ElementTyID = TyIt->second;

  // Create new sel_* intrinsic
  IRBuilder<> Builder(&SelInst);
  Instruction *NewSelInst =
      CreateSelIntrinsic(Builder, CmpI1, TV, FV, CmpTy, Ty, ID, ElementTyID);
  CallInst *NewSelIntrinsic = CreateSelIntrinsic(Builder, CmpI0, TV, NewSelInst,
                                                 CmpTy, Ty, ID, ElementTyID);

  SelInst.replaceAllUsesWith(NewSelIntrinsic);
  SelList.push_back(NewSelIntrinsic);
  DeleteList.insert(&SelInst);
  DeleteList.insert(OrInst);
  return true;
}

bool TPCReplaceCmpSelPass::replaceCmpSel(Instruction &I) {
  auto *SI = dyn_cast<SelectInst>(&I);
  LLVM_DEBUG(dbgs() << "Finding Cmp-Sel pattern\n");
  if (!SI)
    return false;

  auto *CmpI = dyn_cast<CmpInst>(SI->getCondition());
  if (!CmpI) {
    if (replaceOrSel(*SI))
      return true;
    LLVM_DEBUG(dbgs() << "Cmp/Sel not found\n");
    return false;
  }

  auto &Context = I.getParent()->getContext();
  auto PredKind = CmpI->getPredicate();
  auto It = IntrinsicMap.find(PredKind);
  if (It == IntrinsicMap.end()) {
    LLVM_DEBUG(dbgs() << "Replacement sel_* intrinsic not found\n");
    return false;
  }

  auto Ty = SI->getOperand(1)->getType();
  auto CmpTy = CmpI->getOperand(0)->getType();

  // We only need this transformation for vector types.
  // Exclude co-ordinate vector type.
  auto CoorType = VectorType::get(IntegerType::get(Context, 32), 5);
  if (!Ty->isVectorTy() || !CmpTy->isVectorTy() || Ty == CoorType ||
      CmpTy == CoorType) {
    LLVM_DEBUG(dbgs() << "Vector type check failed\n");
    return false;
  }

  // If the type is not supported for sel_* intrinsic, then return.
  VectorType *VType = dyn_cast<VectorType>(Ty);
  assert(VType != nullptr && "Type should be a vector.");
  Type *ElementTy = VType->getElementType();
  unsigned TyID = ElementTy->getTypeID();
  if (ElementTy->isIntegerTy())
    TyID = ElementTy->getScalarSizeInBits() * TyID;

  auto TyIt = TypeMap.find(TyID);
  if (TyIt == TypeMap.end()) {
    LLVM_DEBUG(dbgs() << "Unsupported type for sel_* intrinsic\n");
    return false;
  }

  auto ElementTyID = TyIt->second;

  auto ID = It->second;
  IRBuilder<> Builder(&I);
  SmallVector<Type *, 8> Types{CmpTy,
                               CmpTy,
                               Ty,
                               Ty,
                               IntegerType::get(Context, 8),
                               IntegerType::get(Context, 32),
                               Ty,
                               llvm::Type::getInt1Ty(Context),
                               llvm::Type::getInt1Ty(Context)};

  FunctionType *FType = FunctionType::get(Ty, Types, false);
  Function *Intrinsic = cast<Function>(
      I.getModule()
          ->getOrInsertFunction(getIntrinsicName(ID, FType), FType)
          .getCallee());
  auto *Call = Builder.CreateCall(
      Intrinsic,
      {CmpI->getOperand(0), CmpI->getOperand(1), SI->getOperand(1),
       SI->getOperand(2),
       llvm::ConstantInt::get(IntegerType::get(Context, 8), ElementTyID),
       llvm::ConstantInt::get(IntegerType::get(Context, 32), 0),
       UndefValue::get(SI->getOperand(1)->getType()),
       llvm::ConstantInt::get(Type::getInt1Ty(Context), 1),
       llvm::ConstantInt::getFalse(Context)});

  SI->replaceAllUsesWith(Call);
  SelList.push_back(Call);
  DeleteList.insert(SI);
  DeleteList.insert(CmpI);

  return true;
}

unsigned TPCReplaceCmpSelPass::getElementID(Value *op1) {
  auto Ty = op1->getType();
  VectorType *VType = dyn_cast<VectorType>(Ty);
  Type *ElementTy = VType->getElementType();
  unsigned TyID = ElementTy->getTypeID();
  if (ElementTy->isIntegerTy())
    TyID = ElementTy->getScalarSizeInBits() * TyID;
  auto TyIt = TypeMap.find(TyID);
  if (TyIt == TypeMap.end()) {
    LLVM_DEBUG(dbgs() << "Unsupported type for sel_* intrinsic\n");
    return INVALID_ID;
  }
  return TyIt->second;
}
static Type *getSel2ReturnType(Type *Ty) {
  if (!Ty->isVectorTy())
    return nullptr;
  auto VecTy = dyn_cast<VectorType>(Ty);
  auto ElemenTy = VecTy->getElementType();
  unsigned ElementCount = VecTy->getElementCount().Min;
  return VectorType::get(ElemenTy, 2 * ElementCount);
}

bool TPCReplaceCmpSelPass::replaceSelWithSel2(
    std::pair<Instruction *, Instruction *> sel) {
  Instruction *inst2 = sel.first;
  Instruction *inst1 = sel.second;
  /*Check if both are Intrinsics*/
  if (auto *Intrins1 = (dyn_cast<IntrinsicInst>(inst1))) {
    if (auto *Intrins2 = (dyn_cast<IntrinsicInst>(inst2))) {
      /*compare the instinsic ID for both of them*/
      Intrinsic::ID Inid1 = Intrins1->getIntrinsicID();
      Intrinsic::ID Inid2 = Intrins2->getIntrinsicID();
      if (Inid1 == Inid2) {
        /*Check if the first two arg match next 2*/
        Value *op0, *op1, *op2, *op3;
        op0 = inst1->getOperand(0);
        op1 = inst1->getOperand(1);
        op2 = inst1->getOperand(2);
        op3 = inst1->getOperand(3);
        if (((op0 == op2) && (op1 == op3)) || ((op0 == op3) && (op1 == op2))) {
          /*compare this with inst 2*/
          Value *inst1op0, *inst1op1, *inst1op2, *inst1op3;
          inst1op0 = inst2->getOperand(0);
          inst1op1 = inst2->getOperand(1);
          inst1op2 = inst2->getOperand(2);
          inst1op3 = inst2->getOperand(3);
          if (((op0 == inst1op0) && (op1 == inst1op1)) ||
              ((op0 == inst1op1) && (op1 == inst1op0))) {
            auto &Context = inst1->getParent()->getContext();
            Intrinsic::ID ID;
            if (Inid1 == Intrinsic::tpc_sel_grt) {
              ID = Intrinsic::tpc_sel2_grt;
            } else if (Inid1 == Intrinsic::tpc_sel_leq) {
              ID = Intrinsic::tpc_sel2_leq;
            } else if (Inid1 == Intrinsic::tpc_sel_less) {
              ID = Intrinsic::tpc_sel2_less;
            } else if (Inid1 == Intrinsic::tpc_sel_geq) {
              ID = Intrinsic::tpc_sel2_geq;
            } else {
              return false;
            }
            /*Now we need to build the sel2 intrinsic*/

            IRBuilder<> Builder(inst1);
            auto IncomeTy = getSel2ReturnType(inst1op2->getType());
            SmallVector<Type *, 8> Types{op0->getType(),
                                         op0->getType(),
                                         op1->getType(),
                                         op1->getType(),
                                         IntegerType::get(Context, 8),
                                         IntegerType::get(Context, 32),
                                         IncomeTy,
                                         llvm::Type::getInt1Ty(Context),
                                         llvm::Type::getInt1Ty(Context)};

            FunctionType *FType = FunctionType::get(IncomeTy, Types, false);
            Function *Intrinsic = cast<Function>(
                inst1->getModule()
                    ->getOrInsertFunction(getIntrinsicName(ID, FType), FType)
                    .getCallee());

            unsigned ElementTyID = getElementID(op1);
            if (ElementTyID == INVALID_ID) {
              return false;
            }

            auto *Call = Builder.CreateCall(
                Intrinsic,
                {op0, op1, inst1op2, inst1op3,
                 llvm::ConstantInt::get(IntegerType::get(Context, 8),
                                        ElementTyID),
                 llvm::ConstantInt::get(IntegerType::get(Context, 32), 0),
                 UndefValue::get(IncomeTy),
                 llvm::ConstantInt::get(Type::getInt1Ty(Context), 1),
                 llvm::ConstantInt::getFalse(Context)});

            auto Sel2Type = inst1op2->getType();
            unsigned Val = 0;
            if (Sel2Type->isVectorTy()) {
              auto VectorTy = dyn_cast<VectorType>(Sel2Type);
              Val = (VectorTy->getElementCount().Min);
            }
            auto Mask = createSequentialMask(Builder, 0, Val, 0);
            auto Shuffle1 = Builder.CreateShuffleVector(
                Call, UndefValue::get(Call->getType()), Mask);

            Mask = createSequentialMask(Builder, Val, Val, 0);
            auto Shuffle2 = Builder.CreateShuffleVector(
                Call, UndefValue::get(Call->getType()), Mask);

            inst1->replaceAllUsesWith(Shuffle2);
            inst2->replaceAllUsesWith(Shuffle1);
            DeleteList.insert(inst1);
            DeleteList.insert(inst2);
          }
        }
      }
    }
  }
  return false;
}

bool TPCReplaceCmpSelPass::runOnFunction(Function &F) {
  if (skipFunction(F))
    return false;

  bool Change = false;
  for (auto &BB : F)
    for (auto &I : BB) {
      Change |= replaceCmpSel(I);
    }

  /*Check the list and make it a pair*/
  if (SelList.size() < 2) {
    return Change;
  }

  while (SelList.size() > 1) {
    sels.first = SelList.back();
    SelList.pop_back();
    sels.second = SelList.back();
    SelList.pop_back();
    replaceSelWithSel2(sels);
  }

  for (auto *I : DeleteList) {
    I->replaceAllUsesWith(UndefValue::get(I->getType()));
    I->eraseFromParent();
  }

  return Change;
}
