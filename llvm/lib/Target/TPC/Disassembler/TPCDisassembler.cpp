//===-- TPCDisassembler.cpp - Disassembler for x86 and x86_64 -------------===//
//
//===----------------------------------------------------------------------===//
//
// This file is part of the TPC Disassembler.
// It contains code to translate the data produced by the decoder into
// MCInsts.
//
//===----------------------------------------------------------------------===//

#include "TPC.h"
#include "TPCSubtarget.h"
#include "MCTargetDesc/TPCMCTargetDesc.h"
#include "MCTargetDesc/TPCInstrDecomposer.h"
#include "llvm-c/Disassembler.h"
#include "llvm/ADT/APInt.h"
#include "llvm/ADT/ArrayRef.h"
#include "llvm/ADT/SmallVector.h"
#include "llvm/ADT/Triple.h"
#include "llvm/BinaryFormat/ELF.h"
#include "llvm/MC/MCAsmInfo.h"
#include "llvm/MC/MCContext.h"
#include "llvm/MC/MCDisassembler/MCDisassembler.h"
#include "llvm/MC/MCDisassembler/MCRelocationInfo.h"
#include "llvm/MC/MCDisassembler/MCSymbolizer.h"
#include "llvm/MC/MCExpr.h"
#include "llvm/MC/MCFixedLenDisassembler.h"
#include "llvm/MC/MCInst.h"
#include "llvm/MC/MCInstPrinter.h"
#include "llvm/MC/MCInstrDesc.h"
#include "llvm/MC/MCInstrInfo.h"
#include "llvm/MC/MCInstrItineraries.h"
#include "llvm/MC/MCRegisterInfo.h"
#include "llvm/MC/MCSchedule.h"
#include "llvm/MC/MCSubtargetInfo.h"
#include "llvm/Support/Debug.h"
#include "llvm/Support/ErrorHandling.h"
#include "llvm/Support/FormattedStream.h"
#include "llvm/Support/TargetRegistry.h"
#include "llvm/Support/raw_ostream.h"

#include <algorithm>
#include <bitset>
#include <cassert>
#include <cstddef>
#include <cstdint>
#include <iostream>
#include <iterator>
#include <unordered_map>
#include <vector>


using namespace llvm;

#define DEBUG_TYPE "tpc-disassembler"

using DecodeStatus = llvm::MCDisassembler::DecodeStatus;

unsigned getSPUOpCode(uint64_t Inst) {
  Inst >>= TPCII::SPUOpCodeStart;
  Inst &= ((1 << TPCII::SPUOpCodeSize) - 1);
  return static_cast<unsigned>(Inst);
}

unsigned getVPUOpCode(uint64_t Inst) {
  Inst >>= TPCII::VpuOpCodeStart;
  Inst &= ((1 << TPCII::VpuOpCodeSize) - 1);
  return static_cast<unsigned>(Inst);
}

struct LoopExtraValues {
    uint64_t Address;
    bool     MayCompress;
    uint8_t  Comparison;
    uint16_t Offset;
    uint64_t Target;
    uint32_t Start; 
    uint32_t Boundary; 
    uint32_t Step; 
};

struct JmpExtraValues {
    uint64_t Address;
    bool     MayCompress;
    uint8_t  Predicate;
    uint8_t  Polarity;
    int32_t  Imm;
};

namespace llvm {

class MCInst;
class MCOperand;
class MCSubtargetInfo;
class Twine;

//===----------------------------------------------------------------------===//
// TPCDisassembler
//===----------------------------------------------------------------------===//

class TPCDisassembler : public MCDisassembler {
private:
  std::unique_ptr<MCInstrInfo const> const MCII;
  mutable uint32_t Immediate;
public:
  TPCDisassembler(const MCSubtargetInfo &STI, MCContext &Ctx,
                     MCInstrInfo const *MCII) :
    MCDisassembler(STI, Ctx), MCII(MCII) {}

  ~TPCDisassembler() override = default;

  uint32_t getImmediate() const { return Immediate; }
  DecodeStatus getInstruction(MCInst &MI, uint64_t &Size,
                              ArrayRef<uint8_t> Bytes, uint64_t Address,
                              raw_ostream &CStream) const override;
private:
  DecodeStatus tryDecodeLoopInstruction(MCInst &instr, uint64_t &size,
                              ArrayRef<uint8_t> Bytes, uint64_t Address,
                              raw_ostream &vStream,
                              raw_ostream &cStream) const;

  DecodeStatus readLoopInstruction(ArrayRef<uint8_t> Bytes,
                                   uint64_t &Size,
                                   std::bitset<256> &Bundle,
                                   uint64_t &InsnLoop,
                                   LoopExtraValues &Extra,
                                   bool &IsPredDoron1) const;
};

//===----------------------------------------------------------------------===//
// TPCSymbolizer
//===----------------------------------------------------------------------===//

class TPCSymbolizer : public MCSymbolizer {
private:
  void *DisInfo;

public:
  TPCSymbolizer(MCContext &Ctx, std::unique_ptr<MCRelocationInfo> &&RelInfo,
                   void *disInfo)
                   : MCSymbolizer(Ctx, std::move(RelInfo)), DisInfo(disInfo) {}

  bool tryAddingSymbolicOperand(MCInst &Inst, raw_ostream &cStream,
                                       int64_t Value, uint64_t Address,
                                       bool IsBranch, uint64_t Offset,
                                       uint64_t InstSize) override;
  void tryAddingPcLoadReferenceComment(raw_ostream &cStream,
                                       int64_t Value,
                                       uint64_t Address) override;
};

} // end namespace llvm


static MCDisassembler::DecodeStatus decodeMovLd(MCInst &Inst, uint64_t insn,
                                                uint64_t Address, const void *Decoder);
static MCDisassembler::DecodeStatus decodeMovSpu(MCInst &Inst, uint64_t insn,
                                                 uint64_t Address, const void *Decoder);
static MCDisassembler::DecodeStatus decodeMovVpu(MCInst &Inst, uint64_t insn,
                                                 uint64_t Address, const void *Decoder);
static MCDisassembler::DecodeStatus decodeConvertScalar(MCInst &Inst,
                                                           uint64_t insn,
                                                           uint64_t Address,
                                                           const void *Decoder);
static MCDisassembler::DecodeStatus decodeConvertIntScalar(MCInst &Inst, uint64_t insn,
                                                     uint64_t Address, const void *Decoder);
static MCDisassembler::DecodeStatus decodeConvertIntVector(MCInst &Inst, uint64_t insn,
                                                     uint64_t Address, const void *Decoder);
static MCDisassembler::DecodeStatus decodeAdd(MCInst &Inst, uint64_t insn,
                                              uint64_t Address, const void *Decoder);
static MCDisassembler::DecodeStatus decodeSub(MCInst &Inst, uint64_t insn,
                                              uint64_t Address, const void *Decoder);
static MCDisassembler::DecodeStatus decodeLdTnsr(MCInst &Inst, uint64_t insn,
                                                 uint64_t Address,
                                                 const void *Decoder);
static MCDisassembler::DecodeStatus decodeLdTnsrSt(MCInst &Inst, uint64_t insn,
                                                   uint64_t Address,
                                                   const void *Decoder);
static MCDisassembler::DecodeStatus decodeStTnsr(MCInst &Inst, uint64_t insn,
                                                 uint64_t Address, const void *Decoder);
static MCDisassembler::DecodeStatus decodeStTnsrSqz(MCInst &Inst, uint64_t insn,
                                                    uint64_t Address, const void *Decoder);
static MCDisassembler::DecodeStatus decodeStTnsrS(MCInst &Inst, uint64_t insn,
                                                  uint64_t Address, const void *Decoder);
static MCDisassembler::DecodeStatus decodeMovGroup(MCInst &Inst, uint64_t insn,
                                                   uint64_t Address,
                                                   const void *Decoder);
static MCDisassembler::DecodeStatus decodeMovDualGroup(MCInst &Inst,
                                                       uint64_t insn,
                                        uint64_t Address, const void *Decoder);
static MCDisassembler::DecodeStatus decodeUdivAll(MCInst &Inst, uint64_t insn,
                                                    uint64_t Address, const void *Decoder);
static MCDisassembler::DecodeStatus decodeLookupLutPtr(MCInst &Inst, uint64_t insn,
                                                       uint64_t Address, const void *Decoder);
static MCDisassembler::DecodeStatus decodeMacZp(MCInst &Inst, uint64_t insn,
                                                   uint64_t Address, const void *Decoder);
static MCDisassembler::DecodeStatus decodeMacMulX2(MCInst &Inst, uint64_t insn,
                                                   uint64_t Address,
                                                   const void *Decoder);
static MCDisassembler::DecodeStatus decodeMaddZp(MCInst &Inst, uint64_t insn,
                                                uint64_t Address, const void *Decoder);
static MCDisassembler::DecodeStatus decodeNearbyint(MCInst &Inst, uint64_t insn,
                                                    uint64_t Address, const void *Decoder);
static MCDisassembler::DecodeStatus decodeLD_G(MCInst &Inst, uint64_t insn,
                                               uint64_t Address, const void *Decoder);
static MCDisassembler::DecodeStatus decodeFclass(MCInst &Inst, uint64_t insn,
                                                 uint64_t Address, const void *Decoder);

static MCDisassembler::DecodeStatus decodeMaddX2(MCInst &Inst, uint64_t insn,
                                                 uint64_t Address, const void *Decoder);

static MCDisassembler::DecodeStatus decodeStoreInstPredAddr(MCInst &Inst, uint64_t insn,
                                                            uint64_t Address,
                                                            const void *Decoder);

static const unsigned RegisterTableSPU[] = {
  // 0-35 - SRF
  TPC::S0,  TPC::S1,  TPC::S2,   TPC::S3,   TPC::S4,   TPC::S5,   TPC::S6,   TPC::S7,
  TPC::S8,  TPC::S9,  TPC::S10,  TPC::S11,  TPC::S12,  TPC::S13,  TPC::S14,  TPC::S15,
  TPC::S16, TPC::S17, TPC::S18,  TPC::S19,  TPC::S20,  TPC::S21,  TPC::S22,  TPC::S23,
  TPC::S24, TPC::S25, TPC::S26,  TPC::S27,  TPC::S28,  TPC::S29,  TPC::S30,  TPC::S31,
  TPC::S32, TPC::S33, TPC::S34,  TPC::S35,  ~0U,       ~0U,       ~0U,       ~0U,
  TPC::S_LFSR, TPC::S_LFSR_NO_CHANGE, ~0U, ~0U, ~0U,   ~0U,       ~0U,       ~0U,
  TPC::SP0, TPC::SP1, TPC::SP2,  TPC::SP3,  TPC::SP4,  TPC::SP5,  TPC::SP6,  TPC::SP7,
  TPC::SP8, TPC::SP9, TPC::SP10, TPC::SP11, TPC::SP12, TPC::SP13, TPC::SP14, TPC::SP15,
  TPC::I0,  TPC::I1,  TPC::I2,   TPC::I3,   TPC::I4,   TPC::I5,   TPC::I6,   TPC::I7,
  TPC::I8,  TPC::I9,  TPC::I10,  TPC::I11,  TPC::I12,  TPC::I13,  TPC::I14,  TPC::I15,
  TPC::I16, TPC::I17, TPC::I18,  TPC::I19,  TPC::I20,  TPC::I21,  TPC::I22,  TPC::I23,
  TPC::I24, TPC::I25, TPC::I26,  TPC::I27,  TPC::I28,  TPC::I29,  TPC::I30,  TPC::I31,
  TPC::AD0, TPC::AD1, TPC::AD2,  TPC::AD3,  TPC::AD4,  TPC::AD5,  TPC::AD6,  TPC::AD7,
};

static const unsigned RegisterTableVPU[] = {
  // 0-44 - VRF
  TPC::V0,  TPC::V1,  TPC::V2,   TPC::V3,   TPC::V4,   TPC::V5,   TPC::V6,   TPC::V7,
  TPC::V8,  TPC::V9,  TPC::V10,  TPC::V11,  TPC::V12,  TPC::V13,  TPC::V14,  TPC::V15,
  TPC::V16, TPC::V17, TPC::V18,  TPC::V19,  TPC::V20,  TPC::V21,  TPC::V22,  TPC::V23,
  TPC::V24, TPC::V25, TPC::V26,  TPC::V27,  TPC::V28,  TPC::V29,  TPC::V30,  TPC::V31,
  TPC::V32, TPC::V33, TPC::V34,  TPC::V35,  TPC::V36,  TPC::V37,  TPC::V38,  TPC::V39,
  TPC::LFSR, TPC::LFSR_NO_CHANGE, TPC::V_LANE_ID_32,  TPC::V_LANE_ID_16,  TPC::V_LANE_ID_8,
  // 45-63 - Reserved
  ~0U, ~0U, ~0U,
  ~0U, ~0U, ~0U, ~0U, ~0U, ~0U, ~0U, ~0U,
  ~0U, ~0U, ~0U, ~0U, ~0U, ~0U, ~0U, ~0U,
  // 64-99 - SRF
  TPC::S0,  TPC::S1,  TPC::S2,   TPC::S3,   TPC::S4,   TPC::S5,   TPC::S6,   TPC::S7,
  TPC::S8,  TPC::S9,  TPC::S10,  TPC::S11,  TPC::S12,  TPC::S13,  TPC::S14,  TPC::S15,
  TPC::S16, TPC::S17, TPC::S18,  TPC::S19,  TPC::S20,  TPC::S21,  TPC::S22,  TPC::S23,
  TPC::S24, TPC::S25, TPC::S26,  TPC::S27,  TPC::S28,  TPC::S29,  TPC::S30,  TPC::S31,
  TPC::S32, TPC::S33, TPC::S34,  TPC::S35,
  // 100-126 - Reserved
  ~0U, ~0U, ~0U, ~0U,
  TPC::S_LFSR, TPC::S_LFSR_NO_CHANGE, ~0U, ~0U, ~0U, ~0U, ~0U, ~0U,
  ~0U, ~0U, ~0U, ~0U, ~0U, ~0U, ~0U, ~0U,
  ~0U, ~0U, ~0U, ~0U, ~0U, ~0U, ~0U,
  // 127 - Immediate
  ~0U,
  // 128-159 - IRF
  TPC::I0,  TPC::I1,  TPC::I2,   TPC::I3,   TPC::I4,   TPC::I5,   TPC::I6,   TPC::I7,
  TPC::I8,  TPC::I9,  TPC::I10,  TPC::I11,  TPC::I12,  TPC::I13,  TPC::I14,  TPC::I15,
  TPC::I16, TPC::I17, TPC::I18,  TPC::I19,  TPC::I20,  TPC::I21,  TPC::I22,  TPC::I23,
  TPC::I24, TPC::I25, TPC::I26,  TPC::I27,  TPC::I28,  TPC::I29,  TPC::I30,  TPC::I31,
  // 160-167 - ADRF
  TPC::AD0, TPC::AD1, TPC::AD2,  TPC::AD3,  TPC::AD4,  TPC::AD5,  TPC::AD6,  TPC::AD7,
  // 168-223 - Reserved
  ~0U, ~0U, ~0U, ~0U, ~0U, ~0U, ~0U, ~0U,
  ~0U, ~0U, ~0U, ~0U, ~0U, ~0U, ~0U, ~0U,
  ~0U, ~0U, ~0U, ~0U, ~0U, ~0U, ~0U, ~0U,
  ~0U, ~0U, ~0U, ~0U, ~0U, ~0U, ~0U, ~0U,
  ~0U, ~0U, ~0U, ~0U, ~0U, ~0U, ~0U, ~0U,
  ~0U, ~0U, ~0U, ~0U, ~0U, ~0U, ~0U, ~0U,
  ~0U, ~0U, ~0U, ~0U, ~0U, ~0U, ~0U, ~0U,
  // 224-239 - SPRF
  TPC::SP0,  TPC::SP1,  TPC::SP2,   TPC::SP3,   TPC::SP4,   TPC::SP5,   TPC::SP6,   TPC::SP7,
  TPC::SP8,  TPC::SP9,  TPC::SP10,  TPC::SP11,  TPC::SP12,  TPC::SP13,  TPC::SP14,  TPC::SP15,
  // 240-255 - VPRF
  TPC::VP0,  TPC::VP1,  TPC::VP2,   TPC::VP3,   TPC::VP4,   TPC::VP5,   TPC::VP6,   TPC::VP7,
  TPC::VP8,  TPC::VP9,  TPC::VP10,  TPC::VP11,  TPC::VP12,  TPC::VP13,  TPC::VP14,  TPC::VP15
};

static const unsigned RegisterTableUnified[] = {
  // 0-44 - VRF
  TPC::V0,  TPC::V1,  TPC::V2,   TPC::V3,   TPC::V4,   TPC::V5,   TPC::V6,   TPC::V7,
  TPC::V8,  TPC::V9,  TPC::V10,  TPC::V11,  TPC::V12,  TPC::V13,  TPC::V14,  TPC::V15,
  TPC::V16, TPC::V17, TPC::V18,  TPC::V19,  TPC::V20,  TPC::V21,  TPC::V22,  TPC::V23,
  TPC::V24, TPC::V25, TPC::V26,  TPC::V27,  TPC::V28,  TPC::V29,  TPC::V30,  TPC::V31,
  TPC::V32, TPC::V33, TPC::V34,  TPC::V35,  TPC::V36,  TPC::V37,  TPC::V38,  TPC::V39,
  TPC::LFSR, TPC::LFSR_NO_CHANGE, TPC::V_LANE_ID_32,  TPC::V_LANE_ID_16,  TPC::V_LANE_ID_8,
  // 45-47 - Reserved
  ~0U, ~0U, ~0U,
  // 48-63 - VPRF
  TPC::VP0,  TPC::VP1,  TPC::VP2,   TPC::VP3,   TPC::VP4,   TPC::VP5,   TPC::VP6,   TPC::VP7,
  TPC::VP8,  TPC::VP9,  TPC::VP10,  TPC::VP11,  TPC::VP12,  TPC::VP13,  TPC::VP14,  TPC::VP15,
  // 64-127 - SRF
  TPC::S0,  TPC::S1,  TPC::S2,   TPC::S3,   TPC::S4,   TPC::S5,   TPC::S6,   TPC::S7,
  TPC::S8,  TPC::S9,  TPC::S10,  TPC::S11,  TPC::S12,  TPC::S13,  TPC::S14,  TPC::S15,
  TPC::S16, TPC::S17, TPC::S18,  TPC::S19,  TPC::S20,  TPC::S21,  TPC::S22,  TPC::S23,
  TPC::S24, TPC::S25, TPC::S26,  TPC::S27,  TPC::S28,  TPC::S29,  TPC::S30,  TPC::S31,
  TPC::S32, TPC::S33, TPC::S34,  TPC::S35,  TPC::S36,  TPC::S37,  TPC::S38,  TPC::S39,
  TPC::S40, TPC::S41, TPC::S42,  TPC::S43,  TPC::S44,  TPC::S45,  TPC::S46,  TPC::S47,
  TPC::S48, TPC::S49, TPC::S50,  TPC::S51,  TPC::S52,  TPC::S53,  TPC::S54,  TPC::S55,
  TPC::S56, TPC::S57, TPC::S58,  TPC::S59,  TPC::S60,  TPC::S61,  TPC::S62,  TPC::S63,
  // 128-191 - IRF
  TPC::I0,  TPC::I1,  TPC::I2,   TPC::I3,   TPC::I4,   TPC::I5,   TPC::I6,   TPC::I7,
  TPC::I8,  TPC::I9,  TPC::I10,  TPC::I11,  TPC::I12,  TPC::I13,  TPC::I14,  TPC::I15,
  TPC::I16, TPC::I17, TPC::I18,  TPC::I19,  TPC::I20,  TPC::I21,  TPC::I22,  TPC::I23,
  TPC::I24, TPC::I25, TPC::I26,  TPC::I27,  TPC::I28,  TPC::I29,  TPC::I30,  TPC::I31,
  TPC::I32, TPC::I33, TPC::I34,  TPC::I35,  TPC::I36,  TPC::I37,  TPC::I38,  TPC::I39,
  TPC::I40, TPC::I41, TPC::I42,  TPC::I43,  TPC::I44,  TPC::I45,  TPC::I46,  TPC::I47,
  TPC::I48, TPC::I49, TPC::I50,  TPC::I51,  TPC::I52,  TPC::I53,  TPC::I54,  TPC::I55,
  TPC::I56, TPC::I57, TPC::I58,  TPC::I59,  TPC::I60,  TPC::I61,  TPC::I62,  TPC::I63,
  // 192-207 - SPRF
  TPC::SP0,  TPC::SP1,  TPC::SP2,   TPC::SP3,   TPC::SP4,   TPC::SP5,   TPC::SP6,   TPC::SP7,
  TPC::SP8,  TPC::SP9,  TPC::SP10,  TPC::SP11,  TPC::SP12,  TPC::SP13,  TPC::SP14,  TPC::SP15,
  // 208-223 - ADRF
  TPC::AD0, TPC::AD1, TPC::AD2,  TPC::AD3,  TPC::AD4,  TPC::AD5,  TPC::AD6,  TPC::AD7,
  TPC::AD8, TPC::AD9, TPC::AD10, TPC::AD11, TPC::AD12, TPC::AD13, TPC::AD14, TPC::AD15,
  // 224 - Context Switch
  ~0U,
  // 225-238 - Reserved
  ~0U, ~0U, ~0U, ~0U, ~0U, ~0U, ~0U, ~0U,
  ~0U, ~0U, ~0U, ~0U, ~0U, ~0U,
  // 239-254 - Small IMm
  ~0U, ~0U, ~0U, ~0U, ~0U, ~0U, ~0U, ~0U,
  ~0U, ~0U, ~0U, ~0U, ~0U, ~0U, ~0U, ~0U,
  // 255 - Immediate
  ~0U
};

static const unsigned PredicateTable[] = {
  TPC::SP0, TPC::SP1 ,TPC::SP2,  TPC::SP3,  TPC::SP4,  TPC::SP5,  TPC::SP6,  TPC::SP7,
  TPC::SP8, TPC::SP9, TPC::SP10, TPC::SP11, TPC::SP12, TPC::SP13, TPC::SP14, TPC::SP15,
  TPC::VP0, TPC::VP1, TPC::VP2,  TPC::VP3,  TPC::VP4,  TPC::VP5,  TPC::VP6,  TPC::VP7,
  TPC::VP8, TPC::VP9, TPC::VP10, TPC::VP11, TPC::VP12, TPC::VP13, TPC::VP14, TPC::VP15
};

static bool isVRF(unsigned Field) {
  return Field <= 44;
}

static bool isVPRF(unsigned Field, bool UnifiedEncoding) {
  if (UnifiedEncoding) {
    return Field >= 48 && Field <= 63;
  } else {
    return Field >=240 && Field <=255;
  }
}

static bool isSPRF(unsigned Field, bool IsSPU, bool UnifiedEncoding) {
  if (UnifiedEncoding)
    return Field >= 192 && Field <= 207;
  else if (IsSPU)
    return Field >= 48 && Field <= 63;
  else
    return Field >= 224 && Field <= 239;
}

static bool isSRF(unsigned Field, bool IsSPU, bool UnifiedEncoding) {
  if (UnifiedEncoding) {
    return (Field >= 64 && Field <= 127) || Field == 40;
  } else if (IsSPU) {
    // LFSR is SRF register.
    return (Field <= 35 || Field == 40);
  } else {
    return ((Field >= 64 && Field <= 99) || Field == 104 || Field == 105);
  }
}

static bool isIRF(unsigned Field, bool IsSPU, bool UnifiedEncoding) {
  if (UnifiedEncoding)
    return Field >= 128 && Field <= 191;
  else if (IsSPU)
    return Field >= 64 && Field <= 95;
  else
    return Field >= 128 && Field <= 159;
}

static bool isADRF(unsigned Field, bool IsSPU, bool UnifiedEncoding) {
  if (UnifiedEncoding)
    return Field >= 208 && Field <= 223;
  else if (IsSPU)
    return Field >= 96 && Field <= 103;
  else
    return Field >= 160 && Field <= 167;
}

static bool isLongImmediate(unsigned Field, bool UnifiedEncoding) {
  if (UnifiedEncoding)
    return Field == 255;
  else
    return Field == 127;
}

static bool isShortImmediate(unsigned Field, bool UnifiedEncoding) {
  if (UnifiedEncoding)
    return Field >= 239 && Field <= 254;
  else
    return Field >= 111 && Field <= 126;
}

static unsigned getShortImmediate(unsigned Field, bool UnifiedEncoding) {
  if (UnifiedEncoding) {
    return Field == 239 ? 0x0f
                        : (Field - 240);
  } else {
    return Field == 111 ? 0x0f
                        : (Field - 112);
  }
}

// tryAddingSymbolicOperand - trys to add a symbolic operand in place of the
/// immediate Value in the MCInst.  The immediate Value has had any PC
/// adjustment made by the caller.  If the instruction is a branch instruction
/// then isBranch is true, else false.  If the getOpInfo() function was set as
/// part of the setupForSymbolicDisassembly() call then that function is called
/// to get any symbolic information at the Address for this instruction.  If
/// that returns non-zero then the symbolic information it returns is used to
/// create an MCExpr and that is added as an operand to the MCInst.  If
/// getOpInfo() returns zero and isBranch is true then a symbol look up for
/// Value is done and if a symbol is found an MCExpr is created with that, else
/// an MCExpr with Value is created.  This function returns true if it adds an
/// operand to the MCInst and false otherwise.
static bool tryAddingSymbolicOperand(uint64_t Address, int32_t Value,
                                     bool isBranch, uint64_t InstSize,
                                     MCInst &MI, const void *Decoder) {
  const MCDisassembler *Dis = static_cast<const MCDisassembler*>(Decoder);
  // FIXME: Does it make sense for value to be negative?
  bool result = Dis->tryAddingSymbolicOperand(MI, (uint32_t)Value, Address, isBranch, /* Offset */ 0, InstSize);
  return result;
}

static DecodeStatus decodeJmpTargetImm(MCInst &Inst, unsigned Val,
                                           uint64_t Addr,
                                           const void *Decoder) {
  const JmpExtraValues *extra = (reinterpret_cast<const JmpExtraValues*>(Addr));
  uint64_t address = extra->Address + extra->Imm;
  if (!tryAddingSymbolicOperand(address, 0, true, 0, Inst, Decoder)) {
    Inst.addOperand(MCOperand::createImm(extra->Imm));
  }

  unsigned Register = PredicateTable[extra->Predicate];
  Inst.addOperand(MCOperand::createReg(Register));
  Inst.addOperand(MCOperand::createImm(extra->Polarity));
  return MCDisassembler::Success;
}

static DecodeStatus decodeLoopStartImm(MCInst &Inst, unsigned Val,
                                           uint64_t Addr,
                                           const void *Decoder) {
  const LoopExtraValues *extra = (reinterpret_cast<const LoopExtraValues*>(Addr));
  Inst.addOperand(MCOperand::createImm(extra->Start));
  return MCDisassembler::Success;
}

static DecodeStatus decodeLoopBoundaryImm(MCInst &Inst, unsigned Val,
                                           uint64_t Addr,
                                           const void *Decoder) {
  const LoopExtraValues *extra = (reinterpret_cast<const LoopExtraValues*>(Addr));
  Inst.addOperand(MCOperand::createImm(extra->Boundary));
  return MCDisassembler::Success;
}

static DecodeStatus decodeLoopStepImm(MCInst &Inst, unsigned Val,
                                           uint64_t Addr,
                                           const void *Decoder) {
  const LoopExtraValues *extra = (reinterpret_cast<const LoopExtraValues*>(Addr));
  Inst.addOperand(MCOperand::createImm(extra->Step));
  return MCDisassembler::Success;
}

static DecodeStatus decodeLoopComparison(MCInst &Inst, unsigned Val,
                                           uint64_t Addr,
                                           const void *Decoder) {
  const LoopExtraValues *extra = (reinterpret_cast<const LoopExtraValues*>(Addr));
  uint64_t address = extra->Address + extra->Offset;

  // We need to fix the address for LOOP instruction because the label
  // always points to the instruction next to END_PC_OFFSET encoded in instruction
  address += 32;

  Inst.addOperand(MCOperand::createImm(extra->Comparison));
  if (!tryAddingSymbolicOperand(address, 0, true, 0, Inst, Decoder)) {
    Inst.addOperand(MCOperand::createImm(extra->Offset));
  }

  return MCDisassembler::Success;
}

static bool DecodeImmediate(MCInst &Inst, unsigned FieldVal,
                            uint64_t Addr,
                            const void *Decoder) {
  auto *Disasm = static_cast<const TPCDisassembler *>(Decoder);
  const FeatureBitset &Bits = Disasm->getSubtargetInfo().getFeatureBits();
  bool IsDoron1 = (bool)Bits[TPC::FeatureDoron1];

  bool IsLongImm = IsDoron1 ? (FieldVal == 255) : (FieldVal == 127);
  if (IsLongImm) {
    // This is a long immediate, encoded in separate field.
    auto *Disasm = static_cast<const TPCDisassembler *>(Decoder);
    Inst.addOperand(MCOperand::createImm(Disasm->getImmediate()));
    return true;
  }

  if (isShortImmediate(FieldVal, IsDoron1)) {
    Inst.addOperand(MCOperand::createImm(getShortImmediate(FieldVal, IsDoron1)));
    return true;
  }
  return false;
}

static DecodeStatus DecodeSRFRegisterClass(MCInst &Inst, unsigned RegNo,
                                           uint64_t Addr,
                                           const void *Decoder) {
  if (DecodeImmediate(Inst, RegNo, Addr, Decoder))
    return MCDisassembler::Success;
  auto *Disasm = static_cast<const TPCDisassembler *>(Decoder);
  bool UnifiedEncoding = Disasm->getSubtargetInfo().hasFeature(TPC::FeatureDoron1);

  bool VPUEncoding =
    (Inst.getFlags() & TPCII::MCSlotFlagMask) != TPCII::MCFlagSPU &&
    (Inst.getFlags() & TPCII::MCSlotFlagMask) != TPCII::MCFlagLOOP;

  // Decode all registers, that can be found in SrcA/B/Dest in scalar slot, not
  // only SRF.
  unsigned Register;
  if (UnifiedEncoding) {
    if ((Inst.getFlags() & TPCII::MCSlotFlagMask) == TPCII::MCFlagLOOP)
      RegNo += 64;
    Register = RegisterTableUnified[RegNo];
  } else if (VPUEncoding)
    Register = RegisterTableVPU[RegNo];
  else
    Register = RegisterTableSPU[RegNo];
  if (Register ==  ~0U)
    return MCDisassembler::Fail;
  Inst.addOperand(MCOperand::createReg(Register));
  return MCDisassembler::Success;
}

