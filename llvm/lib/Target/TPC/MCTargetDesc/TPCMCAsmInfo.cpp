//===---- TPCMCAsmInfo.cpp ------------------------------------------------===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
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
