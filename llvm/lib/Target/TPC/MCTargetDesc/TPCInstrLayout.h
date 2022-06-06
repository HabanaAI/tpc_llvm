//
// Created by mgrishchenko on 19.08.2019.
//

#ifndef LLVM_TPCINSTRLAYOUT_H
#define LLVM_TPCINSTRLAYOUT_H

#include "MCTargetDesc/TPCMCInstrInfo.h"
#include "MCTargetDesc/TPCMCTargetDesc.h"
#include "TPCSubtarget.h"
#include <bitset>
#include <map>

namespace llvm {

struct Field {
  unsigned startBin;
  unsigned startLLVM;
  unsigned sizeBin;
  unsigned sizeLLVM;
  unsigned size;

  Field(unsigned _startBin, unsigned _startLLVM, unsigned _sizeBin,
        unsigned _sizeLLVM)
      : startBin(_startBin), startLLVM(_startLLVM), sizeBin(_sizeBin),
        sizeLLVM(_sizeLLVM){};
};

enum Fields {
  SPU_OPCODE,
  SPU_SRC_A,
  SPU_SRC_B,
  SPU_DEST,
  SPU_OPERANDS_TYPE,
  SPU_PREDICATE_POLARITY,
  SPU_PREDICATE_ADDRESS,
  SPU_SWITCHES,
  VPU_OPCODE,
  VPU_SRC_A,
  VPU_SRC_B,
  VPU_SRC_C,
  VPU_SRC_D,
  VPU_DEST,
  VPU_SWITCHES,
  VPU_OPERANDS_TYPE,
  VPU_PREDICATE_POLARITY,
  VPU_PREDICATE_ADDRESS,
  LOAD_SRC_A,
  LOAD_SRC_B,
  LOAD_DST,
  LOAD_OPCODE,
  STORE_SRC_A,
  STORE_SRC_B,
  STORE_SRC_C,
  STORE_OPCODE,
  LOAD_STORE_PREDICATE_POLARITY,
  LOAD_STORE_PREDICATE_ADDRESS,
  IMM,
  LOAD_SWITCHES,
  STORE_SWITCHES,
  VPU_SWITCHES_EXTRA,
  VPU_SWITCHES_SUPER_EXTRA,
  SLOT_IS_PREDICATED,
  // LOOP specific fields
  START_VALUE,
  BOUNDARY_VALUE,
  STEP_VALUE,
  START_VALUE_SEL,
  BOUNDARY_VALUE_SEL,
  STEP_VALUE_SEL,
  REPEAT_AT_MOST_ONCE,
  LOOP_PREDICATE_POLARITY,
  LOOP_PREDICATE_ADDRESS,
  LOOP_IS_PREDICATED,
  COMP_MODE
};

const char *getFieldName(Fields F);

enum CompressionType { SPU_VPU, LD_ST };

namespace TPCInstrLayout {
const std::map<Fields, Field> &
getLDInstrLayout(bool Compressed, const FeatureBitset &TPCFeatures);

const std::map<Fields, Field> &
getSTInstrLayout(bool Compressed, const FeatureBitset &TPCFeatures);

const std::map<Fields, Field> &
getVPUInstrLayout(bool Compressed, const FeatureBitset &TPCFeatures);

const std::map<Fields, Field> &
getSPUInstrLayout(bool Compressed, const FeatureBitset &TPCFeatures);

const std::map<Fields, Field> &
getLoopInstrLayout(bool Compressed, const FeatureBitset &TPCFeatures);

const std::map<Fields, Field> &getLDInstrLayoutUnsafe();
const std::map<Fields, Field> &getSTInstrLayoutUnsafe();

} // namespace TPCInstrLayout
} // namespace llvm

#endif // LLVM_TPCINSTRLAYOUT_H
