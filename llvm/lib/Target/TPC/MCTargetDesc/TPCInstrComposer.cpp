//===-- TPCInstrComposer.cpp - Convert LLVM instructions layout to TPC instructions layout -------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

#include <map>
#include "TPCInstrComposer.h"

using namespace llvm;

#define DEBUG_TYPE "tpc-repack"
#define MASK(Len) ((1<<Len)-1)

APInt TPCInstrComposer::getBinaryInst( uint64_t LLVMInstr, const std::map<Fields, Field> &Layout, unsigned IsPredicated) {
  APInt Insn(InstructionSizeInBits, 0);
  uint64_t Bits = 0;
  APInt Field;
  for (auto FieldLayout : Layout) {
    if (FieldLayout.first == SLOT_IS_PREDICATED) continue; // VPU layout has no space in TD so ignore
    Bits = LLVMInstr >> FieldLayout.second.startLLVM;
    Bits &= MASK(FieldLayout.second.sizeBin);
    Field = APInt(InstructionSizeInBits, Bits);
    Insn |= Field.shl(FieldLayout.second.startBin);
    LLVM_DEBUG(dbgs() << getFieldName(FieldLayout.first) << "=" << Bits << ",");
  }

  if (IsPredicated) {
    Field = APInt(InstructionSizeInBits, IsPredicated);
    Insn |= Field.shl(Layout.at(SLOT_IS_PREDICATED).startBin);
  }
  LLVM_DEBUG(dbgs() << getFieldName(SLOT_IS_PREDICATED) << "=" << IsPredicated);
  LLVM_DEBUG(dbgs() << "\n");
  return Insn;
}

APInt TPCInstrComposer::getIMM(uint32_t &IMM) {
  uint32_t ImmOffset = MayCompress ? 96 : 151;
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
      Bundle |= getBinaryInst(SPUInst, TPCInstrLayout::getSPUInstrLayout(IsCompressed, TPCFeatures), SPUPred);
      Bundle |= getBinaryInst(VPUInst, TPCInstrLayout::getVPUInstrLayout(IsCompressed, TPCFeatures), VPUPred);
    } else if (CT == CompressionType::LD_ST) {
      Bundle |= (1 << 1); // Compression type bit
      Bundle |= getBinaryInst(LDInst, TPCInstrLayout::getLDInstrLayout(IsCompressed, TPCFeatures), LoadPred);
      Bundle |= getBinaryInst(STInst, TPCInstrLayout::getSTInstrLayout(IsCompressed, TPCFeatures), StorePred);
    }
  } else {
    Bundle |= getBinaryInst(SPUInst, TPCInstrLayout::getSPUInstrLayout(IsCompressed, TPCFeatures), SPUPred);
    Bundle |= getBinaryInst(VPUInst, TPCInstrLayout::getVPUInstrLayout(IsCompressed, TPCFeatures), VPUPred);
    Bundle |= getBinaryInst(LDInst,  TPCInstrLayout::getLDInstrLayout(IsCompressed, TPCFeatures), LoadPred);
    Bundle |= getBinaryInst(STInst,  TPCInstrLayout::getSTInstrLayout(IsCompressed, TPCFeatures), StorePred);
  }
  Bundle |= getIMM(IMM);
  return Bundle;
}
