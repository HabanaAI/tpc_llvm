/*****************************************************************************
 *  Copyright (C) 2021 HabanaLabs, Ltd.
 *  All Rights Reserved.
 *
 *  Unauthorized copying of this file, via any medium is strictly prohibited.
 *  Proprietary and confidential.
 *
 *  Authors:
 *   Anand Kandomi     <akodnani@habana.ai>
 *   Michael Zuckerman <mzuckerman@habana.ai>
 ****************************************************************************/

#ifndef _WIN32
#include "llvm/Transforms/IPO/LinkTPCHeaders.h"

#include "LinkTPCFunctionsMMIOAccess.h"
#include "LinkTPCFunctionsRNG.h"

#include "llvm/ADT/DenseSet.h"
#include "llvm/ADT/Hashing.h"
#include "llvm/ADT/Optional.h"
#include "llvm/ADT/SmallVector.h"
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
#include "llvm/Pass.h"
#include "llvm/Support/CommandLine.h"
#include "llvm/Support/Debug.h"
#include "llvm/Support/Path.h"
#include "llvm/Support/Process.h"
#include "llvm/Support/SourceMgr.h"
#include "llvm/Support/raw_ostream.h"
#include "llvm/Transforms/Utils/TPCIntrinsicUtils.h"
#include <cstddef>
#include <dlfcn.h>
#include <string>
#include <tuple>
#include <unordered_map>
#include <utility>
using namespace llvm;

#define DEBUG_TYPE "link-tpc-headers"

static cl::opt<std::string>
    LTOFilePath("ILTO", cl::desc("Dynamic link file absolute path to the so"),
                cl::init(""), cl::Hidden);

using ArchTy = LinkTPCHeadersPass::ArchTy;

namespace {
struct IntrinsicKey {
  unsigned IntrinsicID;
  unsigned ResTyKey;
  SmallVector<unsigned, 4> ArgsTyKeys;
  ArchTy Arch;
};

bool operator==(const IntrinsicKey &L, const IntrinsicKey &R) {
  return std::tie(L.IntrinsicID, L.ResTyKey, L.ArgsTyKeys, L.Arch) ==
         std::tie(R.IntrinsicID, R.ResTyKey, R.ArgsTyKeys, R.Arch);
}

struct LinkInfo {
  StringRef LinkCode;
  StringRef FuncName;
};
} // namespace

static constexpr unsigned TypeKey(Type::TypeID T, unsigned VecLen = 0,
                           unsigned BitWidth = 0) {
  return T + (BitWidth << 1) + Type::FixedVectorTyID * VecLen;
}

enum TypeKey {
  VOID = TypeKey(Type::VoidTyID),
  S_F32 = TypeKey(Type::FloatTyID),
  V_F32 = TypeKey(Type::FloatTyID, 64),
  V_F32_X2 = TypeKey(Type::FloatTyID, 128),
  V_F32_X4 = TypeKey(Type::FloatTyID, 256),
  S_F16 = TypeKey(Type::HalfTyID),
  V_F16 = TypeKey(Type::HalfTyID, 128),
  S_BF16 = TypeKey(Type::BFloatTyID),
  V_BF16 = TypeKey(Type::BFloatTyID, 128),
  V_BF16_X2 = TypeKey(Type::BFloatTyID, 256),
  S_I32 = TypeKey(Type::IntegerTyID, 0, 32),
  V_I32 = TypeKey(Type::IntegerTyID, 64, 32),
  S_I16 = TypeKey(Type::IntegerTyID, 0, 16),
  V_I16 = TypeKey(Type::IntegerTyID, 128, 16),
  S_I8 = TypeKey(Type::IntegerTyID, 0, 8),
  V_I8 = TypeKey(Type::IntegerTyID, 256, 8),
};

// Custom std::hash for IntrinsicKey to support unordered containers.
namespace std {
template <> struct hash<IntrinsicKey> {
  size_t operator()(const IntrinsicKey K) const {
    return hash_combine(
        K.IntrinsicID, K.ResTyKey,
        llvm::hash_combine_range(K.ArgsTyKeys.begin(), K.ArgsTyKeys.end()),
        K.Arch);
  }
};
} // namespace std

static IntrinsicKey fun_VF32_of_SI32(const unsigned ID, const ArchTy Arch) {
  return {ID, V_F32, {S_I32}, Arch};
}

static IntrinsicKey fun_VF32_of_VF32(const unsigned ID, const ArchTy Arch) {
  return {ID, V_F32, {V_F32}, Arch};
}

static IntrinsicKey fun_VF32_X2_of_VF32_X2(const unsigned ID, const ArchTy Arch) {
  return {ID, V_F32_X2, {V_F32_X2}, Arch};
}

static IntrinsicKey fun_VF32_X4_of_VF32_X4(const unsigned ID, const ArchTy Arch) {
  return {ID, V_F32_X4, {V_F32_X4}, Arch};
}

static IntrinsicKey fun_VBF16_X2_of_VBF16_X2(const unsigned ID, const ArchTy Arch) {
  return {ID, V_BF16_X2, {V_BF16_X2}, Arch};
}

static IntrinsicKey fun_SF32_of_VF32(const unsigned ID, const ArchTy Arch) {
  return {ID, S_F32, {V_F32}, Arch};
}

static IntrinsicKey fun_VBF16_of_VBF16(const unsigned ID, const ArchTy Arch) {
  return {ID, V_BF16, {V_BF16}, Arch};
}

static IntrinsicKey fun_SBF16_of_VBF16(const unsigned ID, const ArchTy Arch) {
  return {ID, S_BF16, {V_BF16}, Arch};
}

static IntrinsicKey fun_SF16_of_VF16(const unsigned ID, const ArchTy Arch) {
  return {ID, S_F16, {V_F16}, Arch};
}

static IntrinsicKey fun_VF16_of_VF16(const unsigned ID, const ArchTy Arch) {
  return {ID, V_F16, {V_F16}, Arch};
}

static IntrinsicKey fun_SI16_of_VI16(const unsigned ID, const ArchTy Arch) {
  return {ID, S_I16, {V_I16}, Arch};
}

static IntrinsicKey fun_VI16_of_VI16(const unsigned ID, const ArchTy Arch) {
  return {ID, V_I16, {V_I16}, Arch};
}

