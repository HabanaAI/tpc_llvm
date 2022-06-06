#include "TPCInstrLayout.h"

#include "llvm/Support/ErrorHandling.h"

using namespace llvm;

// clang-format off

static const std::map<Fields, Field> SPUInstrLayout{
    {SPU_OPCODE,             Field(0, 0, 6, 6)},
    {SPU_SRC_A,              Field(6, 6, 7, 8)},
    {SPU_SRC_B,              Field(13, 14, 7, 8)},
    {SPU_DEST,               Field(20, 22, 7, 8)},
    {SPU_OPERANDS_TYPE,      Field(27, 30, 4, 4)},
    {SPU_PREDICATE_POLARITY, Field(31, 34, 1, 1)},
    {SPU_PREDICATE_ADDRESS,  Field(32, 35, 4, 4)},
    {SPU_SWITCHES,           Field(36, 40, 7, 7)},
};

static const std::map<Fields, Field> VPUInstrLayout{
    {VPU_OPCODE,             Field(43, 0, 6, 6)},
    {VPU_SRC_A,              Field(49, 6, 8, 8)},
    {VPU_SRC_B,              Field(57, 14, 8, 8)},
    {VPU_SRC_C,              Field(65, 22, 8, 8)},
    {VPU_SRC_D,              Field(73, 30, 9, 9)},
    {VPU_DEST,               Field(82, 39, 8, 8)},
    {VPU_SWITCHES,           Field(90, 47, 3, 3)},
    {VPU_OPERANDS_TYPE,      Field(93, 50, 4, 4)},
    {VPU_PREDICATE_POLARITY, Field(97, 54, 1, 1)},
    {VPU_PREDICATE_ADDRESS,  Field(98, 55, 5, 5)},
};

static const std::map<Fields, Field> LDInstrLayout{
    {LOAD_SRC_A,                    Field(103, 0, 8, 8)},
    {LOAD_SRC_B,                    Field(73, 28, 9, 9)},
    {LOAD_DST,                      Field(111, 8, 8, 8)},
    {LOAD_OPCODE,                   Field(119, 16, 5, 5)},
    {LOAD_STORE_PREDICATE_POLARITY, Field(145, 21, 1, 1)},
    {LOAD_STORE_PREDICATE_ADDRESS,  Field(146, 22, 5, 5)},
    {LOAD_SWITCHES,                 Field(183, 37, 4, 4)},
};

static const std::map<Fields, Field> STInstrLayout{
    {STORE_SRC_A,                   Field(124, 0, 8, 8)},
    {STORE_SRC_B,                   Field(132, 8, 8, 8)},
    {STORE_SRC_C,                   Field(65, 28, 8, 8)},
    {STORE_OPCODE,                  Field(140, 16, 5, 5)},
    {LOAD_STORE_PREDICATE_POLARITY, Field(145, 21, 1, 1)},
    {LOAD_STORE_PREDICATE_ADDRESS,  Field(146, 22, 5, 5)},
    {STORE_SWITCHES,                Field(187, 36, 4, 4)},
};

static const std::map<Fields, Field> SPUInstrLayoutGen3{
    {SPU_OPCODE,             Field(2, 0, 6, 6)},
    {SPU_SRC_A,              Field(8, 6, 7, 8)},
    {SPU_SRC_B,              Field(15, 14, 7, 8)},
    {SPU_DEST,               Field(22, 22, 7, 8)},
    {SPU_OPERANDS_TYPE,      Field(29, 30, 4, 4)},
    {SPU_PREDICATE_POLARITY, Field(33, 34, 1, 1)},
    {SPU_PREDICATE_ADDRESS,  Field(34, 35, 4, 4)},
    {SPU_SWITCHES,           Field(38, 40, 7, 7)},
};

