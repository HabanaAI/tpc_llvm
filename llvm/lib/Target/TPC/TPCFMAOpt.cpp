//===- TPCFMAOpt.cpp ------------------------------------------------------===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//
// This pass
// transforms a*b + c => ..mac..(a,b,c,0)
//            a*b - c => ..mac..(a,b,c,1)
//===----------------------------------------------------------------------===//
#ifdef LLVM_TPC_COMPILER

#include "TPCTargetMachine.h"
#include "llvm/IR/Function.h"
#include "llvm/IR/Instruction.h"
#include "llvm/IR/InstIterator.h"
#include "llvm/IR/IntrinsicInst.h"
#include "llvm/IR/Intrinsics.h"
#include "llvm/IR/IntrinsicsTPC.h"
#include "llvm/IR/IRBuilder.h"
#include "llvm/IR/Module.h"
#include "llvm/IR/LLVMContext.h"
#include "llvm/Pass.h"
#include "llvm/PassRegistry.h"

#define DEBUG_TYPE "TPCFMAopt"

using namespace llvm;

namespace llvm {
  FunctionPass *createTPCFMAoptPass();
  void initializeTPCFMAoptPass(PassRegistry&);
}

static const char PassDescription[] = "TPC FMA optimization";
static const char PassName[] = "tpc-fma-opt";

static cl::opt<bool>
EnableTPCFMAopt(PassName,
  cl::Hidden,
  cl::init(false));


namespace {
  class TPCFMAopt : public FunctionPass {
    Function *F = nullptr;
    unsigned NumTransformed = 0;
  public:
    static char ID;

    StringRef getPassName() const override { return PassDescription; }

    TPCFMAopt() : FunctionPass(ID) {
      initializeTPCFMAoptPass(*PassRegistry::getPassRegistry());
    }
    bool runOnFunction(Function &F) override;
  };//class
}//namespace

char TPCFMAopt::ID = 0;
INITIALIZE_PASS(TPCFMAopt, PassName, PassDescription, false, false)

FunctionPass *llvm::createTPCFMAoptPass() {
  return new TPCFMAopt();
}

bool TPCFMAopt::runOnFunction(Function &Func) {

  if (!EnableTPCFMAopt) {
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
  Type *HalfType = Type::getHalfTy(Ctx);
  Type *BF16Type = Type::getBFloat16Ty(Ctx);
  VectorType* Int64Type = VectorType::get(I32Type, 64);
  VectorType* Float64Type = VectorType::get(F32Type, 64);
  VectorType* Short128Type = VectorType::get(I16Type, 128);
  VectorType* Bfloat128Type = VectorType::get(BF16Type, 128);
  VectorType* Char256Type = VectorType::get(I8Type, 256);
  VectorType* Int5Type = VectorType::get(I32Type, 5);


  for (auto BBIt = Func.begin(), BBEnd = Func.end(); BBIt != BBEnd;) {
    BasicBlock &BB = *BBIt;
    ++BBIt;
    SmallVector<Instruction *, 16> MulList;
    for (auto It = BB.begin(), E = BB.end(); It != E;) {
      Instruction &I = *It;
      ++It;
      /////////////////////// Insert here //////////////////
      if (I.getOpcode() == Instruction::Mul || I.getOpcode() == Instruction::FMul) {
        MulList.push_back(&I);
      }
    } //Instruction loop
    // this opt better to do for each BB, otherwise would not be optimal
    //------------------------------------------------------
    for (auto muli : MulList) {
      Type *itp = muli->getType();
      VectorType *vt = dyn_cast<VectorType>(itp);
      if (vt) { // not yet
        continue;
      }
      Value *opnd0 = muli->getOperand(0);
      Value *opnd1 = muli->getOperand(1);
#define SIGNED 1
#define UNSIGN 2
      int sun0 = 0, sun1 = 0; // signed/unsigned 1,2
      if (auto sex = dyn_cast<SExtInst>(opnd0)) {
        opnd0 = sex->getOperand(0);
        sun0 = SIGNED;
      } else if (auto zex = dyn_cast<ZExtInst>(opnd0)) {
        opnd0 = zex->getOperand(0);
        sun0 = UNSIGN;
      }
      if (auto sex = dyn_cast<SExtInst>(opnd1)) {
        opnd1 = sex->getOperand(0);
        sun1 = SIGNED;
      } else if (auto zex = dyn_cast<ZExtInst>(opnd1)) {
        opnd1 = zex->getOperand(0);
        sun1 = UNSIGN;
      }
      assert(sun0 == sun1);
      Type *opnd_type = opnd1->getType();
      SmallVector<Instruction *, 4> UserList;
      UserList.clear();
      for (auto Usr : muli->users()) {
        Instruction *usi = dyn_cast<Instruction>(Usr);
        if (usi) {
          UserList.push_back(usi);
        }
      }

      for (auto Usr : UserList) { // may be better to have only 1 user (
                                  // otherwise regs will be duplicated
        Instruction *usi = dyn_cast<Instruction>(Usr);
        if (usi) {
          bool intop = (usi->getOpcode() == Instruction::Add ||
                        usi->getOpcode() == Instruction::Sub);

          bool floatop = (usi->getOpcode() == Instruction::FAdd ||
                          usi->getOpcode() == Instruction::FSub);

          IRBuilder<> Builder(usi);
          Value *ExtF;
          Type *mactype = (intop) ? I32Type : F32Type;
          unsigned swity = 0;
          unsigned swval = 0;
          if (intop) {
            // in that case we can replace usi by MAC
            if (opnd_type == I16Type) {
              swity = 7;
            } else if (opnd_type == I8Type) {
              swity = 4;
            }
          } else if (floatop) {
            if (opnd_type == F32Type) {
              swity = 0;
            } else if (opnd_type == BF16Type) {
              swity = 1;
              mactype = BF16Type;
            } else if (opnd_type == HalfType) {
              swity = 11;
              mactype = HalfType;
            }
            // FP8: no  mac intrin with FP8
          } else {
            llvm_unreachable("unexpected opearion");
          }
          ExtF = Intrinsic::getDeclaration(F->getParent(), Intrinsic::tpc_mac,
                                           {mactype, opnd_type, I1Type});
          Value *acc_opnd;
          Value *us_opnd0 = usi->getOperand(0);
          Instruction *usi0 = dyn_cast<Instruction>(us_opnd0);
          Value *us_opnd1 = usi->getOperand(1);
          acc_opnd = us_opnd1;
          if (usi->getOpcode() == Instruction::Sub) {
            if (usi0 && usi0->getOpcode() == Instruction::Mul) {
              acc_opnd =
                  Builder.CreateSub(ConstantInt::get(mactype, 0), us_opnd1);
            } else
              continue;
          }
          if (usi->getOpcode() == Instruction::FSub) {
            if (usi0 && usi0->getOpcode() == Instruction::FMul) {
              acc_opnd =
                  Builder.CreateFSub(ConstantFP::get(mactype, 0.0), us_opnd1);
            } else {
              swval = 1 << 1;
              acc_opnd = us_opnd0;
            }
          }
          Value *NewIns = Builder.CreateCall(
              ExtF, {opnd0, opnd1, ConstantInt::get(I8Type, swity), // op_type
                     ConstantInt::get(I32Type, swval),              // switch
                     acc_opnd, ConstantInt::get(I1Type, 1),
                     ConstantInt::get(I1Type, 0)});
          usi->replaceAllUsesWith(NewIns);
          usi->eraseFromParent();
          NumTransformed++;
        }
      }
    }
  } // BB loop
  return NumTransformed > 0;
}

#endif // LLVM_TPC_COMPILER
