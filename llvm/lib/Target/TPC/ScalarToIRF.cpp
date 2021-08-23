//===- ScalarToIRF.cpp ----------------------------------------------------===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//
/// This pass replaces constructs like:
///
/// \code
///   %vecext = extractelement <5 x i32> %1, i32 0
///   %add = add nsw i32 %vecext, %src
///   %vecins = insertelement <5 x i32> %1, i32 %add, i32 0
/// \endcode
///
/// with calls to intrinsic:
///
/// \code
///     %2 = call <5 x i32> @llvm.tpc.add.mask.v5i32.i32(<5 x i32> %1,
///               i32 %src, i32 1, i8 2, i32 0, <5 x i32> %1, i1 true, i1 false)
/// \endcode
//===----------------------------------------------------------------------===//

#include "TPCTargetMachine.h"
#include "llvm/IR/Function.h"
#include "llvm/IR/IntrinsicsTPC.h"
#include "llvm/IR/Instruction.h"
#include "llvm/IR/InstIterator.h"
#include "llvm/IR/IRBuilder.h"
#include "llvm/IR/LLVMContext.h"
#include "llvm/IR/Module.h"
#include "llvm/IR/PatternMatch.h"
#include "llvm/Pass.h"
#include "llvm/PassRegistry.h"
#include "llvm/Support/Debug.h"
#include "llvm/Transforms/Scalar.h"
#include <algorithm>
#include <set>

#define DEBUG_TYPE "scalar2irf"

using namespace llvm;
using namespace PatternMatch;

//
// Option to enable/disable Scalar to IRF pass.
//
static cl::opt<bool> EnableScalarToIRF("scalar-to-irf",
    cl::ZeroOrMore, cl::init(true));

static bool FoldComparisons = true;

//
// Returns true if the scalar instruction 'Inst' is an arithmetic instruction,
// which can be transformed to IRF arithmetic.
//
static bool canBeIRF(Instruction *Inst) {
  switch (Inst->getOpcode()) {
  default:
    return false;
  case Instruction::Add:
  case Instruction::Mul:
  case Instruction::Or:
  case Instruction::Xor:
  case Instruction::And:
  case Instruction::Shl:
  case Instruction::LShr:
    return true;
  case Instruction::Sub:
    // We can't transform scalar SUB to IRF SUB because the order of operands is opposite.
    // We can do it by extra subtraction: sub_mask(a,b) => 0 -(b - a)
    // Issue was exposed by GAUDI-1677, test was added
    return true;
  case Instruction::ICmp:
    return FoldComparisons;
  }
  return false;
}

//
// Returns an IRF intrinsic function, which can replace scalar arithmetic instruction.
//
static Function *getIRFIntrinsic(Module *M, unsigned OpCode) {
  IntegerType *Int32Ty = Type::getInt32Ty(M->getContext());
  VectorType *Int5Ty = VectorType::get(Int32Ty, 5);
  // IntegerType *BitTy = Type::getInt1Ty(M->getContext());

  Function *Func = nullptr;
  switch (OpCode) {
  default:
    return nullptr;
  case Instruction::Add:
    Func = Intrinsic::getDeclaration(M, Intrinsic::tpc_add_mask, { Int5Ty, Int32Ty });
    break;
    //  case Instruction::Mul:
    //    Func = Intrinsic::getDeclaration(M, Intrinsic::tpc_mul, { Int5Ty, Int5Ty, BitTy });
    //    break;
  case Instruction::Sub:
    Func = Intrinsic::getDeclaration(M, Intrinsic::tpc_sub_mask, { Int32Ty, Int5Ty });
    break;
  }
  return Func;
}

