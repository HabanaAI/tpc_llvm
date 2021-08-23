//===-- TPCInstrLayout.h - ----------------------------------------*- C++ -*--//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//
//
// This class prints a TPC MCInst to a .s file.
//
//===----------------------------------------------------------------------===//

#ifndef LLVM_TPCINSTRLAYOUT_H
#define LLVM_TPCINSTRLAYOUT_H

#include "TPCSubtarget.h"
#include "MCTargetDesc/TPCMCTargetDesc.h"
#include "MCTargetDesc/TPCMCInstrInfo.h"
#include <bitset>

namespace llvm {

  struct Field {
    unsigned startBin;
    unsigned startLLVM;
    unsigned size;

    Field(unsigned _startBin, unsigned _startLLVM, unsigned _size) : startBin(_startBin), startLLVM(_startLLVM),
                                                                     size(_size) {};
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
    VPU_SWITCHES_EXTRA
  };

  static const std::map<Fields, Field> SPUInstrLayout{
          {SPU_OPCODE,             Field(0, 0, 6)},
          {SPU_SRC_A,              Field(6, 6, 7)},
          {SPU_SRC_B,              Field(13, 13, 7)},
          {SPU_DEST,               Field(20, 20, 7)},
          {SPU_OPERANDS_TYPE,      Field(27, 27, 4)},
          {SPU_PREDICATE_POLARITY, Field(31, 31, 1)},
          {SPU_PREDICATE_ADDRESS,  Field(32, 32, 4)},
          {SPU_SWITCHES,           Field(36, 36, 7)}
  };
  static const std::map<Fields, Field> VPUInstrLayout{
          {VPU_OPCODE,             Field(43, 0, 6)},
          {VPU_SRC_A,              Field(49, 6, 8)},
          {VPU_SRC_B,              Field(57, 14, 8)},
          {VPU_SRC_C,              Field(65, 22, 8)},
          {VPU_SRC_D,              Field(73, 30, 9)},
          {VPU_DEST,               Field(82, 39, 8)},
          {VPU_SWITCHES,           Field(90, 47, 3)},
          {VPU_OPERANDS_TYPE,      Field(93, 50, 4)},
          {VPU_PREDICATE_POLARITY, Field(97, 54, 1)},
          {VPU_PREDICATE_ADDRESS,  Field(98, 55, 5)},
  };
  static const std::map<Fields, Field> LDInstrLayout{
          {LOAD_SRC_A,                    Field(103, 0, 8)},
          {LOAD_SRC_B,                    Field(73, 48, 9)},
          {LOAD_DST,                      Field(111, 8, 8)},
          {LOAD_OPCODE,                   Field(119, 16, 5)},
          {LOAD_STORE_PREDICATE_POLARITY, Field(145, 42, 1)},
          {LOAD_STORE_PREDICATE_ADDRESS,  Field(146, 43, 5)},
          {LOAD_SWITCHES,                 Field(183, 57, 4)},
  };
  static const std::map<Fields, Field> STInstrLayout{
          {STORE_SRC_A,                   Field(124, 0, 8)},
          {STORE_SRC_B,                   Field(132, 8, 8)},
          {STORE_SRC_C,                   Field(65, 27, 8)},
          {STORE_OPCODE,                  Field(140, 16, 5)},
          {LOAD_STORE_PREDICATE_POLARITY, Field(145, 21, 1)},
          {LOAD_STORE_PREDICATE_ADDRESS,  Field(146, 22, 5)},
          {STORE_SWITCHES,                Field(187, 35, 4)}
  };

