//===-- API.h -----------------------------------------------------*- C -*-===//
//
// Copyright (C) 2021 HabanaLabs, Ltd.
// All Rights Reserved.
// Unauthorized copying of this file, via any medium is strictly prohibited.
// Proprietary and confidential.
//
//===----------------------------------------------------------------------===//

#ifndef LLVM_CLANG_TOOLS_LLVM_TPC_API_H
#define LLVM_CLANG_TOOLS_LLVM_TPC_API_H

#ifdef __cplusplus
extern "C" {
#endif

typedef void (*WritePfn)(const char *ptr, unsigned size);

/// Set the write functions for redirecting the `out`, `err` and `dbg` streams.
void llvm_tpc_redirectOutput(WritePfn out, WritePfn err, WritePfn dbg);

struct HeapAllocationFuncTable {
  /// Function pointer for setting the allocation heap for the current thread.
  void *(*setThread)(void *heap);

  /// Function pointer for re/allocating memory.
  char *(*realloc)(char *ptr, unsigned oldSize, unsigned newSize, void *heap);
};

/// Set the heap allocation functions.
void llvm_tpc_setHeapAllocationFuncs(
    const struct HeapAllocationFuncTable *funcs);

/// Set the global allocation heap.
void llvm_tpc_setGlobalAllocHeap(void *heap);

/// Prepare internal state for usage, and configure the compiler with the given
/// arguments.
void llvm_tpc_startup(const char *args);

/// Destroy all used resources.
void llvm_tpc_shutdown();

/// Free resources used in compilation.
void llvm_tpc_cleanup();

/// Compile a given LLVM module to an ELF binary.
///
/// \param[in] moduleBuf The LLVM module to compile (assembly or bitcode).
/// \param[in] moduleSize The size (in bytes) of `moduleBuf`.
/// \param[out] elfBin The resulting compiled ELF binary.
/// \param[out] asmIR The final LLVM IR assembly (before codegen).
/// \param[in] cpu The target CPU architecture.
/// \param[in] verify Run the verifier.
///
/// \returns The size (in bytes) of the resulting compiled ELF binary, if the
/// operation was successful; zero otherwise.
unsigned llvm_tpc_compileModule(const char *moduleBuf, unsigned moduleSize,
                                char **elfBin, char **asmIR, const char *cpu,
                                bool verify);

/// Disassemble the TPC code in an ELF binary.
///
/// \param[in] elfBin The ELF binary to disassemble.
/// \param[in] elfSize The size (in bytes) of `elfBin`.
/// \param[out] tpcAsm The resulting TPC assembly.
///
/// \returns The size (in bytes) of the resulting TPC assembly, if the operation
/// was successful; zero otherwise.
unsigned llvm_tpc_disassembleTPC(const char *elfBin, unsigned elfSize,
                                 char **tpcAsm);

#ifdef __cplusplus
} // extern "C"
#endif

#endif // LLVM_CLANG_TOOLS_LLVM_TPC_API_H
