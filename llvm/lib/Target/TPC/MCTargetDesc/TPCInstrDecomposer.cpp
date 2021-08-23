//===-- TPCInstrDecomposer.cpp - Convert TPC instructions layout to LLVM instructions layout  -------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
//
//===----------------------------------------------------------------------===//


#include "TPC.h"
#include "TPCSubtarget.h"
#include "TPCMCInstrInfo.h"
#include "TPCMCTargetDesc.h"
#include "TPCInstrDecomposer.h"

using namespace llvm;
using DecodeStatus = MCDisassembler::DecodeStatus;


#define DEBUG_TYPE "tpc-repack"

#define MASK(Len) ((1UL<<Len)-1)
namespace llvm {

DecodeStatus
TPCInstrDecomposer::getLLVMInst(uint64_t &Insn, const std::map<Fields, Field> &Layout, TPCII::IType SlotType) {
  Insn = 0;
  uint64_t bits = 0;
  for (auto FieldLayout : Layout) {
    std::bitset<256> FieldBits = Bundle >> FieldLayout.second.startBin;
    FieldBits &= MASK(FieldLayout.second.size);
    bits = static_cast<uint64_t>(FieldBits.to_ulong());
    Insn |= bits << FieldLayout.second.startLLVM;
  }
  return DecodeStatus::Success;
}

void TPCInstrDecomposer::createLLVMNOP(uint64_t &Insn, const Field &F) {
  Insn |= (63UL & MASK(F.size)) << F.startLLVM;
}


DecodeStatus llvm::TPCInstrDecomposer::getLLVMInstSPU(uint64_t &Insn) {
  const std::map<Fields, Field> Layout = TPCInstrLayout::getSPUInstrLayout(IsCompressed, TPCFeatures);
  if (MayCompress && IsCompressed && (CT == CompressionType::LD_ST)) {
    createLLVMNOP(Insn, Layout.at(Fields::SPU_OPCODE));
    return MCDisassembler::Success;
  }
  return getLLVMInst(Insn, Layout, TPCII::TypeSPU);
}

DecodeStatus llvm::TPCInstrDecomposer::getLLVMInstVPU(uint64_t &Insn) {
  const std::map<Fields, Field> Layout = TPCInstrLayout::getVPUInstrLayout(IsCompressed, TPCFeatures);
  if (MayCompress && IsCompressed && (CT == CompressionType::LD_ST)) {
    createLLVMNOP(Insn, Layout.at(Fields::VPU_OPCODE));
    return MCDisassembler::Success;
  }
  return getLLVMInst(Insn, Layout, TPCII::TypeVPU);
}

DecodeStatus llvm::TPCInstrDecomposer::getLLVMInstLoad(uint64_t &Insn) {
  const std::map<Fields, Field> Layout = TPCInstrLayout::getLDInstrLayout(IsCompressed, TPCFeatures);
  if (MayCompress && IsCompressed && (CT == CompressionType::SPU_VPU)) {
    createLLVMNOP(Insn, Layout.at(Fields::LOAD_OPCODE));
    return MCDisassembler::Success;
  }
  return getLLVMInst(Insn, Layout, TPCII::TypeLOAD);
}

DecodeStatus llvm::TPCInstrDecomposer::getLLVMInstStore(uint64_t &Insn) {
  const std::map<Fields, Field> Layout = TPCInstrLayout::getSTInstrLayout(IsCompressed, TPCFeatures);
  if (MayCompress && IsCompressed && (CT == CompressionType::SPU_VPU)) {
    createLLVMNOP(Insn, Layout.at(Fields::STORE_OPCODE));
    return MCDisassembler::Success;
  }
  return getLLVMInst(Insn, Layout, TPCII::TypeSTORE);
}

//{IMM,             Field(151,32)},
DecodeStatus TPCInstrDecomposer::getLLVMInstIMM(uint32_t &IMM) {
  uint32_t ImmOffset =  151;
  std::bitset<256> IMMBits = Bundle >> ImmOffset;
  IMMBits &= UINT32_MAX;
  IMM = static_cast<uint32_t>(IMMBits.to_ullong());
  return DecodeStatus::Success;
}

}
