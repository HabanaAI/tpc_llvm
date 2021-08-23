//===-- llvm/Target/TPCTargetObjectFile.h - TPC Object Info ---*- C++ -*-===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

#ifndef LLVM_LIB_TARGET_TPC_TPCTARGETOBJECTFILE_H
#define LLVM_LIB_TARGET_TPC_TPCTARGETOBJECTFILE_H

#include "llvm/CodeGen/TargetLoweringObjectFileImpl.h"

namespace llvm {
class TPCTargetMachine;
  class TPCTargetObjectFile : public TargetLoweringObjectFileELF {

  };
} // end namespace llvm

#endif