static const std::map<Fields, Field> VPUInstrLayoutGen3{
    {VPU_OPCODE,             Field(45, 0, 6, 6)},
    {VPU_SRC_A,              Field(51, 6, 8, 8)},
    {VPU_SRC_B,              Field(59, 14, 8, 8)},
    {VPU_SRC_C,              Field(224, 22, 8, 8)},
    {VPU_SRC_D,              Field(232, 30, 8, 9)},  // NB: 8 in PRM but 9 in TD, luckily the extra bit overlaps with reserved area. FIXME ??
    {VPU_DEST,               Field(67, 39, 8, 8)},
    {VPU_SWITCHES,           Field(85, 47, 3, 3)},
    {VPU_OPERANDS_TYPE,      Field(75, 50, 4, 4)},
    {VPU_PREDICATE_POLARITY, Field(79, 54, 1, 1)},
    {VPU_PREDICATE_ADDRESS,  Field(80, 55, 5, 5)},
    {VPU_SWITCHES_EXTRA,     Field(88, 60, 4, 4)},
};

static const std::map<Fields, Field> LDInstrLayoutGen3{
    {LOAD_SRC_A,                    Field(130, 0, 8, 8)},
    {LOAD_SRC_B,                    Field(151, 28, 9, 9)},
    {LOAD_DST,                      Field(138, 8, 8, 8)},
    {LOAD_OPCODE,                   Field(146, 16, 5, 5)},
    {LOAD_STORE_PREDICATE_POLARITY, Field(160, 21, 1, 1)},
    {LOAD_STORE_PREDICATE_ADDRESS,  Field(161, 22, 5, 5)},
    {LOAD_SWITCHES,                 Field(166, 37, 7, 7)},
};

static const std::map<Fields, Field> STInstrLayoutGen3{
    {STORE_SRC_A,                   Field(173, 0, 8, 8)},
    {STORE_SRC_B,                   Field(181, 8, 8, 8)},
    {STORE_SRC_C,                   Field(194, 28, 8, 8)},
    {STORE_OPCODE,                  Field(189, 16, 5, 5)},
    {LOAD_STORE_PREDICATE_POLARITY, Field(160, 21, 1, 1)},
    {LOAD_STORE_PREDICATE_ADDRESS,  Field(161, 22, 5, 5)},
    {STORE_SWITCHES,                Field(202, 36, 6, 6)},
};

static const std::map<Fields, Field> SPUInstrLayoutGen3Compressed{
    {SPU_OPCODE,             Field(2, 0, 6, 6)},
    {SPU_SRC_A,              Field(8, 6, 7, 8)},
    {SPU_SRC_B,              Field(15, 14, 7, 8)},
    {SPU_DEST,               Field(22, 22, 7, 8)},
    {SPU_OPERANDS_TYPE,      Field(29, 30, 4, 4)},
    {SPU_PREDICATE_POLARITY, Field(33, 34, 1, 1)},
    {SPU_PREDICATE_ADDRESS,  Field(34, 35, 4, 4)},
    {SPU_SWITCHES,           Field(38, 40, 7, 7)},
};

static const std::map<Fields, Field> VPUInstrLayoutGen3Compressed{
    {VPU_OPCODE,             Field(45, 0, 6, 6)},
    {VPU_SRC_A,              Field(51, 6, 8, 8)},
    {VPU_SRC_B,              Field(59, 14, 8, 8)},
    {VPU_DEST,               Field(67, 39, 8, 8)},
    {VPU_SWITCHES,           Field(85, 47, 3, 3)},
    {VPU_OPERANDS_TYPE,      Field(75, 50, 4, 4)},
    {VPU_PREDICATE_POLARITY, Field(79, 54, 1, 1)},
    {VPU_PREDICATE_ADDRESS,  Field(80, 55, 5, 5)},
    {VPU_SWITCHES_EXTRA,     Field(88, 60, 4, 4)},
};

