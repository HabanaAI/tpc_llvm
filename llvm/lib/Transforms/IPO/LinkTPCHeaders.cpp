//===- InferFunctionAttrs.cpp - Infer implicit function attributes --------===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//

#include "llvm/Transforms/IPO/LinkTPCHeaders.h"
#include "llvm/ADT/APInt.h"
#include "llvm/ADT/DenseSet.h"
#include "llvm/IR/DerivedTypes.h"
#include "llvm/IR/Function.h"
#include "llvm/IR/IRBuilder.h"
#include "llvm/IR/Instruction.h"
#include "llvm/IR/IntrinsicInst.h"
#include "llvm/IR/Intrinsics.h"
#include "llvm/IR/IntrinsicsTPC.h"
#include "llvm/IR/LLVMContext.h"
#include "llvm/IR/Module.h"
#include "llvm/IRReader/IRReader.h"
#include "llvm/InitializePasses.h"
#include "llvm/Linker/Linker.h"
#include "llvm/PassSupport.h"
#include "llvm/Support/CommandLine.h"
#include "llvm/Support/Debug.h"
#include "llvm/Support/Process.h"
#include "llvm/Support/SourceMgr.h"
#include "llvm/Support/raw_ostream.h"
#include <string>
using namespace llvm;

#define DEBUG_TYPE "link-tpc-headers"

namespace {
class LinkTPCHeadersLegacyPass : public ModulePass {
private:
  // Function name map.
  DenseMap<ArrayRef<unsigned>, StringRef> NameMap;
  // IR map.
  DenseMap<ArrayRef<unsigned>, StringRef> LinkFileMap;
  // A set of already linked IRs.
  DenseSet<StringRef> LinkedIRs;

  DenseMap<StringRef, unsigned> ArchMap;
  // Supported architectures.
  enum SubArchType { Goya = 1, Gaudi };

  StringRef SubArchName = "gaudi";

