//===----------------------------------------------------------------------===//
//
// Utility functions for TPC specific MCInst queries
//
//===----------------------------------------------------------------------===//

#ifndef LLVM_LIB_TARGET_TPC_MCTARGETDESC_TPCMCINSTRINFO_H
#define LLVM_LIB_TARGET_TPC_MCTARGETDESC_TPCMCINSTRINFO_H

//#include "TPCMCExpr.h"
#include "InstructionDB.h"
#include "llvm/MC/MCInst.h"
#include "llvm/MC/MCInstrDesc.h"

namespace llvm {
class MachineInstr;
class MCContext;
class MCInstrDesc;
class MCInstrInfo;
class MCInst;
class MCOperand;
class MCSubtargetInfo;

namespace TPCII {

// Instruction types.
// *** Must match TPCInstrFormat*.td ***
enum IType {
  TypeNone      = 0,
  TypeVPU       = 1,
  TypeSPU       = 2,
  TypeLOAD      = 3,
  TypeSTORE     = 4,
  TypeLOOP      = 5
};

enum CmpMode {
  LoopGT =  0,
  LoopGE =  1,
  LoopLT =  2,
  LoopLE =  3,
  LoopEQ =  4,
  LoopNE =  5,
  LoopErr = -1
};


// MCInstrDesc TSFlags layout
// *** Must match TPCInstrFormat*.td ***

const unsigned ITypeStart  = 0;
const unsigned ITypeEnd  = 2;
const unsigned ITypeSize = (ITypeEnd - ITypeStart + 1);
const uint64_t ITypeMask = (1ULL << ITypeSize) - 1;

const unsigned HasImmStart = 3;
const uint64_t HasImmMask = 1ULL;

const unsigned IsPredicatedStart = 4;
const uint64_t IsPredicatedMask = 1ULL;

const unsigned OutOfSlotDataStart = 5;
const uint64_t OutOfSlotDataMask = 1ULL;

const unsigned HasImmFieldStart = 6;
const uint64_t HasImmFieldMask = 1ULL;

const unsigned ImmStartBitStart = 7;
const unsigned ImmStartBitEnd = 11;
const unsigned ImmStartBitSize = (ImmStartBitEnd - ImmStartBitStart + 1);
const uint64_t ImmStartBitMask = (1ULL << ImmStartBitSize) - 1;

const unsigned ImmEndBitStart = 12;
const unsigned ImmEndBitEnd = 16;
const unsigned ImmEndBitSize = (ImmEndBitEnd - ImmEndBitStart + 1);
const uint64_t ImmEndBitMask = (1ULL << ImmEndBitSize) - 1;

const unsigned ImmOpNumStart  = 17;
const unsigned ImmOpNumEnd  = 20;
const unsigned ImmOpNumSize = (ImmOpNumEnd - ImmOpNumStart + 1);
const uint64_t ImmOpNumMask = (1ULL << ImmOpNumSize) - 1;

const unsigned SlotOpCodeStart  = 24;
const unsigned SlotOpCodeEnd  = 29;
const unsigned SlotOpCodeSize = (SlotOpCodeEnd - SlotOpCodeStart + 1);
const uint64_t SlotOpCodeMask = (1ULL << SlotOpCodeSize) - 1;

// TODO: Remove this.
const unsigned OpTypeCodeStart  = 30;
const unsigned OpTypeCodeEnd  = 33;
const unsigned OpTypeCodeSize = (OpTypeCodeEnd - OpTypeCodeStart + 1);
const uint64_t OpTypeCodeMask = (1ULL << OpTypeCodeSize) - 1;

const unsigned HasCompositeImmStart = 34;
const uint64_t HasCompositeImmMask = 1ULL;

const unsigned ImmOpCountStart = 35;
const uint64_t ImmOpCountMask = 3ULL; //011b

const unsigned SecondImmOpStart = 37;
const unsigned SecondImmOpEnd = 40;
const unsigned SecondImmOpSize = (SecondImmOpEnd - SecondImmOpStart + 1);
const uint64_t SecondImmOpMask = (1ULL << SecondImmOpSize) - 1;

const unsigned ThirdImmOpStart = 41;
const unsigned ThirdImmOpEnd = 44;
const unsigned ThirdImmOpSize = (ThirdImmOpEnd - ThirdImmOpStart + 1);
const uint64_t ThirdImmOpMask = (1ULL << ThirdImmOpSize) - 1;

const unsigned FirstImmBitsStart = 45;
const unsigned FirstImmBitsEnd = 48;
const unsigned FirstImmBitsSize = (FirstImmBitsEnd - FirstImmBitsStart + 1);
const uint64_t FirstImmBitsMask = (1ULL << FirstImmBitsSize) - 1;

const unsigned SecondImmBitsStart = 49;
const unsigned SecondImmBitsEnd = 52;
const unsigned SecondImmBitsSize = (SecondImmBitsEnd - SecondImmBitsStart + 1);
const uint64_t SecondImmBitsMask = (1ULL << SecondImmBitsSize) - 1;

const unsigned ThirdImmBitsStart = 53;
const unsigned ThirdImmBitsEnd = 56;
const unsigned ThirdImmBitsSize = (ThirdImmBitsEnd - ThirdImmBitsStart + 1);
const uint64_t ThirdImmBitsMask = (1ULL << ThirdImmBitsSize) - 1;

const unsigned LanesStart = 57;
const unsigned LanesEnd = 59;
const unsigned LanesSize = (LanesEnd - LanesStart + 1);
const uint64_t LanesMask = (1ULL << LanesSize) - 1;

const unsigned SrcCIsStoreSrcCStart = 61;
const uint64_t SrcCIsStoreSrcCMask = 1ULL;

const unsigned HasSrcCStart = 62;
const uint64_t HasSrcCMask = 1ULL;

const unsigned HasSrcDStart = 63;
const uint64_t HasSrcDMask = 1ULL;

// MCInst Flags
const unsigned MCFlagSPU       = 0;
const unsigned MCFlagVPU       = 1;
const unsigned MCFlagLDU       = 2;
const unsigned MCFlagSTU       = 3;
const unsigned MCFlagLOOP      = 5;
const unsigned MCSlotFlagMask  = 0b111;

const unsigned MCFlagIsUnpredDoron1 = 8;

// Accessors to the data in TSFlags.

inline unsigned getInstrType(const MCInstrDesc &MCInstD) {
  return static_cast<unsigned>((MCInstD.TSFlags >> ITypeStart) & TPCII::ITypeMask);
}

inline bool isVPUInst(const MCInstrDesc &MCInstD) {
  return getInstrType(MCInstD) == TPCII::TypeVPU;
}

inline bool isSPUInst(const MCInstrDesc &MCInstD) {
  return getInstrType(MCInstD) == TPCII::TypeSPU;
}

inline bool isStoreInst(const MCInstrDesc &MCInstD) {
  return getInstrType(MCInstD) == TPCII::TypeSTORE;
}

inline bool isLoadInst(const MCInstrDesc &MCInstD) {
  return getInstrType(MCInstD) == TPCII::TypeLOAD;
}

inline bool isLoopInst(const MCInstrDesc &MCInstD) {
  return getInstrType(MCInstD) == TPCII::TypeLOOP;
}

inline bool getHasImm(const MCInstrDesc &MCInstD) {
  return (MCInstD.TSFlags >> HasImmStart) & TPCII::HasImmMask;
}

inline bool getHasCompositeImm(const MCInstrDesc &MCInstD) {
  return (MCInstD.TSFlags >> HasCompositeImmStart) & TPCII::HasCompositeImmMask;
}

inline bool getIsPredicated(const MCInstrDesc &MCInstD) {
  return (MCInstD.TSFlags >> IsPredicatedStart) & TPCII::IsPredicatedMask;
}

inline bool getHasOutOfSlotData(const MCInstrDesc &MCInstD) {
  return (MCInstD.TSFlags >> OutOfSlotDataStart) & TPCII::OutOfSlotDataMask;
}

inline bool getHasImmField(const MCInstrDesc &MCInstD) {
  return (MCInstD.TSFlags >> HasImmFieldStart) & TPCII::HasImmFieldMask;
}

inline unsigned getImmStart(const MCInstrDesc &MCInstD) {
  return static_cast<unsigned>((MCInstD.TSFlags >> ImmStartBitStart) & TPCII::ImmStartBitMask);
}

inline unsigned getImmEnd(const MCInstrDesc &MCInstD) {
  return static_cast<unsigned>((MCInstD.TSFlags >> ImmEndBitStart) & TPCII::ImmEndBitMask);
}

inline unsigned getImmFieldOpNum(const MCInstrDesc &MCInstD) {
  return static_cast<unsigned>((MCInstD.TSFlags >> ImmOpNumStart) & TPCII::ImmOpNumMask);
}

inline unsigned getSlotOpCode(const MCInstrDesc &MCInstD) {
  return static_cast<unsigned>((MCInstD.TSFlags >> SlotOpCodeStart) & TPCII::SlotOpCodeMask);
}

inline bool getImmOpCount(const MCInstrDesc &MCInstD) {
  return (MCInstD.TSFlags >> ImmOpCountStart) & TPCII::ImmOpCountMask;
}

inline bool getSecondImmOp(const MCInstrDesc &MCInstD) {
  return (MCInstD.TSFlags >> SecondImmOpStart) & TPCII::SecondImmOpMask;
}

inline bool getThirdImmOp(const MCInstrDesc &MCInstD) {
  return (MCInstD.TSFlags >> ThirdImmOpStart) & TPCII::ThirdImmOpMask;
}

inline bool getFirstImmBits(const MCInstrDesc &MCInstD) {
  return (MCInstD.TSFlags >> FirstImmBitsStart) & TPCII::FirstImmBitsMask;
}

inline bool getSecondImmBits(const MCInstrDesc &MCInstD) {
  return (MCInstD.TSFlags >> SecondImmBitsStart) & TPCII::SecondImmBitsMask;
}

inline bool getThirdImmBits(const MCInstrDesc &MCInstD) {
  return (MCInstD.TSFlags >> ThirdImmBitsStart) & TPCII::ThirdImmBitsMask;
}

// TODO: Remove this.
inline OpType getOpType(const MCInstrDesc &MCInstD) {
  uint64_t Value = (MCInstD.TSFlags >> OpTypeCodeStart) & TPCII::OpTypeCodeMask;
  assert(Value <= OpType::Max);
  return static_cast<OpType>(Value);
}

inline unsigned getLanesCount(const MCInstrDesc &MCInstD) {
  return static_cast<unsigned>((MCInstD.TSFlags >> LanesStart) & TPCII::LanesMask);
}

inline bool getSrcCIsStoreSrcC(const MCInstrDesc &MCInstD) {
  return (MCInstD.TSFlags >> SrcCIsStoreSrcCStart) & TPCII::SrcCIsStoreSrcCMask;
}

inline bool getHasSrcC(const MCInstrDesc &MCInstD) {
  return (MCInstD.TSFlags >> HasSrcCStart) & TPCII::HasSrcCMask;
}

inline bool getHasSrcD(const MCInstrDesc &MCInstD) {
  return (MCInstD.TSFlags >> HasSrcDStart) & TPCII::HasSrcDMask;
}

// Instruction logical layout.
// *** Must match TPCInstrFormat*.td ***

// Load slot.
const unsigned LdSrcAStart      = 0;
const unsigned LdSrcAEnd        = 7;
const unsigned LdSrcASize       = LdSrcAEnd - LdSrcAStart + 1;

const unsigned LdDestStart      = 8;
const unsigned LdDestEnd        = 15;
const unsigned LdDestSize       = LdDestEnd - LdDestStart + 1;

const unsigned LdOpCodeStart    = 16;
const unsigned LdOpCodeEnd      = 20;
const unsigned LdOpCodeSize     = LdOpCodeEnd - LdOpCodeStart + 1;

const unsigned LdPolarityBit    = 21;

const unsigned LdPredicateStart = 22;
const unsigned LdPredicateEnd   = 25;
const unsigned LdPredicateSize  = LdPredicateEnd - LdPredicateStart + 1;

const unsigned LdVectorPredBit  = 26;

const unsigned LdSrcBStart      = 28;
const unsigned LdSrcBEnd        = 36;
const unsigned LdSrcBSize       = LdSrcBEnd - LdSrcBStart + 1;

const unsigned LdSwitchesStart  = 37;
const unsigned LdSwitchesEnd    = 43;
const unsigned LdSwitchesSize   = LdSwitchesEnd - LdSwitchesStart + 1;

// SPU slot.
const unsigned SpuOpCodeStart     = 0;
const unsigned SpuOpCodeEnd       = 5;
const unsigned SpuOpCodeSize      = SpuOpCodeEnd - SpuOpCodeStart + 1;

const unsigned SpuSrcAStart       = 6;
const unsigned SpuSrcAEnd         = 13;
const unsigned SpuSrcASize        = SpuSrcAEnd - SpuSrcAStart + 1;

const unsigned SpuSrcBStart       = 14;
const unsigned SpuSrcBEnd         = 21;
const unsigned SpuSrcBSize        = SpuSrcBEnd - SpuSrcBStart + 1;

const unsigned SpuDestStart       = 22;
const unsigned SpuDestEnd         = 29;
const unsigned SpuDestSize        = SpuDestEnd - SpuDestStart + 1;

const unsigned SpuOpTypeStart     = 30;
const unsigned SpuOpTypeEnd       = 33;
const unsigned SpuOpTypeSize      = SpuOpTypeEnd - SpuOpTypeStart + 1;

const unsigned SpuPolarityBit     = 34;

const unsigned SpuPredicateStart  = 35;
const unsigned SpuPredicateEnd    = 39;
const unsigned SpuPredicateSize   = SpuPredicateEnd - SpuPredicateStart + 1;

const unsigned SpuSwitchesStart   = 40;
const unsigned SpuSwitchesEnd     = 46;
const unsigned SpuSwitchesSize    = SpuSwitchesEnd - SpuSwitchesStart + 1;

// VPU slot.
const unsigned VpuOpCodeStart     = 0;
const unsigned VpuOpCodeEnd       = 5;
const unsigned VpuOpCodeSize      = VpuOpCodeEnd - VpuOpCodeStart + 1;

const unsigned VpuSrcAStart       = 6;
const unsigned VpuSrcAEnd         = 13;
const unsigned VpuSrcASize        = VpuSrcAEnd - VpuSrcAStart + 1;

const unsigned VpuSrcBStart       = 14;
const unsigned VpuSrcBEnd         = 21;
const unsigned VpuSrcBSize        = VpuSrcBEnd - VpuSrcBStart + 1;

const unsigned VpuSrcCStart       = 22;
const unsigned VpuSrcCEnd         = 29;
const unsigned VpuSrcCSize        = VpuSrcCEnd - VpuSrcCStart + 1;

const unsigned VpuSrcDStart       = 30;
const unsigned VpuSrcDEnd         = 38;
const unsigned VpuSrcDSize        = VpuSrcDEnd - VpuSrcDStart + 1;

const unsigned VpuDestStart       = 39;
const unsigned VpuDestEnd         = 46;
const unsigned VpuDestSize        = VpuDestEnd - VpuDestStart + 1;

const unsigned VpuSwitches1Start  = 47;
const unsigned VpuSwitches1End    = 49;
const unsigned VpuSwitches1Size   = VpuSwitches1End - VpuSwitches1Start + 1;

const unsigned VpuOpTypeStart     = 50;
const unsigned VpuOpTypeEnd       = 53;
const unsigned VpuOpTypeSize      = VpuOpTypeEnd - VpuOpTypeStart + 1;

const unsigned VpuPolarityBit     = 54;

const unsigned VpuPredicateStart  = 55;
const unsigned VpuPredicateEnd    = 58;
const unsigned VpuPredicateSize   = VpuPredicateEnd - VpuPredicateStart + 1;

const unsigned VpuVectorPredBit   = 59;

const unsigned VpuSwitches2Start  = 60;
const unsigned VpuSwitches2End    = 63;
const unsigned VpuSwitches2Size   = VpuSwitches2End - VpuSwitches2Start + 1;

// Store slot

const unsigned StSrcAStart        = 0;
const unsigned StSrcAEnd          = 7;
const unsigned StSrcASize         = StSrcAEnd - StSrcAStart + 1;

const unsigned StSrcBStart        = 8;
const unsigned StSrcBEnd          = 15;
const unsigned StSrcBSize         = StSrcBEnd - StSrcBStart + 1;

const unsigned StOpCodeStart      = 16;
const unsigned StOpCodeEnd        = 20;
const unsigned StOpCodeSize       = StOpCodeEnd - StOpCodeStart + 1;

const unsigned StPolarityBit      = 21;

const unsigned StPredicateStart   = 22;
const unsigned StPredicateEnd     = 25;
const unsigned StPredicateSize    = StPredicateEnd - StPredicateStart + 1;

const unsigned StVectorPredBit    = 26;

const unsigned StSrcCStart        = 28;
const unsigned StSrcCEnd          = 35;
const unsigned StSrcCSize         = StSrcCEnd - StSrcCStart + 1;

const unsigned StSwitchesStart    = 36;
const unsigned StSwitchesEnd      = 41;
const unsigned StSwitchesSize     = StSwitchesEnd - StSwitchesStart + 1;

const unsigned Gen4StSwitchesStart = 36;
const unsigned Gen4StSwitchesEnd   = 42;
const unsigned Gen4StSwitchesSize  = Gen4StSwitchesEnd - Gen4StSwitchesStart + 1;

// Instruction physic layout.

// SPU slot
const unsigned SPUStart = 0;
const unsigned SPUEnd = 42;
const unsigned SPUSize = SPUEnd - SPUStart + 1;

const unsigned Gen3SPUStart = 2;
const unsigned Gen3SPUEnd = 44;
const unsigned Gen3SPUSize = Gen3SPUEnd - Gen3SPUStart + 1;

const unsigned Gen3SPUPredicateStart  = 34;
const unsigned Gen3SPUPredicateEnd    = 37;
const unsigned Gen3SPUPredicateSize   = Gen3SPUPredicateEnd - Gen3SPUPredicateStart + 1;

const unsigned SPUOpCodeStart = 0;
const unsigned SPUOpCodeEnd = 5;
const unsigned SPUOpCodeSize = (SPUOpCodeEnd - SPUOpCodeStart + 1);

// VPU slot
const unsigned VPUStart = 43;
const unsigned VPUSrcCStart = 65;
const unsigned VPUSrcDStart = 73;
const unsigned VPUSrcCEnd = 72;
const unsigned VPUSrcDEnd = 81;
const unsigned VPUSrcCSize = VPUSrcCEnd - VPUSrcCStart + 1;
const unsigned VPUSrcDSize = VPUSrcDEnd - VPUSrcDStart + 1;
const unsigned VPUEnd = 102;
const unsigned VPUSize = VPUEnd - VPUStart + 1;

const unsigned Gen3VPUStart = 45;
const unsigned Gen3VPUSrcCStart = 224;
const unsigned Gen3VPUSrcCEnd = 231;
const unsigned Gen3VPUSrcDStart = 232;
const unsigned Gen3VPUSrcDEnd = 239;
const unsigned Gen3VPUSrcEStart = 240;
const unsigned Gen3VPUSrcEEnd = 247;
const unsigned Gen3VPUSrcCSize = Gen3VPUSrcCEnd - Gen3VPUSrcCStart + 1;
const unsigned Gen3VPUSrcDSize = Gen3VPUSrcDEnd - Gen3VPUSrcDStart + 1;
const unsigned Gen3VPUSrcESize = Gen3VPUSrcEEnd - Gen3VPUSrcEStart + 1;
const unsigned Gen3VPUEnd = 91; // was 88 in 0.52
const unsigned Gen3VPUSize = Gen3VPUEnd - Gen3VPUStart + 1;

// Load slot
const unsigned LDStart = 103;
const unsigned LDEnd = 150;
const unsigned LDSize = LDEnd - LDStart + 1;

const unsigned Gen3LDStart = 130;
const unsigned Gen3LDEnd = 172;
const unsigned Gen3LDSize = Gen3LDEnd - Gen3LDStart + 1;

// Store slot
const unsigned STStart = 124;
const unsigned STEnd = 150;
const unsigned STSize = STEnd - STStart + 1;

const unsigned Gen3STStart = 173;
const unsigned Gen3STEnd = 219; // was 216 in 0.52
const unsigned Gen3STSize = Gen3STEnd - Gen3STStart + 1;

// Immediate slot
const unsigned ImmStart = 151;
const unsigned ImmEnd = 182;
const unsigned ImmSize = ImmEnd - ImmStart + 1;

const unsigned Gen3ImmStart = 96;
const unsigned Gen3ImmEnd = 127;
const unsigned Gen3ImmSize = Gen3ImmEnd - Gen3ImmStart + 1;
const unsigned Gen3Imm1Start = 96;
const unsigned Gen3Imm1End = 127;
const unsigned Gen3Imm2Start = 224;
const unsigned Gen3Imm2End = 255;

// Extra fields
const unsigned LDSTPolarity = 145;
const unsigned LDSTPredicateStart = 146;
const unsigned LDSTPredicateEnd = 150;
const unsigned LDSTPredicateSize = LDSTPredicateEnd - LDSTPredicateStart + 1;

//
// LD/ST switches layout for Gen1 (Goya, aka Dali) and Gen2 (Gaudi)
//
const unsigned LDSwitchStart = 183;
const unsigned LDSwitchEnd = 186;
const unsigned LDSwitchSize = LDSwitchEnd - LDSwitchStart + 1;
const unsigned STSwitchStart = 187;
const unsigned STSwitchEnd = 190;
const unsigned STSwitchSize = STSwitchEnd - STSwitchStart + 1;

//
// Gen3 (Goya2) layout of LD/ST switches differs from Gen1 (Goya, aka Dali) and Gen2 (Gaudi)
// So let's differentiate them by adding Gen3 prefix.
//
const unsigned Gen3LDSwitchStart = 166;
const unsigned Gen3LDSwitchEnd = 171;
const unsigned Gen3LDSwitchSize = Gen3LDSwitchEnd - Gen3LDSwitchStart + 1;
const unsigned Gen3STSwitchStart = 202;
const unsigned Gen3STSwitchEnd = 207;
const unsigned Gen3STSwitchSize = Gen3STSwitchEnd - Gen3STSwitchStart + 1;


const unsigned InstructionSize = 256;

// Gen3 compressed format fields
const unsigned slot1Size    = 43;
const unsigned slot2Size    = 44;
const unsigned slot1Start  = 2;
const unsigned slot2Start  = 45;
const unsigned slot3Start  = 130;
const unsigned slot4Start  = 173;
const unsigned imm1Start   = 96;
const unsigned imm2Start   = 224;
const unsigned cmpr1Start  = 0;
const unsigned cmpr2Start  = 128;

// LOOP fields
const unsigned LoopStartImmStart = 151;
const unsigned LoopBoundaryImmStart = 36;
const unsigned LoopStepImmStart = 68;
const unsigned LoopOffsetStart = 100;

const unsigned Gen3LoopStartImmStart = 96;
const unsigned Gen3LoopBoundaryImmStart = 38;
const unsigned Gen3LoopStepImmStart = 128;
const unsigned Gen3LoopOffsetStart = 70;

const unsigned Gen4LoopStartImmStart = 96;
const unsigned Gen4LoopBoundaryImmStart = 42;
const unsigned Gen4LoopStepImmStart = 128;
const unsigned Gen4LoopOffsetStart = 74;


// Return whether insn is LOOKUP_C1C2 or LOOKUP_C0
inline bool isLookupC(const MCInstrDesc &MCInstD) {
  if (!isLoadInst(MCInstD))
    return false;
  unsigned opc = TPCII::getSlotOpCode(MCInstD);
  // Also LOOKUP_1C and LOOKUP_2C
  return (opc == TPCII::LOOKUP_C1C2) || (opc == TPCII::LOOKUP_C0);
}

inline bool isLookup(const MCInstrDesc &MCInstD) {
  if (!isLoadInst(MCInstD))
    return false;
  switch (getSlotOpCode(MCInstD)) {
  case LOOKUP_C1C2: // also LOOKUP_2C
  case LOOKUP_C0:   // also LOOKUP_1C
  case LOOKUP:
    return true;
  default:
    return false;
  }
}

inline bool isMac(const MCInstrDesc &MCInstD) {
  if (isVPUInst(MCInstD)) {
    return getSlotOpCode(MCInstD) == TPCII::vpuMAC;
  } else if (isSPUInst(MCInstD)) {
    return getSlotOpCode(MCInstD) == TPCII::spuMAC;
  } else {
    return false;
  }
}

inline bool isAdd(const MCInstrDesc &MCInstD) {
  if (isSPUInst(MCInstD))
    return getSlotOpCode(MCInstD) == spuADD;
  if (isVPUInst(MCInstD))
    return getSlotOpCode(MCInstD) == vpuADD;
  return false;
}

inline bool isSub(const MCInstrDesc &MCInstD) {
  if (isSPUInst(MCInstD))
    return getSlotOpCode(MCInstD) == spuSUB;
  if (isVPUInst(MCInstD))
    return getSlotOpCode(MCInstD) == vpuSUB;
  return false;
}

inline bool isMul(const MCInstrDesc &MCInstD) {
  if (isSPUInst(MCInstD))
    return getSlotOpCode(MCInstD) == spuMUL;
  if (isVPUInst(MCInstD))
    return getSlotOpCode(MCInstD) == vpuMUL;
  return false;
}

inline bool isMadd(const MCInstrDesc &MCInstD) {
  if (isVPUInst(MCInstD))
    return getSlotOpCode(MCInstD) == vpuMADD;

  return false;
}

inline bool isFMA(const MCInstrDesc &MCInstD) {
  return isMac(MCInstD) || isMul(MCInstD) ||
      isAdd(MCInstD) || isSub(MCInstD) || isMadd(MCInstD);
}

inline bool isVpuConvert(const MCInstrDesc &MCInstD) {
  if (!isVPUInst(MCInstD))
    return false;
  switch (getSlotOpCode(MCInstD)) {
  case vpuCONVERT:
  case vpuCONVERT_INT32:
  case vpuCONVERT_UINT32:
  case vpuCONVERT_INT16:
  case vpuCONVERT_UINT16:
    return true;
  default:
    return false;
  }
}

inline bool isSel2(const MCInstrDesc &MCInstD) {
  if (!isVPUInst(MCInstD))
    return false;
  switch (getSlotOpCode(MCInstD)) {
  case vpuSEL2_LESS:
  case vpuSEL2_LEQ:
  case vpuSEL2_GRT:
  case vpuSEL2_GEQ:
    return true;
  default:
    return false;
  }
}

inline bool isStoreForSlotOpcode(const MCInstrDesc &Desc,
                                 const std::uint64_t Slot) {
  return isStoreInst(Desc) && getSlotOpCode(Desc) == Slot;
}

inline bool isLoadForSlotOpcode(const MCInstrDesc &Desc,
                                const std::uint64_t Slot) {
  return isLoadInst(Desc) && getSlotOpCode(Desc) == Slot;
}

inline bool isEvent(const MCInstrDesc &MCInstD) {
  return isStoreForSlotOpcode(MCInstD, ST_EVENT) ||
         isLoadForSlotOpcode(MCInstD, LD_EVENT);
}

inline bool is_ld_l_v(const MCInstrDesc &MCInstD) {
  return isLoadForSlotOpcode(MCInstD, LD_L_V);
}

inline bool is_st_l_v(const MCInstrDesc &MCInstD) {
  return isStoreForSlotOpcode(MCInstD, ST_L_V);
}

inline bool is_st_l(const MCInstrDesc &MCInstD) {
  return isStoreForSlotOpcode(MCInstD, ST_L);
}

inline bool is_ld_l(const MCInstrDesc &MCInstD) {
  return isLoadForSlotOpcode(MCInstD, LD_L);
}

inline bool is_st_g(const MCInstrDesc &MCInstD) {
    return isStoreForSlotOpcode(MCInstD, ST_G);
}

inline bool is_ld_g(const MCInstrDesc &MCInstD) {
    return isLoadForSlotOpcode(MCInstD, LD_G);
}

inline bool is_ld_l_v_family(const MCInstrDesc &MCInstD) {
  if (!isLoadInst(MCInstD))
    return false;
  switch (getSlotOpCode(MCInstD)) {
  case LD_L_V:
  case LD_L_V_LOW:
  case LD_L_V_HIGH:
    return true;
  }
  return false;
}

inline bool is_st_l_v_family(const MCInstrDesc &MCInstD) {
  if (!isStoreInst(MCInstD))
    return false;
  switch (getSlotOpCode(MCInstD)) {
  case ST_L_V:
  case ST_L_V_LOW:
  case ST_L_V_HIGH:
    return true;
  }
  return false;
}

inline bool isLdTnsr(const MCInstrDesc &MCInstD, bool IsGen3Plus) {
  unsigned Opcode = getSlotOpCode(MCInstD);
  if (TPCII::isLoadInst(MCInstD) &&
      (Opcode == LD_TNSR ||
       Opcode == LD_TNSR_LOW ||
       Opcode == LD_TNSR_HIGH ||
       Opcode == LD_TNSR_CNVRT))
    return true;
  else if (IsGen3Plus && TPCII::isStoreInst(MCInstD) &&
           (Opcode == stLD_TNSR ||
            Opcode == stLD_TNSR_LOW ||
            Opcode == stLD_TNSR_HIGH ||
            Opcode == stLD_TNSR_CNVRT))
    return true;
  else
    return false;
}

inline bool isStTnsr(const MCInstrDesc &MCInstD) {
  if (!isStoreInst(MCInstD))
    return false;
  switch (getSlotOpCode(MCInstD)) {
  case ST_TNSR:
  case ST_TNSR_LOW:
  case ST_TNSR_HIGH:
  case ST_TNSR_S:
  case ST_TNSR_SQZ:
    return true;
  }
  return false;
}

inline bool isGenAddr(const MCInstrDesc &MCInstD) {
  if (isLoadInst(MCInstD))
    return getSlotOpCode(MCInstD) == TPCII::ldGEN_ADDR;
  if (isStoreInst(MCInstD))
    return getSlotOpCode(MCInstD) == TPCII::stGEN_ADDR;
  return false;
}

}


namespace TPCMCInstrInfo {

//
// Bundle support
//
size_t const bundleInstructionsOffset = 1;

// Returns a iterator range of instructions in this bundle
iterator_range<MCInst::const_iterator> bundleInstructions(MCInst const &MCI);

// Returns the number of instructions in the bundle
size_t bundleSize(MCInst const &MCI);

// Put the packet in to canonical form, compound, duplex, pad, and shuffle
//bool canonicalizePacket(MCInstrInfo const &MCII, MCSubtargetInfo const &STI,
//                        MCContext &Context, MCInst &MCB,
//                        TPCMCChecker *Checker);

MCInst createBundle();

//
// End of Bundle support
//

// Returns whether this MCInst is a wellformed bundle
bool isBundle(MCInst const &MCI);

MCInstrDesc const &getDesc(MCInstrInfo const &MCII, MCInst const &MCI);

// Return instruction name
StringRef const getName(MCInstrInfo const &MCII, MCInst const &MCI);

// Return the TPC ISA class for the insn.
TPCII::IType getType(MCInstrInfo const &MCII, MCInst const &MCI);

// Return whether insn has imm.
bool hasImm(MCInstrInfo const &MCII, MCInst const &MCI);

// Return whether insn has composite imm in TS flags
bool hasCompositeImm(MCInstrInfo const &MCII, MCInst const &MCI);

// Return whether insn is predicated
bool isPredicated(MCInstrInfo const &MCII, MCInst const &MCI);

bool hasOutOfSlotData(MCInstrInfo const &MCII, MCInst const &MCI);

// Return whether insn has a field in imm.
bool hasImmField(MCInstrInfo const &MCII, MCInst const &MCI);

// Return imm field start bit.
unsigned getImmFieldStart(MCInstrInfo const &MCII, MCInst const &MCI);

// Return imm field end bit.
unsigned getImmFieldEnd(MCInstrInfo const &MCII, MCInst const &MCI);

// Return operand number with imm field.
unsigned getImmFieldOpNum(MCInstrInfo const &MCII, MCInst const &MCI);

unsigned getImmOpCount(MCInstrInfo const &MCII, MCInst const &MCI);
unsigned getSecondImmOp(MCInstrInfo const &MCII, MCInst const &MCI);
unsigned getThirdImmOp(MCInstrInfo const &MCII, MCInst const &MCI);
unsigned getFirstImmBits(MCInstrInfo const &MCII, MCInst const &MCI);
unsigned getSecondImmBits(MCInstrInfo const &MCII, MCInst const &MCI);
unsigned getThirdImmBits(MCInstrInfo const &MCII, MCInst const &MCI);
unsigned getLanesCount(MCInstrInfo const &MCII, MCInst const &MCI);

TPCII::OpType getOpType(const MCInstrDesc &Desc, const MCInst &I);
TPCII::OpType getOpType(const MCInstrDesc &Desc, const MachineInstr &I);
bool isInstTypeSigned(const MCInstrDesc &Desc, const MCInst &I);
bool isInstTypeSigned(const MCInstrDesc &Desc, const MachineInstr &I);
bool useImmSlotForImm(const MCOperandInfo &IInfo, int64_t imm, bool isSigned);

std::tuple<int, bool> getPredicatePolarity(const MachineInstr &MI);
bool HasPredicate(const MCInst &MI, const MCInstrDesc &MD, bool isDoron1Plus);
bool hasPredicate(const MachineInstr &MI, bool isDoron1Plus);

} // namespace TPCMCInstrInfo

// Return whether insn is LOOKUP_C1C2 or LOOKUP_C0
bool isLookupC(MCInstrInfo const &MCII, MCInst const &MCI);
}

#endif // LLVM_LIB_TARGET_TPC_MCTARGETDESC_TPCMCINSTRINFO_H