  static const std::map<Fields, Field> SPUInstrLayoutGen3{
          {SPU_OPCODE,             Field(2, 0, 6)},
          {SPU_SRC_A,              Field(8, 6, 7)},
          {SPU_SRC_B,              Field(15, 13, 7)},
          {SPU_DEST,               Field(22, 20, 7)},
          {SPU_OPERANDS_TYPE,      Field(29, 27, 4)},
          {SPU_PREDICATE_POLARITY, Field(33, 31, 1)},
          {SPU_PREDICATE_ADDRESS,  Field(34, 32, 4)},
          {SPU_SWITCHES,           Field(38, 36, 7)}
  };
  static const std::map<Fields, Field> VPUInstrLayoutGen3{
          {VPU_OPCODE,             Field(45, 0, 6)},
          {VPU_SRC_A,              Field(51, 6, 8)},
          {VPU_SRC_B,              Field(59, 14, 8)},
          {VPU_SRC_C,              Field(224, 22, 8)},
          {VPU_SRC_D,              Field(232, 30, 9)},  // NB: 8 in PRM but 9 in TD, luckily the extra bit overlaps with reserved area. FIXME ??
          {VPU_DEST,               Field(67, 39, 8)},
          {VPU_SWITCHES,           Field(85, 47, 3)},
          {VPU_OPERANDS_TYPE,      Field(75, 50, 4)},
          {VPU_PREDICATE_POLARITY, Field(79, 54, 1)},
          {VPU_PREDICATE_ADDRESS,  Field(80, 55, 5)},
          {VPU_SWITCHES_EXTRA,     Field(88, 60, 4)}
  };
  static const std::map<Fields, Field> LDInstrLayoutGen3{
          {LOAD_SRC_A,                    Field(130, 0, 8)},
          {LOAD_SRC_B,                    Field(151, 48, 9)},
          {LOAD_DST,                      Field(138, 8, 8)},
          {LOAD_OPCODE,                   Field(146, 16, 5)},
          {LOAD_STORE_PREDICATE_POLARITY, Field(160, 42, 1)},
          {LOAD_STORE_PREDICATE_ADDRESS,  Field(161, 43, 5)},
          {LOAD_SWITCHES,                 Field(166, 57, 7)},
  };
  static const std::map<Fields, Field> STInstrLayoutGen3{
          {STORE_SRC_A,                   Field(173, 0, 8)},
          {STORE_SRC_B,                   Field(181, 8, 8)},
          {STORE_SRC_C,                   Field(194, 27, 8)},
          {STORE_OPCODE,                  Field(189, 16, 5)},
          {LOAD_STORE_PREDICATE_POLARITY, Field(160, 21, 1)},
          {LOAD_STORE_PREDICATE_ADDRESS,  Field(161, 22, 5)},
          {STORE_SWITCHES,                Field(202, 35, 6)}
  };

  static const std::map<Fields, Field> SPUInstrLayoutGen3Compressed{
          {SPU_OPCODE,             Field(2, 0, 6)},
          {SPU_SRC_A,              Field(8, 6, 7)},
          {SPU_SRC_B,              Field(15, 13, 7)},
          {SPU_DEST,               Field(22, 20, 7)},
          {SPU_OPERANDS_TYPE,      Field(29, 27, 4)},
          {SPU_PREDICATE_POLARITY, Field(33, 31, 1)},
          {SPU_PREDICATE_ADDRESS,  Field(34, 32, 4)},
          {SPU_SWITCHES,           Field(38, 36, 7)}
  };
  static const std::map<Fields, Field> VPUInstrLayoutGen3Compressed{
          {VPU_OPCODE,             Field(45, 0, 6)},
          {VPU_SRC_A,              Field(51, 6, 8)},
          {VPU_SRC_B,              Field(59, 14, 8)},
          {VPU_DEST,               Field(67, 39, 8)},
          {VPU_SWITCHES,           Field(85, 47, 3)},
          {VPU_OPERANDS_TYPE,      Field(75, 50, 4)},
          {VPU_PREDICATE_POLARITY, Field(79, 54, 1)},
          {VPU_PREDICATE_ADDRESS,  Field(80, 55, 5)},
          {VPU_SWITCHES_EXTRA,     Field(88, 60, 4)}
  };
  static const std::map<Fields, Field> LDInstrLayoutGen3Compressed{
          {LOAD_SRC_A,                    Field(2, 0, 8)},
          {LOAD_SRC_B,                    Field(23, 48, 9)},
          {LOAD_DST,                      Field(10, 8, 8)},
          {LOAD_OPCODE,                   Field(18, 16, 5)},
          {LOAD_STORE_PREDICATE_POLARITY, Field(32, 42, 1)},
          {LOAD_STORE_PREDICATE_ADDRESS,  Field(33, 43, 5)},
          {LOAD_SWITCHES,                 Field(38, 57, 7)},
  };
  static const std::map<Fields, Field> STInstrLayoutGen3Compressed{
          {STORE_SRC_A,                   Field(45, 0, 8)},
          {STORE_SRC_B,                   Field(53, 8, 8)},
          {STORE_SRC_C,                   Field(66, 27, 8)},
          {STORE_OPCODE,                  Field(61, 16, 5)},
          {LOAD_STORE_PREDICATE_POLARITY, Field(32, 21, 1)},
          {LOAD_STORE_PREDICATE_ADDRESS,  Field(33, 22, 5)},
          {STORE_SWITCHES,                Field(74, 35, 6)}
  };