  // Replacement list.
  SmallVector<Instruction *, 8> ReplacementList;

public:
  static char ID; // Pass identification, replacement for typeid
  LinkTPCHeadersLegacyPass() : ModulePass(ID) {
    initializeLinkTPCHeadersLegacyPassPass(*PassRegistry::getPassRegistry());

    // Initializing the Dense Map for Arch values.
    ArchMap["goya"] = SubArchType::Goya;
    ArchMap["gaudi"] = SubArchType::Gaudi;

    // Sin F32
    LinkFileMap[ArrayRef<unsigned>({Intrinsic::sin, Type::FloatTyID * 64 * 32,
                                    Type::FloatTyID * 64 * 32, 0,
                                    SubArchType::Gaudi})] = GaudiSinF32LL;
    NameMap[ArrayRef<unsigned>({Intrinsic::sin, Type::FloatTyID * 64 * 32,
                                Type::FloatTyID * 64 * 32, 0,
                                SubArchType::Gaudi})] = "sin_f32";
    // Cos F32
    LinkFileMap[ArrayRef<unsigned>({Intrinsic::cos, Type::FloatTyID * 64 * 32,
                                    Type::FloatTyID * 64 * 32, 0,
                                    SubArchType::Gaudi})] = GaudiCosF32LL;
    NameMap[ArrayRef<unsigned>({Intrinsic::cos, Type::FloatTyID * 64 * 32,
                                Type::FloatTyID * 64 * 32, 0,
                                SubArchType::Gaudi})] = "cos_f32";
    // Exp F32
    LinkFileMap[ArrayRef<unsigned>({Intrinsic::exp, Type::FloatTyID * 64 * 32,
                                    Type::FloatTyID * 64 * 32, 0,
                                    SubArchType::Gaudi})] = GaudiExpF32LL;
    NameMap[ArrayRef<unsigned>({Intrinsic::exp, Type::FloatTyID * 64 * 32,
                                Type::FloatTyID * 64 * 32, 0,
                                SubArchType::Gaudi})] = "exp_cephes_f32";
    // Log F32
    LinkFileMap[ArrayRef<unsigned>({Intrinsic::log, Type::FloatTyID * 64 * 32,
                                    Type::FloatTyID * 64 * 32, 0,
                                    SubArchType::Gaudi})] = GaudiLogF32LL;
    NameMap[ArrayRef<unsigned>({Intrinsic::log, Type::FloatTyID * 64 * 32,
                                Type::FloatTyID * 64 * 32, 0,
                                SubArchType::Gaudi})] = "log_f32";
    // Sqrt F32
    LinkFileMap[ArrayRef<unsigned>({Intrinsic::sqrt, Type::FloatTyID * 64 * 32,
                                    Type::FloatTyID * 64 * 32, 0,
                                    SubArchType::Gaudi})] = GaudiSqrtF32LL;
    NameMap[ArrayRef<unsigned>({Intrinsic::sqrt, Type::FloatTyID * 64 * 32,
                                Type::FloatTyID * 64 * 32, 0,
                                SubArchType::Gaudi})] = "sqrt_f32";
    // Tanh F32
    LinkFileMap[ArrayRef<unsigned>(
        {Intrinsic::tpc_tanh, Type::FloatTyID * 64 * 32,
         Type::FloatTyID * 64 * 32, 0, SubArchType::Gaudi})] = GaudiTanhF32LL;
    NameMap[ArrayRef<unsigned>({Intrinsic::tpc_tanh, Type::FloatTyID * 64 * 32,
                                Type::FloatTyID * 64 * 32, 0,
                                SubArchType::Gaudi})] = "tanh_f32";
    // RSqrt F32
    LinkFileMap[ArrayRef<unsigned>(
        {Intrinsic::tpc_rsqrt, Type::FloatTyID * 64 * 32,
         Type::FloatTyID * 64 * 32, 0, SubArchType::Gaudi})] = GaudiRSqrtF32LL;
    NameMap[ArrayRef<unsigned>({Intrinsic::tpc_rsqrt, Type::FloatTyID * 64 * 32,
                                Type::FloatTyID * 64 * 32, 0,
                                SubArchType::Gaudi})] = "rsqrt_f32";
    // Reciprocal F32
    LinkFileMap[ArrayRef<unsigned>(
        {Intrinsic::tpc_reciprocal, Type::FloatTyID * 64 * 32,
         Type::FloatTyID * 64 * 32, 0, SubArchType::Gaudi})] = GaudiRecipF32LL;
    NameMap[ArrayRef<unsigned>(
        {Intrinsic::tpc_reciprocal, Type::FloatTyID * 64 * 32,
         Type::FloatTyID * 64 * 32, 0, SubArchType::Gaudi})] = "reciprocal_f32";

    // reduction F32 fadd
    LinkFileMap[ArrayRef<unsigned>(
        {Intrinsic::experimental_vector_reduce_add, Type::FloatTyID * 32,
         Type::FloatTyID * 64 * 32, 0, SubArchType::Gaudi})] =
        GaudiReduceAddF32;
    NameMap[ArrayRef<unsigned>({Intrinsic::experimental_vector_reduce_add,
                                Type::FloatTyID * 32, Type::FloatTyID * 64 * 32,
                                0, SubArchType::Gaudi})] = "v_f32_reduce_add";

    // reduction F32 fmul
    LinkFileMap[ArrayRef<unsigned>(
        {Intrinsic::experimental_vector_reduce_mul, Type::FloatTyID * 32,
         Type::FloatTyID * 64 * 32, 0, SubArchType::Gaudi})] =
        GaudiReduceMulF32LL;
    NameMap[ArrayRef<unsigned>({Intrinsic::experimental_vector_reduce_mul,
                                Type::FloatTyID * 32, Type::FloatTyID * 64 * 32,
                                0, SubArchType::Gaudi})] = "v_f32_reduce_mul";

    // reduction F32 fmax
    LinkFileMap[ArrayRef<unsigned>(
        {Intrinsic::experimental_vector_reduce_fmax, Type::FloatTyID * 32,
         Type::FloatTyID * 64 * 32, 0, SubArchType::Gaudi})] =
        GaudiReduceMaxF32LL;
    NameMap[ArrayRef<unsigned>({Intrinsic::experimental_vector_reduce_fmax,
                                Type::FloatTyID * 32, Type::FloatTyID * 64 * 32,
                                0, SubArchType::Gaudi})] = "v_f32_reduce_max";

    // reduction F32 fmin
    LinkFileMap[ArrayRef<unsigned>(
        {Intrinsic::experimental_vector_reduce_fmin, Type::FloatTyID * 32,
         Type::FloatTyID * 64 * 32, 0, SubArchType::Gaudi})] =
        GaudiReduceMinF32LL;
    NameMap[ArrayRef<unsigned>({Intrinsic::experimental_vector_reduce_fmin,
                                Type::FloatTyID * 32, Type::FloatTyID * 64 * 32,
                                0, SubArchType::Gaudi})] = "v_f32_reduce_min";

    // Sin BF16
    LinkFileMap[ArrayRef<unsigned>({Intrinsic::sin, Type::BFloat16ID * 128 * 16,
                                    Type::BFloat16ID * 128 * 16, 0,
                                    SubArchType::Gaudi})] = GaudiSinBF16LL;
    NameMap[ArrayRef<unsigned>({Intrinsic::sin, Type::BFloat16ID * 128 * 16,
                                Type::BFloat16ID * 128 * 16, 0,
                                SubArchType::Gaudi})] = "sin_bf16";
    // reduction BF16 fadd
    LinkFileMap[ArrayRef<unsigned>(
        {Intrinsic::experimental_vector_reduce_add, Type::BFloat16ID * 16,
         Type::BFloat16ID * 128 * 16, 0, SubArchType::Gaudi})] =
        GaudiReduceAddBF16LL;
    NameMap[ArrayRef<unsigned>(
        {Intrinsic::experimental_vector_reduce_add, Type::BFloat16ID * 16,
         Type::BFloat16ID * 128 * 16, 0, SubArchType::Gaudi})] =
        "v_bf16_reduce_add";

    // reduction BF16 fmax
    LinkFileMap[ArrayRef<unsigned>(
        {Intrinsic::experimental_vector_reduce_fmax, Type::BFloat16ID * 16,
         Type::BFloat16ID * 128 * 16, 0, SubArchType::Gaudi})] =
        GaudiReduceMaxBF16LL;
    NameMap[ArrayRef<unsigned>(
        {Intrinsic::experimental_vector_reduce_fmax, Type::BFloat16ID * 16,
         Type::BFloat16ID * 128 * 16, 0, SubArchType::Gaudi})] =
        "v_bf16_reduce_max";

    // reduction BF16 fmin
    LinkFileMap[ArrayRef<unsigned>(
        {Intrinsic::experimental_vector_reduce_fmin, Type::BFloat16ID * 16,
         Type::BFloat16ID * 128 * 16, 0, SubArchType::Gaudi})] =
        GaudiReduceMinBF16LL;
    NameMap[ArrayRef<unsigned>(
        {Intrinsic::experimental_vector_reduce_fmin, Type::BFloat16ID * 16,
         Type::BFloat16ID * 128 * 16, 0, SubArchType::Gaudi})] =
        "v_bf16_reduce_min";

    // Cos BF16
    LinkFileMap[ArrayRef<unsigned>({Intrinsic::cos, Type::BFloat16ID * 128 * 16,
                                    Type::BFloat16ID * 128 * 16, 0,
                                    SubArchType::Gaudi})] = GaudiCosBF16LL;
    NameMap[ArrayRef<unsigned>({Intrinsic::cos, Type::BFloat16ID * 128 * 16,
                                Type::BFloat16ID * 128 * 16, 0,
                                SubArchType::Gaudi})] = "cos_bf16";
    // Exp BF16
    LinkFileMap[ArrayRef<unsigned>({Intrinsic::exp, Type::BFloat16ID * 128 * 16,
                                    Type::BFloat16ID * 128 * 16, 0,
                                    SubArchType::Gaudi})] = GaudiExpBF16LL;
    NameMap[ArrayRef<unsigned>({Intrinsic::exp, Type::BFloat16ID * 128 * 16,
                                Type::BFloat16ID * 128 * 16, 0,
                                SubArchType::Gaudi})] = "exp_bf16";
    // Log BF16
    LinkFileMap[ArrayRef<unsigned>({Intrinsic::log, Type::BFloat16ID * 128 * 16,
                                    Type::BFloat16ID * 128 * 16, 0,
                                    SubArchType::Gaudi})] = GaudiLogBF16LL;
    NameMap[ArrayRef<unsigned>({Intrinsic::log, Type::BFloat16ID * 128 * 16,
                                Type::BFloat16ID * 128 * 16, 0,
                                SubArchType::Gaudi})] = "log_bf16";
    // Sqrt BF16
    LinkFileMap[ArrayRef<unsigned>(
        {Intrinsic::sqrt, Type::BFloat16ID * 128 * 16,
         Type::BFloat16ID * 128 * 16, 0, SubArchType::Gaudi})] =
        GaudiSqrtBF16LL;
    NameMap[ArrayRef<unsigned>({Intrinsic::sqrt, Type::BFloat16ID * 128 * 16,
                                Type::BFloat16ID * 128 * 16, 0,
                                SubArchType::Gaudi})] = "sqrt_bf16";
    // Tanh BF16
    LinkFileMap[ArrayRef<unsigned>(
        {Intrinsic::tpc_tanh, Type::BFloat16ID * 128 * 16,
         Type::BFloat16ID * 128 * 16, 0, SubArchType::Gaudi})] =
        GaudiTanhBF16LL;
    NameMap[ArrayRef<unsigned>(
        {Intrinsic::tpc_tanh, Type::BFloat16ID * 128 * 16,
         Type::BFloat16ID * 128 * 16, 0, SubArchType::Gaudi})] = "tanh_bf16";
    // RSqrt BF16
    LinkFileMap[ArrayRef<unsigned>(
        {Intrinsic::tpc_rsqrt, Type::BFloat16ID * 128 * 16,
         Type::BFloat16ID * 128 * 16, 0, SubArchType::Gaudi})] =
        GaudiRSqrtBF16LL;
    NameMap[ArrayRef<unsigned>(
        {Intrinsic::tpc_rsqrt, Type::BFloat16ID * 128 * 16,
         Type::BFloat16ID * 128 * 16, 0, SubArchType::Gaudi})] = "rsqrt_bf16";
    // Reciprocal BF16
    LinkFileMap[ArrayRef<unsigned>(
        {Intrinsic::tpc_reciprocal, Type::BFloat16ID * 128 * 16,
         Type::BFloat16ID * 128 * 16, 0, SubArchType::Gaudi})] =
        GaudiRecipBF16LL;
    NameMap[ArrayRef<unsigned>(
        {Intrinsic::tpc_reciprocal, Type::BFloat16ID * 128 * 16,
         Type::BFloat16ID * 128 * 16, 0, SubArchType::Gaudi})] =
        "reciprocal_bf16";

    // reduction I16 max
    LinkFileMap[ArrayRef<unsigned>(
        {Intrinsic::experimental_vector_reduce_smax, Type::IntegerTyID * 16,
         Type::IntegerTyID * 128 * 16, 0, SubArchType::Gaudi})] =
        GaudiReduceMaxI16LL;
    NameMap[ArrayRef<unsigned>(
        {Intrinsic::experimental_vector_reduce_smax, Type::IntegerTyID * 16,
         Type::IntegerTyID * 128 * 16, 0, SubArchType::Gaudi})] =
        "v_i16_reduce_max";

    // reduction I16 min
    LinkFileMap[ArrayRef<unsigned>(
        {Intrinsic::experimental_vector_reduce_smin, Type::IntegerTyID * 16,
         Type::IntegerTyID * 128 * 16, 0, SubArchType::Gaudi})] =
        GaudiReduceMinI16LL;
    NameMap[ArrayRef<unsigned>(
        {Intrinsic::experimental_vector_reduce_smin, Type::IntegerTyID * 16,
         Type::IntegerTyID * 128 * 16, 0, SubArchType::Gaudi})] =
        "v_i16_reduce_min";

    // reduction I8 max
    LinkFileMap[ArrayRef<unsigned>(
        {Intrinsic::experimental_vector_reduce_smax, Type::IntegerTyID * 8,
         Type::IntegerTyID * 256 * 8, 0, SubArchType::Gaudi})] =
        GaudiReduceMaxI8LL;
    NameMap[ArrayRef<unsigned>(
        {Intrinsic::experimental_vector_reduce_smax, Type::IntegerTyID * 8,
         Type::IntegerTyID * 256 * 8, 0, SubArchType::Gaudi})] =
        "v_i8_reduce_max";

    // reduction I8 min
    LinkFileMap[ArrayRef<unsigned>(
        {Intrinsic::experimental_vector_reduce_smin, Type::IntegerTyID * 8,
         Type::IntegerTyID * 256 * 8, 0, SubArchType::Gaudi})] =
        GaudiReduceMinI8LL;
    NameMap[ArrayRef<unsigned>(
        {Intrinsic::experimental_vector_reduce_smin, Type::IntegerTyID * 8,
         Type::IntegerTyID * 256 * 8, 0, SubArchType::Gaudi})] =
        "v_i8_reduce_min";

    // reduction U8 min
    LinkFileMap[ArrayRef<unsigned>(
        {Intrinsic::experimental_vector_reduce_umin, Type::IntegerTyID * 8,
         Type::IntegerTyID * 256 * 8, 0, SubArchType::Gaudi})] =
        GaudiReduceMinU8LL;
    NameMap[ArrayRef<unsigned>(
        {Intrinsic::experimental_vector_reduce_umin, Type::IntegerTyID * 8,
         Type::IntegerTyID * 256 * 8, 0, SubArchType::Gaudi})] =
        "v_u8_reduce_min";

    // reduction U8 max
    LinkFileMap[ArrayRef<unsigned>(
        {Intrinsic::experimental_vector_reduce_umax, Type::IntegerTyID * 8,
         Type::IntegerTyID * 256 * 8, 0, SubArchType::Gaudi})] =
        GaudiReduceMaxU8LL;
    NameMap[ArrayRef<unsigned>(
        {Intrinsic::experimental_vector_reduce_umax, Type::IntegerTyID * 8,
         Type::IntegerTyID * 256 * 8, 0, SubArchType::Gaudi})] =
        "v_u8_reduce_max";

    // Goya
    // Reciprocal F32
    LinkFileMap[ArrayRef<unsigned>(
        {Intrinsic::tpc_reciprocal, Type::FloatTyID * 64 * 32,
         Type::FloatTyID * 64 * 32, 0, SubArchType::Goya})] =
        GoyaReciprocalF32LL;
    NameMap[ArrayRef<unsigned>(
        {Intrinsic::tpc_reciprocal, Type::FloatTyID * 64 * 32,
         Type::FloatTyID * 64 * 32, 0, SubArchType::Goya})] =
        "reciprocal_cephes_f32";
  }

