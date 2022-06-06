//===----------------------------------------------------------------------===//
//
//===----------------------------------------------------------------------===//
//
// Information about instruction suffixes.
//
//===----------------------------------------------------------------------===//

#include "InstructionDB.h"
#include "MCTargetDesc/TPCMCInstrInfo.h"
#include "MCTargetDesc/TPCMCTargetDesc.h"
#include "TPCRegisterInfo.h"
#include "llvm/MC/MCSubtargetInfo.h"
#include "llvm/Support/Debug.h"

#include <algorithm>
#include <cctype>
#include <cstdlib>
#include <map>
#include <set>
#include <sstream>
#include <unordered_map>
#include <utility>

#define DEBUG_TYPE "TPCinstructionDB"
#if 1
#  define DISASM_DEBUG(X)
#else
#  define DISASM_DEBUG(X) LLVM_DEBUG(X)
#endif

using namespace llvm;
using namespace llvm::TPCII;

// Constant used as a value of SwitchInfo::Features if the switch is valid for all processors.
const unsigned AllCores = ~0U;

// Bit mask of issue slots.
const unsigned S_Special = 0x01;
const unsigned S_Load    = 0x02;
const unsigned S_Scalar  = 0x04;
const unsigned S_Vector  = 0x08;
const unsigned S_Store   = 0x10;

const unsigned S_Arith   = S_Scalar | S_Vector;
// const unsigned S_Mem     = S_Load | S_Store;

inline unsigned getSlotCode(SlotParser x) {
  switch (x) {
  case SlotParser::Special: return S_Special;
  case SlotParser::Load: return S_Load;
  case SlotParser::Scalar: return S_Scalar;
  case SlotParser::Vector: return S_Vector;
  case SlotParser::Store: return S_Store;
  default: llvm_unreachable("Invalid slot code");
  }
}

inline SlotParser getSlotParserCode(unsigned x) {
  switch (x) {
  case S_Special: return SlotParser::Special;
  case S_Load: return SlotParser::Load;
  case S_Scalar: return SlotParser::Scalar;
  case S_Vector: return SlotParser::Vector;
  case S_Store: return SlotParser::Store;
  default: llvm_unreachable("Invalid slot code");
  }
}


const unsigned T_FP32    = 1 << OpType::FP32;
const unsigned T_BF16    = 1 << OpType::BF16;
const unsigned T_INT32   = 1 << OpType::INT32;
const unsigned T_UINT32  = 1 << OpType::UINT32;
const unsigned T_INT8    = 1 << OpType::INT8;
const unsigned T_UINT8   = 1 << OpType::UINT8;
const unsigned T_BOOL    = 1 << OpType::BOOL;
const unsigned T_INT16   = 1 << OpType::INT16;
const unsigned T_UINT16  = 1 << OpType::UINT16;
const unsigned T_INT4    = 1 << OpType::INT4;
const unsigned T_UINT4   = 1 << OpType::UINT4;
const unsigned T_FP16    = 1 << OpType::FP16;
const unsigned T_F8_143  = 1 << OpType::FP8_143;
const unsigned T_F8_152  = 1 << OpType::FP8_152;
const unsigned T_INT64   = 1 << OpType::INT64;
const unsigned T_FLOAT   = T_FP32 | T_BF16 | T_FP16 | T_F8_143 | T_F8_152;
const unsigned T_INTEGER = T_INT32 | T_UINT32 | T_INT8 | T_UINT8 | T_INT16 | T_UINT16;
const unsigned T_F8 = T_F8_143 | T_F8_152;
const unsigned T_ALL = ~0U;

inline unsigned getTypeCode(TPCII::OpType T) {
  switch ((unsigned)T) {
    case OpType::FP32: return T_FP32;
    case OpType::BF16: return T_BF16;
    case OpType::INT32: return T_INT32;
    case OpType::UINT32: return T_UINT32;
    case OpType::INT8: return T_INT8;
    case OpType::UINT8: return T_UINT8;
    case OpType::BOOL: return T_BOOL;
    case OpType::INT16: return T_INT16;
    case OpType::UINT16: return T_UINT16;
    case OpType::INT4: return T_INT4;
    case OpType::UINT4: return T_UINT4;
    case OpType::FP16: return T_FP16;
    case OpType::FP8_143: return T_F8_143;
    case OpType::FP8_152: return T_F8_152;
    case OpType::INT64: return T_INT64;
    case OpType::Invalid: return T_ALL;
    default:
      llvm_unreachable("invalid type");
    }
}


// Represents an instruction opcode. As we have several slots each having its own
// instruction set, numerical value of opcode is not sufficient to disambigute
// an instruction.
struct OpCodeKey {
  SlotParser Slot;
  unsigned   OpCode;

  friend bool operator < (const OpCodeKey &A, const OpCodeKey &B) {
    if (A.Slot < B.Slot)
      return true;
    if (A.Slot > B.Slot)
      return false;
    return A.OpCode < B.OpCode;
  }
};


// Represents an instruction name. As we have several slots each having its own
// instruction set, just an instruction name (like 'set_indx') is not sufficient
// to disambigute an instruction.
struct MnemonicKey {
  std::string Mnemonic;
  unsigned Slot;

  friend bool operator < (const MnemonicKey &A, const MnemonicKey &B) {
    if (A.Slot < B.Slot)
      return true;
    if (A.Slot > B.Slot)
      return false;
    return A.Mnemonic < B.Mnemonic;
  }

  friend bool operator == (const MnemonicKey &A, const MnemonicKey &B) {
    return A.Slot == B.Slot && A.Mnemonic == B.Mnemonic;
  }
};


// Represents a value of multibit switch (like RHNE of CONVERT mode).
struct SwitchValue {
  StringRef   Name;   // Name (like RHNE)
  unsigned    Value;  // Bit value (like SW_RHNE)
};


/// Describe a dependecy of a switch on another switch value.
///
/// For example, the switch WEG0 makes sense only if the switch ALL is set. In
/// this case the switch "ALL" is a dependency for "WEG0".
///
struct Dependency {
  /// Name of the switch on which the described switch depends on. For example,
  /// "ALL" for "WEG0".
  StringRef Switch;

  /// Value of the dependency switch, which must be set to make the described
  /// switch valid. For example, "ALL" must be set to "1" for switch "WEG0" be
  /// valid.
  unsigned Value;
};


/// Requests instruction printer to always print the switch in the
/// form "key=value".
const unsigned OptPrintWithKey = 0x01;

/// Requests instruction printer to always print the switch even if it has
/// default value.
const unsigned OptPrintAlways = 0x02;

/// Flag that indicates that the switch should not be printed. It is useful, if
/// the switch is already encoded in the instruction name.
const unsigned OptDontPrint = 0x04;

/// Flag that indicates the switch is an alias (e.g. for backward compatibilty).
/// Such switch participates in parsing assembler sources but not in printing.
const unsigned OptAlias = 0x08;

/// Flag that indicates that the switch value must printed in binary format.
const unsigned OptPrintAsBinary = 0x10;

/// If this flag presents, the switch value can be used as instruction suffix,
/// even if it is a member of multibit set. One-bit flags are always allowed in
/// suffix.
const unsigned OptMayBeSuffix = 0x20;


// Flag that indicates that the switch contain immediate mask.
const unsigned IsImmMask = 0x10000000;

const unsigned PrintFlagMask = IsImmMask;

/// Keep information about a switch.
struct SwitchInfo {
  /// Switch name, like 'acc_fp32'.
  StringRef Name;

  /// Instructions where the switch may appear.
  std::vector<MnemonicKey> Instructions;

  /// Bit mask corresponding to the switch. For one-bit switch, like SW_SAT it
  /// is just the value or the switch. For multibit switch, like rounding mode,
  /// it represents bit mask of all pertinent bits.
  unsigned Value;

  /// The feature, like FeatureFP8, required for this switch.
  unsigned Feature;

  /// Instruction optypes for which the switch is valid.
  unsigned OpType;

  /// Keeps all possible values for multibit switch.
  std::vector<SwitchValue> Values;

  /// Value of the switch if it is not specified in assembler form.
  unsigned Default;

  /// Set of dependencies.
  std::vector<Dependency> Dependencies;

  /// Various option for the switch, for example, how to print the switch.
  unsigned Options;

  bool isGroup() const {
    return !Values.empty() || std::bitset<32>(Value).count() > 1;
  }

  bool fitsTargetFeatures(const FeatureBitset &TargetFeatures) const {
    return (Feature == AllCores || TargetFeatures[Feature]);
  }

  bool fitsOtherFeatures(const SwitchInfo &Other) const {
    /*FIXME (Other.Features == Features || Other.Features == AllCores
      || (Features == TPC::FeatureBF16 && Other.Features == TPC::FeatureGen3Plus))*/
    return true;
  }
};


struct DependencyRecord {
  const SwitchInfo *Switch;
  unsigned Selection;
};


// Keeps information about switches that can be used in an instruction.
struct InstructionInfo {
  StringRef Name;
  std::set<const SwitchInfo *> Switches;
};


