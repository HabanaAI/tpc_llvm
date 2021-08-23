//===---------------------------- TPCIntrinsicUtils.h ------------------===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//
//
// Declares intrinsics API interface and instruction switches table for middle
// end.This shall be extended as and when required.
//
//===----------------------------------------------------------------------===//

#ifndef LLVM_TRASNSFORM_UTILS_TPC_INTRINSIC_H
#define LLVM_TRASNSFORM_UTILS_TPC_INTRINSIC_H

#include "llvm/IR/Instructions.h" 

namespace llvm {
// Creates the required intrinsic instruction corresponding the instruction \p
// InstrToReplace.\p Switch is used to supply rounding mode switch to be encoded
// in assembly instruction.
CallInst *createConvertIntrinsic(Instruction *InstrToReplace,
                                 unsigned int Switch);
} // namespace llvm
#endif