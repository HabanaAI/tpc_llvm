//===-- TPCMCAsmInfo.h - TPC Asm Info ------------------------*- C++ -*----===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//
//
// This file contains the declaration of the TPCMCAsmInfo class.
//
//===----------------------------------------------------------------------===//

#ifndef LLVM_LIB_TARGET_TPC_MCTARGETDESC_TPCMCASMINFO_H
#define LLVM_LIB_TARGET_TPC_MCTARGETDESC_TPCMCASMINFO_H

#include "llvm/MC/MCAsmInfoELF.h"
#include "llvm/ADT/Triple.h"

namespace llvm {

class Triple;

class TPCMCAsmInfo : public MCAsmInfoELF {
  void anchor() override;

public:
  explicit TPCMCAsmInfo(const Triple &TheTriple);
};

} // namespace llvm

#endif