// Database of all instruction opcodes.
static const std::multimap<OpCodeKey, StringRef> AllOpCodes = {
  std::make_pair(OpCodeKey{ SlotParser::Load, TPCII::ldGEN_ADDR }, "GEN_ADDR"),
  std::make_pair(OpCodeKey{ SlotParser::Load, TPCII::ldPRMT_INDX }, "PRMT_INDX"),
  std::make_pair(OpCodeKey{ SlotParser::Load, TPCII::ldSET_INDX }, "SET_INDX"),
  std::make_pair(OpCodeKey{ SlotParser::Load, TPCII::ldMOV }, "MOV"),
  std::make_pair(OpCodeKey{ SlotParser::Load, TPCII::LOOKUP }, "LOOKUP"),
  std::make_pair(OpCodeKey{ SlotParser::Load, TPCII::LOOKUP_C1C2 }, "LOOKUP_C1C2"),
  std::make_pair(OpCodeKey{ SlotParser::Load, TPCII::LOOKUP_2C }, "LOOKUP_2C"),
  std::make_pair(OpCodeKey{ SlotParser::Load, TPCII::LOOKUP_C0 }, "LOOKUP_C0"),
  std::make_pair(OpCodeKey{ SlotParser::Load, TPCII::LOOKUP_1C }, "LOOKUP_1C"),
  std::make_pair(OpCodeKey{ SlotParser::Load, TPCII::LD_L }, "LD_L"),
  std::make_pair(OpCodeKey{ SlotParser::Load, TPCII::LD_G }, "LD_G"),
  std::make_pair(OpCodeKey{ SlotParser::Load, TPCII::PREFETCH }, "PREFETCH"),
  std::make_pair(OpCodeKey{ SlotParser::Load, TPCII::LD_L_V }, "LD_L_V"),
  std::make_pair(OpCodeKey{ SlotParser::Load, TPCII::LD_L_V_LOW }, "LD_L_V_LOW"),
  std::make_pair(OpCodeKey{ SlotParser::Load, TPCII::LD_L_V_HIGH }, "LD_L_V_HIGH"),
  std::make_pair(OpCodeKey{ SlotParser::Load, TPCII::LD_TNSR }, "LD_TNSR"),
  std::make_pair(OpCodeKey{ SlotParser::Load, TPCII::LD_TNSR_LOW }, "LD_TNSR_LOW"),
  std::make_pair(OpCodeKey{ SlotParser::Load, TPCII::LD_TNSR_HIGH }, "LD_TNSR_HIGH"),
  std::make_pair(OpCodeKey{ SlotParser::Load, TPCII::LD_FCLASS }, "FCLASS"),
  std::make_pair(OpCodeKey{ SlotParser::Load, TPCII::LD_CALC_FP_SPECIAL }, "CALC_FP_SPECIAL"),
  std::make_pair(OpCodeKey{ SlotParser::Load, TPCII::LD_EVENT }, "EVENT"),
  std::make_pair(OpCodeKey{ SlotParser::Load, TPCII::LD_TNSR_CNVRT}, "LD_TNSR_CNVRT"),
  std::make_pair(OpCodeKey{ SlotParser::Load, TPCII::ldNOP }, "NOP"),

  std::make_pair(OpCodeKey{ SlotParser::Scalar, TPCII::spuMAC }, "MAC"),
  std::make_pair(OpCodeKey{ SlotParser::Scalar, TPCII::spuMUL }, "MUL"),
  std::make_pair(OpCodeKey{ SlotParser::Scalar, TPCII::spuADD }, "ADD"),
  std::make_pair(OpCodeKey{ SlotParser::Scalar, TPCII::spuSUB }, "SUB"),
  std::make_pair(OpCodeKey{ SlotParser::Scalar, TPCII::spuCONVERT_INT16 }, "CONVERT_INT16"),
  std::make_pair(OpCodeKey{ SlotParser::Scalar, TPCII::spuMAX }, "MAX"),
  std::make_pair(OpCodeKey{ SlotParser::Scalar, TPCII::spuMIN }, "MIN"),
  std::make_pair(OpCodeKey{ SlotParser::Scalar, TPCII::spuABS }, "ABS"),
  std::make_pair(OpCodeKey{ SlotParser::Scalar, TPCII::spuMOV }, "MOV"),
  std::make_pair(OpCodeKey{ SlotParser::Scalar, TPCII::spuCMP_EQ }, "CMP_EQ"),
  std::make_pair(OpCodeKey{ SlotParser::Scalar, TPCII::spuCMP_NEQ }, "CMP_NEQ"),
  std::make_pair(OpCodeKey{ SlotParser::Scalar, TPCII::spuCMP_LESS }, "CMP_LESS"),
  std::make_pair(OpCodeKey{ SlotParser::Scalar, TPCII::spuCMP_LEQ }, "CMP_LEQ"),
  std::make_pair(OpCodeKey{ SlotParser::Scalar, TPCII::spuCMP_GRT }, "CMP_GRT"),
  std::make_pair(OpCodeKey{ SlotParser::Scalar, TPCII::spuCMP_GEQ }, "CMP_GEQ"),
  std::make_pair(OpCodeKey{ SlotParser::Scalar, TPCII::spuOR }, "OR"),
  std::make_pair(OpCodeKey{ SlotParser::Scalar, TPCII::spuAND }, "AND"),
  std::make_pair(OpCodeKey{ SlotParser::Scalar, TPCII::spuXOR }, "XOR"),
  std::make_pair(OpCodeKey{ SlotParser::Scalar, TPCII::spuNOT }, "NOT"),
  std::make_pair(OpCodeKey{ SlotParser::Scalar, TPCII::spuSHR }, "SHR"),
  std::make_pair(OpCodeKey{ SlotParser::Scalar, TPCII::spuSHL }, "SHL"),
  std::make_pair(OpCodeKey{ SlotParser::Scalar, TPCII::spuASH }, "ASH"),
  std::make_pair(OpCodeKey{SlotParser::Scalar, TPCII::spuCONVERT_INT64}, "CONVERT.INT64"),
  std::make_pair(OpCodeKey{SlotParser::Scalar, TPCII::spuCONVERT}, "CONVERT"),
  std::make_pair(OpCodeKey{ SlotParser::Scalar, TPCII::spuCONVERT_INT32 }, "CONVERT_INT32"),
  std::make_pair(OpCodeKey{ SlotParser::Scalar, TPCII::spuCONVERT_UINT32 }, "CONVERT_UINT32"),
  std::make_pair(OpCodeKey{ SlotParser::Scalar, TPCII::spuPOPCNT }, "POPCNT"),
  std::make_pair(OpCodeKey{ SlotParser::Scalar, TPCII::spuFIND_FIRST }, "FIND_FIRST"),
  std::make_pair(OpCodeKey{ SlotParser::Scalar, TPCII::spuNEARBYINT }, "NEARBYINT"),
  std::make_pair(OpCodeKey{ SlotParser::Scalar, TPCII::spuCONVERT_UINT16 }, "CONVERT_UINT16"),
  std::make_pair(OpCodeKey{ SlotParser::Scalar, TPCII::spuEXTRACT_EXP }, "EXTRACT_EXP"),
  std::make_pair(OpCodeKey{ SlotParser::Scalar, TPCII::spuFCLASS }, "FCLASS"),
  std::make_pair(OpCodeKey{ SlotParser::Scalar, TPCII::spuBREV }, "BREV"),
  std::make_pair(OpCodeKey{ SlotParser::Scalar, TPCII::spuHALT }, "HALT"),
  std::make_pair(OpCodeKey{ SlotParser::Scalar, TPCII::spuLOOP }, "LOOP"),
  std::make_pair(OpCodeKey{ SlotParser::Scalar, TPCII::spuJMPR }, "JMPR"),
  std::make_pair(OpCodeKey{ SlotParser::Scalar, TPCII::spuJMPA }, "JMPA"),
  std::make_pair(OpCodeKey{ SlotParser::Scalar, TPCII::spuMOV_IRF_DIM }, "MOV_IRF_DIM"),
  std::make_pair(OpCodeKey{ SlotParser::Scalar, TPCII::spuSET_INDX }, "SET_INDX"),
  std::make_pair(OpCodeKey{ SlotParser::Scalar, TPCII::spuUDIV_STEP }, "UDIV_STEP"),
  std::make_pair(OpCodeKey{ SlotParser::Scalar, TPCII::spuUDIV_4STEP }, "UDIV_4STEP"),
  std::make_pair(OpCodeKey{ SlotParser::Scalar, TPCII::spuUDIV }, "UDIV"),
  std::make_pair(OpCodeKey{ SlotParser::Scalar, TPCII::spuCALC_FP_SPECIAL }, "CALC_FP_SPECIAL"),
  std::make_pair(OpCodeKey{ SlotParser::Scalar, TPCII::spuCONVERT_INT8 }, "CONVERT_INT8"),
  std::make_pair(OpCodeKey{ SlotParser::Scalar, TPCII::spuCONVERT_UINT8 }, "CONVERT_UINT8"),
  std::make_pair(OpCodeKey{ SlotParser::Scalar, TPCII::spuTHREAD_SYNC }, "THREAD_SYNC"),
  std::make_pair(OpCodeKey{ SlotParser::Scalar, TPCII::spuDBG }, "DBG"),
  std::make_pair(OpCodeKey{ SlotParser::Scalar, TPCII::spuNOP }, "NOP"),

  std::make_pair(OpCodeKey{ SlotParser::Vector, TPCII::vpuMAC }, "MAC"),
  std::make_pair(OpCodeKey{ SlotParser::Vector, TPCII::vpuMUL }, "MUL"),
  std::make_pair(OpCodeKey{ SlotParser::Vector, TPCII::vpuADD }, "ADD"),
  std::make_pair(OpCodeKey{ SlotParser::Vector, TPCII::vpuSUB }, "SUB"),
  std::make_pair(OpCodeKey{ SlotParser::Vector, TPCII::vpuCONVERT_INT16 }, "CONVERT_INT16"),
  std::make_pair(OpCodeKey{ SlotParser::Vector, TPCII::vpuMAX }, "MAX"),
  std::make_pair(OpCodeKey{ SlotParser::Vector, TPCII::vpuMIN }, "MIN"),
  std::make_pair(OpCodeKey{ SlotParser::Vector, TPCII::vpuABS }, "ABS"),
  std::make_pair(OpCodeKey{ SlotParser::Vector, TPCII::vpuMOV }, "MOV"),
  std::make_pair(OpCodeKey{ SlotParser::Vector, TPCII::vpuCMP_EQ }, "CMP_EQ"),
  std::make_pair(OpCodeKey{ SlotParser::Vector, TPCII::vpuCMP_NEQ }, "CMP_NEQ"),
  std::make_pair(OpCodeKey{ SlotParser::Vector, TPCII::vpuCMP_LESS }, "CMP_LESS"),
  std::make_pair(OpCodeKey{ SlotParser::Vector, TPCII::vpuCMP_LEQ }, "CMP_LEQ"),
  std::make_pair(OpCodeKey{ SlotParser::Vector, TPCII::vpuCMP_GRT }, "CMP_GRT"),
  std::make_pair(OpCodeKey{ SlotParser::Vector, TPCII::vpuCMP_GEQ }, "CMP_GEQ"),
  std::make_pair(OpCodeKey{ SlotParser::Vector, TPCII::vpuOR }, "OR"),
  std::make_pair(OpCodeKey{ SlotParser::Vector, TPCII::vpuAND }, "AND"),
  std::make_pair(OpCodeKey{ SlotParser::Vector, TPCII::vpuXOR }, "XOR"),
  std::make_pair(OpCodeKey{ SlotParser::Vector, TPCII::vpuNOT }, "NOT"),
  std::make_pair(OpCodeKey{ SlotParser::Vector, TPCII::vpuSHR }, "SHR"),
  std::make_pair(OpCodeKey{ SlotParser::Vector, TPCII::vpuSHL }, "SHL"),
  std::make_pair(OpCodeKey{ SlotParser::Vector, TPCII::vpuASH }, "ASH"),
  std::make_pair(OpCodeKey{ SlotParser::Vector, TPCII::vpuCONVERT }, "CONVERT"),
  std::make_pair(OpCodeKey{ SlotParser::Vector, TPCII::vpuCONVERT_INT32 }, "CONVERT_INT32"),
  std::make_pair(OpCodeKey{ SlotParser::Vector, TPCII::vpuCONVERT_UINT32 }, "CONVERT_UINT32"),
  std::make_pair(OpCodeKey{ SlotParser::Vector, TPCII::vpuPOPCNT }, "POPCNT"),
  std::make_pair(OpCodeKey{ SlotParser::Vector, TPCII::vpuFIND_FIRST }, "FIND_FIRST"),
  std::make_pair(OpCodeKey{ SlotParser::Vector, TPCII::vpuNEARBYINT }, "NEARBYINT"),
  std::make_pair(OpCodeKey{ SlotParser::Vector, TPCII::vpuCONVERT_UINT16 }, "CONVERT_UINT16"),
  std::make_pair(OpCodeKey{ SlotParser::Vector, TPCII::vpuEXTRACT_EXP }, "EXTRACT_EXP"),
  std::make_pair(OpCodeKey{ SlotParser::Vector, TPCII::vpuFCLASS }, "FCLASS"),
  std::make_pair(OpCodeKey{ SlotParser::Vector, TPCII::vpuBREV }, "BREV"),
  std::make_pair(OpCodeKey{ SlotParser::Vector, TPCII::vpuHALT }, "HALT"),
  std::make_pair(OpCodeKey{ SlotParser::Vector, TPCII::vpuSEL_EQ }, "SEL_EQ"),
  std::make_pair(OpCodeKey{ SlotParser::Vector, TPCII::vpuSEL_NEQ }, "SEL_NEQ"),
  std::make_pair(OpCodeKey{ SlotParser::Vector, TPCII::vpuSEL_LESS }, "SEL_LESS"),
  std::make_pair(OpCodeKey{ SlotParser::Vector, TPCII::vpuSEL_LEQ }, "SEL_LEQ"),
  std::make_pair(OpCodeKey{ SlotParser::Vector, TPCII::vpuSEL_GRT }, "SEL_GRT"),
  std::make_pair(OpCodeKey{ SlotParser::Vector, TPCII::vpuSEL_GEQ }, "SEL_GEQ"),
  std::make_pair(OpCodeKey{ SlotParser::Vector, TPCII::vpuSEL2_LESS }, "SEL2_LESS"),
  std::make_pair(OpCodeKey{ SlotParser::Vector, TPCII::vpuSEL2_LEQ }, "SEL2_LEQ"),
  std::make_pair(OpCodeKey{ SlotParser::Vector, TPCII::vpuSEL2_GRT }, "SEL2_GRT"),
  std::make_pair(OpCodeKey{ SlotParser::Vector, TPCII::vpuSEL2_GEQ }, "SEL2_GEQ"),
  std::make_pair(OpCodeKey{ SlotParser::Vector, TPCII::vpuSHUFFLE }, "SHUFFLE"),
  std::make_pair(OpCodeKey{ SlotParser::Vector, TPCII::vpuPACK }, "PACK"),
  std::make_pair(OpCodeKey{ SlotParser::Vector, TPCII::vpuUNPACK }, "UNPACK"),
  std::make_pair(OpCodeKey{ SlotParser::Vector, TPCII::vpuGET_LUT_ENTRY_AND_INTERVAL_START }, "GET_LUT_ENTRY_AND_INTERVAL_START"),
  std::make_pair(OpCodeKey{ SlotParser::Vector, TPCII::vpuFORM_FP_NUM }, "FORM_FP_NUM"),
  std::make_pair(OpCodeKey{ SlotParser::Vector, TPCII::vpuMOV_DUAL_GROUP }, "MOV_DG"),
  std::make_pair(OpCodeKey{ SlotParser::Vector, TPCII::vpuMOV_GROUP }, "MOV_G"),
  std::make_pair(OpCodeKey{ SlotParser::Vector, TPCII::vpuMSAC }, "MSAC"),
  std::make_pair(OpCodeKey{ SlotParser::Vector, TPCII::vpuCALC_FP_SPECIAL }, "CALC_FP_SPECIAL"),
  std::make_pair(OpCodeKey{ SlotParser::Vector, TPCII::vpuCONVERT_INT8 }, "CONVERT_INT8"),
  std::make_pair(OpCodeKey{ SlotParser::Vector, TPCII::vpuCONVERT_UINT8 }, "CONVERT_UINT8"),
  std::make_pair(OpCodeKey{ SlotParser::Vector, TPCII::vpuCONVERT_FP_FLEX }, "CONVERT_FP_FLEX"),
  std::make_pair(OpCodeKey{ SlotParser::Vector, TPCII::vpuMADD }, "MADD"),
  std::make_pair(OpCodeKey{ SlotParser::Vector, TPCII::vpuDBG }, "NOP"),
  std::make_pair(OpCodeKey{ SlotParser::Vector, TPCII::vpuNOP }, "NOP"),

  std::make_pair(OpCodeKey{ SlotParser::Store, TPCII::stGEN_ADDR }, "GEN_ADDR"),
  std::make_pair(OpCodeKey{ SlotParser::Store, TPCII::stPRMT_INDX }, "PRMT_INDX"),
  std::make_pair(OpCodeKey{ SlotParser::Store, TPCII::stSET_INDX }, "SET_INDX"),
  std::make_pair(OpCodeKey{ SlotParser::Store, TPCII::ST_L }, "ST_L"),
  std::make_pair(OpCodeKey{ SlotParser::Store, TPCII::ST_G }, "ST_G"),
  std::make_pair(OpCodeKey{ SlotParser::Store, TPCII::ST_L_V }, "ST_L_V"),
  std::make_pair(OpCodeKey{ SlotParser::Store, TPCII::ST_L_V_LOW }, "ST_L_V_LOW"),
  std::make_pair(OpCodeKey{ SlotParser::Store, TPCII::ST_L_V_HIGH }, "ST_L_V_HIGH"),
  std::make_pair(OpCodeKey{ SlotParser::Store, TPCII::ASO }, "ASO"),
  std::make_pair(OpCodeKey{ SlotParser::Store, TPCII::ST_TNSR }, "ST_TNSR"),
  std::make_pair(OpCodeKey{ SlotParser::Store, TPCII::ST_TNSR_LOW }, "ST_TNSR_LOW"),
  std::make_pair(OpCodeKey{ SlotParser::Store, TPCII::ST_TNSR_HIGH }, "ST_TNSR_HIGH"),
  std::make_pair(OpCodeKey{ SlotParser::Store, TPCII::stLD_TNSR }, "LD_TNSR"),
  std::make_pair(OpCodeKey{ SlotParser::Store, TPCII::stLD_TNSR_LOW }, "LD_TNSR_LOW"),
  std::make_pair(OpCodeKey{ SlotParser::Store, TPCII::stLD_TNSR_HIGH }, "LD_TNSR_HIGH"),
  std::make_pair(OpCodeKey{ SlotParser::Store, TPCII::CACHE_FLUSH }, "CACHE_FLUSH"),
  std::make_pair(OpCodeKey{ SlotParser::Store, TPCII::CACHE_INVALIDATE }, "CACHE_INVALIDATE"),
  std::make_pair(OpCodeKey{ SlotParser::Store, TPCII::ST_TNSR_SQZ }, "ST_TNSR_SQZ"),
  std::make_pair(OpCodeKey{ SlotParser::Store, TPCII::ST_TNSR_S }, "ST_TNSR_S"),
  std::make_pair(OpCodeKey{ SlotParser::Store, TPCII::ST_EVENT }, "EVENT"),
  std::make_pair(OpCodeKey{ SlotParser::Store, TPCII::stLD_TNSR_CNVRT}, "LD_TNSR_CNVRT"),
  std::make_pair(OpCodeKey{ SlotParser::Store, TPCII::stNOP }, "NOP"),
};

#if defined(__GNUC__) || defined(__clang__)
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wmissing-field-initializers"
#endif

