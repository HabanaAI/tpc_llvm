//===--- EmbeddedTpcHeaders.cpp -------------------------------------------===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//
//
//  This file contains content of embedded headers.
//
//===----------------------------------------------------------------------===//

#include "clang/Frontend/CompilerInvocation.h"
#include "llvm/ADT/STLExtras.h"
#include <memory>
using namespace clang;
using namespace llvm;

static const char ContentOf_tpcdefs[] = {
#include "tpc-defs.h.inc"
};

static MemoryBuffer *BufferOf_tpc_defs
    = MemoryBuffer::getMemBuffer(ContentOf_tpcdefs, "tpc-defs.h").release();

static const char ContentOf_reduction[] = {
#include "tpc-reduction_functions_core.h.inc"
};

static MemoryBuffer *BufferOf_reduction
    = MemoryBuffer::getMemBuffer(ContentOf_reduction, "tpc-reduction_functions_core.h").release();

static const char ContentOf_tpc_special[] = {
#include "tpc-special.h.inc"
};

static MemoryBuffer *BufferOf_tpc_special
    = MemoryBuffer::getMemBuffer(ContentOf_tpc_special, "tpc-special.h").release();

static const CompilerInvocation::EmbeddedFile EmbeddedFiles[] = {
  CompilerInvocation::EmbeddedFile { "tpc-defs.h", BufferOf_tpc_defs },
  CompilerInvocation::EmbeddedFile { "tpc-reduction_functions_core.h", BufferOf_reduction },
  CompilerInvocation::EmbeddedFile { "tpc-special.h", BufferOf_tpc_special }
};

ArrayRef<CompilerInvocation::EmbeddedFile> CompilerInvocation::getEmbeddedFiles() {
  return ArrayRef<CompilerInvocation::EmbeddedFile>(
      EmbeddedFiles, array_lengthof(EmbeddedFiles));
}
