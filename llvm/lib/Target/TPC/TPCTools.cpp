//===- TPCTools.cpp --- makes Global Variables Locals ---------------------===//
//
//===----------------------------------------------------------------------===//
//
// TPC related support functions for manipulating IR.
//
//===----------------------------------------------------------------------===//

#include "TPCTools.h"
#include "llvm/IR/GlobalVariable.h"
#include "llvm/IR/IntrinsicInst.h"
#include "llvm/IR/Instructions.h"
#include "llvm/IR/DerivedTypes.h"
#include "llvm/IR/IRBuilder.h"
#include "llvm/IR/Operator.h"
#include "llvm/IR/Value.h"
#include "llvm/IR/ValueHandle.h"
#include "llvm/Support/Casting.h"
#include "llvm/Support/Debug.h"
#include "llvm/Support/raw_ostream.h"

#include "../lib/IR/ConstantsContext.h"

namespace llvm {

bool isTpcVectorType(const Type *Ty) {
  if (const auto *ST = dyn_cast<StructType>(Ty)) {
    for (const Type *ET : ST->elements()) {
      if (!isTpcVectorType(ET))
        return false;
    }
    return true;
  }
  if (const auto *AT = dyn_cast<ArrayType>(Ty)) {
    return isTpcVectorType(AT->getElementType());
  }
  if (const auto *VT = dyn_cast<VectorType>(Ty)) {
    const Type *ET = VT->getElementType();
    unsigned Sz = cast<FixedVectorType>(VT)->getNumElements();
    if (ET->isFloatTy())
      return Sz == 64 || Sz == 128;
    if (ET->isBFloatTy())
      return Sz == 128 || Sz == 256;
    if (ET->isHalfTy())
      return Sz == 128 || Sz == 256;
    if (ET->isF8_152Ty() || ET->isF8_143Ty())
      return Sz == 256 || Sz == 512;
    if (const auto *IT = dyn_cast<IntegerType>(ET)) {
      if (IT->getBitWidth() == 32)
        return Sz == 64 || Sz == 128 || Sz == 256;
      if (IT->getBitWidth() == 16)
        return Sz == 128 || Sz == 256;
      if (IT->getBitWidth() == 8)
        return Sz == 256 || Sz == 512;
      if (IT->getBitWidth() == 1)
        return Sz == 256;
    }
    llvm_unreachable("Unsupported vector type");
  }
  return false;
}


bool isVolatile(const Instruction *Inst) {
  if (auto *LI = dyn_cast<LoadInst>(Inst))
    return LI->isVolatile();
  else if (auto *SI = dyn_cast<StoreInst>(Inst))
    return SI->isVolatile();
  else if (auto *AI = dyn_cast<AtomicCmpXchgInst>(Inst))
    return AI->isVolatile();
  return false;
}



static bool hasVolatileAccess(const Instruction *Inst) {
  if (isVolatile(Inst))
    return true;
  if (auto *GEP = dyn_cast<GetElementPtrInst>(Inst))
    for (auto Usr : GEP->users())
      if (auto I = dyn_cast<Instruction>(Usr))
        if (hasVolatileAccess(I))
          return true;
  return false;
}

bool isVolatileVariable(const GlobalVariable &V) {
  for (auto U : V.users())
    if (auto *I = dyn_cast<Instruction>(U))
      if (hasVolatileAccess(I))
        return true;
  return false;
}


/// Checks if the global value pointer is used in some complex way.
///
bool isAddressTaken(const Value *V) {
  if (!V->getType()->isPointerTy())
    return false;

  for (const Use &U : V->uses()) {
    User *I = U.getUser();
    if (isa<LoadInst>(I)) {
      // It is OK to use pointer for loading.
    } else if (StoreInst *SI = dyn_cast<StoreInst>(I)) {
      if (V != SI->getOperand(1))
        return true; // Storing the pointer
    } else if (Operator::getOpcode(I) == Instruction::GetElementPtr) {
      if (isAddressTaken(I))
        return true;
    } else if (Operator::getOpcode(I) == Instruction::BitCast) {
      if (isAddressTaken(I))
        return true;
    } else if (ICmpInst *ICI = dyn_cast<ICmpInst>(I)) {
      if (!isa<ConstantPointerNull>(ICI->getOperand(1)))
        return true; // Allow comparison against null.
    } else if (Constant *C = dyn_cast<Constant>(I)) {
      // Ignore constants which don't have any live uses.
      if (isa<GlobalValue>(C) || C->isConstantUsed())
        return true;
    }
  }

  return false;
}


Constant *replaceInConstantExpr(Constant *E, Constant *Old, Constant *New,
                                unsigned NewAddressSpace) {
  if (auto *GEP = dyn_cast<GetElementPtrConstantExpr>(E)) {
    assert(GEP->getOperand(0) == Old);
    PointerType *OT = cast<PointerType>(GEP->getType());
    SmallVector<Constant *, 8> Args;
    for (unsigned I = 1; I < GEP->getNumOperands(); ++I)
      Args.push_back(cast<Constant>(GEP->getOperand(I)));
    return ConstantExpr::getGetElementPtr(OT->getElementType(), New, Args);
  }

  if (auto *UCE = dyn_cast<UnaryConstantExpr>(E)) {
    assert(Instruction::isCast(UCE->getOpcode()));
    assert(UCE->getOperand(0) == Old);
    PointerType *OT = cast<PointerType>(UCE->getType());
    PointerType *NT = PointerType::get(OT->getElementType(), NewAddressSpace);
    return ConstantExpr::getCast(UCE->getOpcode(), New, NT);
  }

  llvm_unreachable("Unsupported constant expression");
}


Value *replaceConstantByMutable(Constant *Usr, Constant *Old, Value *New,
                                unsigned NewAddressSpace, IRBuilder<> &Builder) {
  // User value must have pointer type if it can change address space.
  PointerType *OT = cast<PointerType>(Usr->getType());
  assert(OT->getAddressSpace() != NewAddressSpace);
  PointerType *NT = PointerType::get(OT->getElementType(), NewAddressSpace);

  if (auto C = dyn_cast<GetElementPtrConstantExpr>(Usr)) {
    SmallVector<Value *, 8> IndexVals;
    for (unsigned I = 1; I < C->getNumOperands(); ++I) {
      Value *Arg = C->getOperand(I);
      if (Arg == Old)
        Arg = New;
      IndexVals.push_back(Arg);
    }
    Value *NewBase = (C->getOperand(0) == Old) ? New : C->getOperand(0);
    return Builder.CreateInBoundsGEP(C->getSourceElementType(), NewBase, IndexVals);
  }

  if (auto C = dyn_cast<UnaryConstantExpr>(Usr)) {
    assert(Usr->getOperand(0) == Old);
    switch (C->getOpcode()) {
    case Instruction::BitCast:
      return Builder.CreateBitCast(New, NT);
    default:
      llvm_unreachable("Unhandled unary constant expression");
    }
  }

  llvm_unreachable("Unhandled constant");
}

void rauwConvertingAddressSpace(Value *Old, Value *New, IRBuilder<> *Builder,
                                SmallPtrSetImpl<Instruction *> *DeletedInstrs) {
  DEBUG_WITH_TYPE("localizer", dbgs() << "== Replacing: " << *Old << "\n");
  DEBUG_WITH_TYPE("localizer", dbgs() << "== With     : " << *New << "\n");

  assert(Old != New);
  if (DeletedInstrs)
    if (auto *I = dyn_cast<Instruction>(Old))
      DeletedInstrs->insert(I);

  // Check if we make the replacement that changes address space of the value.
  bool AddressSpaceChanged = false;
  unsigned NewAddressSpace = 0;
  if (auto *OPT = dyn_cast<PointerType>(Old->getType())) {
    auto *NPT = cast<PointerType>(New->getType());
    NewAddressSpace = NPT->getAddressSpace();
    if (OPT->getAddressSpace() != NewAddressSpace)
      AddressSpaceChanged = true;
  }

  // If address space is not changed, use usual RAUW procedure.
  if (!AddressSpaceChanged) {
    Old->replaceAllUsesWith(New);
    return;
  }

  // Notify all ValueHandles (if present) that this value is going away.
  if (Old->hasValueHandle())
    ValueHandleBase::ValueIsRAUWd(Old, New);

  // Scan all uses of 'Old' and replace in them 'Old' for 'New'. This
  // replacement can cause replacement of the user and users of the user etc.
  // Changes may propargate recursively.
  while (!Old->use_empty()) {
    Use &U = *Old->use_begin();
    User *Usr = U.getUser();

    DEBUG_WITH_TYPE("localizer", dbgs() << "== Transform:\n");
    DEBUG_WITH_TYPE("localizer", dbgs() << "   " << *Usr << "\n");

    // If a user itself has no uses, try eliminating it.
    if (Usr->getNumUses() == 0) {
      if (auto I = dyn_cast<Instruction>(Usr)) {
        if (!I->mayHaveSideEffects()) {
          U.set(nullptr);
          if (DeletedInstrs)
            DeletedInstrs->insert(I);
          DEBUG_WITH_TYPE("localizer", dbgs() << "   Removed\n");
          continue;
        }
      } else {
        U.set(nullptr);
        DEBUG_WITH_TYPE("localizer", dbgs() << "   Removed\n");
        continue;
      }
    }

    // Must handle 'Constant' specially, we cannot call replaceAllUsesWith on it
    // because constants are uniqued.
    if (auto *C = dyn_cast<Constant>(Usr)) {
      assert(isa<Constant>(Old) && "Constant may use only constants");
      if (auto *NewC = dyn_cast<Constant>(New)) {
        // Replace a constant by a constant.
        if (!isa<GlobalValue>(C)) {
          Constant *Replacement = replaceInConstantExpr(C, cast<Constant>(Old),
                                                        NewC, NewAddressSpace);
          DEBUG_WITH_TYPE("localizer", dbgs() << "== Replaced (Constant with Constant):\n");
          DEBUG_WITH_TYPE("localizer", dbgs() << " " << *Replacement << "\n");
          rauwConvertingAddressSpace(Usr, Replacement, Builder, DeletedInstrs);
          continue;
        }
      } else {
        // Replace a constant by non-constant expression.
        assert(Builder && "Must have IRBuilder to replace const with non-const");
        Value *Replacement = replaceConstantByMutable(C, cast<Constant>(Old),
                                                      New, NewAddressSpace, *Builder);
        DEBUG_WITH_TYPE("localizer", dbgs() << "== Replaced (Constant with non-const):\n");
        DEBUG_WITH_TYPE("localizer", dbgs() << " " << *Replacement << "\n");
        rauwConvertingAddressSpace(Usr, Replacement, Builder, DeletedInstrs);
        continue;
      }
    }

    U.set(New);
    // Skip instructions scheduled for removal.
    if (DeletedInstrs)
      if (auto *I = dyn_cast<Instruction>(Usr))
        if (DeletedInstrs->count(I))
          continue;

    // The user of replaced value may change its type due to the replacement. In
    // this case its users must be updated properly.
    if (auto GEP = dyn_cast<GetElementPtrInst>(Usr)) {
      Type *T = GEP->getSourceElementType();
      SmallVector<Value *, 8> Args;
      for (unsigned I = 1; I < GEP->getNumOperands(); ++I)
        Args.push_back(GEP->getOperand(I));
      auto NewGEP = GEP->isInBounds()
          ? GetElementPtrInst::CreateInBounds(T, GEP->getPointerOperand(), Args, GEP->getName(), GEP)
          : GetElementPtrInst::Create(T, GEP->getPointerOperand(), Args, GEP->getName(), GEP);
      DEBUG_WITH_TYPE("localizer", dbgs() << "== Replaced:\n");
      DEBUG_WITH_TYPE("localizer", dbgs() << " " << *NewGEP << "\n");
      rauwConvertingAddressSpace(GEP, NewGEP, Builder, DeletedInstrs);
    }
    else if (auto *BC = dyn_cast<BitCastInst>(Usr)) {
      PointerType *T = cast<PointerType>(BC->getType());
      Type* ElT = T->getElementType();
      if (PointerType*SubPT = dyn_cast<PointerType>(ElT)) {
        Type* TSubEl = SubPT->getElementType();
        assert(dyn_cast<PointerType>(TSubEl)==0);
        ElT = PointerType::get(TSubEl, NewAddressSpace);
      }
      PointerType *NT = PointerType::get(ElT, NewAddressSpace);
      auto NewBC = BitCastInst::Create(Instruction::CastOps::BitCast, BC->getOperand(0), NT, BC->getName(), BC);
      DEBUG_WITH_TYPE("localizer", dbgs() << "== Replaced:\n");
      DEBUG_WITH_TYPE("localizer", dbgs() << " " << *NewBC << "\n");
      rauwConvertingAddressSpace(BC, NewBC, Builder, DeletedInstrs);
    }
    else if (auto *CI = dyn_cast<CallInst>(Usr)) {
      Intrinsic::ID FuncID = CI->getCalledFunction()->getIntrinsicID();
      // Skip lifitime calls.
      if (FuncID == Intrinsic::lifetime_start ||
          FuncID == Intrinsic::lifetime_end) {
        if (DeletedInstrs)
          DeletedInstrs->insert(CI);
        continue;
      }
      // Update function calls.
      if (FuncID == Intrinsic::memset) {
        IRBuilder<> Builder(cast<Instruction>(Usr));
        auto *OldCall = cast<MemSetInst>(CI);
        auto NewCall = Builder.CreateMemSet(OldCall->getDest(), OldCall->getValue(),
                                            OldCall->getLength(),
                                            OldCall->getDestAlign(), OldCall->isVolatile());
        DEBUG_WITH_TYPE("localizer", dbgs() << "== Replaced:\n");
        DEBUG_WITH_TYPE("localizer", dbgs() << " " << *NewCall << "\n");
        rauwConvertingAddressSpace(CI, NewCall, &Builder, DeletedInstrs);
        continue;
      }
      if (FuncID == Intrinsic::memcpy) {
        IRBuilder<> Builder(cast<Instruction>(Usr));
        auto *OldCall = cast<MemCpyInst>(CI);
        auto NewCall = Builder.CreateMemCpy(OldCall->getDest(), OldCall->getDestAlign(),
                                            OldCall->getSource(), OldCall->getSourceAlign(),
                                            OldCall->getLength(), OldCall->isVolatile());  //TODO: other parameters
        DEBUG_WITH_TYPE("localizer", dbgs() << "== Replaced:\n");
        DEBUG_WITH_TYPE("localizer", dbgs() << " " << *NewCall << "\n");
        rauwConvertingAddressSpace(CI, NewCall, &Builder, DeletedInstrs);
        continue;
      }
    } else if (auto PHI = dyn_cast<PHINode>(Usr)) {
      // All other income values must be converted.
      for (Use &Income : PHI->incoming_values()) {
        if (Income.get() == New)
          continue;
      }
    } else if (isa<LoadInst>(Usr) || isa<StoreInst>(Usr)) {
        DEBUG_WITH_TYPE("localizer", dbgs() << "== Remained as is:\n");
        DEBUG_WITH_TYPE("localizer", dbgs() << " " << *Usr << "\n");
    } else {
      llvm_unreachable("Cannot transform");
    }
  }
}


bool ValueReplacer::PHIInfo::IsDetermined() const {
  for (auto Arg : NewArguments)
    if (!Arg)
      return false;
  return true;
}


void ValueReplacer::PHIInfo::resolve() {
  for (unsigned I = 0; I < NewArguments.size(); ++I)
    NewPHI->addIncoming(NewArguments[I], NewPHI->getIncomingBlock(I));
}


void ValueReplacer::clear() {
  for (Instruction *I : DeletedInstructions)
    I->eraseFromParent();
}


void ValueReplacer::schedule(Value *Old, Value *New) {
  WorkList.push_back(std::make_pair(Old, New));
}


void ValueReplacer::replace(Value *Old, Value *New) {
  schedule(Old, New);
  while (!WorkList.empty()) {
    auto &Item = WorkList.back();
    WorkList.pop_back();
    processItem(Item.first, Item.second);
  }
  for (auto &I : PHIMapping) {
    PHINode *OldPHI = I.first;
    PHIInfo &Info = I.second;
    PHINode *NewPHI = Info.NewPHI;
    assert(NewPHI);
    for (unsigned I = 0; I < OldPHI->getNumIncomingValues(); ++I) {
      if (Info.NewArguments[I])
        NewPHI->addIncoming(Info.NewArguments[I], OldPHI->getIncomingBlock(I));
      else
        NewPHI->addIncoming(OldPHI->getIncomingValue(I), OldPHI->getIncomingBlock(I));
    }
    OldPHI->eraseFromParent();
  }
  PHIMapping.clear();
}



void ValueReplacer::processItem(Value *Old, Value *New) {

  DEBUG_WITH_TYPE("replacer", dbgs() << "== Replacing: " << *Old << "\n");
  DEBUG_WITH_TYPE("replacer", dbgs() << "== With     : " << *New << "\n");

  if (Old == New)
    return;

  if (auto *I = dyn_cast<Instruction>(Old)) {
    if (!isa<PHINode>(I)) {
      DeletedInstructions.insert(I);
      DEBUG_WITH_TYPE("replacer", dbgs() << "   <" << *Old << "> will be deleted\n");
    }
  }

  // If values are not of pointer types, use usual RAUW mechanism.
  if (!isa<PointerType>(Old->getType())) {
    assert(Old->getType() == New->getType());
    DEBUG_WITH_TYPE("replacer", dbgs() << "   Use RAUW as values are not of pointer type\n");
    Old->replaceAllUsesWith(New);
    return;
  }

  // Both values must be of pointer types.
  auto OldType = cast<PointerType>(Old->getType());
  auto NewType = cast<PointerType>(New->getType());

  unsigned OldAddrSpace = OldType->getAddressSpace();
  unsigned NewAddrSpace = NewType->getAddressSpace();
  Type *PointeeType = OldType->getElementType();
  (void) PointeeType;

  if (!dyn_cast<BitCastInst>(Old) && dyn_cast<BitCastInst>(New))
    assert(PointeeType == NewType->getElementType());

  // If address space is not changed, use usual RAUW mechanism.
  if (OldAddrSpace == NewAddrSpace) {
    DEBUG_WITH_TYPE("replacer", dbgs() << "   Use RAUW as address spaces are the same\n");
    Old->replaceAllUsesWith(New);
    return;
  }
  DEBUG_WITH_TYPE("replacer", dbgs() << "   Apply transformation as address spaces differ\n");

  // Notify all ValueHandles (if present) that this value is going away.
  if (Old->hasValueHandle())
    ValueHandleBase::ValueIsRAUWd(Old, New);

  // Scan all uses of 'Old' and replace in them 'Old' for 'New'. This
  // replacement can cause replacement of the user and users of the user etc.
  unsigned Cnt = 0;
  while (!Old->use_empty()) {
    Use &U = *Old->use_begin();
    User *Usr = U.getUser();

    DEBUG_WITH_TYPE("replacer", dbgs() << "  -- Transform (" << Cnt++ << "): " << *Usr << "\n");

    // If the user itself has no uses, try eliminating it. No not eliminate PHI
    // nodes however, as they carry references to their arguments. Without them
    // arguments of new PHI nodes wouldn't be transformed.
    if (!isa<PHINode>(Usr) && Usr->getNumUses() == 0) {
      U.set(nullptr);
      DEBUG_WITH_TYPE("replacer", dbgs() << "   Removed\n");
      if (auto I = dyn_cast<Instruction>(Usr)) {
        if (!I->mayHaveSideEffects()) {
          DeletedInstructions.insert(I);
          continue;
        }
      } else {
        continue;
      }
    }

    // Skip instructions scheduled for removal.
    if (auto *I = dyn_cast<Instruction>(Usr))
      if (DeletedInstructions.count(I))
        continue;

    // Update user reference.
    U.set(New);
    // If an argument of a value change its type, the type of the value itself
    // also can change. In this case we must replace the user itself with a new
    // value that we construct here.

    if (isa<LoadInst>(Usr) || isa<StoreInst>(Usr)) {
      DEBUG_WITH_TYPE("replacer", dbgs() << "   Remained as is: " << *Usr << "\n");
    }

    else if (auto *BC = dyn_cast<BitCastInst>(Usr)) {
      assert(U.getOperandNo() == 0);
      PointerType *T = cast<PointerType>(BC->getType());
      assert(T);
      int tas = T->getAddressSpace();
      if (tas == 0) {
        Type *ElT = T->getElementType();
        PointerType *SubPT = dyn_cast<PointerType>(ElT);
        if (SubPT) {
          int eltas = SubPT->getAddressSpace();
          if (eltas == 0) { // stack replacing on global
            Type *TSubEl = SubPT->getElementType();
            assert(dyn_cast<PointerType>(TSubEl) == 0);
            ElT = PointerType::get(TSubEl, NewAddrSpace);
          }
        }
        PointerType *NT = PointerType::get(ElT, NewAddrSpace);
        auto NBC = new BitCastInst(BC->getOperand(0), NT, BC->getName(), BC);
        DEBUG_WITH_TYPE("replacer", dbgs() << "  Replacing:\n");
        DEBUG_WITH_TYPE("replacer", dbgs() << "    " << *BC << "\n");
        DEBUG_WITH_TYPE("replacer", dbgs() << "  with:\n");
        DEBUG_WITH_TYPE("replacer", dbgs() << "    " << *NBC << "\n");
        schedule(BC, NBC);
      }
    }

    else if (auto GEP = dyn_cast<GetElementPtrInst>(Usr)) {
      assert(U.getOperandNo() == 0);
      Type *T = GEP->getSourceElementType();
      SmallVector<Value *, 8> Args;
      for (unsigned I = 1; I < GEP->getNumOperands(); ++I)
        Args.push_back(GEP->getOperand(I));
      auto NGEP = GEP->isInBounds()
        ? GetElementPtrInst::CreateInBounds(T, New, Args, GEP->getName(), GEP)
        : GetElementPtrInst::Create(T, New, Args, GEP->getName(), GEP);
      DEBUG_WITH_TYPE("replacer", dbgs() << "  Replacing:\n");
      DEBUG_WITH_TYPE("replacer", dbgs() << "    " << *GEP << "\n");
      DEBUG_WITH_TYPE("replacer", dbgs() << "  with:\n");
      DEBUG_WITH_TYPE("replacer", dbgs() << "    " << *NGEP << "\n");
      schedule(GEP, NGEP);
    }

    else if (auto *CI = dyn_cast<CallInst>(Usr)) {
      Intrinsic::ID FuncID = CI->getCalledFunction()->getIntrinsicID();
      if (FuncID == Intrinsic::lifetime_start ||
          FuncID == Intrinsic::lifetime_end) {
        // Lifetime intrinsics are used for marking alloca life ranges. We cannot
        // use them when moving allocas to global scope and cannot see then when
        // moving global variables to local scope.
        assert(U.getOperandNo() == 1);
        DeletedInstructions.insert(CI);
        DEBUG_WITH_TYPE("replacer", dbgs() << "   Remove:\n");
        DEBUG_WITH_TYPE("replacer", dbgs() << "     " << *CI << "\n");
      }
      else if (FuncID == Intrinsic::memset) {
        assert(U.getOperandNo() == 0);
        IRBuilder<> Builder(CI);
        auto *MemSetCall = cast<MemSetInst>(CI);
        MDNode *TBAA = MemSetCall->getMetadata(LLVMContext::MD_tbaa);
        MDNode *ScopeMD = MemSetCall->getMetadata(LLVMContext::MD_alias_scope);
        MDNode *NoAliasMD = MemSetCall->getMetadata(LLVMContext::MD_noalias);

        // We need to use other function due to change of argument type.
        CallInst *NewMemSet = Builder.CreateMemSet(New, MemSetCall->getValue(),
            MemSetCall->getLength(), MemSetCall->getDestAlign(),
            MemSetCall->isVolatile(), TBAA, ScopeMD, NoAliasMD);
        DEBUG_WITH_TYPE("replacer", dbgs() << "   Updated to:\n");
        DEBUG_WITH_TYPE("replacer", dbgs() << "     " << *NewMemSet << "\n");
        (void)NewMemSet;
        DeletedInstructions.insert(MemSetCall);
      }
      else if (FuncID == Intrinsic::memcpy) {
        assert(U.getOperandNo() == 0 || U.getOperandNo() == 1);
        IRBuilder<> Builder(CI);
        auto *MemCpyCall = cast<MemCpyInst>(CI);
        MDNode *TBAA = MemCpyCall->getMetadata(LLVMContext::MD_tbaa);
        MDNode *TBAAStruct = MemCpyCall->getMetadata(LLVMContext::MD_tbaa_struct);
        MDNode *ScopeMD = MemCpyCall->getMetadata(LLVMContext::MD_alias_scope);
        MDNode *NoAliasMD = MemCpyCall->getMetadata(LLVMContext::MD_noalias);

        // We need to use other function due to change of argument type.
        CallInst *NewMemCpy = Builder.CreateMemCpy(
            MemCpyCall->getDest(), MemCpyCall->getDestAlign(),
            MemCpyCall->getSource(), MemCpyCall->getSourceAlign(),
            MemCpyCall->getLength(), MemCpyCall->isVolatile(),
            TBAA, TBAAStruct, ScopeMD, NoAliasMD);
        DEBUG_WITH_TYPE("replacer", dbgs() << "   Replaced:\n");
        DEBUG_WITH_TYPE("replacer", dbgs() << "     " << *NewMemCpy << "\n");
        (void)NewMemCpy;
        DeletedInstructions.insert(MemCpyCall);
      }
      else {
        llvm_unreachable("Unexpected function call");
      }
    }

    else if (auto PHI = dyn_cast<PHINode>(Usr)) {
      PHIInfo &Info = PHIMapping[PHI];
      if (!Info.NewPHI) {
        IRBuilder<> Builder(PHI);
        unsigned NumIncomes = PHI->getNumIncomingValues();
        Info.NewArguments.resize(NumIncomes, nullptr);
        Info.NewPHI = Builder.CreatePHI(NewType, NumIncomes, PHI->getName());
        DEBUG_WITH_TYPE("replacer", dbgs() << "   Created new PHI: " << *Info.NewPHI << '\n');
        schedule(PHI, Info.NewPHI);
      } else {
        assert(!Info.IsDetermined());
      }

      unsigned ArgNo = U.getOperandNo();
      assert(ArgNo < Info.NewArguments.size());
      assert(Info.NewArguments[ArgNo] == nullptr);
      Info.NewArguments[ArgNo] = New;
/*      if (Info.IsDetermined()) {
        Info.resolve();
        DEBUG_WITH_TYPE("replacer", dbgs() << "   Resolved new PHI for: " << *Info.NewPHI);
      }
*/
    }

    else if (auto Cmp = dyn_cast<CmpInst>(Usr)) {
      unsigned OtherOperandNo;
      if (New == Cmp->getOperand(0)) {
        OtherOperandNo = 1;
      } else {
        assert(New == Cmp->getOperand(1));
        OtherOperandNo = 0;
      }
      Value *OtherOperand = Cmp->getOperand(OtherOperandNo);

      // If type of the other argument has already the required address space,
      // assume that it is already convered.
      auto OtherType = cast<PointerType>(OtherOperand->getType());
      if (OtherType->getAddressSpace() == NewAddrSpace)
        continue;

      // Scan pending replacements. If the operand is in this set, replace it now.
      Value *Replacement = nullptr;
      for (auto Item : WorkList) {
        if (Item.first == OtherOperand) {
          Replacement = Item.second;
          break;
        }
      }
      assert(Replacement);
      Cmp->setOperand(OtherOperandNo, Replacement);
    }

    else if (auto ASC = dyn_cast<AddrSpaceCastInst>(Usr)) {
      auto *PT = cast<PointerType>(ASC->getType());
      (void) PT;
      assert(PT->getAddressSpace() == NewAddrSpace);
      assert(ASC->getOperand(0) == New);
      ASC->replaceAllUsesWith(New);
      ASC->eraseFromParent();
    }

    else if (auto CE = dyn_cast<ConstantExpr>(Usr)) {
      DEBUG_WITH_TYPE("replacer", dbgs() << "   Constant expression:\n");
      DEBUG_WITH_TYPE("replacer", dbgs() << "     " << *CE << "\n");
      if (CE->isCast()) {
        assert(CE->getOperand(0) == New);
        auto *PT = cast<PointerType>(CE->getType());
        if (PT->getAddressSpace() != NewAddrSpace) {
          PointerType *OT = cast<PointerType>(CE->getType());
          PointerType *NT = PointerType::get(OT->getElementType(), NewAddrSpace);
          auto NCE = ConstantExpr::getCast(CE->getOpcode(), CE->getOperand(0), NT);
          schedule(CE, NCE);
        }
      } else {
        llvm_unreachable("Cannot transform constant expression");
      }
    }
    else if (auto PI = dyn_cast<PtrToIntInst>(Usr)) {
      auto oprnd = PI->getOperand(0);
      PointerType  *pt = cast<PointerType>(oprnd->getType());
      if (pt->getAddressSpace() == OldAddrSpace) {
        PointerType *NT = PointerType::get(pt, NewAddrSpace);
        auto NBC = new PtrToIntInst(oprnd, NT, PI->getName(), PI);
        schedule(BC, NBC);
      }
    }
    else {
      llvm_unreachable("Cannot transform");
    }
  }
}


#ifdef _DEBUG
void dump_users(const Value *V) {
  if (!V) {
    dbgs() << "NULL\n";
    return;
  }
  dbgs() << "------ Users of:\n";
  V->dump();
  dbgs() << "------\n";
  unsigned cnt = 0;
  for (const Use &U : V->uses()) {
    dbgs() << "[" << cnt << "]  ";
    auto *Usr = U.getUser();
    if (Usr)
      Usr->dump();
    else
      dbgs() << "NULL\n";
    ++cnt;
  }
  dbgs() << "------\n";
}
#endif
}
