//===- TPCCopyElision.cpp -------------------------------------------------===//
//
//===----------------------------------------------------------------------===//
//
// Frontend generates extra copies when it deals with intrinsics that return
// values of structure type. For instance, the piece of IR:
//
//  %tmp = alloca %struct._int256, align 256
//  %252 = bitcast %struct._int256* %tmp to i8*
//  call void @llvm.lifetime.start.p0i8(i64 1024, i8* %252)
//  store %struct._int256 %270, %struct._int256 *%tmp, align 256
//  %271 = bitcast %struct._int256* %acc0 to i8*
//  %272 = bitcast %struct._int256* %tmp to i8*
//  call void @llvm.memcpy.p0i8.p0i8.i32(i8* align 256 %271, i8* align 256 %272, i32 1024, i1 false)
//  %273 = bitcast %struct._int256* %tmp to i8*
//  call void @llvm.lifetime.end.p0i8(i64 1024, i8* %273)
//
// can actually be replaced with:
//
//  store %struct._int256 %270, %struct._int256* %acc0, align 256
//
// Similarly, argument of structure type involves extra copy. For instance, the
// IR piece:
//
//  %tmp10 = alloca %struct._float64_pair_t, align 256
//  %58 = bitcast %struct._float64_pair_t* %tmp10 to i8*
//  %59 = bitcast %struct._float64_pair_t* %result to i8*
//  call void @llvm.memcpy.p0i8.p0i8.i32(i8* align 256 %58, i8* align 256 %59, i32 512, i1 false)
//  %60 = bitcast %struct._float64_pair_t* %tmp10 to <128 x float>*
//  %61 = load <128 x float>, <128 x float>* %60, align 256
//
// can be replaced with:
//
//  %60 = bitcast %struct._float64_pair_t* %result to <128 x float>*
//  %61 = load <128 x float>, <128 x float>* %60, align 256
//
// This pass removes the extra IR nodes in hope that it can help code
// generation.
//
//===----------------------------------------------------------------------===//

#include "TPCTargetMachine.h"
#include "llvm/IR/Function.h"
#include "llvm/IR/Instruction.h"
#include "llvm/IR/InstIterator.h"
#include "llvm/IR/IRBuilder.h"
#include "llvm/IR/Module.h"
#include "llvm/Pass.h"
#include "llvm/PassRegistry.h"
#include "llvm/Support/Debug.h"

#define DEBUG_TYPE "copyelision"

using namespace llvm;

static cl::opt<bool> EnableCopyElision("tpc-copy-elision",
                                       cl::Hidden, cl::ZeroOrMore, cl::init(true));

namespace {
struct TpcCopyElision : public FunctionPass {
  static char ID; // Pass identification, replacement for typeid

  TpcCopyElision() : FunctionPass(ID) {}

  bool runOnFunction(Function &F) override;
};
}


char TpcCopyElision::ID = 0;
INITIALIZE_PASS(TpcCopyElision, "copyelide",
                "Structure copy removal", false, false)

  FunctionPass *llvm::createTpcCopyElision() {
  return new TpcCopyElision();
}