  virtual ~LinkTPCHeadersLegacyPass() {}
  void getAnalysisUsage(AnalysisUsage &AU) const override {}
  SmallVector<unsigned, 4> getHashVector(IntrinsicInst *II);
  bool instructionNeedsLinking(const Instruction &I, StringRef &IRStr);
  bool replaceWithCall(Instruction *I);

  bool runOnModule(Module &M) override;
};
} // namespace

static inline unsigned getTypeVal(Type *Ty) {
  unsigned TypeVal =
      Ty->getScalarType()->getTypeID() * Ty->getScalarSizeInBits();
  if (Ty->isVectorTy()) {
    auto VectorTy = dyn_cast<VectorType>(Ty);
    TypeVal *= (VectorTy->getElementCount().Min);
  }
  return TypeVal;
}

SmallVector<unsigned, 4>
LinkTPCHeadersLegacyPass::getHashVector(IntrinsicInst *II) {
  unsigned ID = II->getIntrinsicID();
  Type *OutTy = II->getType();
  SmallVector<unsigned, 4> Hash;
  Hash.push_back(ID);
  auto OutTypeVal = getTypeVal(OutTy);
  Hash.push_back(OutTypeVal);

  for (Value *V : II->operands())
    Hash.push_back(getTypeVal(V->getType()));

  // Put SubArch in Hash.
  Hash.push_back(ArchMap[SubArchName]);
  return Hash;
}