static IntrinsicKey fun_SI8_of_VI8(const unsigned ID, const ArchTy Arch) {
  return {ID, S_I8, {V_I8}, Arch};
}

static IntrinsicKey fun_VI8_of_VI8(const unsigned ID, const ArchTy Arch) {
  return {ID, V_I8, {V_I8}, Arch};
}

static IntrinsicKey fun_VI8_of_VI32_VI32_VI32_VI32(const unsigned ID,
                                                   const ArchTy Arch) {
  return {ID, V_I8, {V_I32, V_I32, V_I32, V_I32}, Arch};
}

static IntrinsicKey fun_SI32_of_SI32(const unsigned ID, const ArchTy Arch) {
  return {ID, S_I32, {S_I32}, Arch};
}

static IntrinsicKey fun_SI16_of_SI32(const unsigned ID, const ArchTy Arch) {
  return {ID, S_I16, {S_I32}, Arch};
}

static IntrinsicKey fun_SI8_of_SI32(const unsigned ID, const ArchTy Arch) {
  return {ID, S_I8, {S_I32}, Arch};
}

static IntrinsicKey fun_SF32_of_SI32(const unsigned ID, const ArchTy Arch) {
  return {ID, S_F32, {S_I32}, Arch};
}

static IntrinsicKey fun_SBF16_of_SI32(const unsigned ID, const ArchTy Arch) {
  return {ID, S_BF16, {S_I32}, Arch};
}

static IntrinsicKey fun_SI32_of_SI32_SI32(const unsigned ID,
                                          const ArchTy Arch) {
  return {ID, S_I32, {S_I32, S_I32}, Arch};
}

static IntrinsicKey fun_SI32(const unsigned ID, const ArchTy Arch) {
  return {ID, S_I32, {}, Arch};
}

static IntrinsicKey fun_of_SI32(const unsigned ID, const ArchTy Arch) {
  return {ID, VOID, {S_I32}, Arch};
}

static IntrinsicKey fun_of_SI32_SI32(const unsigned ID, const ArchTy Arch) {
  return {ID, VOID, {S_I32, S_I32}, Arch};
}

static IntrinsicKey fun_of_SI32_SI16(const unsigned ID, const ArchTy Arch) {
  return {ID, VOID, {S_I32, S_I16}, Arch};
}

static IntrinsicKey fun_of_SI32_SI8(const unsigned ID, const ArchTy Arch) {
  return {ID, VOID, {S_I32, S_I8}, Arch};
}

static IntrinsicKey fun_of_SI32_SF32(const unsigned ID, const ArchTy Arch) {
  return {ID, VOID, {S_I32, S_F32}, Arch};
}

static IntrinsicKey fun_of_SI32_SBF16(const unsigned ID, const ArchTy Arch) {
  return {ID, VOID, {S_I32, S_BF16}, Arch};
}

static IntrinsicKey fun_of_SI32_SI32_SI32(const unsigned ID,
                                          const ArchTy Arch) {
  return {ID, VOID, {S_I32, S_I32, S_I32}, Arch};
}

namespace {
class LinkTPCHeadersLegacyPass : public ModulePass {
private:
  std::unordered_map<IntrinsicKey, LinkInfo> KeyToInfo;

  // Container for generated LTO function on the fly. std::deque is chosen
  // because it guarantees references validity after insertion at the back.
  std::deque<std::string> GeneratedFunStorage;

  // A set of already linked IRs.
  DenseSet<StringRef> LinkedIRs;

  DenseMap<StringRef, ArchTy> ArchMap;

  StringRef SubArchName = "gaudi";

  // Query contain a pointer to a share object.
  LibHandle Query = NULL;

  bool linkStage(Module &M, StringRef IRStr);