/// Checks if the given value is a temporary that stores function return value
/// of structure type.
/// \param V         The value under test.
/// \param StI       StoreInst that make write to this value. Valid only if the
///                  function returns True.
/// \param TempUsers Array of instructions which should be deleted when the
///                  value is removed. Valid only if the function returns True.
///
static bool isReturnTempValue(Value *V, StoreInst *&StI, SmallVectorImpl<Instruction *> &TempUsers) {
  StI = nullptr;
  TempUsers.clear();

  // Temp value is an alloca that may be used only in:
  // - start lifetime call, like:
  //       %252 = bitcast %struct._int256* %tmp to i8*
  //       call void @llvm.lifetime.start.p0i8(i64 1024, i8* %252)
  //
  // - one store operation, like:
  //       store %struct._int256 %270, %struct._int256* %tmp, align 256
  //   where the stored value (%270) is a known transformation of some
  //   intrinsic call.
  //       %262 = call <256 x i32> @llvm.tpc.mac.x2.zp.v256i32.v256i8.i1(...)
  //       %263 = shufflevector <256 x i32> %262, <256 x i32> undef, <64 x i32> <i32 0, ... i32 63>
  //       %264 = shufflevector <256 x i32> %262, <256 x i32> undef, <64 x i32> <i32 64, ... i32 127>
  //       %265 = shufflevector <256 x i32> %262, <256 x i32> undef, <64 x i32> <i32 128, ... i32 191>
  //       %266 = shufflevector <256 x i32> %262, <256 x i32> undef, <64 x i32> <i32 192, ... i32 255>
  //       %267 = insertvalue %struct._int256 undef, <64 x i32> %263, 0
  //       %268 = insertvalue %struct._int256 %267, <64 x i32> %264, 1
  //       %269 = insertvalue %struct._int256 %268, <64 x i32> %265, 2
  //       %270 = insertvalue %struct._int256 %269, <64 x i32> %266, 3
  //
  // - as a source in one memcpy operation:
  //       %272 = bitcast %struct._int256* %tmp to i8*
  //       call void @llvm.memcpy.p0i8.p0i8.i32(i8* align 256 %271, i8* align 256 %272, i32 1024, i1 false)
  //
  // - end lifetime call:
  //       %273 = bitcast %struct._int256* %tmp to i8*
  //       call void @llvm.lifetime.end.p0i8(i64 1024, i8* %273)

  auto *Inst = dyn_cast<AllocaInst>(V);
  if (!isa<AllocaInst>(V))
    return false;

  bool Seen_memcpy = false;
  bool Seen_store = false;
  for (User *U : Inst->users()) {
    if (auto BC = dyn_cast<BitCastInst>(U)) {
      if (BC->getNumUses() != 1)
        return false;
      auto BCUser = dyn_cast<CallInst>(*BC->user_begin());
      if (!BCUser)
        return false;
      Function *F = BCUser->getCalledFunction();
      if (!F->isIntrinsic())
        return false;
      switch (F->getIntrinsicID()) {
      case Intrinsic::lifetime_start:
      case Intrinsic::lifetime_end:
        TempUsers.push_back(BCUser);
        TempUsers.push_back(BC);
        break;
      case Intrinsic::memcpy:
        if (Seen_memcpy)
          return false;
        Seen_memcpy = true;
        TempUsers.push_back(BCUser);
        TempUsers.push_back(BC);
        if (BCUser->getOperand(1) != BC)
          return false;
        if (auto DPtr = dyn_cast<BitCastInst>(BCUser->getOperand(0)))
          if (DPtr->getNumUses() != 1)
            return false;
          else
            TempUsers.push_back(DPtr);
        else
          return false;
        break;
      default:
        return false;
      }
    } else if (auto St = dyn_cast<StoreInst>(U)) {
      if (Seen_store)
        return false;
      Seen_store = true;
      if (St->getPointerOperand() != Inst)
        return false;
      StI = St;
    } else {
      // This alloca is used in some other way.
      return false;
    }
  }
  TempUsers.push_back(Inst);
  return Seen_memcpy && Seen_store;
}

static bool isArgumentTempValue(Value *V, BitCastInst *&BCI, SmallVectorImpl<Instruction *> &TempUsers) {
  BCI = nullptr;
  TempUsers.clear();

  auto *Inst = dyn_cast<AllocaInst>(V);
  if (!isa<AllocaInst>(V))
    return false;

  bool Seen_memcpy = false;
  bool Seen_load = false;
  for (User *U : Inst->users()) {
    if (auto BC = dyn_cast<BitCastInst>(U)) {
      if (BC->getNumUses() != 1)
        return false;
      if (auto Call = dyn_cast<CallInst>(*BC->user_begin())) {
        Function *F = Call->getCalledFunction();
        if (!F->isIntrinsic())
          return false;
        if (F->getIntrinsicID() != Intrinsic::memcpy)
          return false;
        //  %58 = bitcast %struct._float64_pair_t* %tmp10 to i8*
        //  %59 = bitcast %struct._float64_pair_t* %result to i8*
        //  call void @llvm.memcpy.p0i8.p0i8.i32(i8* align 256 %58, i8* align 256 %59, i32 512, i1 false)
        if (Seen_memcpy)
          return false;
        Seen_memcpy = true;
        TempUsers.push_back(Call);
        TempUsers.push_back(BC);
        if (Call->getOperand(0) != BC)
          return false;
        if (auto DPtr = dyn_cast<BitCastInst>(Call->getOperand(1)))
          if (DPtr->getNumUses() != 1)
            return false;
          else
            TempUsers.push_back(DPtr);
        else
          return false;
      } else if (auto BCUser = dyn_cast<LoadInst>(*BC->user_begin())) {
        //  %60 = bitcast %struct._float64_pair_t* %tmp10 to <128 x float>*
        //  %61 = load <128 x float>, <128 x float>* %60, align 256
        BCI = BC;
        Seen_load = true;
      } else {
        return false;
      }
    } else {
      // This alloca is used in some other way.
      return false;
    }
  }
  TempUsers.push_back(Inst);
  return Seen_memcpy && Seen_load;
}