bool LinkTPCHeadersLegacyPass::instructionNeedsLinking(const Instruction &I,
                                                       StringRef &IRStr) {
  if (const IntrinsicInst *II = dyn_cast<IntrinsicInst>(&I)) {
    auto Hash = getHashVector(const_cast<IntrinsicInst *>(II));
    auto It = LinkFileMap.find(Hash);
    if (It == LinkFileMap.end())
      return false;

    // If the special function has already been linked.
    if (LinkedIRs.find(It->second) != LinkedIRs.end()) {
      ReplacementList.push_back(const_cast<Instruction *>(&I));
      return false;
    }

    LinkedIRs.insert(It->second);
    IRStr = It->second;
    return true;
  }
  return false;
}

bool IsReduction(Instruction *I) {
  if (IntrinsicInst *II = dyn_cast<IntrinsicInst>(I)) {
    Intrinsic::ID Inid = II->getIntrinsicID();
    if ((Inid == Intrinsic::experimental_vector_reduce_add) ||
        (Inid == Intrinsic::experimental_vector_reduce_mul) ||
        (Inid == Intrinsic::experimental_vector_reduce_fmax) ||
        (Inid == Intrinsic::experimental_vector_reduce_fmin) ||
        (Inid == Intrinsic::experimental_vector_reduce_smax) ||
        (Inid == Intrinsic::experimental_vector_reduce_smin) ||
        (Inid == Intrinsic::experimental_vector_reduce_umin) ||
        (Inid == Intrinsic::experimental_vector_reduce_umax)) {
      return true;
    }
  }
  return false;
}