// Array of all instruction switches.
static const SwitchInfo AllSwitches[] = {
  // Instructions that do not have switch flags yet
  SwitchInfo { "",
               { { "cmp_neq",   S_Arith  },
                 { "cmp_less",  S_Arith  },
                 { "cmp_leq",   S_Arith  },
                 { "cmp_grt",   S_Arith  },
                 { "cmp_geq",   S_Arith  },
                 { "add",       S_Arith  },
                 { "sub",       S_Arith  },
                 { "abs",       S_Arith  },
                 { "sel_neq",   S_Vector },
                 { "sel_less",  S_Vector },
                 { "sel_leq",   S_Vector },
                 { "sel_grt",   S_Vector },
                 { "sel_geq",   S_Vector },
                 { "sel2_less", S_Vector },
                 { "sel2_leq",  S_Vector },
                 { "sel2_grt",  S_Vector },
                 { "sel2_geq",  S_Vector },
                 { "max",       S_Arith  },
                 { "min",       S_Arith  },
                 { "and",       S_Arith  },
                 { "or",        S_Arith  },
                 { "xor",       S_Arith  },
                 { "not",       S_Arith  },
                 { "shr",       S_Arith  },
                 { "shl",       S_Arith  },
                 { "brev",      S_Arith  },
                 { "shuffle",   S_Arith  },
                 { "prmt_indx", S_Load | S_Store },
                 { "mov",       S_Scalar | S_Load | S_Vector },
                 { "cache_flush", S_Store},
                 { "prefetch",  S_Load },
                 { "ld_l_v",    S_Load },
                 { "ld_l_v_low",  S_Load },
                 { "ld_l_v_high", S_Load },
                 { "udiv_step", S_Scalar },
                 { "udiv_4step", S_Scalar },
                 { "set_indx",  S_Load | S_Scalar | S_Store },
                 { "mov_irf_dim", S_Scalar },
                 { "gen_addr", S_Load | S_Store },
                 { "fclass", S_Load | S_Scalar | S_Vector },
                 { "st_l_v",      S_Store },
                 { "ld_g",         S_Load },
                 { "st_g",         S_Store},
                 { "ld_tnsr", S_Load | S_Store },
                 { "ld_tnsr_high", S_Load | S_Store},
                 { "ld_tnsr_low",  S_Load | S_Store},
                 { "st_tnsr", S_Store },
                 { "st_tnsr_low",  S_Store },
                 { "st_tnsr_high", S_Store }, 
                 { "st_l_v",      S_Store },
                 { "st_l_v_low",  S_Store },
                 { "st_l_v_high", S_Store }
               },
               0U, AllCores, T_ALL },
  SwitchInfo { "dimmask",
               { { "mul",     S_Scalar },
                 { "add",     S_Scalar },
                 { "sub",     S_Scalar },
                 { "max",     S_Scalar },
                 { "min",     S_Scalar },
                 { "abs",     S_Scalar },
                 { "not",     S_Scalar },
                 { "mov",     S_Scalar },
                 { "cmp_eq",  S_Scalar },
                 { "cmp_neq", S_Scalar },
                 { "cmp_less", S_Scalar },
                 { "cmp_leq", S_Scalar },
                 { "cmp_grt", S_Scalar },
                 { "cmp_geq", S_Scalar },
                 { "or",      S_Scalar },
                 { "and",     S_Scalar },
                 { "xor",     S_Scalar },
                 { "shr",     S_Scalar },
                 { "shl",     S_Scalar } },
               SW_DIMMASK, AllCores, T_INT32 }, //FIXME no Values ???
  SwitchInfo { "st",
               { { "mac", S_Arith },
                 { "madd", S_Vector },
                 { "add", S_Arith },
                 { "sub", S_Arith } },
               TPCII::SW_SAT, AllCores, T_INTEGER },
  SwitchInfo { "sat",
               { { "mac", S_Arith },
                 { "madd", S_Vector },
                 { "add", S_Arith },
                 { "sub", S_Arith } },
               TPCII::SW_SAT, AllCores, T_INTEGER, {}, 0,
               {}, OptAlias},
  SwitchInfo { "neg",
               { { "mac", S_Arith },
                 { "madd", S_Vector },
                 { "sub", S_Arith } },
               TPCII::SW_NEG, AllCores, T_ALL },
	SwitchInfo{"fp16_ftz_in",
               {{"mac", S_Arith},
                {"madd", S_Vector},
                {"mul", S_Arith},
                {"add", S_Arith},
                {"sub", S_Arith}},
               TPCII::SW_FP16_FTZ_IN,
               TPC::FeatureGaudiB,
               T_FP16},
  SwitchInfo { "no_borrow_gen",
               { { "sub", S_Arith } },
               TPCII::SW_NO_BORROW_GEN, TPC::FeatureGen4Plus, T_INT32 | T_UINT32 | T_INT16 | T_UINT16 },
  SwitchInfo { "borrow", { { "sub", S_Arith } },
               TPCII::SW_BORROW, TPC::FeatureGen4Plus, T_INT32 | T_UINT32 | T_INT16 | T_UINT16 },
  SwitchInfo { "acc_fp32",
               { { "mac", S_Arith },
                 { "madd", S_Vector },
                 { "mul", S_Arith } },
               TPCII::SW_ACC_FP32, TPC::FeatureBF16, T_BF16 },
  SwitchInfo { "acc_fp32",
               { { "mac", S_Arith },
                 { "madd", S_Vector },
                 { "mul", S_Arith } },
               TPCII::SW_ACC_FP32, TPC::FeatureFP16, T_FP16 },
  SwitchInfo { "acc_fp32",
               { { "mac", S_Arith }, {"mul", S_Arith}, {"madd", S_Arith} },
               TPCII::SW_ACC_FP32, TPC::FeatureFP8,T_F8},
  SwitchInfo { "acc_i16",
               { { "mac", S_Arith },
                 { "madd", S_Vector } },
               TPCII::SW_ACC_I16, TPC::FeatureAccI16, T_INT8 | T_UINT8 },
  SwitchInfo { "acc_i32",
               { { "mac", S_Arith },
                 { "madd", S_Vector } },
               TPCII::SW_ACC_I32, TPC::FeatureAccI32, T_UINT8 | T_UINT16 },
  SwitchInfo { "x2",
               { { "mac", S_Vector } },
               TPCII::SW_X2_ARITHMETIC, TPC::FeatureX2Mac, T_INT8 | T_UINT8 },
  SwitchInfo { "zp",
               { { "mac", S_Vector },
                 { "madd", S_Vector } },
               TPCII::SW_ZP, TPC::FeatureZP, T_INT8 | T_UINT8 },
  SwitchInfo { "neg_zp",
               { { "mac", S_Vector },
                 { "madd", S_Vector } },
               TPCII::SW_NEG_ZP, TPC::FeatureZP, T_INT8 | T_UINT8 },
  SwitchInfo { "double_and_round32", { { "mul", S_Vector } },
               TPCII::SW_DOUBLE_AND_ROUND32, TPC::FeatureGoya, T_INT32 | T_UINT32 },
  SwitchInfo { "double_and_round32", { { "mul", S_Vector } },
               TPCII::SW_DOUBLE_AND_ROUND32, TPC::FeatureGaudi, T_INT32 | T_UINT32 },
/*  SwitchInfo { "double_and_round32", { { "mul", S_Vector } },
               TPCII::SW_DOUBLE_AND_ROUND32, TPC::FeatureGaudiB, T_INT32 | T_UINT32 },*/
  SwitchInfo { "upper32", { { "mul", S_Scalar } },
               TPCII::SW_UPPER32, AllCores, T_INT32 | T_UINT32 },
  SwitchInfo { "rnd32", { { "mul", S_Vector } },
               TPCII::SW_GROUP_RND32, TPC::FeatureGen3Plus, T_INT32 | T_UINT32,
               { { "no_round",           SW_RND32_NO_ROUND        },
                 { "double_and_round32", SW_RND32_DNR32           },
                 { "keep_rs",            SW_RND32_KEEP_RS         },
                 { "keep_rs_for_add",    SW_RND32_KEEP_RS_FOR_ADD } } },
  SwitchInfo { "carry", { { "add", S_Arith } },
               TPCII::SW_CARRY, TPC::FeatureCarry, T_INTEGER },
  SwitchInfo { "no_carry_gen", { { "add", S_Arith } },
               TPCII::SW_NO_CARRY_GEN, TPC::FeatureGen3Plus, T_INTEGER },
  SwitchInfo { "mask_eq_zero", { { "cmp_eq", S_Arith },
                                 { "sel_eq", S_Vector } },
               TPCII::SW_MASK_EQ_ZERO, TPC::FeatureBF16, T_ALL },    // TODO: use more correct feature
  SwitchInfo { "high32", { { "cmp_eq", S_Arith },
                           { "cmp_neq", S_Arith },
                           { "cmp_less", S_Arith },
                           { "cmp_leq", S_Arith },
                           { "cmp_grt", S_Arith },
                           { "cmp_geq", S_Arith } },
               TPCII::SW_HIGH32, TPC::FeatureDoron1, T_INT32 | T_UINT32 },
  SwitchInfo { "rhu", { { "ash", S_Arith } },
               TPCII::SW_RHU, AllCores, T_INTEGER },
  SwitchInfo { "rhaz_rs", { { "ash", S_Vector } },
               TPCII::SW_RHAZ_RS, TPC::FeatureGen3Plus, T_INT32 | T_UINT32 },
  SwitchInfo { "subtract_bias", { { "extract_exp", S_Arith } },
               TPCII::SW_SUBTRACT_BIAS, TPC::FeatureGoya, T_FLOAT },
  SwitchInfo { "pre_log", { { "extract_exp", S_Arith } },
               TPCII::SW_PRE_LOG, TPC::FeatureGen4Plus, T_FP32 },
  SwitchInfo { "biased", { { "extract_exp", S_Arith } },
               TPCII::SW_SUBTRACT_BIAS, TPC::FeatureBF16, T_FLOAT }, // TODO: use more correct feature
 SwitchInfo { "exp_add_bias", { { "form_fp_num", S_Vector } },
               TPCII::SW_ADD_BIAS, AllCores, T_FLOAT },
  SwitchInfo { "force_sign0", { { "form_fp_num", S_Vector } },
               TPCII::SW_FORCE_SIGN0, AllCores, T_FLOAT },
  SwitchInfo { "force_sign1", { { "form_fp_num", S_Vector } },
               TPCII::SW_FORCE_SIGN1, AllCores, T_FLOAT },
  SwitchInfo { "exp_is_num", { { "form_fp_num", S_Vector } },
               TPCII::SW_EXP_IS_NUM, AllCores, T_FLOAT },
  SwitchInfo { "sign_lsb", { { "form_fp_num", S_Vector } },
               TPCII::SW_SIGN_LSB, AllCores, T_FLOAT },
  SwitchInfo { "special_func", 
               { { "form_fp_num", S_Vector } },
               TPCII::SW_SPECIAL_FUNC, TPC::FeatureGaudi2, T_FLOAT,
               { { "1", TPCII::SW_PRE_SQRT_RSQRT },
                 { "2", TPCII::SW_POST_SQRT      },
                 { "3", TPCII::SW_POST_RSQRT     },
                 { "4", TPCII::SW_POST_RECIP     } }, 0,
               {}, OptPrintAlways | OptPrintWithKey },
  SwitchInfo { "special_func", 
               { { "form_fp_num", S_Vector } },
               TPCII::SW_SPECIAL_FUNC, TPC::FeatureDoron1, T_FLOAT,
               { { "1", TPCII::SW_PRE_SQRT_RSQRT },
                 { "2", TPCII::SW_POST_SQRT      },
                 { "3", TPCII::SW_POST_RSQRT     },
                 { "4", TPCII::SW_POST_RECIP     },
                 { "5", TPCII::SW_PRE_LOG_FUNC   } }, 0,
               {}, OptPrintAlways | OptPrintWithKey },
  SwitchInfo { "auto_inc",
               { { "ld_g", S_Load } },
               TPCII::SW_AUTO_INC_G3, TPC::FeatureGreco, T_ALL,
               {}, 0, {}, OptDontPrint },
  SwitchInfo { "inc_val",
               { { "ld_g", S_Load } },
               TPCII::SW_INC_VAL_G3, TPC::FeatureGreco, T_ALL,
               { { "inc_1", SW_INC_1_G3 },
                 { "inc_2", SW_INC_2_G3 },
                 { "inc_4", SW_INC_4_G3 },
                 { "inc_8", SW_INC_8_G3 },
                 { "1",     SW_INC_1_G3 },
                 { "2",     SW_INC_2_G3 },
                 { "4",     SW_INC_4_G3 },
                 { "8",     SW_INC_8_G3 } }, SW_INC_1_G3,
                 { { "auto_inc", TPCII::SW_AUTO_INC_G3 } }, OptPrintAlways },
  SwitchInfo { "inc_val",
               { { "ld_g", S_Load  },
                 { "st_g", S_Store } },
               TPCII::SW_INC_VAL, TPC::FeatureGen4Plus, T_ALL,
               { { "inc_0", SW_INC_0 },
                 { "inc_1", SW_INC_1 },
                 { "inc_2", SW_INC_2 },
                 { "inc_4", SW_INC_4 },
                 { "inc_8", SW_INC_8 } }, SW_INC_0 },
  SwitchInfo { "sup_nan",
               { { "min", S_Scalar | S_Vector },
                 { "max", S_Scalar | S_Vector } },
               TPCII::SW_SUP_NAN, TPC::FeatureDoron1, T_FLOAT },
  SwitchInfo { "sup_nan",
               { { "sel_less",  S_Vector },
                 { "sel_leq",   S_Vector },
                 { "sel_grt",   S_Vector },
                 { "sel_geq",   S_Vector },
                 { "sel2_less", S_Vector },
                 { "sel2_leq",  S_Vector },
                 { "sel2_grt",  S_Vector },
                 { "sel2_geq",  S_Vector } },
               TPCII::SW_SUP_NAN, TPC::FeatureDoron1, T_FLOAT },
  SwitchInfo { "pd",
               { { "ld_g", S_Load  },
                 { "st_g", S_Store } },
               TPCII::SW_PD, TPC::FeatureGen4Plus, T_ALL },
  SwitchInfo { "bv64", { { "ld_g", S_Load } },
               TPCII::SW_BV64, TPC::FeatureGen3Plus, T_ALL },
  SwitchInfo { "bv64",
               { { "st_g", S_Store } },
               TPCII::SW_BV64, TPC::FeatureDoron1, T_ALL },
  SwitchInfo { "bv64", { { "st_tnsr_s", S_Store } },
               TPCII::SW_ST_TNSR_S_BV64, TPC::FeatureGen4Plus, T_ALL },
  SwitchInfo { "ev_hint",
               { { "ld_g", S_Load } },
                TPCII::SW_EV_HINT, TPC::FeatureGen3Plus, T_ALL },
  SwitchInfo { "ev_hint",
               { { "st_g", S_Store } },
                TPCII::SW_EV_HINT, TPC::FeatureGen4Plus, T_ALL },
  SwitchInfo { "rhu",
               { {"msac", S_Vector} },
               TPCII::SW_RHU, TPC::FeatureGoya, T_ALL},
  SwitchInfo { "normsel",
               { {"msac", S_Vector} },
               TPCII::SW_NORMALIZE, TPC::FeatureGoya, T_ALL,
               { { "normalize_ab", SW_NORMALIZE_AB },
                 { "normalize_c",  SW_NORMALIZE_C  }, }, 0,
               {}, OptPrintAlways },
  SwitchInfo { "rhu",
               { {"msac", S_Vector} },
               TPCII::SW_RHU, TPC::FeatureGen3Plus, T_ALL },
  SwitchInfo { "normsel",
               { { "msac", S_Vector} },
               TPCII::SW_NORMALIZE, TPC::FeatureGen3Plus, T_ALL,
               { { "normalize_ab", SW_NORMALIZE_AB },
                 { "normalize_c",  SW_NORMALIZE_C  } }, 0,
               {}, OptPrintAlways },
  SwitchInfo { "g_en",
               { { "mov_g", S_Vector },
                 { "mov_group", S_Vector } },
               TPCII::SW_GROUP_EN, AllCores, T_ALL,
               { { "0", 0 },
                 { "1", 1 },
                 { "2", 2 },
                 { "3", 3 } }, IsImmMask,
               {}, OptPrintAlways | OptPrintWithKey | OptPrintAsBinary },
  SwitchInfo { "group_en",
               { { "mov_g", S_Vector },
                 { "mov_group", S_Vector } },
               TPCII::SW_GROUP_EN, AllCores, T_ALL,
               { { "0", 0 },
                 { "1", 1 },
                 { "2", 2 },
                 { "3", 3 } }, IsImmMask,
               {}, OptPrintWithKey | OptAlias | OptPrintAsBinary },
  SwitchInfo { "dg_en",
               { { "mov_g", S_Vector },
                 { "mov_group", S_Vector } },
               TPCII::SW_DUAL_GROUP_EN, AllCores, T_ALL,
               { { "0",  (0 << 2)  },
                 { "1",  (1 << 2)  },
                 { "2",  (2 << 2)  },
                 { "3",  (3 << 2)  },
                 { "4",  (4 << 2)  },
                 { "5",  (5 << 2)  },
                 { "6",  (6 << 2)  },
                 { "7",  (7 << 2)  },
                 { "8",  (8 << 2)  },
                 { "9",  (9 << 2)  },
                 { "10", (10 << 2) },
                 { "11", (11 << 2) },
                 { "12", (12 << 2) },
                 { "13", (13 << 2) },
                 { "14", (14 << 2) },
                 { "15", (15 << 2) } }, IsImmMask,
                 {}, OptPrintAlways | OptPrintWithKey | OptPrintAsBinary },
  SwitchInfo { "dual_group_en",
               { { "mov_g", S_Vector },
                 { "mov_group", S_Vector } },
               TPCII::SW_DUAL_GROUP_EN, AllCores, T_ALL,
               { { "0",  (0 << 2)  },
                 { "1",  (1 << 2)  },
                 { "2",  (2 << 2)  },
                 { "3",  (3 << 2)  },
                 { "4",  (4 << 2)  },
                 { "5",  (5 << 2)  },
                 { "6",  (6 << 2)  },
                 { "7",  (7 << 2)  },
                 { "8",  (8 << 2)  },
                 { "9",  (9 << 2)  },
                 { "10", (10 << 2) },
                 { "11", (11 << 2) },
                 { "12", (12 << 2) },
                 { "13", (13 << 2) },
                 { "14", (14 << 2) },
                 { "15", (15 << 2) } }, IsImmMask,
               {}, OptPrintWithKey | OptAlias | OptPrintAsBinary },
  SwitchInfo { "sbcd",
               { { "lookup",    S_Load },
                 { "lookup_1c", S_Load },
                 { "lookup_2c", S_Load } },
               TPCII::SW_SBCD, TPC::FeatureGen4Plus, T_ALL },
  SwitchInfo { "upper_half",
               { { "lookup",    S_Load },
                 { "lookup_1c", S_Load },
                 { "lookup_2c", S_Load } },
               TPCII::SW_UPPER_HALF, TPC::FeatureBF16, T_ALL },
  SwitchInfo { "lut_ptr",
               { { "lookup",    S_Load },
                 { "lookup_1c", S_Load },
                 { "lookup_2c", S_Load } },
               TPCII::SW_LUT_PTR, TPC::FeatureGen3Plus, T_ALL },
  SwitchInfo { "lkp_part",
               { { "lookup_1c", S_Load },
                 { "lookup_2c", S_Load } },
               TPCII::SW_LKP_PART2, TPC::FeatureDoron1, T_FLOAT | T_INTEGER,
               { {"part1", TPCII::SW_LKP_PART1}, {"part2", TPCII::SW_LKP_PART2} }, 0, 
               { { "x2", 0 }}, OptPrintAlways},
  SwitchInfo { "dt",
               { { "lookup",      S_Load },
                 { "lookup_c0",   S_Load },
                 { "lookup_c1c2", S_Load } },
               TPCII::SW_LOOKUP_G1, TPC::FeatureGoya, T_ALL,
               { { "BV32",         SW_BV32 },
                 { "BV16_LOW",     SW_BV16_LOW  },
                 { "BV16_HIGH",    SW_BV16_HIGH },
                 { "BV8_0",        SW_BV8_0 },
                 { "BV8_1",        SW_BV8_1 },
                 { "BV8_2",        SW_BV8_2 },
                 { "BV8_3",        SW_BV8_3 },
                 // Aliases
                 { "F32",          SW_BV32 },
                 { "F16_LOW",      SW_BV16_LOW },
                 { "F16_HIGH",     SW_BV16_HIGH },
                 { "I16_LOW",      SW_BV16_LOW },
                 { "I16_HIGH",     SW_BV16_HIGH },
                 { "BV8_ELEMENT_0", SW_BV8_0 },
                 { "BV8_ELEMENT_1", SW_BV8_1 },
                 { "BV8_ELEMENT_2", SW_BV8_2 },
                 { "BV8_ELEMENT_3", SW_BV8_3 },
                 { "I8_ELEMENT_0", SW_BV8_0 },
                 { "I8_ELEMENT_1", SW_BV8_1 },
                 { "I8_ELEMENT_2", SW_BV8_2 },
                 { "I8_ELEMENT_3", SW_BV8_3 }, }, 0,
               {}, OptPrintAlways },
  SwitchInfo { "dt",
               { { "lookup", S_Load } },
               TPCII::SW_LOOKUP_G2, TPC::FeatureBF16, T_ALL,   // TODO: use more correct feature
               { { "BV32",   SW_BV32 } }, 0,
               {}, OptPrintAlways },
  SwitchInfo { "dt",
               { { "lookup_1c",   S_Load },
                 { "lookup_2c",   S_Load } },
               TPCII::SW_LOOKUP_G2, TPC::FeatureGaudi, T_ALL,
               { { "BV32",    SW_BV32 },
                 { "BV16",    SW_BV16 } }, 0,
               {}, OptPrintAlways },
  SwitchInfo { "dt",
               { { "lookup_1c",   S_Load },
                 { "lookup_2c",   S_Load } },
               TPCII::SW_LOOKUP_G2, TPC::FeatureGaudiB, T_ALL,
               { { "BV32",    SW_BV32 },
                 { "BV16",    SW_BV16 } }, 0,
               {}, OptPrintAlways },
  SwitchInfo { "dt",
               { { "lookup_1c",   S_Load },
                 { "lookup_2c",   S_Load } },
               TPCII::SW_LOOKUP_G3, TPC::FeatureGen3Plus, T_ALL,
               { { "BV32",   SW_BV32     },
                 { "BV16",   SW_BV16     },
                 { "BV8_0",  SW_BV8_0_G3 },
                 { "BV8_1",  SW_BV8_1_G3 } }, 0,
               {}, OptPrintAlways },
  SwitchInfo { "x2",
               { { "lookup_1c",   S_Load },
                 { "lookup_2c",   S_Load } },
               TPCII::SW_X2_ARITHMETIC, TPC::FeatureGen4Plus, T_ALL,
               {}, 0},
  SwitchInfo { "rm",
               { { "convert",   S_Arith },
                 { "nearbyint", S_Arith } },
               TPCII::SW_GROUP_RM, TPC::FeatureGoya, T_ALL,
               { { "rhne",    SW_G1_RHNE },
                 { "rne",     SW_G1_RHNE },
                 { "rd",      SW_G1_RD   },
                 { "ru",      SW_G1_RU   },
                 { "rz",      SW_G1_RZ   },
                 { "default", SW_CSR     } }, SW_CSR },
  SwitchInfo { "rm",
               { { "nearbyint", S_Arith } },
               TPCII::SW_GROUP_RM, TPC::FeatureBF16, T_ALL,   // TODO: use more correct feature
               { { "rhne",    SW_RHNE    },
                 { "rne",     SW_RHNE    },
                 { "rz",      SW_RZ      },
                 { "ru",      SW_RU      },
                 { "rd",      SW_RD      },
                 { "sr",      SW_SR      },
                 { "default", SW_CSR     },
                 { "rhaz",    SW_RHAZ    } }, SW_CSR },
  SwitchInfo { "rm",
               { { "convert",   S_Arith } },
               TPCII::SW_GROUP_RM, TPC::FeatureGaudi, T_ALL,
               { { "rhne",    SW_RHNE    },
                 { "rne",     SW_RHNE    },
                 { "rz",      SW_RZ      },
                 { "ru",      SW_RU      },
                 { "rd",      SW_RD      },
                 { "sr",      SW_SR      },
                 { "default", SW_CSR     },
                 { "rhaz",    SW_RHAZ    } }, SW_CSR },
  SwitchInfo { "rm",
               { { "convert",   S_Arith } },
               TPCII::SW_GROUP_RM, TPC::FeatureGaudiB, T_ALL,
               { { "rhne",    SW_RHNE    },
                 { "rne",     SW_RHNE    },
                 { "rz",      SW_RZ      },
                 { "ru",      SW_RU      },
                 { "rd",      SW_RD      },
                 { "sr",      SW_SR      },
                 { "default", SW_CSR     },
                 { "rhaz",    SW_RHAZ    } }, SW_CSR },
  SwitchInfo { "rm",
               { { "convert",   S_Arith } },
               TPCII::SW_GROUP_RM, TPC::FeatureGreco, T_ALL,
               { { "rhne",    SW_RHNE    },
                 { "rne",     SW_RHNE    },
                 { "rz",      SW_RZ      },
                 { "ru",      SW_RU      },
                 { "rd",      SW_RD      },
                 { "sr",      SW_SR      },
                 { "default", SW_CSR     },
                 { "rhaz",    SW_RHAZ    } }, SW_CSR },
  SwitchInfo { "rm",
               { { "convert",   S_Arith } },
               TPCII::SW_GROUP_RM, TPC::FeatureGen4Plus, T_ALL,
               { { "rhne",    SW_RHNE    },
                 { "rne",     SW_RHNE    },
                 { "rz",      SW_RZ      },
                 { "ru",      SW_RU      },
                 { "rd",      SW_RD      },
                 { "sr",      SW_SR      },
                 { "default", SW_CSR     },
                 { "rhaz",    SW_RHAZ    },
                 { "sr_rne",  SW_SR_RNE  },
                 { "sr_rhne", SW_SR_RNE  } }, SW_CSR },
  SwitchInfo { "clip_fp",
               { {"convert", S_Arith},
                 {"ld_tnsr_cnvrt", S_Load | S_Store}},
               TPCII::SW_CLIP_FP, TPC::FeatureGen4Plus, T_ALL,
               {}, 0 },
  SwitchInfo { "clip_fp",
               { { "convert_fp_flex", S_Arith } },
               TPCII::SW_CLIP_FP, TPC::FeatureConvertFpFlex, T_ALL,
               {}, 0 },
  SwitchInfo { "rm",
               { { "convert_int32",  S_Arith },
                 { "convert_uint32", S_Arith },
                 { "convert_int16",  S_Arith },
                 { "convert_uint16", S_Arith } },
               TPCII::SW_GROUP_RM, TPC::FeatureGoya, T_ALL,
               { { "rhne", SW_G1_RHNE },
                 { "rne",  SW_G1_RHNE },
                 { "rd",   SW_G1_RD   },
                 { "ru",   SW_G1_RU   },
                 { "sr",   SW_G1_SR   } }, SW_G1_RHNE,
               {}, OptPrintAlways },
  SwitchInfo { "rm",
               { { "convert_int32",  S_Arith },
                 { "convert_uint32", S_Arith },
                 { "convert_int16",  S_Arith },
                 { "convert_uint16", S_Arith },
                 { "convert_int8",   S_Arith },
                 { "convert_uint8",  S_Arith } },
               TPCII::SW_GROUP_RM, TPC::FeatureBF16, T_ALL,   // TODO: use more correct feature
               { { "rhne", SW_RHNE },
                 { "rne", SW_RHNE },
                 { "rz", SW_RZ },
                 { "ru", SW_RU },
                 { "rd", SW_RD },
              { "sr", SW_SR } }, SW_RHNE,
              {}, OptPrintAlways },
  SwitchInfo { "rm",
               { { "convert_fp_flex", S_Arith } },
               TPCII::SW_GROUP_RM, TPC::FeatureConvertFpFlex, T_ALL,
               { { "rhne",    SW_RHNE    },
                 { "rne",     SW_RHNE    },
                 { "rz",      SW_RZ      },
                 { "ru",      SW_RU      },
                 { "rd",      SW_RD      },
                 { "sr",      SW_SR      },
                 { "default", SW_CSR     },
                 { "rhaz",    SW_RHAZ    },
                 { "sr_rne",  SW_SR_RNE }}, SW_CSR,
               {}, OptPrintAlways },
  SwitchInfo { "to",
               { { "convert_int32",  S_Arith },
                 { "convert_uint32", S_Arith } },
               TPCII::SW_GROUP_TO, AllCores, T_ALL,
               { { "to_8",  SW_TO_8  },
                 { "to_16", SW_TO_16 } }, 0,
               {}, OptPrintAlways },
  SwitchInfo { "target_type",
               { { "convert", S_Arith }, },
               TPCII::SW_TO_TYPE, AllCores, T_ALL,
               { { "fp32",   SW_TO_FP32   },
                 { "bf16",   SW_TO_BF16   },
                 { "int32",  SW_TO_INT32  },
                 { "uint32", SW_TO_UINT32 },
                 { "int8",   SW_TO_INT8   },
                 { "uint8",  SW_TO_UINT8  },
                 { "int16",  SW_TO_INT16  },
                 { "uint16", SW_TO_UINT16 },
                 { "f16",    SW_TO_FP16   }, 
                 { "f8_152", SW_TO_FP8_152},
                 { "f8_143", SW_TO_FP8_143} }, 0,
               {}, OptPrintAlways | OptPrintWithKey },
  SwitchInfo { "lane_sel",
               { { "convert",        S_Vector } },
               TPCII::SW_LANE_SEL, TPC::FeatureGoya, T_ALL,
               { { "0", TPCII::SW_LANE_0 },
                 { "1", TPCII::SW_LANE_1 },
                 { "2", TPCII::SW_LANE_2 },
                 { "3", TPCII::SW_LANE_3 } }, TPCII::SW_LANE_0,
               {}, OptPrintWithKey },
  SwitchInfo { "lane_sel",
               { { "convert",        S_Vector } },
               TPCII::SW_LANE_SEL, TPC::FeatureGaudi, T_ALL,
               { { "0", TPCII::SW_LANE_0 },
                 { "1", TPCII::SW_LANE_1 },
                 { "2", TPCII::SW_LANE_2 },
                 { "3", TPCII::SW_LANE_3 } }, TPCII::SW_LANE_0,
               {}, OptPrintWithKey },
  SwitchInfo { "lane_sel",
               { { "convert",        S_Vector } },
               TPCII::SW_LANE_SEL, TPC::FeatureGaudiB, T_ALL,
               { { "0", TPCII::SW_LANE_0 },
                 { "1", TPCII::SW_LANE_1 },
                 { "2", TPCII::SW_LANE_2 },
                 { "3", TPCII::SW_LANE_3 } }, TPCII::SW_LANE_0,
               { {"num_lanes", TPCII::SW_SINGLE_LANE_SRCB} }, OptPrintWithKey },
  SwitchInfo { "lane_sel",
               { { "convert",        S_Vector } },
               TPCII::SW_LANE_SEL, TPC::FeatureGen3Plus, T_ALL,
               { { "0", TPCII::SW_LANE_0 },
                 { "1", TPCII::SW_LANE_1 },
                 { "2", TPCII::SW_LANE_2 },
                 { "3", TPCII::SW_LANE_3 } }, TPCII::SW_LANE_0,
               { {"num_lanes", TPCII::SW_SINGLE_LANE_SRCB} }, OptPrintWithKey },
  SwitchInfo { "lane_sel",
               { { "convert_int32",  S_Vector },
                 { "convert_uint32", S_Vector },
                 { "convert_int16",  S_Vector },
                 { "convert_uint16", S_Vector } },
               TPCII::SW_LANE_SEL, TPC::FeatureGoya, T_ALL,
               { { "0", TPCII::SW_LANE_0 },
                 { "1", TPCII::SW_LANE_1 },
                 { "2", TPCII::SW_LANE_2 },
                 { "3", TPCII::SW_LANE_3 } }, TPCII::SW_LANE_0,
               { {"num_lanes", TPCII::SW_SINGLE_LANE} }, OptPrintAlways | OptPrintWithKey },
  SwitchInfo { "lane_sel",
               { { "convert_int32",  S_Vector },
                 { "convert_uint32", S_Vector },
                 { "convert_int16",  S_Vector },
                 { "convert_uint16", S_Vector } },
               TPCII::SW_LANE_SEL, TPC::FeatureGaudi, T_ALL,
               { { "0", TPCII::SW_LANE_0 },
                 { "1", TPCII::SW_LANE_1 },
                 { "2", TPCII::SW_LANE_2 },
                 { "3", TPCII::SW_LANE_3 } }, TPCII::SW_LANE_0,
               {}, OptPrintAlways | OptPrintWithKey },
  SwitchInfo { "lane_sel",
               { { "convert_int32",  S_Vector },
                 { "convert_uint32", S_Vector },
                 { "convert_int16",  S_Vector },
                 { "convert_uint16", S_Vector } },
               TPCII::SW_LANE_SEL, TPC::FeatureGaudiB, T_ALL,
               { { "0", TPCII::SW_LANE_0 },
                 { "1", TPCII::SW_LANE_1 },
                 { "2", TPCII::SW_LANE_2 },
                 { "3", TPCII::SW_LANE_3 } }, TPCII::SW_LANE_0,
               {}, OptPrintAlways | OptPrintWithKey },
  SwitchInfo { "lane_sel",
               { { "convert_int32",  S_Vector },
                 { "convert_uint32", S_Vector },
                 { "convert_int16",  S_Vector },
                 { "convert_uint16", S_Vector } },
               TPCII::SW_LANE_SEL, TPC::FeatureGen3Plus, T_ALL,
               { { "0", TPCII::SW_LANE_0 },
                 { "1", TPCII::SW_LANE_1 },
                 { "2", TPCII::SW_LANE_2 },
                 { "3", TPCII::SW_LANE_3 } }, TPCII::SW_LANE_0,
               { {"num_lanes", TPCII::SW_SINGLE_LANE} }, OptPrintAlways | OptPrintWithKey },
  SwitchInfo { "lane_sel",
               { {"convert_fp_flex", S_Vector} },
	               TPCII::SW_LANE_1, TPC::FeatureConvertFpFlex, T_ALL,
               { { "0", TPCII::SW_LANE_0 },
                 { "1", TPCII::SW_LANE_1 } }, 0,
               {}, OptPrintAlways | OptPrintWithKey },
  SwitchInfo { "num_lanes",
                   { { "convert", S_Vector } },
                   TPCII::SW_NUM_LANES_SRCB_G5, TPC::FeatureDoron1, T_ALL,
               { { "all_lanes",   SW_ALL_LANES_SRCB },
                 { "single_lane", SW_SINGLE_LANE_SRCB}, 
                 { "lower_lanes", SW_LOWER_LANES_SRCB},
                 { "upper_lanes", SW_UPPER_LANES_SRCB} }, SW_SINGLE_LANE_SRCB,
                 { {"x4", 0}/*, {"x2", 0}*/ } },
  SwitchInfo { "num_lanes",
               { { "convert", S_Vector } },
               TPCII::SW_NUM_LANES_SRCB, TPC::FeatureGreco, T_ALL,
               { { "all_lanes",   SW_ALL_LANES_SRCB },
                 { "single_lane", SW_SINGLE_LANE_SRCB } }, SW_SINGLE_LANE_SRCB,
                 { {"x4", 0}, {"x2", 0} } },
  SwitchInfo { "num_lanes",
               { { "convert", S_Vector } },
               TPCII::SW_NUM_LANES_SRCB, TPC::FeatureGaudi2, T_ALL,
               { { "all_lanes",   SW_ALL_LANES_SRCB },
                 { "single_lane", SW_SINGLE_LANE_SRCB } }, SW_SINGLE_LANE_SRCB,
                 { {"x4", 0}, {"x2", 0} } },
	SwitchInfo { "num_lanes",
               { { "convert", S_Vector } },
                 TPCII::SW_NUM_LANES_SRCB, TPC::FeatureGaudi, T_FP32,
                 { { "all_lanes",   SW_ALL_LANES_SRCB },
                 { "single_lane", SW_SINGLE_LANE_SRCB } }, SW_SINGLE_LANE_SRCB,
               { {"TARGET_TYPE", SW_TO_BF16} } },
// FIXME: Not supported due to HW bugs, but added for Gaudi compatibility ==>>
  SwitchInfo { "num_lanes",
               { { "convert", S_Vector } },
                 TPCII::SW_NUM_LANES_SRCB, TPC::FeatureGaudiB, T_FP32,
                 { { "all_lanes",   SW_ALL_LANES_SRCB },
                 { "single_lane", SW_SINGLE_LANE_SRCB } }, SW_SINGLE_LANE_SRCB,
               { {"TARGET_TYPE", SW_TO_BF16} } },
  SwitchInfo { "num_lanes",
               { { "convert", S_Vector } },
               TPCII::SW_NUM_LANES_SRCB, TPC::FeatureGaudiB, T_FP32,
               { { "all_lanes",   SW_ALL_LANES_SRCB },
                 { "single_lane", SW_SINGLE_LANE_SRCB } }, SW_SINGLE_LANE_SRCB,
               { {"TARGET_TYPE", SW_TO_FP16} } },
// <<== FIXME
  SwitchInfo { "num_lanes",
               { { "convert", S_Vector } },
                 TPCII::SW_NUM_LANES_SRCB, TPC::FeatureGaudiB, T_BF16 | T_FP16,
                 { { "all_lanes",   SW_ALL_LANES_SRCB } }, SW_ALL_LANES_SRCB,
               { {"TARGET_TYPE", SW_TO_FP32} }, OptPrintAlways },
  SwitchInfo { "num_lanes",
               { { "convert", S_Vector } },
                 TPCII::SW_NUM_LANES_SRCB, TPC::FeatureGaudi, T_BF16,
                 { { "all_lanes",   SW_ALL_LANES_SRCB } }, SW_ALL_LANES_SRCB,
               { {"TARGET_TYPE", SW_TO_FP32} }, OptPrintAlways },

  SwitchInfo { "num_lanes",
               { { "convert_int32",  S_Vector },
                 { "convert_uint32", S_Vector },
                 { "convert_int16",  S_Vector },
                 { "convert_uint16", S_Vector } },
               TPCII::SW_NUM_LANES, TPC::FeatureGreco, T_ALL,
               { { "all_lanes",   SW_ALL_LANES },
                 { "single_lane", SW_SINGLE_LANE } }, SW_SINGLE_LANE,
               {}, OptPrintAlways },
  SwitchInfo { "all_lanes",
               { { "convert_int8",  S_Vector },
                 { "convert_uint8", S_Vector } },
               TPCII::SW_ALL_LANES, TPC::FeatureGen3Plus, T_ALL },
  SwitchInfo { "num_lanes",
               { { "convert_int16",  S_Vector },
                 { "convert_uint16", S_Vector } },
               TPCII::SW_NUM_LANES, TPC::FeatureDoron1, T_ALL,
               { { "single_lane", SW_SINGLE_LANE } }, SW_SINGLE_LANE,
               {}, OptDontPrint },
SwitchInfo { "num_lanes",
               { { "convert_int32",  S_Vector },
                 { "convert_uint32", S_Vector } },
               TPCII::SW_NUM_LANES_G5, TPC::FeatureDoron1, T_ALL,
               { { "all_lanes",   SW_ALL_LANES },
                 { "single_lane", SW_SINGLE_LANE},
                 { "lower_lanes", SW_LOWER_LANES},
                 { "upper_lanes", SW_UPPER_LANES} }, SW_SINGLE_LANE },
  SwitchInfo { "num_lanes",
               { { "convert_int32",  S_Vector },
                 { "convert_uint32", S_Vector },
                 { "convert_int16",  S_Vector },
                 { "convert_uint16", S_Vector } },
               TPCII::SW_NUM_LANES, TPC::FeatureGaudi2, T_ALL,
               { { "all_lanes",   SW_ALL_LANES },
                 { "single_lane", SW_SINGLE_LANE } }, SW_SINGLE_LANE,
               {}, OptPrintAlways },
  SwitchInfo { "all_lanes",
               { { "convert_int8",  S_Vector },
                 { "convert_uint8", S_Vector } },
               TPCII::SW_ALL_LANES, TPC::FeatureGaudi2, T_ALL },

  SwitchInfo { "x2",
               { { "convert", S_Vector} },
               TPCII::SW_X2_CONVERT, TPC::FeatureGen3Plus, T_INT32,
               {}, 0,
               { {"TARGET_TYPE", SW_TO_FP32}, {"x4", 0} } },
  SwitchInfo { "x2",
               { { "convert", S_Vector} },
               TPCII::SW_X2_CONVERT, TPC::FeatureGen4Plus, T_UINT32,
               {}, 0,
               { {"TARGET_TYPE", SW_TO_FP32}, {"x4", 0} } },
  SwitchInfo { "x4",
               { { "convert", S_Vector} },
               TPCII::SW_X4_CONVERT, TPC::FeatureGen3Plus, T_INT32,
               {}, 0,
               { {"TARGET_TYPE", SW_TO_FP32}, {"x2", 0} } },
  SwitchInfo { "x4",
               { { "convert", S_Vector} },
               TPCII::SW_X4_CONVERT, TPC::FeatureGen4Plus, T_UINT32,
               {}, 0,
               { {"TARGET_TYPE", SW_TO_FP32}, {"x2", 0} } },
  SwitchInfo { "slot",
               { { "event", S_Load},
                 { "event", S_Store} },
               TPCII::SW_EVENT_SLOT, TPC::FeatureGen4Plus, T_INT16,
               { { "spu", 0 },
                 { "vpu", 1 } }, 0,
              {}, OptPrintAlways},
  SwitchInfo { "src",
               { { "mov_dg", S_Vector},
                 { "mov_dual_group", S_Vector} },
               TPCII::SW_SRC_DUAL_GROUP, AllCores, T_ALL,
               { { "0", 0 },
                 { "1", 1 << SW_SRC_DUAL_GROUP_SHIFT },
                 { "2", 2 << SW_SRC_DUAL_GROUP_SHIFT },
                 { "3", 3 << SW_SRC_DUAL_GROUP_SHIFT } }, 0,
              { { "all", 0 }, { "ctrl_reg", 0 }, { "pack", 0 }, { "mdg_type", SW_MDG_TYPE_SINGLE } }, OptPrintAlways | OptPrintWithKey },
  SwitchInfo{ "src_dual_group",
               { { "mov_dg", S_Vector },
                 { "mov_dual_group", S_Vector } },
               TPCII::SW_SRC_DUAL_GROUP, AllCores, T_ALL,
               { { "0", 0 },
                 { "1", 1 << SW_SRC_DUAL_GROUP_SHIFT },
                 { "2", 2 << SW_SRC_DUAL_GROUP_SHIFT },
                 { "3", 3 << SW_SRC_DUAL_GROUP_SHIFT }}, 0,
               { { "all", 0 }, { "ctrl_reg", 0 }, { "pack", 0 }, { "mdg_type", SW_MDG_TYPE_SINGLE } }, OptPrintWithKey | OptAlias },
  SwitchInfo{ "dst",
               { { "mov_dg", S_Vector },
                 { "mov_dual_group", S_Vector } },
               TPCII::SW_DST_DUAL_GROUP, AllCores, T_ALL,
               { { "0", 0 },
                 { "1", 1 << SW_DST_DUAL_GROUP_SHIFT },
                 { "2", 2 << SW_DST_DUAL_GROUP_SHIFT },
                 { "3", 3 << SW_DST_DUAL_GROUP_SHIFT } }, 0,
               { { "all", 0 }, { "ctrl_reg", 0 }, { "pack", 0 }, { "mdg_type", SW_MDG_TYPE_SINGLE } }, OptPrintAlways | OptPrintWithKey },
  SwitchInfo{ "dst_dual_group",
               { { "mov_dg", S_Vector },
                 { "mov_dual_group", S_Vector } },
               TPCII::SW_DST_DUAL_GROUP, AllCores, T_ALL,
               { { "0", 0 },
                 { "1", 1 << SW_DST_DUAL_GROUP_SHIFT },
                 { "2", 2 << SW_DST_DUAL_GROUP_SHIFT },
                 { "3", 3 << SW_DST_DUAL_GROUP_SHIFT } }, 0,
               { { "all", 0 }, { "ctrl_reg", 0 }, { "pack", 0 }, { "mdg_type", SW_MDG_TYPE_SINGLE } }, OptPrintWithKey | OptAlias },
  SwitchInfo { "wr_lg",
               { { "mov_dg", S_Vector },
                 { "mov_dual_group", S_Vector } },
               TPCII::SW_WR_LOWER_GROUP, AllCores, T_ALL,
               { { "0", 0 },
                 { "1", TPCII::SW_WR_LOWER_GROUP } }, IsImmMask,
               { { "all", 0 }, { "ctrl_reg", 0 }, { "pack", 0 }, { "mdg_type", SW_MDG_TYPE_SINGLE } }, OptPrintAlways | OptPrintWithKey },
  SwitchInfo { "wr_lower_group",
               { { "mov_dg", S_Vector },
                 { "mov_dual_group", S_Vector } },
               TPCII::SW_WR_LOWER_GROUP, AllCores, T_ALL,
               { { "0", 0 },
                 { "1", TPCII::SW_WR_LOWER_GROUP } }, IsImmMask,
               { { "all", 0 }, { "ctrl_reg", 0 }, { "pack", 0 }, { "mdg_type", SW_MDG_TYPE_SINGLE } }, OptPrintWithKey | OptAlias },
  SwitchInfo { "wr_ug",
               { { "mov_dg", S_Vector },
                 { "mov_dual_group", S_Vector } },
               TPCII::SW_WR_UPPER_GROUP, AllCores, T_ALL,
               { { "0", 0 },
                 { "1", TPCII::SW_WR_UPPER_GROUP } }, IsImmMask,
               { { "all", 0 }, { "ctrl_reg", 0 }, { "pack", 0 }, { "mdg_type", SW_MDG_TYPE_SINGLE } }, OptPrintAlways | OptPrintWithKey },
  SwitchInfo { "wr_upper_group",
               { { "mov_dg", S_Vector },
                 { "mov_dual_group", S_Vector } },
               TPCII::SW_WR_UPPER_GROUP, AllCores, T_ALL,
               { { "0", 0 },
                 { "1", TPCII::SW_WR_UPPER_GROUP } }, IsImmMask,
               { { "all", 0 }, { "ctrl_reg", 0 }, { "pack", 0 }, { "mdg_type", SW_MDG_TYPE_SINGLE } }, OptPrintWithKey | OptAlias },
  SwitchInfo { "sdg0",
               { { "mov_dg", S_Vector },
                 { "mov_dual_group", S_Vector } },
               TPCII::SW_SDG0, TPC::FeatureBF16, T_ALL,
               { { "0", 0                         },
                 { "1", 1 << TPCII::SW_SDG0_SHIFT },
                 { "2", 2 << TPCII::SW_SDG0_SHIFT },
                 { "3", 3 << TPCII::SW_SDG0_SHIFT } }, 0,
               { { "all", SW_MDG_TYPE_ALL }, { "ctrl_reg", 0 }, { "pack", 0 }, { "mdg_type", SW_MDG_TYPE_ALL } }, OptPrintAlways | OptPrintWithKey },
  SwitchInfo { "sdg0",
               { { "mov_dg", S_Vector },
                 { "mov_dual_group", S_Vector } },
               TPCII::SW_SDG0, TPC::FeatureMovDGUnpack, T_ALL,
               { { "0", 0                         },
                 { "1", 1 << TPCII::SW_SDG0_SHIFT },
                 { "2", 2 << TPCII::SW_SDG0_SHIFT },
                 { "3", 3 << TPCII::SW_SDG0_SHIFT } }, 0,
               { { "mdg_type", SW_MDG_TYPE_UNPACK } }, OptPrintAlways | OptPrintWithKey },
  SwitchInfo { "sdg1",
               { { "mov_dg", S_Vector },
                 { "mov_dual_group", S_Vector } },
               TPCII::SW_SDG1, TPC::FeatureBF16, T_ALL,
               { { "0", 0                         },
                 { "1", 1 << TPCII::SW_SDG1_SHIFT },
                 { "2", 2 << TPCII::SW_SDG1_SHIFT },
                 { "3", 3 << TPCII::SW_SDG1_SHIFT } }, 0,
               { { "all", SW_MDG_TYPE_ALL }, { "ctrl_reg", 0 }, { "pack", 0 }, { "mdg_type", SW_MDG_TYPE_ALL } }, OptPrintAlways | OptPrintWithKey },
  SwitchInfo { "sdg1",
               { { "mov_dg", S_Vector },
                 { "mov_dual_group", S_Vector } },
               TPCII::SW_SDG1, TPC::FeatureMovDGUnpack, T_ALL,
               { { "0", 0                         },
                 { "1", 1 << TPCII::SW_SDG1_SHIFT },
                 { "2", 2 << TPCII::SW_SDG1_SHIFT },
                 { "3", 3 << TPCII::SW_SDG1_SHIFT } }, 0,
               { { "mdg_type", SW_MDG_TYPE_UNPACK } }, OptPrintAlways | OptPrintWithKey },
  SwitchInfo { "sdg2",
               { { "mov_dg", S_Vector },
                 { "mov_dual_group", S_Vector } },
               TPCII::SW_SDG2, TPC::FeatureBF16, T_ALL,
               { { "0", 0                         },
                 { "1", 1 << TPCII::SW_SDG2_SHIFT },
                 { "2", 2 << TPCII::SW_SDG2_SHIFT },
                 { "3", 3 << TPCII::SW_SDG2_SHIFT } }, 0,
               { { "all", SW_MDG_TYPE_ALL }, { "ctrl_reg", 0 }, { "pack", 0 }, { "mdg_type", SW_MDG_TYPE_ALL } }, OptPrintAlways | OptPrintWithKey },
  SwitchInfo { "sdg2",
               { { "mov_dg", S_Vector },
                 { "mov_dual_group", S_Vector } },
               TPCII::SW_SDG2, TPC::FeatureMovDGUnpack, T_ALL,
               { { "0", 0                         },
                 { "1", 1 << TPCII::SW_SDG2_SHIFT },
                 { "2", 2 << TPCII::SW_SDG2_SHIFT },
                 { "3", 3 << TPCII::SW_SDG2_SHIFT } }, 0,
               { { "mdg_type", SW_MDG_TYPE_UNPACK } }, OptPrintAlways | OptPrintWithKey },
  SwitchInfo { "sdg3",
               { { "mov_dg", S_Vector },
                 { "mov_dual_group", S_Vector } },
               TPCII::SW_SDG3, TPC::FeatureBF16, T_ALL,
               { { "0", 0                         },
                 { "1", 1 << TPCII::SW_SDG3_SHIFT },
                 { "2", 2 << TPCII::SW_SDG3_SHIFT },
                 { "3", 3 << TPCII::SW_SDG3_SHIFT } }, 0,
               { { "all", SW_MDG_TYPE_ALL }, { "ctrl_reg", 0 }, { "pack", 0 }, { "mdg_type", SW_MDG_TYPE_ALL } }, OptPrintAlways | OptPrintWithKey },
  SwitchInfo { "sdg3",
               { { "mov_dg", S_Vector },
                 { "mov_dual_group", S_Vector } },
               TPCII::SW_SDG3, TPC::FeatureMovDGUnpack, T_ALL,
               { { "0", 0                         },
                 { "1", 1 << TPCII::SW_SDG3_SHIFT },
                 { "2", 2 << TPCII::SW_SDG3_SHIFT },
                 { "3", 3 << TPCII::SW_SDG3_SHIFT } }, 0,
               { { "mdg_type", SW_MDG_TYPE_UNPACK } }, OptPrintAlways | OptPrintWithKey },
  SwitchInfo { "weg0",
               { { "mov_dg", S_Vector },
                 { "mov_dual_group", S_Vector } },
               TPCII::SW_WEG0, TPC::FeatureBF16, T_ALL,
               { { "0", 0                         },
                 { "1", 1 << TPCII::SW_WEG0_SHIFT },
                 { "2", 2 << TPCII::SW_WEG0_SHIFT },
                 { "3", 3 << TPCII::SW_WEG0_SHIFT } }, 0,
               { { "all", SW_MDG_TYPE_ALL }, { "pack", 0 }, { "mdg_type", SW_MDG_TYPE_ALL } }, OptPrintAlways | OptPrintWithKey },
  SwitchInfo { "weg0",
               { { "mov_dg", S_Vector },
                 { "mov_dual_group", S_Vector } },
               TPCII::SW_WEG0, TPC::FeatureMovDGUnpack, T_ALL,
               { { "0", 0                         },
                 { "1", 1 << TPCII::SW_WEG0_SHIFT },
                 { "2", 2 << TPCII::SW_WEG0_SHIFT },
                 { "3", 3 << TPCII::SW_WEG0_SHIFT } }, 0,
               { { "mdg_type", SW_MDG_TYPE_UNPACK } }, OptPrintAlways | OptPrintWithKey },
  SwitchInfo { "weg1",
               { { "mov_dg", S_Vector },
                 { "mov_dual_group", S_Vector } },
               TPCII::SW_WEG1, TPC::FeatureBF16, T_ALL,
               { { "0", 0                         },
                 { "1", 1 << TPCII::SW_WEG1_SHIFT },
                 { "2", 2 << TPCII::SW_WEG1_SHIFT },
                 { "3", 3 << TPCII::SW_WEG1_SHIFT } }, 0,
               { { "all", SW_MDG_TYPE_ALL }, { "pack", 0 }, { "mdg_type", SW_MDG_TYPE_ALL } }, OptPrintAlways | OptPrintWithKey },
  SwitchInfo { "weg1",
               { { "mov_dg", S_Vector },
                 { "mov_dual_group", S_Vector } },
               TPCII::SW_WEG1, TPC::FeatureMovDGUnpack, T_ALL,
               { { "0", 0                         },
                 { "1", 1 << TPCII::SW_WEG1_SHIFT },
                 { "2", 2 << TPCII::SW_WEG1_SHIFT },
                 { "3", 3 << TPCII::SW_WEG1_SHIFT } }, 0,
               { { "mdg_type", SW_MDG_TYPE_UNPACK } }, OptPrintAlways | OptPrintWithKey },
  SwitchInfo { "weg2",
               { { "mov_dg", S_Vector },
                 { "mov_dual_group", S_Vector } },
               TPCII::SW_WEG2, TPC::FeatureBF16, T_ALL,
               { { "0", 0                         },
                 { "1", 1 << TPCII::SW_WEG2_SHIFT },
                 { "2", 2 << TPCII::SW_WEG2_SHIFT },
                 { "3", 3 << TPCII::SW_WEG2_SHIFT } }, 0,
               { { "all", SW_MDG_TYPE_ALL }, { "pack", 0 }, { "mdg_type", SW_MDG_TYPE_ALL } }, OptPrintAlways | OptPrintWithKey },
  SwitchInfo { "weg2",
               { { "mov_dg", S_Vector },
                 { "mov_dual_group", S_Vector } },
               TPCII::SW_WEG2, TPC::FeatureMovDGUnpack, T_ALL,
               { { "0", 0                         },
                 { "1", 1 << TPCII::SW_WEG2_SHIFT },
                 { "2", 2 << TPCII::SW_WEG2_SHIFT },
                 { "3", 3 << TPCII::SW_WEG2_SHIFT } }, 0,
               { { "mdg_type", SW_MDG_TYPE_UNPACK } }, OptPrintAlways | OptPrintWithKey },
  SwitchInfo { "weg3",
               { { "mov_dg", S_Vector },
                 { "mov_dual_group", S_Vector } },
               TPCII::SW_WEG3, TPC::FeatureBF16, T_ALL,
               { { "0", 0                         },
                 { "1", 1 << TPCII::SW_WEG3_SHIFT },
                 { "2", 2 << TPCII::SW_WEG3_SHIFT },
                 { "3", 3 << TPCII::SW_WEG3_SHIFT } }, 0,
               { { "all", SW_MDG_TYPE_ALL }, { "pack", 0 }, { "mdg_type", SW_MDG_TYPE_ALL } }, OptPrintAlways | OptPrintWithKey },
  SwitchInfo { "weg3",
               { { "mov_dg", S_Vector },
                 { "mov_dual_group", S_Vector } },
               TPCII::SW_WEG3, TPC::FeatureMovDGUnpack, T_ALL,
               { { "0", 0                         },
                 { "1", 1 << TPCII::SW_WEG3_SHIFT },
                 { "2", 2 << TPCII::SW_WEG3_SHIFT },
                 { "3", 3 << TPCII::SW_WEG3_SHIFT } }, 0,
               { { "mdg_type", SW_MDG_TYPE_UNPACK } }, OptPrintAlways | OptPrintWithKey },
  SwitchInfo { "mdg_type",
               { { "mov_dg", S_Vector },
                 { "mov_dual_group", S_Vector } },
               TPCII::SW_MDG_TYPE_MASK, TPC::FeatureGen4Plus, T_ALL,
               { { "single", SW_MDG_TYPE_SINGLE },
                 { "all",    SW_MDG_TYPE_ALL    },
                 { "pack",   SW_MDG_TYPE_PACK   },
                 { "unpack", SW_MDG_TYPE_UNPACK } }, 0,
               {}, OptMayBeSuffix | OptDontPrint },
  SwitchInfo { "mdg_type",
               { { "mov_dg", S_Vector },
                 { "mov_dual_group", S_Vector } },
               TPCII::SW_MDG_TYPE_MASK, TPC::FeatureGaudiB, T_ALL,
               { { "single", SW_MDG_TYPE_SINGLE },
                 { "all",    SW_MDG_TYPE_ALL    },
                 { "unpack", SW_MDG_TYPE_UNPACK } }, 0,
               {}, OptMayBeSuffix | OptDontPrint },
  SwitchInfo { "swap_type",
               { { "mov_dg", S_Vector },
                 { "mov_dual_group", S_Vector } },
               TPCII::SW_MDG_SWAP_TYPE, TPC::FeatureDoron1, T_ALL,
               { { "no_swap",   SW_NO_SWAP  },
                 { "swap_hl",   SW_SWAP_HL  },
                 { "swap_eo",   SW_SWAP_EO  },
                 { "swap_rsrv", SW_MDG_SWAP_TYPE } }, 0,
               { { "all", SW_MDG_TYPE_ALL } }, OptMayBeSuffix},
  SwitchInfo { "ctrl_reg",
               { { "mov_dg", S_Vector },
                 { "mov_dual_group", S_Vector } },
               TPCII::SW_MDG_CTRL_REG, TPC::FeatureDoron1, T_ALL,
               {}, 0, {} },
  SwitchInfo { "all",
               { { "mov_dg", S_Vector },
                 { "mov_dual_group", S_Vector } },
               TPCII::SW_MDG_TYPE_ALL, TPC::FeatureGaudi, T_ALL,
               {}, 0,
               {}, OptDontPrint },
  SwitchInfo { "all",
               { { "mov_dg", S_Vector },
                 { "mov_dual_group", S_Vector } },
               TPCII::SW_MDG_TYPE_ALL, TPC::FeatureGreco, T_ALL,
               {}, 0,
               { { "pack", 0 } }, OptDontPrint },
  SwitchInfo { "pack",
               { { "mov_dg", S_Vector },
                 { "mov_dual_group", S_Vector } },
               TPCII::SW_MDG_TYPE_PACK, TPC::FeatureGreco, T_ALL,
               {}, 0,
               { { "all", 0 } }, OptDontPrint },
  SwitchInfo { "pack_type",
               { { "mov_dg", S_Vector },
                 { "mov_dual_group", S_Vector } },
               TPCII::SW_DUAL_GROUP_PACK_TYPE, TPC::FeatureGen3Plus, T_ALL,
               { { "0", TPCII::SW_PACK21 },
                 { "1", TPCII::SW_PACK41 } }, 0,
               { { "pack", TPCII::SW_MDG_TYPE_PACK },
                 { "mdg_type", TPCII::SW_MDG_TYPE_PACK } }, OptPrintAlways | OptPrintWithKey },
  SwitchInfo { "unpack_type",
               { { "mov_dg", S_Vector },
                 { "mov_dual_group", S_Vector } },
               TPCII::SW_MDG_UNPACK_TYPE,
               TPC::FeatureGaudiB,
               T_ALL,
               { { "0", TPCII::SW_UNPACK_UPDATE_LANE0 },
                 { "1", TPCII::SW_UNPACK_UPDATE_LANE1 } }, 0,
               { { "mdg_type", TPCII::SW_MDG_TYPE_UNPACK } }, OptPrintAlways | OptPrintWithKey },
  SwitchInfo { "unpack_type",
               { { "mov_dg", S_Vector },
                 { "mov_dual_group", S_Vector } },
                TPCII::SW_MDG_UNPACK_TYPE, TPC::FeatureGen4Plus, T_ALL,
               { { "0", SW_UNPACK_UPDATE_LANE0 },
                 { "1", SW_UNPACK_UPDATE_LANE1 } }, 0,
               { { "mdg_type", TPCII::SW_MDG_TYPE_UNPACK} }, OptPrintAlways | OptPrintWithKey },
  SwitchInfo { "no_sat",
               { { "abs", S_Arith}},
               TPCII::SW_NO_SAT, TPC::FeatureGen3Plus, T_ALL },
  SwitchInfo { "x2", { { "udiv_4step", S_Scalar}},
               TPCII::SW_X2_UDIV_4STEP, TPC::FeatureGen3Plus, T_ALL},
  SwitchInfo { "vpu",
               { { "aso", S_Store } },
               SW_VPU, AllCores, T_ALL },
  SwitchInfo { "dec",
               { { "aso", S_Store } },
               SW_DEC, TPC::FeatureBF16, T_ALL },   // TODO: use more correct feature
  SwitchInfo { "partial",
               { { "ld_tnsr", S_Load | S_Store },
                 { "st_tnsr", S_Store } },
                 SW_PARTIAL, TPC::FeatureGaudi, T_ALL },
  SwitchInfo { "partial",
               { { "ld_tnsr", S_Load | S_Store },
                 { "st_tnsr", S_Store } },
                 SW_PARTIAL, TPC::FeatureGaudiB, T_ALL },
  SwitchInfo { "partial",
               { { "ld_tnsr",       S_Load | S_Store },
                 { "st_tnsr",       S_Store          },
                 { "ld_tnsr_low",   S_Load | S_Store },
                 { "ld_tnsr_high",  S_Load | S_Store },
                 { "ld_tnsr_cnvrt", S_Load | S_Store }},
                 SW_PARTIAL, TPC::FeaturePartial, T_ALL },
  SwitchInfo { "partial",
               { { "ld_g", S_Load } },
                 SW_PARTIAL_SRCB, TPC::FeatureGen3Plus, T_ALL },
  SwitchInfo { "exc",
               { { "ld_g", S_Load },
                 { "st_g", S_Store } },
                 SW_EXC, TPC::FeatureGen4Plus, T_ALL },
  SwitchInfo { "cnvrt_dt",
               { {"ld_tnsr_cnvrt", S_Load | S_Store } },
               TPCII::SW_CNVRT_DT, TPC::FeatureDoron1, T_ALL,
               { { "fp32_to_bf16", TPCII::SW_FP32_TO_BF16 },
                 { "fp32_to_fp16", TPCII::SW_FP32_TO_FP16 } },
               TPCII::SW_FP32_TO_BF16,
               {}, OptPrintAlways },
  SwitchInfo { "part",
               { {"ld_tnsr_cnvrt", S_Load | S_Store } },
               TPCII::SW_PART, TPC::FeatureDoron1, T_ALL,
               { { "low", TPCII::SW_PART_LOW },
                 { "high", TPCII::SW_PART_HIGH } },
               TPCII::SW_PART_LOW,
               {}, OptPrintAlways },
  SwitchInfo { "auto_inc_dim",
               { {"ld_tnsr_cnvrt", S_Load | S_Store } },
               TPCII::SW_AUTO_INC_DIM, TPC::FeatureDoron1, T_ALL,
               { { "inc_dim0", TPCII::SW_INC_DIM0 },
                 { "inc_dim1", TPCII::SW_INC_DIM1 },
                 { "inc_dim2", TPCII::SW_INC_DIM2 },
                 { "inc_dim3", TPCII::SW_INC_DIM3 },
                 { "inc_dim4", TPCII::SW_INC_DIM4 } },
               TPCII::SW_INC_DIM0,
               {}, OptPrintAlways },
  SwitchInfo { "unpack_dt",
               { { "ld_tnsr",      S_Load | S_Store },
                 { "ld_tnsr_low",  S_Load | S_Store },
                 { "ld_tnsr_high", S_Load | S_Store } },
               TPCII::SW_UNPACK_DT, TPC::FeatureTnsrUnpack, T_ALL,
               { { "UNPCK_16_TO_32", TPCII::SW_UNPCK_16_TO_32 },
                 { "UNPCK_8_TO_16",  TPCII::SW_UNPCK_8_TO_16 },
                 { "UNPCK_8_TO_32",  TPCII::SW_UNPCK_8_TO_32 },
                 { "UNPCK_4_TO_8",   TPCII::SW_UNPCK_4_TO_8  } } },
  SwitchInfo { "unpack",
               { { "ld_tnsr",      S_Load | S_Store },
                 { "ld_tnsr_low",  S_Load | S_Store },
                 { "ld_tnsr_high", S_Load | S_Store } },
                 SW_UNPACK, TPC::FeatureTnsrUnpack, T_ALL },
  SwitchInfo { "l0cs",
               { { "ld_tnsr",       S_Load | S_Store },
                 { "ld_tnsr_low",   S_Load | S_Store },
                 { "ld_tnsr_high",  S_Load | S_Store },
                 { "ld_g",          S_Load           },
                 { "st_g",          S_Store          },
                 { "ld_tnsr_cnvrt", S_Load | S_Store } },
               SW_L0CS, TPC::FeatureL0CS, T_ALL },
  SwitchInfo { "direct",
               { { "ld_tnsr", S_Load | S_Store },
                 { "ld_tnsr_high", S_Load | S_Store},
                 { "ld_tnsr_low",  S_Load | S_Store},
                 { "st_tnsr", S_Store },
                 { "st_tnsr_low",  S_Store },
                 { "st_tnsr_high", S_Store },
                 { "ld_tnsr_cnvrt", S_Load | S_Store } },
               SW_DIRECT, TPC::FeatureGen4Plus, T_ALL },
  SwitchInfo { "rmw_sel",
               { { "st_tnsr",      S_Store },
                 { "st_tnsr_low",  S_Store },
                 { "st_tnsr_high", S_Store },
                 { "st_tnsr_s",    S_Store } },
               SW_RMW_SEL, TPC::FeatureRMW, T_ALL },
  SwitchInfo { "rmw_sel",
               { { "st_tnsr_sqz",  S_Store } },
               SW_RMW_SEL, TPC::FeatureGen4Plus, T_ALL },
  SwitchInfo { "hw_reg",
               { { "st_tnsr_s",  S_Store } },
               SW_HW_REG, TPC::FeatureGen4Plus, T_ALL },
  SwitchInfo { "pack",
               { { "st_tnsr",      S_Store },
                 { "st_tnsr_low",  S_Store },
                 { "st_tnsr_high", S_Store } },
               SW_PACK, TPC::FeatureTnsrPack, T_ALL },
  SwitchInfo { "pack_dt",
               { { "st_tnsr",      S_Store },
                 { "st_tnsr_low",  S_Store },
                 { "st_tnsr_high", S_Store } },
               SW_PACK_DT, TPC::FeatureTnsrPackDT, T_ALL,
               { { "PCK_32_TO_16", TPCII::SW_PCK_32_TO_16 },
                 { "PCK_16_TO_8",  TPCII::SW_PCK_16_TO_8 },
                 { "PCK_32_TO_8",  TPCII::SW_PCK_32_TO_8 },
                 { "PCK_8_TO_4",   TPCII::SW_PCK_8_TO_4 } } },
  SwitchInfo { "cnt_only",
               {{"st_tnsr_sqz", S_Store}},
               SW_CNT_ONLY, TPC::FeatureDoron1, T_ALL},
  SwitchInfo { "flush",
               {{"st_tnsr_sqz", S_Store}},
               SW_FLUSH, TPC::FeatureDoron1, T_ALL},
  SwitchInfo { "func",
               { { "calc_fp_special", S_Arith } },
               // TODO: use more correct feature
               SW_FUNCID, TPC::FeatureGen2Plus, T_FP32 | T_BF16 | T_FP16,
               { { "recip", TPCII::SW_RECIP },
                 { "rsqrt", TPCII::SW_RSQRT },
                 { "sqrt",  TPCII::SW_SQRT  },
                 { "log",   TPCII::SW_LOG   },
                 { "exp",   TPCII::SW_EXP   },
                 { "tanh",  TPCII::SW_TANH  },
                 { "div",   TPCII::SW_DIV   },
                 { "pow",   TPCII::SW_POW   } }, 0,
               {}, OptPrintAlways | OptPrintWithKey },
  SwitchInfo { "func",
               { { "calc_fp_special", S_Load } },
               SW_FUNCID, TPC::FeatureDoron1, T_FP32 | T_BF16 | T_FP16,
               { { "recip", TPCII::SW_RECIP },
                 { "rsqrt", TPCII::SW_RSQRT },
                 { "sqrt",  TPCII::SW_SQRT  },
                 { "log",   TPCII::SW_LOG   },
                 { "exp",   TPCII::SW_EXP   },
                 { "tanh",  TPCII::SW_TANH  } }, 0,
               {}, OptPrintAlways | OptPrintWithKey },
  SwitchInfo { "set",
               { { "popcnt", S_Arith },
                 { "find_first", S_Arith } },
               TPCII::SW_COUNT, AllCores, T_ALL,
               { { "0", TPCII::SW_COUNT_ZEROS },
                 { "1",  TPCII::SW_COUNT_ONES } }, 0,
               {}, OptPrintAlways | OptPrintWithKey },
  SwitchInfo { "dir",
               { { "find_first", S_Arith } },
               TPCII::SW_DIRECTION, AllCores, T_ALL,
               { { "start_lsb", TPCII::SW_LSB },
                 { "start_msb",  TPCII::SW_MSB } }, 0,
               {}, OptPrintAlways },
  SwitchInfo { "source_group",
               { { "pack",   S_Vector },
                 { "unpack", S_Vector } },
               TPCII::SW_GROUP_SOURCE, AllCores, T_ALL,
               { { "0",       TPCII::SW_GROUP_0 },
                 { "1",       TPCII::SW_GROUP_1 },
                 { "group_0", TPCII::SW_GROUP_0 },
                 { "group_1", TPCII::SW_GROUP_1 } }, 0,
               {}, OptPrintAlways | OptPrintWithKey },
  SwitchInfo { "element_stride",
               { { "pack",   S_Vector },
                 { "unpack", S_Vector } },
               TPCII::SW_ELEMENT_STRIDE, AllCores, T_ALL,
               { { "2",        TPCII::SW_STRIDE_2 },
                 { "4",        TPCII::SW_STRIDE_4 },
                 { "stride_2", TPCII::SW_STRIDE_2 },
                 { "stride_4", TPCII::SW_STRIDE_4 } }, 0,
               {}, OptPrintAlways | OptPrintWithKey },
  SwitchInfo { "group_half",
               { { "unpack", S_Vector } },
               TPCII::SW_GROUP_HALF, AllCores, T_ALL,
               { { "0",            TPCII::SW_GROUP_HALF_0 },
                 { "1",            TPCII::SW_GROUP_HALF_1 },
                 { "group_half_0", TPCII::SW_GROUP_HALF_0 },
                 { "group_half_1", TPCII::SW_GROUP_HALF_1 } }, 0,
               {}, OptPrintAlways | OptPrintWithKey },
  SwitchInfo { "unpack_lane",
               { { "unpack", S_Vector} },
               TPCII::SW_UNPACK_LANE,
               TPC::FeatureGaudiB,
               T_ALL,
               { { "0", TPCII::SW_UNPACK_LANE_0 },
                 { "1", TPCII::SW_UNPACK_LANE_1 },
                 { "unpack_lane_0", TPCII::SW_UNPACK_LANE_0 },
                 { "unpack_lane_1", TPCII::SW_UNPACK_LANE_1 } }, 0,
               {}, OptPrintAlways | OptPrintWithKey },
  SwitchInfo { "unpack_lane",
               { { "unpack", S_Vector } },
               TPCII::SW_UNPACK_LANE, TPC::FeatureGen4Plus, T_INT8 | T_UINT8,
               { { "0",      TPCII::SW_UNPACK_LANE_0 },
                 { "1",      TPCII::SW_UNPACK_LANE_1 },
#if defined(__doron1__)
                 { "2",      TPCII::SW_UNPACK_LANE_2 },
                 { "3",      TPCII::SW_UNPACK_LANE_3},
#endif
                 { "lane_0", TPCII::SW_UNPACK_LANE_0 },
                 { "lane_1", TPCII::SW_UNPACK_LANE_1 } }, 0,
#if defined(__doron1__)
                 { "lane_2", TPCII::SW_UNPACK_LANE_2},
                 { "lane_3", TPCII::SW_UNPACK_LANE_3 } }, 0,
#endif
               {}, OptPrintAlways | OptPrintWithKey },
  SwitchInfo { "unpack_lane",
               { { "unpack", S_Vector } },
               TPCII::SW_UNPACK_LANE, TPC::FeatureGen4Plus, T_INT16 | T_UINT16,
               { { "0",      TPCII::SW_UNPACK_LANE_0 },
                 { "1",      TPCII::SW_UNPACK_LANE_1 },
                 { "lane_0", TPCII::SW_UNPACK_LANE_0 },
                 { "lane_1", TPCII::SW_UNPACK_LANE_1 } }, 0,
               {}, OptPrintAlways | OptPrintWithKey },
  SwitchInfo { "funcid",
               { { "get_lut_entry_and_interval_start", S_Vector } },
               TPCII::SW_LUT_FUNC, AllCores, T_ALL,
               { { "tanh",       TPCII::SW_LUT_TANH },
                 { "sqrt_rsqrt", TPCII::SW_LUT_SQRT_RSQRT },
                 { "sin_cos",    TPCII::SW_LUT_SIN_COS },
                 { "log",        TPCII::SW_LUT_LOG } } },
  SwitchInfo { "opt",
               { { "get_lut_entry_and_interval_start", S_Vector } },
               TPCII::SW_LUT_OPT, TPC::FeatureGen3Plus, T_BF16},
  SwitchInfo { "exp0",
               { { "get_lut_entry_and_interval_start", S_Vector } },
               TPCII::SW_LUT_EXP0, TPC::FeatureGen3Plus, T_BF16 | T_FP16},
  SwitchInfo { "dt",
               { { "gen_addr", S_Load | S_Store } },
               TPCII::SW_DT, TPC::FeatureGen3Plus, T_ALL,
               { { "fp32",    SW_DT_FP32    },
                 { "bf16",    SW_DT_BF16    },
                 { "int32",   SW_DT_INT32   },
                 { "uint32",  SW_DT_UINT32  },
                 { "int8",    SW_DT_INT8    },
                 { "uint8",   SW_DT_UINT8   },
                 { "int16",   SW_DT_INT16   },
                 { "uint16",  SW_DT_UINT16  },
                 { "fp16",    SW_DT_FP16    },
                 { "fp8_152", SW_DT_FP8_152 },
                 { "fp8_143", SW_DT_FP8_143 },
                 { "int64",   SW_DT_INT64   },
                 { "f16",     SW_DT_FP16    },
                 { "f32",     SW_DT_FP32    },
                 { "f16",     SW_DT_FP16    },
                 { "i32",     SW_DT_INT32   },
                 { "u32",     SW_DT_UINT32  },
                 { "i16",     SW_DT_INT16   },
                 { "u16",     SW_DT_UINT16  },
                 { "i8",      SW_DT_INT8    },
                 { "u8",      SW_DT_UINT8   },
                 { "f8_152",  SW_DT_FP8_152 },
                 { "f8_143",  SW_DT_FP8_143 },
                 { "int64",   SW_DT_INT64   } }, 0,
               {}, OptPrintAlways | OptPrintWithKey | OptMayBeSuffix },
  SwitchInfo { "dt_override",
               { { "gen_addr", S_Load | S_Store } },
               TPCII::SW_DT_OVERRIDE, TPC::FeatureGen3Plus, T_ALL },
  SwitchInfo { "pad",
               { { "gen_addr", S_Load | S_Store } },
               TPCII::SW_PAD, TPC::FeatureDoron1, T_ALL },
  SwitchInfo { "l2",
               { { "prefetch", S_Load } },
               TPCII::SW_L2, TPC::FeatureDoron1, T_ALL },
  SwitchInfo { "auto_inc",
               { { "prefetch", S_Load } },
               TPCII::SW_INC_VAL, TPC::FeatureDoron1, T_ALL,
               { { "inc_0", SW_INC_0 },
                 { "inc_1", SW_INC_1 },
                 { "inc_2", SW_INC_2 },
                 { "inc_4", SW_INC_4 },
                 { "inc_8", SW_INC_8 } }, SW_INC_0 },
  SwitchInfo { "cl",
               { { "cache_flush", S_Store } },
               TPCII::SW_CL, TPC::FeatureDoron1, T_ALL },
  SwitchInfo { "cl",
               { { "cache_invalidate", S_Store } },
               TPCII::SW_INV_CL, TPC::FeatureDoron1, T_ALL,
               {}, 0,
               { { "d", TPCII::SW_D } } },
  SwitchInfo { "no_inv",
               { { "cache_flush", S_Store } },
               TPCII::SW_NO_INV, TPC::FeatureDoron1, T_ALL },
  SwitchInfo { "log",
               { { "cache_invalidate", S_Store } },
               TPCII::SW_INV_LOG, TPC::FeatureDoron1, T_ALL },
  SwitchInfo { "mmio",
               { { "ld_l", S_Load  },
                 { "st_l", S_Store } },
               TPCII::SW_MMIO, AllCores, T_ALL },
  SwitchInfo { "lock",
               { { "ld_l", S_Load  } },
               TPCII::SW_LOCK, TPC::FeatureGen3Plus, T_ALL },
  SwitchInfo { "unlock",
               { { "st_l", S_Store  } },
               TPCII::SW_UNLOCK, TPC::FeatureGen3Plus, T_ALL },
  SwitchInfo { "vpu_only",
               { { "st_l", S_Store  } },
               TPCII::SW_VPU_ONLY, TPC::FeatureDoron1, T_ALL },
  SwitchInfo { "x2",
               { { "mov", S_Vector } },
               TPCII::SW_X2_MOV, TPC::FeatureGen3Plus, T_ALL },
  SwitchInfo { "clip_fp16",
               {{"convert", S_Vector}},
               TPCII::SW_CLIP_FP16,
               TPC::FeatureGaudiB,
               T_FP32,
               {{"0", 0},
                {"1", SW_CLIP_FP16}}
               },
  SwitchInfo { "sb",
               { { "cache_invalidate", S_Store } },
               TPCII::SW_SB, TPC::FeatureGen3Plus, T_ALL },
  SwitchInfo { "d",
               { { "cache_invalidate", S_Store } },
               TPCII::SW_D, TPC::FeatureGen4Plus, T_ALL },
  SwitchInfo { "lu",
               { { "cache_invalidate", S_Store } },
               TPCII::SW_LU, TPC::FeatureGen4Plus, T_ALL },
  SwitchInfo { "rst_lu",
               { { "cache_invalidate", S_Store } },
               TPCII::SW_RST_LU, TPC::FeatureGen4Plus, T_ALL },
  SwitchInfo { "rst_d_pref",
               { { "cache_invalidate", S_Store } },
               TPCII::SW_RST_D_PREF, TPC::FeatureGen4Plus, T_ALL },
  SwitchInfo { "cnvrt",
               { { "nearbyint", S_Vector } },
               TPCII::SW_CNVRT, TPC::FeatureGen3Plus, T_ALL },
  SwitchInfo { "target_type",
               { { "nearbyint", S_Vector }, },
               TPCII::SW_TO_TYPE, TPC::FeatureGen3Plus, T_FLOAT,
               { { "int32",  SW_TO_INT32  },
                 { "int16",  SW_TO_INT16  }, 
                 {  "int8", SW_TO_INT8    }
               },
               0,
               {}, OptPrintAlways | OptPrintWithKey },
  SwitchInfo { "upper_lanes",
               { { "nearbyint", S_Vector }, },
               TPCII::SW_NEARBY_LANES, TPC::FeatureDoron1, T_F8 },
  SwitchInfo { "limit",
                { {"fclass", S_Arith } },
                TPCII::SW_LIMIT, TPC::FeatureGen3Plus, T_FP32 | T_BF16 | T_FP16 |T_F8},
  SwitchInfo { "x2",
               { { "mac", S_Vector },
                 { "mul", S_Vector },
                 { "madd", S_Vector },
                 { "add", S_Vector },
                 { "sub", S_Vector }},
               TPCII::SW_X2_ARITHMETIC, TPC::FeatureGen4Plus, T_FP32 },
  SwitchInfo { "auto_inc",
               { { "st_l_v",      S_Store },
                 { "st_l_v_low",  S_Store },
                 { "st_l_v_high", S_Store },
                 { "ld_l_v",      S_Load },
                 { "ld_l_v_low",  S_Load },
                 { "ld_l_v_high", S_Load } },
               TPCII::SW_ST_INC, TPC::FeatureGen4Plus, T_ALL,
               { { "0",     SW_V_INC_0 },
                 { "1",     SW_V_INC_1 },
                 { "2",     SW_V_INC_2 },
                 { "4",     SW_V_INC_4 },
                 { "inc_0", SW_V_INC_0 },
                 { "inc_1", SW_V_INC_1 },
                 { "inc_2", SW_V_INC_2 },
                 { "inc_4", SW_V_INC_4 } }, SW_V_INC_0,
               {}, OptPrintWithKey },
  SwitchInfo { "div_mod",
               { { "udiv",   S_Scalar } },
               TPCII::SW_GROUP_DIV_MODE, TPC::FeatureGen4Plus, T_ALL,
               { { "div",          SW_DIV_MODE_DIV  },
                 { "mod",          SW_DIV_MODE_MOD  },
                 { "both_div_mod", SW_DIV_MODE_BOTH } }, 0,
               {}, OptPrintAlways},
  SwitchInfo { "andn",
               {{ "and", S_Vector }},
               TPCII::SW_ANDN, TPC::FeatureGen3Plus, T_ALL},
  SwitchInfo { "both",
               { { "mov_irf_dim", S_Scalar } },
               TPCII::SW_MOV_IRF_DIM_BOTH, TPC::FeatureGen4Plus, T_ALL },
  SwitchInfo { "high",
               { { "set_indx",  S_Load | S_Scalar | S_Store } },
               TPCII::SW_IRF44_HIGH, TPC::FeatureGen4Plus, T_ALL },
  SwitchInfo { "dnorm",
               { { "mac", S_Arith },
                 { "mul", S_Arith },
                 { "add", S_Arith },
                 { "sub", S_Arith },
                 { "madd", S_Vector } },
               TPCII::SW_DNORM, TPC::FeatureDoron1, T_FLOAT}

};
#if defined(__GNUC__) || defined(__clang__)
#pragma GCC diagnostic pop
#endif