/// Check if the given memcpy copies a value of structure type.
static bool isOnlyCopy(const MemCpyInst *MemCopy, Value *&Dest, Value *&Src) {
  Value *DestVal = MemCopy->getOperand(0);
  Value *SrcVal = MemCopy->getOperand(1);
  Value *SizeVal = MemCopy->getOperand(2);
  auto Size = dyn_cast<ConstantInt>(SizeVal);
  if (!Size)
    return false;
  auto DestBitCast = dyn_cast<BitCastInst>(DestVal);
  if (!DestBitCast)
    return false;
  auto SrcBitCast = dyn_cast<BitCastInst>(SrcVal);
  if (!SrcBitCast)
    return false;
  Dest = DestBitCast->getOperand(0);
  Src = SrcBitCast->getOperand(0);   // Probably temporary
  Type *DestType = Dest->getType();
  Type *SrcType = Src->getType();
  if (DestType != SrcType)
    return false;
  PointerType *PTy = dyn_cast<PointerType>(DestType);
  if (!PTy)
    return false;
  if (!PTy->getElementType()->isStructTy())
    return false;

  // TODO: check size.
  return true;
}

bool TpcCopyElision::runOnFunction(Function &F) {
  if (!EnableCopyElision)
    return false;

  if (skipFunction(F))
    return false;

  bool Changed = false;

  for (BasicBlock &BB : F) {
    SmallVector<Instruction *, 16> ToRemove;
    for (Instruction &I : BB) {
      if (auto *Call = dyn_cast<MemCpyInst>(&I)) {
        Value *Dest = nullptr;
        Value *Src = nullptr;
        if (isOnlyCopy(Call, Dest, Src)) {
          if (isa<GetElementPtrInst>(Dest))
            continue;
          StoreInst *StI = nullptr;
          BitCastInst *LdI = nullptr;
          SmallVector<Instruction *, 10> TempUsers;
          if (isReturnTempValue(Src, StI, TempUsers)) {
            assert(StI);
            assert(!TempUsers.empty());
            LLVM_DEBUG(dbgs() << "Copy elision: " << *StI << '\n');
            StI->getOperand(1)->replaceAllUsesWith(Dest);
            LLVM_DEBUG(dbgs() << "          To: " << *StI << '\n');
            LLVM_DEBUG(
              for (auto I : TempUsers)
                dbgs() << "       Erase: " << *I << '\n';
            );
            ToRemove.append(TempUsers.begin(), TempUsers.end());
            Changed = true;
          } else if (isArgumentTempValue(Dest, LdI, TempUsers)) {
            assert(LdI);
            assert(!TempUsers.empty());
            LLVM_DEBUG(dbgs() << "Copy elision: " << *LdI << '\n');
            LdI->getOperand(0)->replaceAllUsesWith(Src);
            LLVM_DEBUG(dbgs() << "          To: " << *LdI << '\n');
            LLVM_DEBUG(
              for (auto I : TempUsers)
                dbgs() << "       Erase: " << *I << '\n';
            );
            ToRemove.append(TempUsers.begin(), TempUsers.end());
            Changed = true;
          }
        }
      }
    }

    for (auto I : ToRemove) {
      LLVM_DEBUG(dbgs() << " Remove: " << *I << '\n';);
      I->eraseFromParent();
    }
  }

  return Changed;
}
