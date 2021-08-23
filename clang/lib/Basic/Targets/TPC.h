//===--- TPC.h - Declare TPC target feature support -------------*- C++ -*-===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//
//
// This file declares TPC TargetInfo objects.
//
//===----------------------------------------------------------------------===//

#ifndef LLVM_CLANG_LIB_BASIC_TARGETS_TPC_H
#define LLVM_CLANG_LIB_BASIC_TARGETS_TPC_H

#include "clang/Basic/TargetInfo.h"
#include "clang/Basic/TargetOptions.h"
#include "llvm/ADT/StringSwitch.h"
#include "llvm/ADT/Triple.h"
#include "llvm/Support/Compiler.h"
#include "llvm/Target/TargetInfoTPC.h"

namespace clang {
namespace targets {

#ifdef LLVM_TPC_COMPILER

static LangASMap TPCAddrSpaceMap = {
  0, // Default
  1, // opencl_global
  0, // opencl_local
  0, // opencl_constant
  0, // opencl_private
  0, // opencl_generic
  0, // cuda_device
  0, // cuda_constant
  1, // cuda_shared
  0, // ptr32_sptr
  0, // ptr32_uptr
  0  // ptr64
};


class TPCTargetInfo : public TargetInfo {
  enum class CPUKind {
    Generic,
    Goya,
    Gaudi
  } CPU = CPUKind::Goya;

  CPUKind getCPUKind(StringRef CPU) const;
  bool checkCPUKind(CPUKind Kind) const;

public:

  bool isValidCPUName(StringRef Name) const override;
  void fillValidCPUList(SmallVectorImpl<StringRef> &Values) const override;
  bool setCPU(const std::string &Name) override;

private:
  static const Builtin::Info BuiltinInfo[];

public:
  TPCTargetInfo(const llvm::Triple &Triple, const TargetOptions &TO);

  void getTargetDefines(const LangOptions &Opts,
                        MacroBuilder &Builder) const override;
  ArrayRef<Builtin::Info> getTargetBuiltins() const override;
  bool initFeatureMap(llvm::StringMap<bool> &Features, DiagnosticsEngine &Diags,
                      StringRef CPU,
                      const std::vector<std::string> &FeaturesVec) const override;
  bool hasFeature(StringRef Feature) const override;
  bool handleTargetFeatures(std::vector<std::string> &Features,
                            DiagnosticsEngine &Diags) override;
  ArrayRef<const char *> getGCCRegNames() const override;
  ArrayRef<TargetInfo::GCCRegAlias> getGCCRegAliases() const override;
  const char *getClobbers() const override;
  bool validateAsmConstraint(const char *&Name,
                             TargetInfo::ConstraintInfo &Info) const override;

  BuiltinVaListKind getBuiltinVaListKind() const override {
    return TargetInfo::CharPtrBuiltinVaList;
  }
};
#endif
} // namespace targets
} // namespace clang
#endif // LLVM_CLANG_LIB_BASIC_TARGETS_TCE_H