static const MCSubtargetInfo *SubTargetInfo = nullptr;

void TPCII::setSubTargetInfo(const MCSubtargetInfo *STI) {
  SubTargetInfo = STI;
}

const MCSubtargetInfo *TPCII::getSubtargetInfo() {
  assert(SubTargetInfo);
  return SubTargetInfo;
}

static const MCInstrInfo *InstrInfo = nullptr;

void TPCII::setInstrInfo(const MCInstrInfo* MCII) {
  InstrInfo = MCII;
}
const MCInstrInfo *TPCII::getInstrInfo() {
  assert(InstrInfo);
  return InstrInfo;
}

static std::map<MnemonicKey, InstructionInfo> SwitchesOfInstruction;
static std::map<const SwitchInfo *, std::vector<DependencyRecord>> SwitchDependency;

static void collectDependencies(const SwitchInfo &E, std::vector<DependencyRecord> &V) {
  for (auto &Other : AllSwitches)
    for (auto &D : E.Dependencies)
      if (Other.Name.equals_lower(D.Switch)) {
        // check if the other switch ever applies to any common case with E
        // exact matching should happen for particular instruction at runtime
        if ((Other.OpType & E.OpType) != 0 && E.fitsOtherFeatures(Other)
          && Other.Instructions.end() != std::find_first_of(Other.Instructions.begin(), Other.Instructions.end(),
          E.Instructions.begin(), E.Instructions.end(), [=](const MnemonicKey &R, const MnemonicKey &L) -> bool {
          return (R.Slot & L.Slot) != 0 && R.Mnemonic == L.Mnemonic ; })
        ) {
          V.push_back(DependencyRecord{ &Other, D.Value });
          DISASM_DEBUG(dbgs() << "Added switch dependency: " << E.Name << " => " << D.first << " == " << D.second << " w/feature " << Other.Features << '\n');
        }
        else {
          DISASM_DEBUG(dbgs() << "Beware refused switch dependency: " << E.Name << " => " << Other.Name << '\n');

        }
      }
}