  // Replacement list.
  SmallVector<Instruction *, 8> ReplacementList;

public:
  static char ID; // Pass identification, replacement for typeid
  LinkTPCHeadersLegacyPass() : ModulePass(ID) {
    initializeLinkTPCHeadersLegacyPassPass(*PassRegistry::getPassRegistry());

    // Initializing the Dense Map for Arch values.
    ArchMap["goya"] = ArchTy::Goya;
    ArchMap["gaudi"] = ArchTy::Gaudi;
    ArchMap["gaudib"] = ArchTy::GaudiB;
    ArchMap["goya2"] = ArchTy::Greco;
    ArchMap["greco"] = ArchTy::Greco;
    ArchMap["gaudi2"] = ArchTy::Gaudi2;
    ArchMap["doron1"] = ArchTy::Doron1;

    // Check if user supplied a path to shared library.
    // todo: fix this to support windows
    if (LTOFilePath != "") {
      if (llvm::sys::path::is_absolute(LTOFilePath.c_str(),
                                       llvm::sys::path::Style::posix))
        Query = LoadLib(LTOFilePath.c_str());
      else {
        llvm::SmallString<64> Cwd;
        llvm::sys::fs::current_path(Cwd);
        Cwd.append("/");
        Cwd.append(LTOFilePath);
        Query = LoadLib(Cwd.c_str());
      }
    }

    struct FunGenData {
      typedef IntrinsicKey (*IKGenT)(const unsigned ID, const ArchTy Arch);
      IKGenT IKGen;

      typedef std::string (*LLGenT)(const ArchTy Arch);
      LLGenT LLGen;

      StringRef FuncName;
    };

    // MMIO access.
    for (const ArchTy Arch : {ArchTy::Goya, ArchTy::Gaudi, ArchTy::GaudiB,
                              ArchTy::Greco, ArchTy::Gaudi2, ArchTy::Doron1}) {

      GeneratedFunStorage.emplace_back(getLLCodeForGetDimSize(Arch));
      KeyToInfo[fun_SI32_of_SI32_SI32(Intrinsic::tpc_get_dim_size, Arch)] =
          LinkInfo{GeneratedFunStorage.back(), "tpc_get_dim_size"};

      GeneratedFunStorage.emplace_back(getLLCodeForSetDimSize(Arch));
      KeyToInfo[fun_of_SI32_SI32_SI32(Intrinsic::tpc_set_dim_size, Arch)] =
          LinkInfo{GeneratedFunStorage.back(), "tpc_set_dim_size"};

      GeneratedFunStorage.emplace_back(getLLCodeForGetDimStride(Arch));
      KeyToInfo[fun_SI32_of_SI32_SI32(Intrinsic::tpc_get_dim_stride, Arch)] =
          LinkInfo{GeneratedFunStorage.back(), "tpc_get_dim_stride"};

      GeneratedFunStorage.emplace_back(getLLCodeForSetDimStride(Arch));
      KeyToInfo[fun_of_SI32_SI32_SI32(Intrinsic::tpc_set_dim_stride, Arch)] =
          LinkInfo{GeneratedFunStorage.back(), "tpc_set_dim_stride"};

      for (const auto &X : {
               // clang-format off
               FunGenData{fun_SF32_of_SI32, getLLCodeForGetPadValueF32, "tpc_get_pad_value_float"},
               FunGenData{fun_SI32_of_SI32, getLLCodeForGetPadValueI32, "tpc_get_pad_value_i32"},
               FunGenData{fun_SI16_of_SI32, getLLCodeForGetPadValueI16, "tpc_get_pad_value_i16"},
               FunGenData{fun_SI8_of_SI32, getLLCodeForGetPadValueI8, "tpc_get_pad_value_i8"},
               // clang-format on
           }) {
        GeneratedFunStorage.emplace_back(X.LLGen(Arch));
        KeyToInfo[X.IKGen(Intrinsic::tpc_get_pad_value, Arch)] =
            LinkInfo{GeneratedFunStorage.back(), X.FuncName};
      }

      for (const auto &X : {
               // clang-format off
               FunGenData{fun_of_SI32_SF32, getLLCodeForSetPadValueF32, "tpc_set_pad_value_float"},
               FunGenData{fun_of_SI32_SI32, getLLCodeForSetPadValueI32, "tpc_set_pad_value_i32"},
               FunGenData{fun_of_SI32_SI16, getLLCodeForSetPadValueI16, "tpc_set_pad_value_i16"},
               FunGenData{fun_of_SI32_SI8, getLLCodeForSetPadValueI8, "tpc_set_pad_value_i8"},
               // clang-format on
           }) {
        GeneratedFunStorage.emplace_back(X.LLGen(Arch));
        KeyToInfo[X.IKGen(Intrinsic::tpc_set_pad_value, Arch)] =
            LinkInfo{GeneratedFunStorage.back(), X.FuncName};
      }

      GeneratedFunStorage.emplace_back(getLLCodeForGetSemaphoreValue(Arch));
      KeyToInfo[fun_SI32(Intrinsic::tpc_get_semaphore_value, Arch)] =
          LinkInfo{GeneratedFunStorage.back(), "tpc_get_semaphore_value"};

      GeneratedFunStorage.emplace_back(getLLCodeForSetSemaphoreValue(Arch));
      KeyToInfo[fun_of_SI32(Intrinsic::tpc_set_semaphore_value, Arch)] =
          LinkInfo{GeneratedFunStorage.back(), "tpc_set_semaphore_value"};

      GeneratedFunStorage.emplace_back(getLLCodeForGetCSRValue(Arch));
      KeyToInfo[fun_SI32(Intrinsic::tpc_get_csr_value, Arch)] =
          LinkInfo{GeneratedFunStorage.back(), "tpc_get_csr_value"};

      GeneratedFunStorage.emplace_back(getLLCodeForSetCSRValue(Arch));
      KeyToInfo[fun_of_SI32(Intrinsic::tpc_set_csr_value, Arch)] =
          LinkInfo{GeneratedFunStorage.back(), "tpc_set_csr_value"};

      GeneratedFunStorage.emplace_back(getLLCodeForGetConvertCSRValue(Arch));
      KeyToInfo[fun_SI32(Intrinsic::tpc_get_convert_csr_value, Arch)] =
          LinkInfo{GeneratedFunStorage.back(), "tpc_get_convert_csr_value"};

      GeneratedFunStorage.emplace_back(getLLCodeForSetConvertCSRValue(Arch));
      KeyToInfo[fun_of_SI32(Intrinsic::tpc_set_convert_csr_value, Arch)] =
          LinkInfo{GeneratedFunStorage.back(), "tpc_set_convert_csr_value"};
    }
    for (const ArchTy Arch : {ArchTy::Gaudi, ArchTy::GaudiB, ArchTy::Greco,
                              ArchTy::Gaudi2, ArchTy ::Doron1}) {

      GeneratedFunStorage.emplace_back(getLLCodeForGetPadValueBF16(Arch));
      KeyToInfo[fun_SBF16_of_SI32(Intrinsic::tpc_get_pad_value, Arch)] =
          LinkInfo{GeneratedFunStorage.back(), "tpc_get_pad_value_bf16"};

      GeneratedFunStorage.emplace_back(getLLCodeForSetPadValueBF16(Arch));
      KeyToInfo[fun_of_SI32_SBF16(Intrinsic::tpc_set_pad_value, Arch)] =
          LinkInfo{GeneratedFunStorage.back(), "tpc_set_pad_value_bf16"};
    }
    for (const ArchTy Arch : {ArchTy::Gaudi2, ArchTy ::Doron1}) {

      GeneratedFunStorage.emplace_back(getLLCodeForGetTensorHWPrefStride(Arch));
      KeyToInfo[fun_SI32_of_SI32(Intrinsic::tpc_get_tensor_hwpref_stride,
                                 Arch)] =
          LinkInfo{GeneratedFunStorage.back(), "tpc_get_tensor_hwpref_stride"};

      GeneratedFunStorage.emplace_back(getLLCodeForSetTensorHWPrefStride(Arch));
      KeyToInfo[fun_of_SI32_SI32(Intrinsic::tpc_set_tensor_hwpref_stride,
                                 Arch)] =
          LinkInfo{GeneratedFunStorage.back(), "tpc_set_tensor_hwpref_stride"};
    }

    // Gadui and Gadui2 F32 operators
    KeyToInfo[fun_VF32_of_VF32(Intrinsic::sin, ArchTy::Gaudi)] =
        LinkInfo{GaudiSinF32LL, "sin_f32"};
    KeyToInfo[fun_VF32_of_VF32(Intrinsic::cos, ArchTy::Gaudi)] =
        LinkInfo{GaudiCosF32LL, "cos_f32"};
    KeyToInfo[fun_VF32_of_VF32(Intrinsic::exp, ArchTy::Gaudi)] =
        LinkInfo{GaudiExpF32LL, "exp_cephes_f32"};
    KeyToInfo[fun_VF32_of_VF32(Intrinsic::log, ArchTy::Gaudi)] =
        LinkInfo{GaudiLogF32LL, "log_f32"};
    KeyToInfo[fun_VF32_of_VF32(Intrinsic::sqrt, ArchTy::Gaudi)] =
        LinkInfo{GaudiSqrtF32LL, "sqrt_f32"};
    KeyToInfo[fun_VF32_of_VF32(Intrinsic::tpc_tanh, ArchTy::Gaudi)] =
        LinkInfo{GaudiTanhF32LL, "tanh_f32"};
    KeyToInfo[fun_VF32_of_VF32(Intrinsic::tpc_rsqrt, ArchTy::Gaudi)] =
        LinkInfo{GaudiRSqrtF32LL, "rsqrt_f32"};
    KeyToInfo[fun_VF32_of_VF32(Intrinsic::tpc_reciprocal, ArchTy::Gaudi)] =
        LinkInfo{GaudiRecipF32LL, "reciprocal_f32"};
    KeyToInfo[fun_VF32_of_VF32(Intrinsic::tpc_reciprocal, ArchTy::Goya)] =
        LinkInfo{GoyaReciprocalF32LL, "reciprocal_cephes_f32"};

    KeyToInfo[fun_VF32_of_VF32(Intrinsic::sin, ArchTy::Gaudi2)] =
        LinkInfo{GaudiSinF32LL, "sin_f32"};
    KeyToInfo[fun_VF32_of_VF32(Intrinsic::cos, ArchTy::Gaudi2)] =
        LinkInfo{GaudiCosF32LL, "cos_f32"};
    KeyToInfo[fun_VF32_of_VF32(Intrinsic::exp, ArchTy::Gaudi2)] =
        LinkInfo{GaudiExpF32LL, "exp_cephes_f32"};
    KeyToInfo[fun_VF32_of_VF32(Intrinsic::log, ArchTy::Gaudi2)] =
        LinkInfo{GaudiLogF32LL, "log_f32"};
    KeyToInfo[fun_VF32_of_VF32(Intrinsic::sqrt, ArchTy::Gaudi2)] =
        LinkInfo{GaudiSqrtF32LL, "sqrt_f32"};
    KeyToInfo[fun_VF32_of_VF32(Intrinsic::tpc_tanh, ArchTy::Gaudi2)] =
        LinkInfo{GaudiTanhF32LL, "tanh_f32"};
    KeyToInfo[fun_VF32_of_VF32(Intrinsic::tpc_rsqrt, ArchTy::Gaudi2)] =
        LinkInfo{GaudiRSqrtF32LL, "rsqrt_f32"};
    KeyToInfo[fun_VF32_of_VF32(Intrinsic::tpc_reciprocal, ArchTy::Gaudi2)] =
        LinkInfo{GaudiRecipF32LL, "reciprocal_f32"};

    KeyToInfo[fun_SF32_of_VF32(Intrinsic::vector_reduce_mul,
                               ArchTy::Gaudi2)] =
        LinkInfo{Gaudi2_ReduceMulF32LL, "v_f32_reduce_mul"};

    KeyToInfo[fun_SF32_of_VF32(Intrinsic::vector_reduce_add,
                               ArchTy::Gaudi)] =
        LinkInfo{Gaudi_ReduceAddF32, "v_f32_reduce_add"};
    KeyToInfo[fun_SF32_of_VF32(Intrinsic::vector_reduce_mul,
                               ArchTy::Gaudi)] =
        LinkInfo{GaudiReduceMulF32LL, "v_f32_reduce_mul"};
    KeyToInfo[fun_SF32_of_VF32(Intrinsic::vector_reduce_fmax,
                               ArchTy::Gaudi)] =
        LinkInfo{GaudiReduceMaxF32LL, "v_f32_reduce_max"};
    KeyToInfo[fun_SF32_of_VF32(Intrinsic::vector_reduce_fmin,
                               ArchTy::Gaudi)] =
        LinkInfo{GaudiReduceMinF32LL, "v_f32_reduce_min"};

    // Gadui and Gadui2 bF16 operators
    KeyToInfo[fun_SBF16_of_VBF16(Intrinsic::vector_reduce_add,
                                 ArchTy::Gaudi)] =
        LinkInfo{GaudiReduceAddBF16LL, "v_bf16_reduce_add"};
    KeyToInfo[fun_SBF16_of_VBF16(Intrinsic::vector_reduce_fmax,
                                 ArchTy::Gaudi)] =
        LinkInfo{GaudiReduceMaxBF16LL, "v_bf16_reduce_max"};
    KeyToInfo[fun_SBF16_of_VBF16(Intrinsic::vector_reduce_fmin,
                                 ArchTy::Gaudi)] =
        LinkInfo{GaudiReduceMinBF16LL, "v_bf16_reduce_min"};
    KeyToInfo[fun_VBF16_of_VBF16(Intrinsic::sin, ArchTy::Gaudi)] =
        LinkInfo{GaudiSinBF16LL, "sin_bf16"};
    KeyToInfo[fun_VBF16_of_VBF16(Intrinsic::cos, ArchTy::Gaudi)] =
        LinkInfo{GaudiCosBF16LL, "cos_bf16"};
    KeyToInfo[fun_VBF16_of_VBF16(Intrinsic::exp, ArchTy::Gaudi)] =
        LinkInfo{GaudiExpBF16LL, "exp_bf16"};
    KeyToInfo[fun_VBF16_of_VBF16(Intrinsic::log, ArchTy::Gaudi)] =
        LinkInfo{GaudiLogBF16LL, "log_bf16"};
    KeyToInfo[fun_VBF16_of_VBF16(Intrinsic::sqrt, ArchTy::Gaudi)] =
        LinkInfo{GaudiSqrtBF16LL, "sqrt_bf16"};
    KeyToInfo[fun_VBF16_of_VBF16(Intrinsic::tpc_tanh, ArchTy::Gaudi)] =
        LinkInfo{GaudiTanhBF16LL, "tanh_bf16"};
    KeyToInfo[fun_VBF16_of_VBF16(Intrinsic::tpc_rsqrt, ArchTy::Gaudi)] =
        LinkInfo{GaudiRSqrtBF16LL, "rsqrt_bf16"};
    KeyToInfo[fun_VBF16_of_VBF16(Intrinsic::tpc_reciprocal, ArchTy::Gaudi)] =
        LinkInfo{GaudiRecipBF16LL, "reciprocal_bf16"};

    KeyToInfo[fun_VBF16_of_VBF16(Intrinsic::sin, ArchTy::Gaudi2)] =
        LinkInfo{GAUDI2_tpc_sin_bf16, "tpc_sin_bf16"};
    KeyToInfo[fun_VBF16_of_VBF16(Intrinsic::cos, ArchTy::Gaudi2)] =
        LinkInfo{GAUDI2_tpc_cos_bf16, "tpc_cos_bf16"};
    KeyToInfo[fun_VBF16_of_VBF16(Intrinsic::exp, ArchTy::Gaudi2)] =
        LinkInfo{GaudiExpBF16LL, "exp_bf16"};
    KeyToInfo[fun_VBF16_of_VBF16(Intrinsic::log, ArchTy::Gaudi2)] =
        LinkInfo{GaudiLogBF16LL, "log_bf16"};
    KeyToInfo[fun_VBF16_of_VBF16(Intrinsic::sqrt, ArchTy::Gaudi2)] =
        LinkInfo{GaudiSqrtBF16LL, "sqrt_bf16"};
    KeyToInfo[fun_VBF16_of_VBF16(Intrinsic::tpc_tanh, ArchTy::Gaudi2)] =
        LinkInfo{GaudiTanhBF16LL, "tanh_bf16"};
    KeyToInfo[fun_VBF16_of_VBF16(Intrinsic::tpc_rsqrt, ArchTy::Gaudi2)] =
        LinkInfo{GaudiRSqrtBF16LL, "rsqrt_bf16"};
    KeyToInfo[fun_VBF16_of_VBF16(Intrinsic::tpc_reciprocal, ArchTy::Gaudi2)] =
        LinkInfo{GAUDI2_tpc_reciprocal_bf16, "tpc_reciprocal_bf16"};

    KeyToInfo[fun_SF16_of_VF16(Intrinsic::vector_reduce_fmax,
                               ArchTy::Greco)] =
        LinkInfo{Goya2ReduceMaxF16LL, "v_f16_reduce_max"};
    KeyToInfo[fun_SF16_of_VF16(Intrinsic::vector_reduce_fmin,
                               ArchTy::Greco)] =
        LinkInfo{Goya2ReduceMinF16LL, "v_f16_reduce_min"};
    KeyToInfo[fun_SI16_of_VI16(Intrinsic::vector_reduce_smax,
                               ArchTy::Gaudi)] =
        LinkInfo{GaudiReduceMaxI16LL, "v_i16_reduce_max"};
    KeyToInfo[fun_SI16_of_VI16(Intrinsic::vector_reduce_smin,
                               ArchTy::Gaudi)] =
        LinkInfo{GaudiReduceMinI16LL, "v_i16_reduce_min"};
    KeyToInfo[fun_SI8_of_VI8(Intrinsic::vector_reduce_smax,
                             ArchTy::Gaudi)] =
        LinkInfo{GaudiReduceMaxI8LL, "v_i8_reduce_max"};
    KeyToInfo[fun_SI8_of_VI8(Intrinsic::vector_reduce_smin,
                             ArchTy::Gaudi)] =
        LinkInfo{GaudiReduceMinI8LL, "v_i8_reduce_min"};
    KeyToInfo[fun_SI8_of_VI8(Intrinsic::vector_reduce_umin,
                             ArchTy::Gaudi)] =
        LinkInfo{GaudiReduceMinU8LL, "v_u8_reduce_min"};
    KeyToInfo[fun_SI8_of_VI8(Intrinsic::vector_reduce_umax,
                             ArchTy::Gaudi)] =
        LinkInfo{GaudiReduceMaxU8LL, "v_u8_reduce_max"};

    // custom reduce + broadcast intrinsics.
    KeyToInfo[fun_VF32_of_VF32(Intrinsic::tpc_vector_reduce_add,
                               ArchTy::Gaudi2)] =
        LinkInfo{Gaudi2_ReduceAddF32, "v_f32_reduce_add"};

    KeyToInfo[fun_VF32_of_VF32(Intrinsic::tpc_vector_reduce_add,
                               ArchTy::Gaudi)] =
        LinkInfo{Gaudi_ReduceAddF32, "v_f32_reduce_add"};
    KeyToInfo[fun_VF32_of_VF32(Intrinsic::tpc_vector_reduce_mul,
                               ArchTy::Gaudi)] =
        LinkInfo{GaudiReduceMulF32LL, "v_f32_reduce_mul"};
    KeyToInfo[fun_VF32_of_VF32(Intrinsic::tpc_vector_reduce_mul,
                               ArchTy::Gaudi2)] =
        LinkInfo{Gaudi2_ReduceMulF32LL, "v_f32_reduce_mul"};
    KeyToInfo[fun_VF32_of_VF32(Intrinsic::tpc_vector_reduce_fmax,
                               ArchTy::Gaudi)] =
        LinkInfo{GaudiReduceMaxF32LL, "v_f32_reduce_max"};
    KeyToInfo[fun_VF32_of_VF32(Intrinsic::tpc_vector_reduce_fmin,
                               ArchTy::Gaudi)] =
        LinkInfo{GaudiReduceMinF32LL, "v_f32_reduce_min"};

    KeyToInfo[fun_VBF16_of_VBF16(Intrinsic::tpc_vector_reduce_add,
                                 ArchTy::Gaudi)] =
        LinkInfo{GaudiReduceAddBF16LL, "v_bf16_reduce_add"};
    KeyToInfo[fun_VBF16_of_VBF16(Intrinsic::tpc_vector_reduce_fmax,
                                 ArchTy::Gaudi)] =
        LinkInfo{GaudiReduceMaxBF16LL, "v_bf16_reduce_max"};
    KeyToInfo[fun_VBF16_of_VBF16(Intrinsic::tpc_vector_reduce_fmin,
                                 ArchTy::Gaudi)] =
        LinkInfo{GaudiReduceMinBF16LL, "v_bf16_reduce_min"};

    KeyToInfo[fun_VF16_of_VF16(Intrinsic::tpc_vector_reduce_fmax,
                               ArchTy::Greco)] =
        LinkInfo{Goya2ReduceMaxF16LL, "v_f16_reduce_max"};
    KeyToInfo[fun_VF16_of_VF16(Intrinsic::tpc_vector_reduce_fmin,
                               ArchTy::Greco)] =
        LinkInfo{Goya2ReduceMinF16LL, "v_f16_reduce_min"};

    KeyToInfo[fun_VI16_of_VI16(Intrinsic::tpc_vector_reduce_smax,
                               ArchTy::Gaudi)] =
        LinkInfo{GaudiReduceMaxI16LL, "v_i16_reduce_max"};
    KeyToInfo[fun_VI16_of_VI16(Intrinsic::tpc_vector_reduce_smin,
                               ArchTy::Gaudi)] =
        LinkInfo{GaudiReduceMinI16LL, "v_i16_reduce_min"};

    KeyToInfo[fun_VI8_of_VI8(Intrinsic::tpc_vector_reduce_smax,
                             ArchTy::Gaudi)] =
        LinkInfo{GaudiReduceMaxI8LL, "v_i8_reduce_max"};
    KeyToInfo[fun_VI8_of_VI8(Intrinsic::tpc_vector_reduce_smin,
                             ArchTy::Gaudi)] =
        LinkInfo{GaudiReduceMinI8LL, "v_i8_reduce_min"};

    KeyToInfo[fun_VI8_of_VI8(Intrinsic::tpc_vector_reduce_umax,
                             ArchTy::Gaudi)] =
        LinkInfo{GaudiReduceMaxU8LL, "v_u8_reduce_max"};
    KeyToInfo[fun_VI8_of_VI8(Intrinsic::tpc_vector_reduce_umin,
                             ArchTy::Gaudi)] =
        LinkInfo{GaudiReduceMinU8LL, "v_u8_reduce_min"};

    // Gadui2
    // sqrt_f16
    KeyToInfo[fun_VF16_of_VF16(Intrinsic::sqrt, ArchTy::Gaudi2)] =
        LinkInfo{Gaudi2SqrtF16LL, "sqrt_f16"};

    // Gadui2
    // rsqrt_f16
    KeyToInfo[fun_VF16_of_VF16(Intrinsic::tpc_rsqrt, ArchTy::Gaudi2)] =
        LinkInfo{Gaudi2RSqrtF16LL, "rsqrt_f16"};


    // Sin F16
    KeyToInfo[fun_VF16_of_VF16(Intrinsic::sin, ArchTy::Gaudi2)] =
        LinkInfo{GAUDI2_tpc_sin_f16, "tpc_sin_f16"};

    // Tanh F16
    KeyToInfo[fun_VF16_of_VF16(Intrinsic::tpc_tanh, ArchTy::Gaudi2)] =
        LinkInfo{GAUDI2_tpc_tanh_f16, "tpc_tanh_f16"};

    // cos F16
    KeyToInfo[fun_VF16_of_VF16(Intrinsic::cos, ArchTy::Gaudi2)] =
        LinkInfo{GAUDI2_tpc_cos_f16, "tpc_cos_f16"};

       // exp F16
    KeyToInfo[fun_VF16_of_VF16(Intrinsic::exp, ArchTy::Gaudi2)] =
        LinkInfo{GAUDI2_tpc_exp_f16, "tpc_exp_f16"};

    // log F16
    KeyToInfo[fun_VF16_of_VF16(Intrinsic::log, ArchTy::Gaudi2)] =
        LinkInfo{GAUDI2_tpc_log_f16, "tpc_log_f16"};


    // reciprocal F16
    KeyToInfo[fun_VF16_of_VF16(Intrinsic::tpc_reciprocal, ArchTy::Gaudi2)] =
        LinkInfo{GAUDI2_tpc_reciprocal_f16, "tpc_reciprocal_f16"};

    // cast i32 to i8 (signless)
    KeyToInfo[fun_VI8_of_VI32_VI32_VI32_VI32(Intrinsic::tpc_trunc_i32_to_i8,
                                             ArchTy::Gaudi)] =
        LinkInfo{GaudiTruncI32I8LL, "trunc_i32_to_i8"};

    // swizzle f32
    KeyToInfo[fun_VF32_X2_of_VF32_X2(Intrinsic::tpc_swizzle, ArchTy::Gaudi)] =
        LinkInfo{GaudiSwizzleX2F32LL, "swizzle_2xf32"};

    // swizzle f32
    KeyToInfo[fun_VF32_X2_of_VF32_X2(Intrinsic::tpc_swizzle, ArchTy::Gaudi2)] =
        LinkInfo{GaudiSwizzleX2F32LL, "swizzle_2xf32"};

    // swizzle f32
    KeyToInfo[fun_VF32_X2_of_VF32_X2(Intrinsic::tpc_swizzle, ArchTy::Greco)] =
        LinkInfo{GaudiSwizzleX2F32LL, "swizzle_2xf32"};

    // swizzle bf16
    KeyToInfo[fun_VBF16_X2_of_VBF16_X2(Intrinsic::tpc_swizzle, ArchTy::Gaudi)] =
        LinkInfo{GaudiSwizzleX2BF16LL, "swizzle_2xbf16"};

    // swizzle bf16
    KeyToInfo[fun_VBF16_X2_of_VBF16_X2(Intrinsic::tpc_swizzle, ArchTy::Gaudi2)] =
        LinkInfo{GaudiSwizzleX2BF16LL, "swizzle_2xbf16"};

    // swizzle bf16
    KeyToInfo[fun_VBF16_X2_of_VBF16_X2(Intrinsic::tpc_swizzle, ArchTy::Greco)] =
        LinkInfo{GaudiSwizzleX2BF16LL, "swizzle_2xbf16"};

    // swizzle f32 x 4
    KeyToInfo[fun_VF32_X4_of_VF32_X4(Intrinsic::tpc_swizzle, ArchTy::Gaudi)] =
        LinkInfo{GaudiSwizzleX4F32LL, "swizzle_4xf32"};

    // swizzle f32 x 4
    KeyToInfo[fun_VF32_X4_of_VF32_X4(Intrinsic::tpc_swizzle, ArchTy::Gaudi2)] =
        LinkInfo{GaudiSwizzleX4F32LL, "swizzle_4xf32"};

    // swizzle f32 x 4
    KeyToInfo[fun_VF32_X4_of_VF32_X4(Intrinsic::tpc_swizzle, ArchTy::Greco)] =
        LinkInfo{GaudiSwizzleX4F32LL, "swizzle_4xf32"};

    // gen_stage1_v64f32
    for (const auto &ArchAndCode : {
             std::make_pair(ArchTy::Gaudi, GenStage1V64F32Gaudi),
             std::make_pair(ArchTy::GaudiB, GenStage1V64F32GaudiB),
             std::make_pair(ArchTy::Greco, GenStage1V64F32Greco),
             std::make_pair(ArchTy::Gaudi2, GenStage1V64F32Gaudi2),
             std::make_pair(ArchTy::Doron1, GenStage1V64F32Doron1),
         }) {
      KeyToInfo[fun_VF32_of_SI32(Intrinsic::tpc_gen_stage1_seed_v64f32,
                                 ArchAndCode.first)] =
          LinkInfo{ArchAndCode.second, "tpc_gen_stage1_seed_v64f32"};
    }
  }

