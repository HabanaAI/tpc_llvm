//===---- TPCMCAsmInfo.cpp ------------------------------------------------===//
//
//===----------------------------------------------------------------------===//
//
// This file contains the definition of the TPCMCAsmInfo class.
//
//===----------------------------------------------------------------------===//

#include "TPCMCAsmInfo.h"
#include "llvm/ADT/Triple.h"

using namespace llvm;

void TPCMCAsmInfo::anchor() { }

TPCMCAsmInfo::TPCMCAsmInfo(const Triple &TheTriple) {
  assert(TheTriple.getArch() == Triple::tpc);

  IsLittleEndian = true;
  ExceptionsType = ExceptionHandling::None;

  CommentString = "//";
  SupportsDebugInformation = true;
  PrivateGlobalPrefix = "$";
}