static const std::map<Fields, Field> LDInstrLayoutGen3Compressed{
    {LOAD_SRC_A,                    Field(2, 0, 8, 8)},
    {LOAD_SRC_B,                    Field(23, 28, 9, 9)},
    {LOAD_DST,                      Field(10, 8, 8, 8)},
    {LOAD_OPCODE,                   Field(18, 16, 5, 5)},
    {LOAD_STORE_PREDICATE_POLARITY, Field(32, 21, 1, 1)},
    {LOAD_STORE_PREDICATE_ADDRESS,  Field(33, 22, 5, 5)},
    {LOAD_SWITCHES,                 Field(38, 37, 7, 7)},
};

static const std::map<Fields, Field> STInstrLayoutGen3Compressed{
    {STORE_SRC_A,                   Field(45, 0, 8, 8)},
    {STORE_SRC_B,                   Field(53, 8, 8, 8)},
    {STORE_SRC_C,                   Field(66, 28, 8, 8)},
    {STORE_OPCODE,                  Field(61, 16, 5, 5)},
    {LOAD_STORE_PREDICATE_POLARITY, Field(32, 21, 1, 1)},
    {LOAD_STORE_PREDICATE_ADDRESS,  Field(33, 22, 5, 5)},
    {STORE_SWITCHES,                Field(74, 36, 6, 6)},
};

static const std::map<Fields, Field> SPUInstrLayoutDoron1{
    {SPU_OPCODE,             Field(2, 0, 6, 6)},
    {SPU_SRC_A,              Field(8, 6, 8, 8)},
    {SPU_SRC_B,              Field(16, 14, 8, 8)},
    {SPU_DEST,               Field(24, 22, 8, 8)},
    {SPU_OPERANDS_TYPE,      Field(32, 30, 4, 4)},
    {SPU_PREDICATE_POLARITY, Field(36, 34, 1, 1)},
    {SPU_PREDICATE_ADDRESS,  Field(37, 35, 4, 4)},
    {SLOT_IS_PREDICATED,     Field(41, 39, 1, 1)},
    {SPU_SWITCHES,           Field(42, 40, 7, 7)},
};

static const std::map<Fields, Field> VPUInstrLayoutDoron1{
    {VPU_OPCODE,             Field(49, 0, 6, 6)},
    {VPU_SRC_A,              Field(55, 6, 8, 8)},
    {VPU_SRC_B,              Field(63, 14, 8, 8)},
    {VPU_SRC_C,              Field(224, 22, 8, 8)},
    {VPU_SRC_D,              Field(232, 30, 9, 9)},  // NB: 8 in PRM but 9 in TD, luckily the extra bit overlaps with reserved area. FIXME ??
    {VPU_DEST,               Field(71, 39, 8, 8)},
    {VPU_SWITCHES,           Field(90, 47, 3, 3)},
    {VPU_OPERANDS_TYPE,      Field(79, 50, 4, 4)},
    {VPU_PREDICATE_POLARITY, Field(83, 54, 1, 1)},
    {VPU_PREDICATE_ADDRESS,  Field(84, 55, 5, 5)},
    {SLOT_IS_PREDICATED,     Field(89, 0, 1, 1)}, // Just get a random bit cause there's no space for it in td encoding.
    {VPU_SWITCHES_EXTRA,     Field(93, 60, 2, 2)},
    {VPU_SWITCHES_SUPER_EXTRA,Field(128, 62, 2, 2)},
};

static const std::map<Fields, Field> LDInstrLayoutDoron1{
    {LOAD_SRC_A,                    Field(130, 0, 8, 8)},
    {LOAD_SRC_B,                    Field(151, 28, 9, 9)},
    {LOAD_DST,                      Field(138, 8, 8, 8)},
    {LOAD_OPCODE,                   Field(146, 16, 5, 5)},
    {LOAD_STORE_PREDICATE_POLARITY, Field(160, 21, 1, 1)},
    {LOAD_STORE_PREDICATE_ADDRESS,  Field(161, 22, 5, 5)},
    {SLOT_IS_PREDICATED,            Field(166, 27, 1, 1)},
    {LOAD_SWITCHES,                 Field(167, 37, 7, 7)},
};

