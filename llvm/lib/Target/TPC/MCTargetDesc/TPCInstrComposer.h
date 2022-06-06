//===-- TPCInstrComposer.h - Convert LLVM instructions layout to TPC instructions layout -------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
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
  bool MayCompress;

  unsigned SPUPred;
  unsigned VPUPred;
  unsigned LoadPred;
  unsigned StorePred;

  APInt getBinaryInst(uint64_t LLVMInstr, const std::map<Fields, Field> &Layout, unsigned IsPredicated);
  APInt getIMM(uint32_t &IMM);

public:
  TPCInstrComposer(uint64_t _SPUInst, uint64_t _VPUInst, uint64_t _LDInst, uint64_t _STInst, uint32_t _IMM,
      const FeatureBitset &Features, bool _IsCompressed, CompressionType _CT,
      unsigned _SPUPred, unsigned _VPUPred, unsigned _LoadPred, unsigned _StorePred) :
      SPUInst(_SPUInst), VPUInst(_VPUInst), LDInst(_LDInst), STInst(_STInst), IMM(_IMM),
      TPCFeatures(Features), IsCompressed(_IsCompressed), CT(_CT),
      SPUPred(_SPUPred), VPUPred(_VPUPred), LoadPred(_LoadPred), StorePred(_StorePred){
    if(IsCompressed) {
      InstructionSizeInBits = 128;
    }
    MayCompress = Features[TPC::FeatureCompress];
    assert(!MayCompress || Features[TPC::FeatureGreco] ||
           Features[TPC::FeatureGaudi2] || Features[TPC::FeatureDoron1]);
  }

  ~TPCInstrComposer() = default;

  APInt createBundle();
};
}


#endif //LLVM_TPCINSTRCOMPOSER_H