  virtual ~LinkTPCHeadersLegacyPass() {}
  void getAnalysisUsage(AnalysisUsage &AU) const override {}
  Optional<IntrinsicKey> getIntrinsicKey(const IntrinsicInst &II);
  bool instructionNeedsLinking(const Instruction &I, StringRef &IRStr);
  bool replaceWithCall(Instruction *I);
  bool replaceWithCallDynamic(Instruction *I);

  bool callInstructionToSearch(const Instruction &I, StringRef &IRStr);
  bool runOnModule(Module &M) override;
};
} // namespace

static inline unsigned getTypeVal(Type *Ty) {
  Type::TypeID ID = Ty->getScalarType()->getTypeID();
  unsigned VecLen =
      Ty->isVectorTy() ? dyn_cast<VectorType>(Ty)->getElementCount().getValue() : 0;
  unsigned BitW = ID == Type::IntegerTyID ? Ty->getScalarSizeInBits() : 0;
  return TypeKey(ID, VecLen, BitW);
}

Optional<IntrinsicKey>
LinkTPCHeadersLegacyPass::getIntrinsicKey(const IntrinsicInst &II) {
  auto It = ArchMap.find(SubArchName);
  if (It == ArchMap.end())
    return None; // Unsupported.

  IntrinsicKey Key;
  Key.IntrinsicID = II.getIntrinsicID();
  Key.ResTyKey = getTypeVal(II.getType());
  for (Value *Arg : II.arg_operands())
    Key.ArgsTyKeys.push_back(getTypeVal(Arg->getType()));
  Key.Arch = It->second;
  return Key;
}

