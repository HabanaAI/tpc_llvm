//===---------------------------- TPCIntrinsicUtils.h ------------------===//
//
//
//                     The LLVM Compiler Infrastructure:
//
//              2020 - This file is a property of Habana labs
//
// Author : Md Shahid
// Email  : aashahid@habana.ai
//
//===----------------------------------------------------------------------===//
//
// Declares intrinsics API interface and instruction switches table for middle
// end.This shall be extended as and when required.
//
//===----------------------------------------------------------------------===//

#ifndef LLVM_TRASNSFORM_UTILS_TPC_INTRINSIC_H
#define LLVM_TRASNSFORM_UTILS_TPC_INTRINSIC_H

#include "llvm/IR/GlobalValue.h"

#include <string>

namespace llvm {
class CallInst;
class FunctionType;
class Instruction;

// Creates the required intrinsic instruction corresponding the instruction \p
// InstrToReplace.\p Switch is used to supply rounding mode switch to be encoded
// in assembly instruction.
CallInst *createConvertIntrinsic(Instruction *InstrToReplace,
                                 unsigned int Switch);

/// Get the tpc intrinsic corresponding to IDNum and FType.
std::string getTPCIntrinsicName(const Intrinsic::ID IDNum,
                                FunctionType *FType);
} // namespace llvm
#endif