  static const std::map<Fields, Field> STInstrLayoutGen4{
          {STORE_SRC_A,                   Field(173, 0, 8)},
          {STORE_SRC_B,                   Field(181, 8, 8)},
          {STORE_SRC_C,                   Field(194, 27, 8)},
          {STORE_OPCODE,                  Field(189, 16, 5)},
          {LOAD_STORE_PREDICATE_POLARITY, Field(160, 21, 1)},
          {LOAD_STORE_PREDICATE_ADDRESS,  Field(161, 22, 5)},
          {STORE_SWITCHES,                Field(202, 35, 7)}
  };
  static const std::map<Fields, Field> STInstrLayoutGen4Compressed{
          {STORE_SRC_A,                   Field(45, 0, 8)},
          {STORE_SRC_B,                   Field(53, 8, 8)},
          {STORE_SRC_C,                   Field(66, 27, 8)},
          {STORE_OPCODE,                  Field(61, 16, 5)},
          {LOAD_STORE_PREDICATE_POLARITY, Field(32, 21, 1)},
          {LOAD_STORE_PREDICATE_ADDRESS,  Field(33, 22, 5)},
          {STORE_SWITCHES,                Field(74, 35, 7)}
  };

  enum CompressionType {
    SPU_VPU,
    LD_ST
  };

  class TPCInstrLayout {
  public:
    static const std::map<Fields, Field> getLDInstrLayout(bool compressed, const FeatureBitset &TPCFeatures) {
      if (TPCFeatures[TPC::FeatureGoya] || TPCFeatures[TPC::FeatureGaudi]) {
        return LDInstrLayout;
      }
      llvm_unreachable("Bad TPC Feature");
    }

    static const std::map<Fields, Field> getSTInstrLayout(bool compressed, const FeatureBitset &TPCFeatures) {
      if (TPCFeatures[TPC::FeatureGoya] || TPCFeatures[TPC::FeatureGaudi]) {
	    return STInstrLayout;
      }
      llvm_unreachable("Bad TPC Feature");
    }

    static const std::map<Fields, Field> getVPUInstrLayout(bool compressed, const FeatureBitset &TPCFeatures) {
      if (TPCFeatures[TPC::FeatureGoya] || TPCFeatures[TPC::FeatureGaudi]) {
	    return VPUInstrLayout;
      }
      llvm_unreachable("Bad TPC Feature");
    }

    static const std::map<Fields, Field> getSPUInstrLayout(bool compressed, const FeatureBitset &TPCFeatures) {
      if (TPCFeatures[TPC::FeatureGoya] || TPCFeatures[TPC::FeatureGaudi]) {
	    return SPUInstrLayout;
      }
      llvm_unreachable("Bad TPC Feature");
    }
  };

}
#endif //LLVM_TPCINSTRLAYOUT_H
