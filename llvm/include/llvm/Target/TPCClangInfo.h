#ifndef TPC_CLANG_INFO_H
#define TPC_CLANG_INFO_H

#include "llvm/ADT/ArrayRef.h"
#include "llvm/ADT/StringRef.h"

#include <cstdint>
#include <string>
namespace llvm {
extern std::string TPCCompilerArguments;
extern bool UsePrintf;
extern unsigned NumArgumentTensors;

std::string makeTPCCompilerArgumentsString(ArrayRef<const char *> Args);

/// Module flag in which clang passes the kernel's hashedISA value to the
/// backend, so that -S and -c agree although only -c embeds the source text.
constexpr const char TPCKernelHashFlag[] = "tpc-kernel-hash";

/// The hashedISA value of a kernel: CRC-32 of the source file's stem, then of
/// the source text.
uint32_t computeTPCKernelHash(StringRef SourceFileName, StringRef SourceText);
}

#endif // TPC_CLANG_INFO_H