bool LinkTPCHeadersLegacyPass::replaceWithCall(Instruction *I) {
  Module *M = I->getModule();
  if (IntrinsicInst *II = dyn_cast<IntrinsicInst>(I)) {
    auto Hash = getHashVector(II);
    auto It = NameMap.find(Hash);
    if (It == NameMap.end())
      return false;

    Function *Special = M->getFunction(It->second);
    assert(Special && "The special function should have been linked.");
    // Set available externally attribute for the special function as it doesn't
    // link using the link API.
    Special->setLinkage(GlobalValue::AvailableExternallyLinkage);

    IRBuilder<> Builder(I);
    SmallVector<Value *, 2> Args;
    for (Value *Val : II->args())
      Args.push_back(Val);

    auto SpecialCall =
        Builder.CreateCall(Special->getFunctionType(), Special, Args);
    LLVM_DEBUG(dbgs() << "\nReplacing instruction \n"; I->dump();
               dbgs() << "with function call \n"; SpecialCall->dump(););
    if (IsReduction(I)) {
      for (const Use &RootInstUse : I->uses()) {
        User *RootInstUser = RootInstUse.getUser();
        if (auto *InsertUserCast = dyn_cast<InsertElementInst>(RootInstUser)) {
          for (const Use &RootInstUse1 : InsertUserCast->uses()) {
            User *RootInstUser1 = RootInstUse1.getUser();
            if (auto *InsertUserCast1 =
                    dyn_cast<ShuffleVectorInst>(RootInstUser1)) {
              InsertUserCast1->replaceAllUsesWith(SpecialCall);
              InsertUserCast->replaceAllUsesWith(
                  UndefValue::get(InsertUserCast->getType()));
              I->replaceAllUsesWith(UndefValue::get(I->getType()));
              InsertUserCast1->eraseFromParent();
              InsertUserCast->eraseFromParent();
              I->eraseFromParent();
              return true;
            }
          }
        }
      }
    }
    I->replaceAllUsesWith(SpecialCall);
    I->eraseFromParent();
    return true;
  }
  return false;
}