static DecodeStatus DecodeIRFRegisterClass(MCInst &Inst, unsigned RegNo,
                                           uint64_t Addr,
                                           const void *Decoder) {
  if (DecodeImmediate(Inst, RegNo, Addr, Decoder))
    return MCDisassembler::Success;
  auto *Disasm = static_cast<const TPCDisassembler *>(Decoder);
  const FeatureBitset &Bits = Disasm->getSubtargetInfo().getFeatureBits();
  bool UnifiedEncoding = (bool) Bits[TPC::FeatureDoron1];

  bool VPUEncoding = (Inst.getFlags() & TPCII::MCSlotFlagMask) !=
      TPCII::MCFlagSPU;

  // Decode all registers, that can be found in SrcA/B/Dest in scalar slot, not
  // only SRF.
  unsigned Register;
  if (UnifiedEncoding)
    Register = RegisterTableUnified[RegNo];
  else if (VPUEncoding)
    Register = RegisterTableVPU[RegNo];
  else
    Register = RegisterTableSPU[RegNo];
  if (Register == ~0U)
    return MCDisassembler::Fail;
  Inst.addOperand(MCOperand::createReg(Register));
  return MCDisassembler::Success;
}

static DecodeStatus DecodeADRFRegisterClass(MCInst &Inst, unsigned RegNo,
                                           uint64_t Addr,
                                           const void *Decoder) {
  auto *Disasm = static_cast<const TPCDisassembler *>(Decoder);
  const FeatureBitset &Bits = Disasm->getSubtargetInfo().getFeatureBits();
  bool UnifiedEncoding = (bool) Bits[TPC::FeatureDoron1];

  bool VPUEncoding = (Inst.getFlags() & TPCII::MCSlotFlagMask) !=
      TPCII::MCFlagSPU;
  unsigned Register;
  if (UnifiedEncoding) {
    if (RegNo > 223 || RegNo < 208)
      return MCDisassembler::Fail;
    Register = RegisterTableUnified[RegNo];
  }
  else if (VPUEncoding) {
    if (RegNo > 167 || RegNo < 160)
      return MCDisassembler::Fail;
    Register = RegisterTableVPU[RegNo];
  } else {
    if (RegNo >= 103 || RegNo < 96)
      return MCDisassembler::Fail;
    Register = RegisterTableSPU[RegNo];
  }

  if (Register == ~0U)
    return MCDisassembler::Fail;
  Inst.addOperand(MCOperand::createReg(Register));
  return MCDisassembler::Success;
}

static DecodeStatus DecodeSPRFRegisterClass(MCInst &Inst, unsigned RegNo,
                                            uint64_t Addr,
                                            const void *Decoder) {
  auto *Disasm = static_cast<const TPCDisassembler *>(Decoder);
  const FeatureBitset &Bits = Disasm->getSubtargetInfo().getFeatureBits();
  bool UnifiedEncoding = (bool) Bits[TPC::FeatureDoron1];
  
  bool VPUEncoding = (Inst.getFlags() & TPCII::MCSlotFlagMask) !=
      TPCII::MCFlagSPU;
  unsigned Register;
  if (UnifiedEncoding) {
    if (RegNo < 192 || RegNo > 207)
      return MCDisassembler::Fail;
    Register = RegisterTableUnified[RegNo];
  }
  else if (VPUEncoding) {
    if (RegNo < 224 || RegNo > 239)
      return MCDisassembler::Fail;
    Register = RegisterTableVPU[RegNo];
  } else {
    if (RegNo < 48 || RegNo > 63)
      return MCDisassembler::Fail;
    Register = RegisterTableSPU[RegNo];
  }

  if (Register == ~0U)
    return MCDisassembler::Fail;
  Inst.addOperand(MCOperand::createReg(Register));
  return MCDisassembler::Success;
}

static DecodeStatus DecodeVRFRegisterClass(MCInst &Inst, unsigned RegNo,
                                           uint64_t Addr,
                                           const void *Decoder) {
  if (DecodeImmediate(Inst, RegNo, Addr, Decoder))
    return MCDisassembler::Success;
  auto *Disasm = static_cast<const TPCDisassembler *>(Decoder);
  const FeatureBitset &Bits = Disasm->getSubtargetInfo().getFeatureBits();
  bool UnifiedEncoding = (bool) Bits[TPC::FeatureDoron1];

  // Decode all registers, that can be found in VPU, LD, ST slots, not
  // only VRF.
  unsigned Register = ~0U;
  if (UnifiedEncoding)
    Register = RegisterTableUnified[RegNo];
  else
    Register = RegisterTableVPU[RegNo];
  if (Register == ~0U)
    return MCDisassembler::Fail;
  Inst.addOperand(MCOperand::createReg(Register));
  return MCDisassembler::Success;
}

static DecodeStatus DecodeVPRFRegisterClass(MCInst &Inst, unsigned RegNo,
                                           uint64_t Addr,
                                           const void *Decoder) {
  auto *Disasm = static_cast<const TPCDisassembler *>(Decoder);
  const FeatureBitset &Bits = Disasm->getSubtargetInfo().getFeatureBits();
  bool UnifiedEncoding = (bool) Bits[TPC::FeatureDoron1];

  unsigned Register = ~0U;
  if (UnifiedEncoding) {
    if (RegNo < 48 || RegNo > 63)
      return MCDisassembler::Fail;
    Register = RegisterTableUnified[RegNo];
  } else {
    if (RegNo < 240 || RegNo > 255)
      return MCDisassembler::Fail;
    Register = RegisterTableVPU[RegNo];
  }
  if (Register == ~0U)
    return MCDisassembler::Fail;
  Inst.addOperand(MCOperand::createReg(Register));
  return MCDisassembler::Success;
}

static DecodeStatus DecodeMRFRegisterClass(MCInst &Inst, unsigned RegNo,
                                           uint64_t Addr,
                                           const void *Decoder) {
  if (RegNo > 3)
    return MCDisassembler::Fail;

  static const unsigned RegistersMRF[4] = { TPC::M0, TPC::M1, TPC::M2, TPC::M3 };
  unsigned Register = RegistersMRF[RegNo];
  Inst.addOperand(MCOperand::createReg(Register));
  return MCDisassembler::Success;
}

static const std::map<unsigned, std::string> ZRFComments = {
    {TPC::Z0,  "Z0=[S0,S1]"},
    {TPC::Z2,  "Z2=[S2,S3]"},
    {TPC::Z4,  "Z4=[S4,S5]"},
    {TPC::Z6,  "Z6=[S6,S7]"},
    {TPC::Z8,  "Z8=[S8,S9]"},
    {TPC::Z10, "Z10=[S10,S11]"},
    {TPC::Z12, "Z12=[S12,S13]"},
    {TPC::Z14, "Z14=[S14,S15]"},
    {TPC::Z16, "Z16=[S16,S17]"},
    {TPC::Z18, "Z18=[S18,S19]"},
    {TPC::Z20, "Z20=[S20,S21]"},
    {TPC::Z22, "Z22=[S22,S23]"},
    {TPC::Z24, "Z24=[S24,S25]"},
    {TPC::Z26, "Z26=[S26,S27]"},
    {TPC::Z28, "Z28=[S28,S29]"},
    {TPC::Z30, "Z30=[S30,S31]"},
    {TPC::Z32, "Z32=[S32,S33]"},
    {TPC::Z34, "Z34=[S34,S35]"},
    {TPC::Z36, "Z36=[S36,S37]"},
    {TPC::Z38, "Z38=[S38,S39]"},
    {TPC::Z40, "Z40=[S40,S41]"},
    {TPC::Z42, "Z42=[S42,S43]"},
    {TPC::Z44, "Z44=[S44,S45]"},
    {TPC::Z46, "Z46=[S46,S47]"},
    {TPC::Z48, "Z48=[S48,S49]"},
    {TPC::Z50, "Z50=[S50,S51]"},
    {TPC::Z52, "Z52=[S52,S53]"},
    {TPC::Z54, "Z54=[S54,S55]"},
    {TPC::Z56, "Z56=[S56,S57]"},
    {TPC::Z58, "Z58=[S58,S59]"},
    {TPC::Z60, "Z60=[S60,S61]"},
    {TPC::Z62, "Z62=[S62,S63]"}
};

static const std::map<unsigned, std::string> DRFComments = {
    {TPC::D0,  "D0=[V0,V1]"},
    {TPC::D2,  "D2=[V2,V3]"},
    {TPC::D4,  "D4=[V4,V5]"},
    {TPC::D6,  "D6=[V6,V7]"},
    {TPC::D8,  "D8=[V8,V9]"},
    {TPC::D10, "D10=[V10,V11]"},
    {TPC::D12, "D12=[V12,V13]"},
    {TPC::D14, "D14=[V14,V15]"},
    {TPC::D16, "D16=[V16,V17]"},
    {TPC::D18, "D18=[V18,V19]"},
    {TPC::D20, "D20=[V20,V21]"},
    {TPC::D22, "D22=[V22,V23]"},
    {TPC::D24, "D24=[V24,V25]"},
    {TPC::D26, "D26=[V26,V27]"},
    {TPC::D28, "D28=[V28,V29]"},
    {TPC::D30, "D30=[V30,V31]"},
    {TPC::D32, "D32=[V32,V33]"},
    {TPC::D34, "D34=[V34,V35]"},
    {TPC::D36, "D36=[V36,V37]"},
    {TPC::D38, "D38=[V38,V39]"}
};

static const std::map<unsigned, std::string> ARFComments = {
    {TPC::A0,  "A0=[V0,V1,V2,V3]"},
    {TPC::A4,  "A4=[V4,V5,V6,V7]"},
    {TPC::A8,  "A8=[V8,V9,V10,V11]"},
    {TPC::A12, "A12=[V12,V13,V14,V15]"},
    {TPC::A16, "A16=[V16,V17,V18,V19]"},
    {TPC::A20, "A20=[V20,V21,V22,V23]"},
    {TPC::A24, "A24=[V24,V25,V26,V27]"},
    {TPC::A28, "A28=[V28,V29,V30,V31]"},
    {TPC::A32, "A32=[V32,V33,V34,V35]"},
    {TPC::A36, "A36=[V36,V37,V38,V39]"}
};

static const std::map<unsigned, std::string> SQZComments = {
  {TPC::SQZ_CNTR0, "SQZ_CNTR0=[SQZ_CNTR0_LO,SQZ_CNTR0_HI]"},
  {TPC::SQZ_CNTR1, "SQZ_CNTR1=[SQZ_CNTR1_LO,SQZ_CNTR1_HI]"},
  {TPC::SQZ_CNTR2, "SQZ_CNTR2=[SQZ_CNTR2_LO,SQZ_CNTR2_HI]"},
  {TPC::SQZ_CNTR3, "SQZ_CNTR3=[SQZ_CNTR3_LO,SQZ_CNTR3_HI]"},
  {TPC::SQZ_CNTR4, "SQZ_CNTR4=[SQZ_CNTR4_LO,SQZ_CNTR4_HI]"},
  {TPC::SQZ_CNTR5, "SQZ_CNTR5=[SQZ_CNTR5_LO,SQZ_CNTR5_HI]"},
  {TPC::SQZ_CNTR6, "SQZ_CNTR6=[SQZ_CNTR6_LO,SQZ_CNTR6_HI]"},
  {TPC::SQZ_CNTR7, "SQZ_CNTR7=[SQZ_CNTR7_LO,SQZ_CNTR7_HI]"}
};

static void ConstructComplexRegisterComment(unsigned RegNo, std::string &comment) {
  for (const std::map<unsigned, std::string> *item :
       {&ZRFComments, &DRFComments, &ARFComments, &SQZComments}) {
    auto Ptr = item->find(RegNo);
    if (Ptr != item->end()) {
      if (comment.find(Ptr->second) != std::string::npos)
        return;

      if (!comment.empty()) {
        comment += " , ";
      }

      comment += Ptr->second; 
      return;
    }
  }
}

static DecodeStatus DecodeDRFRegisterClass(MCInst &Inst, unsigned RegNo,
                                           uint64_t Addr,
                                           const void *Decoder) {
  if (RegNo >= 104)
    return MCDisassembler::Fail;
  auto *Disasm = static_cast<const TPCDisassembler *>(Decoder);
  const FeatureBitset &Bits = Disasm->getSubtargetInfo().getFeatureBits();
  bool UnifiedEncoding = (bool) Bits[TPC::FeatureDoron1];

  static const std::map<unsigned, unsigned> VRF2DRF = {
    { TPC::V0, TPC::D0 },
    { TPC::V2, TPC::D2 },
    { TPC::V4, TPC::D4 },
    { TPC::V6, TPC::D6 },
    { TPC::V8, TPC::D8 },
    { TPC::V10, TPC::D10 },
    { TPC::V12, TPC::D12 },
    { TPC::V14, TPC::D14 },
    { TPC::V16, TPC::D16 },
    { TPC::V18, TPC::D18 },
    { TPC::V20, TPC::D20 },
    { TPC::V22, TPC::D22 },
    { TPC::V24, TPC::D24 },
    { TPC::V26, TPC::D26 },
    { TPC::V28, TPC::D28 },
    { TPC::V30, TPC::D30 },
    { TPC::V32, TPC::D32 },
    { TPC::V34, TPC::D34 },
    { TPC::V36, TPC::D36 },
    { TPC::V38, TPC::D38 }
  };

  unsigned Register = ~0U;
  if (UnifiedEncoding)
    Register = RegisterTableUnified[RegNo];
  else
    Register = RegisterTableVPU[RegNo];
  if (Register == ~0U)
    return MCDisassembler::Fail;
  auto Ptr = VRF2DRF.find(Register);
  if (Ptr == VRF2DRF.end())
    return MCDisassembler::Fail;
  Inst.addOperand(MCOperand::createReg(Ptr->second));
  return MCDisassembler::Success;
}

static DecodeStatus DecodeZRFRegisterClass(MCInst &Inst, unsigned RegNo,
                                           uint64_t Addr,
                                           const void *Decoder) {
  static const std::map<unsigned, unsigned> SRF2ZRF = {
      { TPC::S0, TPC::Z0 },
      { TPC::S2, TPC::Z2 },
      { TPC::S4, TPC::Z4 },
      { TPC::S6, TPC::Z6 },
      { TPC::S8, TPC::Z8 },
      { TPC::S10, TPC::Z10 },
      { TPC::S12, TPC::Z12 },
      { TPC::S14, TPC::Z14 },
      { TPC::S16, TPC::Z16 },
      { TPC::S18, TPC::Z18 },
      { TPC::S20, TPC::Z20 },
      { TPC::S22, TPC::Z22 },
      { TPC::S24, TPC::Z24 },
      { TPC::S26, TPC::Z26 },
      { TPC::S28, TPC::Z28 },
      { TPC::S30, TPC::Z30 },
      { TPC::S32, TPC::Z32 },
      { TPC::S34, TPC::Z34 },
      { TPC::S36, TPC::Z36 },
      { TPC::S38, TPC::Z38 },
      { TPC::S40, TPC::Z40 },
      { TPC::S42, TPC::Z42 },
      { TPC::S44, TPC::Z44 },
      { TPC::S46, TPC::Z46 },
      { TPC::S48, TPC::Z48 },
      { TPC::S50, TPC::Z50 },
      { TPC::S52, TPC::Z52 },
      { TPC::S54, TPC::Z54 },
      { TPC::S56, TPC::Z56 },
      { TPC::S58, TPC::Z58 },
      { TPC::S60, TPC::Z60 },
      { TPC::S62, TPC::Z62 },
  };

  auto *Disasm = static_cast<const TPCDisassembler *>(Decoder);
  const FeatureBitset &Bits = Disasm->getSubtargetInfo().getFeatureBits();
  bool UnifiedEncoding = (bool) Bits[TPC::FeatureDoron1];

  unsigned Register = ~0U;
  if (UnifiedEncoding) {
    Register = RegisterTableUnified[RegNo];
  } else {
    Register = RegisterTableSPU[RegNo];
  }
  if (Register == ~0U)
    return MCDisassembler::Fail;
  auto Ptr = SRF2ZRF.find(Register);
  if (Ptr == SRF2ZRF.end())
    return MCDisassembler::Fail;
  Inst.addOperand(MCOperand::createReg(Ptr->second));
  return MCDisassembler::Success;
}


static DecodeStatus DecodeARFRegisterClass(MCInst &Inst, unsigned RegNo,
                                           uint64_t Addr,
                                           const void *Decoder) {
  if (RegNo >= 104)
    return MCDisassembler::Fail;

  static const std::map<unsigned, unsigned> VRF2ARF = {
    { TPC::V0, TPC::A0 },
    { TPC::V4, TPC::A4 },
    { TPC::V8, TPC::A8 },
    { TPC::V12, TPC::A12 },
    { TPC::V16, TPC::A16 },
    { TPC::V20, TPC::A20 },
    { TPC::V24, TPC::A24 },
    { TPC::V28, TPC::A28 },
    { TPC::V32, TPC::A32 },
    { TPC::V36, TPC::A36 }
  };

  auto *Disasm = static_cast<const TPCDisassembler *>(Decoder);
  const FeatureBitset &Bits = Disasm->getSubtargetInfo().getFeatureBits();
  bool UnifiedEncoding = (bool) Bits[TPC::FeatureDoron1];

  unsigned Register = ~0U;
  if (UnifiedEncoding)
    Register = RegisterTableUnified[RegNo];
  else
    Register = RegisterTableVPU[RegNo];
  if (Register == ~0U)
    return MCDisassembler::Fail;
  auto Ptr = VRF2ARF.find(Register);
  if (Ptr == VRF2ARF.end())
    return MCDisassembler::Fail;
  Inst.addOperand(MCOperand::createReg(Ptr->second));
  return MCDisassembler::Success;
}

static DecodeStatus DecodeHWTnsrRegLdRegisterClass(MCInst &Inst, unsigned Code,
                                                 uint64_t Address,
                                                 const void *Decoder) {
  Inst.addOperand(MCOperand::createReg(TPC::LD_TNSR_ID_REG));
  return MCDisassembler::Success;
}

static DecodeStatus DecodeHWTnsrRegStRegisterClass(MCInst &Inst, unsigned Code,
                                                 uint64_t Address,
                                                 const void *Decoder) {
  Inst.addOperand(MCOperand::createReg(TPC::ST_TNSR_ID_REG));
  return MCDisassembler::Success;
}

/*
static DecodeStatus DecodeHVRFRegisterClass(MCInst &Inst, unsigned Code,
                                            uint64_t Address,
                                            const void *Decoder) {
  unsigned RegCode = 0;
  switch (Code) {
  case 0:
    RegCode = TPC::PC;
    break;
  case 1:
    RegCode = TPC::DIV_STEP;
    break;
  case 2:
    RegCode = TPC::S_CARRY;
    break;
  case 3:
    RegCode = TPC::V_CARRY;
    break;
  case 4:
    RegCode = TPC::M0;
    break;
  case 5:
    RegCode = TPC::M1;
    break;
  case 6:
    RegCode = TPC::M2;
    break;
  case 7:
    RegCode = TPC::M3;
    break;
  case 8:
    RegCode = TPC::LD_TNSR_ID_REG;
    break;
  case 9:
    RegCode = TPC::ST_TNSR_ID_REG;
    break;
  case 10:
    RegCode = TPC::ST_RMW_REG;
    break;
  case 11:
    RegCode = TPC::LD_PARTIAL_REG;
    break;
  case 12:
    RegCode = TPC::ST_PARTIAL_REG;
    break;
  case 13:
    RegCode = TPC::ZP_REG;
    break;
  case 16:
    RegCode = TPC::SQZ_CNTR0_LO;
    break;
  case 17:
    RegCode = TPC::SQZ_CNTR0_HI;
    break;
  case 18:
    RegCode = TPC::SQZ_CNTR1_LO;
    break;
  case 19:
    RegCode = TPC::SQZ_CNTR1_HI;
    break;
  case 20:
    RegCode = TPC::SQZ_CNTR2_LO;
    break;
  case 21:
    RegCode = TPC::SQZ_CNTR2_HI;
    break;
  case 22:
    RegCode = TPC::SQZ_CNTR3_LO;
    break;
  case 23:
    RegCode = TPC::SQZ_CNTR3_HI;
    break;
  case 24:
    RegCode = TPC::SQZ_CNTR4_LO;
    break;
  case 25:
    RegCode = TPC::SQZ_CNTR4_HI;
    break;
  case 26:
    RegCode = TPC::SQZ_CNTR5_LO;
    break;
  case 27:
    RegCode = TPC::SQZ_CNTR5_HI;
    break;
  case 28:
    RegCode = TPC::SQZ_CNTR6_LO;
    break;
  case 29:
    RegCode = TPC::SQZ_CNTR6_HI;
    break;
  case 30:
    RegCode = TPC::SQZ_CNTR7_LO;
    break;
  case 31:
    RegCode = TPC::SQZ_CNTR7_HI;
    break;
  default:
    return MCDisassembler::Fail;
  }

  Inst.addOperand(MCOperand::createReg(RegCode));
  return MCDisassembler::Success;
}

static DecodeStatus DecodeHSRFRegisterClass(MCInst &Inst, unsigned Code,
                                            uint64_t Address,
                                            const void *Decoder) {
  return DecodeHVRFRegisterClass(Inst, Code, Address, Decoder);
}
*/

static DecodeStatus decodeSPredicate(MCInst &Inst, unsigned Code,
                                     uint64_t Address,
                                     const void *Decoder) {
  unsigned RegNo = Code & 0x1f;
  unsigned Polarity = (Code >> 5) & 0x01;
  unsigned Register = PredicateTable[RegNo];
  Inst.addOperand(MCOperand::createReg(Register));
  Inst.addOperand(MCOperand::createImm(Polarity));
  return MCDisassembler::Success;
}

static DecodeStatus decodeVPredicate(MCInst &Inst, unsigned Code,
                                     uint64_t Address,
                                     const void *Decoder) {
  unsigned RegNo = (Code & 0x0f) | 0x10;
  unsigned Polarity = (Code >> 5) & 0x01;
  unsigned Register = PredicateTable[RegNo];
  Inst.addOperand(MCOperand::createReg(Register));
  Inst.addOperand(MCOperand::createImm(Polarity));
  return MCDisassembler::Success;
}

static DecodeStatus DecodeHWSqzCntrRegisterClass(MCInst &Inst, unsigned RegNo,
                                                  uint64_t Addr,
                                                  const void *Decoder) {
  if (RegNo > 7)
    return MCDisassembler::Fail;

  static const unsigned RegistersSQZ[8] ={
    TPC::SQZ_CNTR0, TPC::SQZ_CNTR1, TPC::SQZ_CNTR2, TPC::SQZ_CNTR3,
    TPC::SQZ_CNTR4, TPC::SQZ_CNTR5, TPC::SQZ_CNTR6, TPC::SQZ_CNTR7 };
  unsigned Register = RegistersSQZ[RegNo];
  Inst.addOperand(MCOperand::createReg(Register));
  return MCDisassembler::Success;
}

static DecodeStatus decodeMEMrr(MCInst &Inst, unsigned Addr,
                                uint64_t Address, const void *Decoder) {
  auto *Disasm = static_cast<const TPCDisassembler *>(Decoder);
  unsigned Addr1 = Addr & 0x0ff;
  unsigned Addr2 = (Addr >> 8) & 0x0ff;

  if (Disasm->getSubtargetInfo().hasFeature(TPC::FeatureAddr2)) {
    DecodeStatus Status = DecodeVRFRegisterClass(Inst, Addr1, Address, Decoder);
    if (Status != MCDisassembler::Success)
      return Status;
    return DecodeVRFRegisterClass(Inst, Addr2, Address, Decoder);
  } else {
    DecodeStatus Status = DecodeVRFRegisterClass(Inst, Addr1, Address, Decoder);
    if (Status != MCDisassembler::Success)
      return Status;
    // Put bogus register.
    Inst.addOperand(MCOperand::createReg(TPC::SP0));
    return MCDisassembler::Success;
  }
}


#include "TPCGenDisassemblerTables.inc"

#ifndef NDEBUG
static std::string to_hexstring(const std::bitset<256> &BS) {
  std::bitset<256> V = BS;
  std::string Result;
  for (unsigned I = 0; I < 256; I += 4) {
    std::bitset<256> Digit = V & std::bitset<256>(0x0F);
    if (I && I % 32 == 0)
      Result = "_" + Result;
    Result = "0123456789ABCDEF"[Digit.to_ulong()] + Result;
    V >>= 4;
  }
  return Result;
}

static std::string to_hexstring(uint64_t V) {
  std::string Result;
  while (V) {
    int Digit = V & 0x0F;
    Result = "0123456789ABCDEF"[Digit] + Result;
    V >>= 4;
  }
  if (Result.empty())
    Result = "0";
  return Result;
}
#endif

const static std::unordered_map<unsigned, Register> GaudiHWRegs = {
  {0, TPC::PC}};
const static std::unordered_map<unsigned, Register> GrecoHWRegs = {
  {0, TPC::PC}, {1, TPC::DIV_STEP}, {2, TPC::S_CARRY}, {3, TPC::V_CARRY},
  {4, TPC::M0}, {5, TPC::M1}, {6, TPC::M2}, {7, TPC::M3},
  {8, TPC::LD_TNSR_ID_REG}, {9, TPC::ST_TNSR_ID_REG}, {10, TPC::ST_RMW_REG},
  {11, TPC::LD_PARTIAL_REG}, {12, TPC::ST_PARTIAL_REG}, {13, TPC::ZP_REG}};
const static std::unordered_map<unsigned, Register> Gaudi2HWRegs = {
  {0, TPC::PC}, {2, TPC::S_CARRY}, {3, TPC::V_CARRY},
  {4, TPC::M0}, {5, TPC::M1}, {6, TPC::M2}, {7, TPC::M3},
  {8, TPC::LD_TNSR_ID_REG}, {9, TPC::ST_TNSR_ID_REG}, {10, TPC::ST_RMW_REG},
  {11, TPC::LD_PARTIAL_REG}, {12, TPC::ST_PARTIAL_REG}, {13, TPC::ZP_REG},
  {16, TPC::SQZ_CNTR0_LO}, {17, TPC::SQZ_CNTR0_HI},
  {18, TPC::SQZ_CNTR1_LO}, {19, TPC::SQZ_CNTR1_HI},
  {20, TPC::SQZ_CNTR2_LO}, {21, TPC::SQZ_CNTR2_HI},
  {22, TPC::SQZ_CNTR3_LO}, {23, TPC::SQZ_CNTR3_HI},
  {24, TPC::SQZ_CNTR4_LO}, {25, TPC::SQZ_CNTR4_HI},
  {26, TPC::SQZ_CNTR5_LO}, {27, TPC::SQZ_CNTR5_HI},
  {28, TPC::SQZ_CNTR6_LO}, {29, TPC::SQZ_CNTR6_HI},
  {30, TPC::SQZ_CNTR7_LO}, {31, TPC::SQZ_CNTR7_HI}};
const static std::unordered_map<unsigned, Register> Doron1HWRegs = {
  {0, TPC::PC}, {2, TPC::S_CARRY}, {3, TPC::V_CARRY},
  {4, TPC::M0}, {5, TPC::M1}, {6, TPC::M2}, {7, TPC::M3},
  {8, TPC::LD_TNSR_ID_REG}, {9, TPC::ST_TNSR_ID_REG}, {10, TPC::ST_RMW_REG},
  {11, TPC::LD_PARTIAL_REG}, {12, TPC::ST_PARTIAL_REG}, {13, TPC::ZP_REG},
  {14, TPC::INC_LD_DIM_REG}, {15, TPC::INC_ST_DIM_REG},
  {16, TPC::VPU_LFSR}, {17, TPC::SPU_LFSR}, {18, TPC::VPU_LFSR_RO},
  {19, TPC::SPU_LFSR_RO}, {20, TPC::LANE_ID_4B}, {21, TPC::LANE_ID_2B},
  {22, TPC::LANE_ID_1B}, {23, TPC::THREAD_ID}};

static DecodeStatus DecodeHWRegisters(MCInst &Inst, unsigned RegNo,
                                      uint64_t /*Addr*/, const void *Decoder) {
  auto *Disasm = static_cast<const TPCDisassembler *>(Decoder);
  assert(Disasm);
  
  const FeatureBitset &Bits = Disasm->getSubtargetInfo().getFeatureBits();
  const std::unordered_map<unsigned, Register> *RegistersMap = nullptr;
  
  if (Bits[TPC::FeatureGoya]) {
    assert(false && "There are not HW registers for Dali");
    return MCDisassembler::Fail;
  } else if (Bits[TPC::FeatureGaudi] || Bits[TPC::FeatureGaudiB]) {
    RegistersMap = &GaudiHWRegs;
  } else if (Bits[TPC::FeatureGreco]) {
    RegistersMap = &GrecoHWRegs;
  } else if (Bits[TPC::FeatureGaudi2]) {
    RegistersMap = &Gaudi2HWRegs;
  } else if (Bits[TPC::FeatureDoron1]) {
    RegistersMap = &Doron1HWRegs;
  } else {
    assert(false && "Add new architecture to DecodeHWRegisters!");
    return MCDisassembler::Fail;
  }
  
  assert(RegistersMap);
  auto Iter = RegistersMap->find(RegNo);
  if (Iter != RegistersMap->end()) {
    Inst.addOperand(MCOperand::createReg(Iter->second));
    return MCDisassembler::Success;
  } else {
    return MCDisassembler::Fail;
  }
}