static void buildSwitchMaps() {
  assert(SwitchesOfInstruction.empty());
  for (const auto &Item : AllOpCodes) {
    MnemonicKey K{ Item.second.lower(), getSlotCode(Item.first.Slot) };
    SwitchesOfInstruction[K].Name = Item.second;
  }

  for (auto &Sw : AllSwitches) {
    for (auto InstInSlot : Sw.Instructions) {
      if (InstInSlot.Slot & S_Special) {
        MnemonicKey K{ InstInSlot.Mnemonic, S_Special };
        SwitchesOfInstruction[K].Switches.insert(&Sw);
      }
      if (InstInSlot.Slot & S_Load) {
        MnemonicKey K{ InstInSlot.Mnemonic, S_Load };
        SwitchesOfInstruction[K].Switches.insert(&Sw);
      }
      if (InstInSlot.Slot & S_Scalar) {
        MnemonicKey K{ InstInSlot.Mnemonic, S_Scalar };
        SwitchesOfInstruction[K].Switches.insert(&Sw);
      }
      if (InstInSlot.Slot & S_Vector) {
        MnemonicKey K{ InstInSlot.Mnemonic, S_Vector };
        SwitchesOfInstruction[K].Switches.insert(&Sw);
      }
      if (InstInSlot.Slot & S_Store) {
        MnemonicKey K{ InstInSlot.Mnemonic, S_Store };
        SwitchesOfInstruction[K].Switches.insert(&Sw);
      }
    }

    if (!Sw.Dependencies.empty()) {
      std::vector<DependencyRecord> V;
      collectDependencies(Sw, V);
      if (!V.empty()) {
        auto &D = SwitchDependency[&Sw];
        D.insert(D.end(), V.begin(), V.end());
      }
    }
  }
}

