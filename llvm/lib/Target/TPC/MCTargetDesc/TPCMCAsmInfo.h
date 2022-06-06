//===-- TPCMCAsmInfo.h - TPC Asm Info ------------------------*- C++ -*----===//
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
