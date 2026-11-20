//===- TPCCarryPairs.cpp - Keep each carry read with the add before it ----===//
//
// An ADD with the CARRY switch (a SUB with BORROW) reads the carry written by
// the integer ADD or SUB executed before it on the same unit; a 64-bit add is
// a low add, then a high add with CARRY. The carry is not a value in the IR,
// so nothing links the two calls, and every pass that removes, merges or
// moves an add can leave the high add with the carry of another add: the low
// add is removed when only the carry is used, merged with an equal add
// earlier in the block, folded (x + x into a shift, x + 0 into x, an add of
// disjoint bits into an or) or hoisted out of a loop; two equal high adds are
// merged into one. SelectionDAG does the first and the last even at -O0.
//
// This pass replaces each carry-reading tpc_add or tpc_sub and the integer
// add or sub just before it on the same unit, in the same block, with one
// call of tpc_carry_pair, which the passes after it see as one pure
// operation. It is lowered to the two instructions glued together
// (TPCTargetLowering::lowerTPC_CarryPair), and TPCCarryDeps keeps them in
// order after instruction selection.
//
// It runs first, at every optimization level, on the IR as the front end
// emits it, where a result is stored to its variable right after it is
// computed. The store of the low result is moved after the pair so that the
// low add is not computed twice.
//
// Not joined, and so as before: a carry read with no add before it in the
// block, the third and later adds of a longer chain (the second add reads
// a carry), and adds of other types than i32 and i16.
//
//===----------------------------------------------------------------------===//

#include "MCTargetDesc/InstructionDB.h"
#include "TPCTargetMachine.h"
#include "llvm/ADT/Statistic.h"
#include "llvm/Analysis/ValueTracking.h"
#include "llvm/IR/IRBuilder.h"
#include "llvm/IR/Instructions.h"
#include "llvm/IR/IntrinsicInst.h"
#include "llvm/IR/IntrinsicsTPC.h"
#include "llvm/Support/Debug.h"

using namespace llvm;

#define DEBUG_TYPE "tpc-carry-pairs"

STATISTIC(NumPairs, "Number of carry reads joined with their add");