static bool IsReduction(Instruction *I) {
  if (IntrinsicInst *II = dyn_cast<IntrinsicInst>(I)) {
    Intrinsic::ID Inid = II->getIntrinsicID();
    if ((Inid == Intrinsic::vector_reduce_add) ||
        (Inid == Intrinsic::vector_reduce_mul) ||
        (Inid == Intrinsic::vector_reduce_fadd) ||
        (Inid == Intrinsic::vector_reduce_fmul) ||
        (Inid == Intrinsic::vector_reduce_fmax) ||
        (Inid == Intrinsic::vector_reduce_fmin) ||
        (Inid == Intrinsic::vector_reduce_smax) ||
        (Inid == Intrinsic::vector_reduce_smin) ||
        (Inid == Intrinsic::vector_reduce_umin) ||
        (Inid == Intrinsic::vector_reduce_umax)) {
      return true;
    }
  }
  return false;
}

/*static bool isCustomReduction(IntrinsicInst *I) {
  Intrinsic::ID Inid = I->getIntrinsicID();
  if ((Inid == Intrinsic::tpc_vector_reduce_add) ||
      (Inid == Intrinsic::tpc_vector_reduce_mul) ||
      (Inid == Intrinsic::tpc_vector_reduce_fmax) ||
      (Inid == Intrinsic::tpc_vector_reduce_fmin) ||
      (Inid == Intrinsic::tpc_vector_reduce_smax) ||
      (Inid == Intrinsic::tpc_vector_reduce_smin) ||
      (Inid == Intrinsic::tpc_vector_reduce_umin) ||
      (Inid == Intrinsic::tpc_vector_reduce_umax))
    return true;
  return false;
}*/