static MCDisassembler::DecodeStatus decodeMovLd(MCInst &Inst, uint64_t insn,
                                                uint64_t Address, const void *Decoder) {
  uint64_t SrcA = fieldFromInstruction(insn, TPCII::LdSrcAStart, TPCII::LdSrcASize);
  uint64_t SrcB = fieldFromInstruction(insn, TPCII::LdSrcBStart, TPCII::LdSrcBSize);
  uint64_t Dest = fieldFromInstruction(insn, TPCII::LdDestStart, TPCII::LdDestSize);
  uint64_t Switches = fieldFromInstruction(insn, TPCII::LdSwitchesStart, TPCII::LdSwitchesSize);
  uint64_t Predicate = fieldFromInstruction(insn, TPCII::LdPredicateStart, TPCII::LdPredicateSize);
  bool IsVectorPredicate = (bool) fieldFromInstruction(insn, TPCII::LdVectorPredBit, 1);
  bool Polarity = (bool) fieldFromInstruction(insn, TPCII::LdPolarityBit, 1);
  auto *Disasm = static_cast<const TPCDisassembler *>(Decoder);
  assert(Disasm);
  const FeatureBitset &Bits = Disasm->getSubtargetInfo().getFeatureBits();
  bool IsDoron1 = (bool)Bits[TPC::FeatureDoron1];
  
  if ((Switches & 1) == 0) { // HW_REG
    // VRF <- VRF
    if (isVRF(Dest) && isVRF(SrcA)) {
      Inst.setOpcode(IsVectorPredicate ? TPC::MOV_ld_vvm : TPC::MOV_ld_vvp);
      if (DecodeVRFRegisterClass(Inst, Dest, Address, Decoder) == MCDisassembler::Fail)
        return MCDisassembler::Fail;
      if (DecodeVRFRegisterClass(Inst, SrcA, Address, Decoder) == MCDisassembler::Fail)
        return MCDisassembler::Fail;

    // VRF <- VPRF
    } else if (isVRF(Dest) && isVPRF(SrcA, IsDoron1)) {
      Inst.setOpcode(IsVectorPredicate ? TPC::MOV_ld_vmm : TPC::MOV_ld_vmp);
      if (DecodeVRFRegisterClass(Inst, Dest, Address, Decoder) == MCDisassembler::Fail)
        return MCDisassembler::Fail;
      if (DecodeVPRFRegisterClass(Inst, SrcA, Address, Decoder) == MCDisassembler::Fail)
        return MCDisassembler::Fail;

    // VPRF <- VRF
    } else if (isVPRF(Dest, IsDoron1) && isVRF(SrcA)) {
      Inst.setOpcode(IsVectorPredicate ? TPC::MOV_ld_vmm : TPC::MOV_ld_vmp);
      if (DecodeVPRFRegisterClass(Inst, Dest, Address, Decoder) == MCDisassembler::Fail)
        return MCDisassembler::Fail;
      if (DecodeVRFRegisterClass(Inst, SrcA, Address, Decoder) == MCDisassembler::Fail)
        return MCDisassembler::Fail;

    // VPRF <- VPRF
    } else if (isVPRF(Dest, IsDoron1) && isVPRF(SrcA, IsDoron1)) {
      Inst.setOpcode(IsVectorPredicate ? TPC::MOV_ld_mmm : TPC::MOV_ld_mmp);
      if (DecodeVPRFRegisterClass(Inst, Dest, Address, Decoder) == MCDisassembler::Fail)
        return MCDisassembler::Fail;
      if (DecodeVRFRegisterClass(Inst, SrcA, Address, Decoder) == MCDisassembler::Fail)
        return MCDisassembler::Fail;

    // VPRF <- SPRF
    } else if (isVPRF(Dest, IsDoron1) && isSPRF(SrcA, false, IsDoron1)) {
      Inst.setOpcode(IsVectorPredicate ? TPC::MOV_ld_mpm : TPC::MOV_ld_mpp);
      if (DecodeVPRFRegisterClass(Inst, Dest, Address, Decoder) == MCDisassembler::Fail)
        return MCDisassembler::Fail;
      if (DecodeSPRFRegisterClass(Inst, SrcA, Address, Decoder) == MCDisassembler::Fail)
        return MCDisassembler::Fail;

    // SPRF <- SRF
    } else if (isSPRF(Dest, false, IsDoron1) && isSRF(SrcA, false, IsDoron1)) {
      Inst.setOpcode(TPC::MOV_ld_psp);
      if (DecodeSPRFRegisterClass(Inst, Dest, Address, Decoder) == MCDisassembler::Fail)
        return MCDisassembler::Fail;
      if (DecodeSRFRegisterClass(Inst, SrcA, Address, Decoder) == MCDisassembler::Fail)
        return MCDisassembler::Fail;

    // SPRF <- Imm
    } else if (isSPRF(Dest, false, IsDoron1) && isLongImmediate(SrcA, IsDoron1)) {
      Inst.setOpcode(TPC::MOV_ld_pip);
      if (DecodeSPRFRegisterClass(Inst, Dest, Address, Decoder) == MCDisassembler::Fail)
        return MCDisassembler::Fail;
      auto* Disasm = static_cast<const TPCDisassembler*>(Decoder);
      Inst.addOperand(MCOperand::createImm(Disasm->getImmediate()));

    // SPRF <- short Imm
    } else if (isSPRF(Dest, false, IsDoron1) && isShortImmediate(SrcA, IsDoron1)) {
      Inst.setOpcode(TPC::MOV_ld_pip);
      if (DecodeSPRFRegisterClass(Inst, Dest, Address, Decoder) == MCDisassembler::Fail)
        return MCDisassembler::Fail;
      Inst.addOperand(MCOperand::createImm(getShortImmediate(SrcA, IsDoron1)));

    // VRF <- Imm
    } else if (isVRF(Dest) && isLongImmediate(SrcA, IsDoron1)) {
      Inst.setOpcode(IsVectorPredicate ? TPC::MOV_ld_vim : TPC::MOV_ld_vip);
      if (DecodeVRFRegisterClass(Inst, Dest, Address, Decoder) == MCDisassembler::Fail)
        return MCDisassembler::Fail;
      auto *Disasm = static_cast<const TPCDisassembler *>(Decoder);
      Inst.addOperand(MCOperand::createImm(Disasm->getImmediate()));
      uint64_t OpType = (SrcB >> 4);
      if (OpType > TPCII::OpType::Max)
        return MCDisassembler::Fail;
      Inst.addOperand(MCOperand::createImm(OpType));

    // VRF <- short Imm
    } else if (isVRF(Dest) && isShortImmediate(SrcA, IsDoron1)) {
      Inst.setOpcode(IsVectorPredicate ? TPC::MOV_ld_vim : TPC::MOV_ld_vip);
      if (DecodeVRFRegisterClass(Inst, Dest, Address, Decoder) == MCDisassembler::Fail)
        return MCDisassembler::Fail;
      Inst.addOperand(MCOperand::createImm(getShortImmediate(SrcA, IsDoron1)));
      uint64_t OpType = (SrcB >> 4);
      if (OpType > TPCII::OpType::Max)
        return MCDisassembler::Fail;
      Inst.addOperand(MCOperand::createImm(OpType));

    // VRF <- SRF
    } else if (isVRF(Dest) && isSRF(SrcA, false, IsDoron1)) {
      Inst.setOpcode(IsVectorPredicate ? TPC::MOV_ld_vsm : TPC::MOV_ld_vsp);
      if (DecodeVRFRegisterClass(Inst, Dest, Address, Decoder) == MCDisassembler::Fail)
        return MCDisassembler::Fail;
      if (DecodeSRFRegisterClass(Inst, SrcA, Address, Decoder) == MCDisassembler::Fail)
        return MCDisassembler::Fail;
      uint64_t OpType = (SrcB >> 4);
      if (OpType > TPCII::OpType::Max)
        return MCDisassembler::Fail;
      Inst.addOperand(MCOperand::createImm(OpType));

    // VPRF <- SRF
    } else if (isVPRF(Dest, IsDoron1) && isSRF(SrcA, false, IsDoron1)) {
      if (SrcB == 8)
        Inst.setOpcode(IsVectorPredicate ? TPC::MOVB_ld_msm : TPC::MOVB_ld_msp);
      else
        Inst.setOpcode(IsVectorPredicate ? TPC::MOV_ld_msm : TPC::MOV_ld_msp);
      if (DecodeVRFRegisterClass(Inst, Dest, Address, Decoder) == MCDisassembler::Fail)
        return MCDisassembler::Fail;
      if (DecodeSRFRegisterClass(Inst, SrcA, Address, Decoder) == MCDisassembler::Fail)
        return MCDisassembler::Fail;
      if (SrcB != 8)
        Inst.addOperand(MCOperand::createImm(SrcB));

    // VPRF <- long Imm
    } else if (isVPRF(Dest, IsDoron1) && isLongImmediate(SrcA, IsDoron1)) {
      if (SrcB == 8)
        Inst.setOpcode(IsVectorPredicate ? TPC::MOVB_ld_mim : TPC::MOVB_ld_mip);
      else
        Inst.setOpcode(IsVectorPredicate ? TPC::MOV_ld_mim : TPC::MOV_ld_mip);
      if (DecodeVRFRegisterClass(Inst, Dest, Address, Decoder) == MCDisassembler::Fail)
        return MCDisassembler::Fail;
      auto *Disasm = static_cast<const TPCDisassembler *>(Decoder);
      Inst.addOperand(MCOperand::createImm(Disasm->getImmediate()));
      if (SrcB != 8)
        Inst.addOperand(MCOperand::createImm(SrcB));

    // VPRF <- short Imm
    } else if (isVPRF(Dest, IsDoron1) && isShortImmediate(SrcA, IsDoron1)) {
      if (SrcB == 8)
        Inst.setOpcode(IsVectorPredicate ? TPC::MOVB_ld_mim : TPC::MOVB_ld_mip);
      else
        Inst.setOpcode(IsVectorPredicate ? TPC::MOV_ld_mim : TPC::MOV_ld_mip);
      if (DecodeVRFRegisterClass(Inst, Dest, Address, Decoder) == MCDisassembler::Fail)
        return MCDisassembler::Fail;
      Inst.addOperand(MCOperand::createImm(getShortImmediate(SrcA, IsDoron1)));
      if (SrcB != 8)
        Inst.addOperand(MCOperand::createImm(SrcB));

    // SRF <- SRF
    } else if (isSRF(Dest, false, IsDoron1) && isSRF(SrcA, false, IsDoron1)) {
      Inst.setOpcode(TPC::MOV_ld_ssp);
      if (DecodeSRFRegisterClass(Inst, Dest, Address, Decoder) == MCDisassembler::Fail)
        return MCDisassembler::Fail;
      if (DecodeSRFRegisterClass(Inst, SrcA, Address, Decoder) == MCDisassembler::Fail)
        return MCDisassembler::Fail;
      uint64_t OpType = (SrcB >> 4);
      if (OpType > TPCII::OpType::Max)
        return MCDisassembler::Fail;
      Inst.addOperand(MCOperand::createImm(OpType));

    // SRF <- Imm
    } else if (isSRF(Dest, false, IsDoron1) && isLongImmediate(SrcA, IsDoron1)) {
      Inst.setOpcode(TPC::MOV_ld_ssp);
      if (DecodeSRFRegisterClass(Inst, Dest, Address, Decoder) == MCDisassembler::Fail)
        return MCDisassembler::Fail;
      auto *Disasm = static_cast<const TPCDisassembler *>(Decoder);
      Inst.addOperand(MCOperand::createImm(Disasm->getImmediate()));
      uint64_t OpType = (SrcB >> 4);
      if (OpType > TPCII::OpType::Max)
        return MCDisassembler::Fail;
      Inst.addOperand(MCOperand::createImm(OpType));

    // SRF <- short Imm
    } else if (isSRF(Dest, false, IsDoron1) && isShortImmediate(SrcA, IsDoron1)) {
      Inst.setOpcode(TPC::MOV_ld_ssp);
      if (DecodeSRFRegisterClass(Inst, Dest, Address, Decoder) == MCDisassembler::Fail)
        return MCDisassembler::Fail;
      Inst.addOperand(MCOperand::createImm(getShortImmediate(SrcA, IsDoron1)));
      uint64_t OpType = (SrcB >> 4);
      if (OpType > TPCII::OpType::Max)
        return MCDisassembler::Fail;
      Inst.addOperand(MCOperand::createImm(OpType));

    // IRF <- SRF
    } else if (isIRF(Dest, false, IsDoron1) && isSRF(SrcA, false, IsDoron1)) {
      assert((Switches & TPCII::SW_DIM_MASK_REG) == 0 &&
            "It expects DIM_MASK_REG is not set");
      Inst.setOpcode(TPC::MOV_ld_Isp);
      if (DecodeIRFRegisterClass(Inst, Dest, Address, Decoder) == MCDisassembler::Fail)
        return MCDisassembler::Fail;
      if (DecodeSRFRegisterClass(Inst, SrcA, Address, Decoder) == MCDisassembler::Fail)
        return MCDisassembler::Fail;
      Inst.addOperand(MCOperand::createImm(SrcB >> 2));

    // IRF <- Imm
    } else if (isIRF(Dest, false, IsDoron1) && isLongImmediate(SrcA, IsDoron1)) {
      assert((Switches & TPCII::SW_DIM_MASK_REG) == 0 &&
            "It expects DIM_MASK_REG is not set");
      Inst.setOpcode(TPC::MOV_ld_Iip);
      if (DecodeIRFRegisterClass(Inst, Dest, Address, Decoder) == MCDisassembler::Fail)
        return MCDisassembler::Fail;
      auto *Disasm = static_cast<const TPCDisassembler *>(Decoder);
      Inst.addOperand(MCOperand::createImm(Disasm->getImmediate()));
      Inst.addOperand(MCOperand::createImm(SrcB >> 2));

    // IRF <- short Imm
    } else if (isIRF(Dest, false, IsDoron1) && isShortImmediate(SrcA, IsDoron1)) {
      assert((Switches & TPCII::SW_DIM_MASK_REG) == 0 &&
            "It expects DIM_MASK_REG is not set");
      Inst.setOpcode(TPC::MOV_ld_Iip);
      if (DecodeIRFRegisterClass(Inst, Dest, Address, Decoder) == MCDisassembler::Fail)
        return MCDisassembler::Fail;
      Inst.addOperand(MCOperand::createImm(getShortImmediate(SrcA, IsDoron1)));
      Inst.addOperand(MCOperand::createImm(SrcB >> 2));

    // IRF <- IRF
    } else if (isIRF(Dest, false, IsDoron1) && isIRF(SrcA, false, IsDoron1)) {
      assert((Switches & TPCII::SW_DIM_MASK_REG) == 0 &&
            "It expects DIM_MASK_REG is not set");
      Inst.setOpcode(TPC::MOV_ld_Isp);
      if (DecodeIRFRegisterClass(Inst, Dest, Address, Decoder) == MCDisassembler::Fail)
        return MCDisassembler::Fail;
      if (DecodeIRFRegisterClass(Inst, SrcA, Address, Decoder) == MCDisassembler::Fail)
        return MCDisassembler::Fail;
      Inst.addOperand(MCOperand::createImm(SrcB >> 2));

    // ADRF <- ADRF
    } else if (isADRF(Dest, false, IsDoron1) && isADRF(SrcA, false, IsDoron1)) {
      Inst.setOpcode(TPC::MOV_ld_aap);
      if (DecodeADRFRegisterClass(Inst, Dest, Address, Decoder) == MCDisassembler::Fail)
        return MCDisassembler::Fail;
      if (DecodeADRFRegisterClass(Inst, SrcA, Address, Decoder) == MCDisassembler::Fail)
        return MCDisassembler::Fail;
    } else {
      return MCDisassembler::Fail;
    }
  } else {
    // Don't set opcode use disasm
    const auto &DecodeOrdinaryReg =
    [&Inst, &Address, &Decoder, IsDoron1](unsigned Reg) {
      if (isSRF(Reg, false, IsDoron1)) {
        if (DecodeSRFRegisterClass(Inst, Reg, Address, Decoder) ==
            MCDisassembler::Fail)
          return MCDisassembler::Fail;
      } else if (isSPRF(Reg, false, IsDoron1)) {
        if (DecodeSPRFRegisterClass(Inst, Reg, Address, Decoder) ==
            MCDisassembler::Fail)
          return MCDisassembler::Fail;
      } else if (isVRF(Reg)) {
        if (DecodeVRFRegisterClass(Inst, Reg, Address, Decoder) ==
            MCDisassembler::Fail)
          return MCDisassembler::Fail;
      } else if (isVPRF(Reg, IsDoron1)) {
        if (DecodeVPRFRegisterClass(Inst, Reg, Address, Decoder) ==
            MCDisassembler::Fail)
          return MCDisassembler::Fail;
      } else {
        return MCDisassembler::Fail;
      }
      
      return MCDisassembler::Success;
    };
    
    bool ToHWReg = SrcB & 0x40; // TO_HW_REG
    if (!ToHWReg) // Dest is ordinary register
      if (DecodeOrdinaryReg(Dest) == MCDisassembler::Fail)
        return MCDisassembler::Fail;
    
    unsigned HWReg = SrcB & 0b111111;
    if (DecodeHWRegisters(Inst, HWReg, Address, Decoder) ==
        MCDisassembler::Fail)
      return MCDisassembler::Fail;
    
    if (ToHWReg) //  is ordinary register or imm
      if (DecodeOrdinaryReg(SrcA) == MCDisassembler::Fail) {
        if (isShortImmediate(SrcA, IsDoron1)) {
          Inst.addOperand(MCOperand::createImm(getShortImmediate(SrcA, IsDoron1)));
        } else if (isLongImmediate(SrcA, IsDoron1)) {
          Inst.addOperand(MCOperand::createImm(Disasm->getImmediate()));
        } else {
          return MCDisassembler::Fail;
        }
      }
  }
  
  Inst.addOperand(MCOperand::createImm(Switches));
  if (DecodeVRFRegisterClass(Inst, Dest, Address, Decoder) == MCDisassembler::Fail)
    return MCDisassembler::Fail;
  uint64_t PredValue = (Polarity << 5) | (IsVectorPredicate << 4) | Predicate;
  if (IsVectorPredicate) {
    if (decodeVPredicate(Inst, PredValue, Address, Decoder) == MCDisassembler::Fail)
      return MCDisassembler::Fail;
  } else {
    if (decodeSPredicate(Inst, PredValue, Address, Decoder) == MCDisassembler::Fail)
      return MCDisassembler::Fail;
  }
  return MCDisassembler::Success;
}


static MCDisassembler::DecodeStatus decodeMovSpu(MCInst &Inst, uint64_t insn,
                                                 uint64_t Address, const void *Decoder) {
  uint64_t SrcA = fieldFromInstruction(insn, TPCII::SpuSrcAStart, TPCII::SpuSrcASize);
  uint64_t SrcB = fieldFromInstruction(insn, TPCII::SpuSrcBStart, TPCII::SpuSrcBSize);
  uint64_t OpType = fieldFromInstruction(insn, TPCII::SpuOpTypeStart, TPCII::SpuOpTypeSize);
  uint64_t Switches = fieldFromInstruction(insn, TPCII::SpuSwitchesStart, TPCII::SpuSwitchesSize);
  uint64_t Dest = fieldFromInstruction(insn, TPCII::SpuDestStart, TPCII::SpuDestSize);
  uint64_t Predicate = fieldFromInstruction(insn, TPCII::SpuPredicateStart, TPCII::SpuPredicateSize);
  bool Polarity = (bool)fieldFromInstruction(insn, TPCII::SpuPolarityBit, 1);
  bool HasDimmask = false;
  auto *Disasm = static_cast<const TPCDisassembler *>(Decoder);
  assert(Disasm);
  const FeatureBitset &Bits = Disasm->getSubtargetInfo().getFeatureBits();
  bool IsDoron1 = (bool) Bits[TPC::FeatureDoron1];
  
  if ((Switches & 1) == 0) { // HW
    // SRF <- SRF
    if (isSRF(Dest, true, IsDoron1) && isSRF(SrcA, true, IsDoron1)) {
      Inst.setOpcode(TPC::MOVssp);
      if (DecodeSRFRegisterClass(Inst, Dest, Address, Decoder) == MCDisassembler::Fail)
        return MCDisassembler::Fail;
      if (DecodeSRFRegisterClass(Inst, SrcA, Address, Decoder) == MCDisassembler::Fail)
        return MCDisassembler::Fail;

    // SRF <- Imm
    } else if (isSRF(Dest, true, IsDoron1) && isLongImmediate(SrcA, IsDoron1)) {
      Inst.setOpcode(TPC::MOVsip);
      if (DecodeSRFRegisterClass(Inst, Dest, Address, Decoder) == MCDisassembler::Fail)
        return MCDisassembler::Fail;
      auto *Disasm = static_cast<const TPCDisassembler *>(Decoder);
      Inst.addOperand(MCOperand::createImm(Disasm->getImmediate()));

    // SRF <- short Imm
    } else if (isSRF(Dest, true, IsDoron1) && isShortImmediate(SrcA, IsDoron1)) {
      Inst.setOpcode(TPC::MOVsip);
      if (DecodeSRFRegisterClass(Inst, Dest, Address, Decoder) == MCDisassembler::Fail)
        return MCDisassembler::Fail;
      Inst.addOperand(MCOperand::createImm(getShortImmediate(SrcA, IsDoron1)));

    // SPRF <- SRF
    } else if (isSPRF(Dest, true, IsDoron1) && isSRF(SrcA, true, IsDoron1)) {
      Inst.setOpcode(TPC::MOVpsp);
      if (DecodeSPRFRegisterClass(Inst, Dest, Address, Decoder) == MCDisassembler::Fail)
        return MCDisassembler::Fail;
      if (DecodeSRFRegisterClass(Inst, SrcA, Address, Decoder) == MCDisassembler::Fail)
        return MCDisassembler::Fail;

    // SPRF <- Imm
    } else if (isSPRF(Dest, true, IsDoron1) && isLongImmediate(SrcA, IsDoron1)) {
      Inst.setOpcode(TPC::MOVpip);
      if (DecodeSPRFRegisterClass(Inst, Dest, Address, Decoder) == MCDisassembler::Fail)
        return MCDisassembler::Fail;
      auto *Disasm = static_cast<const TPCDisassembler *>(Decoder);
      Inst.addOperand(MCOperand::createImm(Disasm->getImmediate()));

    // SPRF <- Short Imm
    } else if (isSPRF(Dest, true, IsDoron1) && isShortImmediate(SrcA, IsDoron1)) {
      Inst.setOpcode(TPC::MOVpip);
      if (DecodeSPRFRegisterClass(Inst, Dest, Address, Decoder) == MCDisassembler::Fail)
        return MCDisassembler::Fail;
      Inst.addOperand(MCOperand::createImm(getShortImmediate(SrcA, IsDoron1)));

    // SPRF <- SPRF
    } else if (isSPRF(Dest, true, IsDoron1) && isSPRF(SrcA, true, IsDoron1)) {
      Inst.setOpcode(TPC::MOVppp);
      if (DecodeSRFRegisterClass(Inst, Dest, Address, Decoder) == MCDisassembler::Fail)
        return MCDisassembler::Fail;
      if (DecodeSRFRegisterClass(Inst, SrcA, Address, Decoder) == MCDisassembler::Fail)
        return MCDisassembler::Fail;

    // IRF <- SRF
    } else if (isIRF(Dest, true, IsDoron1) && isSRF(SrcA, true, IsDoron1) && (Switches & TPCII::SW_DIM_MASK_REG) == 0) {
      Inst.setOpcode(TPC::MOVIsp);
      if (DecodeIRFRegisterClass(Inst, Dest, Address, Decoder) == MCDisassembler::Fail)
        return MCDisassembler::Fail;
      if (DecodeSRFRegisterClass(Inst, SrcA, Address, Decoder) == MCDisassembler::Fail)
        return MCDisassembler::Fail;
      Inst.addOperand(MCOperand::createImm(Switches >> 2));
      HasDimmask = true;

    // IRF <- Imm
    } else if (isIRF(Dest, true, IsDoron1) && isLongImmediate(SrcA, IsDoron1) && (Switches & TPCII::SW_DIM_MASK_REG) == 0) {
      Inst.setOpcode(TPC::MOVIip);
      if (DecodeIRFRegisterClass(Inst, Dest, Address, Decoder) == MCDisassembler::Fail)
        return MCDisassembler::Fail;
      auto *Disasm = static_cast<const TPCDisassembler *>(Decoder);
      Inst.addOperand(MCOperand::createImm(Disasm->getImmediate()));
      Inst.addOperand(MCOperand::createImm(Switches >> 2));
      HasDimmask = true;

    // IRF <- short Imm
    } else if (isIRF(Dest, true, IsDoron1) && isShortImmediate(SrcA, IsDoron1) && (Switches & TPCII::SW_DIM_MASK_REG) == 0) {
      Inst.setOpcode(TPC::MOVIip);
      if (DecodeIRFRegisterClass(Inst, Dest, Address, Decoder) == MCDisassembler::Fail)
        return MCDisassembler::Fail;
      Inst.addOperand(MCOperand::createImm(getShortImmediate(SrcA, IsDoron1)));
      Inst.addOperand(MCOperand::createImm(Switches >> 2));
      HasDimmask = true;

    // IRF <- IRF
    } else if (isIRF(Dest, true, IsDoron1) && isIRF(SrcA, true, IsDoron1) && (Switches & TPCII::SW_DIM_MASK_REG) == 0) {
      Inst.setOpcode(TPC::MOVIsp);
      if (DecodeIRFRegisterClass(Inst, Dest, Address, Decoder) == MCDisassembler::Fail)
        return MCDisassembler::Fail;
      if (DecodeIRFRegisterClass(Inst, SrcA, Address, Decoder) == MCDisassembler::Fail)
        return MCDisassembler::Fail;
      Inst.addOperand(MCOperand::createImm(Switches >> 2));
      HasDimmask = true;

    // IRF <- SRF
    } else if (isIRF(Dest, true, IsDoron1) && isSRF(SrcA, true, IsDoron1) && (Switches & TPCII::SW_DIM_MASK_REG)) {
      Inst.setOpcode(TPC::MOVwIsp);
      if (DecodeIRFRegisterClass(Inst, Dest, Address, Decoder) == MCDisassembler::Fail)
        return MCDisassembler::Fail;
      if (DecodeSRFRegisterClass(Inst, SrcA, Address, Decoder) == MCDisassembler::Fail)
        return MCDisassembler::Fail;
      if (DecodeMRFRegisterClass(Inst, Switches >> 2, Address, Decoder) == MCDisassembler::Fail)
        return MCDisassembler::Fail;
      HasDimmask = true;

    // IRF <- Imm
    } else if (isIRF(Dest, true, IsDoron1) && isLongImmediate(SrcA, IsDoron1) && (Switches & TPCII::SW_DIM_MASK_REG)) {
      Inst.setOpcode(TPC::MOVwIip);
      if (DecodeIRFRegisterClass(Inst, Dest, Address, Decoder) == MCDisassembler::Fail)
        return MCDisassembler::Fail;
      auto* Disasm = static_cast<const TPCDisassembler*>(Decoder);
      Inst.addOperand(MCOperand::createImm(Disasm->getImmediate()));
      if (DecodeMRFRegisterClass(Inst, Switches >> 2, Address, Decoder) == MCDisassembler::Fail)
        return MCDisassembler::Fail;
      HasDimmask = true;

    // IRF <- short Imm
    } else if (isIRF(Dest, true, IsDoron1) && isShortImmediate(SrcA, IsDoron1) && (Switches & TPCII::SW_DIM_MASK_REG)) {
      Inst.setOpcode(TPC::MOVwIip);
      if (DecodeIRFRegisterClass(Inst, Dest, Address, Decoder) == MCDisassembler::Fail)
        return MCDisassembler::Fail;
      Inst.addOperand(MCOperand::createImm(getShortImmediate(SrcA, IsDoron1)));
      if (DecodeMRFRegisterClass(Inst, Switches >> 2, Address, Decoder) == MCDisassembler::Fail)
        return MCDisassembler::Fail;
      HasDimmask = true;

    // IRF <- IRF
    } else if (isIRF(Dest, true, IsDoron1) && isIRF(SrcA, true, IsDoron1) && (Switches & TPCII::SW_DIM_MASK_REG)) {
      Inst.setOpcode(TPC::MOVwIsp);
      if (DecodeIRFRegisterClass(Inst, Dest, Address, Decoder) == MCDisassembler::Fail)
        return MCDisassembler::Fail;
      if (DecodeIRFRegisterClass(Inst, SrcA, Address, Decoder) == MCDisassembler::Fail)
        return MCDisassembler::Fail;
      if (DecodeMRFRegisterClass(Inst, Switches >> 2, Address, Decoder) == MCDisassembler::Fail)
        return MCDisassembler::Fail;
      HasDimmask = true;

    // ADRF <- ADRF
    } else if (isADRF(Dest, true, IsDoron1) && isADRF(SrcA, true, IsDoron1)) {
      Inst.setOpcode(TPC::MOVaap);
      if (DecodeADRFRegisterClass(Inst, Dest, Address, Decoder) == MCDisassembler::Fail)
        return MCDisassembler::Fail;
      if (DecodeADRFRegisterClass(Inst, SrcA, Address, Decoder) == MCDisassembler::Fail)
        return MCDisassembler::Fail;

    // ADRF <- SRF
    } else if (isADRF(Dest, true, IsDoron1) && isSRF(SrcA, true, IsDoron1)) {
      Inst.setOpcode(TPC::MOVazp);
      if (DecodeADRFRegisterClass(Inst, Dest, Address, Decoder) == MCDisassembler::Fail)
        return MCDisassembler::Fail;
      if (DecodeZRFRegisterClass(Inst, SrcA, Address, Decoder) == MCDisassembler::Fail)
        return MCDisassembler::Fail;

    // SRF <- ADRF
    } else if (isSRF(Dest, true, IsDoron1) && isADRF(SrcA, true, IsDoron1)) {
      Inst.setOpcode(TPC::MOVzap);
      if (DecodeZRFRegisterClass(Inst, Dest, Address, Decoder) == MCDisassembler::Fail)
        return MCDisassembler::Fail;
      if (DecodeADRFRegisterClass(Inst, SrcA, Address, Decoder) == MCDisassembler::Fail)
        return MCDisassembler::Fail;

    } else {
      return MCDisassembler::Fail;
    }
    
    if (!isADRF(SrcA, true, IsDoron1) && !isADRF(Dest, true, IsDoron1) && !isIRF(Dest, true, IsDoron1) && !isSPRF(Dest, true, IsDoron1)) {
      if (OpType > TPCII::OpType::Max)
        return MCDisassembler::Fail;
      Inst.addOperand(MCOperand::createImm(OpType));
    }
    if (HasDimmask)
      Switches &= ~TPCII::SW_DIMMASK;
  } else {
    const auto &DecodeOrdinaryReg =
    [&Inst, &Address, &Decoder, IsDoron1](unsigned Reg) {
      if (isSRF(Reg, true, IsDoron1)) {
        if (DecodeSRFRegisterClass(Inst, Reg, Address, Decoder) ==
            MCDisassembler::Fail)
          return MCDisassembler::Fail;
      } else if (isSPRF(Reg, true, IsDoron1)) {
        if (DecodeSPRFRegisterClass(Inst, Reg, Address, Decoder) ==
            MCDisassembler::Fail)
          return MCDisassembler::Fail;
      } else {
        return MCDisassembler::Fail;
      }
      
      return MCDisassembler::Success;
    };
    
    bool ToHWReg = SrcB & 0x40; // TO_HW_REG
    if (!ToHWReg) { // Dest is ordinary register
      Inst.setOpcode(TPC::MOV_FromHW_Dis);
      if (DecodeOrdinaryReg(Dest) == MCDisassembler::Fail)
        return MCDisassembler::Fail;
    }
    unsigned HWReg = SrcB & 0b111111;
    if (DecodeHWRegisters(Inst, HWReg, Address, Decoder) ==
        MCDisassembler::Fail)
      return MCDisassembler::Fail;
    
    if (ToHWReg) { // Src is ordinary register or imm
      Inst.setOpcode(TPC::MOV_ToHW_Dis);
      if (DecodeOrdinaryReg(SrcA) == MCDisassembler::Fail) {
        if (isShortImmediate(SrcA, IsDoron1)) {
          Inst.addOperand(MCOperand::createImm(getShortImmediate(SrcA,
                                                                 IsDoron1)));
        } else if (isLongImmediate(SrcA, IsDoron1)) {
          Inst.addOperand(MCOperand::createImm(Disasm->getImmediate()));
        } else {
          return MCDisassembler::Fail;
        }
      }
    }
  }

  Inst.addOperand(MCOperand::createImm(Switches));
  if (DecodeSRFRegisterClass(Inst, Dest, Address, Decoder) == MCDisassembler::Fail)
    return MCDisassembler::Fail;
  uint64_t PredValue = (Polarity << 5) | Predicate;
  if (decodeSPredicate(Inst, PredValue, Address, Decoder) == MCDisassembler::Fail)
    return MCDisassembler::Fail;
  return MCDisassembler::Success;
}


