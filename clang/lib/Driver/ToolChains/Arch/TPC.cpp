//===--- TPC.cpp - TPC Helpers for Tools ------------------------*- C++ -*-===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//
//
//===----------------------------------------------------------------------===//
#include "TPC.h"
#include "ToolChains/CommonArgs.h"
#include "clang/Driver/Driver.h"
#include "clang/Driver/DriverDiagnostic.h"
#include "clang/Driver/Options.h"
#include "llvm/ADT/StringSwitch.h"
#include "llvm/Option/ArgList.h"

using namespace clang::driver;
using namespace clang::driver::tools;
using namespace clang;
using namespace llvm::opt;

const char *tpc::getTPCTargetCPU(const ArgList &Args,
                                 const llvm::Triple &Triple) {
  StringRef CPU;
  if (const Arg *A = Args.getLastArg(options::OPT_march_EQ))
    CPU = A->getValue();
  else if (const Arg *A = Args.getLastArg(options::OPT_mtune_EQ))
    CPU = A->getValue();
  else if (const Arg *A = Args.getLastArg(options::OPT_mcpu_EQ))
    CPU = A->getValue();

  if (CPU.equals("dali"))
    return "goya";
  if (!CPU.empty())
    return CPU.data();

  return "goya";
}

void tpc::getTPCTargetFeatures(const Driver &D, const ArgList &Args,
                               std::vector<StringRef> &Features) {
}
