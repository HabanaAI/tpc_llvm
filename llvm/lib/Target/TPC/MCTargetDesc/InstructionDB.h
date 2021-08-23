//===--------InstructionDB.cpp---------------------------------------------===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//
//
// Declares interface to instruction switches database
//
//===----------------------------------------------------------------------===//

#ifndef LLVM_LIB_TARGET_TPC_MCTARGETDESC_SWITCHDATABASE_H
#define LLVM_LIB_TARGET_TPC_MCTARGETDESC_SWITCHDATABASE_H

#include "llvm/ADT/StringRef.h"
#include "llvm/MC/MCInst.h"
#include "llvm/MC/MCInstrInfo.h"
#include "llvm/Support/ErrorOr.h"
#include <string>


enum class SwitchError {
  OK = 0,
  UnknownSwitch,
  UnsupportedByHardware,
  UnapplicableForType,
  NonBooleanValueOfFlag,
  SwitchGroupNoValue,
  BitsBeyondSwitchGroup
};

namespace std {
template <> struct is_error_code_enum<SwitchError> : std::true_type {};
}

std::error_code make_error_code(SwitchError);


namespace llvm {
namespace TPCII {

// Emumerate available parsers. Numeric values must correspond to definition in
// SpecialSlotVariant, LoadSlotVariant, ScalarSlotVariant, VectorSlotVariant and
// StoreSlotVariant (see file TPC.td).
enum SlotParser {
  Special = 0,
  Load = 1,
  Scalar = 2,
  Vector = 3,
  Store = 4,
  Unknown
};

// Operand data types.
// *** Must match TPCInstrFormat*.td ***
enum OpType : unsigned {
  FP32 = 0,
  BF16 = 1,
  INT32 = 2,
  UINT32 = 3,
  INT8 = 4,
  UINT8 = 5,
  BOOL = 6,
  INT16 = 7,
  UINT16 = 8,
  INT4 = 9,
  UINT4 = 10,
  FP16 = 11,
  FP8_152 = 12,
  FP8_143 = 13,
  INT64 = 14,
  Max = INT64,
  Invalid = ~0U
};

// Special Instruction opcodes
// *** Must match TPCInstrFormat*.td ***

// Load slot
const uint64_t
  ldGEN_ADDR      = 0x00,
  ldPRMT_INDX     = 0x03,
  ldSET_INDX      = 0x04,
  ldMOV           = 0x05,
  LOOKUP          = 0x06,
  LOOKUP_C1C2     = 0x07,
  LOOKUP_C0       = 0x09,
  LOOKUP_2C       = 0x07,
  LOOKUP_1C       = 0x09,
  LD_L            = 0x0b,
  LD_G            = 0x0c,
  PREFETCH        = 0x0d,
  LD_L_V          = 0x0e,
  LD_L_V_LOW      = 0x0f,
  LD_L_V_HIGH     = 0x10,
  LD_TNSR         = 0x11,
  LD_TNSR_LOW     = 0x12,
  LD_TNSR_HIGH    = 0x13,
  LD_EVENT        = 0x18,
  ldNOP           = 0x1f;

// Scalar slot
const uint64_t
  spuMAC            = 0x00,
  spuMUL            = 0x01,
  spuADD            = 0x02,
  spuSUB            = 0x03,
  spuCONVERT_INT16  = 0x04,
  spuMAX            = 0x05,
  spuMIN            = 0x06,
  spuABS            = 0x07,
  spuMOV            = 0x08,
  spuCMP_EQ         = 0x09,
  spuCMP_NEQ        = 0x0a,
  spuCMP_LESS       = 0x0b,
  spuCMP_LEQ        = 0x0c,
  spuCMP_GRT        = 0x0d,
  spuCMP_GEQ        = 0x0e,
  spuOR             = 0x0f,
  spuAND            = 0x10,
  spuXOR            = 0x11,
  spuNOT            = 0x12,
  spuSHR            = 0x13,
  spuSHL            = 0x14,
  spuASH            = 0x15,
  spuCONVERT        = 0x16,
  spuCONVERT_INT32  = 0x17,
  spuCONVERT_UINT32 = 0x18,
  spuPOPCNT         = 0x19,
  spuFIND_FIRST     = 0x1a,
  spuNEARBYINT      = 0x1b,
  spuCONVERT_UINT16 = 0x1c,
  spuEXTRACT_EXP    = 0x1d,
  spuFCLASS         = 0x1e,
  spuBREV           = 0x1f,
  spuHALT           = 0x20,
  spuLOOP           = 0x22,
  spuJMPR           = 0x24,
  spuJMPA           = 0x25,
  spuMOV_IRF_DIM    = 0x26,
  spuSET_INDX       = 0x27,
  spuUDIV_STEP      = 0x28,
  spuUDIV_4STEP     = 0x28,
  spuUDIV           = 0x28,
  spuCALC_FP_SPECIAL= 0x34,
  spuCONVERT_INT8   = 0x35,
  spuCONVERT_UINT8  = 0x36,
  spuCONVERT_FP_FLEX= 0x38,
  spuDBG            = 0x3E,
  spuNOP            = 0x3F,
  spuCONVERT_INT64  = 0x40;

const uint64_t
  vpuMAC            = 0x00,
  vpuMUL            = 0x01,
  vpuADD            = 0x02,
  vpuSUB            = 0x03,
  vpuCONVERT_INT16  = 0x04,
  vpuMAX            = 0x05,
  vpuMIN            = 0x06,
  vpuABS            = 0x07,
  vpuMOV            = 0x08,
  vpuCMP_EQ         = 0x09,
  vpuCMP_NEQ        = 0x0a,
  vpuCMP_LESS       = 0x0b,
  vpuCMP_LEQ        = 0x0c,
  vpuCMP_GRT        = 0x0d,
  vpuCMP_GEQ        = 0x0e,
  vpuOR             = 0x0f,
  vpuAND            = 0x10,
  vpuXOR            = 0x11,
  vpuNOT            = 0x12,
  vpuSHR            = 0x13,
  vpuSHL            = 0x14,
  vpuASH            = 0x15,
  vpuCONVERT        = 0x16,
  vpuCONVERT_INT32  = 0x17,
  vpuCONVERT_UINT32 = 0x18,
  vpuPOPCNT         = 0x19,
  vpuFIND_FIRST     = 0x1a,
  vpuNEARBYINT      = 0x1b,
  vpuCONVERT_UINT16 = 0x1c,
  vpuEXTRACT_EXP    = 0x1d,
  vpuFCLASS         = 0x1e,
  vpuBREV           = 0x1f,
  vpuHALT           = 0x20,
  vpuSEL_EQ         = 0x22,
  vpuSEL_NEQ        = 0x23,
  vpuSEL_LESS       = 0x24,
  vpuSEL_LEQ        = 0x25,
  vpuSEL_GRT        = 0x26,
  vpuSEL_GEQ        = 0x27,
  vpuSEL2_LESS      = 0x28,
  vpuSEL2_LEQ       = 0x29,
  vpuSEL2_GRT       = 0x2a,
  vpuSEL2_GEQ       = 0x2b,
  vpuSHUFFLE        = 0x2c,
  vpuPACK           = 0x2d,
  vpuUNPACK         = 0x2e,
  vpuGET_LUT_ENTRY_AND_INTERVAL_START = 0x2f,
  vpuFORM_FP_NUM    = 0x30,
  vpuMOV_DUAL_GROUP = 0x31,
  vpuMOV_GROUP      = 0x32,
  vpuMSAC           = 0x33,
  vpuCALC_FP_SPECIAL =0x34,
  vpuCONVERT_INT8   = 0x35,
  vpuCONVERT_UINT8  = 0x36,
  vpuMADD           = 0x37,
  vpuCONVERT_FP_FLEX= 0x38,
  vpuDBG            = 0x3E,
  vpuNOP            = 0x3F;

// Store slot
const uint64_t
  stGEN_ADDR      = 0x00,
  stPRMT_INDX     = 0x03,
  stSET_INDX      = 0x04,
  ST_L            = 0x05,
  ST_G            = 0x06,
  ST_L_V          = 0x07,
  ST_L_V_LOW      = 0x08,
  ST_L_V_HIGH     = 0x09,
  ASO             = 0x0a,
  ST_TNSR         = 0x0b,
  ST_TNSR_LOW     = 0x0c,
  ST_TNSR_HIGH    = 0x0d,
  stLD_TNSR       = 0x11,
  stLD_TNSR_LOW   = 0x12,
  stLD_TNSR_HIGH  = 0x13,
  CACHE_FLUSH     = 0x14,
  CACHE_INVALIDATE = 0x15,
  stNOP           = 0x1f;


// Instruction switches that change signature of corresponding intrinsics. These
// switches must not be exposed to user in C/C++ code.
// *** Must match SwitchVal/SW definitions in TPCInstrFormat*.td ***
enum : unsigned {
  SW_ACC_FP32           = 1U << 2,
  SW_ACC_I16            = 1U << 2,
  SW_DIM_MASK_REG       = 1U << 1,
  SW_DIM_MASK_REG_G4    = 1U << 3,
  SW_ACC_I32            = 1U << 3,
  SW_X2_ARITHMETIC      = 1U << 4,
  SW_ZP                 = 1U << 5,
  SW_PARTIAL            = 1U << 0,
  SW_HW_REG             = 1U << 0,
  SW_RMW_SEL            = 1U << 1,
  SW_DOUBLE_AND_ROUND32 = 1U << 1,
  SW_X2_MOV             = 1U << 2,