static MCDisassembler::DecodeStatus decodeMovVpu(MCInst &Inst, uint64_t insn,
                                                 uint64_t Address, const void *Decoder) {
  uint64_t SrcA = fieldFromInstruction(insn, TPCII::VpuSrcAStart, TPCII::VpuSrcASize);
  uint64_t SrcB = fieldFromInstruction(insn, TPCII::VpuSrcBStart, TPCII::VpuSrcBSize);
  uint64_t Dest = fieldFromInstruction(insn, TPCII::VpuDestStart, TPCII::VpuDestSize);
  uint64_t Switches = fieldFromInstruction(insn, TPCII::VpuSwitches1Start, TPCII::VpuSwitches1Size);
  //Switches = Switches | (fieldFromInstruction(insn, TPCII::VpuSwitches2Start, TPCII::VpuSwitches2Size) <<3);
  auto OpType = static_cast<TPCII::OpType>(fieldFromInstruction(insn, TPCII::VpuOpTypeStart, TPCII::VpuOpTypeSize));
  uint64_t Predicate = fieldFromInstruction(insn, TPCII::VpuPredicateStart, TPCII::VpuPredicateSize);
  bool IsVectorPredicate = (bool)fieldFromInstruction(insn, TPCII::VpuVectorPredBit, 1);
  bool Polarity = (bool)fieldFromInstruction(insn, TPCII::VpuPolarityBit, 1);
  auto *Disasm = static_cast<const TPCDisassembler *>(Decoder);
  assert(Disasm);
  const FeatureBitset &Bits = Disasm->getSubtargetInfo().getFeatureBits();
  bool IsDoron1 = (bool) Bits[TPC::FeatureDoron1];

  if ((Switches & 1) == 0) {
    // VRF <- VRF
    if (isVRF(Dest) && isVRF(SrcA)) {
      if (Switches & TPCII::SW_X2_MOV) {
        Inst.setOpcode(IsVectorPredicate ? TPC::MOVddm : TPC::MOVddp);
        if (DecodeDRFRegisterClass(Inst, Dest, Address, Decoder) == MCDisassembler::Fail) { return MCDisassembler::Fail; }
        if (DecodeDRFRegisterClass(Inst, SrcA, Address, Decoder) == MCDisassembler::Fail) { return MCDisassembler::Fail; }
      } else {
        Inst.setOpcode(IsVectorPredicate ? TPC::MOVvvm : TPC::MOVvvp);
        if (DecodeVRFRegisterClass(Inst, Dest, Address, Decoder) == MCDisassembler::Fail) { return MCDisassembler::Fail; }
        if (DecodeVRFRegisterClass(Inst, SrcA, Address, Decoder) == MCDisassembler::Fail) { return MCDisassembler::Fail; }
      }

    // VRF <- VPRF
    } else if (isVRF(Dest) && isVPRF(SrcA, IsDoron1)) {
      Inst.setOpcode(IsVectorPredicate ? TPC::MOVvmm : TPC::MOVvmp);
      if (DecodeVRFRegisterClass(Inst, Dest, Address, Decoder) == MCDisassembler::Fail)
        return MCDisassembler::Fail;
      if (DecodeVPRFRegisterClass(Inst, SrcA, Address, Decoder) == MCDisassembler::Fail)
        return MCDisassembler::Fail;

    // VPRF <- VRF
    } else if (isVPRF(Dest, IsDoron1) && isVRF(SrcA)) {
      Inst.setOpcode(IsVectorPredicate ? TPC::MOVmvm : TPC::MOVmvp);
      if (DecodeVPRFRegisterClass(Inst, Dest, Address, Decoder) == MCDisassembler::Fail)
        return MCDisassembler::Fail;
      if (DecodeVRFRegisterClass(Inst, SrcA, Address, Decoder) == MCDisassembler::Fail)
        return MCDisassembler::Fail;

    // VPRF <- VPRF
    } else if (isVPRF(Dest, IsDoron1) && isVPRF(SrcA, IsDoron1)) {
      Inst.setOpcode(IsVectorPredicate ? TPC::MOVmmm : TPC::MOVmmp);
      if (DecodeVPRFRegisterClass(Inst, Dest, Address, Decoder) == MCDisassembler::Fail)
        return MCDisassembler::Fail;
      if (DecodeVRFRegisterClass(Inst, SrcA, Address, Decoder) == MCDisassembler::Fail)
        return MCDisassembler::Fail;

    // VPRF <- SPRF
    } else if (isVPRF(Dest, IsDoron1) && isSPRF(SrcA, false, IsDoron1)) {
      Inst.setOpcode(IsVectorPredicate ? TPC::MOVmpm : TPC::MOVmpp);
      if (DecodeVPRFRegisterClass(Inst, Dest, Address, Decoder) == MCDisassembler::Fail)
        return MCDisassembler::Fail;
      if (DecodeSPRFRegisterClass(Inst, SrcA, Address, Decoder) == MCDisassembler::Fail)
        return MCDisassembler::Fail;

    // VRF <- SRF
    } else if (isVRF(Dest) && isSRF(SrcA, false, IsDoron1)) {
      Inst.setOpcode(IsVectorPredicate ? TPC::MOVvsm : TPC::MOVvsp);
      if (DecodeVRFRegisterClass(Inst, Dest, Address, Decoder) == MCDisassembler::Fail)
        return MCDisassembler::Fail;
      if (DecodeSRFRegisterClass(Inst, SrcA, Address, Decoder) == MCDisassembler::Fail)
        return MCDisassembler::Fail;
      Inst.addOperand(MCOperand::createImm(OpType));

    // VRF <- Imm
    } else if (isVRF(Dest) && isLongImmediate(SrcA, IsDoron1)) {
      Inst.setOpcode(IsVectorPredicate ? TPC::MOVvim : TPC::MOVvip);
      if (DecodeVRFRegisterClass(Inst, Dest, Address, Decoder) == MCDisassembler::Fail)
        return MCDisassembler::Fail;
      if (OpType > TPCII::OpType::Max)
        return MCDisassembler::Fail;
      auto* Disasm = static_cast<const TPCDisassembler*>(Decoder);
      Inst.addOperand(MCOperand::createImm(Disasm->getImmediate()));
      Inst.addOperand(MCOperand::createImm(OpType));

    // VRF <- short Imm
    } else if (isVRF(Dest) && isShortImmediate(SrcA, IsDoron1)) {
      Inst.setOpcode(IsVectorPredicate ? TPC::MOVvim : TPC::MOVvip);
      if (DecodeVRFRegisterClass(Inst, Dest, Address, Decoder) == MCDisassembler::Fail)
        return MCDisassembler::Fail;
      if (OpType > TPCII::OpType::Max)
        return MCDisassembler::Fail;
      Inst.addOperand(MCOperand::createImm(getShortImmediate(SrcA, IsDoron1)));
      Inst.addOperand(MCOperand::createImm(OpType));

    // VPRF <- SRF
    } else if (isVPRF(Dest, IsDoron1) && isSRF(SrcA, false, IsDoron1)) {
      if (SrcB == 8)
        Inst.setOpcode(IsVectorPredicate ? TPC::MOVBmsm : TPC::MOVBmsp);
      else
        Inst.setOpcode(IsVectorPredicate ? TPC::MOVmsm : TPC::MOVmsp);
      if (DecodeVRFRegisterClass(Inst, Dest, Address, Decoder) == MCDisassembler::Fail)
        return MCDisassembler::Fail;
      if (DecodeSRFRegisterClass(Inst, SrcA, Address, Decoder) == MCDisassembler::Fail)
        return MCDisassembler::Fail;
      if (SrcB != 8)
        Inst.addOperand(MCOperand::createImm(SrcB));

    // VPRF <- long Imm
    } else if (isVPRF(Dest, IsDoron1) && isLongImmediate(SrcA, IsDoron1)) {
      if (SrcB == 8)
        Inst.setOpcode(IsVectorPredicate ? TPC::MOVBmim : TPC::MOVBmip);
      else
        Inst.setOpcode(IsVectorPredicate ? TPC::MOVmim : TPC::MOVmip);
      if (DecodeVRFRegisterClass(Inst, Dest, Address, Decoder) == MCDisassembler::Fail)
        return MCDisassembler::Fail;
      auto *Disasm = static_cast<const TPCDisassembler *>(Decoder);
      Inst.addOperand(MCOperand::createImm(Disasm->getImmediate()));
      if (SrcB != 8)
        Inst.addOperand(MCOperand::createImm(SrcB));

      // VPRF <- short Imm
    } else if (isVPRF(Dest, IsDoron1) && isShortImmediate(SrcA, IsDoron1)) {
      if (SrcB == 8)
        Inst.setOpcode(IsVectorPredicate ? TPC::MOVBmim : TPC::MOVBmip);
      else
        Inst.setOpcode(IsVectorPredicate ? TPC::MOVmim : TPC::MOVmip);
      if (DecodeVRFRegisterClass(Inst, Dest, Address, Decoder) == MCDisassembler::Fail)
        return MCDisassembler::Fail;
      if (SrcB > TPCII::OpType::Max)
        return MCDisassembler::Fail;
      Inst.addOperand(MCOperand::createImm(getShortImmediate(SrcA, IsDoron1)));
      if (SrcB != 8)
        Inst.addOperand(MCOperand::createImm(SrcB));
    } else {
      return MCDisassembler::Fail;
    }
  } else {
    if (IsVectorPredicate)
      Inst.setOpcode(TPC::MOV_vm_Dis);
    else
      Inst.setOpcode(TPC::MOV_vp_Dis);
    
    const auto &DecodeOrdinaryReg =
    [&Inst, &Address, &Decoder, IsDoron1](unsigned Reg) {
      if (isSRF(Reg, false, IsDoron1)) {
        if (DecodeSRFRegisterClass(Inst, Reg, Address, Decoder) ==
            MCDisassembler::Fail)
          return MCDisassembler::Fail;
      } else if (isVRF(Reg)) {
        if (DecodeVRFRegisterClass(Inst, Reg, Address, Decoder) ==
            MCDisassembler::Fail)
          return MCDisassembler::Fail;
      } else if (isVPRF(Reg, IsDoron1)) {
        if (DecodeVPRFRegisterClass(Inst, Reg, Address, Decoder) ==
            MCDisassembler::Fail)
          return MCDisassembler::Fail;
      } else {
        return MCDisassembler::Fail;
      }
      
      return MCDisassembler::Success;
    };
    
    bool ToHWReg = SrcB & 0x40; // TO_HW_REG
    if (!ToHWReg) // Dest is ordinary register
      if (DecodeOrdinaryReg(Dest) == MCDisassembler::Fail)
        return MCDisassembler::Fail;
    
    unsigned HWReg = SrcB & 0b111111;
    if (DecodeHWRegisters(Inst, HWReg, Address, Decoder) ==
        MCDisassembler::Fail)
      return MCDisassembler::Fail;
    
    if (ToHWReg) //  is ordinary register or imm
      if (DecodeOrdinaryReg(SrcA) == MCDisassembler::Fail) {
        if (isShortImmediate(SrcA, IsDoron1)) {
          Inst.addOperand(MCOperand::createImm(getShortImmediate(SrcA,
                                                                 IsDoron1)));
        } else if (isLongImmediate(SrcA, IsDoron1)) {
          Inst.addOperand(MCOperand::createImm(Disasm->getImmediate()));
        } else {
          return MCDisassembler::Fail;
        }
      }
  }

  Inst.addOperand(MCOperand::createImm(Switches));
  if (DecodeVRFRegisterClass(Inst, Dest, Address, Decoder) == MCDisassembler::Fail)
    return MCDisassembler::Fail;
  uint64_t PredValue = (Polarity << 5) | (IsVectorPredicate << 4) | Predicate;
  if (IsVectorPredicate) {
    if (decodeVPredicate(Inst, PredValue, Address, Decoder) == MCDisassembler::Fail)
      return MCDisassembler::Fail;
  } else {
    if (decodeSPredicate(Inst, PredValue, Address, Decoder) == MCDisassembler::Fail)
      return MCDisassembler::Fail;
  }
  return MCDisassembler::Success;
}

static MCDisassembler::DecodeStatus
decodeConvertScalar(MCInst &Inst, uint64_t insn, uint64_t Address,
                       const void *Decoder) {
  uint64_t SrcA =
      fieldFromInstruction(insn, TPCII::SpuSrcAStart, TPCII::SpuSrcASize);
  uint64_t SrcB =
      fieldFromInstruction(insn, TPCII::SpuSrcBStart, TPCII::SpuSrcBSize);
  uint64_t Dest =
      fieldFromInstruction(insn, TPCII::SpuDestStart, TPCII::SpuDestSize);
  uint64_t OpType =
      fieldFromInstruction(insn, TPCII::SpuOpTypeStart, TPCII::SpuOpTypeSize);
  uint64_t Switches = fieldFromInstruction(insn, TPCII::SpuSwitchesStart,
                                           TPCII::SpuSwitchesSize);
  uint64_t Predicate = fieldFromInstruction(insn, TPCII::SpuPredicateStart,
                                            TPCII::SpuPredicateSize);
  bool Polarity = (bool)fieldFromInstruction(insn, TPCII::SpuPolarityBit, 1);

  bool IsINT64 = OpType == TPCII::OpType::INT64;
  Inst.setOpcode((IsINT64) ? TPC::CONVERTINT64ssp : TPC::CONVERTssp);
  if (DecodeSRFRegisterClass(Inst, Dest, Address, Decoder) == MCDisassembler::Fail)     //0
    return MCDisassembler::Fail;
  if (DecodeSRFRegisterClass(Inst, SrcA, Address, Decoder) ==  MCDisassembler::Fail)    //1
    return MCDisassembler::Fail;
  if (IsINT64) {                                                                      
    if (DecodeSRFRegisterClass(Inst, SrcB, Address, Decoder) ==  MCDisassembler::Fail)  //2
      return MCDisassembler::Fail;
    Inst.addOperand(MCOperand::createImm(Switches));                                     // 3
  } 
  else {
    Inst.addOperand(MCOperand::createImm(OpType));                                      // 2
    Inst.addOperand(MCOperand::createImm((Switches << 16) | (SrcB << 8)));              // 3
  }
  // income is not printed
  if (DecodeSRFRegisterClass(Inst, Dest, Address, Decoder) ==  MCDisassembler::Fail)   //4
    return MCDisassembler::Fail;
  uint64_t PredValue = (Polarity << 5) | Predicate;                                    //5
  if (decodeSPredicate(Inst, PredValue, Address, Decoder) ==  MCDisassembler::Fail)
    return MCDisassembler::Fail;
  return MCDisassembler::Success;
}

static MCDisassembler::DecodeStatus
decodeConvertIntScalar(MCInst &Inst, uint64_t insn, uint64_t Address, const void *Decoder) {
  uint64_t OpCode = fieldFromInstruction(insn, TPCII::SpuOpCodeStart, TPCII::SpuOpCodeSize);
  uint64_t SrcA = fieldFromInstruction(insn, TPCII::SpuSrcAStart, TPCII::SpuSrcASize);
  uint64_t SrcB = fieldFromInstruction(insn, TPCII::SpuSrcBStart, TPCII::SpuSrcBSize);
  uint64_t Dest = fieldFromInstruction(insn, TPCII::SpuDestStart, TPCII::SpuDestSize);
  uint64_t OpType = fieldFromInstruction(insn, TPCII::SpuOpTypeStart, TPCII::SpuOpTypeSize);
  uint64_t Switches = fieldFromInstruction(insn, TPCII::SpuSwitchesStart, TPCII::SpuSwitchesSize);
  uint64_t Predicate = fieldFromInstruction(insn, TPCII::SpuPredicateStart, TPCII::SpuPredicateSize);
  bool Polarity = (bool)fieldFromInstruction(insn, TPCII::SpuPolarityBit, 1);

  auto *Disasm = static_cast<const TPCDisassembler *>(Decoder);
  const FeatureBitset &Bits = Disasm->getSubtargetInfo().getFeatureBits();
  bool IsGoya = Bits[TPC::FeatureGoya];
  bool IsDoron1 = (bool) Bits[TPC::FeatureDoron1];


  // Adjust instruction opcode.
  bool Src2IsImm = isLongImmediate(SrcB, IsDoron1) || isShortImmediate(SrcB, IsDoron1);
  if (OpCode == TPCII::spuCONVERT_INT32) {
    if (IsGoya)
      Inst.setOpcode(Src2IsImm ? TPC::CONVERT_INT32sip : TPC::CONVERT_INT32ssp);
    else
      Inst.setOpcode(Src2IsImm ? TPC::CONVERT_INT32g2sip : TPC::CONVERT_INT32g2ssp);
  } else if (OpCode == TPCII::spuCONVERT_UINT32) {
    if (IsGoya)
      Inst.setOpcode(Src2IsImm ? TPC::CONVERT_UINT32sip : TPC::CONVERT_UINT32ssp);
    else
      Inst.setOpcode(Src2IsImm ? TPC::CONVERT_UINT32g2sip : TPC::CONVERT_UINT32g2ssp);
  } else if (OpCode == TPCII::spuCONVERT_INT16) {
    if (IsGoya)
      Inst.setOpcode(Src2IsImm ? TPC::CONVERT_INT16sip : TPC::CONVERT_INT16ssp);
    else
      Inst.setOpcode(Src2IsImm ? TPC::CONVERT_INT16g2sip : TPC::CONVERT_INT16g2ssp);
  } else if (OpCode == TPCII::spuCONVERT_UINT16) {
    if (IsGoya)
      Inst.setOpcode(Src2IsImm ? TPC::CONVERT_UINT16sip : TPC::CONVERT_UINT16ssp);
    else
      Inst.setOpcode(Src2IsImm ? TPC::CONVERT_UINT16g2sip : TPC::CONVERT_UINT16g2ssp);
  } else {
    llvm_unreachable("Unexpected opcode");
  }

  // Prepare switches.
  uint64_t SwitchesVal = 0;
  if (IsGoya) {
    //   let OperandType{1-0} = sw{17-16}; // round wode
    SwitchesVal |= (OpType & 0x03) << 16;
    //   let OperandType{2} = sw{19};      // destination type
    SwitchesVal |= (OpType & 0x04) << 17;
    //   let Switches{1-0} = sw{1-0};      // LANE_SEL
    SwitchesVal |= (Switches & 0x03);
  } else {
    //   let OperandType{2-0} = sw{18-16}; // round wode
    SwitchesVal |= (OpType & 0x07) << 16;
    //   let OperandType{3} = sw{19};      // destination type
    SwitchesVal |= (OpType & 0x08) << 16;
    //   let Switches{1-0} = sw{1-0};      // LANE_SEL
    //   let Switches{2} = sw{2};          // NUM_LANES
    SwitchesVal |= (Switches & 0x07);
  }

  // Generate the instruction operands.
  if (DecodeSRFRegisterClass(Inst, Dest, Address, Decoder) == MCDisassembler::Fail)
    return MCDisassembler::Fail;
  if (DecodeSRFRegisterClass(Inst, SrcA, Address, Decoder) == MCDisassembler::Fail)
    return MCDisassembler::Fail;
  if (isLongImmediate(SrcB, IsDoron1)) {
    Inst.addOperand(MCOperand::createImm(Disasm->getImmediate()));
  } else if (isShortImmediate(SrcB, IsDoron1)) {
    Inst.addOperand(MCOperand::createImm(getShortImmediate(SrcB, IsDoron1)));
  } else {
    if (DecodeSRFRegisterClass(Inst, SrcB, Address, Decoder) == MCDisassembler::Fail)
      return MCDisassembler::Fail;
  }
  Inst.addOperand(MCOperand::createImm(SwitchesVal));
  if (DecodeSRFRegisterClass(Inst, Dest, Address, Decoder) == MCDisassembler::Fail)
    return MCDisassembler::Fail;
  uint64_t PredValue = (Polarity << 5) | Predicate;
  if (decodeSPredicate(Inst, PredValue, Address, Decoder) == MCDisassembler::Fail)
    return MCDisassembler::Fail;

  return MCDisassembler::Success;
}


static MCDisassembler::DecodeStatus
decodeConvertIntVector(MCInst &Inst, uint64_t insn, uint64_t Address, const void *Decoder) {
  uint64_t OpCode = fieldFromInstruction(insn, TPCII::SpuOpCodeStart, TPCII::SpuOpCodeSize);
  uint64_t SrcA = fieldFromInstruction(insn, TPCII::VpuSrcAStart, TPCII::VpuSrcASize);
  uint64_t SrcB = fieldFromInstruction(insn, TPCII::VpuSrcBStart, TPCII::VpuSrcBSize);
  uint64_t Dest = fieldFromInstruction(insn, TPCII::VpuDestStart, TPCII::VpuDestSize);
  uint64_t OpType = fieldFromInstruction(insn, TPCII::VpuOpTypeStart, TPCII::VpuOpTypeSize);
  uint64_t Switches = fieldFromInstruction(insn, TPCII::VpuSwitches1Start, TPCII::VpuSwitches1Size);
  uint64_t Predicate = fieldFromInstruction(insn, TPCII::VpuPredicateStart, TPCII::VpuPredicateSize);
  bool IsVectorPredicate = (bool)fieldFromInstruction(insn, TPCII::VpuVectorPredBit, 1);
  bool Polarity = (bool)fieldFromInstruction(insn, TPCII::VpuPolarityBit, 1);

  auto *Disasm = static_cast<const TPCDisassembler *>(Decoder);
  const FeatureBitset &Bits = Disasm->getSubtargetInfo().getFeatureBits();
  bool IsGoya = Bits[TPC::FeatureGoya];
  bool IsDoron1 = Bits[TPC::FeatureDoron1];

  // Adjust instruction opcode.
  bool Src2IsImm = isLongImmediate(SrcB, IsDoron1) || isShortImmediate(SrcB, IsDoron1);
  bool Src2IsScalar = isSRF(SrcB, false, IsDoron1);
  if (OpCode == TPCII::vpuCONVERT_INT32) {
    if (IsVectorPredicate) {
      if (IsGoya)
        Inst.setOpcode(Src2IsImm    ? TPC::CONVERT_INT32vim :
                       Src2IsScalar ? TPC::CONVERT_INT32vsm :
                                      TPC::CONVERT_INT32vvm);
      else
        Inst.setOpcode(Src2IsImm    ? TPC::CONVERT_INT32g2vim :
                       Src2IsScalar ? TPC::CONVERT_INT32g2vsm :
                                      TPC::CONVERT_INT32g2vvm);
    } else {
      if (IsGoya)
        Inst.setOpcode(Src2IsImm    ? TPC::CONVERT_INT32vip :
                       Src2IsScalar ? TPC::CONVERT_INT32vsp :
                                      TPC::CONVERT_INT32vvp);
      else
        Inst.setOpcode(Src2IsImm    ? TPC::CONVERT_INT32g2vip :
                       Src2IsScalar ? TPC::CONVERT_INT32g2vsp :
                                      TPC::CONVERT_INT32g2vvp);
    }
  } else if (OpCode == TPCII::vpuCONVERT_UINT32) {
    if (IsVectorPredicate) {
      if (IsGoya)
        Inst.setOpcode(Src2IsImm    ? TPC::CONVERT_UINT32vim :
                       Src2IsScalar ? TPC::CONVERT_UINT32vsm :
                                      TPC::CONVERT_UINT32vvm);
      else
        Inst.setOpcode(Src2IsImm    ? TPC::CONVERT_UINT32g2vim :
                       Src2IsScalar ? TPC::CONVERT_UINT32g2vsm :
                                      TPC::CONVERT_UINT32g2vvm);
    } else {
      if (IsGoya)
        Inst.setOpcode(Src2IsImm    ? TPC::CONVERT_UINT32vip :
                       Src2IsScalar ? TPC::CONVERT_UINT32vsp :
                                      TPC::CONVERT_UINT32vvp);
      else
        Inst.setOpcode(Src2IsImm    ? TPC::CONVERT_UINT32g2vip :
                       Src2IsScalar ? TPC::CONVERT_UINT32g2vsp :
                                      TPC::CONVERT_UINT32g2vvp);
    }
  } else if (OpCode == TPCII::spuCONVERT_INT16) {
    if (IsVectorPredicate) {
      if (IsGoya)
        Inst.setOpcode(Src2IsImm    ? TPC::CONVERT_INT16vim :
                       Src2IsScalar ? TPC::CONVERT_INT16vsm :
                                      TPC::CONVERT_INT16vvm);
      else
        Inst.setOpcode(Src2IsImm    ? TPC::CONVERT_INT16g2vim :
                       Src2IsScalar ? TPC::CONVERT_INT16g2vsm :
                                      TPC::CONVERT_INT16g2vvm);
    } else {
      if (IsGoya)
        Inst.setOpcode(Src2IsImm    ? TPC::CONVERT_INT16vip :
                       Src2IsScalar ? TPC::CONVERT_INT16vsp :
                                      TPC::CONVERT_INT16vvp);
      else
        Inst.setOpcode(Src2IsImm    ? TPC::CONVERT_INT16g2vip :
                       Src2IsScalar ? TPC::CONVERT_INT16g2vsp :
                                      TPC::CONVERT_INT16g2vvp);
    }
  } else if (OpCode == TPCII::spuCONVERT_UINT16) {
    if (IsVectorPredicate) {
      if (IsGoya)
        Inst.setOpcode(Src2IsImm ? TPC::CONVERT_UINT16vim :
                       Src2IsScalar ? TPC::CONVERT_UINT16vsm :
                       TPC::CONVERT_UINT16vvm);
      else
        Inst.setOpcode(Src2IsImm ? TPC::CONVERT_UINT16g2vim :
                       Src2IsScalar ? TPC::CONVERT_UINT16g2vsm :
                       TPC::CONVERT_UINT16g2vvm);
    } else {
      if (IsGoya)
        Inst.setOpcode(Src2IsImm ? TPC::CONVERT_UINT16vip :
                       Src2IsScalar ? TPC::CONVERT_UINT16vsp :
                       TPC::CONVERT_UINT16vvp);
      else
        Inst.setOpcode(Src2IsImm ? TPC::CONVERT_UINT16g2vip :
                       Src2IsScalar ? TPC::CONVERT_UINT16g2vsp :
                       TPC::CONVERT_UINT16g2vvp);
    }
  } else if (OpCode == TPCII::spuCONVERT_INT8) {
    if (IsVectorPredicate) {
        Inst.setOpcode(Src2IsImm    ? TPC::CONVERT_INT8vim :
                       Src2IsScalar ? TPC::CONVERT_INT8vsm :
                       TPC::CONVERT_INT8vvm);
    } else {
        Inst.setOpcode(Src2IsImm    ? TPC::CONVERT_INT8vip :
                       Src2IsScalar ? TPC::CONVERT_INT8vsp :
                       TPC::CONVERT_INT8vvp);
    }
  } else if (OpCode == TPCII::spuCONVERT_UINT8) {
    if (IsVectorPredicate) {
        Inst.setOpcode(Src2IsImm    ? TPC::CONVERT_UINT8vim :
                       Src2IsScalar ? TPC::CONVERT_UINT8vsm :
                       TPC::CONVERT_UINT8vvm);
    } else {
        Inst.setOpcode(Src2IsImm    ? TPC::CONVERT_UINT8vip :
                       Src2IsScalar ? TPC::CONVERT_UINT8vsp :
                       TPC::CONVERT_UINT8vvp);
    }
  } else {
    llvm_unreachable("Unexpected opcode");
  }

  // Prepare switches.
  uint64_t SwitchesVal = 0;
  if (IsGoya) {
    //   let OperandType{1-0} = sw{17-16}; // round wode
    SwitchesVal |= (OpType & 0x03) << 16;
    //   let OperandType{2} = sw{19};      // destination type
    SwitchesVal |= (OpType & 0x04) << 17;
    //   let Switches{1-0} = sw{1-0};      // LANE_SEL
    SwitchesVal |= (Switches & 0x03);
  } else {
    //   let OperandType{2-0} = sw{18-16}; // round wode
    SwitchesVal |= (OpType & 0x07) << 16;
    //   let OperandType{3} = sw{19};      // destination type
    SwitchesVal |= (OpType & 0x08) << 16;
    //   let Switches{1-0} = sw{1-0};      // LANE_SEL
    //   let Switches{2} = sw{2};          // NUM_LANES
    SwitchesVal |= (Switches & 0x07);
  }

  // Generate the instruction operands.
  if (DecodeVRFRegisterClass(Inst, Dest, Address, Decoder) == MCDisassembler::Fail)
    return MCDisassembler::Fail;
  if(OpCode == TPCII::spuCONVERT_INT8 ||OpCode == TPCII::spuCONVERT_UINT8) {
    if (DecodeDRFRegisterClass(Inst, SrcA, Address, Decoder) == MCDisassembler::Fail)
      return MCDisassembler::Fail;
  } else {
    if (DecodeVRFRegisterClass(Inst, SrcA, Address, Decoder) == MCDisassembler::Fail)
      return MCDisassembler::Fail;
  }
  if (isLongImmediate(SrcB, IsDoron1)) {
    Inst.addOperand(MCOperand::createImm(Disasm->getImmediate()));
  } else if (isShortImmediate(SrcB, IsDoron1)) {
    Inst.addOperand(MCOperand::createImm(getShortImmediate(SrcB, IsDoron1)));
  } else {
    if (DecodeVRFRegisterClass(Inst, SrcB, Address, Decoder) == MCDisassembler::Fail)
      return MCDisassembler::Fail;
  }
  Inst.addOperand(MCOperand::createImm(SwitchesVal));
  if (DecodeVRFRegisterClass(Inst, Dest, Address, Decoder) == MCDisassembler::Fail)
    return MCDisassembler::Fail;
  uint64_t PredValue = (Polarity << 5) | (IsVectorPredicate << 4) | Predicate;
  if (IsVectorPredicate) {
    if (decodeVPredicate(Inst, PredValue, Address, Decoder) == MCDisassembler::Fail)
      return MCDisassembler::Fail;
  } else {
    if (decodeSPredicate(Inst, PredValue, Address, Decoder) == MCDisassembler::Fail)
      return MCDisassembler::Fail;
  }

  return MCDisassembler::Success;
}


