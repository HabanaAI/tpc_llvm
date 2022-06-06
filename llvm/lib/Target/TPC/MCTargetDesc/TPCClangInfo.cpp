#include "llvm/Target/TPCClangInfo.h"

namespace llvm {

std::string TPCCompilerArguments("");
bool UsePrintf = false;
unsigned NumArgumentTensors = 0;

std::string makeTPCCompilerArgumentsString(ArrayRef<const char *> Args) {
  std::string Result;

  for (auto Iter = Args.begin(); Iter != Args.end(); ++Iter) {
    if (*Iter) {
      Result.push_back('"');
      Result.append(*Iter);
      Result.push_back('"');
      Result.push_back(' ');
    }
  }

  if (!Result.empty())
    Result.pop_back();

  return Result;
}

}