static const std::map<Fields, Field> STInstrLayoutDoron1{
    {STORE_SRC_A,                   Field(177, 0, 8, 8)},
    {STORE_SRC_B,                   Field(185, 8, 8, 8)},
    {STORE_SRC_C,                   Field(198, 28, 8, 8)},
    {STORE_OPCODE,                  Field(193, 16, 5, 5)},
    {LOAD_STORE_PREDICATE_POLARITY, Field(206, 21, 1, 1)},
    {LOAD_STORE_PREDICATE_ADDRESS,  Field(207, 22, 5, 5)},
    {SLOT_IS_PREDICATED,            Field(212, 27, 1, 1)},
    {STORE_SWITCHES,                Field(213, 36, 7, 7)},
};

static const std::map<Fields, Field> SPUInstrLayoutDoron1Compressed{
    {SPU_OPCODE,             Field(2, 0, 6, 6)},
    {SPU_SRC_A,              Field(8, 6, 8, 8)},
    {SPU_SRC_B,              Field(16, 14, 8, 8)},
    {SPU_DEST,               Field(24, 22, 8, 8)},
    {SPU_OPERANDS_TYPE,      Field(32, 30, 4, 4)},
    {SPU_PREDICATE_POLARITY, Field(36, 34, 1, 1)},
    {SPU_PREDICATE_ADDRESS,  Field(37, 35, 4, 4)},
    {SLOT_IS_PREDICATED,      Field(41, 39, 1, 1)},
    {SPU_SWITCHES,           Field(42, 40, 7, 7)},
};

static const std::map<Fields, Field> VPUInstrLayoutDoron1Compressed{
    {VPU_OPCODE,             Field(49, 0, 6, 6)},
    {VPU_SRC_A,              Field(55, 6, 8, 8)},
    {VPU_SRC_B,              Field(63, 14, 8, 8)},
    {VPU_DEST,               Field(71, 39, 8, 8)},
    {VPU_SWITCHES,           Field(90, 47, 3, 3)},
    {VPU_OPERANDS_TYPE,      Field(79, 50, 4, 4)},
    {VPU_PREDICATE_POLARITY, Field(83, 54, 1, 1)},
    {VPU_PREDICATE_ADDRESS,  Field(84, 55, 5, 5)},
    {SLOT_IS_PREDICATED,      Field(89, 0, 1, 1)}, // Just get a random bit cause there's no space for it in td encoding.
    {VPU_SWITCHES_EXTRA,     Field(93, 60, 2, 2)}
};
static const std::map<Fields, Field> LDInstrLayoutDoron1Compressed{
    {LOAD_SRC_A,                    Field(2, 0, 8, 8)},
    {LOAD_SRC_B,                    Field(23, 28, 9, 9)},
    {LOAD_DST,                      Field(10, 8, 8, 8)},
    {LOAD_OPCODE,                   Field(18, 16, 5, 5)},
    {LOAD_STORE_PREDICATE_POLARITY, Field(32, 21, 1, 1)},
    {LOAD_STORE_PREDICATE_ADDRESS,  Field(33, 22, 5, 5)},
    {SLOT_IS_PREDICATED,      Field(38, 27, 1, 1)},
    {LOAD_SWITCHES,                 Field(39, 37, 7, 7)},
};

static const std::map<Fields, Field> STInstrLayoutDoron1Compressed{
    {STORE_SRC_A,                   Field(49, 0, 8, 8)},
    {STORE_SRC_B,                   Field(57, 8, 8, 8)},
    {STORE_SRC_C,                   Field(70, 28, 8, 8)},
    {STORE_OPCODE,                  Field(65, 16, 5, 5)},
    {LOAD_STORE_PREDICATE_POLARITY, Field(78, 21, 1, 1)},
    {LOAD_STORE_PREDICATE_ADDRESS,  Field(79, 22, 5, 5)},
    {SLOT_IS_PREDICATED,      Field(84, 27, 1, 1)},
    {STORE_SWITCHES,                Field(85, 36, 7, 7)},
};

