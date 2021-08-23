//===--- TPC.h - TPC ToolChain Implementations ------------------*- C++ -*-===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//
//
//===----------------------------------------------------------------------===//

#ifndef LLVM_CLANG_LIB_DRIVER_TOOLCHAINS_TPC_H
#define LLVM_CLANG_LIB_DRIVER_TOOLCHAINS_TPC_H

#include "Gnu.h"
#include "clang/Driver/Tool.h"
#include "clang/Driver/ToolChain.h"

namespace clang {
namespace driver {

namespace toolchains {

#ifdef LLVM_TPC_COMPILER
class LLVM_LIBRARY_VISIBILITY TPCToolChain : public Generic_ELF {
protected:
public:
  TPCToolChain(const Driver &D, const llvm::Triple &Triple,
               const llvm::opt::ArgList &Args);
  bool IsIntegratedAssemblerDefault() const override { return true; }
  Tool *buildLinker() const override;
};
#endif

} // end namespace toolchains
} // end namespace driver
} // end namespace clang

#endif // LLVM_CLANG_LIB_DRIVER_TOOLCHAINS_TPC_H
