//===- TPCTransformIntrin.cpp -----------------------------------*- C++ -*-===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//
// This pass
// transforms intrinsic call into expression
// kind of add(a,b) => a+b
//===----------------------------------------------------------------------===//
#ifdef LLVM_TPC_COMPILER

#include "TPCTargetMachine.h"
#include "llvm/IR/Function.h"
#include "llvm/IR/Instruction.h"
#include "llvm/IR/IntrinsicsTPC.h"
#include "llvm/IR/InstIterator.h"
#include "llvm/IR/IntrinsicInst.h"
#include "llvm/IR/Intrinsics.h"
#include "llvm/IR/IRBuilder.h"
#include "llvm/IR/Module.h"
#include "llvm/IR/LLVMContext.h"
#include "llvm/Pass.h"
#include "llvm/PassRegistry.h"

#define DEBUG_TYPE "TPCTransformIntrin"

using namespace llvm;

namespace llvm {
  FunctionPass *createTPCTransformIntrinPass();
  void initializeTPCTransformIntrinPass(PassRegistry&);
}

static const char PassDescription[] = "TPC intrinsic transformations";
static const char PassName[] = "tpc-trans-intr";

static cl::opt<bool>
EnableTPCTransformIntrin(PassName,
  cl::Hidden,
  cl::init(true));

// EXTRA options for management
static cl::opt<bool>
EnableTPCTransformIntrinInt5("tpc-trans-intr-int5",
  cl::Hidden,
  cl::init(false));

static cl::opt<bool>
EnableTPCTransformIntrinFloat("tpc-trans-intr-float",
  cl::Hidden,
  cl::init(true));


// usual integer cmp causes to perf regression, as these statements mobe to 
// BB when it is used
static cl::opt<bool>
EnableTPCTransformIntrinCmp("tpc-trans-intr-cmp",
  cl::Hidden,
  cl::init(true));

static cl::opt<bool>
EnableTPCTransformIntrinMinMax("tpc-trans-intr-minmax",
  cl::Hidden,
  cl::init(true));

namespace {
  class TPCTransformIntrin : public FunctionPass {
    Function *F = nullptr;
    unsigned NumTransformed = 0;
  public:
    static char ID;

    StringRef getPassName() const override { return PassDescription; }

    TPCTransformIntrin() : FunctionPass(ID) {
      initializeTPCTransformIntrinPass(*PassRegistry::getPassRegistry());
    }
    bool runOnFunction(Function &F) override;
  };//class
}//namespace

char TPCTransformIntrin::ID = 0;
INITIALIZE_PASS(TPCTransformIntrin, PassName, PassDescription, false, false)

FunctionPass *llvm::createTPCTransformIntrinPass() {
  return new TPCTransformIntrin();
}

