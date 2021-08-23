//===- Disassembler.cpp - TPC LLVM disassembler entry point ---------------===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//
//
//===----------------------------------------------------------------------===//

#include "API.h"
#include "MemoryManager.h"

static char *tpcAsmString = nullptr;
static unsigned tpcAsmSize = 0;
static llvm::tpc::GlobalBufOStream tpcAsmStream(tpcAsmString, tpcAsmSize);
static llvm::raw_ostream &getTPCAsmStream() { return tpcAsmStream; }

#define main objdump_main
#define outs getTPCAsmStream
#include "../../../llvm/tools/llvm-objdump/llvm-objdump.cpp"

using namespace llvm;
using namespace llvm::tpc;

static unsigned takeTPCAsmString(char **tpcAsm) {
  tpcAsmStream.flush();
  unsigned size = tpcAsmSize;
  *tpcAsm = tpcAsmString;
  tpcAsmStream.reset();
  return size;
}

unsigned llvm_tpc_disassembleTPC(const char *elfBin, unsigned elfSize,
                                 char **tpcAsm) {
  auto binOrErr = createBinary(MemoryBufferRef({elfBin, elfSize}, {}));
  if (!binOrErr) {
    handleAllErrors(binOrErr.takeError(), [](const ErrorInfoBase &errInfo) {
      auto &os = WithColor::error(errs());
      errInfo.log(os);
      os << '\n';
    });
    return 0;
  }

  auto *binary = cast<ObjectFile>(binOrErr->get());

  TripleName = "tpc";
  NoShowRawInsn = true;
  NoLeadingAddr = true;
  FilterSections.addValue(".text");

  disassembleObject(binary, Relocations);
  return takeTPCAsmString(tpcAsm);
}