/**
 * @brief The function works on each call instruction. In the case
 * function find a match between the function name and symbol from a file,
 * the function fill the StringRef with a string from the symbol of the file.
 *
 * @param I general instruction.
 * @param IRStr reference to IRString contain a copy of the target function.
 * @return true find a match
 * @return false other wise
 */
bool LinkTPCHeadersLegacyPass::callInstructionToSearch(const Instruction &I,
                                                       StringRef &IRStr) {
  // IF user didn't pass any file in the driver.
  if (!Query)
    return false;
  // Check first if instruction is a call instruction.
  if (const CallInst *II = dyn_cast<CallInst>(&I)) {
    std::string CallName = std::string(II->getCalledFunction()->getName());
    // tpc function namespace begin with tpc_
    if (CallName.find("tpc_") != 0)
      return false;
    else {
      // This is tpc_ namespace
      FnHandle SymbolValue;
      // According to the convention between LLVM and TPC_KERNEL. The string
      // wrapper is a another symbol named {GAUDI|GOYA2|GAUDI2}_tpc_*_Func
      CallName = SubArchName.upper() + "_" + CallName + "_Func";
      // There is a function with above name in the so.
      SymbolValue = GetLibFunction(Query, CallName.c_str());
      typedef int (*pFunc)(const char **);
      const char *Ptr;
      int Val = ((pFunc)(SymbolValue))(&Ptr);
      IRStr = StringRef(Ptr, Val);
      return true;
    }
  }
  return false;
}

