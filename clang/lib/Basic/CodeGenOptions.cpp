//===--- CodeGenOptions.cpp -----------------------------------------------===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//

#include "clang/Basic/CodeGenOptions.h"
#include <string.h>

namespace clang {

CodeGenOptions::CodeGenOptions() {
#define CODEGENOPT(Name, Bits, Default) Name = Default;
#define ENUM_CODEGENOPT(Name, Type, Bits, Default) set##Name(Default);
#include "clang/Basic/CodeGenOptions.def"

  RelocationModel = llvm::Reloc::PIC_;
  memcpy(CoverageVersion, "402*", 4);
#ifdef LLVM_TPC_COMPILER
  // see PRM 1.3.10. Memory regions
  ScalarLocalMemory = 1024;
  VectorLocalMemory = 0;
  SpillVlm = 0;
  MarkAsUsingPrintf = false;
  AllLoopsTaken = false;
  RegMemCount = false;
  LutWarn = true;
  LongIRF = false;
#endif
}

bool CodeGenOptions::isNoBuiltinFunc(const char *Name) const {
  StringRef FuncName(Name);
  for (unsigned i = 0, e = NoBuiltinFuncs.size(); i != e; ++i)
    if (FuncName.equals(NoBuiltinFuncs[i]))
      return true;
  return false;
}

}  // end namespace clang
