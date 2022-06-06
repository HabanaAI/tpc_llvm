//===--- TPC.h - Declare TPC target feature support -------------*- C++ -*-===//
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

class LLVM_LIBRARY_VISIBILITY TPCTargetInfo : public TargetInfo {
  enum class CPUKind {
    Generic,
    Goya,
    Gaudi,
    GaudiB,
    Greco,
    Gaudi2,
    Doron1
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

  BuiltinVaListKind getBuiltinVaListKind() const override;
  const char *getBFloat16Mangling() const override;
};
} // namespace targets
} // namespace clang
#endif // LLVM_CLANG_LIB_BASIC_TARGETS_TPC_H