static const std::map<MnemonicKey, InstructionInfo> &getSwitchesOfInstruction() {
  if (SwitchesOfInstruction.empty())
    buildSwitchMaps();
  return SwitchesOfInstruction;
}

static std::pair<std::map<OpCodeKey, StringRef>::const_iterator, std::map<OpCodeKey, StringRef>::const_iterator>
        getMnemonicsFromOpCode(unsigned OpCode, SlotParser Slot) {
  auto Ptr = AllOpCodes.equal_range(OpCodeKey{ Slot, OpCode });
  assert(Ptr.first != AllOpCodes.end());
  return Ptr;
}

static const InstructionInfo *getSwitchesForMnemonic(StringRef Mnemonic, SlotParser Slot) {
  auto &SwitchesOfInstruction = getSwitchesOfInstruction();
  auto Ptr = SwitchesOfInstruction.find(MnemonicKey{ Mnemonic.lower(), getSlotCode(Slot) });
  if (Ptr == SwitchesOfInstruction.end())
    return nullptr;
  return &Ptr->second;
}

// Excludes switches not suitable for current architecture, operation type and aliases.
static std::vector<const SwitchInfo *> getApplicableSwitches(const InstructionInfo *IInfo, const FeatureBitset &Features, OpType Type) {
  std::vector<const SwitchInfo *> Switches(IInfo->Switches.begin(), IInfo->Switches.end());
  unsigned T_Type = getTypeCode(Type);
  Switches.erase(std::remove_if(Switches.begin(), Switches.end(), [&](const SwitchInfo *E) -> bool {
      return (E->Options & OptAlias) || !E->fitsTargetFeatures(Features) || (E->OpType & T_Type) == 0;
    }), Switches.end());
  return Switches;
}