static MCDisassembler::DecodeStatus
decodeAdd(MCInst &Inst, uint64_t insn, uint64_t Address, const void *Decoder) {
  uint64_t OpCode = fieldFromInstruction(insn, TPCII::SpuOpCodeStart, TPCII::SpuOpCodeSize);
  uint64_t SrcA = fieldFromInstruction(insn, TPCII::SpuSrcAStart, TPCII::SpuSrcASize);
  uint64_t SrcB = fieldFromInstruction(insn, TPCII::SpuSrcBStart, TPCII::SpuSrcBSize);
  uint64_t Dest = fieldFromInstruction(insn, TPCII::SpuDestStart, TPCII::SpuDestSize);
  uint64_t OpType = fieldFromInstruction(insn, TPCII::SpuOpTypeStart, TPCII::SpuOpTypeSize);
  uint64_t Switches = fieldFromInstruction(insn, TPCII::SpuSwitchesStart, TPCII::SpuSwitchesSize);
  uint64_t Predicate = fieldFromInstruction(insn, TPCII::SpuPredicateStart, TPCII::SpuPredicateSize);
  bool Polarity = (bool)fieldFromInstruction(insn, TPCII::SpuPolarityBit, 1);

  auto *Disasm = static_cast<const TPCDisassembler *>(Decoder);
  const FeatureBitset &Bits = Disasm->getSubtargetInfo().getFeatureBits();
  bool IsDoron1 = (bool) Bits[TPC::FeatureDoron1];
  (void)OpCode;
  assert(OpCode == TPCII::spuADD);

  // Adjust instruction opcode.
  if (!isIRF(Dest, true, IsDoron1)) {
    if (DecodeSRFRegisterClass(Inst, Dest, Address, Decoder) == MCDisassembler::Fail)
      return MCDisassembler::Fail;
    if (DecodeSRFRegisterClass(Inst, SrcA, Address, Decoder) == MCDisassembler::Fail)
      return MCDisassembler::Fail;
    if (DecodeSRFRegisterClass(Inst, SrcB, Address, Decoder) == MCDisassembler::Fail)
      return MCDisassembler::Fail;
    if (isLongImmediate(SrcB, IsDoron1) || isShortImmediate(SrcB, IsDoron1))
      Inst.setOpcode(TPC::ADDsip);
    else
      Inst.setOpcode(TPC::ADDssp);
  } else if ((Switches & TPCII::SW_DIM_MASK_REG) == 0) {
    if (DecodeIRFRegisterClass(Inst, Dest, Address, Decoder) == MCDisassembler::Fail)
      return MCDisassembler::Fail;
    if (isLongImmediate(SrcA, IsDoron1)) {
      Inst.addOperand(MCOperand::createImm(Disasm->getImmediate()));
      Inst.setOpcode(TPC::ADDwiIp);
      Inst.setOpcode(TPC::ADDiIp);
    } else if (isShortImmediate(SrcA, IsDoron1)) {
      Inst.addOperand(MCOperand::createImm(getShortImmediate(SrcA, IsDoron1)));
      Inst.setOpcode(TPC::ADDwiIp);
      Inst.setOpcode(TPC::ADDiIp);
    } else if (isIRF(SrcA, true, IsDoron1)) {
      if (DecodeIRFRegisterClass(Inst, SrcA, Address, Decoder) == MCDisassembler::Fail)
        return MCDisassembler::Fail;
      Inst.setOpcode(TPC::ADDIIp);
    } else {
      if (DecodeSRFRegisterClass(Inst, SrcA, Address, Decoder) == MCDisassembler::Fail)
        return MCDisassembler::Fail;
      Inst.setOpcode(TPC::ADDsIp);
    }
    if (DecodeIRFRegisterClass(Inst, SrcB, Address, Decoder) == MCDisassembler::Fail)
      return MCDisassembler::Fail;
    Inst.addOperand(MCOperand::createImm((Switches >> 2) & 0x1f));
    Switches &= ~0x7c;
  } else {
    assert(Bits[TPC::FeatureDimMaskR]);
    if (DecodeIRFRegisterClass(Inst, Dest, Address, Decoder) == MCDisassembler::Fail)
      return MCDisassembler::Fail;
    if (isLongImmediate(SrcA, IsDoron1)) {
      Inst.addOperand(MCOperand::createImm(Disasm->getImmediate()));
      Inst.setOpcode(TPC::ADDwiIp);
    } else if (isShortImmediate(SrcA, IsDoron1)) {
      Inst.addOperand(MCOperand::createImm(getShortImmediate(SrcA, IsDoron1)));
      Inst.setOpcode(TPC::ADDwiIp);
    } else if (isIRF(SrcA, true, IsDoron1)) {
      if (DecodeIRFRegisterClass(Inst, SrcA, Address, Decoder) == MCDisassembler::Fail)
        return MCDisassembler::Fail;
      Inst.setOpcode(TPC::ADDwIIp);
    } else {
      if (DecodeSRFRegisterClass(Inst, SrcA, Address, Decoder) == MCDisassembler::Fail)
        return MCDisassembler::Fail;
      Inst.setOpcode(TPC::ADDwsIp);
    }
    if (DecodeIRFRegisterClass(Inst, SrcB, Address, Decoder) == MCDisassembler::Fail)
      return MCDisassembler::Fail;
    unsigned MReg = (Switches >> 2) & 0x03;
    if (DecodeMRFRegisterClass(Inst, MReg, Address, Decoder) == MCDisassembler::Fail)
      return MCDisassembler::Fail;
    Switches &= ~0x7e;
  }
  Inst.addOperand(MCOperand::createImm(OpType));
  Inst.addOperand(MCOperand::createImm(Switches));
  if (DecodeSRFRegisterClass(Inst, Dest, Address, Decoder) == MCDisassembler::Fail)
    return MCDisassembler::Fail;
  uint64_t PredValue = (Polarity << 5) | Predicate;
  if (decodeSPredicate(Inst, PredValue, Address, Decoder) == MCDisassembler::Fail)
    return MCDisassembler::Fail;
  return MCDisassembler::Success;
}


static MCDisassembler::DecodeStatus
decodeSub(MCInst &Inst, uint64_t insn, uint64_t Address, const void *Decoder) {
  uint64_t OpCode = fieldFromInstruction(insn, TPCII::SpuOpCodeStart, TPCII::SpuOpCodeSize);
  uint64_t SrcA = fieldFromInstruction(insn, TPCII::SpuSrcAStart, TPCII::SpuSrcASize);
  uint64_t SrcB = fieldFromInstruction(insn, TPCII::SpuSrcBStart, TPCII::SpuSrcBSize);
  uint64_t Dest = fieldFromInstruction(insn, TPCII::SpuDestStart, TPCII::SpuDestSize);
  uint64_t OpType = fieldFromInstruction(insn, TPCII::SpuOpTypeStart, TPCII::SpuOpTypeSize);
  uint64_t Switches = fieldFromInstruction(insn, TPCII::SpuSwitchesStart, TPCII::SpuSwitchesSize);
  uint64_t Predicate = fieldFromInstruction(insn, TPCII::SpuPredicateStart, TPCII::SpuPredicateSize);
  bool Polarity = (bool)fieldFromInstruction(insn, TPCII::SpuPolarityBit, 1);

  auto *Disasm = static_cast<const TPCDisassembler *>(Decoder);
  const FeatureBitset &Bits = Disasm->getSubtargetInfo().getFeatureBits();
  bool IsDoron1 = (bool) Bits[TPC::FeatureDoron1];
  (void)OpCode;
  assert(OpCode == TPCII::spuSUB);

  // Adjust instruction opcode.
  if (!isIRF(Dest, true, IsDoron1)) {
    if (DecodeSRFRegisterClass(Inst, Dest, Address, Decoder) == MCDisassembler::Fail)
      return MCDisassembler::Fail;
    if (DecodeSRFRegisterClass(Inst, SrcA, Address, Decoder) == MCDisassembler::Fail)
      return MCDisassembler::Fail;
    if (DecodeSRFRegisterClass(Inst, SrcB, Address, Decoder) == MCDisassembler::Fail)
      return MCDisassembler::Fail;
    if (isLongImmediate(SrcB, IsDoron1) || isShortImmediate(SrcB, IsDoron1))
      Inst.setOpcode(TPC::SUBsip);
    else
      Inst.setOpcode(TPC::SUBssp);
  } else if ((Switches & TPCII::SW_DIM_MASK_REG) == 0) {
    if (DecodeIRFRegisterClass(Inst, Dest, Address, Decoder) == MCDisassembler::Fail)
      return MCDisassembler::Fail;
    if (isLongImmediate(SrcA, IsDoron1)) {
      Inst.addOperand(MCOperand::createImm(Disasm->getImmediate()));
      Inst.setOpcode(TPC::SUBwiIp);
      Inst.setOpcode(TPC::SUBiIp);
    } else if (isShortImmediate(SrcA, IsDoron1)) {
      Inst.addOperand(MCOperand::createImm(getShortImmediate(SrcA, IsDoron1)));
      Inst.setOpcode(TPC::SUBwiIp);
      Inst.setOpcode(TPC::SUBiIp);
    } else if (isIRF(SrcA, true, IsDoron1)) {
      if (DecodeIRFRegisterClass(Inst, SrcA, Address, Decoder) == MCDisassembler::Fail)
        return MCDisassembler::Fail;
      Inst.setOpcode(TPC::SUBIIp);
    } else {
      if (DecodeSRFRegisterClass(Inst, SrcA, Address, Decoder) == MCDisassembler::Fail)
        return MCDisassembler::Fail;
      Inst.setOpcode(TPC::SUBsIp);
    }
    if (DecodeIRFRegisterClass(Inst, SrcB, Address, Decoder) == MCDisassembler::Fail)
      return MCDisassembler::Fail;
    Inst.addOperand(MCOperand::createImm((Switches >> 2) & 0x1f));
    Switches &= ~0x7c;
  } else {
    assert(Bits[TPC::FeatureDimMaskR]);
    if (DecodeIRFRegisterClass(Inst, Dest, Address, Decoder) == MCDisassembler::Fail)
      return MCDisassembler::Fail;
    if (isLongImmediate(SrcA, IsDoron1)) {
      Inst.addOperand(MCOperand::createImm(Disasm->getImmediate()));
      Inst.setOpcode(TPC::SUBwiIp);
    } else if (isShortImmediate(SrcA, IsDoron1)) {
      Inst.addOperand(MCOperand::createImm(getShortImmediate(SrcA, IsDoron1)));
      Inst.setOpcode(TPC::SUBwiIp);
    } else if (isIRF(SrcA, true, IsDoron1)) {
      if (DecodeIRFRegisterClass(Inst, SrcA, Address, Decoder) == MCDisassembler::Fail)
        return MCDisassembler::Fail;
      Inst.setOpcode(TPC::SUBwIIp);
    } else {
      if (DecodeSRFRegisterClass(Inst, SrcA, Address, Decoder) == MCDisassembler::Fail)
        return MCDisassembler::Fail;
      Inst.setOpcode(TPC::SUBwsIp);
    }
    if (DecodeIRFRegisterClass(Inst, SrcB, Address, Decoder) == MCDisassembler::Fail)
      return MCDisassembler::Fail;
    unsigned MReg = (Switches >> 2) & 0x03;
    if (DecodeMRFRegisterClass(Inst, MReg, Address, Decoder) == MCDisassembler::Fail)
      return MCDisassembler::Fail;
    Switches &= ~0x7e;
  }
  Inst.addOperand(MCOperand::createImm(OpType));
  Inst.addOperand(MCOperand::createImm(Switches));
  if (DecodeSRFRegisterClass(Inst, Dest, Address, Decoder) == MCDisassembler::Fail)
    return MCDisassembler::Fail;
  uint64_t PredValue = (Polarity << 5) | Predicate;
  if (decodeSPredicate(Inst, PredValue, Address, Decoder) == MCDisassembler::Fail)
    return MCDisassembler::Fail;
  return MCDisassembler::Success;
}

  static MCDisassembler::DecodeStatus decodeStoreInstPredAddr(MCInst &Inst, uint64_t insn, uint64_t Address, const void *Decoder)
  {
    uint64_t OpCode    = fieldFromInstruction(insn, TPCII::StOpCodeStart, TPCII::StOpCodeSize);
    uint64_t Addr      = fieldFromInstruction(insn, TPCII::StSrcAStart, TPCII::StSrcASize);
    uint64_t Switches  = fieldFromInstruction(insn, TPCII::Gen4StSwitchesStart, TPCII::Gen4StSwitchesSize);
    uint64_t Predicate = fieldFromInstruction(insn, TPCII::StPredicateStart, TPCII::StPredicateSize);
    bool Polarity      = (bool)fieldFromInstruction(insn, TPCII::StPolarityBit, 1);

    if (OpCode == TPCII::CACHE_FLUSH) {
      Inst.setOpcode(TPC::CACHE_FLUSH_ADDR);
      Switches |= TPCII::SW_CL;
    } else {
      Inst.setOpcode(TPC::CACHE_INVALIDATE_ADDR);
      Switches = TPCII::SW_INV_CL | TPCII::SW_D;
    }

    if (DecodeADRFRegisterClass(Inst, Addr, Address, Decoder) == MCDisassembler::Fail)
        return MCDisassembler::Fail;

    Inst.addOperand(MCOperand::createImm(Switches));

    uint64_t PredValue = (Polarity << 5) |  Predicate;
    if (decodeSPredicate(Inst, PredValue, Address, Decoder) == MCDisassembler::Fail)
      return MCDisassembler::Fail;

    return MCDisassembler::Success;
  }

static MCDisassembler::DecodeStatus decodeLdTnsr(MCInst &Inst, uint64_t insn,
                                                 uint64_t Address,
                                                 const void *Decoder) {
  uint64_t OpCode = fieldFromInstruction(insn, TPCII::LdOpCodeStart, TPCII::LdOpCodeSize);
  uint64_t SrcA = fieldFromInstruction(insn, TPCII::LdSrcAStart, TPCII::LdSrcASize);
  uint64_t SrcB = fieldFromInstruction(insn, TPCII::LdSrcBStart, TPCII::LdSrcBSize);
  uint64_t Dest = fieldFromInstruction(insn, TPCII::LdDestStart, TPCII::LdDestSize);
  uint64_t Switches = fieldFromInstruction(insn, TPCII::LdSwitchesStart, TPCII::LdSwitchesSize);
  uint64_t Predicate = fieldFromInstruction(insn, TPCII::LdPredicateStart, TPCII::LdPredicateSize);
  bool IsVectorPredicate = (bool)fieldFromInstruction(insn, TPCII::LdVectorPredBit, 1);
  bool Polarity = (bool)fieldFromInstruction(insn, TPCII::LdPolarityBit, 1);

  auto *Disasm = static_cast<const TPCDisassembler *>(Decoder);
  const FeatureBitset &Bits = Disasm->getSubtargetInfo().getFeatureBits();
  bool IsGaudi = Bits[TPC::FeatureGaudi];
  bool IsGaudiB = Bits[TPC::FeatureGaudiB];
  bool IsDoron1 = (bool) Bits[TPC::FeatureDoron1];
  bool IsVPRF = isVPRF(Dest, IsDoron1);

  const unsigned SW_TNSR_ID_REG = (1 << 3);

  bool IsPartial = false;
  bool IsTnsrIsReg = false;
  bool IsDirect = false;

  if (Switches & SW_TNSR_ID_REG) {
    IsTnsrIsReg = true;
    Switches &= ~SW_TNSR_ID_REG;
  }

  if (Switches & TPCII::SW_PARTIAL) {
    IsPartial = true;
    Switches &= ~TPCII::SW_PARTIAL;
  }

  IsDirect = Switches & TPCII::SW_DIRECT;

  if (OpCode == TPCII::LD_TNSR) {
    if (IsPartial) {
      if (IsTnsrIsReg) {
        if (IsGaudi || IsGaudiB)
          Inst.setOpcode(IsVPRF ? (IsVectorPredicate ? TPC::LD_TNSR_PGen2Tmm : TPC::LD_TNSR_PGen2Tmp) : (IsVectorPredicate ? TPC::LD_TNSR_PGen2Tvm: TPC::LD_TNSR_PGen2Tvp));
        else
          Inst.setOpcode(IsVPRF ? (IsVectorPredicate ? TPC::LD_TNSR_PTmm : TPC::LD_TNSR_PTmp) : (IsVectorPredicate ? TPC::LD_TNSR_PTvm : TPC::LD_TNSR_PTvp));
      } else {
        if (IsGaudi || IsGaudiB)
          Inst.setOpcode(IsVPRF ? (IsVectorPredicate ? TPC::LD_TNSR_PGen2mm : TPC::LD_TNSR_PGen2mp) : (IsVectorPredicate ? TPC::LD_TNSR_PGen2vm : TPC::LD_TNSR_PGen2vp));
        else {
          if (IsDirect)
            Inst.setOpcode(IsVPRF ? (IsVectorPredicate ? TPC::LD_TNSR_PDmm : TPC::LD_TNSR_PDmp) : (IsVectorPredicate ? TPC::LD_TNSR_PDvm : TPC::LD_TNSR_PDvp));
          else
            Inst.setOpcode(IsVPRF ? (IsVectorPredicate ? TPC::LD_TNSR_Pmm : TPC::LD_TNSR_Pmp) : (IsVectorPredicate ? TPC::LD_TNSR_Pvm : TPC::LD_TNSR_Pvp));
        }
      }
    } else {
      if (IsTnsrIsReg) {
        if (IsGaudi || IsGaudiB)
          Inst.setOpcode(IsVPRF ? (IsVectorPredicate ? TPC::LD_TNSRGen2Tmm : TPC::LD_TNSRGen2Tmp) : (IsVectorPredicate ? TPC::LD_TNSRGen2Tvm : TPC::LD_TNSRGen2Tvp));
        else
          Inst.setOpcode(IsVPRF ? (IsVectorPredicate ? TPC::LD_TNSRTmm : TPC::LD_TNSRTmp) : (IsVectorPredicate ? TPC::LD_TNSRTvm : TPC::LD_TNSRTvp));
      } else {
        if (IsDirect)
          Inst.setOpcode(IsVPRF ? (IsVectorPredicate ? TPC::LD_TNSR_Dmm : TPC::LD_TNSR_Dmp) : (IsVectorPredicate ? TPC::LD_TNSR_Dvm : TPC::LD_TNSR_Dvp));
        else
          Inst.setOpcode(IsVPRF ? (IsVectorPredicate ? TPC::LD_TNSRmm : TPC::LD_TNSRmp) : (IsVectorPredicate ? TPC::LD_TNSRvm : TPC::LD_TNSRvp));
      }
    }
  } else if (OpCode == TPCII::LD_TNSR_HIGH) {
    if (IsPartial) {
      assert(Bits[TPC::FeatureGen3Plus]);
      if (IsTnsrIsReg)
        Inst.setOpcode(IsVPRF ? (IsVectorPredicate ? TPC::LD_TNSR_HIGH_Pmm : TPC::LD_TNSR_HIGH_Pmp) : (IsVectorPredicate ? TPC::LD_TNSR_HIGH_Pvm : TPC::LD_TNSR_HIGH_Pvp));
      else {
        if (IsDirect)
          Inst.setOpcode(IsVPRF ? (IsVectorPredicate ? TPC::LD_TNSR_HIGH_PDmm : TPC::LD_TNSR_HIGH_PDmp) : (IsVectorPredicate ? TPC::LD_TNSR_HIGH_PDvm : TPC::LD_TNSR_HIGH_PDvp));
        else
          Inst.setOpcode(IsVPRF ? (IsVectorPredicate ? TPC::LD_TNSR_HIGH_PTmm : TPC::LD_TNSR_HIGH_PTmp) : (IsVectorPredicate ? TPC::LD_TNSR_HIGH_PTvm : TPC::LD_TNSR_HIGH_PTvp));
      }
    } else {
      if (IsTnsrIsReg) {
        if (IsGaudi || IsGaudiB)
          Inst.setOpcode(IsVPRF ? (IsVectorPredicate ? TPC::LD_TNSR_HIGHGen2Tmm : TPC::LD_TNSR_HIGHGen2Tmp) : (IsVectorPredicate ? TPC::LD_TNSR_HIGHGen2Tvm : TPC::LD_TNSR_HIGHGen2Tvp));
        else
          Inst.setOpcode(IsVPRF ? (IsVectorPredicate ? TPC::LD_TNSR_HIGHTmm : TPC::LD_TNSR_HIGHTmp) : (IsVectorPredicate ? TPC::LD_TNSR_HIGHTvm : TPC::LD_TNSR_HIGHTvp));
      } else {
        if (IsDirect)
          Inst.setOpcode(IsVPRF ? (IsVectorPredicate ? TPC::LD_TNSR_HIGH_Dmm : TPC::LD_TNSR_HIGH_Dmp) : (IsVectorPredicate ? TPC::LD_TNSR_HIGH_Dvm : TPC::LD_TNSR_HIGH_Dvp));
        else
          Inst.setOpcode(IsVPRF ? (IsVectorPredicate ? TPC::LD_TNSR_HIGHmm : TPC::LD_TNSR_HIGHmp) : (IsVectorPredicate ? TPC::LD_TNSR_HIGHvm : TPC::LD_TNSR_HIGHvp));
      }
    }
  } else if (OpCode == TPCII::LD_TNSR_LOW) {
    if (IsPartial) {
      assert(Bits[TPC::FeatureGen3Plus]);
      if (IsTnsrIsReg)
        Inst.setOpcode(IsVPRF ? (IsVectorPredicate ? TPC::LD_TNSR_LOW_PTmm : TPC::LD_TNSR_LOW_PTmp) : (IsVectorPredicate ? TPC::LD_TNSR_LOW_PTvm : TPC::LD_TNSR_LOW_PTvp));
      else {
        if (IsDirect)
          Inst.setOpcode(IsVPRF ? (IsVectorPredicate ? TPC::LD_TNSR_LOW_PDmm : TPC::LD_TNSR_LOW_PDmp) : (IsVectorPredicate ? TPC::LD_TNSR_LOW_PDvm : TPC::LD_TNSR_LOW_PDvp));
        else
          Inst.setOpcode(IsVPRF ? (IsVectorPredicate ? TPC::LD_TNSR_LOW_Pmm : TPC::LD_TNSR_LOW_Pmp) : (IsVectorPredicate ? TPC::LD_TNSR_LOW_Pvm : TPC::LD_TNSR_LOW_Pvp));
      }
    } else {
      if (IsTnsrIsReg) {
        if (IsGaudi || IsGaudiB)
          Inst.setOpcode(IsVPRF ? (IsVectorPredicate ? TPC::LD_TNSR_LOWGen2Tmm : TPC::LD_TNSR_LOWGen2Tmp) : (IsVectorPredicate ? TPC::LD_TNSR_LOWGen2Tvm : TPC::LD_TNSR_LOWGen2Tvp));
        else
          Inst.setOpcode(IsVPRF ? (IsVectorPredicate ? TPC::LD_TNSR_LOWTmm : TPC::LD_TNSR_LOWTmp) : (IsVectorPredicate ? TPC::LD_TNSR_LOWTvm : TPC::LD_TNSR_LOWTvp));
      } else {
        if (IsDirect)
          Inst.setOpcode(IsVPRF ? (IsVectorPredicate ? TPC::LD_TNSR_LOW_Dmm : TPC::LD_TNSR_LOW_Dmp) : (IsVectorPredicate ? TPC::LD_TNSR_LOW_Dvm : TPC::LD_TNSR_LOW_Dvp));
        else
          Inst.setOpcode(IsVPRF ? (IsVectorPredicate ? TPC::LD_TNSR_LOWmm : TPC::LD_TNSR_LOWmp) : (IsVectorPredicate ? TPC::LD_TNSR_LOWvm : TPC::LD_TNSR_LOWvp));
      }
    }
  } else if (OpCode == TPCII::LD_TNSR_CNVRT) {
    assert(IsDoron1);
    assert(!IsVPRF);
    if (IsPartial) {
      if (IsTnsrIsReg) {
        Inst.setOpcode(IsVectorPredicate ?
                       TPC::LD_TNSR_CNVRT_PTvm : TPC::LD_TNSR_CNVRT_PTvp);
      } else if (IsDirect) {
        Inst.setOpcode(IsVectorPredicate ?
                       TPC::LD_TNSR_CNVRT_PDvm : TPC::LD_TNSR_CNVRT_PDvp);
      } else {
        Inst.setOpcode(IsVectorPredicate ?
                       TPC::LD_TNSR_CNVRT_Pvm : TPC::LD_TNSR_CNVRT_Pvp);
      }
    } else {
      if (IsTnsrIsReg) {
        Inst.setOpcode(IsVectorPredicate ?
                       TPC::LD_TNSR_CNVRTTvm : TPC::LD_TNSR_CNVRTTvp);
      } else if (IsDirect) {
        Inst.setOpcode(IsVectorPredicate ?
                       TPC::LD_TNSR_CNVRT_Dvm : TPC::LD_TNSR_CNVRT_Dvp);
      } else {
        Inst.setOpcode(IsVectorPredicate ?
                       TPC::LD_TNSR_CNVRTvm : TPC::LD_TNSR_CNVRTvp);
      }
    }
  } else {
    llvm_unreachable("Unhandled case");
  }

  // Data
  if (DecodeVRFRegisterClass(Inst, Dest, Address, Decoder) == MCDisassembler::Fail)
    return MCDisassembler::Fail;

  // Index
  if (DecodeIRFRegisterClass(Inst, SrcA, Address, Decoder) == MCDisassembler::Fail)
    return MCDisassembler::Fail;

  // Tensor
  if (IsTnsrIsReg) {
    if (IsGaudi || IsGaudiB)
      Inst.addOperand(MCOperand::createReg(TPC::S27));
    else
      Inst.addOperand(MCOperand::createReg(TPC::LD_TNSR_ID_REG));
  } else {
    if (IsDirect) {
      if (DecodeSRFRegisterClass(Inst, SrcB, Address, Decoder) == MCDisassembler::Fail)
        return MCDisassembler::Fail;
    } else {
      Inst.addOperand(MCOperand::createImm(SrcB& 0b11111));
    }
  }

  // Start/Offset
  if (IsPartial) {
    if (IsGaudi || IsGaudiB)
      Inst.addOperand(MCOperand::createReg(TPC::S30));
    else
      Inst.addOperand(MCOperand::createReg(TPC::LD_PARTIAL_REG));
  }

  if (IsDoron1 && OpCode == TPCII::LD_TNSR_CNVRT) {
    bool ClipFP = Switches & 0b100;
    if (ClipFP) {
      Switches &= ~0b100;
      Switches |= TPCII::SW_CLIP_FP;
    }
  }
  if (IsDoron1 && !IsDirect) {
    unsigned AutoDimInc = (SrcB >> 5) & 0b111;
    Switches |= (AutoDimInc << 8);
  }
  Inst.addOperand(MCOperand::createImm(Switches));

  if (DecodeVRFRegisterClass(Inst, Dest, Address, Decoder) == MCDisassembler::Fail)
    return MCDisassembler::Fail;
  uint64_t PredValue = (Polarity << 5) | (IsVectorPredicate << 4) | Predicate;
  if (IsVectorPredicate) {
    if (decodeVPredicate(Inst, PredValue, Address, Decoder) == MCDisassembler::Fail)
      return MCDisassembler::Fail;
  } else {
    if (decodeSPredicate(Inst, PredValue, Address, Decoder) == MCDisassembler::Fail)
      return MCDisassembler::Fail;
  }

  return MCDisassembler::Success;
}

  static MCDisassembler::DecodeStatus decodeLdTnsrSt(MCInst &Inst, uint64_t insn,
                                                   uint64_t Address,
                                                   const void *Decoder) {
  auto *Disasm = static_cast<const TPCDisassembler *>(Decoder);
  const FeatureBitset &Bits = Disasm->getSubtargetInfo().getFeatureBits();
  bool isGen4plus = Bits[TPC::FeatureGen4Plus];
  bool IsDoron1 = (bool) Bits[TPC::FeatureDoron1];

  uint64_t OpCode    = fieldFromInstruction(insn, TPCII::StOpCodeStart, TPCII::StOpCodeSize);
  uint64_t SrcA      = fieldFromInstruction(insn, TPCII::StSrcAStart, TPCII::StSrcASize);
  uint64_t SrcB      = fieldFromInstruction(insn, TPCII::StSrcBStart, TPCII::StSrcBSize);
  uint64_t Dest      = fieldFromInstruction(insn, TPCII::StSrcCStart, TPCII::StSrcCSize);
  uint64_t Switches  = fieldFromInstruction(insn, isGen4plus ? TPCII::Gen4StSwitchesStart : TPCII::StSwitchesStart, isGen4plus ? TPCII::Gen4StSwitchesSize : TPCII::StSwitchesSize);
  uint64_t Predicate = fieldFromInstruction(insn, TPCII::StPredicateStart, TPCII::StPredicateSize);

  bool IsVectorPredicate = (bool)fieldFromInstruction(insn, TPCII::StVectorPredBit, 1);
  bool Polarity = (bool)fieldFromInstruction(insn, TPCII::StPolarityBit, 1);

  bool IsVPRF  = isVPRF(Dest, IsDoron1);

  const unsigned SW_TNSR_ID_REG = 1 << 3;

  bool IsPartial = false;
  bool IsTnsrIsReg = false;

  if (Switches & SW_TNSR_ID_REG) {
    IsTnsrIsReg = true;
    Switches &= ~SW_TNSR_ID_REG;
  }

  if (Switches & TPCII::SW_PARTIAL) {
    IsPartial = true;
    Switches &= ~TPCII::SW_PARTIAL;
  }

  bool IsDirect = Switches & TPCII::SW_DIRECT;

  if (OpCode == TPCII::stLD_TNSR) {
    if (IsPartial) {
      if (IsTnsrIsReg)
          Inst.setOpcode(IsVPRF ? (IsVectorPredicate ? TPC::LD_TNSR_ST_PTmm : TPC::LD_TNSR_ST_PTmp) : (IsVectorPredicate ? TPC::LD_TNSR_ST_PTvm : TPC::LD_TNSR_ST_PTvp));
      else {
        if (IsDirect)
          Inst.setOpcode(IsVPRF ? (IsVectorPredicate ? TPC::LD_TNSR_ST_PDmm : TPC::LD_TNSR_ST_PDmp) : (IsVectorPredicate ? TPC::LD_TNSR_ST_PDvm : TPC::LD_TNSR_ST_PDvp));
        else
          Inst.setOpcode(IsVPRF ? (IsVectorPredicate ? TPC::LD_TNSR_ST_Pmm : TPC::LD_TNSR_ST_Pmp) : (IsVectorPredicate ? TPC::LD_TNSR_ST_Pvm : TPC::LD_TNSR_ST_Pvp));
      }
    } else {
      if (IsTnsrIsReg)
        Inst.setOpcode(IsVPRF ? (IsVectorPredicate ? TPC::LD_TNSR_STTmm : TPC::LD_TNSR_STTmp) : (IsVectorPredicate ? TPC::LD_TNSR_STTvm : TPC::LD_TNSR_STTvp));
      else {
        if (IsDirect)
          Inst.setOpcode(IsVPRF ? (IsVectorPredicate ? TPC::LD_TNSR_ST_Dmm : TPC::LD_TNSR_ST_Dmp) : (IsVectorPredicate ? TPC::LD_TNSR_ST_Dvm : TPC::LD_TNSR_ST_Dvp));
        else
          Inst.setOpcode(IsVPRF ? (IsVectorPredicate ? TPC::LD_TNSR_STmm : TPC::LD_TNSR_STmp) : (IsVectorPredicate ? TPC::LD_TNSR_STvm : TPC::LD_TNSR_STvp));
      }
    }
  } else if (OpCode == TPCII::stLD_TNSR_HIGH) {
    if (IsPartial) {
      assert(Bits[TPC::FeatureLdInStore]);
      if (IsTnsrIsReg)
          Inst.setOpcode(IsVPRF ? (IsVectorPredicate ? TPC::LD_TNSR_HIGH_ST_PTmm : TPC::LD_TNSR_HIGH_ST_PTmp) : (IsVectorPredicate ? TPC::LD_TNSR_HIGH_ST_PTvm : TPC::LD_TNSR_HIGH_ST_PTvp));
      else {
        if (IsDirect)
          Inst.setOpcode(IsVPRF ? (IsVectorPredicate ? TPC::LD_TNSR_HIGH_ST_PDmm : TPC::LD_TNSR_HIGH_ST_PDmp) : (IsVectorPredicate ? TPC::LD_TNSR_HIGH_ST_PDvm : TPC::LD_TNSR_HIGH_ST_PDvp));
        else
          Inst.setOpcode(IsVPRF ? (IsVectorPredicate ? TPC::LD_TNSR_HIGH_ST_Pmm : TPC::LD_TNSR_HIGH_ST_Pmp) : (IsVectorPredicate ? TPC::LD_TNSR_HIGH_ST_Pvm : TPC::LD_TNSR_HIGH_ST_Pvp));
      }
    } else {
      if (IsTnsrIsReg)
        Inst.setOpcode(IsVPRF ? (IsVectorPredicate ? TPC::LD_TNSR_HIGH_STTmm : TPC::LD_TNSR_HIGH_STTmp) : (IsVectorPredicate ? TPC::LD_TNSR_HIGH_STTvm : TPC::LD_TNSR_HIGH_STTvp));
      else {
        if (IsDirect)
          Inst.setOpcode(IsVPRF ? (IsVectorPredicate ? TPC::LD_TNSR_HIGH_ST_Dmm : TPC::LD_TNSR_HIGH_ST_Dmp) : (IsVectorPredicate ? TPC::LD_TNSR_HIGH_ST_Dvm : TPC::LD_TNSR_HIGH_ST_Dvp));
        else
          Inst.setOpcode(IsVPRF ? (IsVectorPredicate ? TPC::LD_TNSR_HIGH_STmm : TPC::LD_TNSR_HIGH_STmp) : (IsVectorPredicate ? TPC::LD_TNSR_HIGH_STvm : TPC::LD_TNSR_HIGH_STvp));
      }
    }
  } else if (OpCode == TPCII::stLD_TNSR_LOW) {
    if (IsPartial) {
      assert(Bits[TPC::FeatureLdInStore]);
      if (IsTnsrIsReg)
          Inst.setOpcode(IsVPRF ? (IsVectorPredicate ? TPC::LD_TNSR_LOW_ST_PTmm : TPC::LD_TNSR_LOW_ST_PTmp) : (IsVectorPredicate ? TPC::LD_TNSR_LOW_ST_PTvm : TPC::LD_TNSR_LOW_ST_PTvp));
      else {
        if (IsDirect)
          Inst.setOpcode(IsVPRF ? (IsVectorPredicate ? TPC::LD_TNSR_LOW_ST_PDmm : TPC::LD_TNSR_LOW_ST_PDmp) : (IsVectorPredicate ? TPC::LD_TNSR_LOW_ST_PDvm : TPC::LD_TNSR_LOW_ST_PDvp));
        else
          Inst.setOpcode(IsVPRF ? (IsVectorPredicate ? TPC::LD_TNSR_LOW_ST_Pmm : TPC::LD_TNSR_LOW_ST_Pmp) : (IsVectorPredicate ? TPC::LD_TNSR_LOW_ST_Pvm : TPC::LD_TNSR_LOW_ST_Pvp));
      }
    } else {
      if (IsTnsrIsReg)
          Inst.setOpcode(IsVPRF ? (IsVectorPredicate ? TPC::LD_TNSR_LOW_STTmm : TPC::LD_TNSR_LOW_STTmp) : (IsVectorPredicate ? TPC::LD_TNSR_LOW_STTvm : TPC::LD_TNSR_LOW_STTvp));
      else {
        if (IsDirect)
          Inst.setOpcode(IsVPRF ? (IsVectorPredicate ? TPC::LD_TNSR_LOW_ST_Dmm : TPC::LD_TNSR_LOW_ST_Dmp) : (IsVectorPredicate ? TPC::LD_TNSR_LOW_ST_Dvm : TPC::LD_TNSR_LOW_ST_Dvp));
        else
          Inst.setOpcode(IsVPRF ? (IsVectorPredicate ? TPC::LD_TNSR_LOW_STmm : TPC::LD_TNSR_LOW_STmp) : (IsVectorPredicate ? TPC::LD_TNSR_LOW_STvm : TPC::LD_TNSR_LOW_STvp));
      }
    }
  } else if (OpCode == TPCII::stLD_TNSR_CNVRT) {
    assert(IsDoron1);
    assert(!IsVPRF);
    if (IsPartial) {
      if (IsTnsrIsReg) {
        Inst.setOpcode(IsVectorPredicate ? TPC::LD_TNSR_CNVRT_PT_ST_vm :
                       TPC::LD_TNSR_CNVRT_PT_ST_vp);
      } else if (IsDirect) {
        Inst.setOpcode(IsVectorPredicate ? TPC::LD_TNSR_CNVRT_PD_ST_vm :
                       TPC::LD_TNSR_CNVRT_PD_ST_vp);
      } else {
        Inst.setOpcode(IsVectorPredicate ? TPC::LD_TNSR_CNVRT_P_ST_vm :
                       TPC::LD_TNSR_CNVRT_P_ST_vp);
      }
    } else {
      if (IsTnsrIsReg) {
        Inst.setOpcode(IsVectorPredicate ? TPC::LD_TNSR_CNVRTT_ST_vm :
                       TPC::LD_TNSR_CNVRTT_ST_vp);
      } else if (IsDirect) {
        Inst.setOpcode(IsVectorPredicate ? TPC::LD_TNSR_CNVRT_D_ST_vm :
                       TPC::LD_TNSR_CNVRT_D_ST_vp);
      } else {
        Inst.setOpcode(IsVectorPredicate ? TPC::LD_TNSR_CNVRT_ST_vm :
                       TPC::LD_TNSR_CNVRT_ST_vp);
      }
    }
  } else {
    llvm_unreachable("Unhandled case");
  }

  // Data
  if (DecodeVRFRegisterClass(Inst, Dest, Address, Decoder) == MCDisassembler::Fail)
    return MCDisassembler::Fail;

  // Index
  if (DecodeIRFRegisterClass(Inst, SrcA, Address, Decoder) == MCDisassembler::Fail)
    return MCDisassembler::Fail;

  // Tensor
  if (IsTnsrIsReg)
    Inst.addOperand(MCOperand::createReg(TPC::LD_TNSR_ID_REG));
  else {
    if (IsDirect) {
      if (DecodeSRFRegisterClass(Inst, SrcB, Address, Decoder) ==
          MCDisassembler::Fail)
        return MCDisassembler::Fail;
    } else {
      Inst.addOperand(MCOperand::createImm(SrcB & 0b11111));
    }
  }

  // Start/Offset
  if (IsPartial)
      Inst.addOperand(MCOperand::createReg(TPC::LD_PARTIAL_REG));

  if (IsDoron1 && OpCode == TPCII::stLD_TNSR_CNVRT) {
    bool ClipFP = Switches & 0b100;
    if (ClipFP) {
      Switches &= ~0b100;
      Switches |= TPCII::SW_CLIP_FP;
    }
  }
  if (IsDoron1 && !IsDirect) {
    unsigned AutoDimInc = (SrcB >> 5) & 0b111;
    Switches |= (AutoDimInc << 8);
  }

  Inst.addOperand(MCOperand::createImm(Switches));

  if (DecodeVRFRegisterClass(Inst, Dest, Address, Decoder) == MCDisassembler::Fail)
    return MCDisassembler::Fail;

  uint64_t PredValue = (Polarity << 5) | (IsVectorPredicate << 4) | Predicate;
  if (IsVectorPredicate) {
    if (decodeVPredicate(Inst, PredValue, Address, Decoder) == MCDisassembler::Fail)
      return MCDisassembler::Fail;
  } else {
    if (decodeSPredicate(Inst, PredValue, Address, Decoder) == MCDisassembler::Fail)
      return MCDisassembler::Fail;
  }

  return MCDisassembler::Success;
}

