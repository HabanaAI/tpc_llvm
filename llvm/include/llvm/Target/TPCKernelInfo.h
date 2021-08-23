//===- llvm/Target/TPCProgramHeader.h - TPC Specific header -----*- C++ -*-===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
//
// This file declares a TPC kernel info. This info usages for write
// .KernelInfo section.
//
//===----------------------------------------------------------------------===//

#ifndef TPC_KERNEL_INFO_H
#define TPC_KERNEL_INFO_H

#include <string>

namespace llvm {

static const char *KernelInfoSectionName = ".KernelInfo";

std::string GenerateKernelInfo(llvm::StringRef KernelInfo);

}

#endif // TPC_KERNEL_INFO_H