  SW_NUM_LANES_SRCB     = 1U << 6,
  SW_ALL_LANES_SRCB     = 0,
  SW_SINGLE_LANE_SRCB   = SW_NUM_LANES_SRCB,
  SW_X2_CONVERT         = 1U << 7,
  SW_CLIP_FP16          = 1U << 7,

  SW_NUM_LANES          = 1U << 2,
  SW_ALL_LANES          = 0,
  SW_SINGLE_LANE        = SW_NUM_LANES,

  SW_TYPE               = 0x0f,
  SW_FP32               = OpType::FP32,
  SW_BF16               = OpType::BF16,
  SW_INT32              = OpType::INT32,
  SW_UINT32             = OpType::UINT32,
  SW_INT8               = OpType::INT8,
  SW_UINT8              = OpType::UINT8,
  SW_INT16              = OpType::INT16,
  SW_UINT16             = OpType::UINT16,
  SW_FP16               = OpType::FP16,
  SW_INT64              = OpType::INT64,

  SW_TO_TYPE            = 0x0f << 8,
  SW_TO_FP32            = OpType::FP32 << 8,
  SW_TO_BF16            = OpType::BF16 << 8,
  SW_TO_INT32           = OpType::INT32 << 8,
  SW_TO_UINT32          = OpType::UINT32 << 8,
  SW_TO_INT8            = OpType::INT8 << 8,
  SW_TO_UINT8           = OpType::UINT8 << 8,
  SW_TO_INT16           = OpType::INT16 << 8,
  SW_TO_UINT16          = OpType::UINT16 << 8,
  SW_TO_FP16            = OpType::FP16 << 8,
  SW_TO_FP8_152         = OpType::FP8_152 << 8,
  SW_TO_FP8_143         = OpType::FP8_143 << 8,