bool LinkTPCHeadersLegacyPass::instructionNeedsLinking(const Instruction &I,
                                                       StringRef &IRStr) {
  if (const IntrinsicInst *II = dyn_cast<IntrinsicInst>(&I)) {
    const Optional<IntrinsicKey> Key = getIntrinsicKey(*II);
    if (!Key.hasValue())
      return false;

    auto It = KeyToInfo.find(Key.getValue());
    if (It == KeyToInfo.end())
      return false;

    // If the special function has already been linked.
    if (LinkedIRs.find(It->second.LinkCode) != LinkedIRs.end()) {
      ReplacementList.push_back(const_cast<Instruction *>(&I));
      return false;
    }

    LinkedIRs.insert(It->second.LinkCode);
    IRStr = It->second.LinkCode;
    return true;
  }
  return false;
}

/**
 * @brief The function replace all call with a inline function.
 *
 * @param I general instruction.
 * @return true find a match and replace was done
 * @return false other wise
 */
bool LinkTPCHeadersLegacyPass::replaceWithCallDynamic(Instruction *I) {
  Module *M = I->getModule();
  if (CallInst *II = dyn_cast<CallInst>(I)) {
    std::string callName = std::string(II->getCalledFunction()->getName());
    Function *Special = M->getFunction(callName);
    assert(Special && "The special function should have been linked.");
    // Set available externally attribute for the special function as it doesn't
    // link using the link API.
    Special->setLinkage(GlobalValue::AvailableExternallyLinkage);

    IRBuilder<> Builder(I);
    SmallVector<Value *, 2> Args;
    for (Value *Val : II->args())
      Args.push_back(Val);

    auto *SpecialCall =
        Builder.CreateCall(Special->getFunctionType(), Special, Args);
    LLVM_DEBUG(dbgs() << "\nReplacing instruction \n"; I->dump();
               dbgs() << "with function call \n"; SpecialCall->dump(););

    I->replaceAllUsesWith(SpecialCall);
    I->eraseFromParent();
    return true;
  }
  return false;
}

