//===--- TPC.h - TPC-specific Tool Helpers ----------------------*- C++ -*-===//
//
//===----------------------------------------------------------------------===//

#ifndef LLVM_CLANG_LIB_DRIVER_TOOLCHAINS_ARCH_TPC_H
#define LLVM_CLANG_LIB_DRIVER_TOOLCHAINS_ARCH_TPC_H

#include "clang/Driver/Driver.h"
#include "llvm/ADT/StringRef.h"
#include "llvm/ADT/Triple.h"
#include "llvm/Option/Option.h"
#include <string>
#include <vector>

namespace clang {
namespace driver {
namespace tools {
namespace tpc {

const char *getTPCTargetCPU(const llvm::opt::ArgList &Args);

void getTPCTargetFeatures(const Driver &D, const llvm::opt::ArgList &Args,
                          std::vector<llvm::StringRef> &Features);

} // end namespace tpc
} // end namespace target
} // end namespace driver
} // end namespace clang

#endif // LLVM_CLANG_LIB_DRIVER_TOOLCHAINS_ARCH_TPC_H
