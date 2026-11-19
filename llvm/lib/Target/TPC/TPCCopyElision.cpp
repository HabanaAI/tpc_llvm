//===- TPCCopyElision.cpp -------------------------------------------------===//
//
//===----------------------------------------------------------------------===//
//
// Frontend generates extra copies when it deals with intrinsics that return
// values of structure type. For instance, the piece of IR:
//
//  %tmp = alloca %struct._int256, align 256
//  call void @llvm.lifetime.start.p0(i64 1024, ptr %tmp)
//  store %struct._int256 %270, ptr %tmp, align 256
//  call void @llvm.memcpy.p0.p0.i32(ptr align 256 %acc0, ptr align 256 %tmp, i32 1024, i1 false)
//  call void @llvm.lifetime.end.p0(i64 1024, ptr %tmp)
//
// can actually be replaced with:
//
//  store %struct._int256 %270, ptr %acc0, align 256
//
// Similarly, argument of structure type involves extra copy. For instance, the
// IR piece:
//
//  %tmp10 = alloca %struct._float64_pair_t, align 256
//  call void @llvm.memcpy.p0.p0.i32(ptr align 256 %tmp10, ptr align 256 %result, i32 512, i1 false)
//  %61 = load <128 x float>, ptr %tmp10, align 256
//
// can be replaced with:
//
//  %61 = load <128 x float>, ptr %result, align 256
//
// This pass removes the extra IR nodes in hope that it can help code
// generation.
//
// Both rewrites move an access: the first makes the store write the destination
// where it used to write the temporary, before the memcpy; the second makes the
// loads read the source after the memcpy. So the first needs the destination
// untouched between the store and the memcpy, and the second needs the source
// unmodified between the memcpy and the loads. Without that check,
//
//   float128 q = f(); use(p.v1); p = q;
//
// became `p = f(); use(p.v1)` (a value carried across loop iterations read the
// new one).
//
//===----------------------------------------------------------------------===//

#include "TPCTargetMachine.h"
#include "llvm/ADT/SmallPtrSet.h"
#include "llvm/Analysis/AliasAnalysis.h"
#include "llvm/Analysis/MemoryLocation.h"
#include "llvm/IR/Function.h"
#include "llvm/IR/Instruction.h"
#include "llvm/IR/InstIterator.h"
#include "llvm/IR/IRBuilder.h"
#include "llvm/IR/IntrinsicInst.h"
#include "llvm/IR/Module.h"
#include "llvm/Support/Debug.h"

#define DEBUG_TYPE "copyelision"

using namespace llvm;

static cl::opt<bool> EnableCopyElision("tpc-copy-elision",
                                       cl::Hidden, cl::ZeroOrMore, cl::init(true));

namespace {
struct TpcCopyElision {
  static bool runImpl(Function &F, AAResults &AA);
};
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
  //       call void @llvm.lifetime.start.p0(i64 1024, ptr %tmp)
  //
  // - one store operation, like:
  //       store %struct._int256 %270, ptr %tmp, align 256
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
  //       call void @llvm.memcpy.p0.p0.i32(ptr align 256 %acc0, ptr align 256 %tmp, i32 1024, i1 false)
  //
  // - end lifetime call:
  //       call void @llvm.lifetime.end.p0(i64 1024, ptr %tmp)

  auto *Inst = dyn_cast<AllocaInst>(V);
  if (!Inst)
    return false;

