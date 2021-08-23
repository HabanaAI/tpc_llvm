//===-- MemoryManager.h -----------------------------------------*- C++ -*-===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//
//
//===----------------------------------------------------------------------===//

#ifndef LLVM_CLANG_TOOLS_LLVM_TPC_MEMORYMANAGER_H
#define LLVM_CLANG_TOOLS_LLVM_TPC_MEMORYMANAGER_H

#include "llvm/Support/raw_ostream.h"

namespace llvm {
namespace tpc {

/// The class sets the global allocation heap as the current thread's allocation
/// heap, in the constructor, and restores the previous in the destructor.
class ScopedGlobalAllocator {
  void *newHeap;
  void *oldHeap;

public:
  ScopedGlobalAllocator();
  ~ScopedGlobalAllocator();

  operator bool() { return newHeap != nullptr; }
};

/// A raw buffer implementation of `raw_pwrite_stream`, using the global
/// allocation heap.
class GlobalBufOStream : public raw_pwrite_stream {
  char *&buf;
  unsigned &size;
  unsigned capacity;

  /// See raw_ostream::write_impl.
  void write_impl(const char *ptr, size_t len) override;

  void pwrite_impl(const char *ptr, size_t len, uint64_t offset) override {
    memcpy(buf + offset, ptr, len);
  }

  /// Return the current position within the stream.
  uint64_t current_pos() const override { return size; }

public:
  GlobalBufOStream(char *&buf, unsigned &size);
  ~GlobalBufOStream() override { flush(); }

  void reset() {
    buf = nullptr;
    size = 0;
    capacity = 0;
  }
};

} // end namespace tpc
} // end namespace llvm

#endif // LLVM_CLANG_TOOLS_LLVM_TPC_MEMORYMANAGER_H