static MCDisassembler::DecodeStatus decodeStTnsr(MCInst &Inst, uint64_t insn,
                                                 uint64_t Address, const void *Decoder) {
  auto *Disasm = static_cast<const TPCDisassembler *>(Decoder);
  const FeatureBitset &Bits = Disasm->getSubtargetInfo().getFeatureBits();
  bool isGen4plus = Bits[TPC::FeatureGen4Plus];
  bool IsDoron1 = (bool) Bits[TPC::FeatureDoron1];

  uint64_t OpCode = fieldFromInstruction(insn, TPCII::StOpCodeStart, TPCII::StOpCodeSize);
  uint64_t SrcA = fieldFromInstruction(insn, TPCII::StSrcAStart, TPCII::StSrcASize);
  uint64_t SrcB = fieldFromInstruction(insn, TPCII::StSrcBStart, TPCII::StSrcBSize);
  uint64_t SrcC = fieldFromInstruction(insn, TPCII::StSrcCStart, TPCII::StSrcCSize);
  uint64_t Switches  = fieldFromInstruction(insn, isGen4plus ? TPCII::Gen4StSwitchesStart : TPCII::StSwitchesStart, isGen4plus ? TPCII::Gen4StSwitchesSize : TPCII::StSwitchesSize);
  uint64_t Predicate = fieldFromInstruction(insn, TPCII::StPredicateStart, TPCII::StPredicateSize);
  bool IsVectorPredicate = (bool)fieldFromInstruction(insn, TPCII::StVectorPredBit, 1);
  bool Polarity = (bool)fieldFromInstruction(insn, TPCII::StPolarityBit, 1);

  bool IsGaudi = Bits[TPC::FeatureGaudi];
  bool IsGaudiB = Bits[TPC::FeatureGaudiB];
  bool IsVPRF = isVPRF(SrcC, IsDoron1);

  const unsigned SW_TNSR_ID_REG = (1 << 3);

  bool IsPartial = false;
  bool IsRMW = false;
  bool IsTnsrIsReg = false;

  if (Switches & SW_TNSR_ID_REG) {
    IsTnsrIsReg = true;
    Switches &= ~SW_TNSR_ID_REG;
  }
  if (Switches & TPCII::SW_PARTIAL) {
    IsPartial = true;
    Switches &= ~TPCII::SW_PARTIAL;
  }
  if (Switches & TPCII::SW_RMW_SEL) {
    IsRMW = true;
    Switches &= ~TPCII::SW_RMW_SEL;
  }

  assert(!IsVPRF || !IsRMW);

  bool IsDirect = Switches & TPCII::SW_DIRECT;

  if (OpCode == TPCII::ST_TNSR) {
    if (IsPartial) {
      if (IsTnsrIsReg) {
        if (IsRMW) {
          if (IsGaudi || IsGaudiB)
            Inst.setOpcode(TPC::ST_TNSR_PGen2RTvp);
          else
            Inst.setOpcode(TPC::ST_TNSR_PRTvp);
        } else {
          if (IsGaudi || IsGaudiB)
            Inst.setOpcode(IsVPRF ? TPC::ST_TNSR_PGen2Tmp : TPC::ST_TNSR_PGen2Tvp);
          else
            Inst.setOpcode(IsVPRF ? TPC::ST_TNSR_PTmp : TPC::ST_TNSR_PTvp);
        }
      } else {
        if (IsRMW) {
          if (IsGaudi || IsGaudiB)
            Inst.setOpcode(TPC::ST_TNSR_PGen2Rvp);
          else {
            if (IsDirect)
              Inst.setOpcode(TPC::ST_TNSR_PDRvp);
            else
              Inst.setOpcode(TPC::ST_TNSR_PRvp);
          }
        } else {
          if (IsGaudi || IsGaudiB)
            Inst.setOpcode(IsVPRF ? TPC::ST_TNSR_PGen2mp : TPC::ST_TNSR_PGen2vp);
          else {
            if (IsDirect)
              Inst.setOpcode(IsVPRF ? TPC::ST_TNSR_PDmp : TPC::ST_TNSR_PDvp);
            else
              Inst.setOpcode(IsVPRF ? TPC::ST_TNSR_Pmp : TPC::ST_TNSR_Pvp);
          }
        }
      }
    } else {
      if (IsTnsrIsReg) {
        if (IsRMW) {
          if (IsGaudi || IsGaudiB)
            Inst.setOpcode(TPC::ST_TNSR_RGen2Tvp);
          else
            Inst.setOpcode(TPC::ST_TNSR_RTvp);
        } else {
          if (IsGaudi || IsGaudiB)
            Inst.setOpcode(IsVPRF ? TPC::ST_TNSRGen2Tmp : TPC::ST_TNSRGen2Tvp);
          else
            Inst.setOpcode(IsVPRF ? TPC::ST_TNSRTmp : TPC::ST_TNSRTvp);
        }
      } else {
        if (IsRMW) {
          if (IsGaudi || IsGaudiB)
            Inst.setOpcode(TPC::ST_TNSR_RGen2vp);
          else {
            if (IsDirect)
              Inst.setOpcode(TPC::ST_TNSR_RDvp);
            else
              Inst.setOpcode(TPC::ST_TNSR_Rvp);
          }
        } else {
          if (IsDirect)
            Inst.setOpcode(IsVPRF ? TPC::ST_TNSR_Dmp : TPC::ST_TNSR_Dvp);
          else
            Inst.setOpcode(IsVPRF ? TPC::ST_TNSRmp : TPC::ST_TNSRvp);
        }
      }
    }
  }
  else if (OpCode == TPCII::ST_TNSR_HIGH) {
    if (IsTnsrIsReg) {
      if (IsRMW) {
        if (IsGaudi || IsGaudiB)
          Inst.setOpcode(TPC::ST_TNSR_HIGH_RGen2Tvp);
        else
          Inst.setOpcode(TPC::ST_TNSR_HIGH_RTvp);
      } else {
        if (IsGaudi || IsGaudiB)
          Inst.setOpcode(IsVPRF ? TPC::ST_TNSR_HIGHGen2Tmp : TPC::ST_TNSR_HIGHGen2Tvp);
        else
          Inst.setOpcode(IsVPRF ? TPC::ST_TNSR_HIGHTmp : TPC::ST_TNSR_HIGHTvp);
      }
    } else {
      if (IsRMW) {
        if (IsGaudi || IsGaudiB)
          Inst.setOpcode(TPC::ST_TNSR_HIGH_RGen2vp);
        else {
          if (IsDirect) {
            Inst.setOpcode(TPC::ST_TNSR_HIGH_RDvp);
          } else {
            Inst.setOpcode(TPC::ST_TNSR_HIGH_Rvp);
         }
        }
      } else {
        if (IsDirect)
          Inst.setOpcode(IsVPRF ? TPC::ST_TNSR_HIGH_Dmp : TPC::ST_TNSR_HIGH_Dvp);
        else
          Inst.setOpcode(IsVPRF ? TPC::ST_TNSR_HIGHmp : TPC::ST_TNSR_HIGHvp);
      }
    }
  }
  else if (OpCode == TPCII::ST_TNSR_LOW) {
    if (IsTnsrIsReg) {
      if (IsRMW) {
        if (IsGaudi || IsGaudiB)
          Inst.setOpcode(TPC::ST_TNSR_LOW_RGen2Tvp);
        else
          Inst.setOpcode(TPC::ST_TNSR_LOW_RTvp);
      } else {
        if (IsGaudi || IsGaudiB)
          Inst.setOpcode(IsVPRF ? TPC::ST_TNSR_LOWGen2Tmp : TPC::ST_TNSR_LOWGen2Tvp);
        else
          Inst.setOpcode(IsVPRF ? TPC::ST_TNSR_LOWTmp : TPC::ST_TNSR_LOWTvp);
      }
    } else {
      if (IsRMW) {
        if (IsGaudi || IsGaudiB)
          Inst.setOpcode(TPC::ST_TNSR_LOW_RGen2vp);
        else {
          if (IsDirect)
            Inst.setOpcode(TPC::ST_TNSR_LOW_RDvp);
          else
            Inst.setOpcode(TPC::ST_TNSR_LOW_Rvp);
        }
      } else {
        if (IsDirect)
          Inst.setOpcode(IsVPRF ? TPC::ST_TNSR_LOW_Dmp : TPC::ST_TNSR_LOW_Dvp);
        else
          Inst.setOpcode(IsVPRF ? TPC::ST_TNSR_LOWmp : TPC::ST_TNSR_LOWvp);
      }
    }
  }


  // Index
  if (DecodeIRFRegisterClass(Inst, SrcA, Address, Decoder) == MCDisassembler::Fail)
    return MCDisassembler::Fail;

  // Tensor
  if (IsTnsrIsReg) {
    if (IsGaudi || IsGaudiB)
      Inst.addOperand(MCOperand::createReg(TPC::S28));
    else
      Inst.addOperand(MCOperand::createReg(TPC::ST_TNSR_ID_REG));
  } else {
    if (IsDirect) {
      if (DecodeSRFRegisterClass(Inst, SrcB, Address, Decoder) == MCDisassembler::Fail)
        return MCDisassembler::Fail;
    }
    else
      Inst.addOperand(MCOperand::createImm(SrcB));
  }

  // Data
  if (DecodeVRFRegisterClass(Inst, SrcC, Address, Decoder) == MCDisassembler::Fail)
    return MCDisassembler::Fail;

  // RMW
  if (IsRMW) {
    if (IsGaudi || IsGaudiB)
      Inst.addOperand(MCOperand::createReg(TPC::S29));
    else
      Inst.addOperand(MCOperand::createReg(TPC::ST_RMW_REG));
  }

  // Start/Offset
  if (IsPartial) {
    if (IsGaudi || IsGaudiB)
      Inst.addOperand(MCOperand::createReg(TPC::S31));
    else
      Inst.addOperand(MCOperand::createReg(TPC::ST_PARTIAL_REG));
  }

  Inst.addOperand(MCOperand::createImm(Switches));

  uint64_t PredValue = (Polarity << 5) | (IsVectorPredicate << 4) | Predicate;
  if (IsVectorPredicate) {
    if (decodeVPredicate(Inst, PredValue, Address, Decoder) == MCDisassembler::Fail)
      return MCDisassembler::Fail;
  } else {
    if (decodeSPredicate(Inst, PredValue, Address, Decoder) == MCDisassembler::Fail)
      return MCDisassembler::Fail;
  }

  return MCDisassembler::Success;
}

static MCDisassembler::DecodeStatus decodeStTnsrSqz(MCInst &Inst, uint64_t insn,
                                                    uint64_t Address, const void *Decoder) {
  auto *Disasm = static_cast<const TPCDisassembler *>(Decoder);
  const FeatureBitset &Bits = Disasm->getSubtargetInfo().getFeatureBits();
  assert(Bits[TPC::FeatureGen4Plus]);

  uint64_t OpCode = fieldFromInstruction(insn, TPCII::StOpCodeStart, TPCII::StOpCodeSize);
  uint64_t SrcA = fieldFromInstruction(insn, TPCII::StSrcAStart, TPCII::StSrcASize);
  uint64_t SrcB = fieldFromInstruction(insn, TPCII::StSrcBStart, TPCII::StSrcBSize);
  uint64_t SrcC = fieldFromInstruction(insn, TPCII::StSrcCStart, TPCII::StSrcCSize);
  uint64_t Switches  = fieldFromInstruction(insn, TPCII::Gen4StSwitchesStart, TPCII::Gen4StSwitchesSize);
  uint64_t Predicate = fieldFromInstruction(insn, TPCII::StPredicateStart, TPCII::StPredicateSize);
  bool IsVectorPredicate = (bool)fieldFromInstruction(insn, TPCII::StVectorPredBit, 1);
  bool Polarity = (bool)fieldFromInstruction(insn, TPCII::StPolarityBit, 1);

  assert(OpCode == TPCII::ST_TNSR_SQZ && "Wrong OpCode");
  const unsigned SW_TNSR_ID_REG = (1 << 3);

  bool IsRMW = false;
  bool IsCntOnly = false;
  bool IsFlush = false;
  bool IsTnsrIsReg = false;

  if (Switches & TPCII::SW_RMW_SEL)
    IsRMW = true;
  if (Switches & TPCII::SW_CNT_ONLY)
    IsCntOnly = true;
  if (Switches & TPCII::SW_FLUSH)
    IsFlush = true;
  if (Switches & SW_TNSR_ID_REG) {
    IsTnsrIsReg = true;
    Switches &= ~SW_TNSR_ID_REG;
  }

  if (IsRMW)
    Inst.setOpcode(IsTnsrIsReg ? TPC::ST_TNSR_SQZ_R_T : TPC::ST_TNSR_SQZ_R);
  else
    Inst.setOpcode(IsTnsrIsReg ? TPC::ST_TNSR_SQZ_T : TPC::ST_TNSR_SQZ);

  // With this switch everything except squeeze index and predicate is ignored, so don't print it out
  if (IsCntOnly) {
    Inst.setOpcode(TPC::ST_TNSR_SQZ_CNT_ONLY);

    // Squeese
    const unsigned SqueeseIdMask = 0x07;
    unsigned SqueeseId = (SrcB >> 5) & SqueeseIdMask;
    Inst.addOperand(MCOperand::createImm(SqueeseId));

    // Switches
    Inst.addOperand(MCOperand::createImm(Switches));

    // Predicate
    uint64_t PredValue = (Polarity << 5) | (IsVectorPredicate << 4) | Predicate;
    if (decodeVPredicate(Inst, PredValue, Address, Decoder) == MCDisassembler::Fail)
      return MCDisassembler::Fail;

    return MCDisassembler::Success;
  }

  // Analogous to CntOnly, only squeeze index is relevant
  if (IsFlush) {
    Inst.setOpcode(TPC::ST_TNSR_SQZ_FLUSH);

    // Squeese
    const unsigned SqueeseIdMask = 0x07;
    unsigned SqueeseId = (SrcB >> 5) & SqueeseIdMask;
    Inst.addOperand(MCOperand::createImm(SqueeseId));

    // Switches
    Inst.addOperand(MCOperand::createImm(Switches));

    return MCDisassembler::Success;
  }

  // Index
  if (DecodeIRFRegisterClass(Inst, SrcA, Address, Decoder) == MCDisassembler::Fail)
    return MCDisassembler::Fail;

  // Tensor
  const unsigned TensorIdMask = 0x1f;
  if (IsTnsrIsReg)
    Inst.addOperand(MCOperand::createReg(TPC::ST_TNSR_ID_REG));
  else {
    unsigned TensorId = SrcB & TensorIdMask;
    Inst.addOperand(MCOperand::createImm(TensorId));
  }

  // Squeese
  const unsigned SqueeseIdMask = 0x07;
  unsigned SqueeseId = (SrcB >> 5) & SqueeseIdMask;
  Inst.addOperand(MCOperand::createImm(SqueeseId));

  // Data
  if (DecodeVRFRegisterClass(Inst, SrcC, Address, Decoder) == MCDisassembler::Fail)
    return MCDisassembler::Fail;

  // RMW
  if (IsRMW)
    Inst.addOperand(MCOperand::createReg(TPC::ST_RMW_REG));

  // Switches
  Inst.addOperand(MCOperand::createImm(Switches));

  // Predicate
  uint64_t PredValue = (Polarity << 5) | (IsVectorPredicate << 4) | Predicate;
  if (decodeVPredicate(Inst, PredValue, Address, Decoder) == MCDisassembler::Fail)
    return MCDisassembler::Fail;

  return MCDisassembler::Success;
}

static MCDisassembler::DecodeStatus decodeStTnsrS(MCInst &Inst, uint64_t insn,
                                                  uint64_t Address, const void *Decoder) {
  auto *Disasm = static_cast<const TPCDisassembler *>(Decoder);
  const FeatureBitset &Bits = Disasm->getSubtargetInfo().getFeatureBits();
  assert(Bits[TPC::FeatureGen4Plus]);

  uint64_t OpCode = fieldFromInstruction(insn, TPCII::StOpCodeStart, TPCII::StOpCodeSize);
  uint64_t SrcA = fieldFromInstruction(insn, TPCII::StSrcAStart, TPCII::StSrcASize);
  uint64_t SrcB = fieldFromInstruction(insn, TPCII::StSrcBStart, TPCII::StSrcBSize);
  uint64_t SrcC = fieldFromInstruction(insn, TPCII::StSrcCStart, TPCII::StSrcCSize);
  uint64_t Switches  = fieldFromInstruction(insn, TPCII::Gen4StSwitchesStart, TPCII::Gen4StSwitchesSize);
  uint64_t Predicate = fieldFromInstruction(insn, TPCII::StPredicateStart, TPCII::StPredicateSize);
  bool IsVectorPredicate = (bool)fieldFromInstruction(insn, TPCII::StVectorPredBit, 1);
  bool Polarity = (bool)fieldFromInstruction(insn, TPCII::StPolarityBit, 1);

  assert(OpCode == TPCII::ST_TNSR_S && "Wrong OpCode");
  assert(!IsVectorPredicate && "Only a scalar predicate available");

  const unsigned SW_TNSR_ID_REG = (1 << 3);

  bool IsHwReg = false;
  bool IsRMW = false;
  bool IsBv64 = false;
  bool IsTnsrIsReg = false;

  if (Switches & TPCII::SW_HW_REG)
    IsHwReg = true;
  if (Switches & TPCII::SW_RMW_SEL)
    IsRMW = true;
  if (Switches & TPCII::SW_ST_TNSR_S_BV64)
    IsBv64 = true;
  if (Switches & SW_TNSR_ID_REG) {
    IsTnsrIsReg = true;
    Switches &= ~SW_TNSR_ID_REG;
  }

  if (IsHwReg) {
    if (IsRMW) {
      if (IsTnsrIsReg)
        Inst.setOpcode(TPC::ST_TNSR_S_HWR_Rr_Dis);
      else
        Inst.setOpcode(TPC::ST_TNSR_S_HWR_Ri_Dis);
    } else {
      if (IsTnsrIsReg)
        Inst.setOpcode(TPC::ST_TNSR_S_HWRr_Dis);
      else
        Inst.setOpcode(TPC::ST_TNSR_S_HWRi_Dis);
    }
  } else {
    if (IsRMW) {
      if (IsTnsrIsReg)
        Inst.setOpcode(TPC::ST_TNSR_S_Rsr_Dis);
      else
        Inst.setOpcode(TPC::ST_TNSR_S_Rsi_Dis);
    } else {
      if (IsTnsrIsReg)
        Inst.setOpcode(TPC::ST_TNSR_Ssr_Dis);
      else
        Inst.setOpcode(TPC::ST_TNSR_Ssi_Dis);
    }
  }

  // Index
  if (DecodeIRFRegisterClass(Inst, SrcA, Address, Decoder) == MCDisassembler::Fail)
    return MCDisassembler::Fail;

  // Tensor
  const unsigned TensorIdMask = 0x1f;
  if (IsTnsrIsReg)
    Inst.addOperand(MCOperand::createReg(TPC::ST_TNSR_ID_REG));
  else {
    unsigned TensorId = SrcB & TensorIdMask;
    Inst.addOperand(MCOperand::createImm(TensorId));
  }

  if (IsHwReg) {
    // Squeese
    const unsigned SqueeseIdMask = 0x07;
    unsigned SqueeseId = (SrcB >> 5) & SqueeseIdMask;
    DecodeHWSqzCntrRegisterClass(Inst, SqueeseId, Address, Decoder);
  } else {
    // Data
    if (DecodeSRFRegisterClass(Inst, SrcC, Address, Decoder) == MCDisassembler::Fail)
      return MCDisassembler::Fail;
  }

  // RMW
  if (IsRMW)
    Inst.addOperand(MCOperand::createReg(TPC::ST_RMW_REG));

  // Switches
  Inst.addOperand(MCOperand::createImm(Switches));

  // Predicate
  uint64_t PredValue = (Polarity << 5) | (IsVectorPredicate << 4) | Predicate;
  if (decodeSPredicate(Inst, PredValue, Address, Decoder) == MCDisassembler::Fail)
    return MCDisassembler::Fail;

  return MCDisassembler::Success;
}