  bool Seen_memcpy = false;
  bool Seen_store = false;
  for (User *U : Inst->users()) {
    if (auto *Call = dyn_cast<IntrinsicInst>(U)) {
      switch (Call->getIntrinsicID()) {
      case Intrinsic::lifetime_start:
      case Intrinsic::lifetime_end:
        TempUsers.push_back(Call);
        break;
      case Intrinsic::memcpy:
        if (Seen_memcpy)
          return false;
        Seen_memcpy = true;
        if (Call->getOperand(1) != Inst)
          return false;
        TempUsers.push_back(Call);
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

static bool isArgumentTempValue(Value *V, SmallVectorImpl<LoadInst *> &Loads,
                                SmallVectorImpl<Instruction *> &TempUsers) {
  Loads.clear();
  TempUsers.clear();

  auto *Inst = dyn_cast<AllocaInst>(V);
  if (!Inst)
    return false;

  bool Seen_memcpy = false;
  bool Seen_load = false;
  for (User *U : Inst->users()) {
    if (auto *Call = dyn_cast<IntrinsicInst>(U)) {
      if (Call->getIntrinsicID() != Intrinsic::memcpy)
        return false;
      //  call void @llvm.memcpy.p0.p0.i32(ptr align 256 %tmp10, ptr align 256 %result, i32 512, i1 false)
      if (Seen_memcpy)
        return false;
      Seen_memcpy = true;
      TempUsers.push_back(Call);
      if (Call->getOperand(0) != Inst)
        return false;
    } else if (auto *Load = dyn_cast<LoadInst>(U)) {
      //  %61 = load <128 x float>, ptr %tmp10, align 256
      Loads.push_back(Load);
      Seen_load = true;
    } else {
      // This alloca is used in some other way.
      return false;
    }
  }
  TempUsers.push_back(Inst);
  return Seen_memcpy && Seen_load;
}

/// Check if the given memcpy copies a whole value of structure type, held in an
/// alloca on one of its sides.
static bool isOnlyCopy(const MemCpyInst *MemCopy, Value *&Dest, Value *&Src) {
  auto *Size = dyn_cast<ConstantInt>(MemCopy->getLength());
  if (!Size)
    return false;
  Dest = MemCopy->getRawDest();
  Src = MemCopy->getRawSource();
  if (Dest->getType() != Src->getType())
    return false;

  const DataLayout &DL = MemCopy->getModule()->getDataLayout();
  auto IsWholeStruct = [&](Value *V) {
    auto *AI = dyn_cast<AllocaInst>(V);
    return AI && AI->getAllocatedType()->isStructTy() &&
           DL.getTypeAllocSize(AI->getAllocatedType()) == Size->getZExtValue();
  };
  return IsWholeStruct(Dest) || IsWholeStruct(Src);
}

/// Check if an instruction strictly between \p From and \p To, which are in
/// the same block with \p From first, may modify \p Loc, or with \p AnyAccess
/// may also read it. Instructions in \p Dead are about to be erased.
static bool isAccessedBetween(Instruction *From, Instruction *To,
                              const MemoryLocation &Loc, bool AnyAccess,
                              AAResults &AA,
                              const SmallPtrSetImpl<Instruction *> &Dead) {
  for (auto It = std::next(From->getIterator()); &*It != To; ++It) {
    if (Dead.contains(&*It))
      continue;
    ModRefInfo MR = AA.getModRefInfo(&*It, Loc);
    if (AnyAccess ? isModOrRefSet(MR) : isModSet(MR))
      return true;
  }
  return false;
}

bool TpcCopyElision::runImpl(Function &F, AAResults &AA) {
  bool Changed = false;

  for (BasicBlock &BB : F) {
    SmallVector<Instruction *, 16> ToRemove;
    SmallPtrSet<Instruction *, 16> Dead;
    for (Instruction &I : BB) {
      if (auto *Call = dyn_cast<MemCpyInst>(&I)) {
        Value *Dest = nullptr;
        Value *Src = nullptr;
        if (isOnlyCopy(Call, Dest, Src)) {
          if (isa<GetElementPtrInst>(Dest))
            continue;
          StoreInst *StI = nullptr;
          SmallVector<LoadInst *, 4> Loads;
          SmallVector<Instruction *, 10> TempUsers;
          LocationSize Size = LocationSize::precise(
              cast<ConstantInt>(Call->getLength())->getZExtValue());
          if (isReturnTempValue(Src, StI, TempUsers)) {
            assert(StI);
            assert(!TempUsers.empty());
            // The store will write Dest in place of the temporary, so nothing
            // between it and the memcpy may read or write Dest.
            if (StI->getParent() != &BB || !StI->comesBefore(Call) ||
                isAccessedBetween(StI, Call, MemoryLocation(Dest, Size),
                                  /*AnyAccess=*/true, AA, Dead)) {
              LLVM_DEBUG(dbgs() << "Copy elision: destination accessed before "
                                   "the copy, keeping " << *Call << '\n');
              continue;
            }
            LLVM_DEBUG(dbgs() << "Copy elision: " << *StI << '\n');
            StI->getOperand(1)->replaceAllUsesWith(Dest);
            LLVM_DEBUG(dbgs() << "          To: " << *StI << '\n');
            LLVM_DEBUG(
              for (auto I : TempUsers)
                dbgs() << "       Erase: " << *I << '\n';
            );
            ToRemove.append(TempUsers.begin(), TempUsers.end());
            Dead.insert(TempUsers.begin(), TempUsers.end());
            Changed = true;
          } else if (isArgumentTempValue(Dest, Loads, TempUsers)) {
            assert(!Loads.empty());
            assert(!TempUsers.empty());
            // The loads will read Src in place of the temporary, so nothing
            // between the memcpy and them may modify Src.
            Instruction *Last = Call;
            bool Safe = true;
            for (LoadInst *LdI : Loads) {
              if (LdI->getParent() != &BB || !Call->comesBefore(LdI)) {
                Safe = false;
                break;
              }
              if (Last->comesBefore(LdI))
                Last = LdI;
            }
            if (!Safe || isAccessedBetween(Call, Last, MemoryLocation(Src, Size),
                                           /*AnyAccess=*/false, AA, Dead)) {
              LLVM_DEBUG(dbgs() << "Copy elision: source modified before the "
                                   "loads, keeping " << *Call << '\n');
              continue;
            }
            LLVM_DEBUG(dbgs() << "Copy elision: " << *Loads.front() << '\n');
            Dest->replaceAllUsesWith(Src);
            LLVM_DEBUG(dbgs() << "          To: " << *Loads.front() << '\n');
            LLVM_DEBUG(
              for (auto I : TempUsers)
                dbgs() << "       Erase: " << *I << '\n';
            );
            ToRemove.append(TempUsers.begin(), TempUsers.end());
            Dead.insert(TempUsers.begin(), TempUsers.end());
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

PreservedAnalyses TpcCopyElisionPass::run(Function &F,
                                          FunctionAnalysisManager &AM) {
  if (!EnableCopyElision)
    return PreservedAnalyses::all();

  if (!TpcCopyElision::runImpl(F, AM.getResult<AAManager>(F)))
    return PreservedAnalyses::all();
  return PreservedAnalyses::none();
}