static const std::map<Fields, Field> STInstrLayoutGen4{
    {STORE_SRC_A,                   Field(173, 0, 8, 8)},
    {STORE_SRC_B,                   Field(181, 8, 8, 8)},
    {STORE_SRC_C,                   Field(194, 28, 8, 8)},
    {STORE_OPCODE,                  Field(189, 16, 5, 5)},
    {LOAD_STORE_PREDICATE_POLARITY, Field(160, 21, 1, 1)},
    {LOAD_STORE_PREDICATE_ADDRESS,  Field(161, 22, 5, 5)},
    {STORE_SWITCHES,                Field(202, 36, 7, 7)},
};

static const std::map<Fields, Field> STInstrLayoutGen4Compressed{
    {STORE_SRC_A,                   Field(45, 0, 8, 8)},
    {STORE_SRC_B,                   Field(53, 8, 8, 8)},
    {STORE_SRC_C,                   Field(66, 28, 8, 8)},
    {STORE_OPCODE,                  Field(61, 16, 5, 5)},
    {LOAD_STORE_PREDICATE_POLARITY, Field(32, 21, 1, 1)},
    {LOAD_STORE_PREDICATE_ADDRESS,  Field(33, 22, 5, 5)},
    {STORE_SWITCHES,                Field(74, 36, 7, 7)},
};

static const std::map<Fields, Field> LoopInstrLayout{
    {SPU_OPCODE,                    Field(0, 0, 6, 6)},
    {START_VALUE,                   Field(6, 6, 7, 7)},
    {BOUNDARY_VALUE,                Field(13, 14, 7, 7)},
    {STEP_VALUE,                    Field(20, 22, 6, 6)},
    {START_VALUE_SEL,               Field(26, 28, 1, 1)},
    {BOUNDARY_VALUE_SEL,            Field(27, 29, 1, 1)},
    {STEP_VALUE_SEL,                Field(28, 30, 1, 1)},
    {REPEAT_AT_MOST_ONCE,           Field(29, 31, 1, 1)},
    {LOOP_PREDICATE_POLARITY,       Field(31, 33, 1, 1)},
    {LOOP_PREDICATE_ADDRESS,        Field(32, 34, 4, 4)},
    {COMP_MODE,                     Field(116, 38, 3, 3)},
};

static const std::map<Fields, Field> LoopCompressionInstrLayout{
    {SPU_OPCODE,                    Field(2, 0, 6, 6)},
    {START_VALUE,                   Field(8, 6, 7, 7)},
    {BOUNDARY_VALUE,                Field(15, 14, 7, 7)},
    {STEP_VALUE,                    Field(22, 22, 6, 6)},
    {START_VALUE_SEL,               Field(28, 28, 1, 1)},
    {BOUNDARY_VALUE_SEL,            Field(29, 29, 1, 1)},
    {STEP_VALUE_SEL,                Field(30, 30, 1, 1)},
    {REPEAT_AT_MOST_ONCE,           Field(31, 31, 1, 1)},
    {LOOP_PREDICATE_POLARITY,       Field(33, 33, 1, 1)},
    {LOOP_PREDICATE_ADDRESS,        Field(34, 34, 4, 4)},
    {COMP_MODE,                     Field(160, 38, 3, 3)},
};

static const std::map<Fields, Field> LoopDoron1InstrLayout{
    {SPU_OPCODE,                    Field(2, 0, 6, 6)},
    {START_VALUE,                   Field(8, 6, 8, 8)},
    {BOUNDARY_VALUE,                Field(16, 14, 8, 8)},
    {STEP_VALUE,                    Field(24, 22, 6, 6)},
    {START_VALUE_SEL,               Field(30, 28, 1, 1)},
    {BOUNDARY_VALUE_SEL,            Field(31, 29, 1, 1)},
    {STEP_VALUE_SEL,                Field(32, 30, 1, 1)},
    {REPEAT_AT_MOST_ONCE,           Field(33, 31, 1, 1)},
    {LOOP_PREDICATE_POLARITY,       Field(36, 33, 1, 1)},
    {LOOP_PREDICATE_ADDRESS,        Field(37, 34, 4, 4)},
    {LOOP_IS_PREDICATED,            Field(41, 41, 1, 1)},
    {COMP_MODE,                     Field(160, 38, 3, 3)},
};