static Function *getIRFIntrinsicForCmp(Module *M, ICmpInst *Inst) {
  IntegerType *Int32Ty = Type::getInt32Ty(M->getContext());
  VectorType *Int5Ty = VectorType::get(Int32Ty, 5);

  Function *Func = nullptr;
  switch (Inst->getPredicate()) {
  default:
    return nullptr;
  case ICmpInst::Predicate::ICMP_EQ:
    Func = Intrinsic::getDeclaration(M, Intrinsic::tpc_cmp_eq_mask, { Int5Ty, Int5Ty });
    break;
  case ICmpInst::Predicate::ICMP_NE:
    Func = Intrinsic::getDeclaration(M, Intrinsic::tpc_cmp_neq_mask, { Int5Ty, Int5Ty });
    break;
  case ICmpInst::Predicate::ICMP_SLT:
    Func = Intrinsic::getDeclaration(M, Intrinsic::tpc_cmp_less_mask, { Int5Ty, Int5Ty });
    break;
  case ICmpInst::Predicate::ICMP_SLE:
    Func = Intrinsic::getDeclaration(M, Intrinsic::tpc_cmp_leq_mask, { Int5Ty, Int5Ty });
    break;
  case ICmpInst::Predicate::ICMP_SGT:
    Func = Intrinsic::getDeclaration(M, Intrinsic::tpc_cmp_grt_mask, { Int5Ty, Int5Ty });
    break;
  case ICmpInst::Predicate::ICMP_SGE:
    Func = Intrinsic::getDeclaration(M, Intrinsic::tpc_cmp_geq_mask, { Int5Ty, Int5Ty });
    break;
  }
  return Func;
}

static bool isInt5(Value &V) {
  if (auto *VTy = dyn_cast<VectorType>(V.getType()))
    return VTy->getVectorNumElements() == 5 && VTy->getElementType()->isIntegerTy(32);
  return false;
}

static bool areSameInt5Values(Value *V1, Value *V2) {
  if (V1 == V2)
    return true;
  if (auto Ld1 = dyn_cast<LoadInst>(V1))
    if (auto Ld2 = dyn_cast<LoadInst>(V2))
      if (Ld1->getPointerOperand() == Ld2->getPointerOperand())
        return true;
  return false;
}

// Represents piece of source IR that will undergo transformation.
//
class TransformItem {
  Instruction *Extract;
  Instruction *Operation;
  Instruction *InsertOrExtract;

public:

  TransformItem(ExtractElementInst *Ext, Instruction *Op, InsertElementInst *Ins)
  : Extract(Ext), Operation(Op), InsertOrExtract(Ins) {
    assert(areSameInt5Values(Ext->getOperand(0), Ins->getOperand(0)));
    assert(Operation->getOperand(0) == Ext);
    assert(Ext->getOperand(1) == Ins->getOperand(2));
    assert(Ins->getOperand(1) == Op);
  }

  TransformItem(ICmpInst *Cmp, ExtractElementInst *Op1, ExtractElementInst *Op2)
  : Extract(Op1), Operation(Cmp), InsertOrExtract(Op2) {
    assert(Operation->getOperand(0) == Op1);
    assert(Operation->getOperand(1) == Op2);
    assert(Op1->getOperand(1) == Op2->getOperand(1));
  }

  bool isCompare() const {
    return isa<ICmpInst>(Operation);
  }

  unsigned getOpCode() const {
    return Operation->getOpcode();
  }

  Value *getVector() const {
    return Extract->getOperand(0);
  }

  Value *getScalar() const {
    return Operation->getOperand(1);
  }

  unsigned getIndex() const {
    return cast<ConstantInt>(Extract->getOperand(1))->getZExtValue();
  }

  InsertElementInst *getInsert() const {
    return cast<InsertElementInst>(InsertOrExtract);
  }

  ExtractElementInst *getExtract() const {
    return cast<ExtractElementInst>(Extract);
  }

  ExtractElementInst *getExtract2() const {
    return cast<ExtractElementInst>(InsertOrExtract);
  }

  Instruction *getOperation() const {
    return Operation;
  }

  bool erase(Instruction *I) {
    if (I->hasNUses(0)) {
      LLVM_DEBUG(dbgs() << "** Erased: " << *I << " \n");
      I->eraseFromParent();
      return true;
    } else {
      LLVM_DEBUG(dbgs() << "** Cannot erase: " << "(" << I->getNumUses() <<") " << *I << " \n");
      return false;
    }
  }

  bool erase() {
    if (isa<InsertElementInst>(InsertOrExtract))
      return erase(InsertOrExtract) && erase(Operation) && erase(Extract);
    else
      return erase(Operation) && erase(Extract) && erase(InsertOrExtract);
  }