static MCDisassembler::DecodeStatus decodeMovDualGroup(MCInst &Inst, uint64_t insn,
                                                       uint64_t Address, const void *Decoder) {
  uint64_t Opcode = fieldFromInstruction(insn, TPCII::VpuOpCodeStart, TPCII::VpuOpCodeSize);
  uint64_t SrcA = fieldFromInstruction(insn, TPCII::VpuSrcAStart, TPCII::VpuSrcASize);
  uint64_t SrcB = fieldFromInstruction(insn, TPCII::VpuSrcBStart, TPCII::VpuSrcBSize);
  uint64_t SrcC = fieldFromInstruction(insn, TPCII::VpuSrcCStart, TPCII::VpuSrcCSize);
  uint64_t Dest = fieldFromInstruction(insn, TPCII::VpuDestStart, TPCII::VpuDestSize);
  uint64_t Switches = fieldFromInstruction(insn, TPCII::VpuSwitches1Start, TPCII::VpuSwitches1Size);
  Switches = Switches | (fieldFromInstruction(insn, TPCII::VpuSwitches2Start, TPCII::VpuSwitches2Size) <<3);
  uint64_t Predicate = fieldFromInstruction(insn, TPCII::VpuPredicateStart, TPCII::VpuPredicateSize);
  bool IsVectorPredicate = (bool)fieldFromInstruction(insn, TPCII::VpuVectorPredBit, 1);
  bool Polarity = (bool)fieldFromInstruction(insn, TPCII::VpuPolarityBit, 1);
  assert(Opcode == TPCII::vpuMOV_DUAL_GROUP);

  unsigned Type = Switches & TPCII::SW_MDG_TYPE_MASK;
  bool ctrl_reg = Switches & TPCII::SW_MDG_CTRL_REG;
  switch (Type) {
  case TPCII::SW_MDG_TYPE_SINGLE:
    Inst.setOpcode(IsVectorPredicate ?
                     (ctrl_reg ? TPC::MOV_DUAL_GROUP_CTRL_REGm_Dis : TPC::MOV_DUAL_GROUPm_Dis) :
                     (ctrl_reg ? TPC::MOV_DUAL_GROUP_CTRL_REGp_Dis : TPC::MOV_DUAL_GROUPp_Dis));
    break;
  case TPCII::SW_MDG_TYPE_ALL:
    Inst.setOpcode(IsVectorPredicate ?
                     (ctrl_reg ? TPC::MOV_DUAL_GROUP_CTRL_REG_ALLm_Dis : TPC::MOV_DUAL_GROUP_ALLm_Dis) :
                     (ctrl_reg ? TPC::MOV_DUAL_GROUP_CTRL_REG_ALLp_Dis : TPC::MOV_DUAL_GROUP_ALLp_Dis));
    break;
  case TPCII::SW_MDG_TYPE_PACK:
    Inst.setOpcode(IsVectorPredicate ?
                     TPC::MOV_DUAL_GROUP_PACKm_Dis :
                     TPC::MOV_DUAL_GROUP_PACKp_Dis);
    break;
  case TPCII::SW_MDG_TYPE_UNPACK:
    Inst.setOpcode(IsVectorPredicate ?
                     TPC::MOV_DUAL_GROUP_UNPACKm_Dis :
                     TPC::MOV_DUAL_GROUP_UNPACKp_Dis);
    break;
  }

  if (DecodeVRFRegisterClass(Inst, Dest, Address, Decoder) == MCDisassembler::Fail)
    return MCDisassembler::Fail;
  if (DecodeVRFRegisterClass(Inst, SrcA, Address, Decoder) == MCDisassembler::Fail)
    return MCDisassembler::Fail;
  if (Type != TPCII::SW_MDG_TYPE_PACK) {
    auto *Disasm = static_cast<const TPCDisassembler *>(Decoder);
    Inst.addOperand(MCOperand::createImm(Disasm->getImmediate()));
  }

  unsigned SwitchSet = Switches;
  if (ctrl_reg) {
    if (DecodeSRFRegisterClass(Inst, SrcB, Address, Decoder) == MCDisassembler::Fail)
      return MCDisassembler::Fail;
  } else {
    SwitchSet = Switches | (SrcB << 8);
  }

  if (Type == TPCII::SW_MDG_TYPE_ALL || Type == TPCII::SW_MDG_TYPE_UNPACK)
    SwitchSet |= (SrcC << 16);

  Inst.addOperand(MCOperand::createImm(SwitchSet));

  if (DecodeVRFRegisterClass(Inst, Dest, Address, Decoder) == MCDisassembler::Fail)
    return MCDisassembler::Fail;

  uint64_t PredValue = (Polarity << 5) | (IsVectorPredicate << 4) | Predicate;
  if (IsVectorPredicate) {
    if (decodeVPredicate(Inst, PredValue, Address, Decoder) == MCDisassembler::Fail)
      return MCDisassembler::Fail;
  } else {
    if (decodeSPredicate(Inst, PredValue, Address, Decoder) == MCDisassembler::Fail)
      return MCDisassembler::Fail;
  }
  return MCDisassembler::Success;
}

static MCDisassembler::DecodeStatus decodeUdivAll(MCInst &Inst, uint64_t insn,
                                                       uint64_t Address, const void *Decoder) {
    uint64_t Opcode = fieldFromInstruction(insn, TPCII::SpuOpCodeStart, TPCII::SpuOpCodeSize);
    uint64_t SrcA = fieldFromInstruction(insn, TPCII::SpuSrcAStart, TPCII::SpuSrcASize);
    uint64_t SrcB = fieldFromInstruction(insn, TPCII::SpuSrcBStart, TPCII::SpuSrcBSize);
    uint64_t Dest = fieldFromInstruction(insn, TPCII::SpuDestStart, TPCII::SpuDestSize);
    uint64_t Optype = fieldFromInstruction(insn, TPCII::SpuOpTypeStart, TPCII::SpuOpTypeSize);
    uint64_t Switches = fieldFromInstruction(insn, TPCII::SpuSwitchesStart, TPCII::SpuSwitchesSize);
    uint64_t Predicate = fieldFromInstruction(insn, TPCII::SpuPredicateStart, TPCII::SpuPredicateSize);
    bool Polarity = (bool)fieldFromInstruction(insn, TPCII::SpuPolarityBit, 1);
    assert(Opcode == TPCII::spuUDIV_STEP);
    (void) Opcode;

    auto *Disasm = static_cast<const TPCDisassembler *>(Decoder);
    const FeatureBitset &Bits = Disasm->getSubtargetInfo().getFeatureBits();

    if (Bits[TPC::FeatureGoya]) {
        Inst.setOpcode(TPC::UDIV_STEP);
    } else if (Bits[TPC::FeatureGaudi] || Bits[TPC::FeatureGaudiB]) {
      Inst.setOpcode(TPC::UDIV_4STEP);
    } else if (Bits[TPC::FeatureGreco]) {
      Inst.setOpcode(Switches & TPCII::SW_STEP_REG ? TPC::UDIV_4STEPT : TPC::UDIV_4STEP);
    } else { // Gaudi2 or Higher
      Inst.setOpcode(TPC::UDIV_DIS);
    }

    if (!Bits[TPC::FeatureGen4Plus] || Switches & TPCII::SW_DIV_MODE_BOTH) {
      if (DecodeZRFRegisterClass(Inst, Dest, Address, Decoder) == MCDisassembler::Fail)
          return MCDisassembler::Fail;
    } else {
      if (DecodeSRFRegisterClass(Inst, Dest, Address, Decoder) == MCDisassembler::Fail)
          return MCDisassembler::Fail;
    }

    if (DecodeSRFRegisterClass(Inst, SrcA, Address, Decoder) == MCDisassembler::Fail)
        return MCDisassembler::Fail;

    if (!Bits[TPC::FeatureGen4Plus]) {
      if ((Switches & TPCII::SW_STEP_REG)) {
          Inst.addOperand(MCOperand::createReg(TPC::DIV_STEP));
      } else {
          Inst.addOperand(MCOperand::createImm(Switches & ((1ULL<<5)-1))); //get step from switches
      }
    } else {
      if (DecodeSRFRegisterClass(Inst, SrcB, Address, Decoder) == MCDisassembler::Fail)
          return MCDisassembler::Fail;
    }

    Inst.addOperand(MCOperand::createImm(Optype));
    if (!Bits[TPC::FeatureGen4Plus])
      Inst.addOperand(MCOperand::createImm(Switches & (1ULL<<6)));
    else
      Inst.addOperand(MCOperand::createImm(Switches));
    if (!Bits[TPC::FeatureGen4Plus] || Switches & TPCII::SW_DIV_MODE_BOTH) {
      if (DecodeZRFRegisterClass(Inst, Dest, Address, Decoder) == MCDisassembler::Fail)
          return MCDisassembler::Fail;
    } else {
      if (DecodeSRFRegisterClass(Inst, Dest, Address, Decoder) == MCDisassembler::Fail)
          return MCDisassembler::Fail;
    }

    uint64_t PredValue = (Polarity << 5) | Predicate;
    if (decodeSPredicate(Inst, PredValue, Address, Decoder) == MCDisassembler::Fail)
            return MCDisassembler::Fail;
    return MCDisassembler::Success;
}

static MCDisassembler::DecodeStatus decodeLookupLutPtr(MCInst &Inst, uint64_t insn,
                                                   uint64_t Address, const void *Decoder) {
  uint64_t Opcode = fieldFromInstruction(insn, TPCII::LdOpCodeStart, TPCII::LdOpCodeSize);
  uint64_t SrcA = fieldFromInstruction(insn, TPCII::LdSrcAStart, TPCII::LdSrcASize);
  uint64_t SrcExtra = fieldFromInstruction(insn, TPCII::LdSrcBStart, TPCII::LdSrcBSize);
  uint64_t Dest = fieldFromInstruction(insn, TPCII::LdDestStart, TPCII::LdDestSize);
  uint64_t Switches = fieldFromInstruction(insn, TPCII::LdSwitchesStart, TPCII::LdSwitchesSize);
  uint64_t Predicate = fieldFromInstruction(insn, TPCII::LdPredicateStart, TPCII::LdPredicateSize);
  bool Polarity = (bool)fieldFromInstruction(insn, TPCII::LdPolarityBit, 1);
  assert(Opcode == TPCII::LOOKUP || Opcode == TPCII::LOOKUP_1C || Opcode == TPCII::LOOKUP_2C);

  if (Opcode == TPCII::LOOKUP_1C || Opcode == TPCII::LOOKUP){
    if (DecodeVRFRegisterClass(Inst, Dest, Address, Decoder) == MCDisassembler::Fail)
      return MCDisassembler::Fail;
  } else {
    if (DecodeDRFRegisterClass(Inst, Dest, Address, Decoder) == MCDisassembler::Fail)
      return MCDisassembler::Fail;
  }
  if (DecodeVRFRegisterClass(Inst, SrcA, Address, Decoder) == MCDisassembler::Fail)
    return MCDisassembler::Fail;

  if (Switches & TPCII::SW_LUT_PTR) {
    if (DecodeSRFRegisterClass(Inst, SrcExtra, Address, Decoder) == MCDisassembler::Fail)
      return MCDisassembler::Fail;
  } else {
    return MCDisassembler::Fail;
  }

  Inst.addOperand(MCOperand::createImm(Switches));

  if (Opcode == TPCII::LOOKUP_1C || Opcode == TPCII::LOOKUP){
    if (DecodeVRFRegisterClass(Inst, Dest, Address, Decoder) == MCDisassembler::Fail)
      return MCDisassembler::Fail;
  } else {
    if (DecodeDRFRegisterClass(Inst, Dest, Address, Decoder) == MCDisassembler::Fail)
      return MCDisassembler::Fail;
  }
  uint64_t PredValue = (Polarity << 5) | Predicate;
  if (decodeSPredicate(Inst, PredValue, Address, Decoder) == MCDisassembler::Fail)
    return MCDisassembler::Fail;
  return MCDisassembler::Success;
}


static MCDisassembler::DecodeStatus decodeMovGroup(MCInst &Inst, uint64_t insn,
                                                   uint64_t Address,
                                                   const void *Decoder) {
  uint64_t Opcode = fieldFromInstruction(insn, 0, 6);

  if (Opcode != 50) {
    return MCDisassembler::Fail;
  }

  uint64_t tmp;

  tmp = fieldFromInstruction(insn, 39, 8);
  if (DecodeVRFRegisterClass(Inst, tmp, Address, Decoder) ==
      MCDisassembler::Fail) {
    return MCDisassembler::Fail;
  }

  tmp = fieldFromInstruction(insn, 6, 8);
  if (DecodeVRFRegisterClass(Inst, tmp, Address, Decoder) ==
      MCDisassembler::Fail) {
    return MCDisassembler::Fail;
  }

  auto *Disasm = static_cast<const TPCDisassembler *>(Decoder);
  Inst.addOperand(MCOperand::createImm(Disasm->getImmediate()));

  tmp = fieldFromInstruction(insn, 14, 6);
  Inst.addOperand(MCOperand::createImm(tmp));

  tmp = fieldFromInstruction(insn, 39, 8);
  if (DecodeVRFRegisterClass(Inst, tmp, Address, Decoder) ==
      MCDisassembler::Fail) {
    return MCDisassembler::Fail;
  }

  tmp = 0;
  tmp |= fieldFromInstruction(insn, 54, 1) << 5;
  tmp |= fieldFromInstruction(insn, 55, 4) << 0;

  if (fieldFromInstruction(insn, 59, 1) == 1) {
    if (decodeVPredicate(Inst, tmp, Address, Decoder) == MCDisassembler::Fail) {
      return MCDisassembler::Fail;
    }
  } else {
    if (decodeSPredicate(Inst, tmp, Address, Decoder) == MCDisassembler::Fail) {
      return MCDisassembler::Fail;
    }
  }
  
  return MCDisassembler::Success;
}

static MCDisassembler::DecodeStatus decodeMacZp(MCInst &Inst, uint64_t insn,
                                                uint64_t Address, const void *Decoder) {
  uint64_t Opcode = fieldFromInstruction(insn, TPCII::VpuOpCodeStart, TPCII::VpuOpCodeSize);
  uint64_t SrcA = fieldFromInstruction(insn, TPCII::VpuSrcAStart, TPCII::VpuSrcASize);
  uint64_t SrcB = fieldFromInstruction(insn, TPCII::VpuSrcBStart, TPCII::VpuSrcBSize);
  uint64_t Dest = fieldFromInstruction(insn, TPCII::VpuDestStart, TPCII::VpuDestSize);
  uint64_t Optype = fieldFromInstruction(insn, TPCII::VpuOpTypeStart, TPCII::VpuOpTypeSize);
  uint64_t Switches = fieldFromInstruction(insn, TPCII::VpuSwitches1Start, TPCII::VpuSwitches1Size);
  Switches = Switches | (fieldFromInstruction(insn, TPCII::VpuSwitches2Start, TPCII::VpuSwitches2Size) <<3);
  uint64_t Predicate = fieldFromInstruction(insn, TPCII::VpuPredicateStart, TPCII::VpuPredicateSize);
  bool VectorPredicate = (bool) fieldFromInstruction(insn, TPCII::VpuVectorPredBit, 1);
  bool Polarity = (bool)fieldFromInstruction(insn, TPCII::VpuPolarityBit, 1);
  assert(Opcode == TPCII::vpuMAC);
  (void) Opcode;

  bool IsAccI16 = Switches & TPCII::SW_ACC_I16;
  bool IsX2 = Switches & TPCII::SW_X2_ARITHMETIC;
  bool IsZP = Switches & TPCII::SW_ZP;

  assert(IsZP && "Custom decoder is used only for MAC with ZP");
  (void) IsZP;

  if (Optype == TPCII::OpType::INT8) {
    if (IsAccI16) {
      if (IsX2)
        Inst.setOpcode(VectorPredicate ? TPC::MACAx2zpi8m_Dis : TPC::MACAx2zpi8p_Dis );
      else
        Inst.setOpcode(VectorPredicate ? TPC::MACAzpi8m_Dis : TPC::MACAzpi8p_Dis );
    } else {
      if (IsX2)
        Inst.setOpcode(VectorPredicate ? TPC::MACx2zpi8m_Dis : TPC::MACx2zpi8p_Dis );
      else
        Inst.setOpcode(VectorPredicate ? TPC::MACzpi8m_Dis : TPC::MACzpi8p_Dis );
    }
  } else if (Optype == TPCII::OpType::UINT8) {
    if (IsAccI16) {
      if (IsX2)
        Inst.setOpcode(VectorPredicate ? TPC::MACAx2zpu8m_Dis : TPC::MACAx2zpu8p_Dis );
      else
        Inst.setOpcode(VectorPredicate ? TPC::MACAzpu8m_Dis : TPC::MACAzpu8p_Dis );
    } else {
      if (IsX2)
        Inst.setOpcode(VectorPredicate ? TPC::MACx2zpu8m_Dis : TPC::MACx2zpu8p_Dis );
      else
        Inst.setOpcode(VectorPredicate ? TPC::MACzpu8m_Dis : TPC::MACzpu8p_Dis );
    }
  } else {
    return MCDisassembler::Fail;
  }
  if (IsAccI16) {
    if (DecodeDRFRegisterClass(Inst, Dest, Address, Decoder) == MCDisassembler::Fail)
      return MCDisassembler::Fail;
  } else {
    if (DecodeARFRegisterClass(Inst, Dest, Address, Decoder) == MCDisassembler::Fail)
      return MCDisassembler::Fail;
  }

  if (DecodeVRFRegisterClass(Inst, SrcA, Address, Decoder) == MCDisassembler::Fail)
    return MCDisassembler::Fail;
  if (DecodeVRFRegisterClass(Inst, SrcB, Address, Decoder) == MCDisassembler::Fail)
    return MCDisassembler::Fail;
  if(IsX2) {
    uint64_t SrcC = fieldFromInstruction(insn, TPCII::VpuSrcCStart, TPCII::VpuSrcCSize);
    uint64_t SrcD = fieldFromInstruction(insn, TPCII::VpuSrcDStart, TPCII::VpuSrcDSize);
    if (DecodeVRFRegisterClass(Inst, SrcC, Address, Decoder) == MCDisassembler::Fail)
      return MCDisassembler::Fail;
    if (DecodeVRFRegisterClass(Inst, SrcD, Address, Decoder) == MCDisassembler::Fail)
      return MCDisassembler::Fail;
  }
  Inst.addOperand(MCOperand::createReg(TPC::ZP_REG));

  Inst.addOperand(MCOperand::createImm(Switches & ~(TPCII::SW_ZP | TPCII::SW_ACC_I16 | TPCII::SW_X2_ARITHMETIC)));

  if (IsAccI16) {
    if (DecodeDRFRegisterClass(Inst, Dest, Address, Decoder) == MCDisassembler::Fail)
      return MCDisassembler::Fail;
  } else {
    if (DecodeARFRegisterClass(Inst, Dest, Address, Decoder) == MCDisassembler::Fail)
      return MCDisassembler::Fail;
  }

  uint64_t PredValue = (Polarity << 5) | (VectorPredicate << 4)| Predicate;
  if (VectorPredicate) {
    if (decodeVPredicate(Inst, PredValue, Address, Decoder) == MCDisassembler::Fail)
      return MCDisassembler::Fail;
  } else {
    if (decodeSPredicate(Inst, PredValue, Address, Decoder) == MCDisassembler::Fail)
      return MCDisassembler::Fail;
  }
  return MCDisassembler::Success;
}

static MCDisassembler::DecodeStatus decodeMacMulX2(MCInst &Inst, uint64_t insn,
                                                   uint64_t Address,
                                                   const void *Decoder) {

  uint64_t OpCode = fieldFromInstruction(insn, TPCII::VpuOpCodeStart, TPCII::VpuOpCodeEnd);
  uint64_t SrcA = fieldFromInstruction(insn, TPCII::VpuSrcAStart, TPCII::VpuSrcASize);
  uint64_t SrcB = fieldFromInstruction(insn, TPCII::VpuSrcBStart, TPCII::VpuSrcBSize);
  uint64_t SrcD = fieldFromInstruction(insn, TPCII::VpuSrcDStart, TPCII::VpuSrcDSize);
  uint64_t Dest = fieldFromInstruction(insn, TPCII::VpuDestStart, TPCII::VpuDestSize);
  uint64_t OpType = fieldFromInstruction(insn, TPCII::VpuOpTypeStart, TPCII::VpuOpTypeSize);
  uint64_t Switches = fieldFromInstruction(insn, TPCII::VpuSwitches1Start, TPCII::VpuSwitches1Size);
  Switches = Switches | (fieldFromInstruction(insn, TPCII::VpuSwitches2Start, TPCII::VpuSwitches2Size) << TPCII::VpuSwitches1Size);
  uint64_t Predicate = fieldFromInstruction(insn, TPCII::VpuPredicateStart, TPCII::VpuPredicateSize);
  bool IsVectorPredicate = (bool)fieldFromInstruction(insn, TPCII::VpuVectorPredBit, 1);
  bool Polarity = (bool)fieldFromInstruction(insn, TPCII::VpuPolarityBit, 1);

  auto *Disasm = static_cast<const TPCDisassembler *>(Decoder);
  const FeatureBitset &Bits = Disasm->getSubtargetInfo().getFeatureBits();

  assert(OpCode == TPCII::vpuMAC || OpCode == TPCII::vpuMUL);

  if (OpType != TPCII::OpType::FP32)
    return MCDisassembler::Fail;
  if ((Switches & TPCII::SW_X2_ARITHMETIC) == 0)
    return MCDisassembler::Fail;
  if (!(Bits[TPC::FeatureGaudi2] == 0 || Bits[TPC::FeatureDoron1] == 0))
    return MCDisassembler::Fail;

  switch (OpCode) {
  case TPCII::vpuMAC:
    if (IsVectorPredicate)
      Inst.setOpcode(TPC::MACx2f32vvvm_Dis);
    else
      Inst.setOpcode(TPC::MACx2f32vvvp_Dis);
    break;
  case TPCII::vpuMUL:
    if (IsVectorPredicate)
      Inst.setOpcode(TPC::MULx2f32vvvm_Dis);
    else
      Inst.setOpcode(TPC::MULx2f32vvvp_Dis);
    break;
  default:
    llvm_unreachable("Unhandled case");
  }

  if (DecodeDRFRegisterClass(Inst, Dest, Address, Decoder) == MCDisassembler::Fail)
    return MCDisassembler::Fail;

  if (DecodeDRFRegisterClass(Inst, SrcA, Address, Decoder) == MCDisassembler::Fail)
    if (DecodeSRFRegisterClass(Inst, SrcA, Address, Decoder) == MCDisassembler::Fail)
      return MCDisassembler::Fail;
  if (DecodeVRFRegisterClass(Inst, SrcB, Address, Decoder) == MCDisassembler::Fail)
    return MCDisassembler::Fail;
  if (DecodeVRFRegisterClass(Inst, SrcD, Address, Decoder) == MCDisassembler::Fail)
    return MCDisassembler::Fail;

  Inst.addOperand(MCOperand::createImm(OpType));
  Inst.addOperand(MCOperand::createImm(Switches));

  //Income
  DecodeDRFRegisterClass(Inst, Dest, Address, Decoder);

  uint64_t PredValue = (Polarity << 5) | (IsVectorPredicate << 4)| Predicate;
  if (IsVectorPredicate) {
    if (decodeVPredicate(Inst, PredValue, Address, Decoder) == MCDisassembler::Fail)
      return MCDisassembler::Fail;
  } else {
    if (decodeSPredicate(Inst, PredValue, Address, Decoder) == MCDisassembler::Fail)
      return MCDisassembler::Fail;
  }

  return MCDisassembler::Success;
}

static MCDisassembler::DecodeStatus decodeMaddZp(MCInst &Inst, uint64_t insn,
                                                uint64_t Address, const void *Decoder) {
  uint64_t Opcode = fieldFromInstruction(insn, TPCII::VpuOpCodeStart, TPCII::VpuOpCodeSize);
  uint64_t SrcA = fieldFromInstruction(insn, TPCII::VpuSrcAStart, TPCII::VpuSrcASize);
  uint64_t SrcB = fieldFromInstruction(insn, TPCII::VpuSrcBStart, TPCII::VpuSrcBSize);
  uint64_t SrcC = fieldFromInstruction(insn, TPCII::VpuSrcCStart, TPCII::VpuSrcCSize);
  uint64_t Dest = fieldFromInstruction(insn, TPCII::VpuDestStart, TPCII::VpuDestSize);
  uint64_t Optype = fieldFromInstruction(insn, TPCII::VpuOpTypeStart, TPCII::VpuOpTypeSize);
  uint64_t Switches = fieldFromInstruction(insn, TPCII::VpuSwitches1Start, TPCII::VpuSwitches1Size);
  Switches = Switches | (fieldFromInstruction(insn, TPCII::VpuSwitches2Start, TPCII::VpuSwitches2Size) <<3);
  uint64_t Predicate = fieldFromInstruction(insn, TPCII::VpuPredicateStart, TPCII::VpuPredicateSize);
  bool VectorPredicate = (bool) fieldFromInstruction(insn, TPCII::VpuVectorPredBit, 1);
  bool Polarity = (bool)fieldFromInstruction(insn, TPCII::VpuPolarityBit, 1);
  assert(Opcode == TPCII::vpuMADD);
  (void) Opcode;

  auto *Disasm = static_cast<const TPCDisassembler *>(Decoder);
  const FeatureBitset &Bits = Disasm->getSubtargetInfo().getFeatureBits();
  assert(Bits[TPC::FeatureMADD]);

  bool IsAccI16 = Switches & TPCII::SW_ACC_I16;
  bool IsZP = Switches & TPCII::SW_ZP;
  assert(IsZP && "Custom decoder is used only for MADD with ZP");
  (void) IsZP;

  if (Optype == TPCII::OpType::INT8) {
    if(IsAccI16) {
      Inst.setOpcode(VectorPredicate ? TPC::MADDAzpi8m_Dis : TPC::MADDAzpi8p_Dis );
    } else {
      Inst.setOpcode(VectorPredicate ? TPC::MADDzpi8m_Dis : TPC::MADDzpi8p_Dis );
    }
  } else if (Optype == TPCII::OpType::UINT8) {
    if(IsAccI16) {
      Inst.setOpcode(VectorPredicate ? TPC::MADDAzpu8m_Dis : TPC::MADDAzpu8p_Dis );
    } else {
      Inst.setOpcode(VectorPredicate ? TPC::MADDzpu8m_Dis : TPC::MADDzpu8p_Dis );
    }
  } else {
    return MCDisassembler::Fail;
  }
  if (IsAccI16) {
    if (DecodeDRFRegisterClass(Inst, Dest, Address, Decoder) == MCDisassembler::Fail)
      return MCDisassembler::Fail;
  } else {
    if (DecodeARFRegisterClass(Inst, Dest, Address, Decoder) == MCDisassembler::Fail)
      return MCDisassembler::Fail;
  }

  if (DecodeVRFRegisterClass(Inst, SrcA, Address, Decoder) == MCDisassembler::Fail)
    return MCDisassembler::Fail;
  if (DecodeVRFRegisterClass(Inst, SrcB, Address, Decoder) == MCDisassembler::Fail)
    return MCDisassembler::Fail;
  if (IsAccI16) {
    if (DecodeDRFRegisterClass(Inst, SrcC, Address, Decoder) == MCDisassembler::Fail)
      return MCDisassembler::Fail;
  } else {
    if (DecodeARFRegisterClass(Inst, SrcC, Address, Decoder) == MCDisassembler::Fail)
      return MCDisassembler::Fail;
  }

  Inst.addOperand(MCOperand::createReg(TPC::ZP_REG));

  Inst.addOperand(MCOperand::createImm(Switches & ~(TPCII::SW_ZP | TPCII::SW_ACC_I16)));

  if (IsAccI16) {
    if (DecodeDRFRegisterClass(Inst, Dest, Address, Decoder) == MCDisassembler::Fail)
      return MCDisassembler::Fail;
  } else {
    if (DecodeARFRegisterClass(Inst, Dest, Address, Decoder) == MCDisassembler::Fail)
      return MCDisassembler::Fail;
  }

  uint64_t PredValue = (Polarity << 5) | (VectorPredicate << 4)| Predicate;
  if (VectorPredicate) {
    if (decodeVPredicate(Inst, PredValue, Address, Decoder) == MCDisassembler::Fail)
      return MCDisassembler::Fail;
  } else {
    if (decodeSPredicate(Inst, PredValue, Address, Decoder) == MCDisassembler::Fail)
      return MCDisassembler::Fail;
  }
  return MCDisassembler::Success;
}

static MCDisassembler::DecodeStatus
decodeNearbyint(MCInst &Inst, uint64_t Insn, uint64_t Address, const void *Decoder) {
  uint64_t Opcode = fieldFromInstruction(Insn, TPCII::VpuOpCodeStart, TPCII::VpuOpCodeSize);
  uint64_t SrcA = fieldFromInstruction(Insn, TPCII::VpuSrcAStart, TPCII::VpuSrcASize);
  uint64_t SrcB = fieldFromInstruction(Insn, TPCII::VpuSrcBStart, TPCII::VpuSrcBSize);
  uint64_t Dest = fieldFromInstruction(Insn, TPCII::VpuDestStart, TPCII::VpuDestSize);
  uint64_t OpType = fieldFromInstruction(Insn, TPCII::VpuOpTypeStart, TPCII::VpuOpTypeSize);
  uint64_t Switches = fieldFromInstruction(Insn, TPCII::VpuSwitches1Start, TPCII::VpuSwitches1Size);
  Switches = Switches | (fieldFromInstruction(Insn, TPCII::VpuSwitches2Start, TPCII::VpuSwitches2Size) <<3);
  uint64_t Predicate = fieldFromInstruction(Insn, TPCII::VpuPredicateStart, TPCII::VpuPredicateSize);
  bool IsVectorPredicate = (bool)fieldFromInstruction(Insn, TPCII::VpuVectorPredBit, 1);
  bool Polarity = (bool)fieldFromInstruction(Insn, TPCII::VpuPolarityBit, 1);
  assert(Opcode == TPCII::vpuNEARBYINT);
  (void)Opcode;

  bool HasCnvrt = Switches & TPCII::SW_CNVRT;
  if (HasCnvrt) {
    if (DecodeDRFRegisterClass(Inst, Dest, Address, Decoder) == MCDisassembler::Fail)
      return MCDisassembler::Fail;
  } else {
    if (DecodeVRFRegisterClass(Inst, Dest, Address, Decoder) == MCDisassembler::Fail)
      return MCDisassembler::Fail;
  }
  if (DecodeVRFRegisterClass(Inst, SrcA, Address, Decoder) == MCDisassembler::Fail)
    return MCDisassembler::Fail;
  Inst.addOperand(MCOperand::createImm(OpType));
  unsigned RM = Switches & 0x07;
  Switches &= ~0x07;
  Switches |= (SrcB << 8);
  Switches |= (RM << 16);
  Inst.addOperand(MCOperand::createImm(Switches));
  if (HasCnvrt) {
    if (DecodeDRFRegisterClass(Inst, Dest, Address, Decoder) == MCDisassembler::Fail)
      return MCDisassembler::Fail;
  } else {
    if (DecodeVRFRegisterClass(Inst, Dest, Address, Decoder) == MCDisassembler::Fail)
      return MCDisassembler::Fail;
  }
  uint64_t PredValue = (Polarity << 5) | (IsVectorPredicate << 4) | Predicate;
  if (IsVectorPredicate) {
    if (decodeVPredicate(Inst, PredValue, Address, Decoder) == MCDisassembler::Fail)
      return MCDisassembler::Fail;
  } else {
    if (decodeSPredicate(Inst, PredValue, Address, Decoder) == MCDisassembler::Fail)
      return MCDisassembler::Fail;
  }
  return MCDisassembler::Success;
}

