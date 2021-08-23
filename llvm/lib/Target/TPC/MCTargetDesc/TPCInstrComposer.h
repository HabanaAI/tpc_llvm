//===-- TPCInstrComposer.h - Convert LLVM instructions layout to TPC instructions layout -------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
//
//===----------------------------------------------------------------------===//


#ifndef LLVM_TPCINSTRCOMPOSER_H
#define LLVM_TPCINSTRCOMPOSER_H

#include "TPCMCInstrInfo.h"
#include "TPCInstrLayout.h"
#include "TPCSubtarget.h"
#include "MCTargetDesc/TPCMCTargetDesc.h"
#include "MCTargetDesc/TPCMCInstrInfo.h"
#include "llvm/ADT/APInt.h"
#include <bitset>

namespace llvm {

class TPCInstrComposer {
  unsigned InstructionSizeInBits = 256;
  uint64_t SPUInst;
  uint64_t VPUInst;
  uint64_t LDInst;
  uint64_t STInst;
  uint32_t IMM;

  const FeatureBitset &TPCFeatures;
  bool IsCompressed;
  CompressionType CT;
  bool MayCompress = false;

  APInt getBinaryInst(uint64_t LLVMInstr, const std::map<Fields, Field> &Layout);
  APInt getIMM(uint32_t &IMM);

public:
  TPCInstrComposer(uint64_t _SPUInst, uint64_t _VPUInst, uint64_t _LDInst, uint64_t _STInst, uint32_t _IMM,
      const FeatureBitset &Features, bool _IsCompressed, CompressionType _CT) :
      SPUInst(_SPUInst), VPUInst(_VPUInst), LDInst(_LDInst), STInst(_STInst), IMM(_IMM),
      TPCFeatures(Features), IsCompressed(_IsCompressed), CT(_CT) {
    if(IsCompressed) {
      InstructionSizeInBits = 128;
    }
    MayCompress = false;
  }

  ~TPCInstrComposer() = default;

  APInt createBundle();
};
}


#endif //LLVM_TPCINSTRCOMPOSER_H