bool TPCTransformIntrin::runOnFunction(Function &Func) {

  if (!EnableTPCTransformIntrin) {
    return false;
  }

  if (skipFunction(Func))
    return false;
  F = &Func;
  LLVMContext &Ctx = Func.getContext();
  NumTransformed = 0;
  IntegerType *I32Type = Type::getInt32Ty(Ctx);
  IntegerType *I1Type = Type::getInt1Ty(Ctx);
  IntegerType *I8Type = Type::getInt8Ty(Ctx);
  IntegerType *I16Type = Type::getInt16Ty(Ctx);
  Type *F32Type = Type::getFloatTy(Ctx);
  Type *F16Type = Type::getBFloat16Ty(Ctx);
  VectorType* Int64Type = VectorType::get(I32Type, 64);
  VectorType* Float64Type = VectorType::get(F32Type, 64);
  VectorType* Short128Type = VectorType::get(I16Type, 128);
  VectorType* Bfloat128Type = VectorType::get(F16Type, 128);
  VectorType* Char256Type = VectorType::get(I8Type, 256);
  VectorType* Int5Type = VectorType::get(I32Type, 5);
  for (auto BBIt = Func.begin(), BBEnd = Func.end(); BBIt != BBEnd;) {
    BasicBlock &BB = *BBIt;
    ++BBIt;

    for (auto It = BB.begin(), E = BB.end(); It != E; ) {
      Instruction &I = *It;
      ++It;
      /////////////////////// Insert here //////////////////
      if (const IntrinsicInst* intrins = dyn_cast<IntrinsicInst>(&I)) {
        IRBuilder<> Builder(&I);
        Intrinsic::ID inid = intrins->getIntrinsicID();
        int no = intrins->getNumOperands();
        switch (inid) {
        case Intrinsic::tpc_add_mask:
          // all other _mask operations is not properly supported
          // works only tpc_add_mask with combination vector/scalar
          // vector/vector is not supported due to problems in clang
        case Intrinsic::tpc_sub_mask:
        case Intrinsic::tpc_and_mask:
        case Intrinsic::tpc_or_mask:
        case Intrinsic::tpc_xor_mask:
        {
          if (no != 9) {
            continue;
          }
          //last arg op 9 is not intrested
          auto op7 = intrins->getOperand(7);
          auto op6 = intrins->getOperand(6);
          auto op5 = intrins->getOperand(5);
          auto op4 = intrins->getOperand(4);  
          auto op3 = intrins->getOperand(3);
          auto op2 = intrins->getOperand(2);
          auto op1 = intrins->getOperand(1);
          auto op0 = intrins->getOperand(0);
          Type* t1 = op1->getType();
          Type* t0 = op0->getType();
          if (t0 != Int5Type) {
            continue;
          }
          APInt apint;
          Constant* cv;
          cv = dyn_cast<Constant>(op7);
          if (!cv) { continue; }
          if (cv->getType() != I1Type) {
            continue;
          }
          apint = cv->getUniqueInteger();
          if (apint != 0) {
            // polarity must be zero
            continue;
          }
          // predicate must be true
          cv = dyn_cast<Constant>(op6);
          if (!cv) { continue; }
          if (cv->getType() != I1Type) {
            continue;
          }
          apint = cv->getUniqueInteger();
          if (apint != 1) {
            continue;
          }
          // income arg 
          Type* income_type = op5->getType();
          if (income_type != t0) {
            continue;
          }

          cv = dyn_cast<Constant>(op4);
          if (!cv) { continue; }
          if (cv->getType() != I32Type) {
            continue;
          }
          apint = cv->getUniqueInteger();
          if (apint != 0) {
            continue;
          }
          cv = dyn_cast<Constant>(op3);
          if (!cv) { continue; }
          if (cv->getType() != I8Type) {
            continue;
          }
          apint = cv->getUniqueInteger();
          if (apint != 2) {
            continue;
          }
          // dimmask
          cv = dyn_cast<Constant>(op2);
          if (!cv) { continue; }
          if (cv->getType() != I32Type) {
            continue;
          }
          apint = cv->getUniqueInteger();     
          Value* newins;
          // int5 sake
          int vne;
          if (t0->isVectorTy()) {
            vne = t0->getVectorNumElements();
            if (vne == 5 && EnableTPCTransformIntrinInt5) { //int5 case
              if (t0->isVectorTy() && t1->isVectorTy()) {
                // however at the moment this config is imposible under tpc_add_mask
                // only tpc_add, need to wait some stabilization

                // maybe some is splat
                if (auto shins = dyn_cast<ShuffleVectorInst>(op0)) {
                  const Constant * mask = shins->getMask();
                  if (mask && mask->isZeroValue()) {
                    Constant *CI = ConstantInt::get(I32Type, 0);
                    auto sav0 = op0;
                    op0 = op1;
                    op1 = Builder.CreateExtractElement(sav0, CI);
                    t1 = op1->getType();
                    t0 = op0->getType();
                  }
                }
		/*
                else if (ConstantVector* cv = dyn_cast<ConstantVector>(op0)) {
                  // <5 x i32> <i32 1, i32 1, i32 1, i32 1, i32 1>
                  // however do not recognize as ConstantVector
                  // may be change in clang would be more reliable
                }*/
              }
              if (!t1->isVectorTy()) { // int 5 with scalar
                if (income_type != t0) {
                  continue;
                }
                if (apint == 0) { //dimmask ?
                  continue;
                }
                // ready to emit code for int5
                if (inid == Intrinsic::tpc_add_mask) {
                  if ((apint & 1) != 0) {
                    Constant *CI = ConstantInt::get(I32Type, 0);
                    newins = Builder.CreateExtractElement(op0, CI);
                    newins = Builder.CreateNSWAdd(newins, op1);
                    newins = Builder.CreateInsertElement(op0, newins, CI);
                  }
                  if ((apint & 2) != 0) {
                    Constant *CI = ConstantInt::get(I32Type, 1);
                    newins = Builder.CreateExtractElement(op0, CI);
                    newins = Builder.CreateNSWAdd(newins, op1);
                    newins = Builder.CreateInsertElement(op0, newins, CI);
                  }
                  if ((apint & 4) != 0) {
                    Constant *CI = ConstantInt::get(I32Type, 2);
                    newins = Builder.CreateExtractElement(op0, CI);
                    newins = Builder.CreateNSWAdd(newins, op1);
                    newins = Builder.CreateInsertElement(op0, newins, CI);
                  }
                  if ((apint & 8) != 0) {
                    Constant *CI = ConstantInt::get(I32Type, 3);
                    newins = Builder.CreateExtractElement(op0, CI);
                    newins = Builder.CreateNSWAdd(newins, op1);
                    newins = Builder.CreateInsertElement(op0, newins, CI);
                  }
                  if ((apint & 16) != 0) {
                    Constant *CI = ConstantInt::get(I32Type, 4);
                    newins = Builder.CreateExtractElement(op0, CI);
                    newins = Builder.CreateNSWAdd(newins, op1);
                    newins = Builder.CreateInsertElement(op0, newins, CI);
                  }
                }
                else if (inid == Intrinsic::tpc_sub_mask) {
                  if ((apint & 1) != 0) {
                    Constant *CI = ConstantInt::get(I32Type, 0);
                    newins = Builder.CreateExtractElement(op0, CI);
                    newins = Builder.CreateNSWSub(newins, op1);
                    newins = Builder.CreateInsertElement(op0, newins, CI);
                  }
                  if ((apint & 2) != 0) {
                    Constant *CI = ConstantInt::get(I32Type, 1);
                    newins = Builder.CreateExtractElement(op0, CI);
                    newins = Builder.CreateNSWSub(newins, op1);
                    newins = Builder.CreateInsertElement(op0, newins, CI);
                  }
                  if ((apint & 4) != 0) {
                    Constant *CI = ConstantInt::get(I32Type, 2);
                    newins = Builder.CreateExtractElement(op0, CI);
                    newins = Builder.CreateNSWSub(newins, op1);
                    newins = Builder.CreateInsertElement(op0, newins, CI);
                  }
                  if ((apint & 8) != 0) {
                    Constant *CI = ConstantInt::get(I32Type, 3);
                    newins = Builder.CreateExtractElement(op0, CI);
                    newins = Builder.CreateNSWSub(newins, op1);
                    newins = Builder.CreateInsertElement(op0, newins, CI);
                  }
                  if ((apint & 16) != 0) {
                    Constant *CI = ConstantInt::get(I32Type, 4);
                    newins = Builder.CreateExtractElement(op0, CI);
                    newins = Builder.CreateNSWSub(newins, op1);
                    newins = Builder.CreateInsertElement(op0, newins, CI);
                  }
                }
                else if (inid == Intrinsic::tpc_or_mask) {
                  if ((apint & 1) != 0) {
                    Constant *CI = ConstantInt::get(I32Type, 0);
                    newins = Builder.CreateExtractElement(op0, CI);
                    newins = Builder.CreateOr(newins, op1);
                    newins = Builder.CreateInsertElement(op0, newins, CI);
                  }
                  if ((apint & 2) != 0) {
                    Constant *CI = ConstantInt::get(I32Type, 1);
                    newins = Builder.CreateExtractElement(op0, CI);
                    newins = Builder.CreateOr(newins, op1);
                    newins = Builder.CreateInsertElement(op0, newins, CI);
                  }
                  if ((apint & 4) != 0) {
                    Constant *CI = ConstantInt::get(I32Type, 2);
                    newins = Builder.CreateExtractElement(op0, CI);
                    newins = Builder.CreateOr(newins, op1);
                    newins = Builder.CreateInsertElement(op0, newins, CI);
                  }
                  if ((apint & 8) != 0) {
                    Constant *CI = ConstantInt::get(I32Type, 3);
                    newins = Builder.CreateExtractElement(op0, CI);
                    newins = Builder.CreateOr(newins, op1);
                    newins = Builder.CreateInsertElement(op0, newins, CI);
                  }
                  if ((apint & 16) != 0) {
                    Constant *CI = ConstantInt::get(I32Type, 4);
                    newins = Builder.CreateExtractElement(op0, CI);
                    newins = Builder.CreateOr(newins, op1);
                    newins = Builder.CreateInsertElement(op0, newins, CI);
                  }
                }
                else if (inid == Intrinsic::tpc_and_mask) {
                  if ((apint & 1) != 0) {
                    Constant *CI = ConstantInt::get(I32Type, 0);
                    newins = Builder.CreateExtractElement(op0, CI);
                    newins = Builder.CreateAnd(newins, op1);
                    newins = Builder.CreateInsertElement(op0, newins, CI);
                  }
                  if ((apint & 2) != 0) {
                    Constant *CI = ConstantInt::get(I32Type, 1);
                    newins = Builder.CreateExtractElement(op0, CI);
                    newins = Builder.CreateAnd(newins, op1);
                    newins = Builder.CreateInsertElement(op0, newins, CI);
                  }
                  if ((apint & 4) != 0) {
                    Constant *CI = ConstantInt::get(I32Type, 2);
                    newins = Builder.CreateExtractElement(op0, CI);
                    newins = Builder.CreateAnd(newins, op1);
                    newins = Builder.CreateInsertElement(op0, newins, CI);
                  }
                  if ((apint & 8) != 0) {
                    Constant *CI = ConstantInt::get(I32Type, 3);
                    newins = Builder.CreateExtractElement(op0, CI);
                    newins = Builder.CreateAnd(newins, op1);
                    newins = Builder.CreateInsertElement(op0, newins, CI);
                  }
                  if ((apint & 16) != 0) {
                    Constant *CI = ConstantInt::get(I32Type, 4);
                    newins = Builder.CreateExtractElement(op0, CI);
                    newins = Builder.CreateAnd(newins, op1);
                    newins = Builder.CreateInsertElement(op0, newins, CI);
                  }
                }
                else if (inid == Intrinsic::tpc_xor_mask) {
                  if ((apint & 1) != 0) {
                    Constant *CI = ConstantInt::get(I32Type, 0);
                    newins = Builder.CreateExtractElement(op0, CI);
                    newins = Builder.CreateXor(newins, op1);
                    newins = Builder.CreateInsertElement(op0, newins, CI);
                  }
                  if ((apint & 2) != 0) {
                    Constant *CI = ConstantInt::get(I32Type, 1);
                    newins = Builder.CreateExtractElement(op0, CI);
                    newins = Builder.CreateXor(newins, op1);
                    newins = Builder.CreateInsertElement(op0, newins, CI);
                  }
                  if ((apint & 4) != 0) {
                    Constant *CI = ConstantInt::get(I32Type, 2);
                    newins = Builder.CreateExtractElement(op0, CI);
                    newins = Builder.CreateXor(newins, op1);
                    newins = Builder.CreateInsertElement(op0, newins, CI);
                  }
                  if ((apint & 8) != 0) {
                    Constant *CI = ConstantInt::get(I32Type, 3);
                    newins = Builder.CreateExtractElement(op0, CI);
                    newins = Builder.CreateXor(newins, op1);
                    newins = Builder.CreateInsertElement(op0, newins, CI);
                  }
                  if ((apint & 16) != 0) {
                    Constant *CI = ConstantInt::get(I32Type, 4);
                    newins = Builder.CreateExtractElement(op0, CI);
                    newins = Builder.CreateXor(newins, op1);
                    newins = Builder.CreateInsertElement(op0, newins, CI);
                  }
                }
                else {
                  continue;
                }
                I.replaceAllUsesWith(newins);
                I.eraseFromParent();
                NumTransformed++;
                continue;
              }
            }
          }

        } break;
        case Intrinsic::tpc_add:  
        case Intrinsic::tpc_sub:  
        case Intrinsic::tpc_mul:
        case Intrinsic::tpc_and:  
        case Intrinsic::tpc_or:   
        case Intrinsic::tpc_xor:  
        case Intrinsic::tpc_min:
        case Intrinsic::tpc_max:
        case Intrinsic::tpc_cmp_eq:
        case Intrinsic::tpc_cmp_neq:
        case Intrinsic::tpc_cmp_less:
        case Intrinsic::tpc_cmp_leq:
        case Intrinsic::tpc_cmp_grt:
        case Intrinsic::tpc_cmp_geq:
        case Intrinsic::tpc_shl:
        case Intrinsic::tpc_shr:
        {
          if (no != 8) {
            continue;
          }

          //auto op7 = intrins->getOperand(7); //last arg is not intrested
          auto op6 = intrins->getOperand(6);
          auto op5 = intrins->getOperand(5);
          auto op4 = intrins->getOperand(4);  // income , need to look when MUL
          auto op3 = intrins->getOperand(3);
          auto op2 = intrins->getOperand(2);
          auto op1 = intrins->getOperand(1);
          auto op0 = intrins->getOperand(0);

          Constant* cv = dyn_cast<Constant>(op6);
          Constant* cv2;
          if (!cv) { continue; }
          if (cv->getType() != I1Type) {
            continue;
          }
          APInt apint = cv->getUniqueInteger();
          if (apint != 0) {
            // polarity must be zero
            continue;
          }
          // predicate must be true
          cv = dyn_cast<Constant>(op5);
          if (!cv) { continue; }
          if (cv->getType() != I1Type) {
            continue;
          }
          apint = cv->getUniqueInteger();
          if (apint != 1) {
            continue;
          }
          // op4 : need to look type
          Type* income_type = op4->getType();
          // op3 
          cv = dyn_cast<Constant>(op3);

          if (!cv) { continue; }
          if (cv->getType() != I32Type) {
            continue;
          }
          apint = cv->getUniqueInteger();

          Type* t1 = op1->getType();
          Type* t0 = op0->getType();
          Value* newins = nullptr;
          // 1 with saturation for int. skip it
         if (apint != 0) {
            continue;
          }

          cv2 = dyn_cast<Constant>(op2);
          if (!cv2) { continue; }
          if (cv2->getType() != I8Type) {
            continue;
          }
          bool shins = false;
          if (inid == Intrinsic::tpc_shl || inid == Intrinsic::tpc_shr) {
            if (t1->isVectorTy()) {
              if (((t0 == Int64Type || t0 == Float64Type) && t1 == Int64Type) ||
                ((t0 == Short128Type || t0 == Bfloat128Type) && t1 == Short128Type) ||
                (t0 == Char256Type  && t1 == Char256Type) 
                ) {
              }
              else {
                continue;
              }
            }
            else if (t0->isVectorTy() && t1->isIntegerTy()) {
              // as imposible to call CreateShl with different types case is not processable
              // however need to consider case when op1 is static constant
              // for this there is method CreateShl(Value, int64_t (APint)
              continue;
              shins = true;
            }
            else {
              if (t1 != I8Type) {
                continue;
              }
            }
            shins = true;
          }
          else if (t1 != t0) {
            // now only equal types are supported
            continue;
          }
          if (t0 == t1 || shins) {
            apint = cv2->getUniqueInteger();
            if (t0->isFPOrFPVectorTy() && EnableTPCTransformIntrinFloat) {
              // op2 0 - float, 1-bfloat 2-int
              if (!(apint == 0 || apint == 1)) {
                continue;
              }
              if (inid == Intrinsic::tpc_add) {
                newins = Builder.CreateFAdd(op0, op1);
              }
              else  if (inid == Intrinsic::tpc_sub) {
                newins = Builder.CreateFSub(op0, op1);
              }
              else  if (inid == Intrinsic::tpc_mul) {
                if (t0 != income_type) { // for mul may it is type of result
                  continue;              // not supported if is other type
                }
                newins = Builder.CreateFMul(op0, op1);
              }
              else  if (inid == Intrinsic::tpc_and) {
                continue; // operation for float is not allowed by language
              }
              else  if (inid == Intrinsic::tpc_or) {
                continue; // operation for float is not allowed by language
              }
              else  if (inid == Intrinsic::tpc_xor) {
                continue; // operation for float is not allowed by language
              }
              else  if (inid == Intrinsic::tpc_min && EnableTPCTransformIntrinMinMax) {
                if (t0->isVectorTy()) {
                  // there is llvm intrinsic, which can be used  onle for fp type
                  if (apint == 0) {
                    newins = Builder.CreateMinNum(op0, op1);
                  }
                  else {
                    continue;
                  }
                }
                else {
                  //Builder.CreateMinNum(op0, op1); cant be be applied 
                  // as it is not implemented yet
                  // it will be assert in include\llvm/CodeGen/TargetLowering.h:3199!
                  // same for max
                  /*if (apint == 0) {
                    newins = Builder.CreateMinNum(op0, op1);
                  }
                  else*/ { //bfloat is not supported in llvm
                    newins = Builder.CreateSelect(Builder.CreateFCmpOLT(op0, op1), op0, op1);
                  }
                }
              }
              else  if (inid == Intrinsic::tpc_max && EnableTPCTransformIntrinMinMax) {
                if (t0->isVectorTy()) {
                  // there is llvm intrinsic, which can be used  onle for fp type
                  if (apint == 0) {
                    newins = Builder.CreateMaxNum(op0, op1);
                  }
                  else {
                    continue;
                  }
                }
                else {
                  /*if (apint == 0) {
                    newins = Builder.CreateMaxNum(op0, op1);
                  }
                  else*/ { //bfloat is not supported in llvm
                    newins = Builder.CreateSelect(Builder.CreateFCmpOGT(op0, op1), op0, op1);
                  }
                }
              }
              else  if (inid == Intrinsic::tpc_cmp_eq && EnableTPCTransformIntrinCmp) {
                // cmp expression for vector is not implemented at the moment
                if (t0->isVectorTy()) { continue; }
                newins = Builder.CreateFCmpOEQ(op0, op1);
              }
              else  if (inid == Intrinsic::tpc_cmp_neq && EnableTPCTransformIntrinCmp) {
                if (t0->isVectorTy()) { continue; }
                newins = Builder.CreateFCmpUNE(op0, op1);
              }
              else  if (inid == Intrinsic::tpc_cmp_less && EnableTPCTransformIntrinCmp) {
                if (t0->isVectorTy()) { continue; }
                newins = Builder.CreateFCmpOLT(op0, op1);
              }
              else  if (inid == Intrinsic::tpc_cmp_leq && EnableTPCTransformIntrinCmp) {
                if (t0->isVectorTy()) { continue; }
                newins = Builder.CreateFCmpOLE(op0, op1);
              }
              else  if (inid == Intrinsic::tpc_cmp_grt && EnableTPCTransformIntrinCmp) {
                if (t0->isVectorTy()) { continue; }
                newins = Builder.CreateFCmpOGT(op0, op1);
              }
              else  if (inid == Intrinsic::tpc_cmp_geq && EnableTPCTransformIntrinCmp) {
                if (t0->isVectorTy()) { continue; }
                newins = Builder.CreateFCmpOGE(op0, op1);
              }
              else  if (inid == Intrinsic::tpc_shl) {
                if (apint == 0) { // float
                  if (t0->isVectorTy()) {
                    op0 = Builder.CreateBitCast(op0, Int64Type);
                    newins = Builder.CreateShl(op0, op1);
                    newins = Builder.CreateBitCast(newins, t0);
                  }
                  else {
                    op0 = Builder.CreateBitCast(op0, I32Type);
                    op1 = Builder.CreateZExt(op1, I32Type);
                    newins = Builder.CreateShl(op0, op1);
                    newins = Builder.CreateBitCast(newins, t0);
                  }
                }
                else if (apint == 1) { //bfloat
                  if (t0->isVectorTy()) {
                    op0 = Builder.CreateBitCast(op0, Short128Type);
                    newins = Builder.CreateShl(op0, op1);
                    newins = Builder.CreateBitCast(newins, t0);
                  }
                  else {
                    op0 = Builder.CreateBitCast(op0, I16Type);
                    op0 = Builder.CreateZExt(op0, I32Type);
                    op1 = Builder.CreateZExt(op1, I32Type);
                    newins = Builder.CreateShl(op0, op1);
                    newins = Builder.CreateTrunc(newins, I16Type);
                    newins = Builder.CreateBitCast(newins, t0);
                  }
                }
              }
              else  if (inid == Intrinsic::tpc_shr) {
                if (apint == 0) { // float
                  if (t0->isVectorTy()) {
                    op0 = Builder.CreateBitCast(op0, Int64Type);
                    newins = Builder.CreateLShr(op0, op1);
                    newins = Builder.CreateBitCast(newins, t0);
                  }
                  else {
                    op0 = Builder.CreateBitCast(op0, I32Type);
                    op1 = Builder.CreateZExt(op1, I32Type);
                    newins = Builder.CreateLShr(op0, op1);
                    newins = Builder.CreateBitCast(newins, t0);
                  }
                }
                else if (apint == 1) { //bfloat
                  if (t0->isVectorTy()) {
                    op0 = Builder.CreateBitCast(op0, Short128Type);
                    newins = Builder.CreateLShr(op0, op1);
                    newins = Builder.CreateBitCast(newins, t0);
                  }
                  else {
                    op0 = Builder.CreateBitCast(op0, I16Type);
                    op0 = Builder.CreateZExt(op0, I32Type);
                    op1 = Builder.CreateZExt(op1, I32Type);
                    newins = Builder.CreateLShr(op0, op1);
                    newins = Builder.CreateTrunc(newins, I16Type);
                    newins = Builder.CreateBitCast(newins, t0);
                  }
                }
              }
              else {
                continue;
              }
            }
            else if (t0->isVectorTy()) {
              if (t0->getVectorNumElements() < 32) {
                // not yet for int5
                continue;
              }
              if (inid == Intrinsic::tpc_add) {
                newins = Builder.CreateAdd(op0, op1);
              }
              else  if (inid == Intrinsic::tpc_sub) {
                newins = Builder.CreateSub(op0, op1);
              }
              else  if (inid == Intrinsic::tpc_mul) {
                continue; // action demands ASHR, which is done in kernels
                newins = Builder.CreateMul(op0, op1);
              }
              else  if (inid == Intrinsic::tpc_and) {
                newins = Builder.CreateAnd(op0, op1);
              }
              else  if (inid == Intrinsic::tpc_or) {
                newins = Builder.CreateOr(op0, op1);
              }
              else  if (inid == Intrinsic::tpc_xor) {
                newins = Builder.CreateXor(op0, op1);
              }
              //tpc_min,tpc_max - no expression for vectors
              /*
                  tpc_cmp_eq:
                  tpc_cmp_neq:
                  tpc_cmp_less:
                  tpc_cmp_leq:
                  tpc_cmp_grt:
                  tpc_cmp_geq:
                               not supported for vector now
             */
              else  if (inid == Intrinsic::tpc_shl) {
                op1 = Builder.CreateZExt(op1, t0);
                newins = Builder.CreateShl(op0, op1);
              }
              else  if (inid == Intrinsic::tpc_shr) {
                op1 = Builder.CreateZExt(op1, t0);
                newins = Builder.CreateLShr(op0, op1);
              }
              else {
                continue;
              }
            }
            else if (t0->isIntegerTy()) {
              // 2 signed, 3-unsigned, 7- short, 8-unsigned short
              // 4 char, 5 - unsigned char
              if (apint != 2 && apint != 3 && apint != 7 &&
                apint != 8 && apint != 4 && apint != 5) {
                continue;
              }
              if (apint == 3) { // unsigned
                if (inid == Intrinsic::tpc_add) {
                  newins = Builder.CreateAdd(op0, op1);
                }
                else  if (inid == Intrinsic::tpc_sub) {
                  newins = Builder.CreateSub(op0, op1);
                }
                else  if (inid == Intrinsic::tpc_mul) {
                  if (I32Type != income_type) {
                    continue;
                  }
                  newins = Builder.CreateMul(op0, op1);
                }
                else  if (inid == Intrinsic::tpc_and) {
                  newins = Builder.CreateAnd(op0, op1);
                }
                else  if (inid == Intrinsic::tpc_or) {
                  newins = Builder.CreateOr(op0, op1);
                }
                else  if (inid == Intrinsic::tpc_xor) {
                  newins = Builder.CreateXor(op0, op1);
                }
                else  if (inid == Intrinsic::tpc_min && EnableTPCTransformIntrinMinMax) {
                  newins = Builder.CreateSelect(Builder.CreateICmpULT(op0, op1), op0, op1);
                }
                else  if (inid == Intrinsic::tpc_max && EnableTPCTransformIntrinMinMax) {
                  newins = Builder.CreateSelect(Builder.CreateICmpUGT(op0, op1), op0, op1);
                }
                else  if (inid == Intrinsic::tpc_cmp_eq && EnableTPCTransformIntrinCmp) {
                  newins = Builder.CreateICmpEQ(op0, op1);
                }
                else  if (inid == Intrinsic::tpc_cmp_neq && EnableTPCTransformIntrinCmp) {
                  newins = Builder.CreateICmpNE(op0, op1);
                }
                else  if (inid == Intrinsic::tpc_cmp_less && EnableTPCTransformIntrinCmp) {
                  newins = Builder.CreateICmpULT(op0, op1);
                }
                else  if (inid == Intrinsic::tpc_cmp_leq && EnableTPCTransformIntrinCmp) {
                  newins = Builder.CreateICmpULE(op0, op1);
                }
                else  if (inid == Intrinsic::tpc_cmp_grt && EnableTPCTransformIntrinCmp) {
                  newins = Builder.CreateICmpUGT(op0, op1);
                }
                else  if (inid == Intrinsic::tpc_cmp_geq && EnableTPCTransformIntrinCmp) {
                  newins = Builder.CreateICmpUGE(op0, op1);
                }
                else  if (inid == Intrinsic::tpc_shl) {
                  op1 = Builder.CreateZExt(op1, t0);
                  newins = Builder.CreateShl(op0, op1);
                }
                else  if (inid == Intrinsic::tpc_shr) {
                  op1 = Builder.CreateZExt(op1, t0);
                  newins = Builder.CreateLShr(op0, op1);
                }
                else {
                  continue;
                }
              }
              else if (apint == 2) { //signed
                if (inid == Intrinsic::tpc_add) {
                  newins = Builder.CreateNSWAdd(op0, op1);
                }
                else  if (inid == Intrinsic::tpc_sub) {
                  newins = Builder.CreateNSWSub(op0, op1);
                }
                else  if (inid == Intrinsic::tpc_mul) {
                  if (I32Type != income_type) {
                    continue;
                  }
                  newins = Builder.CreateNSWMul(op0, op1);
                }
                else  if (inid == Intrinsic::tpc_and) {
                  newins = Builder.CreateAnd(op0, op1);
                }
                else  if (inid == Intrinsic::tpc_or) {
                  newins = Builder.CreateOr(op0, op1);
                }
                else  if (inid == Intrinsic::tpc_xor) {
                  newins = Builder.CreateXor(op0, op1);
                }
                else  if (inid == Intrinsic::tpc_min && EnableTPCTransformIntrinMinMax) {
                  newins = Builder.CreateSelect(Builder.CreateICmpSLT(op0, op1), op0, op1);
                }
                else  if (inid == Intrinsic::tpc_max && EnableTPCTransformIntrinMinMax) {
                  newins = Builder.CreateSelect(Builder.CreateICmpSGT(op0, op1), op0, op1);
                }
                // if use of cmp is outside BB, it moves on CodeGenPrepare
                // into use block, so we skip transfo to avoid perf regression
                // Issue is fixed now by tuning  by HasMultipleConditionRegisters (TPCSelLowering.cpp)
                else  if (inid == Intrinsic::tpc_cmp_eq && EnableTPCTransformIntrinCmp) {
                  newins = Builder.CreateICmpEQ(op0, op1);
                }
                else  if (inid == Intrinsic::tpc_cmp_neq && EnableTPCTransformIntrinCmp) {
                  newins = Builder.CreateICmpNE(op0, op1);
                }
                else  if (inid == Intrinsic::tpc_cmp_less && EnableTPCTransformIntrinCmp) {
                  newins = Builder.CreateICmpSLT(op0, op1);
                }
                else  if (inid == Intrinsic::tpc_cmp_leq && EnableTPCTransformIntrinCmp) {
                  newins = Builder.CreateICmpSLE(op0, op1);
                }
                else  if (inid == Intrinsic::tpc_cmp_grt && EnableTPCTransformIntrinCmp) {
                  newins = Builder.CreateICmpSGT(op0, op1);
                }
                else  if (inid == Intrinsic::tpc_cmp_geq && EnableTPCTransformIntrinCmp) {
                  newins = Builder.CreateICmpSGE(op0, op1);
                }
                else  if (inid == Intrinsic::tpc_shl) {
                  op1 = Builder.CreateZExt(op1, t0);
                  newins = Builder.CreateShl(op0, op1);
                }
                else  if (inid == Intrinsic::tpc_shr) {
                  op1 = Builder.CreateZExt(op1, t0);
                  newins = Builder.CreateLShr(op0, op1);
                }
                else {
                  continue;
                }
              }
              else if (apint == 7) { // short int
                if (inid != Intrinsic::tpc_shr)
                  op0 = Builder.CreateSExt(op0, I32Type);
                op1 = Builder.CreateSExt(op1, I32Type);
                if (inid == Intrinsic::tpc_add) {
                  newins = Builder.CreateNSWAdd(op0, op1);
                  newins = Builder.CreateTrunc(newins, I16Type);
                }
                else  if (inid == Intrinsic::tpc_sub) {
                  newins = Builder.CreateNSWSub(op0, op1);
                  newins = Builder.CreateTrunc(newins, I16Type);
                }
                else  if (inid == Intrinsic::tpc_mul) {
                  if (I32Type != income_type) {
                    continue;
                  }                  
                  newins = Builder.CreateNSWMul(op0, op1);
                }
                else  if (inid == Intrinsic::tpc_and) {
                  newins = Builder.CreateAnd(op0, op1);
                  newins = Builder.CreateTrunc(newins, I16Type);
                }
                else  if (inid == Intrinsic::tpc_or) {
                  newins = Builder.CreateOr(op0, op1);
                  newins = Builder.CreateTrunc(newins, I16Type);
                }
                else  if (inid == Intrinsic::tpc_xor) {
                  newins = Builder.CreateXor(op0, op1);
                  newins = Builder.CreateTrunc(newins, I16Type);
                }
                else  if (inid == Intrinsic::tpc_min && EnableTPCTransformIntrinMinMax) {
                  newins = Builder.CreateSelect(Builder.CreateICmpSLT(op0, op1), op0, op1);
                  newins = Builder.CreateTrunc(newins, I16Type);
                }
                else  if (inid == Intrinsic::tpc_max && EnableTPCTransformIntrinMinMax) {
                  newins = Builder.CreateSelect(Builder.CreateICmpSGT(op0, op1), op0, op1);
                  newins = Builder.CreateTrunc(newins, I16Type);
                }
                else  if (inid == Intrinsic::tpc_cmp_eq && EnableTPCTransformIntrinCmp) {
                  newins = Builder.CreateICmpEQ(op0, op1);
                }
                else  if (inid == Intrinsic::tpc_cmp_neq && EnableTPCTransformIntrinCmp) {
                  newins = Builder.CreateICmpNE(op0, op1);
                }
                else  if (inid == Intrinsic::tpc_cmp_less && EnableTPCTransformIntrinCmp) {
                  newins = Builder.CreateICmpSLT(op0, op1);
                }
                else  if (inid == Intrinsic::tpc_cmp_leq && EnableTPCTransformIntrinCmp) {
                  newins = Builder.CreateICmpSLE(op0, op1);
                }
                else  if (inid == Intrinsic::tpc_cmp_grt && EnableTPCTransformIntrinCmp) {
                  newins = Builder.CreateICmpSGT(op0, op1);
                }
                else  if (inid == Intrinsic::tpc_cmp_geq && EnableTPCTransformIntrinCmp) {
                  newins = Builder.CreateICmpSGE(op0, op1);
                }
                else  if (inid == Intrinsic::tpc_shl) {
                  //continue; //cant process due to any_extend
                  newins = Builder.CreateShl(op0, op1);
                  newins = Builder.CreateTrunc(newins, t0);
                }
                else  if (inid == Intrinsic::tpc_shr) {
                  //op0 = Builder.CreateAnd(op0, 0xFFFF);
                  op0 = Builder.CreateZExt(op0, I32Type);
                  newins = Builder.CreateLShr(op0, op1);
                  newins = Builder.CreateTrunc(newins, t0);
                }
                else {
                  continue;
                }
              }
              else if (apint == 8) { // unsigned short int
                op0 = Builder.CreateZExt(op0, I32Type);
                op1 = Builder.CreateZExt(op1, I32Type);
                if (inid == Intrinsic::tpc_add) {
                  newins = Builder.CreateNSWAdd(op0, op1);
                  newins = Builder.CreateTrunc(newins, I16Type);
                }
                else  if (inid == Intrinsic::tpc_sub) {
                  newins = Builder.CreateNSWSub(op0, op1);
                  newins = Builder.CreateTrunc(newins, I16Type);
                }
                else  if (inid == Intrinsic::tpc_mul) {
                  if (I32Type != income_type) { // for mul may it is type of result
                    continue;              // not supported if is other type
                  }
                  newins = Builder.CreateNSWMul(op0, op1);
                }
                else  if (inid == Intrinsic::tpc_and) {
                  newins = Builder.CreateAnd(op0, op1);
                  newins = Builder.CreateTrunc(newins, I16Type);
                }
                else  if (inid == Intrinsic::tpc_or) {
                  newins = Builder.CreateOr(op0, op1);
                  newins = Builder.CreateTrunc(newins, I16Type);
                }
                else  if (inid == Intrinsic::tpc_xor) {
                  newins = Builder.CreateXor(op0, op1);
                  newins = Builder.CreateTrunc(newins, I16Type);
                }
                else  if (inid == Intrinsic::tpc_min && EnableTPCTransformIntrinMinMax) {
                  newins = Builder.CreateSelect(Builder.CreateICmpULT(op0, op1), op0, op1);
                  newins = Builder.CreateTrunc(newins, I16Type);
                }
                else  if (inid == Intrinsic::tpc_max && EnableTPCTransformIntrinMinMax) {
                  newins = Builder.CreateSelect(Builder.CreateICmpUGT(op0, op1), op0, op1);
                  newins = Builder.CreateTrunc(newins, I16Type);
                }
                else  if (inid == Intrinsic::tpc_cmp_eq && EnableTPCTransformIntrinCmp) {
                  newins = Builder.CreateICmpEQ(op0, op1);
                }
                else  if (inid == Intrinsic::tpc_cmp_neq && EnableTPCTransformIntrinCmp) {
                  newins = Builder.CreateICmpNE(op0, op1);
                }
                else  if (inid == Intrinsic::tpc_cmp_less && EnableTPCTransformIntrinCmp) {
                  newins = Builder.CreateICmpSLT(op0, op1);
                }
                else  if (inid == Intrinsic::tpc_cmp_leq && EnableTPCTransformIntrinCmp) {
                  newins = Builder.CreateICmpSLE(op0, op1);
                }
                else  if (inid == Intrinsic::tpc_cmp_grt && EnableTPCTransformIntrinCmp) {
                  newins = Builder.CreateICmpSGT(op0, op1);
                }
                else  if (inid == Intrinsic::tpc_cmp_geq && EnableTPCTransformIntrinCmp) {
                  newins = Builder.CreateICmpSGE(op0, op1);
                }
                else  if (inid == Intrinsic::tpc_shl) {
                  newins = Builder.CreateShl(op0, op1);
                  newins = Builder.CreateTrunc(newins, t0);
                }
                else  if (inid == Intrinsic::tpc_shr) {
                  newins = Builder.CreateLShr(op0, op1);
                  newins = Builder.CreateTrunc(newins, t0);
                }
                else {
                  continue;
                }
              }
              else if (apint == 4) { // char
                if (inid != Intrinsic::tpc_shr)
                  op0 = Builder.CreateSExt(op0, I32Type);
                op1 = Builder.CreateSExt(op1, I32Type);
                if (inid == Intrinsic::tpc_add) {
                  newins = Builder.CreateNSWAdd(op0, op1);
                  newins = Builder.CreateTrunc(newins, I8Type);
                }
                else  if (inid == Intrinsic::tpc_sub) {
                  newins = Builder.CreateNSWSub(op0, op1);
                  newins = Builder.CreateTrunc(newins, I8Type);
                }
                else  if (inid == Intrinsic::tpc_mul) {
                  if (I32Type != income_type) {
                    continue;
                  }
                  newins = Builder.CreateNSWMul(op0, op1);
                }
                else  if (inid == Intrinsic::tpc_and) {
                  newins = Builder.CreateAnd(op0, op1);
                  newins = Builder.CreateTrunc(newins, I8Type);
                }
                else  if (inid == Intrinsic::tpc_or) {
                  newins = Builder.CreateOr(op0, op1);
                  newins = Builder.CreateTrunc(newins, I8Type);
                }
                else  if (inid == Intrinsic::tpc_xor) {
                  newins = Builder.CreateXor(op0, op1);
                  newins = Builder.CreateTrunc(newins, I8Type);
                }
                else  if (inid == Intrinsic::tpc_min && EnableTPCTransformIntrinMinMax) {
                  newins = Builder.CreateSelect(Builder.CreateICmpSLT(op0, op1), op0, op1);
                  newins = Builder.CreateTrunc(newins, I8Type);
                }
                else  if (inid == Intrinsic::tpc_max && EnableTPCTransformIntrinMinMax) {
                  newins = Builder.CreateSelect(Builder.CreateICmpSGT(op0, op1), op0, op1);
                  newins = Builder.CreateTrunc(newins, I8Type);
                }
                else  if (inid == Intrinsic::tpc_cmp_eq && EnableTPCTransformIntrinCmp) {
                  newins = Builder.CreateICmpEQ(op0, op1);
                }
                else  if (inid == Intrinsic::tpc_cmp_neq && EnableTPCTransformIntrinCmp) {
                  newins = Builder.CreateICmpNE(op0, op1);
                }
                else  if (inid == Intrinsic::tpc_cmp_less && EnableTPCTransformIntrinCmp) {
                  newins = Builder.CreateICmpSLT(op0, op1);
                }
                else  if (inid == Intrinsic::tpc_cmp_leq && EnableTPCTransformIntrinCmp) {
                  newins = Builder.CreateICmpSLE(op0, op1);
                }
                else  if (inid == Intrinsic::tpc_cmp_grt && EnableTPCTransformIntrinCmp) {
                  newins = Builder.CreateICmpSGT(op0, op1);
                }
                else  if (inid == Intrinsic::tpc_cmp_geq && EnableTPCTransformIntrinCmp) {
                  newins = Builder.CreateICmpSGE(op0, op1);
                }
                else  if (inid == Intrinsic::tpc_shl) {
                  newins = Builder.CreateShl(op0, op1);
                  newins = Builder.CreateTrunc(newins, t0);
                }
                else  if (inid == Intrinsic::tpc_shr) {
                  op0 = Builder.CreateZExt(op0, I32Type);
                  newins = Builder.CreateLShr(op0, op1);
                  newins = Builder.CreateTrunc(newins, t0);
                }
                else {
                  continue;
                }
              }
              else if (apint == 5) { //unsigned char
                op0 = Builder.CreateZExt(op0, I32Type);
                op1 = Builder.CreateZExt(op1, I32Type);
                if (inid == Intrinsic::tpc_add) {
                  newins = Builder.CreateNSWAdd(op0, op1);
                  newins = Builder.CreateTrunc(newins, I8Type);
                }
                else  if (inid == Intrinsic::tpc_sub) {
                  newins = Builder.CreateNSWSub(op0, op1);
                  newins = Builder.CreateTrunc(newins, I8Type);
                }
                else  if (inid == Intrinsic::tpc_mul) {
                  if (I32Type != income_type) {
                    continue;
                  }
                  newins = Builder.CreateNSWMul(op0, op1);
                }
                else  if (inid == Intrinsic::tpc_and) {
                  newins = Builder.CreateAnd(op0, op1);
                  newins = Builder.CreateTrunc(newins, I8Type);
                }
                else  if (inid == Intrinsic::tpc_or) {
                  newins = Builder.CreateOr(op0, op1);
                  newins = Builder.CreateTrunc(newins, I8Type);
                }
                else  if (inid == Intrinsic::tpc_xor) {
                  newins = Builder.CreateXor(op0, op1);
                  newins = Builder.CreateTrunc(newins, I8Type);
                }
                else  if (inid == Intrinsic::tpc_min && EnableTPCTransformIntrinMinMax) {
                  newins = Builder.CreateSelect(Builder.CreateICmpULT(op0, op1), op0, op1);
                  newins = Builder.CreateTrunc(newins, I8Type);
                }
                else  if (inid == Intrinsic::tpc_max && EnableTPCTransformIntrinMinMax) {
                  newins = Builder.CreateSelect(Builder.CreateICmpUGT(op0, op1), op0, op1);
                  newins = Builder.CreateTrunc(newins, I8Type);
                }
                else  if (inid == Intrinsic::tpc_cmp_eq && EnableTPCTransformIntrinCmp) {
                  newins = Builder.CreateICmpEQ(op0, op1);
                }
                else  if (inid == Intrinsic::tpc_cmp_neq && EnableTPCTransformIntrinCmp) {
                  newins = Builder.CreateICmpNE(op0, op1);
                }
                else  if (inid == Intrinsic::tpc_cmp_less && EnableTPCTransformIntrinCmp) {
                  newins = Builder.CreateICmpSLT(op0, op1);
                }
                else  if (inid == Intrinsic::tpc_cmp_leq && EnableTPCTransformIntrinCmp) {
                  newins = Builder.CreateICmpSLE(op0, op1);
                }
                else  if (inid == Intrinsic::tpc_cmp_grt && EnableTPCTransformIntrinCmp) {
                  newins = Builder.CreateICmpSGT(op0, op1);
                }
                else  if (inid == Intrinsic::tpc_cmp_geq && EnableTPCTransformIntrinCmp) {
                  newins = Builder.CreateICmpSGE(op0, op1);
                }
                else  if (inid == Intrinsic::tpc_shl) {
                  newins = Builder.CreateShl(op0, op1);
                  newins = Builder.CreateTrunc(newins, t0);
                }
                else  if (inid == Intrinsic::tpc_shr) {
                  newins = Builder.CreateLShr(op0, op1);
                  newins = Builder.CreateTrunc(newins, t0);
                }
                else {
                  continue;
                }
              }
              else {
                continue;
              }
            }
            I.replaceAllUsesWith(newins);
            I.eraseFromParent();
            NumTransformed++;
          }
        }
        break;
        // unary operations
        case Intrinsic::tpc_popcnt:
        //case Intrinsic::tpc_find_first:
        case Intrinsic::tpc_not:
        case Intrinsic::tpc_abs:
        {
          if (no != 7) {
            continue;
          }
          Value* newins;
          auto op5 = intrins->getOperand(5);
          auto op4 = intrins->getOperand(4);
          auto op3 = intrins->getOperand(3);
          auto op2 = intrins->getOperand(2);
          auto op1 = intrins->getOperand(1);
          auto op0 = intrins->getOperand(0);

          Constant* cv = dyn_cast<Constant>(op5);
          if (!cv) { continue; }
          if (cv->getType() != I1Type) {
            continue;
          }
          APInt apint = cv->getUniqueInteger();
          if (apint != 0) {
            // polarity must be zero
            continue;
          }
          // predicate must be true
          cv = dyn_cast<Constant>(op4);
          if (!cv) { continue; }
          if (cv->getType() != I1Type) {
            continue;
          }
          apint = cv->getUniqueInteger();
          if (apint != 1) {
            continue;
          }
          // op3 : need to look type
          Type* income_type = op3->getType();

          // op2: count type for popcnt 
          cv = dyn_cast<Constant>(op2);
          if (!cv) { continue; }
          if (cv->getType() != I32Type) {
            continue;
          }
          apint = cv->getUniqueInteger();  //count type for pop 0,1
          APInt val2 = apint;

          if (op1->getType() != I8Type) continue;
          cv = dyn_cast<Constant>(op1);
          if (!cv) { continue; }
          apint = cv->getUniqueInteger();
          // 2 - int, 3 - unsigned, 7,8-(u)short  4,5-(u)char
          // 0 - float 1-bfloat  6-bool
         // printf("apint=%d\n", apint);
          if (apint != 2 && apint != 3 && apint != 7 && apint != 8 && apint != 4 && apint != 5
            && apint != 0 && apint != 1 && apint != 6
            ) {
            continue;
          }
          {
            Type* t0 = op0->getType();
            Value *ExtF;
            if (inid == Intrinsic::tpc_popcnt) {
              if (income_type != I8Type &&
                !(income_type->isVectorTy() &&
                  cast<VectorType>(income_type)->getNumElements() == 256 &&
                  cast<VectorType>(income_type)->getElementType() == I8Type)
                ) {
                continue;
              }
              VectorType* vt = dyn_cast<VectorType>(t0);
              if (vt && vt->getElementType() != I8Type) {
                // not able transfor <64xi32>,<128xi16> into uchar256
                continue;
              }
              if (apint == 0) {
                op0 = Builder.CreateBitCast(op0, I32Type);
              }
              else if (apint == 1) {
                op0 = Builder.CreateBitCast(op0, I16Type);
              }
              if (val2 != 1) { //ctpop counts only 1
                continue;
              }
              ExtF = Intrinsic::getDeclaration(F->getParent(), Intrinsic::ctpop, op0->getType());
              newins = Builder.CreateCall(ExtF, { op0 });
              newins = Builder.CreateTrunc(newins, income_type);
            }
            else if (inid == Intrinsic::tpc_find_first) {
              if (income_type != I8Type &&
                !(income_type->isVectorTy() &&
                  cast<VectorType>(income_type)->getNumElements() == 256 &&
                  cast<VectorType>(income_type)->getElementType() == I8Type)
                ) {
                continue;
              }
              VectorType* vt = dyn_cast<VectorType>(t0);
              if (vt && vt->getElementType() != I8Type) {
                // not able transfor <64xi32>,<128xi16> into uchar256
                continue;
              }
              if (apint == 0) {
                op0 = Builder.CreateBitCast(op0, I32Type);
              }
              else if (apint == 1) {
                op0 = Builder.CreateBitCast(op0, I16Type);
              }
              if (val2 != 1 && val2 != 3) { //ctlz,cttz counts only 1
                continue;
              }
              // 3 means cttz  
              ExtF = Intrinsic::getDeclaration(F->getParent(),
                (val2 == 3) ? Intrinsic::cttz : Intrinsic::ctlz, op0->getType());
              Value* uv;// = Constant::getNullValue(I1Type);
              // with null value Codegen Prepare transform code into branches
              // and trunc proves separated from ctlz and pattern cant be selected
              uv = Constant::getAllOnesValue(I1Type);
              newins = Builder.CreateCall(ExtF, { op0, uv });
              newins = Builder.CreateTrunc(newins, income_type);
            }
            else if (inid == Intrinsic::tpc_not) {
              if (income_type != t0) {
                continue;
              }
              if (apint == 0) {
                if (t0->isVectorTy()) {
                  op0 = Builder.CreateBitCast(op0, Int64Type);
                }
                else {
                  op0 = Builder.CreateBitCast(op0, I32Type);
                }
                newins = Builder.CreateNot(op0);
                newins = Builder.CreateBitCast(newins, income_type);
              }
              else if (apint == 1) {
                if (t0->isVectorTy()) {
                  op0 = Builder.CreateBitCast(op0, Short128Type);
                }
                else {
                  op0 = Builder.CreateBitCast(op0, I16Type);
                }
                newins = Builder.CreateNot(op0);
                newins = Builder.CreateBitCast(newins, income_type);
              }
              else {
                newins = Builder.CreateNot(op0);
              }
            }
            else if (inid == Intrinsic::tpc_abs) {
              Value *ExtF;
              if (apint == 0) { //float 
                ExtF = Intrinsic::getDeclaration(F->getParent(), Intrinsic::fabs, op0->getType());
                newins = Builder.CreateCall(ExtF, { op0 });
              }
              else if (apint == 1) { //bfloat
                // no llvm intrinsic for this type
                // lets try to cut
                if (t0->isVectorTy()) {
                  continue; //cut will not be effective due to broadcasting constant
                }
                else {
                  newins = Builder.CreateBitCast(op0, I16Type);
                  newins = Builder.CreateAnd(newins, ConstantInt::get(I16Type, 0x7fff));
                  newins = Builder.CreateBitCast(newins, F16Type);
                }
              }
              else { // int types
                if (t0->isVectorTy()) {
                  // vector int ABS is not supported until vector select will be implemented (as MAX/MIN)
                  continue;
                }
                else {
                  Value*zer = ConstantInt::get(I32Type, 0);
                  op0 = Builder.CreateSExt(op0, I32Type);
                  Value*zersub = Builder.CreateSub(zer, op0);
                  Value* icmp = Builder.CreateICmpSLT(op0, zer);
                  newins = Builder.CreateSelect(icmp, zersub, op0);
                  newins = Builder.CreateTrunc(newins, t0);
                }
              }
            }
            /*else if (inid == Intrinsic::tpc_mov) {
                mov intrinsics are not ready yet
            }*/
            else {
              continue;
            }
          }
          I.replaceAllUsesWith(newins);
          I.eraseFromParent();
          NumTransformed++;
        }
        break;
        default: break;
        }
      } //IntrinsicInst
    } //Instruction loop
  } // BB loop
  return NumTransformed > 0;
}
#endif // LLVM_TPC_COMPILER