bool LinkTPCHeadersLegacyPass::replaceWithCall(Instruction *I) {
  Module *M = I->getModule();
  if (IntrinsicInst *II = dyn_cast<IntrinsicInst>(I)) {
    const Optional<IntrinsicKey> Key = getIntrinsicKey(*II);

    assert(Key.hasValue()); // Already checked at "instructionNeedsLinking".

    auto It = KeyToInfo.find(Key.getValue());
    assert(It != KeyToInfo.end()); // Already checked at "instructionNeedsLinking".

    Function *Special = M->getFunction(It->second.FuncName);
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

static void expandSpecialCaseLLVMIR(Function *Main, ArchTy Arch) {
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

/**
 * @brief The function link an IRFStr function to the module.
 *
 * @param M module.
 * @param IRStr reference to IRString contain a copy of the target function.
 * @return true if function linked successfully, false otherwise.
 */
bool LinkTPCHeadersLegacyPass::linkStage(Module &M, StringRef IRStr) {
  LLVMContext &Ctx = M.getContext();
  SMDiagnostic Err;

  auto ParsedModule = parseIR(MemoryBufferRef(IRStr, "Special"), Err, Ctx);
  if (!ParsedModule){
    Err.print("Special function module parsing failed with error:", errs(), true, true);
    return false;
  }

  LLVM_DEBUG(dbgs() << "\nSpecial function module parsing successful.");
  LLVM_DEBUG(dbgs() << "\nLinked functions : \n"; for (Function &F
                                                       : *ParsedModule) dbgs()
                                                  << F.getName() << "\n");
  ParsedModule->setDataLayout(M.getDataLayout());

  // Link modules.
  bool linkFailed = Linker::linkModules(M, std::move(ParsedModule));

  // Linker::linkModules returns true in case of failure
  return !linkFailed;
}

bool LinkTPCHeadersLegacyPass::runOnModule(Module &M) {
  if (skipModule(M))
    return false;

  llvm::StringRef IRStr;
  Function *Main = M.getFunction("main");
  if (!Main)
    return false;
  Attribute SubArchAttr = Main->getFnAttribute("target-cpu");
  auto SubArchStr = SubArchAttr.getValueAsString();
  if (!SubArchStr.empty())
    SubArchName = SubArchStr;
  auto It = ArchMap.find(SubArchName);
  if (It == ArchMap.end())
    return false; // Unsupported.
  ArchTy Arch = It->second;

  // Expand the IR to appropriate form.
  expandSpecialCaseLLVMIR(Main, Arch);

  // Identify the modules that need to be linked.
  SmallVector<Instruction *, 8> ReplacementListSO;
  for (auto &BB : *Main) {
    for (auto &I : BB) {
      if (callInstructionToSearch(I, IRStr)){
        ReplacementListSO.push_back(&I);
        if (!linkStage(M,IRStr))
          report_fatal_error("LTO pass failed to link a dynamic LTO function");
      }
      else if (instructionNeedsLinking(I, IRStr)) {
        ReplacementList.push_back(&I);
        if (!linkStage(M,IRStr))
          report_fatal_error("LTO pass failed to link a static LTO function");
      }
    }
  }

  if (ReplacementList.empty() && ReplacementListSO.empty())
    return false;

  // Replace the instruction with call.
  for (Instruction *I : ReplacementList) {
    if (!replaceWithCall(I))
      report_fatal_error("Static LTO replacememt failed");
  }

  for (auto I : ReplacementListSO){
    if (!replaceWithCallDynamic(I))
      report_fatal_error("Dynamic LTO replacement failed");
  }
  return true;
}

char LinkTPCHeadersLegacyPass::ID = 0;
INITIALIZE_PASS(LinkTPCHeadersLegacyPass, "link-tpc-headers",
                "Link TPC Headers", false, false)

Pass *llvm::createLinkTPCHeadersLegacyPass() {
  return new LinkTPCHeadersLegacyPass();
}

/**
 * @brief load lib load a share object.
 * @param name a path to the share object.
 * @return llvm::FnHandle void * type indicate the handler pointer to referent
 * to.
 */
llvm::LibHandle llvm::LoadLib(const char *path) {
  char *Error;
  LibHandle LHandle = dlopen(path, RTLD_LAZY);
  if ((Error = dlerror()) != NULL) {
    fputs(Error, stderr);
    fputc('\n', stderr);
    assert(false && "Could not open lib");
  }
  return LHandle;
}

/**
 * @brief Get the Lib Function object
 * The function returns a symbol pointer.
 * This pointer allow to access to the symbol that is a function or variable.
 * @param handle: void * type indicate the handler pointer to referent to.
 * @param name:  symbolName to query.
 * @return FnHandle:  void * type indicate the handler pointer to referent to.
 */
llvm::FnHandle llvm::GetLibFunction(LibHandle MLibHandle,
                                    const char *symbolName) {
  char *Error;
  FnHandle FHandle = dlsym(MLibHandle, symbolName);
  if ((Error = dlerror()) != NULL) {
    fputs(Error, stderr);
    fputc('\n', stderr);
    assert(false && "Could not find symbol.");
  }
  return FHandle;
}
#endif
