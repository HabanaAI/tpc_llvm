//===- MemoryManager.cpp - Compiler memory management related code --------===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//
//
//===----------------------------------------------------------------------===//

#include "MemoryManager.h"
#include "API.h"
#include <atomic>
#include <cstddef>

static HeapAllocationFuncTable heapFuncs = {
    [](void *) -> void * { return nullptr; },
    [](char *ptr, unsigned oldSize, unsigned newSize, void *) -> char * {
      char *newPtr = static_cast<char *>(realloc(ptr, newSize));
      if (!newPtr) {
        newPtr = static_cast<char *>(malloc(newSize));
        if (ptr) {
          memcpy(newPtr, ptr, oldSize);
          free(ptr);
        }
      }
      return newPtr;
    }};
static void *globalHeap;

#define registerPass                                                           \
  registerPass_impl(const PassInfo &passInfo, bool shouldFree = true);         \
  void registerPass
#define registerAnalysisGroup                                                  \
  registerAnalysisGroup_impl(const void *interfaceID, const void *passID,      \
                             PassInfo &registeree, bool isDefault,             \
                             bool shouldFree = true);                          \
  void registerAnalysisGroup
#include "llvm/PassRegistry.h"
#undef registerAnalysisGroup
#undef registerPass

#define registerPass registerPass_impl
#define registerAnalysisGroup registerAnalysisGroup_impl
#include "lib/IR/PassRegistry.cpp"
#undef registerAnalysisGroup
#undef registerPass

void PassRegistry::registerPass(const PassInfo &passInfo, bool shouldFree) {
  llvm::tpc::ScopedGlobalAllocator globalAlloc;
  const PassInfo *passInfoPtr =
      !globalAlloc
          ? &passInfo
          : new PassInfo(passInfo.getPassName(), passInfo.getPassArgument(),
                         passInfo.getTypeInfo(), passInfo.getNormalCtor(),
                         passInfo.isCFGOnlyPass(), passInfo.isAnalysis());

  registerPass_impl(*passInfoPtr, shouldFree);
}

void PassRegistry::registerAnalysisGroup(const void *interfaceID,
                                         const void *passID,
                                         PassInfo &registeree, bool isDefault,
                                         bool shouldFree) {
  llvm::tpc::ScopedGlobalAllocator globalAlloc;
  PassInfo *registereePtr =
      !globalAlloc
          ? &registeree
          : new PassInfo(registeree.getPassName(), registeree.getTypeInfo());

  registerAnalysisGroup_impl(interfaceID, passID, *registereePtr, isDefault,
                             shouldFree);
}

#include "lib/Support/ManagedStatic.cpp"

static const ManagedStaticBase *globalStaticList = nullptr;

void llvm_tpc_setHeapAllocationFuncs(const HeapAllocationFuncTable *funcs) {
  heapFuncs = *funcs;
}

void llvm_tpc_setGlobalAllocHeap(void *heap) {
  if (!globalHeap) {
    globalStaticList = StaticList;
    StaticList = nullptr;
  }

  llvm_shutdown();
  if (!heap)
    StaticList = globalStaticList;

  globalHeap = heap;
}

namespace llvm {
namespace tpc {

ScopedGlobalAllocator::ScopedGlobalAllocator() : newHeap(globalHeap) {
  if (newHeap)
    oldHeap = heapFuncs.setThread(newHeap);
}

ScopedGlobalAllocator::~ScopedGlobalAllocator() {
  if (newHeap)
    heapFuncs.setThread(oldHeap);
}

void GlobalBufOStream::write_impl(const char *ptr, size_t len) {
  unsigned newSize = size + len;
  if (capacity < newSize) {
    capacity = std::max({newSize, capacity + capacity / 2u, 128u});
    buf = heapFuncs.realloc(buf, size, capacity, globalHeap);
  }
  memcpy(buf + size, ptr, len);
  size = newSize;
}

GlobalBufOStream::GlobalBufOStream(char *&buf, unsigned &size)
    : buf(buf), size(size) {
  reset();
  SetUnbuffered();
}

} // end namespace tpc
} // end namespace llvm
