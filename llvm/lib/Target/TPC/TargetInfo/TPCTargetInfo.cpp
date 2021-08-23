//===-- TPCTargetInfo.cpp - TPC Target Implementation ---------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

#include "llvm/Support/TargetRegistry.h"
#include "TPC.h"
using namespace llvm;

Target TheTPCTarget;

Target &llvm::getTheTPCTarget() {
  return TheTPCTarget;
}


extern "C" void LLVMInitializeTPCTargetInfo() {
  RegisterTarget<Triple::tpc, /*HasJIT=*/false>
    X(TheTPCTarget, "tpc", "TPC backend", "TPC");
}