static MCDisassembler::DecodeStatus decodeLD_G(MCInst &Inst, uint64_t insn,
                                               uint64_t Address, const void *Decoder) {
  auto *Disasm = static_cast<const TPCDisassembler *>(Decoder);
  const FeatureBitset &Bits = Disasm->getSubtargetInfo().getFeatureBits();
  bool IsGen4plus = Bits[TPC::FeatureGen4Plus];
  bool IsDoron1 = (bool) Bits[TPC::FeatureDoron1];

  uint64_t OpCode = fieldFromInstruction(insn, TPCII::LdOpCodeStart, TPCII::LdOpCodeSize);
  uint64_t SrcA = fieldFromInstruction(insn, TPCII::LdSrcAStart, TPCII::LdSrcASize);
  uint64_t SrcB = fieldFromInstruction(insn, TPCII::LdSrcBStart, TPCII::LdSrcBSize);
  uint64_t Dest = fieldFromInstruction(insn, TPCII::LdDestStart, TPCII::LdDestSize);
  uint64_t Switches = fieldFromInstruction(insn, TPCII::LdSwitchesStart, TPCII::LdSwitchesSize);
  uint64_t Predicate = fieldFromInstruction(insn, TPCII::LdPredicateStart, TPCII::LdPredicateSize);
  bool IsVectorPredicate = (bool)fieldFromInstruction(insn, TPCII::LdVectorPredBit, 1);
  bool Polarity = (bool)fieldFromInstruction(insn, TPCII::LdPolarityBit, 1);

  uint64_t Dimmask = SrcB;
  bool IsPartial = SrcB & 0x1 && Bits[TPC::FeatureGen3Plus] ? true : false;

  assert(OpCode == TPCII::LD_G);
  (void)OpCode;
  unsigned SwitchValue = 0;
  if (Bits[TPC::FeatureGen3Plus]) {
    // Collect switches.
    SwitchValue = Switches;
    SwitchValue |= SrcB << 8;
    // Partial
    SwitchValue |= SrcB & 0x1 >> 8;
    // Correct bitmask
    Dimmask = (SrcB >> 2) & 0x3f; // Dimmask occupies only 5 bits
  }

  bool dimMaskReg = false;
  if (isSRF(Dest, false, IsDoron1)) {
    Inst.setOpcode(TPC::LD_Gsap);
  } else if (isSPRF(Dest, false, IsDoron1)) {
    Inst.setOpcode(TPC::LD_Gpap);
  } else if (isVRF(Dest) && IsGen4plus) {
    if (IsPartial)
      Inst.setOpcode(IsVectorPredicate ? TPC::LD_G_Pg4vam : TPC::LD_G_Pg4vap);
    else
      Inst.setOpcode(IsVectorPredicate ? TPC::LD_Gg4vam : TPC::LD_Gg4vap);
  } else if (isVRF(Dest) && !IsGen4plus) {
    if (IsPartial)
      Inst.setOpcode(IsVectorPredicate ? TPC::LD_G_Pvam : TPC::LD_G_Pvap);
    else
      Inst.setOpcode(TPC::LD_Gvap);
  } else if (isVPRF(Dest, IsDoron1)) {
    if (IsVectorPredicate)
      Inst.setOpcode(TPC::LD_Gg4mam);
    else
      Inst.setOpcode(TPC::LD_Gg4map);
  } else if (isIRF(Dest, false, IsDoron1)) {
    if (IsGen4plus)
      dimMaskReg = SwitchValue & TPCII::SW_DIM_MASK_REG_G4;
    else
      dimMaskReg = SwitchValue & TPCII::SW_DIM_MASK_REG;
    if (dimMaskReg) {
      if (IsGen4plus)
        Inst.setOpcode(TPC::LD_Gg4Iwap);
      else
        Inst.setOpcode(TPC::LD_GIwap);
    } else {
      if (IsGen4plus)
        Inst.setOpcode(TPC::LD_Gg4Iap);
      else
        Inst.setOpcode(TPC::LD_GIap);
    }
  } else {
    llvm_unreachable("Invalid register class in LD_G");
  }


  if (DecodeVRFRegisterClass(Inst, Dest, Address, Decoder) == MCDisassembler::Fail)
    return MCDisassembler::Fail;
  if (DecodeADRFRegisterClass(Inst, SrcA, Address, Decoder) == MCDisassembler::Fail)
    return MCDisassembler::Fail;
  if (isIRF(Dest, false, IsDoron1)) {
    if (dimMaskReg) {
      if (DecodeMRFRegisterClass(Inst, SrcB, Address, Decoder) == MCDisassembler::Fail)
        return MCDisassembler::Fail;
    } else {
      Inst.addOperand(MCOperand::createImm(Dimmask)); //Write bitmask
    }
  }

  if (IsPartial)
    Inst.addOperand(MCOperand::createReg(TPC::LD_PARTIAL_REG));
  Inst.addOperand(MCOperand::createImm(SwitchValue));


  if (DecodeVRFRegisterClass(Inst, Dest, Address, Decoder) == MCDisassembler::Fail)
    return MCDisassembler::Fail;
  uint64_t PredValue = (Polarity << 5) | (IsVectorPredicate << 4) | Predicate;
  if (IsVectorPredicate) {
    if (decodeVPredicate(Inst, PredValue, Address, Decoder) == MCDisassembler::Fail)
      return MCDisassembler::Fail;
  } else {
    if (decodeSPredicate(Inst, PredValue, Address, Decoder) == MCDisassembler::Fail)
      return MCDisassembler::Fail;
  }
  return MCDisassembler::Success;
}

static DecodeStatus readJmpInstruction(uint64_t InsnSPU,
                                    ArrayRef<uint8_t> Bytes,
                                    JmpExtraValues &Extra) {
  std::bitset<256> Bundle;
  const unsigned InstructionSize = 256 / 8;
  static_assert(sizeof(Bundle) == InstructionSize, "Extra fields in std::bitset");

  // We want to read exactly 256 bits of data.
  if (Bytes.size() < InstructionSize) {
    return MCDisassembler::Fail;
  }

  // Prepare entire bundle.
  memcpy(&Bundle, &Bytes.front(), InstructionSize);

  LLVM_DEBUG(dbgs() << "== Decoding bundle: " << to_hexstring(Bundle) << "\n");

  // Prepare Predicate field.
  Extra.Predicate = (InsnSPU >> TPCII::SpuPredicateStart) &  (((uint8_t)1 << TPCII::SpuPredicateSize) - 1);

  // Prepare Polarity bit.
  Extra.Polarity = (InsnSPU >> TPCII::SpuPolarityBit) &  1;

  // Prepare Imm field.
  std::bitset<256> bundle;
  int32_t val32;
  bundle = Bundle >> ((Extra.MayCompress) ? TPCII::Gen3ImmStart:TPCII::ImmStart);
  memcpy(&val32, &bundle, 4);
  Extra.Imm=val32;

  return MCDisassembler::Success;
}

#define MASK(Len) ((1UL<<Len)-1)

DecodeStatus TPCDisassembler::readLoopInstruction(
    ArrayRef<uint8_t> Bytes, uint64_t &Size, std::bitset<256> &Bundle,
    uint64_t &InsnLoop, LoopExtraValues &Extra, bool &IsPredDoron1) const {
  const unsigned InstructionSize = 256 / 8;
  static_assert(sizeof(Bundle) == InstructionSize, "Extra fields in std::bitset");
  
  // We want to read exactly 256 bits of data.
  if (Bytes.size() < InstructionSize) {
    Size = 0;
    return MCDisassembler::Fail;
  }

  bool IsDoron1 = STI.getFeatureBits()[TPC::FeatureDoron1];
  unsigned loop_start_imm_start = IsDoron1 ? TPCII::Gen4LoopStartImmStart : (Extra.MayCompress ? TPCII::Gen3LoopStartImmStart    : TPCII::LoopStartImmStart);
  unsigned loop_bound_imm_start = IsDoron1 ? TPCII::Gen4LoopBoundaryImmStart : (Extra.MayCompress ? TPCII::Gen3LoopBoundaryImmStart : TPCII::LoopBoundaryImmStart);
  unsigned loop_step_imm_start  = IsDoron1 ? TPCII::Gen4LoopStepImmStart : (Extra.MayCompress ? TPCII::Gen3LoopStepImmStart     : TPCII::LoopStepImmStart);
  unsigned loop_offset_start    = IsDoron1 ? TPCII::Gen4LoopOffsetStart : (Extra.MayCompress ? TPCII::Gen3LoopOffsetStart      : TPCII::LoopOffsetStart);
  const std::map<Fields, Field> &Layout = TPCInstrLayout::getLoopInstrLayout(
      Extra.MayCompress, STI.getFeatureBits());

  // Prepare entire bundle.
  memcpy(&Bundle, &Bytes.front(), InstructionSize);
  LLVM_DEBUG(dbgs() << "== Decoding bundle: " << to_hexstring(Bundle) << "\n");

  std::bitset<256> bundle;

  uint64_t bits = 0;
  InsnLoop = 0;
  for (auto FieldLayout : Layout) {
    std::bitset<256> FieldBits = Bundle >> FieldLayout.second.startBin;
    FieldBits &= MASK(FieldLayout.second.sizeBin);
    bits = static_cast<uint64_t>(FieldBits.to_ulong());

    if (IsDoron1 && bits) {
      // STEP register encoding is odd in Doron1, cannot use unified decoder
      // So convert START and BOUNDARY to plain SRFs, DecodeSRFRegisterClass() is aware
      if ((FieldLayout.first == START_VALUE) ||
          (FieldLayout.first == BOUNDARY_VALUE)) {
        // Other values are immediate
        if (bits >= 64 && bits <= 127)
          bits -= 64;
      }
      if (FieldLayout.first == LOOP_IS_PREDICATED) {
        IsPredDoron1 = bits;
        bits = 0; //TODO handle predication
      }
    }
    InsnLoop |= bits << FieldLayout.second.startLLVM;
  }
  LLVM_DEBUG(dbgs() << "-- LOOP: " << to_hexstring(InsnLoop) << "\n");

  uint32_t val32;

  // Prepare BOUNDARY field.
  bundle = Bundle >> loop_bound_imm_start;
  memcpy(&val32, &bundle, 4);
  Extra.Boundary=val32;

  // Prepare STEP field.
  bundle = Bundle >> loop_step_imm_start;
  memcpy(&val32, &bundle, 4);
  Extra.Step=val32;

  // Prepare OFFSET field.
  bundle = Bundle >> loop_offset_start;
  uint16_t offset;
  memcpy(&offset, &bundle, 2);
  Extra.Offset=offset;

  // Prepare COMP_MODE field.
  bundle = Bundle >> Layout.at(COMP_MODE).startBin;
  uint8_t bt;
  memcpy(&bt, &bundle, 1);
  bt &= ((1 << 3) - 1);
  Extra.Comparison=bt;

  // Prepare START field.
  bundle = Bundle >> loop_start_imm_start;
  memcpy(&val32, &bundle, 4);
  Extra.Start=val32;

  return MCDisassembler::Success;
}

static DecodeStatus decodeFclass(MCInst &Inst, uint64_t insn,
                                 uint64_t Address, const void *Decoder) {
  uint64_t OpCode = fieldFromInstruction(insn, TPCII::VpuOpCodeStart, TPCII::VpuOpCodeSize);
  uint64_t SrcA = fieldFromInstruction(insn, TPCII::VpuSrcAStart, TPCII::VpuSrcASize);
  uint64_t SrcB = fieldFromInstruction(insn, TPCII::VpuSrcBStart, TPCII::VpuSrcBSize);
  uint64_t SrcD = fieldFromInstruction(insn, TPCII::VpuSrcDStart, TPCII::VpuSrcDSize);
  uint64_t Dest = fieldFromInstruction(insn, TPCII::VpuDestStart, TPCII::VpuDestSize);
  uint64_t OpType = fieldFromInstruction(insn, TPCII::VpuOpTypeStart, TPCII::VpuOpTypeSize);
  uint64_t Switches = fieldFromInstruction(insn, TPCII::VpuSwitches1Start, TPCII::VpuSwitches1Size);
  Switches = Switches | (fieldFromInstruction(insn, TPCII::VpuSwitches2Start, TPCII::VpuSwitches2Size) << TPCII::VpuSwitches1Size);
  uint64_t Predicate = fieldFromInstruction(insn, TPCII::VpuPredicateStart, TPCII::VpuPredicateSize);
  bool IsVectorPredicate = (bool)fieldFromInstruction(insn, TPCII::VpuVectorPredBit, 1);
  bool Polarity = (bool)fieldFromInstruction(insn, TPCII::VpuPolarityBit, 1);

  auto *Disasm = static_cast<const TPCDisassembler *>(Decoder);
  const FeatureBitset &Bits = Disasm->getSubtargetInfo().getFeatureBits();
  bool IsDoron1 = (bool) Bits[TPC::FeatureDoron1];

  bool HasLimit = Switches != 0;
  assert(OpCode == TPCII::vpuFCLASS);

  bool HasSrf = false;
  if (isSRF(SrcA, false, IsDoron1))
    HasSrf = true;

  if (HasLimit) {
    if (isSRF(SrcB, false, IsDoron1)) {
      if (HasSrf)
        return MCDisassembler::Fail;
      HasSrf = true;
    }

    if (isSRF(SrcD, false, IsDoron1))
      if (HasSrf)
        return MCDisassembler::Fail;
  }

  if (HasLimit)
    Inst.setOpcode(IsVectorPredicate ? TPC::FCLASS_LIMITm_Dis : TPC::FCLASS_LIMITp_Dis);
  else
    Inst.setOpcode(IsVectorPredicate ? TPC::FCLASSm_Dis : TPC::FCLASSp_Dis);

  if (DecodeVRFRegisterClass(Inst, Dest, Address, Decoder) == MCDisassembler::Fail)
    return MCDisassembler::Fail;

  if (DecodeVRFRegisterClass(Inst, SrcA, Address, Decoder) == MCDisassembler::Fail)
    return MCDisassembler::Fail;
  if (HasLimit) {
    if (DecodeVRFRegisterClass(Inst, SrcB, Address, Decoder) == MCDisassembler::Fail)
      return MCDisassembler::Fail;
    if (DecodeVRFRegisterClass(Inst, SrcD, Address, Decoder) == MCDisassembler::Fail)
      return MCDisassembler::Fail;
  }

  Inst.addOperand(MCOperand::createImm(OpType));
  Inst.addOperand(MCOperand::createImm(Switches));

  //Income
  DecodeVRFRegisterClass(Inst, Dest, Address, Decoder);

  uint64_t PredValue = (Polarity << 5) | (IsVectorPredicate << 4)| Predicate;
  if (IsVectorPredicate) {
    if (decodeVPredicate(Inst, PredValue, Address, Decoder) == MCDisassembler::Fail)
      return MCDisassembler::Fail;
  } else {
    if (decodeSPredicate(Inst, PredValue, Address, Decoder) == MCDisassembler::Fail)
      return MCDisassembler::Fail;
  }

  return MCDisassembler::Success;
}

static DecodeStatus decodeMaddX2(MCInst &Inst, uint64_t insn,
                                 uint64_t Address, const void *Decoder) {
  uint64_t OpCode = fieldFromInstruction(insn, TPCII::VpuOpCodeStart, TPCII::VpuOpCodeSize);
  uint64_t SrcA = fieldFromInstruction(insn, TPCII::VpuSrcAStart, TPCII::VpuSrcASize);
  uint64_t SrcB = fieldFromInstruction(insn, TPCII::VpuSrcBStart, TPCII::VpuSrcBSize);
  uint64_t SrcC = fieldFromInstruction(insn, TPCII::VpuSrcCStart, TPCII::VpuSrcCSize);
  uint64_t SrcD = fieldFromInstruction(insn, TPCII::VpuSrcDStart, TPCII::VpuSrcDSize);
  uint64_t Dest = fieldFromInstruction(insn, TPCII::VpuDestStart, TPCII::VpuDestSize);
  uint64_t OpType = fieldFromInstruction(insn, TPCII::VpuOpTypeStart, TPCII::VpuOpTypeSize);
  uint64_t Switches = fieldFromInstruction(insn, TPCII::VpuSwitches1Start, TPCII::VpuSwitches1Size);
  Switches = Switches | (fieldFromInstruction(insn, TPCII::VpuSwitches2Start, TPCII::VpuSwitches2Size) << TPCII::VpuSwitches1Size);
  uint64_t Predicate = fieldFromInstruction(insn, TPCII::VpuPredicateStart, TPCII::VpuPredicateSize);
  bool IsVectorPredicate = (bool)fieldFromInstruction(insn, TPCII::VpuVectorPredBit, 1);
  bool Polarity = (bool)fieldFromInstruction(insn, TPCII::VpuPolarityBit, 1);

  assert(OpCode == TPCII::vpuMADD);
  assert(Switches & TPCII::SW_X2_ARITHMETIC);

  Inst.setOpcode(IsVectorPredicate ? TPC::MADDx2f32vvvvm_Dis : TPC::MADDx2f32vvvvp_Dis );

  if (DecodeDRFRegisterClass(Inst, Dest, Address, Decoder) == MCDisassembler::Fail)
    return MCDisassembler::Fail;

  if (DecodeDRFRegisterClass(Inst, SrcA, Address, Decoder) == MCDisassembler::Fail)
    if (DecodeSRFRegisterClass(Inst, SrcA, Address, Decoder) == MCDisassembler::Fail)
      return MCDisassembler::Fail;

  if (DecodeVRFRegisterClass(Inst, SrcB, Address, Decoder) == MCDisassembler::Fail)
    return MCDisassembler::Fail;

  if (DecodeDRFRegisterClass(Inst, SrcC, Address, Decoder) == MCDisassembler::Fail)
    if (DecodeSRFRegisterClass(Inst, SrcC, Address, Decoder) == MCDisassembler::Fail)
      return MCDisassembler::Fail;

  if (DecodeVRFRegisterClass(Inst, SrcD, Address, Decoder) == MCDisassembler::Fail)
    return MCDisassembler::Fail;

  Inst.addOperand(MCOperand::createImm(OpType));
  Inst.addOperand(MCOperand::createImm(Switches));

  //Income
  DecodeDRFRegisterClass(Inst, Dest, Address, Decoder);

  uint64_t PredValue = (Polarity << 5) | (IsVectorPredicate << 4)| Predicate;
  if (IsVectorPredicate) {
    if (decodeVPredicate(Inst, PredValue, Address, Decoder) == MCDisassembler::Fail)
      return MCDisassembler::Fail;
  } else {
    if (decodeSPredicate(Inst, PredValue, Address, Decoder) == MCDisassembler::Fail)
      return MCDisassembler::Fail;
  }

  return MCDisassembler::Success;
}

DecodeStatus
TPCDisassembler::tryDecodeLoopInstruction(MCInst &Instr, uint64_t &Size,
                                ArrayRef<uint8_t> Bytes, uint64_t Address,
                                raw_ostream &vStream,
                                raw_ostream &cStream) const {
  std::bitset<256> Bundle;
  uint64_t InsnLoop;
  LoopExtraValues Extra;

  Extra.Address = Address;
  Extra.MayCompress = getSubtargetInfo().getFeatureBits()[TPC::FeatureCompress];
  Address = (uint64_t)(&Extra);

  // Play the role only for doron1
  bool IsPredDoron1 = false;
  DecodeStatus Result = readLoopInstruction(Bytes, Size, Bundle, InsnLoop,
                                            Extra, IsPredDoron1);
  if (Result == MCDisassembler::Fail)
    return MCDisassembler::Fail;

  bool isFakePass = (&vStream == &nulls() && &cStream == &nulls());
  if (isFakePass) {
    int64_t *point = (int64_t*)Extra.Address;
    *point = Extra.Offset;
    Instr.setOpcode(TPCII::spuLOOP);
    return MCDisassembler::Success;
  }
  
  Instr.setOpcode(TPCII::spuLOOP);
  unsigned Flags = TPCII::MCFlagLOOP;
  if (!IsPredDoron1)
    Flags |= TPCII::MCFlagIsUnpredDoron1;
  Instr.setFlags(Flags);
  Instr.addOperand(MCOperand::createImm(0));

  Result = decodeInstruction(DecoderTable64, Instr, InsnLoop, Address, this, STI);
  if (Result == MCDisassembler::Fail) {
    return MCDisassembler::Fail;
  }

  return MCDisassembler::Success;
}

DecodeStatus
TPCDisassembler::getInstruction(MCInst &Instr, uint64_t &Size,
                                ArrayRef<uint8_t> Bytes, uint64_t Address,
                                raw_ostream &CStream) const {
  uint64_t InsnSPU = 0UL;
  uint64_t InsnVPU = 0UL;
  uint64_t InsnLoad = 0UL;
  uint64_t InsnStore = 0UL;
  uint32_t Immediate = 0UL;

  // Play the role only for Doron1
  bool IsSPUPredDoron1 = false;
  bool IsVPUPredDoron1 = false;
  bool IsLoadPredDoron1 = false;
  bool IsStorePredDoron1 = false;
  TPCInstrDecomposer Converter(Bytes, getSubtargetInfo().getFeatureBits());
  DecodeStatus Result;

  if (Converter.getLLVMInstSPU(InsnSPU, IsSPUPredDoron1) ==
      MCDisassembler::Fail ||
      Converter.getLLVMInstVPU(InsnVPU, IsVPUPredDoron1) ==
      MCDisassembler::Fail ||
      Converter.getLLVMInstLoad(InsnLoad, IsLoadPredDoron1) ==
      MCDisassembler::Fail ||
      Converter.getLLVMInstStore(InsnStore, IsStorePredDoron1) ==
      MCDisassembler::Fail ||
      Converter.getLLVMInstIMM(Immediate) == MCDisassembler::Fail) {
    Size = 0;
    return MCDisassembler::Fail;
  }

  Size = Converter.getBundleSizeInBytes();
  this->Immediate = Immediate;
  this->CommentStream = &CStream;

  // Decode solo instructions.
  if (getSPUOpCode(InsnSPU) == TPCII::spuLOOP) {
    return tryDecodeLoopInstruction(Instr, Size, Bytes, Address, CStream, CStream);
  }

  // All other cases represent 4-slot instruction bundle.
  //Instr.setLoc(IDLoc);
  Instr.setOpcode(TPC::BUNDLE);
  Instr.addOperand(MCOperand::createImm(0));

  // Call auto-generated Converter function.

  bool isFakePass = (&CStream == &nulls());

  MCInst *SPUSubInst = new (getContext()) MCInst;
  unsigned Flag = 0;
  Flag = TPCII::MCFlagSPU;
  if (!IsSPUPredDoron1)
    Flag |= TPCII::MCFlagIsUnpredDoron1;
  SPUSubInst->setFlags(Flag);
  
  JmpExtraValues Extra;
  if (getSPUOpCode(InsnSPU) == TPCII::spuJMPR || getSPUOpCode(InsnSPU) == TPCII::spuJMPA) {
    Extra.Address = Address;
    Extra.MayCompress = getSubtargetInfo().getFeatureBits()[TPC::FeatureCompress];
    Address = (uint64_t)(&Extra);

    Result = readJmpInstruction(InsnSPU, Bytes, Extra);
    if (Result == MCDisassembler::Fail)
      return MCDisassembler::Fail;
    if (isFakePass) {
        Instr.setOpcode(TPCII::spuJMPR);
        int64_t *point = (int64_t*)Extra.Address;
        *point = Extra.Imm;
      return MCDisassembler::Success;
    }
  }

  if (isFakePass) {
    return MCDisassembler::Success;
  }

  Result = decodeInstruction(DecoderTableScalarSlot64, *SPUSubInst, InsnSPU, Address, this, STI);
  if (Result == MCDisassembler::Fail)
    return MCDisassembler::Fail;
  Instr.addOperand(MCOperand::createInst(SPUSubInst));

  MCInst *VPUSubInst = new (getContext()) MCInst;
  Flag = TPCII::MCFlagVPU;
  if (!IsVPUPredDoron1)
    Flag |= TPCII::MCFlagIsUnpredDoron1;
  VPUSubInst->setFlags(Flag);
  
  Result = decodeInstruction(DecoderTableVectorSlot64, *VPUSubInst, InsnVPU, Address, this, STI);

  if (Result == MCDisassembler::SoftFail)
    Result = decodeInstruction(DecoderTableVectorSlot64, *VPUSubInst, InsnVPU, Address, this, STI);
  if (Result == MCDisassembler::Fail)
    return MCDisassembler::Fail;
  Instr.addOperand(MCOperand::createInst(VPUSubInst));

  MCInst *LDSubInst = new (getContext()) MCInst;
  Flag = TPCII::MCFlagLDU;
  if (!IsLoadPredDoron1)
    Flag |= TPCII::MCFlagIsUnpredDoron1;
  LDSubInst->setFlags(Flag);
  
  Result = decodeInstruction(DecoderTableLoadSlot64, *LDSubInst, InsnLoad, Address, this, STI);
  if (Result == MCDisassembler::Fail)
    return MCDisassembler::Fail;
  Instr.addOperand(MCOperand::createInst(LDSubInst));

  MCInst *STSubInst = new (getContext()) MCInst;
  Flag = TPCII::MCFlagSTU;
  if (!IsStorePredDoron1)
    Flag |= TPCII::MCFlagIsUnpredDoron1;
  STSubInst->setFlags(Flag);
  
  Result = decodeInstruction(DecoderTableStoreSlot64, *STSubInst, InsnStore, Address, this, STI);
  if (Result == MCDisassembler::Fail)
    return MCDisassembler::Fail;
  Instr.addOperand(MCOperand::createInst(STSubInst));

  std::string comment = "";
  for (llvm::MCInst *inst : {LDSubInst, SPUSubInst, VPUSubInst, STSubInst}) {
    for (unsigned ind = 0; ind < inst->getNumOperands(); ind++) {
      if (inst->getOperand(ind).isReg()) {
        ConstructComplexRegisterComment(inst->getOperand(ind).getReg(), comment);
      }
    }
  }

  if (getSubtargetInfo().getFeatureBits()[TPC::FeatureCompress]) {
    static bool firstCompInstrPrinted = false;
    if (Converter.getIsCompressed()) {
      CStream << " \t// compressed";
      CStream << (firstCompInstrPrinted ? ", part 2" : ", part 1");
      firstCompInstrPrinted = !firstCompInstrPrinted;
      if (!comment.empty()) {
        CStream << " , " << comment;
      }

      return MCDisassembler::Success;
    }
  }

  if (!comment.empty()) {
    CStream << " \t// " << comment;
  }

  return MCDisassembler::Success;
}


//===----------------------------------------------------------------------===//
// TPCSymbolizer
//===----------------------------------------------------------------------===//

bool TPCSymbolizer::tryAddingSymbolicOperand(MCInst &Inst,
                                raw_ostream &/*cStream*/, int64_t Value,
                                uint64_t Address, bool IsBranch,
                                uint64_t Offset, uint64_t /*InstSize*/) {
  uint64_t SearchValue = Address + Value;

  if (!IsBranch) {
    return false;
  }

  auto *Symbols = static_cast<SectionSymbolsTy *>(DisInfo);
  if (!Symbols) {
    return false;
  }

  auto Result = std::find_if(Symbols->begin(), Symbols->end(),
                             [SearchValue](const SymbolInfoTy& Val) {
                                return Val.Addr == static_cast<uint64_t>(SearchValue)
                                    && Val.Type == ELF::STT_NOTYPE;
                             });
  if (Result != Symbols->end()) {
    auto *Sym = Ctx.getOrCreateSymbol(Result->Name);
    const auto *Add = MCSymbolRefExpr::create(Sym, Ctx);
    Inst.addOperand(MCOperand::createExpr(Add));
    return true;
  }
  return false;
}

void TPCSymbolizer::tryAddingPcLoadReferenceComment(raw_ostream &cStream,
                                                       int64_t Value,
                                                       uint64_t Address) {
  llvm_unreachable("unimplemented");
}

//===----------------------------------------------------------------------===//
// Initialization
//===----------------------------------------------------------------------===//

static MCSymbolizer *createTPCSymbolizer(const Triple &/*TT*/,
                              LLVMOpInfoCallback /*GetOpInfo*/,
                              LLVMSymbolLookupCallback /*SymbolLookUp*/,
                              void *DisInfo,
                              MCContext *Ctx,
                              std::unique_ptr<MCRelocationInfo> &&RelInfo) {
  return new TPCSymbolizer(*Ctx, std::move(RelInfo), DisInfo);
}


static MCDisassembler *createTPCDisassembler(const Target &T,
                                             const MCSubtargetInfo &STI,
                                             MCContext &Ctx) {
  return new TPCDisassembler(STI, Ctx, T.createMCInstrInfo());
}


extern "C" void LLVMInitializeTPCDisassembler() {
  TargetRegistry::RegisterMCDisassembler(getTheTPCTarget(),
                                         createTPCDisassembler);
  TargetRegistry::RegisterMCSymbolizer(getTheTPCTarget(),
                                       createTPCSymbolizer);
}