namespace {

enum Unit { SPU = 0, VPU = 1 };

/// What an instruction does to the carry of one unit.
struct CarryOp {
  bool Writes = false;      // May write the carry of unit U.
  bool AllUnits = false;    // Unknown: may write the carry of both units.
  Unit U = SPU;
  bool IsSub = false;
  bool Reads = false;       // Reads the carry (CARRY or BORROW).
  bool Joinable = false;    // May be a half of tpc_carry_pair.
  Value *Ops[7] = {};       // X, Y, DataType, Switches, Income, Pred, Polarity.
};

bool isJoinableType(Type *T) {
  if (auto *VT = dyn_cast<FixedVectorType>(T))
    return (VT->getNumElements() == 64 && VT->getElementType()->isIntegerTy(32)) ||
           (VT->getNumElements() == 128 && VT->getElementType()->isIntegerTy(16));
  return T->isIntegerTy(32) || T->isIntegerTy(16);
}

// int5 operations run on the SPU (IRF), as do scalar ones.
Unit unitOf(Type *T) {
  auto *VT = dyn_cast<FixedVectorType>(T);
  return VT && VT->getNumElements() != 5 ? VPU : SPU;
}

CarryOp classify(Instruction &I) {
  CarryOp C;
  if (auto *BO = dyn_cast<BinaryOperator>(&I)) {
    if ((BO->getOpcode() != Instruction::Add &&
         BO->getOpcode() != Instruction::Sub) ||
        !BO->getType()->isIntOrIntVectorTy())
      return C;
    Type *T = BO->getType();
    C.Writes = true;
    C.U = unitOf(T);
    C.IsSub = BO->getOpcode() == Instruction::Sub;
    C.Joinable = isJoinableType(T);
    if (!C.Joinable)
      return C;
    LLVMContext &Ctx = I.getContext();
    Value *X = BO->getOperand(0), *Y = BO->getOperand(1);
    // The instructions take a constant or a scalar only as the second
    // operand.
    if (!C.IsSub && isa<Constant>(X) && !isa<Constant>(Y))
      std::swap(X, Y);
    bool Is32 = T->getScalarType()->isIntegerTy(32);
    C.Ops[0] = X;
    C.Ops[1] = Y;
    C.Ops[2] = ConstantInt::get(Type::getInt8Ty(Ctx),
                                Is32 ? TPCII::OpType::INT32 : TPCII::OpType::INT16);
    C.Ops[3] = ConstantInt::get(Type::getInt32Ty(Ctx), 0);
    C.Ops[4] = UndefValue::get(T);
    C.Ops[5] = ConstantInt::getTrue(Ctx);
    C.Ops[6] = ConstantInt::getFalse(Ctx);
    return C;
  }

  if (auto *GEP = dyn_cast<GetElementPtrInst>(&I)) {
    // Address arithmetic may be an SPU ADD.
    if (!GEP->hasAllConstantIndices()) {
      C.Writes = true;
      C.U = SPU;
    }
    return C;
  }

  auto *CB = dyn_cast<CallBase>(&I);
  if (!CB)
    return C;
  Function *F = CB->getCalledFunction();
  if (!F || !F->isIntrinsic()) {
    // A call or inline assembly: anything may happen.
    C.Writes = C.AllUnits = true;
    return C;
  }
  switch (F->getIntrinsicID()) {
  case Intrinsic::tpc_add_mask:
  case Intrinsic::tpc_sub_mask:
    C.Writes = true;
    C.U = SPU;
    return C;
  case Intrinsic::tpc_carry_pair:
    // Already joined (this pass ran before); its second half writes.
    C.Writes = true;
    C.U = unitOf(CB->getType()->getStructElementType(1));
    return C;
  case Intrinsic::tpc_add:
  case Intrinsic::tpc_sub:
    break;
  default:
    return C;
  }
  Type *T = CB->getType();
  if (!T->isIntOrIntVectorTy())
    return C; // Floating-point adds do not write the carry.
  C.Writes = true;
  C.U = unitOf(T);
  C.IsSub = F->getIntrinsicID() == Intrinsic::tpc_sub;
  auto *Sw = dyn_cast<ConstantInt>(CB->getArgOperand(3));
  if (!Sw) {
    C.Reads = true; // Cannot tell; never join it.
    return C;
  }
  unsigned S = Sw->getZExtValue();
  C.Reads = S & (C.IsSub ? TPCII::SW_BORROW : TPCII::SW_CARRY);
  // With NO_CARRY_GEN, gaudi writes the carry and gaudi2 does not: leave
  // such an add alone.
  C.Joinable = isJoinableType(T) && CB->getArgOperand(0)->getType() == T &&
               !(S & TPCII::SW_NO_CARRY_GEN);
  for (unsigned I = 0; I < 7; ++I)
    C.Ops[I] = CB->getArgOperand(I);
  return C;
}

// Whether a store of the low result to its variable can be moved down to
// just before Pos: nothing in between may touch that variable.
bool canSinkStore(StoreInst *SI, Instruction *Pos) {
  if (!SI->isSimple())
    return false;
  auto *Var = dyn_cast<AllocaInst>(getUnderlyingObject(SI->getPointerOperand()));
  if (!Var)
    return false;
  for (Instruction *I = SI->getNextNode(); I != Pos; I = I->getNextNode()) {
    if (!I->mayReadOrWriteMemory())
      continue;
    Value *Ptr = nullptr;
    if (auto *LI = dyn_cast<LoadInst>(I))
      Ptr = LI->getPointerOperand();
    else if (auto *S = dyn_cast<StoreInst>(I))
      Ptr = S->getPointerOperand();
    else if (I->isLifetimeStartOrEnd())
      Ptr = cast<IntrinsicInst>(I)->getArgOperand(
          cast<IntrinsicInst>(I)->arg_size() - 1);
    if (!Ptr)
      return false;
    auto *Other = dyn_cast<AllocaInst>(getUnderlyingObject(Ptr));
    if (!Other || Other == Var)
      return false;
  }
  return true;
}

void join(Instruction *Lo, Instruction *Hi) {
  LLVM_DEBUG(dbgs() << "Joining\n  " << *Lo << "\n  " << *Hi << "\n");
  // Classified again: an operand may be the result of a pair joined before.
  CarryOp L = classify(*Lo), H = classify(*Hi);
  IRBuilder<> B(Hi);
  SmallVector<Value *, 15> Args;
  Args.push_back(B.getInt32((L.IsSub ? 1 : 0) | (H.IsSub ? 2 : 0)));
  Args.append(std::begin(L.Ops), std::end(L.Ops));
  Args.append(std::begin(H.Ops), std::end(H.Ops));
  Type *Tys[] = {Lo->getType(), Hi->getType(),
                 L.Ops[0]->getType(), L.Ops[1]->getType(), L.Ops[5]->getType(),
                 H.Ops[0]->getType(), H.Ops[1]->getType(), H.Ops[5]->getType()};
  Function *Decl = Intrinsic::getOrInsertDeclaration(
      Hi->getModule(), Intrinsic::tpc_carry_pair, Tys);
  CallInst *Pair = B.CreateCall(Decl, Args, "carry.pair");
  Value *LoVal = B.CreateExtractValue(Pair, 0);
  Value *HiVal = B.CreateExtractValue(Pair, 1);
  HiVal->takeName(Hi);
  Hi->replaceAllUsesWith(HiVal);
  Hi->eraseFromParent();

  // Uses of the low result after the pair take the pair's; a store of it
  // between the two moves after the pair.
  Instruction *After = cast<Instruction>(HiVal)->getNextNode();
  for (Use &U : make_early_inc_range(Lo->uses())) {
    auto *User = cast<Instruction>(U.getUser());
    if (User == Pair)
      continue;
    if (isa<PHINode>(User) || User->getParent() != Pair->getParent() ||
        Pair->comesBefore(User)) {
      U.set(LoVal);
      continue;
    }
    auto *SI = dyn_cast<StoreInst>(User);
    if (SI && SI->getValueOperand() == Lo && canSinkStore(SI, Pair)) {
      SI->moveBefore(After->getIterator());
      U.set(LoVal);
    }
  }
  if (Lo->use_empty()) {
    LoVal->takeName(Lo);
    Lo->eraseFromParent();
  }
  ++NumPairs;
}

bool runOnBlock(BasicBlock &BB) {
  struct Last {
    Instruction *I = nullptr;
    CarryOp C;
  } LastWrite[2];
  SmallVector<std::pair<Instruction *, Instruction *>, 4> Pairs;
  for (Instruction &I : BB) {
    CarryOp C = classify(I);
    if (!C.Writes)
      continue;
    if (C.AllUnits) {
      LastWrite[SPU] = LastWrite[VPU] = Last{&I, C};
      continue;
    }
    Last &Prev = LastWrite[C.U];
    if (C.Reads && C.Joinable && Prev.I && Prev.C.Joinable && !Prev.C.Reads &&
        !Prev.C.AllUnits)
      Pairs.push_back({Prev.I, &I});
    Prev = Last{&I, C};
  }
  for (auto &P : Pairs)
    join(P.first, P.second);
  return !Pairs.empty();
}

} // namespace

PreservedAnalyses TPCCarryPairsPass::run(Function &F,
                                         FunctionAnalysisManager &AM) {
  bool Changed = false;
  for (BasicBlock &BB : F)
    Changed |= runOnBlock(BB);
  if (!Changed)
    return PreservedAnalyses::all();
  PreservedAnalyses PA;
  PA.preserveSet<CFGAnalyses>();
  return PA;
}
