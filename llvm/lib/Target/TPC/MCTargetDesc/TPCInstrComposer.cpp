//===-- TPCInstrComposer.cpp - Convert LLVM instructions layout to TPC instructions layout -------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
//
//===----------------------------------------------------------------------===//
#include <map>
#include "TPCInstrComposer.h"

using namespace llvm;

#define MASK(Len) ((1<<Len)-1)

APInt TPCInstrComposer::getBinaryInst( uint64_t LLVMInstr, const std::map<Fields, Field> &Layout) {
  APInt Insn(InstructionSizeInBits, 0);
  uint64_t Bits = 0;
  APInt Field;
  for (auto FieldLayout : Layout) {
    Bits = LLVMInstr >> FieldLayout.second.startLLVM;
    Bits &= MASK(FieldLayout.second.size);
    Field = APInt(InstructionSizeInBits, Bits);
    Insn |= Field.shl(FieldLayout.second.startBin);
  }
  return Insn;
}

APInt TPCInstrComposer::getIMM(uint32_t &IMM) {
  uint32_t ImmOffset = 151;
  APInt ImmValue(InstructionSizeInBits, IMM);
  ImmValue = ImmValue.shl(ImmOffset);
  return ImmValue;
}

#undef MASK

APInt TPCInstrComposer::createBundle() {
  APInt Bundle(InstructionSizeInBits, 0);
  if (MayCompress && IsCompressed) {
    Bundle |= 1; // isCompressed bit
    if(CT == CompressionType::SPU_VPU) {
      Bundle |= getBinaryInst(SPUInst, TPCInstrLayout::getSPUInstrLayout(IsCompressed, TPCFeatures));
      Bundle |= getBinaryInst(VPUInst, TPCInstrLayout::getVPUInstrLayout(IsCompressed, TPCFeatures));
    } else if (CT == CompressionType::LD_ST) {
      Bundle |= (1 << 1); // Compression type bit
      Bundle |= getBinaryInst(LDInst, TPCInstrLayout::getLDInstrLayout(IsCompressed, TPCFeatures));
      Bundle |= getBinaryInst(STInst, TPCInstrLayout::getSTInstrLayout(IsCompressed, TPCFeatures));
    }
  } else {
    Bundle |= getBinaryInst(SPUInst, TPCInstrLayout::getSPUInstrLayout(IsCompressed, TPCFeatures));
    Bundle |= getBinaryInst(VPUInst, TPCInstrLayout::getVPUInstrLayout(IsCompressed, TPCFeatures));
    Bundle |= getBinaryInst(LDInst,  TPCInstrLayout::getLDInstrLayout(IsCompressed, TPCFeatures));
    Bundle |= getBinaryInst(STInst,  TPCInstrLayout::getSTInstrLayout(IsCompressed, TPCFeatures));
  }
  Bundle |= getIMM(IMM);
  return Bundle;
}