  // To determine if two IRF operations can be merged, we need to determine if
  // they operate on the same vector. This function checks if this item uses the
  // same int5 value as the item 'Base', which must be defined in the IR
  // earlier.
  //
  // Example of correct chain:
  //
  // Item1:
  //   %vecext = extractelement <5 x i32> %1, i32 0
  //   %add = add nsw i32 %vecext, %src
  //   %vecins = insertelement <5 x i32> %1, i32 %add, i32 0
  // Item2:
  //   %vecext2 = extractelement <5 x i32> %vecins, i32 1
  //   %add2 = add nsw i32 %vecext2, %src
  //   %vecins2 = insertelement <5 x i32> %vecins, i32 %add2, i32 1
  // Item3:
  //   %vecext3 = extractelement <5 x i32> %vecins2, i32 2
  //   %add3 = add nsw i32 %vecext3, %src
  //   %vecins3 = insertelement <5 x i32> %vecins2, i32 %add2, i32 2
  //
  // If this function is called:
  // \code
  //     Item3->areInTheSameChainAs(Item1)
  // \endcode
  // it must return true.
  //
  bool isInTheSameChainAs(TransformItem *Base) const {
    // Scan assignment chain looking through InsertElementInst.
    Value *Vector = getVector();
    Value *BaseVector = Base->getVector();
    while (Vector != BaseVector) {
      if (auto Insert = dyn_cast<InsertElementInst>(Vector)) {
        // The source int5 value must have exactly 2 uses, this InsertElementInst
        // and this ExtractElementInst.
        if (!Insert->hasNUses(2))
          return false;
        bool HasExtract = false, HasInsert = false;
        for (auto U : Insert->users())
          if (isa<ExtractElementInst>(U)) {
            if (HasExtract)
              return false;
            HasExtract = true;
          } else if (isa<InsertElementInst>(U)) {
            if (HasInsert)
              return false;
            HasInsert = true;
          } else {
            return false;
          }
        Vector = Insert->getOperand(0);
      } else {
        return false;
      }
    }
    return true;
  }

  void dump() {
#ifndef NDEBUG
    dbgs() << "  Transform   :\n";
    dbgs() << "    Extract   : " << *Extract << "\n";
    dbgs() << "    Operation : " << *Operation << "\n";
    if (isa<InsertElementInst>(InsertOrExtract))
      dbgs() << "    Insert    : ";
    else
      dbgs() << "    Extract   : ";
    dbgs() << *InsertOrExtract << "\n";
#endif
  }
};


// Describes the transformation that need to be done over IR. May contain
// several source regions.
//
struct WorkItem {
  SmallVector<TransformItem *, 4> Items;
  unsigned OpCode = 0;
  unsigned Mask = 0;

  TransformItem *getLast() {
    assert(!Items.empty());
    return Items.back();
  }

  unsigned getOpCode() const {
    assert(!Items.empty());
    return Items.back()->getOpCode();
  }

  Value *getScalarValue() const {
    assert(!Items.empty());
    return Items.back()->getScalar();
  }

  Value *getVectorValue() const {
    assert(!Items.empty());
    return Items.front()->getVector();
  }

  unsigned getMask() const {
    assert(!Items.empty());
    assert(Mask != 0);
    return Mask;
  }

  bool mayAdd(const TransformItem *TI) const {
    return Items.empty() ||
           (Items.back()->getOpCode() == TI->getOpCode()) &&
            Items.back()->getScalar() == TI->getScalar() &&
            TI->isInTheSameChainAs(Items.back());
  }

  void add(TransformItem *TI) {
    assert(mayAdd(TI));
    if (Items.empty()) {
      LLVM_DEBUG(dbgs() << "new item\n");
    } else {
      LLVM_DEBUG(dbgs() << "merged to:\n"; Items.back()->dump());
    }
    Mask |= (1 << TI->getIndex());
    Items.push_back(TI);
  }

  void erase() {
    while (!Items.empty()) {
      TransformItem *TI = Items.pop_back_val();
      TI->erase();
    }
  }

  void dump() {
#ifndef NDEBUG
    dbgs() << "WorkItem, iMask = " << Mask << " :\n";
    for (auto TI : Items)
      TI->dump();
#endif
  }
};


