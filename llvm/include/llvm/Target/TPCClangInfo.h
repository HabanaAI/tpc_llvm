#ifndef TPC_CLANG_INFO_H
#define TPC_CLANG_INFO_H

#include "llvm/ADT/ArrayRef.h"

#include <string>
namespace llvm {
extern std::string TPCCompilerArguments;
extern bool UsePrintf;
extern unsigned NumArgumentTensors;

std::string makeTPCCompilerArgumentsString(ArrayRef<const char *> Args);
}

#endif // TPC_CLANG_INFO_H