// clang-format on

namespace llvm {
const char *getFieldName(Fields F) {
  switch (F) {
  case SPU_OPCODE:
    return "SPU_OPCODE";
  case SPU_SRC_A:
    return "SPU_SRC_A";
  case SPU_SRC_B:
    return "SPU_SRC_B";
  case SPU_DEST:
    return "SPU_DEST";
  case SPU_OPERANDS_TYPE:
    return "SPU_OPERANDS_TYPE";
  case SPU_PREDICATE_POLARITY:
    return "SPU_PREDICATE_POLARITY";
  case SPU_PREDICATE_ADDRESS:
    return "SPU_PREDICATE_ADDRESS";
  case SPU_SWITCHES:
    return "SPU_SWITCHES";
  case VPU_OPCODE:
    return "VPU_OPCODE";
  case VPU_SRC_A:
    return "VPU_SRC_A";
  case VPU_SRC_B:
    return "VPU_SRC_B";
  case VPU_SRC_C:
    return "VPU_SRC_C";
  case VPU_SRC_D:
    return "VPU_SRC_D";
  case VPU_DEST:
    return "VPU_DEST";
  case VPU_SWITCHES:
    return "VPU_SWITCHES";
  case VPU_OPERANDS_TYPE:
    return "VPU_OPERANDS_TYPE";
  case VPU_PREDICATE_POLARITY:
    return "VPU_PREDICATE_POLARITY";
  case VPU_PREDICATE_ADDRESS:
    return "VPU_PREDICATE_ADDRESS";
  case LOAD_SRC_A:
    return "LOAD_SRC_A";
  case LOAD_SRC_B:
    return "LOAD_SRC_B";
  case LOAD_DST:
    return "LOAD_DST";
  case LOAD_OPCODE:
    return "LOAD_OPCODE";
  case STORE_SRC_A:
    return "STORE_SRC_A";
  case STORE_SRC_B:
    return "STORE_SRC_B";
  case STORE_SRC_C:
    return "STORE_SRC_C";
  case STORE_OPCODE:
    return "STORE_OPCODE";
  case LOAD_STORE_PREDICATE_POLARITY:
    return "LOAD_STORE_PREDICATE_POLARITY";
  case LOAD_STORE_PREDICATE_ADDRESS:
    return "LOAD_STORE_PREDICATE_ADDRESS";
  case IMM:
    return "IMM";
  case LOAD_SWITCHES:
    return "LOAD_SWITCHES";
  case STORE_SWITCHES:
    return "STORE_SWITCHES";
  case VPU_SWITCHES_EXTRA:
    return "VPU_SWITCHES_EXTRA";
  case VPU_SWITCHES_SUPER_EXTRA:
    return "VPU_SWITCHES_SUPER_EXTRA";
  case SLOT_IS_PREDICATED:
    return "SLOT_IS_PREDICATED";
  case START_VALUE:
    return "START_VALUE";
  case BOUNDARY_VALUE:
    return "BOUNDARY_VALUE";
  case STEP_VALUE:
    return "STEP_VALUE";
  case START_VALUE_SEL:
    return "START_VALUE_SEL";
  case BOUNDARY_VALUE_SEL:
    return "BOUNDARY_VALUE_SEL";
  case STEP_VALUE_SEL:
    return "STEP_VALUE_SEL";
  case REPEAT_AT_MOST_ONCE:
    return "REPEAT_AT_MOST_ONCE";
  case LOOP_PREDICATE_POLARITY:
    return "LOOP_PREDICATE_POLARITY";
  case LOOP_PREDICATE_ADDRESS:
    return "LOOP_PREDICATE_ADDRESS";
  case LOOP_IS_PREDICATED:
    return "LOOP_IS_PREDICATED";
  case COMP_MODE:
    return "COMP_MODE";
  }
  llvm_unreachable("Unknown Field");
}

namespace TPCInstrLayout {

const std::map<Fields, Field> &
getLDInstrLayout(const bool Compressed, const FeatureBitset &TPCFeatures) {
  if (TPCFeatures[TPC::FeatureGoya] || TPCFeatures[TPC::FeatureGaudi] ||
      TPCFeatures[TPC::FeatureGaudiB])
    return LDInstrLayout;

  if (TPCFeatures[TPC::FeatureGreco] || TPCFeatures[TPC::FeatureGaudi2])
    return Compressed ? LDInstrLayoutGen3Compressed : LDInstrLayoutGen3;

  if (TPCFeatures[TPC::FeatureDoron1])
    return Compressed ? LDInstrLayoutDoron1Compressed : LDInstrLayoutDoron1;

  llvm_unreachable("Bad TPC Feature");
}

const std::map<Fields, Field> &
getSTInstrLayout(const bool Compressed, const FeatureBitset &TPCFeatures) {
  if (TPCFeatures[TPC::FeatureGoya] || TPCFeatures[TPC::FeatureGaudi] ||
      TPCFeatures[TPC::FeatureGaudiB])
    return STInstrLayout;

  if (TPCFeatures[TPC::FeatureGreco])
    return Compressed ? STInstrLayoutGen3Compressed : STInstrLayoutGen3;

  if (TPCFeatures[TPC::FeatureGaudi2])
    return Compressed ? STInstrLayoutGen4Compressed : STInstrLayoutGen4;

  if (TPCFeatures[TPC::FeatureDoron1])
    return Compressed ? STInstrLayoutDoron1Compressed : STInstrLayoutDoron1;

  llvm_unreachable("Bad TPC Feature");
}

const std::map<Fields, Field> &
getVPUInstrLayout(const bool Compressed, const FeatureBitset &TPCFeatures) {
  if (TPCFeatures[TPC::FeatureGoya] || TPCFeatures[TPC::FeatureGaudi] ||
      TPCFeatures[TPC::FeatureGaudiB])
    return VPUInstrLayout;

  if (TPCFeatures[TPC::FeatureGreco] || TPCFeatures[TPC::FeatureGaudi2])
    return Compressed ? VPUInstrLayoutGen3Compressed : VPUInstrLayoutGen3;

  if (TPCFeatures[TPC::FeatureDoron1])
    return Compressed ? VPUInstrLayoutDoron1Compressed : VPUInstrLayoutDoron1;

  llvm_unreachable("Bad TPC Feature");
}

const std::map<Fields, Field> &
getSPUInstrLayout(bool Compressed, const FeatureBitset &TPCFeatures) {
  if (TPCFeatures[TPC::FeatureGoya] || TPCFeatures[TPC::FeatureGaudi] ||
      TPCFeatures[TPC::FeatureGaudiB])
    return SPUInstrLayout;

  if (TPCFeatures[TPC::FeatureGreco] || TPCFeatures[TPC::FeatureGaudi2])
    return Compressed ? SPUInstrLayoutGen3Compressed : SPUInstrLayoutGen3;

  if (TPCFeatures[TPC::FeatureDoron1])
    return Compressed ? SPUInstrLayoutDoron1Compressed : SPUInstrLayoutDoron1;

  llvm_unreachable("Bad TPC Feature");
}

const std::map<Fields, Field> &
getLoopInstrLayout(bool Compressed, const FeatureBitset &TPCFeatures) {
  if (TPCFeatures[TPC::FeatureDoron1])
    return LoopDoron1InstrLayout;

  return Compressed ? LoopCompressionInstrLayout : LoopInstrLayout;
}

const std::map<Fields, Field> &getLDInstrLayoutUnsafe() {
  return LDInstrLayout;
}

const std::map<Fields, Field> &getSTInstrLayoutUnsafe() {
  return STInstrLayout;
}

} // namespace TPCInstrLayout
} // namespace llvm