  SW_GROUP_TO           = 1 << (16 + 3),
  SW_TO_8               = 0,
  SW_TO_16              = SW_GROUP_TO,
  SW_CNVRT              = 1U << 4,

  SW_LIMIT              = 1U << 0,
  SW_MOV_IRF_DIM_BOTH   = 1U << 3
};

// DimMask as switch.
enum : unsigned {
  SW_DIMMASK = 0x1F << 2,
  SW_B00000 = 0,
  SW_B00001 = 0x01 << 2,
  SW_B00010 = 0x02 << 2,
  SW_B00011 = 0x03 << 2,
  SW_B00100 = 0x04 << 2,
  SW_B00101 = 0x05 << 2,
  SW_B00110 = 0x06 << 2,
  SW_B00111 = 0x07 << 2,
  SW_B01000 = 0x08 << 2,
  SW_B01001 = 0x09 << 2,
  SW_B01010 = 0x0A << 2,
  SW_B01011 = 0x0B << 2,
  SW_B01100 = 0x0C << 2,
  SW_B01101 = 0x0D << 2,
  SW_B01110 = 0x0E << 2,
  SW_B01111 = 0x0F << 2,
  SW_B10000 = 0x10 << 2,
  SW_B10001 = 0x11 << 2,
  SW_B10010 = 0x12 << 2,
  SW_B10011 = 0x13 << 2,
  SW_B10100 = 0x14 << 2,
  SW_B10101 = 0x15 << 2,
  SW_B10110 = 0x16 << 2,
  SW_B10111 = 0x17 << 2,
  SW_B11000 = 0x18 << 2,
  SW_B11001 = 0x19 << 2,
  SW_B11010 = 0x1A << 2,
  SW_B11011 = 0x1B << 2,
  SW_B11100 = 0x1C << 2,
  SW_B11101 = 0x1D << 2,
  SW_B11110 = 0x1E << 2,
  SW_B11111 = 0x1F << 2
};

// Instruction switches.
const unsigned SW_EVENT_SLOT      = 1U << 0;
const unsigned SW_SAT             = 1U << 0;
const unsigned SW_FP16_FTZ_IN     = 1U << 0;
const unsigned SW_CARRY           = 1U << 1;
const unsigned SW_NO_CARRY_GEN    = 1U << 2;
const unsigned SW_NEG             = 1U << 1;
const unsigned SW_MASK_EQ_ZERO    = 1U << 0;
const unsigned SW_UPPER32         = 1U << 2;
const unsigned SW_AUTO_INC_G3     = 1U << 0;
const unsigned SW_INC_VAL_G3      = 0x3 << 2;
const unsigned SW_INC_1_G3        = 0;
const unsigned SW_INC_2_G3        = 1  << 2;
const unsigned SW_INC_4_G3        = 2  << 2;
const unsigned SW_INC_8_G3        = 3  << 2;
const unsigned SW_INC_VAL         = 0x7;
const unsigned SW_INC_0           = 0;
const unsigned SW_INC_1           = 1;
const unsigned SW_INC_2           = 2;
const unsigned SW_INC_4           = 3;
const unsigned SW_INC_8           = 4;
const unsigned SW_BV64            = 1 << 4;
const unsigned SW_ST_TNSR_S_BV64  = 1 << 2;
const unsigned SW_EV_HINT         = 1U << 6;
const unsigned SW_SUBTRACT_BIAS   = 1U << 0;
const unsigned SW_GROUP_RND32     = 0x03;
const unsigned SW_RND32_NO_ROUND  = 0;
const unsigned SW_RND32_DNR32     = 0x1;
const unsigned SW_RND32_KEEP_RS   = 0x2;
const unsigned SW_RND32_KEEP_RS_FOR_ADD = 0x3;
const unsigned SW_DEC             = 1U << 0;
const unsigned SW_VPU             = 1U << 1;
const unsigned SW_LANE_SEL        = 0x3;
const unsigned SW_LANE_0          = 0;
const unsigned SW_LANE_1          = 0x1;
const unsigned SW_LANE_2          = 0x2;
const unsigned SW_LANE_3          = 0x3;
const unsigned SW_UNPACK_DT       = 0x3 << 1;
const unsigned SW_UNPCK_16_TO_32  = 0;
const unsigned SW_UNPCK_8_TO_16   = 0x1 << 1;
const unsigned SW_UNPCK_8_TO_32   = 0x2 << 1;
const unsigned SW_UNPCK_4_TO_8    = 0x03 << 1;
const unsigned SW_UNPACK          = 1 << 4;
const unsigned SW_L0CD            = 1 << 5;
const unsigned SW_DIRECT          = 1 << 6;
const unsigned SW_PACK            = 1 << 2;
const unsigned SW_PACK_DT         = 0x03 << 4;
const unsigned SW_PCK_32_TO_16    = 0;
const unsigned SW_PCK_16_TO_8     = 1 << 4;
const unsigned SW_PCK_32_TO_8     = 2 << 4;
const unsigned SW_PCK_8_TO_4      = 3 << 4;
const unsigned SW_COUNT           = 1;
const unsigned SW_COUNT_ZEROS     = 0;
const unsigned SW_COUNT_ONES      = 1;
const unsigned SW_FIND_BIT        = 1;
const unsigned SW_FIND_ZERO       = 0;
const unsigned SW_FIND_ONE        = 1;
const unsigned SW_DIRECTION       = 1 << 1;
const unsigned SW_LSB             = 0;
const unsigned SW_MSB             = SW_DIRECTION;
const unsigned SW_GROUP_SOURCE    = 1 << 8;
const unsigned SW_GROUP_0         = 0;
const unsigned SW_GROUP_1         = SW_GROUP_SOURCE;
const unsigned SW_ELEMENT_STRIDE  = 1 << 9;
const unsigned SW_STRIDE_2        = 0;
const unsigned SW_STRIDE_4        = SW_ELEMENT_STRIDE;
const unsigned SW_GROUP_HALF      = 1 << 10;
const unsigned SW_GROUP_HALF_0    = 0;
const unsigned SW_GROUP_HALF_1    = SW_GROUP_HALF;
const unsigned SW_UNPACK_LANE     = 3 << 11;
const unsigned SW_UNPACK_LANE_0   = 0;
const unsigned SW_UNPACK_LANE_1   = 1 << 11;
#if 0 // until it used (LLVM-965).
const unsigned SW_UNPACK_LANE_2   = 2 << 11;
const unsigned SW_UNPACK_LANE_3   = SW_UNPACK_LANE;
#endif
const unsigned SW_LUT_FUNC        = 0x07 << (8 + 5);
const unsigned SW_LUT_TANH        = 0x01 << (8 + 5);
const unsigned SW_LUT_SQRT_RSQRT  = 0x02 << (8 + 5);
const unsigned SW_LUT_SIN_COS     = 0x03 << (8 + 5);
const unsigned SW_LUT_LOG         = 0x04 << (8 + 5);
const unsigned SW_LUT_OPT         = 1 << 0;
const unsigned SW_LUT_EXP0        = 1 << 1;
const unsigned SW_ADD_BIAS        = 1 << 8;
const unsigned SW_FORCE_SIGN0     = 1 << 9;
const unsigned SW_FORCE_SIGN1     = 1 << 10;
const unsigned SW_EXP_IS_NUM      = 1 << 11;
const unsigned SW_SIGN_LSB        = 1 << 12;
const unsigned SW_DT_OVERRIDE     = 1 << 4;
const unsigned SW_MMIO            = 1;
const unsigned SW_LOCK            = 1 << 1;
const unsigned SW_UNLOCK          = 1 << 1;
const unsigned SW_NEG_ZP          = 1U << 6;
const unsigned SW_SB              = 1U << 0;
const unsigned SW_PD              = 1U << 15;
const unsigned SW_D               = 1U << 1;
const unsigned SW_LU              = 1U << 2;
const unsigned SW_RST_LU          = 1U << 3;
const unsigned SW_RST_D_PREF      = 1U << 4;
const unsigned SW_NO_BORROW_GEN = 1U << 2;
const unsigned SW_BORROW          = 1U << 3;
const unsigned SW_ST_INC          = 0x03U;
const unsigned SW_V_INC_0         = 0U;
const unsigned SW_V_INC_1         = 0x01U;
const unsigned SW_V_INC_2         = 0x02U;
const unsigned SW_V_INC_4         = 0x03U;

// MOV_DUAL_GROUP switches. Switches passed in SrcB are shifted by 8,
// in SrcC - by 16.
const unsigned SW_MDG_TYPE_MASK         = 3;
const unsigned SW_MDG_TYPE_SINGLE       = 0;
const unsigned SW_MDG_TYPE_ALL          = 1;
const unsigned SW_MDG_TYPE_PACK         = 2;
const unsigned SW_MDG_TYPE_UNPACK       = 3;
const unsigned SW_DUAL_GROUP_PACK_TYPE  = 4;
const unsigned SW_PACK21                = 0;
const unsigned SW_PACK41                = SW_DUAL_GROUP_PACK_TYPE;
const unsigned SW_MDG_UNPACK_TYPE       = 4;
const unsigned SW_UNPACK_UPDATE_LANE0   = 0;
const unsigned SW_UNPACK_UPDATE_LANE1   = SW_MDG_UNPACK_TYPE;
const unsigned SW_SRC_DUAL_GROUP_SHIFT  = 8;
const unsigned SW_DST_DUAL_GROUP_SHIFT = 10;
const unsigned SW_SRC_DUAL_GROUP        = 0x03 << SW_SRC_DUAL_GROUP_SHIFT;
const unsigned SW_DST_DUAL_GROUP        = 0x03 << SW_DST_DUAL_GROUP_SHIFT;
const unsigned SW_WR_LOWER_GROUP        = 1 << 12;
const unsigned SW_WR_UPPER_GROUP        = 1 << 13;
const unsigned SW_SDG0_SHIFT            = 8;
const unsigned SW_SDG1_SHIFT            = 10;
const unsigned SW_SDG2_SHIFT            = 12;
const unsigned SW_SDG3_SHIFT            = 14;
const unsigned SW_SDG0                  = 0x03 << SW_SDG0_SHIFT;
const unsigned SW_SDG1                  = 0x03 << SW_SDG1_SHIFT;
const unsigned SW_SDG2                  = 0x03 << SW_SDG2_SHIFT;
const unsigned SW_SDG3                  = 0x03 << SW_SDG3_SHIFT;
const unsigned SW_WEG0_SHIFT            = 16;
const unsigned SW_WEG1_SHIFT            = 18;
const unsigned SW_WEG2_SHIFT            = 20;
const unsigned SW_WEG3_SHIFT            = 22;
const unsigned SW_WEG0                  = 0x03 << SW_WEG0_SHIFT;
const unsigned SW_WEG1                  = 0x03 << SW_WEG1_SHIFT;
const unsigned SW_WEG2                  = 0x03 << SW_WEG2_SHIFT;
const unsigned SW_WEG3                  = 0x03 << SW_WEG3_SHIFT;
const unsigned SW_WR_LOWER_GROUP0       = 1 << 16;
const unsigned SW_WR_UPPER_GROUP0       = 1 << 17;
const unsigned SW_WR_LOWER_GROUP1       = 1 << 18;
const unsigned SW_WR_UPPER_GROUP1       = 1 << 19;
const unsigned SW_WR_LOWER_GROUP2       = 1 << 20;
const unsigned SW_WR_UPPER_GROUP2       = 1 << 21;
const unsigned SW_WR_LOWER_GROUP3       = 1 << 22;
const unsigned SW_WR_UPPER_GROUP3       = 1 << 23;

// Round modes. Are used in CONVERT* instructions. Bit values differ depending
// on core and instruction.
// As these switches are encoded in OperandType, put them to the 3rd byte.
const unsigned SW_GROUP_RM  = 0x070000;
const unsigned SW_RHNE      = 0;
const unsigned SW_RZ        = 0x010000;
const unsigned SW_RU        = 0x020000;
const unsigned SW_RD        = 0x030000;
const unsigned SW_SR        = 0x040000;
const unsigned SW_CSR       = 0x050000;
const unsigned SW_RHAZ      = 0x060000;
const unsigned SW_SR_RNE    = 0x070000;
const unsigned SW_X4_CONVERT= 0x080000;
const unsigned SW_CLIP_FP   = 0x100000;
const unsigned SW_G1_RHNE   = 0;
const unsigned SW_G1_RD     = 0x010000;
const unsigned SW_G1_RU     = 0x020000;
const unsigned SW_G1_SR     = 0x030000;
const unsigned SW_G1_RZ     = 0x040000;

// MSAC switches
const unsigned SW_RHU          = 0x2;
const unsigned SW_NORMALIZE    = 0x4;
const unsigned SW_NORMALIZE_AB = 0x0;
const unsigned SW_NORMALIZE_C  = 0x4;

// LOOKUP switches
const unsigned SW_LOOKUP_G1 = 0x7;
const unsigned SW_BV32      = 0;
const unsigned SW_BV16_LOW  = 0x2;
const unsigned SW_BV16_HIGH = 0x3;
const unsigned SW_BV8_0     = 0x4;
const unsigned SW_BV8_1     = 0x5;
const unsigned SW_BV8_2     = 0x6;
const unsigned SW_BV8_3     = 0x7;

const unsigned SW_LOOKUP_G2 = 0x3;
const unsigned SW_LOOKUP_G3 = 0x3;
const unsigned SW_BV16      = 0x1;
const unsigned SW_BV8_0_G3  = 0x2;
const unsigned SW_BV8_1_G3  = 0x3;

const unsigned SW_GROUP_EN      = 0x3;
const unsigned SW_DUAL_GROUP_EN = 0x3c;

const unsigned SW_UPPER_HALF = 1 << 2;
const unsigned SW_LUT_PTR    = 1 << 3;
const unsigned SW_SBCD       = 1 << 5;

// Function codes in CALC_FP_SPECIAL.
const unsigned SW_FUNCID  = 0x7;
const unsigned SW_RECIP   = 0;
const unsigned SW_RSQRT   = 1;
const unsigned SW_SQRT    = 2;
const unsigned SW_LOG     = 3;
const unsigned SW_EXP     = 4;
const unsigned SW_TANH    = 5;
const unsigned SW_DIV     = 6;
const unsigned SW_POW     = 7;

// Function codes in FORM_FP_NUMBER
const unsigned SW_SPECIAL_FUNC    = 7 << 13;
const unsigned SW_PRE_SQRT_RSQRT  = 1 << 13;
const unsigned SW_POST_SQRT       = 2 << 13;
const unsigned SW_POST_RSQRT      = 3 << 13;
const unsigned SW_POST_RECIP      = 4 << 13;

const unsigned SW_RHAZ_RS = 0x04;
const unsigned SW_NO_SAT = 0x01;
const unsigned SW_STEP_REG = 0x1 << 5;
const unsigned SW_X2_UDIV_4STEP = 0x1 << 6;

// UDIV
const unsigned SW_GROUP_DIV_MODE = 3;
const unsigned SW_DIV_MODE_DIV = 0;
const unsigned SW_DIV_MODE_MOD = 1;
const unsigned SW_DIV_MODE_BOTH = 2;

// PARTIAL & PD switches are passed via Src2 but not via LOAD or STORE slot switches.
// So let's keep the slot switches 7 bits, and start Src2 switches from eighth bit.
const unsigned SW_PARTIAL_SRCB = 1 << 8;

// AND
const unsigned SW_ANDN = 1 << 1;

// SET_INDX
const unsigned SW_IRF44_HIGH = 1;

void setSubTargetInfo(const MCSubtargetInfo *STI);
const MCSubtargetInfo *getSubtargetInfo();

/// @brief Update switch set given new switch name.
/// @return Text of error if the operation cannot be executed or empty string in
/// the case of success.
///
std::string incorporateSwitch(
  StringRef Switch,     ///< text of the switch, like 'PACK'.
  StringRef Value,      ///< Value of the switch if it is specified in form 'SWITCH=VALUE'.
  StringRef Mnemonic,   ///< Instruction name, like 'ADD'.
  SlotParser Slot,      ///< Slot where the instruction resides.
  OpType Type,          ///< OpType of the instruction if presents, OpTypeInvalid otherwise.
  bool IsSuffix,        ///< True if the switch is found in instruction suffix.
  bool &IsUnknown,      ///< Is set to True if the switch is unknown.
  unsigned &CurrentSwitchSet,         ///< Current value of switch set.
  std::vector<std::string> &Switches  ///< Set of switches specified explicitly.
);

/// @brief Assign default values to switches that have non-zero default values.
/// @return True if switch set was updated.
bool getDefaultSwitches(
    StringRef Mnemonic,   ///< Instruction name, like 'ADD'.
    SlotParser Slot,      ///< Slot where the instruction resides.
    OpType Type,          ///< OpType of the instruction if presents, OpTypeInvalid otherwise.
    unsigned &CurrentSwitchSet,               ///< Current value of switch set.
    const std::vector<std::string> &Switches  ///< Set of switches in the switch set.
);

bool doesInstructionHasASwitch(StringRef Mnemonic, SlotParser Slot);

std::string spellSwitchSet(unsigned SwSet, const MCInst *MI, unsigned OpNum, const MCInstrDesc &MCID, const MCRegisterInfo &MRI);

}
}
#endif
