//===-- TPCInstrDecomposer.h - Convert TPC instructions layout to LLVM instructions layout  -------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
//
//===----------------------------------------------------------------------===//


#ifndef LLVM_TPCINSTRDECOMPOSER_H
#define LLVM_TPCINSTRDECOMPOSER_H

#include "llvm/MC/MCDisassembler/MCDisassembler.h"
#include "TPCInstrLayout.h"
#include <bitset>

namespace llvm {

class TPCInstrDecomposer {

  const FeatureBitset &TPCFeatures;
  bool MayCompress = false;
  bool IsCompressed;
  CompressionType CT;
  unsigned InstructionSize = 256 / 8;
  std::bitset<256> Bundle;

  void createLLVMNOP(uint64_t &Insn, const Field &F);

  MCDisassembler::DecodeStatus
  getLLVMInst(uint64_t &Insn, const std::map<Fields, Field> &Layout, TPCII::IType SlotType);

public:
  TPCInstrDecomposer(const ArrayRef<uint8_t> &MIBytes, const FeatureBitset &Features) : TPCFeatures(Features) {
    memcpy(&Bundle, &MIBytes.front(), InstructionSize);
  }

  ~TPCInstrDecomposer() {};

  MCDisassembler::DecodeStatus getLLVMInstSPU(uint64_t &Insn);

  MCDisassembler::DecodeStatus getLLVMInstVPU(uint64_t &Insn);

  MCDisassembler::DecodeStatus getLLVMInstLoad(uint64_t &Insn);

  MCDisassembler::DecodeStatus getLLVMInstStore(uint64_t &Insn);

  MCDisassembler::DecodeStatus getLLVMInstIMM(uint32_t &IMM);

  uint64_t getBundleSizeInBytes() { return InstructionSize; }
  bool     getIsCompressed()      { return IsCompressed; }
};
}

#endif //LLVM_TPCINSTRDECOMPOSER_H
