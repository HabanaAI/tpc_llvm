//===- TPCMetadataSection.cpp - TPC Specific header -------------*- C++ -*-===//
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

#include "llvm/Support/FormatVariadic.h"
#include "llvm/Target/TPCKernelInfo.h"
#include <cassert>

std::string llvm::GenerateKernelInfo(StringRef KernelInfo) {
   assert(!KernelInfo.empty());
   return formatv("KERNELINFOBEGIN KernelName:[{0}] #KERNELINFOEND",
                  KernelInfo);
}