class SwitchErrorCat : public std::error_category {
public:
  const char *name() const noexcept override { return "switch"; }
  std::string message(int x) const override {
    switch (static_cast<SwitchError>(x)) {
    case SwitchError::OK: return "ok";
    case SwitchError::UnknownSwitch: return "unknown switch";
    case SwitchError::UnsupportedByHardware: return "switch is not supported by the current processor";
    case SwitchError::UnapplicableForType: return "switch is not valid for the used operand type";
    case SwitchError::NonBooleanValueOfFlag: return "Olny values '0' or '1' are allowed for this switch";
    case SwitchError::SwitchGroupNoValue: return "expected specification in the form 'Switch=VALUE'";
    default: return "error " + std::to_string(x);
    }
  }
};

const SwitchErrorCat theSwitchErrorCat;

std::error_code make_error_code(SwitchError e) {
  return { static_cast<int>(e), theSwitchErrorCat };
}

unsigned llvm::TPCII::getOpTypeMinSizeBits(const OpType Ty) {
  // clang-format off
  switch (Ty) {
  case FP32:    return 32;
  case BF16:    return 16;
  case INT32:   return 32;
  case UINT32:  return 32;
  case INT8:    return 8;
  case UINT8:   return 8;
  case BOOL:    return 1;
  case INT16:   return 16;
  case UINT16:  return 16;
  case INT4:    return 4;
  case UINT4:   return 4;
  case FP16:    return 16;
  case FP8_152: return 8;
  case FP8_143: return 8;
  case INT64:   return 64;
  case Invalid:
    assert(false);
    return 64;
  }
  // clang-format on
  llvm_unreachable("unexpected type");
}

