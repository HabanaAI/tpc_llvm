//===-- TPCInstrDecomposer.h - Convert TPC instructions layout to LLVM instructions layout  -------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
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
  bool MayCompress  = false;
  bool IsCompressed = false;
  bool IgnoreIsPredicated = false; //workaround, TODO handle predication
  CompressionType CT;
  unsigned InstructionSize = 256 / 8;
  std::bitset<256> Bundle;

  void createLLVMNOP(uint64_t &Insn, const Field &F);

  MCDisassembler::DecodeStatus
  getLLVMInst(uint64_t &Insn, const std::map<Fields, Field> &Layout,
              TPCII::IType SlotType, bool &IsPred);

public:
  TPCInstrDecomposer(const ArrayRef<uint8_t> &MIBytes, const FeatureBitset &Features) : TPCFeatures(Features) {
    MayCompress = Features[TPC::FeatureCompress];
    if (MayCompress) {
      IsCompressed = (MIBytes.front() & 1);
      CT = (MIBytes.front() & 2) ? CompressionType::LD_ST : CompressionType::SPU_VPU;
      if (IsCompressed) {
        InstructionSize = 128 / 8;
      }
    }
    IgnoreIsPredicated = Features[TPC::FeatureDoron1];
    memcpy(&Bundle, &MIBytes.front(), InstructionSize);
  }

  ~TPCInstrDecomposer() {};

  MCDisassembler::DecodeStatus getLLVMInstSPU(uint64_t &Insn, bool &IsPred);

  MCDisassembler::DecodeStatus getLLVMInstVPU(uint64_t &Insn, bool &IsPred);

  MCDisassembler::DecodeStatus getLLVMInstLoad(uint64_t &Insn, bool &IsPred);

  MCDisassembler::DecodeStatus getLLVMInstStore(uint64_t &Insn, bool &IsPred);

  MCDisassembler::DecodeStatus getLLVMInstIMM(uint32_t &IMM);

  uint64_t getBundleSizeInBytes() { return InstructionSize; }
  bool     getIsCompressed()      { return IsCompressed; }
};
}

#endif //LLVM_TPCINSTRDECOMPOSER_H