static bool runOnBasicBlock(BasicBlock &BB) {
  bool Changed = false;
  SmallVector<TransformItem, 32> ToReplace;

  const auto findOperation = [&ToReplace](Instruction *Op)->bool {
    for (auto &TI : ToReplace)
      if (TI.getOperation() == Op)
        return true;
    return false;
  };

  for (BasicBlock::iterator I = BB.begin(), E = BB.end(); I != E;) {
    Instruction *Inst = &*I++;
    Value *Vect;
    ConstantInt *Idx;
    if (match(Inst, m_ExtractElement(m_Value(Vect), m_ConstantInt(Idx))) &&
        isInt5(*Vect) &&
        Inst->hasOneUse()) {
      LLVM_DEBUG(dbgs() << "Analyzing: " << *Inst << "\n");
      // Look at the user of this vector element. If it is used in an operation
      // which can be folded into IRF instruction, keep it for subsequent
      // transformation.
      for (auto U : Inst->users())
        if (const auto Operation = dyn_cast<Instruction>(U)) {
          if (canBeIRF(Operation))
            if (auto ICmp = dyn_cast<ICmpInst>(Operation)) {
              if (findOperation(Operation))
                continue;
              // If operation is a compare instruction, check if we have
              // IRF+IRF case.
              Value *Operand1 = Operation->getOperand(0);
              Value *Operand2 = Operation->getOperand(1);
              Value *Other = Inst == Operand1 ? Operand2 : Operand1;
              Value *Vect2;
              ConstantInt *Idx2;
              if (match(Other, m_ExtractElement(m_Value(Vect2),
                                                m_ConstantInt(Idx2))) &&
                  Other->hasOneUse() &&
                  Idx == Idx2) {
                // This is IRF+IRF case.
                ToReplace.emplace_back(
                  TransformItem(cast<ICmpInst>(Operation),
                                cast<ExtractElementInst>(Operand1),
                                cast<ExtractElementInst>(Operand2)));
              }
            } else if (Operation->hasOneUse()) {
              Value *Vect2;
              Value *Elt2;
              ConstantInt *Idx2;
              Value *InsInst = *Operation->user_begin();
              if (match(InsInst,
                        m_InsertElement(m_Value(Vect2),
                                        m_Value(Elt2),
                                        m_ConstantInt(Idx2)))) {
                bool VectorsMatch = areSameInt5Values(Vect2, Vect);
                if (VectorsMatch && Idx2 == Idx) {
                  // This operation can be folded.
                  ToReplace.emplace_back(
                      TransformItem(cast<ExtractElementInst>(Inst),
                                    Operation,
                                    cast<InsertElementInst>(InsInst)));
                }
              }
            }
        }
    }
  }

  // Try to combine items that refer to the same operation on the same vector
  // but for different dimension.
  SmallVector<WorkItem, 32> WorkList;
  for (auto &TI : ToReplace) {
    LLVM_DEBUG(dbgs() << "** Candidate for replacement: \n");
    LLVM_DEBUG(TI.dump());
    bool Merged = false;
    if (!TI.isCompare())
      for (auto &TI2 : WorkList)
        if (TI2.mayAdd(&TI)) {
          TI2.add(&TI);
          Merged = true;
          break;
        }
    if (!Merged) {
      WorkList.emplace_back();
      WorkList.back().add(&TI);
    }
  }

  // Do the replacement.
  Module *M = BB.getParent()->getParent();
  for (auto &WI : WorkList) {
    LLVM_DEBUG(WI.dump());
    if (WI.getOpCode() == Instruction::ICmp) {
      TransformItem *TI = WI.getLast();
      IRBuilder<> Builder(TI->getExtract());
      if (Function *Func = getIRFIntrinsicForCmp(M, cast<ICmpInst>(WI.getLast()->getOperation()))) {
        Value *Replacement = Builder.CreateCall(
          Func,
          { TI->getExtract()->getOperand(0),
            TI->getExtract2()->getOperand(0),
            ConstantInt::get(Type::getInt32Ty(BB.getContext()), WI.Mask),
            ConstantInt::get(Type::getInt8Ty(BB.getContext()), TPCII::OpType::INT32),
            ConstantInt::get(Type::getInt32Ty(BB.getContext()), 0),
            UndefValue::get(Type::getInt1Ty(M->getContext())),
            ConstantInt::get(Type::getInt1Ty(BB.getContext()), 1),
            ConstantInt::get(Type::getInt1Ty(BB.getContext()), 0)
          });
        TI->getOperation()->replaceAllUsesWith(Replacement);
        LLVM_DEBUG(dbgs() << "** Replaced: " << *TI->getOperation() << '\n');
        LLVM_DEBUG(dbgs() << "** with:     " << *Replacement << '\n');
        WI.erase();
        Changed = true;
      }
    } else {
      Instruction *Replaced = cast<Instruction>(WI.getLast()->getInsert());
      unsigned ReplacementOpCode = WI.getOpCode();
      Value *Scalar = WI.getScalarValue();
      Value *Vector = WI.getVectorValue();
      IRBuilder<> Builder(Replaced);
      Instruction *Replacement = nullptr;

      // As arguments of IRF operation go in opposite order (Scalar-Vector), we
      // have to treat subtraction separately.
      if (ReplacementOpCode == Instruction::Sub) {
        // If this is a subtraction of immediate value, try replacing it with
        // addition.
        if (auto CInt = dyn_cast<ConstantInt>(Scalar)) {
          APInt C = CInt->getValue();
          if (!C.isMinSignedValue()) {
            C.negate();
            Scalar = ConstantInt::get(Type::getInt32Ty(BB.getContext()), C);
            ReplacementOpCode = Instruction::Add;
          }
        } else {
          if (Function *Func = getIRFIntrinsic(M, ReplacementOpCode)) {
            Replacement = Builder.CreateCall(
              Func,
              { Scalar,
                WI.getVectorValue(),
                ConstantInt::get(Type::getInt32Ty(BB.getContext()), WI.getMask()),
                ConstantInt::get(Type::getInt8Ty(BB.getContext()), TPCII::OpType::INT32),
                ConstantInt::get(Type::getInt32Ty(BB.getContext()), 0),
                WI.getVectorValue(),
                ConstantInt::get(Type::getInt1Ty(BB.getContext()), 1),
                ConstantInt::get(Type::getInt1Ty(BB.getContext()), 0)
              });
            // When replacing subtraction with IRF operation we swapped arguments.
            // To get correct result we must negate the result.
            Replacement = Builder.CreateCall(
              Func,
              { ConstantInt::get(Type::getInt32Ty(BB.getContext()), 0),
                Replacement,
                ConstantInt::get(Type::getInt32Ty(BB.getContext()), WI.getMask()),
                ConstantInt::get(Type::getInt8Ty(BB.getContext()), TPCII::OpType::INT32),
                ConstantInt::get(Type::getInt32Ty(BB.getContext()), 0),
                Replacement,
                ConstantInt::get(Type::getInt1Ty(BB.getContext()), 1),
                ConstantInt::get(Type::getInt1Ty(BB.getContext()), 0)
              });
          }
        }
      }

      if (!Replacement)
        if (Function *Func = getIRFIntrinsic(M, ReplacementOpCode)) {
          Replacement = Builder.CreateCall(
            Func,
            { Vector,
              Scalar,
              ConstantInt::get(Type::getInt32Ty(BB.getContext()), WI.Mask),
              ConstantInt::get(Type::getInt8Ty(BB.getContext()), TPCII::OpType::INT32),
              ConstantInt::get(Type::getInt32Ty(BB.getContext()), 0),
              Vector,
              ConstantInt::get(Type::getInt1Ty(BB.getContext()), 1),
              ConstantInt::get(Type::getInt1Ty(BB.getContext()), 0)
            });
        }

      if (Replacement) {
        Replaced->replaceAllUsesWith(Replacement);
        LLVM_DEBUG(dbgs() << "** Replaced: " << *Replaced << '\n');
        LLVM_DEBUG(dbgs() << "** with:     " << *Replacement << '\n');
        WI.erase();
        Changed = true;
      }
    }
  }

  return Changed;
}

namespace {
struct ScalarToIRFPass : public FunctionPass {
  static char ID; // Pass identification, replacement for typeid

  ScalarToIRFPass()
    : FunctionPass(ID) {
    initializeScalarToIRFPassPass(*PassRegistry::getPassRegistry());
  }

  bool runOnFunction(Function &F) override {
    if (!EnableScalarToIRF)
      return false;

    if (skipFunction(F))
      return false;

    LLVM_DEBUG(dbgs() << "**** ScalarToIRF Pass\n");
    bool Changed = false;
    for (BasicBlock &BB : F) {
      Changed |= runOnBasicBlock(BB);
    }

    return Changed;
  }

  StringRef getPassName() const override {
    return "Scalar to IRF";
  }
};
}

char ScalarToIRFPass::ID = 0;
INITIALIZE_PASS_BEGIN(ScalarToIRFPass, "scalar2irf",
                      "Scalar to IRF", false, false)
INITIALIZE_PASS_END(ScalarToIRFPass, "scalar2irf",
                    "Scalar to IRF", false, false)

FunctionPass *llvm::createScalarToIRFPass(bool FoldCmp) {
  FoldComparisons = FoldCmp;
  return new ScalarToIRFPass();
}