std::string TPCII::incorporateSwitch(StringRef Switch, StringRef Value,
                                     StringRef Mnemonic, SlotParser Slot,
                                     OpType Type, bool IsSuffix, bool &IsUnknown,
                                     unsigned &CurrentSwitchSet,
                                     std::vector<std::string> &Switches) {
  assert(Slot != SlotParser::Unknown);
  IsUnknown = false;

  // If we found a record for the switch which is not suitable, keep them. It
  // allows us to distinguish between an unknow switch and a known but unsupported.
  const SwitchInfo *UnsupportedCPU = nullptr;
  const SwitchInfo *UnapplicableType = nullptr;

  std::string ErrorMessage;

  if (const InstructionInfo *Info = getSwitchesForMnemonic(Mnemonic, Slot)) {
    for (auto SwPtr : Info->Switches) {
      bool IsGroup = SwPtr->isGroup();
      if (SwPtr->Name.equals_lower(Switch)) {

        // Check if the found switch is supported.
        assert(SubTargetInfo);
        if (!SwPtr->fitsTargetFeatures(SubTargetInfo->getFeatureBits())) {
          UnsupportedCPU = SwPtr;
          continue;
        }
        if (Type != TPCII::OpType::Invalid) {
          if ((SwPtr->OpType & getTypeCode(Type)) == 0) {
            UnapplicableType = SwPtr;
            continue;
          }
        }

        unsigned SwValue = SwPtr->Value;  // Either switch value or a group mask.

        // If this switch depends on some other switch, check if the dependency
        // is not violated.
        if (SwitchDependency.count(SwPtr))
          for (auto R : SwitchDependency[SwPtr]) {
            if (R.Switch->fitsTargetFeatures(SubTargetInfo->getFeatureBits())
              && (R.Switch->OpType & getTypeCode(Type)) != 0) {
              unsigned SelectionBits = CurrentSwitchSet & R.Switch->Value;
              if (SelectionBits && SelectionBits != R.Selection) {
                // Possibly, there is another variant
                if (ErrorMessage.empty())
                  ErrorMessage = "Switch '" + Switch.str() +
                    "' conflicts with switch '" + R.Switch->Name.str();
                continue;
              }
            }
          }

        if (!IsGroup && !Value.empty()) {
          // Even if switch is boolean, we allow specification of it in the form:
          // 'SWITCH=1' or 'SWITCH=0'.

          // Check for redefinition.
          if (SwValue & CurrentSwitchSet ||
              std::find(Switches.begin(), Switches.end(), SwPtr->Name) != Switches.end()) {
            return "this switch is already specified";
          }

          // Check the specified value.
          if (Value == "0") {
            // Zero value is specified by default.
            Switches.push_back(std::string(SwPtr->Name));
            return "";
          } else if (Value == "1") {
            CurrentSwitchSet |= SwValue;
            Switches.push_back(std::string(SwPtr->Name));
            return "";
          }
          return "olny values '0' or '1' are allowed for this switch";
        }

        if (IsGroup) {
          if (Value.empty())
            return "expected specification in the form 'SWITCH=VALUE'";

          // The switch may be represented as 'SWITCH=123' or 'SWITCH=STRING'.
          unsigned ItemValue;
          std::string LCValue = Value.lower();
          bool Found = false;
          for (auto V : SwPtr->Values)
            if (V.Name.equals_lower(LCValue)) {
              ItemValue = V.Value;
              Found = true;
              break;
            }
          if (!Found) {
            if (!Value.getAsInteger(0, ItemValue)) {
              unsigned Shift = 0;
              unsigned Mask = SwValue;
              while (Mask && ((Mask & 1) == 0)) {
                ++Shift;
                Mask >>= 1;
              }
              ItemValue <<= Shift;
            }
          }
          if (ItemValue & ~SwPtr->Value)
            return "switch value has bits beyond its group";
          if (ItemValue & CurrentSwitchSet)
            return "value for this group is already specified";
          CurrentSwitchSet |= ItemValue;
          Switches.push_back(std::string(SwPtr->Name));
          return "";
        }

        // Switch is specified by its name, like 'SAT'.
        if (SwValue & CurrentSwitchSet)
          return "this switch is already specified";
        CurrentSwitchSet |= SwValue;
        Switches.push_back(std::string(SwPtr->Name));
        return "";
      }

      // Check if this is a value specified in some switch group.
      if (IsGroup) {
        for (auto SwVal : SwPtr->Values) {
          if (SwVal.Name.equals_lower(Switch)) {
            assert(SubTargetInfo);
            if (!SwPtr->fitsTargetFeatures(SubTargetInfo->getFeatureBits())) {
              UnsupportedCPU = SwPtr;
              continue;
            }
            if (Type != TPCII::OpType::Invalid) {
              if ((SwPtr->OpType & getTypeCode(Type)) == 0) {
                UnapplicableType = SwPtr;
                continue;
              }
            }
            if (IsSuffix && (SwPtr->Options & OptMayBeSuffix) == 0)
              return "switch \"" + Switch.str() + "\" cannot be specified as a suffix";
            if (SwVal.Value & CurrentSwitchSet)
              return "value for this group is already specified";
            CurrentSwitchSet |= SwVal.Value;
            Switches.push_back(std::string(SwPtr->Name));
            return "";
          }
        }
      }
    }

    if (!ErrorMessage.empty())
      return ErrorMessage;
  }



  if (UnsupportedCPU)
    return "switch is not supported by the current processor";
  if (UnapplicableType)
    return "switch is not valid for the used operand type";
  IsUnknown = true;
  return "unknown switch";
}

static bool meetsDependencies(const unsigned SwSet, const SwitchInfo *E, const std::vector<const SwitchInfo *> &Switches) {
  if (SwitchDependency.count(E) == 0)
    return true;
  const FeatureBitset &Features = SubTargetInfo->getFeatureBits();
  std::vector<DependencyRecord> &Deps = SwitchDependency[E];
  for (DependencyRecord &R : Deps) {
    if (R.Switch->Feature != AllCores && !Features[R.Switch->Feature])
      continue;
    if (std::find(Switches.begin(), Switches.end(), E) != Switches.end()) {
      LLVM_DEBUG(dbgs() << "Checking " << E->Name << " switch dependency: " << R.Switch->Name << " == " << R.Selection << '\n');
      if ((R.Switch->Value & SwSet) != R.Selection) {
        LLVM_DEBUG(dbgs() << "Switch " << E->Name << " does not meet dependency: " << R.Switch->Name << " == " << R.Selection << '\n');
        return false;
      }
    }
  }
  return true;
}


bool TPCII::getDefaultSwitches(StringRef Mnemonic, SlotParser Slot, OpType Type,
                               unsigned &CurrentSwitchSet,
                               const std::vector<std::string> &Switches) {
  assert(Slot != SlotParser::Unknown);
  assert(SubTargetInfo);
  bool Changed = false;
  if (const InstructionInfo *Info = getSwitchesForMnemonic(Mnemonic, Slot)) {
    auto Sws = getApplicableSwitches(Info, SubTargetInfo->getFeatureBits(), Type);
    for (auto SwPtr : Sws) {
      if (((SwPtr->Default) & ~PrintFlagMask) == 0)
        continue;
      if (std::find(Switches.begin(), Switches.end(), SwPtr->Name) != Switches.end())
        continue;
      if (!meetsDependencies(CurrentSwitchSet, SwPtr, Sws))
        continue;
      CurrentSwitchSet |= (SwPtr->Default) & ~PrintFlagMask;
      Changed = true;
    }
  }
  return Changed;
}

static TPCII::OpType guessDataType(const MCInst *MI, unsigned OpNum, const MCInstrDesc &MCID) {
  // We expect datatype just before switch set.
  if (OpNum) {
      const MCOperandInfo &Info = MCID.OpInfo[OpNum - 1];
      if (Info.OperandType == TPC::OPERAND_DATATYPE) {
        const MCOperand &Op = MI->getOperand(OpNum - 1);
        assert(Op.isImm() && Op.getImm() <= TPCII::OpType::Max);
        if (Op.getImm() <= TPCII::OpType::Max)
          return static_cast<TPCII::OpType>(Op.getImm());
      }
  }
  DISASM_DEBUG(dbgs() << "Failed to guessDataType: " << *MI << " for oOpNum = " << OpNum);
  return TPCII::OpType::Invalid;
}

static std::string numToBinStr(long Number) {
  std::string Result("0b");
  auto Bits = std::bitset<sizeof(Number) * 8>(Number);
  // find first signed bit
  int StartPos = 0;
  for (int i = Bits.size() -1; i >= 0; --i)
    if(Bits.test(i)) {
      StartPos = i;
      break;
    }
  for (int i = StartPos; i >=0; --i)
    Result += Bits.test(i) ? '1' : '0';

  return Result;
}

static std::string numToHexStr(long Number) {
  std::stringstream Stream;
  Stream << "0x";
  Stream << std::hex << Number;
  return Stream.str();
}

// If a switch has a immediate mask a value convert to binary or hexadecimal
// format.
// Otherwise value is returned without a changes.
static std::string tryFormatMaskedSwitch(const SwitchInfo *SwInfo,
                                         unsigned SwSet, bool NeedGroupName) {
  assert(SwInfo != nullptr);
  std::string SwStr;
  for (auto V : SwInfo->Values)
    if ((SwSet & SwInfo->Value) == V.Value) {
      char *EndPtr = nullptr;
      long NumberValue = strtol(V.Name.data(), &EndPtr, 10);
      if ((SwInfo->Default & IsImmMask) && EndPtr == V.Name.end())
        SwStr = (SwInfo->Options & OptPrintAsBinary) ? numToBinStr(NumberValue)
                                                     : numToHexStr(NumberValue);
      else
        SwStr = std::string(V.Name);

      // If we found numeric value for the switch, and printing group
      // name was not requested, continue the search so that we could
      // find identifier.
      if (NeedGroupName || !std::isdigit(SwStr[0]))
        break;
    }

  return SwStr;
}

static bool switchIsAmbiguous(const unsigned SwSet, const SwitchInfo *SwPtr,
                              const std::vector<const SwitchInfo *> &Switches, const FeatureBitset &Features) {
  for (auto Ptr : Switches)
    if (Ptr != SwPtr && Ptr->fitsTargetFeatures(Features))
      if (Ptr->Value == SwPtr->Value && meetsDependencies(SwSet, Ptr, Switches)) {
        DISASM_DEBUG(dbgs() << "switchIsAmbiguous: " << SwPtr->Name << " and " << Ptr->Name << '\n');
        return true;
      }
  return false;
}

static std::string
buildSwitchSetString(const unsigned SwitchSet, const std::string &Mnemonic, SlotParser Slot, OpType Type,  bool IsIRFOp) {
  assert(SubTargetInfo);
  const FeatureBitset &Features = SubTargetInfo->getFeatureBits();

  // Get set of switches valid for this instruction.
  const InstructionInfo *IInfo = getSwitchesForMnemonic(Mnemonic, Slot);
  if (!IInfo) {
    // No information about switches of this instruction. Print the switch set as a number.
    return std::to_string(SwitchSet);
  }

  // Filter out incompatible switches.
  // Get array of switches for more convenient manipulations.
  std::vector<const SwitchInfo *> Switches = getApplicableSwitches(IInfo, Features, Type);

  Switches.erase(std::remove_if(Switches.begin(), Switches.end(), [&](const SwitchInfo *E) -> bool {
    if (IsIRFOp && (E->Value & SW_DIMMASK) && !E->Name.equals("dimmask")) {
      DISASM_DEBUG(dbgs() << "Filtered out " << E->Name << " since it overlaps with dimmask\n");
      return true;
    }
    return false; }), Switches.end());

  // Sort switches by value.
  std::sort(Switches.begin(), Switches.end(), [](const SwitchInfo *A, const SwitchInfo *B) -> bool {
    return A->Value < B->Value;
  });

  // Scan all valid switches and print them if we found them in the provided switch set.
  std::string Result;
  unsigned SwSet = SwitchSet;
  for (auto SwInfo : Switches) {
    DISASM_DEBUG(dbgs() << " testing switch " << SwInfo->Name << '\n');
    if (!meetsDependencies(SwitchSet, SwInfo, Switches))
      continue;
    std::bitset<32> SwBits(SwInfo->Value);

    bool NeedGroupName = !SwInfo->Name.empty() && SwInfo->Options & OptPrintWithKey;
    if (SwSet & SwInfo->Value) {
      if ((Slot == SlotParser::Scalar || Slot == SlotParser::Vector) && Type == OpType::Invalid) {
        if (Slot == SlotParser::Vector) {
          // TPCDisasmInstrInfo.td defines some simplified formats w/o explicit DataType operand
          // and hardcoded switches, have to ignore such switch here
          if (SwInfo->Value == TPCII::SW_ACC_FP32 && (Mnemonic == "mac" || Mnemonic == "madd")) {
            SwSet &= ~SwInfo->Value;
            continue;
          }
        }
      }
      assert(!switchIsAmbiguous(SwitchSet, SwInfo, Switches, Features));

      // If the switch should not be printed, just remove its bits from the
      // switch set value.
      if (SwInfo->Options & OptDontPrint) {
        SwSet &= ~SwInfo->Value;
        continue;
      }

      if (!Result.empty())
        Result += " ";
      if (SwBits.count() == 1 && SwInfo->Values.empty()) {
        // Single bit switches are printed as their names.
        Result += SwInfo->Name.lower();
      } else if ((SwSet & SwInfo->Value) == SwInfo->Default &&
                 (SwInfo->Options & OptPrintAlways) == 0) {
        // If value of the multibit switch is default, do not print it.
        SwSet &= ~SwInfo->Value;
      } else {
        // This is multibit switch.
        if (SwInfo->Name.equals("dimmask")) {
          Result += 'b';
          Result += std::bitset<5>((SwSet >> 2) & 0x1f).to_string();
        } else {
          std::string SwStr = tryFormatMaskedSwitch(SwInfo, SwSet, NeedGroupName);
          if (NeedGroupName || SwStr.empty()) {
            Result += SwInfo->Name.lower();
            Result += '=';
          }
          if (!SwStr.empty()) {
            Result += SwStr;
          } else
            Result += numToHexStr(SwSet & SwInfo->Value);
        }
      }
      SwSet &= ~SwInfo->Value;
    } else {
      // Current switch has zero value. Print it if it represents a switch group,
      // but not the default value.
      bool ShallPrint = SwInfo->Default != 0 || (SwInfo->Options & OptPrintAlways);
      if (ShallPrint && !SwInfo->Values.empty()) {
        std::string ValueStr = tryFormatMaskedSwitch(SwInfo, SwSet, NeedGroupName);
        if (!ValueStr.empty()) {
          if (!Result.empty())
            Result += " ";
          if (NeedGroupName) {
            Result += SwInfo->Name.lower();
            Result += '=';
          }
          Result += ValueStr;
        }
      }
    }

    // If nothing was printed for the switch, print its hexadecimal value.
    if (SwSet & SwInfo->Value) {
      Result += "0x";
      std::stringstream stream;
      stream << std::hex << (SwSet & SwInfo->Value);
      Result += stream.str();
      SwSet &= ~SwInfo->Value;
    }
  }

  return Result;
}

static bool hasDimMask(const MCInst *MI, const MCInstrDesc &MCID,
                       const MCRegisterInfo &MRI) {
  for (const MCOperandInfo &Info : MCID.operands()) {
    if (Info.OperandType == TPC::OPERAND_DIMMASK)
      return true;
  }
  // Disasm often uses RR hack (e.g. matches MULssp instead of MULIIp) so the
  // above condition does not work Then just check if destination register is
  // IRF
  if (MI->getNumOperands() && MCID.getNumDefs()) {
    const MCOperand &Op = MI->getOperand(0);
    if (Op.isReg() && MRI.getRegClass(TPC::IRFRegClassID).contains(Op.getReg()))
      return true;
  }

  return false;
}

static TPCII::OpType getTPCTypeByName(const MCInst *MI) {
  assert(MI);
  StringRef RecordName = InstrInfo->getName(MI->getOpcode());
  
  // Order from longest to shortest
  std::unordered_map<const char *, TPCII::OpType> Types = {
    {"f8_152", TPCII::OpType::FP8_152},
    {"f8_143", TPCII::OpType::FP8_143},
    {"bf16", TPCII::OpType::BF16},
    {"f32", TPCII::OpType::FP32},
    {"f16", TPCII::OpType::FP16},
    {"i32", TPCII::OpType::INT32},
    {"u32", TPCII::OpType::UINT32},
    {"i16", TPCII::OpType::INT16},
    {"u16", TPCII::OpType::UINT16},
    {"i64", TPCII::OpType::INT64},
    {"i8", TPCII::OpType::INT8},
    {"u8", TPCII::OpType::UINT8},
    {"i4", TPCII::OpType::INT4},
    {"u4", TPCII::OpType::UINT4}
  };
  
  for (auto &Pair : Types) {
    if (RecordName.contains(Pair.first))
      return Pair.second;
  }
  
  return TPCII::OpType::Invalid;
}

std::string TPCII::spellSwitchSet(unsigned SwSet, const MCInst *MI, unsigned OpNum, const MCInstrDesc &MCID, const MCRegisterInfo &MRI) {
  // Get mnemonic for this instruction.
  unsigned OpCode = TPCII::getSlotOpCode(MCID);
  unsigned SlotType = TPCII::getInstrType(MCID);
  SlotParser Slot;
  switch (SlotType) {
  case TPCII::TypeLOAD: Slot = SlotParser::Load; break;
  case TPCII::TypeSPU: Slot = SlotParser::Scalar; break;
  case TPCII::TypeVPU: Slot = SlotParser::Vector; break;
  case TPCII::TypeSTORE: Slot = SlotParser::Store; break;
  case TPCII::TypeLOOP: Slot = SlotParser::Special; break;
  default:
    llvm_unreachable("Invalid slot type");
  }
  bool isIRFop = Slot == SlotParser::Scalar && hasDimMask(MI, MCID, MRI);
  TPCII::OpType Type = TPCII::OpType::Invalid;
  if (Slot == SlotParser::Scalar || Slot == SlotParser::Vector) {
    // If there are more than one spelling for the same bit (like 'upper32'
    // and 'acc_fp32'), try to determine the appropriate one. To do this we
    // need to guess data type associated with this instruction. It is possible
    // only for arithmetic slots.
    Type = guessDataType(MI, OpNum, MCID);
  }
  
  // Сrutch! TODO: DO something with it.
  if (Type == TPCII::OpType::Invalid)
    Type = getTPCTypeByName(MI);

  std::string Result;
  auto OpCodes = getMnemonicsFromOpCode(OpCode, Slot);
  for(auto OpcodeIt = OpCodes.first; OpcodeIt != OpCodes.second; OpcodeIt++) {
    DISASM_DEBUG(dbgs() << "buildSwitchSetString for " << *MI << " aka " << OpcodeIt->second.lower() << '\n');
    Result += buildSwitchSetString(SwSet, OpcodeIt->second.lower(), Slot, Type, isIRFop);
  }
  return Result;
}


bool TPCII::doesInstructionHasASwitch(StringRef Mnemonic, SlotParser Slot) {
  const std::map<MnemonicKey, InstructionInfo> &SOI =
      getSwitchesOfInstruction();
  auto IPtr = SOI.find(MnemonicKey{ std::string(Mnemonic), getSlotCode(Slot) });
  if (IPtr == SOI.end() || IPtr->second.Switches.empty())
    return false;
  // If the only switch is dimmask, report as if the instruction does not have switches.
  if (IPtr->second.Switches.size() == 1 && (*IPtr->second.Switches.begin())->Name.equals("dimmask"))
    return false;
  return true;
}