static std::string getTPCIntrinsicName(Intrinsic::ID IDNum,
                                       FunctionType *FType) {
  SmallVector<Intrinsic::IITDescriptor, 8> Table;
  Intrinsic::getIntrinsicInfoTableEntries(IDNum, Table);
  ArrayRef<Intrinsic::IITDescriptor> TableRef = Table;
  (void)TableRef;
  SmallVector<Type *, 4> ArgTys;
  Intrinsic::matchIntrinsicSignature(FType, TableRef, ArgTys);
  return Intrinsic::getName(IDNum, ArgTys);
}

static void expandFDiv(Instruction *I) {
  IRBuilder<> Builder(I);
  Module *M = I->getModule();
  Type *Ty = I->getType();
  Value *Numerator = I->getOperand(0), *Denominator = I->getOperand(1);
  SmallVector<Type *, 1> Types = {Ty};
  auto FType = FunctionType::get(Ty, Types, false);
  auto Intrinsic = cast<Function>(
      M->getOrInsertFunction(
           getTPCIntrinsicName(Intrinsic::tpc_reciprocal, FType), FType)
          .getCallee());
  auto Recip = Builder.CreateCall(Intrinsic, Denominator);
  auto FMul = Builder.CreateFMul(Numerator, Recip);
  I->replaceAllUsesWith(FMul);
}

