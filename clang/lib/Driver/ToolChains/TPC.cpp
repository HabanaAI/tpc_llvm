//===--- TPC.cpp - TPC ToolChain Implementations ----------------*- C++ -*-===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//
//
//===----------------------------------------------------------------------===//

#include "TPC.h"
#include "CommonArgs.h"
#include "clang/Driver/DriverDiagnostic.h"

using namespace clang::driver;
using namespace clang::driver::toolchains;
using namespace clang;
using namespace llvm::opt;

#ifdef LLVM_TPC_COMPILER
TPCToolChain::TPCToolChain(const Driver &D, const llvm::Triple &Triple,
                           const ArgList &Args)
  : Generic_ELF(D, Triple, Args) {
}

Tool *TPCToolChain::buildLinker() const {
  getDriver().Diag(diag::err_drv_link_unavailable);
  return nullptr;
}
#endif
