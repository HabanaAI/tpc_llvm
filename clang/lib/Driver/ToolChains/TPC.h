//===--- TPC.h - TPC ToolChain Implementations ------------------*- C++ -*-===//
//
//===----------------------------------------------------------------------===//

#ifndef LLVM_CLANG_LIB_DRIVER_TOOLCHAINS_TPC_H
#define LLVM_CLANG_LIB_DRIVER_TOOLCHAINS_TPC_H

#include "Gnu.h"
#include "clang/Driver/Tool.h"
#include "clang/Driver/ToolChain.h"

namespace clang {
namespace driver {

/// Add TPC-specific options common for 'clang' and 'clang-as'.
void addCommonTPCTargetArgs(const llvm::opt::ArgList &Args,
                            llvm::opt::ArgStringList &CmdArgs);

namespace toolchains {

class LLVM_LIBRARY_VISIBILITY TPCToolChain : public Generic_ELF {
protected:
public:
  TPCToolChain(const Driver &D, const llvm::Triple &Triple,
               const llvm::opt::ArgList &Args);
  bool IsIntegratedAssemblerDefault() const override { return true; }
  Tool *buildLinker() const override;
  void
    addClangTargetOptions(const llvm::opt::ArgList &DriverArgs,
                          llvm::opt::ArgStringList &CC1Args,
                          Action::OffloadKind DeviceOffloadKind) const override;
};

} // end namespace toolchains
} // end namespace driver
} // end namespace clang

#endif // LLVM_CLANG_LIB_DRIVER_TOOLCHAINS_TPC_H