static void expandSpecialCaseLLVMIR(Function *Main) {
  SmallVector<Instruction *, 8> EraseList;
  for (auto &BB : *Main) {
    for (auto &I : BB) {
      // Handle vector FDIV case.
      if (I.getType()->isVectorTy() && I.getOpcode() == Instruction::FDiv) {
        expandFDiv(&I);
        EraseList.push_back(&I);
      }
    }
  }
  for (Instruction *I : EraseList)
    I->eraseFromParent();
}

bool LinkTPCHeadersLegacyPass::runOnModule(Module &M) {
  if (skipModule(M))
    return false;

  LLVMContext &Ctx = M.getContext();
  SMDiagnostic Err;
  llvm::StringRef IRStr;
  Function *Main = M.getFunction("main");
  if (!Main)
    return false;
  Attribute SubArchAttr = Main->getFnAttribute("target-cpu");
  auto SubArchStr = SubArchAttr.getValueAsString();
  if (SubArchStr.size() > 0)
    SubArchName = SubArchStr;

  // Expand the IR to appropriate form.
  expandSpecialCaseLLVMIR(Main);

  bool LinkFail = false;
  // Identify the modules that need to be linked.
  for (auto &BB : *Main) {
    for (auto &I : BB) {
      std::string LinkFileName = "";
      if (instructionNeedsLinking(I, IRStr)) {
        ReplacementList.push_back(&I);

        auto ParsedModule =
            parseIR(MemoryBufferRef(IRStr, "Special"), Err, Ctx);
        if (!ParsedModule)
          return false;

        LLVM_DEBUG(dbgs() << "\nSpecial function module parsing successful.");
        LLVM_DEBUG(dbgs() << "\nLinked functions : \n";
                   for (Function &F
                        : *ParsedModule) dbgs()
                   << F.getName() << "\n";);
        ParsedModule->setDataLayout(M.getDataLayout());
        // Link modules.
        LinkFail |= Linker::linkModules(M, std::move(ParsedModule));
      }
    }
  }
  // Replace the instruction with call.
  for (Instruction *I : ReplacementList) {
    replaceWithCall(I);
  }

  return !LinkFail;
}

char LinkTPCHeadersLegacyPass::ID = 0;
INITIALIZE_PASS(LinkTPCHeadersLegacyPass, "link-tpc-headers",
                "Link TPC Headers", false, false)

Pass *llvm::createLinkTPCHeadersLegacyPass() {
  return new LinkTPCHeadersLegacyPass();
}
