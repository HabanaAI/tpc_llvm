//===-- TPCDisassembler.cpp - Disassembler for x86 and x86_64 -------------===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
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
#include <vector>

using namespace llvm;

#define DEBUG_TYPE "tpc-disassembler"

using DecodeStatus = llvm::MCDisassembler::DecodeStatus;

const unsigned FlagSPU = 0;
const unsigned FlagVPU = 1;
const unsigned FlagLDU = 2;
const unsigned FlagSTU = 3;


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
static MCDisassembler::DecodeStatus decodeStTnsr(MCInst &Inst, uint64_t insn,
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
static MCDisassembler::DecodeStatus decodeNearbyint(MCInst &Inst, uint64_t insn,
                                                    uint64_t Address, const void *Decoder);
static MCDisassembler::DecodeStatus decodeLD_G(MCInst &Inst, uint64_t insn,
                                               uint64_t Address, const void *Decoder);
static MCDisassembler::DecodeStatus decodeFclass(MCInst &Inst, uint64_t insn,
                                                 uint64_t Address, const void *Decoder);


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

static const unsigned PredicateTable[] = {
  TPC::SP0, TPC::SP1 ,TPC::SP2,  TPC::SP3,  TPC::SP4,  TPC::SP5,  TPC::SP6,  TPC::SP7,
  TPC::SP8, TPC::SP9, TPC::SP10, TPC::SP11, TPC::SP12, TPC::SP13, TPC::SP14, TPC::SP15,
  TPC::VP0, TPC::VP1, TPC::VP2,  TPC::VP3,  TPC::VP4,  TPC::VP5,  TPC::VP6,  TPC::VP7,
  TPC::VP8, TPC::VP9, TPC::VP10, TPC::VP11, TPC::VP12, TPC::VP13, TPC::VP14, TPC::VP15
};

static bool isVRF(unsigned Field) {
  return Field <= 44;
}

static bool isVPRF(unsigned Field) {
  return Field >=240 && Field <=255;
}

static bool isSPRF(unsigned Field, bool IsSPU) {
  if (IsSPU)
    return Field >= 48 && Field <= 63;
  else
    return Field >= 224 && Field <= 239;
}

static bool isSRF(unsigned Field, bool IsSPU) {
  if (IsSPU){
    // LFSR is SRF register.
    return (Field <= 35 || Field == 40);
  } else {
    return ((Field >= 64 && Field <= 99) || Field == 104 || Field == 105);
  }
}

static bool isIRF(unsigned Field, bool IsSPU) {
  if (IsSPU)
    return Field >= 64 && Field <= 95;
  else
    return Field >= 128 && Field <= 159;
}

static bool isADRF(unsigned Field, bool IsSPU) {
  if (IsSPU)
    return Field >= 96 && Field <= 103;
  else
    return Field >= 160 && Field <= 167;
}

static bool isLongImmediate(unsigned Field) {
  return Field == 127;
}

static bool isShortImmediate(unsigned Field) {
  return Field >= 111 && Field <= 126;
}

static unsigned getShortImmediate(unsigned Field) {
  return Field == 111 ? 0x0f
                      : (Field - 112);
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
  if (FieldVal == 127) {
    // This is a long immediate, encoded in separate field.
    auto *Disasm = static_cast<const TPCDisassembler *>(Decoder);
    Inst.addOperand(MCOperand::createImm(Disasm->getImmediate()));
    return true;
  }

  if (isShortImmediate(FieldVal)) {
    Inst.addOperand(MCOperand::createImm(getShortImmediate(FieldVal)));
    return true;
  }
  return false;
}

static DecodeStatus DecodeSRFRegisterClass(MCInst &Inst, unsigned RegNo,
                                           uint64_t Addr,
                                           const void *Decoder) {
  if (DecodeImmediate(Inst, RegNo, Addr, Decoder))
    return MCDisassembler::Success;

  bool VPUEncoding = Inst.getFlags() != FlagSPU;

  // Decode all registers, that can be found in SrcA/B/Dest in scalar slot, not
  // only SRF.
  unsigned Register;
  if (VPUEncoding)
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

  bool VPUEncoding = Inst.getFlags() != FlagSPU;

  // Decode all registers, that can be found in SrcA/B/Dest in scalar slot, not
  // only SRF.
  unsigned Register;
  if (VPUEncoding)
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
  bool VPUEncoding = Inst.getFlags() != FlagSPU;
  unsigned Register;
  if (VPUEncoding) {
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
  bool VPUEncoding = Inst.getFlags() != FlagSPU;
  unsigned Register;
  if (VPUEncoding) {
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

  // Decode all registers, that can be found in VPU, LD, ST slots, not
  // only VRF.
  unsigned Register = RegisterTableVPU[RegNo];
  if (Register == ~0U)
    return MCDisassembler::Fail;
  Inst.addOperand(MCOperand::createReg(Register));
  return MCDisassembler::Success;
}

static DecodeStatus DecodeVPRFRegisterClass(MCInst &Inst, unsigned RegNo,
                                           uint64_t Addr,
                                           const void *Decoder) {
  if (RegNo < 240 || RegNo > 255)
    return MCDisassembler::Fail;

  unsigned Register = RegisterTableVPU[RegNo];
  if (Register == ~0U)
    return MCDisassembler::Fail;
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
    {TPC::Z34, "Z34=[S34,S35]"}
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

static void ConstructComplexRegisterComment(unsigned RegNo, std::string &comment) {
  for (std::map<unsigned, std::string> item : {ZRFComments, DRFComments,
                                               ARFComments}) {
    auto Ptr = item.find(RegNo);
    if (Ptr != item.end()) {
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

  unsigned Register = RegisterTableVPU[RegNo];
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
  if (RegNo >= 36)
    return MCDisassembler::Fail;

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
      { 0, 0 }
  };

  unsigned Register = RegisterTableSPU[RegNo];
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

  unsigned Register = RegisterTableVPU[RegNo];
  if (Register == ~0U)
    return MCDisassembler::Fail;
  auto Ptr = VRF2ARF.find(Register);
  if (Ptr == VRF2ARF.end())
    return MCDisassembler::Fail;
  Inst.addOperand(MCOperand::createReg(Ptr->second));
  return MCDisassembler::Success;
}

static DecodeStatus DecodeHVRFRegisterClass(MCInst &Inst, unsigned Code,
                                            uint64_t Address,
                                            const void *Decoder) {
  unsigned RegCode = 0;
  switch (Code) {
  case 0:
    RegCode = TPC::PC;
    break;
  case 2:
    RegCode = TPC::S_CARRY;
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

static MCDisassembler::DecodeStatus decodeMovLd(MCInst &Inst, uint64_t insn,
                                                uint64_t Address, const void *Decoder) {
  uint64_t SrcA = fieldFromInstruction(insn, TPCII::LdSrcAStart, TPCII::LdSrcASize);
  uint64_t SrcB = fieldFromInstruction(insn, TPCII::LdSrcBStart, TPCII::LdSrcBSize);
  uint64_t Dest = fieldFromInstruction(insn, TPCII::LdDestStart, TPCII::LdDestSize);
  uint64_t Switches = fieldFromInstruction(insn, TPCII::LdSwitchesStart, TPCII::LdSwitchesSize);
  uint64_t Predicate = fieldFromInstruction(insn, TPCII::LdPredicateStart, TPCII::LdPredicateSize);
  bool IsVectorPredicate = (bool) fieldFromInstruction(insn, TPCII::LdVectorPredBit, 1);
  bool Polarity = (bool) fieldFromInstruction(insn, TPCII::LdPolarityBit, 1);

  // VRF <- VRF
  if (isVRF(Dest) && isVRF(SrcA)) {
    Inst.setOpcode(IsVectorPredicate ? TPC::MOV_ld_vvm : TPC::MOV_ld_vvp);
    if (DecodeVRFRegisterClass(Inst, Dest, Address, Decoder) == MCDisassembler::Fail)
      return MCDisassembler::Fail;
    if (DecodeVRFRegisterClass(Inst, SrcA, Address, Decoder) == MCDisassembler::Fail)
      return MCDisassembler::Fail;

  // VPRF <- VPRF
  } else if (isVPRF(Dest) && isVPRF(SrcA)) {
    Inst.setOpcode(IsVectorPredicate ? TPC::MOV_ld_mmm : TPC::MOV_ld_mmp);
    if (DecodeVPRFRegisterClass(Inst, Dest, Address, Decoder) == MCDisassembler::Fail)
      return MCDisassembler::Fail;
    if (DecodeVRFRegisterClass(Inst, SrcA, Address, Decoder) == MCDisassembler::Fail)
      return MCDisassembler::Fail;

  // VPRF <- SPRF
  } else if (isVPRF(Dest) && isSPRF(SrcA, false)) {
    Inst.setOpcode(IsVectorPredicate ? TPC::MOV_ld_mpm : TPC::MOV_ld_mpp);
    if (DecodeVPRFRegisterClass(Inst, Dest, Address, Decoder) == MCDisassembler::Fail)
      return MCDisassembler::Fail;
    if (DecodeSPRFRegisterClass(Inst, SrcA, Address, Decoder) == MCDisassembler::Fail)
      return MCDisassembler::Fail;

  // SPRF <- SRF
  } else if (isSPRF(Dest, false) && isSRF(SrcA, false)) {
    Inst.setOpcode(TPC::MOV_ld_psp);
    if (DecodeSPRFRegisterClass(Inst, Dest, Address, Decoder) == MCDisassembler::Fail)
      return MCDisassembler::Fail;
    if (DecodeSRFRegisterClass(Inst, SrcA, Address, Decoder) == MCDisassembler::Fail)
      return MCDisassembler::Fail;

  // SPRF <- Imm
  } else if (isSPRF(Dest, false) && isLongImmediate(SrcA)) {
    Inst.setOpcode(TPC::MOV_ld_pip);
    if (DecodeSPRFRegisterClass(Inst, Dest, Address, Decoder) == MCDisassembler::Fail)
      return MCDisassembler::Fail;
    auto* Disasm = static_cast<const TPCDisassembler*>(Decoder);
    Inst.addOperand(MCOperand::createImm(Disasm->getImmediate()));

  // SPRF <- short Imm
  } else if (isSPRF(Dest, false) && isShortImmediate(SrcA)) {
    Inst.setOpcode(TPC::MOV_ld_pip);
    if (DecodeSPRFRegisterClass(Inst, Dest, Address, Decoder) == MCDisassembler::Fail)
      return MCDisassembler::Fail;
    Inst.addOperand(MCOperand::createImm(getShortImmediate(SrcA)));

  // VRF <- Imm
  } else if (isVRF(Dest) && isLongImmediate(SrcA)) {
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
  } else if (isVRF(Dest) && isShortImmediate(SrcA)) {
    Inst.setOpcode(IsVectorPredicate ? TPC::MOV_ld_vim : TPC::MOV_ld_vip);
    if (DecodeVRFRegisterClass(Inst, Dest, Address, Decoder) == MCDisassembler::Fail)
      return MCDisassembler::Fail;
    Inst.addOperand(MCOperand::createImm(getShortImmediate(SrcA)));
    uint64_t OpType = (SrcB >> 4);
    if (OpType > TPCII::OpType::Max)
      return MCDisassembler::Fail;
    Inst.addOperand(MCOperand::createImm(OpType));

  // VRF <- SRF
  } else if (isVRF(Dest) && isSRF(SrcA, false)) {
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
  } else if (isVPRF(Dest) && isSRF(SrcA, false)) {
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
  } else if (isVPRF(Dest) && isLongImmediate(SrcA)) {
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
  } else if (isVPRF(Dest) && isShortImmediate(SrcA)) {
    if (SrcB == 8)
      Inst.setOpcode(IsVectorPredicate ? TPC::MOVB_ld_mim : TPC::MOVB_ld_mip);
    else
      Inst.setOpcode(IsVectorPredicate ? TPC::MOV_ld_mim : TPC::MOV_ld_mip);
    if (DecodeVRFRegisterClass(Inst, Dest, Address, Decoder) == MCDisassembler::Fail)
      return MCDisassembler::Fail;
    Inst.addOperand(MCOperand::createImm(getShortImmediate(SrcA)));
    if (SrcB != 8)
      Inst.addOperand(MCOperand::createImm(SrcB));

  // SRF <- SRF
  } else if (isSRF(Dest, false) && isSRF(SrcA, false)) {
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
  } else if (isSRF(Dest, false) && isLongImmediate(SrcA)) {
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
  } else if (isSRF(Dest, false) && isShortImmediate(SrcA)) {
    Inst.setOpcode(TPC::MOV_ld_ssp);
    if (DecodeSRFRegisterClass(Inst, Dest, Address, Decoder) == MCDisassembler::Fail)
      return MCDisassembler::Fail;
    Inst.addOperand(MCOperand::createImm(getShortImmediate(SrcA)));
    uint64_t OpType = (SrcB >> 4);
    if (OpType > TPCII::OpType::Max)
      return MCDisassembler::Fail;
    Inst.addOperand(MCOperand::createImm(OpType));

  // IRF <- SRF
  } else if (isIRF(Dest, false) && isSRF(SrcA, false) && (Switches & TPCII::SW_DIM_MASK_REG) == 0) {
    Inst.setOpcode(TPC::MOV_ld_Isp);
    if (DecodeIRFRegisterClass(Inst, Dest, Address, Decoder) == MCDisassembler::Fail)
      return MCDisassembler::Fail;
    if (DecodeSRFRegisterClass(Inst, SrcA, Address, Decoder) == MCDisassembler::Fail)
      return MCDisassembler::Fail;
    Inst.addOperand(MCOperand::createImm(SrcB >> 2));

  // IRF <- Imm
  } else if (isIRF(Dest, false) && isLongImmediate(SrcA) && (Switches & TPCII::SW_DIM_MASK_REG) == 0) {
    Inst.setOpcode(TPC::MOV_ld_Iip);
    if (DecodeIRFRegisterClass(Inst, Dest, Address, Decoder) == MCDisassembler::Fail)
      return MCDisassembler::Fail;
    auto *Disasm = static_cast<const TPCDisassembler *>(Decoder);
    Inst.addOperand(MCOperand::createImm(Disasm->getImmediate()));
    Inst.addOperand(MCOperand::createImm(SrcB >> 2));

  // IRF <- short Imm
  } else if (isIRF(Dest, false) && isShortImmediate(SrcA) && (Switches & TPCII::SW_DIM_MASK_REG) == 0) {
    Inst.setOpcode(TPC::MOV_ld_Iip);
    if (DecodeIRFRegisterClass(Inst, Dest, Address, Decoder) == MCDisassembler::Fail)
      return MCDisassembler::Fail;
    Inst.addOperand(MCOperand::createImm(getShortImmediate(SrcA)));
    Inst.addOperand(MCOperand::createImm(SrcB >> 2));

  // IRF <- IRF
  } else if (isIRF(Dest, false) && isIRF(SrcA, false) && (Switches & TPCII::SW_DIM_MASK_REG) == 0) {
    Inst.setOpcode(TPC::MOV_ld_Isp);
    if (DecodeIRFRegisterClass(Inst, Dest, Address, Decoder) == MCDisassembler::Fail)
      return MCDisassembler::Fail;
    if (DecodeIRFRegisterClass(Inst, SrcA, Address, Decoder) == MCDisassembler::Fail)
      return MCDisassembler::Fail;
    Inst.addOperand(MCOperand::createImm(SrcB >> 2));
  } else {
    return MCDisassembler::Fail;
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
  uint64_t OpType = fieldFromInstruction(insn, TPCII::SpuOpTypeStart, TPCII::SpuOpTypeSize);
  uint64_t Switches = fieldFromInstruction(insn, TPCII::SpuSwitchesStart, TPCII::SpuSwitchesSize);
  uint64_t Dest = fieldFromInstruction(insn, TPCII::SpuDestStart, TPCII::SpuDestSize);
  uint64_t Predicate = fieldFromInstruction(insn, TPCII::SpuPredicateStart, TPCII::SpuPredicateSize);
  bool Polarity = (bool)fieldFromInstruction(insn, TPCII::SpuPolarityBit, 1);
  bool HasDimmask = false;

  // SRF <- SRF
  if (isSRF(Dest, true) && isSRF(SrcA, true)) {
    Inst.setOpcode(TPC::MOVssp);
    if (DecodeSRFRegisterClass(Inst, Dest, Address, Decoder) == MCDisassembler::Fail)
      return MCDisassembler::Fail;
    if (DecodeSRFRegisterClass(Inst, SrcA, Address, Decoder) == MCDisassembler::Fail)
      return MCDisassembler::Fail;

  // SRF <- Imm
  } else if (isSRF(Dest, true) && isLongImmediate(SrcA)) {
    Inst.setOpcode(TPC::MOVsip);
    if (DecodeSRFRegisterClass(Inst, Dest, Address, Decoder) == MCDisassembler::Fail)
      return MCDisassembler::Fail;
    auto *Disasm = static_cast<const TPCDisassembler *>(Decoder);
    Inst.addOperand(MCOperand::createImm(Disasm->getImmediate()));

  // SRF <- short Imm
  } else if (isSRF(Dest, true) && isShortImmediate(SrcA)) {
    Inst.setOpcode(TPC::MOVsip);
    if (DecodeSRFRegisterClass(Inst, Dest, Address, Decoder) == MCDisassembler::Fail)
      return MCDisassembler::Fail;
    Inst.addOperand(MCOperand::createImm(getShortImmediate(SrcA)));

  // SPRF <- SRF
  } else if (isSPRF(Dest, true) && isSRF(SrcA, true)) {
    Inst.setOpcode(TPC::MOVpsp);
    if (DecodeSPRFRegisterClass(Inst, Dest, Address, Decoder) == MCDisassembler::Fail)
      return MCDisassembler::Fail;
    if (DecodeSRFRegisterClass(Inst, SrcA, Address, Decoder) == MCDisassembler::Fail)
      return MCDisassembler::Fail;

  // SPRF <- Imm
  } else if (isSPRF(Dest, true) && isLongImmediate(SrcA)) {
    Inst.setOpcode(TPC::MOVpip);
    if (DecodeSPRFRegisterClass(Inst, Dest, Address, Decoder) == MCDisassembler::Fail)
      return MCDisassembler::Fail;
    auto *Disasm = static_cast<const TPCDisassembler *>(Decoder);
    Inst.addOperand(MCOperand::createImm(Disasm->getImmediate()));

  // SPRF <- Short Imm
  } else if (isSPRF(Dest, true) && isShortImmediate(SrcA)) {
    Inst.setOpcode(TPC::MOVpip);
    if (DecodeSPRFRegisterClass(Inst, Dest, Address, Decoder) == MCDisassembler::Fail)
      return MCDisassembler::Fail;
    Inst.addOperand(MCOperand::createImm(getShortImmediate(SrcA)));

  // SPRF <- SPRF
  } else if (isSPRF(Dest, true) && isSPRF(SrcA, true)) {
    Inst.setOpcode(TPC::MOVppp);
    if (DecodeSRFRegisterClass(Inst, Dest, Address, Decoder) == MCDisassembler::Fail)
      return MCDisassembler::Fail;
    if (DecodeSRFRegisterClass(Inst, SrcA, Address, Decoder) == MCDisassembler::Fail)
      return MCDisassembler::Fail;

  // IRF <- SRF
  } else if (isIRF(Dest, true) && isSRF(SrcA, true) && (Switches & TPCII::SW_DIM_MASK_REG) == 0) {
    Inst.setOpcode(TPC::MOVIsp);
    if (DecodeIRFRegisterClass(Inst, Dest, Address, Decoder) == MCDisassembler::Fail)
      return MCDisassembler::Fail;
    if (DecodeSRFRegisterClass(Inst, SrcA, Address, Decoder) == MCDisassembler::Fail)
      return MCDisassembler::Fail;
    Inst.addOperand(MCOperand::createImm(Switches >> 2));
    HasDimmask = true;

  // IRF <- Imm
  } else if (isIRF(Dest, true) && isLongImmediate(SrcA) && (Switches & TPCII::SW_DIM_MASK_REG) == 0) {
    Inst.setOpcode(TPC::MOVIip);
    if (DecodeIRFRegisterClass(Inst, Dest, Address, Decoder) == MCDisassembler::Fail)
      return MCDisassembler::Fail;
    auto *Disasm = static_cast<const TPCDisassembler *>(Decoder);
    Inst.addOperand(MCOperand::createImm(Disasm->getImmediate()));
    Inst.addOperand(MCOperand::createImm(Switches >> 2));
    HasDimmask = true;

  // IRF <- short Imm
  } else if (isIRF(Dest, true) && isShortImmediate(SrcA) && (Switches & TPCII::SW_DIM_MASK_REG) == 0) {
    Inst.setOpcode(TPC::MOVIip);
    if (DecodeIRFRegisterClass(Inst, Dest, Address, Decoder) == MCDisassembler::Fail)
      return MCDisassembler::Fail;
    Inst.addOperand(MCOperand::createImm(getShortImmediate(SrcA)));
    Inst.addOperand(MCOperand::createImm(Switches >> 2));
    HasDimmask = true;

  // IRF <- IRF
  } else if (isIRF(Dest, true) && isIRF(SrcA, true) && (Switches & TPCII::SW_DIM_MASK_REG) == 0) {
    Inst.setOpcode(TPC::MOVIsp);
    if (DecodeIRFRegisterClass(Inst, Dest, Address, Decoder) == MCDisassembler::Fail)
      return MCDisassembler::Fail;
    if (DecodeIRFRegisterClass(Inst, SrcA, Address, Decoder) == MCDisassembler::Fail)
      return MCDisassembler::Fail;
    Inst.addOperand(MCOperand::createImm(Switches >> 2));
    HasDimmask = true;

  } else {
    return MCDisassembler::Fail;
  }

  if (!isADRF(SrcA, true) && !isADRF(Dest, true) && !isIRF(Dest, true) && !isSPRF(Dest, true)) {
    if (OpType > TPCII::OpType::Max)
      return MCDisassembler::Fail;
    Inst.addOperand(MCOperand::createImm(OpType));
  }
  if (HasDimmask)
    Switches &= ~TPCII::SW_DIMMASK;
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

  // VRF <- VRF
  if (isVRF(Dest) && isVRF(SrcA)) {
    Inst.setOpcode(IsVectorPredicate ? TPC::MOVvvm : TPC::MOVvvp);
    if (DecodeVRFRegisterClass(Inst, Dest, Address, Decoder) == MCDisassembler::Fail) { return MCDisassembler::Fail; }
    if (DecodeVRFRegisterClass(Inst, SrcA, Address, Decoder) == MCDisassembler::Fail) { return MCDisassembler::Fail; }

  // VPRF <- VPRF
  } else if (isVPRF(Dest) && isVPRF(SrcA)) {
    Inst.setOpcode(IsVectorPredicate ? TPC::MOVmmm : TPC::MOVmmp);
    if (DecodeVPRFRegisterClass(Inst, Dest, Address, Decoder) == MCDisassembler::Fail)
      return MCDisassembler::Fail;
    if (DecodeVRFRegisterClass(Inst, SrcA, Address, Decoder) == MCDisassembler::Fail)
      return MCDisassembler::Fail;

  // VPRF <- SPRF
  } else if (isVPRF(Dest) && isSPRF(SrcA, false)) {
    Inst.setOpcode(IsVectorPredicate ? TPC::MOVmpm : TPC::MOVmpp);
    if (DecodeVPRFRegisterClass(Inst, Dest, Address, Decoder) == MCDisassembler::Fail)
      return MCDisassembler::Fail;
    if (DecodeSPRFRegisterClass(Inst, SrcA, Address, Decoder) == MCDisassembler::Fail)
      return MCDisassembler::Fail;

  // VRF <- SRF
  } else if (isVRF(Dest) && isSRF(SrcA, false)) {
    Inst.setOpcode(IsVectorPredicate ? TPC::MOVvsm : TPC::MOVvsp);
    if (DecodeVRFRegisterClass(Inst, Dest, Address, Decoder) == MCDisassembler::Fail)
      return MCDisassembler::Fail;
    if (DecodeSRFRegisterClass(Inst, SrcA, Address, Decoder) == MCDisassembler::Fail)
      return MCDisassembler::Fail;
    Inst.addOperand(MCOperand::createImm(OpType));

  // VRF <- Imm
  } else if (isVRF(Dest) && isLongImmediate(SrcA)) {
    Inst.setOpcode(IsVectorPredicate ? TPC::MOVvim : TPC::MOVvip);
    if (DecodeVRFRegisterClass(Inst, Dest, Address, Decoder) == MCDisassembler::Fail)
      return MCDisassembler::Fail;
    if (OpType > TPCII::OpType::Max)
      return MCDisassembler::Fail;
    auto* Disasm = static_cast<const TPCDisassembler*>(Decoder);
    Inst.addOperand(MCOperand::createImm(Disasm->getImmediate()));
    Inst.addOperand(MCOperand::createImm(OpType));

  // VRF <- short Imm
  } else if (isVRF(Dest) && isShortImmediate(SrcA)) {
    Inst.setOpcode(IsVectorPredicate ? TPC::MOVvim : TPC::MOVvip);
    if (DecodeVRFRegisterClass(Inst, Dest, Address, Decoder) == MCDisassembler::Fail)
      return MCDisassembler::Fail;
    if (OpType > TPCII::OpType::Max)
      return MCDisassembler::Fail;
    Inst.addOperand(MCOperand::createImm(getShortImmediate(SrcA)));
    Inst.addOperand(MCOperand::createImm(OpType));

  // VPRF <- SRF
  } else if (isVPRF(Dest) && isSRF(SrcA, false)) {
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
  } else if (isVPRF(Dest) && isLongImmediate(SrcA)) {
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
  } else if (isVPRF(Dest) && isShortImmediate(SrcA)) {
    if (SrcB == 8)
      Inst.setOpcode(IsVectorPredicate ? TPC::MOVBmim : TPC::MOVBmip);
    else
      Inst.setOpcode(IsVectorPredicate ? TPC::MOVmim : TPC::MOVmip);
    if (DecodeVRFRegisterClass(Inst, Dest, Address, Decoder) == MCDisassembler::Fail)
      return MCDisassembler::Fail;
    if (SrcB > TPCII::OpType::Max)
      return MCDisassembler::Fail;
    Inst.addOperand(MCOperand::createImm(getShortImmediate(SrcA)));
    if (SrcB != 8)
      Inst.addOperand(MCOperand::createImm(SrcB));
  } else {
    return MCDisassembler::Fail;
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

  Inst.setOpcode(TPC::CONVERTssp);
  if (DecodeSRFRegisterClass(Inst, Dest, Address, Decoder) == MCDisassembler::Fail)     //0
    return MCDisassembler::Fail;
  if (DecodeSRFRegisterClass(Inst, SrcA, Address, Decoder) ==  MCDisassembler::Fail)    //1
    return MCDisassembler::Fail;
  Inst.addOperand(MCOperand::createImm(OpType));                                      // 2
  Inst.addOperand(MCOperand::createImm((Switches << 16) | (SrcB << 8)));              // 3
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

  // Adjust instruction opcode.
  bool Src2IsImm = isLongImmediate(SrcB) || isShortImmediate(SrcB);
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
  if (isLongImmediate(SrcB)) {
    Inst.addOperand(MCOperand::createImm(Disasm->getImmediate()));
  } else if (isShortImmediate(SrcB)) {
    Inst.addOperand(MCOperand::createImm(getShortImmediate(SrcB)));
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

  // Adjust instruction opcode.
  bool Src2IsImm = isLongImmediate(SrcB) || isShortImmediate(SrcB);
  bool Src2IsScalar = isSRF(SrcB, false);
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
  if (isLongImmediate(SrcB)) {
    Inst.addOperand(MCOperand::createImm(Disasm->getImmediate()));
  } else if (isShortImmediate(SrcB)) {
    Inst.addOperand(MCOperand::createImm(getShortImmediate(SrcB)));
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
  (void)OpCode;
  assert(OpCode == TPCII::spuADD);

  // Adjust instruction opcode.
  if (!isIRF(Dest, true)) {
    if (DecodeSRFRegisterClass(Inst, Dest, Address, Decoder) == MCDisassembler::Fail)
      return MCDisassembler::Fail;
    if (DecodeSRFRegisterClass(Inst, SrcA, Address, Decoder) == MCDisassembler::Fail)
      return MCDisassembler::Fail;
    if (DecodeSRFRegisterClass(Inst, SrcB, Address, Decoder) == MCDisassembler::Fail)
      return MCDisassembler::Fail;
    if (isLongImmediate(SrcB) || isShortImmediate(SrcB))
      Inst.setOpcode(TPC::ADDsip);
    else
      Inst.setOpcode(TPC::ADDssp);
  } else {
    if (DecodeIRFRegisterClass(Inst, Dest, Address, Decoder) == MCDisassembler::Fail)
      return MCDisassembler::Fail;
    if (isLongImmediate(SrcA)) {
      Inst.addOperand(MCOperand::createImm(Disasm->getImmediate()));
      Inst.setOpcode(TPC::ADDiIp);
    } else if (isIRF(SrcA, true)) {
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
  (void)OpCode;
  assert(OpCode == TPCII::spuSUB);

  // Adjust instruction opcode.
  if (!isIRF(Dest, true)) {
    if (DecodeSRFRegisterClass(Inst, Dest, Address, Decoder) == MCDisassembler::Fail)
      return MCDisassembler::Fail;
    if (DecodeSRFRegisterClass(Inst, SrcA, Address, Decoder) == MCDisassembler::Fail)
      return MCDisassembler::Fail;
    if (DecodeSRFRegisterClass(Inst, SrcB, Address, Decoder) == MCDisassembler::Fail)
      return MCDisassembler::Fail;
    if (isLongImmediate(SrcB) || isShortImmediate(SrcB))
      Inst.setOpcode(TPC::SUBsip);
    else
      Inst.setOpcode(TPC::SUBssp);
  } else {
    if (DecodeIRFRegisterClass(Inst, Dest, Address, Decoder) == MCDisassembler::Fail)
      return MCDisassembler::Fail;
    if (isLongImmediate(SrcA)) {
      Inst.addOperand(MCOperand::createImm(Disasm->getImmediate()));
      Inst.setOpcode(TPC::SUBiIp);
    } else if (isShortImmediate(SrcA)) {
      Inst.addOperand(MCOperand::createImm(getShortImmediate(SrcA)));
      Inst.setOpcode(TPC::SUBiIp);
    } else if (isIRF(SrcA, true)) {
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
  bool IsVPRF = isVPRF(Dest);

  const unsigned SW_TNSR_ID_REG = (1 << 3);

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

  if (OpCode == TPCII::LD_TNSR) {
    if (IsPartial) {
      if (IsTnsrIsReg) {
        Inst.setOpcode(IsVPRF ? (IsVectorPredicate ? TPC::LD_TNSR_PGen2Tmm : TPC::LD_TNSR_PGen2Tmp) : (IsVectorPredicate ? TPC::LD_TNSR_PGen2Tvm: TPC::LD_TNSR_PGen2Tvp));
      } else {
        Inst.setOpcode(IsVPRF ? (IsVectorPredicate ? TPC::LD_TNSR_PGen2mm : TPC::LD_TNSR_PGen2mp) : (IsVectorPredicate ? TPC::LD_TNSR_PGen2vm : TPC::LD_TNSR_PGen2vp));
      }
    } else {
      if (IsTnsrIsReg) {
        Inst.setOpcode(IsVPRF ? (IsVectorPredicate ? TPC::LD_TNSRGen2Tmm : TPC::LD_TNSRGen2Tmp) : (IsVectorPredicate ? TPC::LD_TNSRGen2Tvm : TPC::LD_TNSRGen2Tvp));
      } else {
        Inst.setOpcode(IsVPRF ? (IsVectorPredicate ? TPC::LD_TNSRmm : TPC::LD_TNSRmp) : (IsVectorPredicate ? TPC::LD_TNSRvm : TPC::LD_TNSRvp));
      }
    }
  } else if (OpCode == TPCII::LD_TNSR_HIGH) {
    if (IsTnsrIsReg) {
      Inst.setOpcode(IsVPRF ? (IsVectorPredicate ? TPC::LD_TNSR_HIGHGen2Tmm : TPC::LD_TNSR_HIGHGen2Tmp) : (IsVectorPredicate ? TPC::LD_TNSR_HIGHGen2Tvm : TPC::LD_TNSR_HIGHGen2Tvp));
    } else {
      Inst.setOpcode(IsVPRF ? (IsVectorPredicate ? TPC::LD_TNSR_HIGHmm : TPC::LD_TNSR_HIGHmp) : (IsVectorPredicate ? TPC::LD_TNSR_HIGHvm : TPC::LD_TNSR_HIGHvp));
    }
  } else if (OpCode == TPCII::LD_TNSR_LOW) {
    if (IsTnsrIsReg) {
      Inst.setOpcode(IsVPRF ? (IsVectorPredicate ? TPC::LD_TNSR_LOWGen2Tmm : TPC::LD_TNSR_LOWGen2Tmp) : (IsVectorPredicate ? TPC::LD_TNSR_LOWGen2Tvm : TPC::LD_TNSR_LOWGen2Tvp));
    } else {
      Inst.setOpcode(IsVPRF ? (IsVectorPredicate ? TPC::LD_TNSR_LOWmm : TPC::LD_TNSR_LOWmp) : (IsVectorPredicate ? TPC::LD_TNSR_LOWvm : TPC::LD_TNSR_LOWvp));
    }
  }

  // Data
  if (DecodeVRFRegisterClass(Inst, Dest, Address, Decoder) == MCDisassembler::Fail)
    return MCDisassembler::Fail;

  // Index
  if (DecodeIRFRegisterClass(Inst, SrcA, Address, Decoder) == MCDisassembler::Fail)
    return MCDisassembler::Fail;

  // Tensor
  if (IsTnsrIsReg) {
    Inst.addOperand(MCOperand::createReg(TPC::S27));
  } else {
    Inst.addOperand(MCOperand::createImm(SrcB));
  }

  // Start/Offset
  if (IsPartial) {
    Inst.addOperand(MCOperand::createReg(TPC::S30));
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

  uint64_t OpCode = fieldFromInstruction(insn, TPCII::StOpCodeStart, TPCII::StOpCodeSize);
  uint64_t SrcA = fieldFromInstruction(insn, TPCII::StSrcAStart, TPCII::StSrcASize);
  uint64_t SrcB = fieldFromInstruction(insn, TPCII::StSrcBStart, TPCII::StSrcBSize);
  uint64_t SrcC = fieldFromInstruction(insn, TPCII::StSrcCStart, TPCII::StSrcCSize);
  uint64_t Switches  = fieldFromInstruction(insn, TPCII::StSwitchesStart, TPCII::StSwitchesSize);
  uint64_t Predicate = fieldFromInstruction(insn, TPCII::StPredicateStart, TPCII::StPredicateSize);
  bool IsVectorPredicate = (bool)fieldFromInstruction(insn, TPCII::StVectorPredBit, 1);
  bool Polarity = (bool)fieldFromInstruction(insn, TPCII::StPolarityBit, 1);

  bool IsGaudi = Bits[TPC::FeatureGaudi];
  bool IsVPRF = isVPRF(SrcC);

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

  bool IsDirect = Switches & TPCII::SW_DIRECT;

  if (OpCode == TPCII::ST_TNSR) {
    if (IsPartial) {
      if (IsTnsrIsReg) {
        if (IsRMW) {
          Inst.setOpcode(IsVPRF ? TPC::ST_TNSR_PGen2RTmp : TPC::ST_TNSR_PGen2RTvp);
        } else {
          Inst.setOpcode(IsVPRF ? TPC::ST_TNSR_PGen2Tmp : TPC::ST_TNSR_PGen2Tvp);
        }
      } else {
        if (IsRMW) {
          Inst.setOpcode(IsVPRF ? TPC::ST_TNSR_PGen2Rmp : TPC::ST_TNSR_PGen2Rvp);
        } else {
          Inst.setOpcode(IsVPRF ? TPC::ST_TNSR_PGen2mp : TPC::ST_TNSR_PGen2vp);
        }
      }
    } else {
      if (IsTnsrIsReg) {
        if (IsRMW) {
          Inst.setOpcode(IsVPRF ? TPC::ST_TNSR_RGen2Tmp : TPC::ST_TNSR_RGen2Tvp);
        } else {
          Inst.setOpcode(IsVPRF ? TPC::ST_TNSRGen2Tmp : TPC::ST_TNSRGen2Tvp);
        }
      } else {
        if (IsRMW) {
          Inst.setOpcode(IsVPRF ? TPC::ST_TNSR_RGen2mp : TPC::ST_TNSR_RGen2vp);
        } else {
          Inst.setOpcode(IsVPRF ? TPC::ST_TNSRmp : TPC::ST_TNSRvp);
        }
      }
    }
  }
  else if (OpCode == TPCII::ST_TNSR_HIGH) {
    if (IsTnsrIsReg) {
      if (IsRMW) {
        Inst.setOpcode(IsVPRF ? TPC::ST_TNSR_HIGH_RGen2Tmp : TPC::ST_TNSR_HIGH_RGen2Tvp);
      } else {
        Inst.setOpcode(IsVPRF ? TPC::ST_TNSR_HIGHGen2Tmp : TPC::ST_TNSR_HIGHGen2Tvp);
      }
    } else {
      if (IsRMW) {
        if (IsGaudi)
          Inst.setOpcode(IsVPRF ? TPC::ST_TNSR_HIGH_RGen2mp : TPC::ST_TNSR_HIGH_RGen2vp);
        else
          Inst.setOpcode(IsVPRF ? TPC::ST_TNSR_HIGH_Rmp : TPC::ST_TNSR_HIGH_Rvp);
      } else {
        Inst.setOpcode(IsVPRF ? TPC::ST_TNSR_HIGHmp : TPC::ST_TNSR_HIGHvp);
      }
    }
  }
  else if (OpCode == TPCII::ST_TNSR_LOW) {
    if (IsTnsrIsReg) {
      if (IsRMW) {
        Inst.setOpcode(IsVPRF ? TPC::ST_TNSR_LOW_RGen2Tmp : TPC::ST_TNSR_LOW_RGen2Tvp);
      } else {
        Inst.setOpcode(IsVPRF ? TPC::ST_TNSR_LOWGen2Tmp : TPC::ST_TNSR_LOWGen2Tvp);
      }
    } else {
      if (IsRMW) {
        Inst.setOpcode(IsVPRF ? TPC::ST_TNSR_LOW_RGen2mp : TPC::ST_TNSR_LOW_RGen2vp);
      } else {
        Inst.setOpcode(IsVPRF ? TPC::ST_TNSR_LOWmp : TPC::ST_TNSR_LOWvp);
      }
    }
  }


  // Index
  if (DecodeIRFRegisterClass(Inst, SrcA, Address, Decoder) == MCDisassembler::Fail)
    return MCDisassembler::Fail;

  // Tensor
  if (IsTnsrIsReg) {
    Inst.addOperand(MCOperand::createReg(TPC::S28));
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
    Inst.addOperand(MCOperand::createReg(TPC::S29));
  }

  // Start/Offset
  if (IsPartial) {
    Inst.addOperand(MCOperand::createReg(TPC::S31));
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
  switch (Type) {
  case TPCII::SW_MDG_TYPE_SINGLE:
    Inst.setOpcode(IsVectorPredicate ?
                     TPC::MOV_DUAL_GROUPm_Dis :
                     TPC::MOV_DUAL_GROUPp_Dis);
    break;
  case TPCII::SW_MDG_TYPE_ALL:
    Inst.setOpcode(IsVectorPredicate ?
                     TPC::MOV_DUAL_GROUP_ALLm_Dis :
                     TPC::MOV_DUAL_GROUP_ALLp_Dis);
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

  unsigned SwitchSet = Switches | (SrcB << 8);
  if (Type == TPCII::SW_MDG_TYPE_ALL ||
      Type == TPCII::SW_MDG_TYPE_UNPACK)
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
    } else if (Bits[TPC::FeatureGaudi]) {
      Inst.setOpcode(TPC::UDIV_4STEP);
    }

    if (DecodeZRFRegisterClass(Inst, Dest, Address, Decoder) == MCDisassembler::Fail)
      return MCDisassembler::Fail;

    if (DecodeSRFRegisterClass(Inst, SrcA, Address, Decoder) == MCDisassembler::Fail)
      return MCDisassembler::Fail;

    if (DecodeSRFRegisterClass(Inst, SrcB, Address, Decoder) == MCDisassembler::Fail)
      return MCDisassembler::Fail;

    Inst.addOperand(MCOperand::createImm(Optype));
    Inst.addOperand(MCOperand::createImm(Switches & (1ULL<<6)));
    if (DecodeZRFRegisterClass(Inst, Dest, Address, Decoder) == MCDisassembler::Fail)
      return MCDisassembler::Fail;

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

  uint64_t OpCode = fieldFromInstruction(insn, TPCII::LdOpCodeStart, TPCII::LdOpCodeSize);
  uint64_t SrcA = fieldFromInstruction(insn, TPCII::LdSrcAStart, TPCII::LdSrcASize);
  uint64_t Dest = fieldFromInstruction(insn, TPCII::LdDestStart, TPCII::LdDestSize);
  uint64_t Predicate = fieldFromInstruction(insn, TPCII::LdPredicateStart, TPCII::LdPredicateSize);
  bool IsVectorPredicate = (bool)fieldFromInstruction(insn, TPCII::LdVectorPredBit, 1);
  bool Polarity = (bool)fieldFromInstruction(insn, TPCII::LdPolarityBit, 1);

  assert(OpCode == TPCII::LD_G);
  (void)OpCode;
  unsigned SwitchValue = 0;

  if (isSRF(Dest, false)) {
    Inst.setOpcode(TPC::LD_Gsap);
  } else if (isSPRF(Dest, false)) {
    Inst.setOpcode(TPC::LD_Gpap);
  } else if (isVRF(Dest)) {
    Inst.setOpcode(TPC::LD_Gvap);
  } else {
    llvm_unreachable("Invalid register class in LD_G");
  }


  if (DecodeVRFRegisterClass(Inst, Dest, Address, Decoder) == MCDisassembler::Fail)
    return MCDisassembler::Fail;
  if (DecodeADRFRegisterClass(Inst, SrcA, Address, Decoder) == MCDisassembler::Fail)
    return MCDisassembler::Fail;

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

static DecodeStatus readJmpInstruction(ArrayRef<uint8_t> Bytes,
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

  std::bitset<256> bundle;
  uint8_t bt;
  int32_t val32;

  // Prepare Predicate field.
  bundle = Bundle >> ((Extra.MayCompress) ? TPCII::Gen3SPUPredicateStart:TPCII::SpuPredicateStart);
  memcpy(&bt, &bundle, 1);
  bt &= ((1 << ((Extra.MayCompress) ? TPCII::Gen3SPUPredicateSize:TPCII::SpuPredicateSize)) - 1);
  Extra.Predicate=bt;

  // Prepare Polarity bit.
  bundle = Bundle >> (TPCII::SpuPolarityBit + ((Extra.MayCompress) ? TPCII::Gen3SPUStart:TPCII::SPUStart));
  memcpy(&bt, &bundle, 1);
  bt &= 1;
  Extra.Polarity=bt;

  // Prepare Imm field.
  bundle = Bundle >> ((Extra.MayCompress) ? TPCII::Gen3ImmStart:TPCII::ImmStart);
  memcpy(&val32, &bundle, 4);
  Extra.Imm=val32;

  return MCDisassembler::Success;
}

static DecodeStatus readLoopInstruction(ArrayRef<uint8_t> Bytes,
                                    uint64_t &Size,
                                    std::bitset<256> &Bundle,
                                    uint64_t &InsnLoop,
                                    LoopExtraValues &Extra) {
  const unsigned InstructionSize = 256 / 8;
  static_assert(sizeof(Bundle) == InstructionSize, "Extra fields in std::bitset");
  
  // We want to read exactly 256 bits of data.
  if (Bytes.size() < InstructionSize) {
    Size = 0;
    return MCDisassembler::Fail;
  }

  // Prepare entire bundle.
  memcpy(&Bundle, &Bytes.front(), InstructionSize);
  LLVM_DEBUG(dbgs() << "== Decoding bundle: " << to_hexstring(Bundle) << "\n");

  std::bitset<256> bundle;

  bundle = Bundle >> ((Extra.MayCompress) ? 2:0);
  memcpy(&InsnLoop, &bundle, 8);
  if (Extra.MayCompress) {
    InsnLoop &= ((1ULL << TPCII::Gen3LoopEncSize) - 1);
  } else {
    InsnLoop &= ((1ULL << TPCII::LoopEncSize) - 1);
  }
                                    
  LLVM_DEBUG(dbgs() << "-- LOOP: " << to_hexstring(InsnLoop) << "\n");

  uint32_t val32;

  // Prepare BOUNDARY field.
  bundle = Bundle >> ((Extra.MayCompress) ? TPCII::Gen3LoopBoundaryImmStart:TPCII::LoopBoundaryImmStart);
  memcpy(&val32, &bundle, 4);
  Extra.Boundary=val32;

  // Prepare STEP field.
  bundle = Bundle >> ((Extra.MayCompress) ? TPCII::Gen3LoopStepImmStart:TPCII::LoopStepImmStart);
  memcpy(&val32, &bundle, 4);
  Extra.Step=val32;

  // Prepare OFFSET field.
  bundle = Bundle >> ((Extra.MayCompress) ? TPCII::Gen3LoopOffsetStart : TPCII::LoopOffsetStart);
  uint16_t offset;
  memcpy(&offset, &bundle, 2);
  Extra.Offset=offset;

  // Prepare COMP_MODE field.
  bundle = Bundle >> ((Extra.MayCompress) ? TPCII::Gen3LoopCmpStart : TPCII::LoopCmpStart);
  uint8_t bt;
  memcpy(&bt, &bundle, 1);
  bt &= ((1 << 3) - 1);
  Extra.Comparison=bt;

  // Prepare START field.
  bundle = Bundle >> ((Extra.MayCompress) ? TPCII::Gen3LoopStartImmStart:TPCII::LoopStartImmStart);
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

  bool HasLimit = Switches != 0;
  assert(OpCode == TPCII::vpuFCLASS);

  bool HasSrf = false;
  if (isSRF(SrcA, false))
    HasSrf = true;

  if (HasLimit) {
    if (isSRF(SrcB, false)) {
      if (HasSrf)
        return MCDisassembler::Fail;
      HasSrf = true;
    }

    if (isSRF(SrcD, false))
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

DecodeStatus
TPCDisassembler::tryDecodeLoopInstruction(MCInst &Instr, uint64_t &Size,
                                ArrayRef<uint8_t> Bytes, uint64_t Address,
                                raw_ostream &vStream,
                                raw_ostream &cStream) const {
  std::bitset<256> Bundle;
  uint64_t InsnLoop;
  LoopExtraValues Extra;

  Extra.Address = Address;
  Extra.MayCompress = false;
  Address = (uint64_t)(&Extra);

  DecodeStatus Result = readLoopInstruction(Bytes, Size, Bundle, InsnLoop, Extra);
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

  TPCInstrDecomposer Converter(Bytes, getSubtargetInfo().getFeatureBits());
  DecodeStatus Result;

  if (Converter.getLLVMInstSPU(InsnSPU) == MCDisassembler::Fail ||
      Converter.getLLVMInstVPU(InsnVPU) == MCDisassembler::Fail ||
      Converter.getLLVMInstLoad(InsnLoad) == MCDisassembler::Fail ||
      Converter.getLLVMInstStore(InsnStore) == MCDisassembler::Fail ||
      Converter.getLLVMInstIMM(Immediate) == MCDisassembler::Fail) {
    Size = 0;
    return MCDisassembler::Fail;
  }

  Size = Converter.getBundleSizeInBytes();
  this->Immediate = Immediate;

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

  JmpExtraValues Extra;
  MCInst *SPUSubInst = new (getContext()) MCInst;
  SPUSubInst->setFlags(FlagSPU);
  if (getSPUOpCode(InsnSPU) == TPCII::spuJMPR || getSPUOpCode(InsnSPU) == TPCII::spuJMPA) {
    Extra.Address = Address;
    Extra.MayCompress = false;
    Address = (uint64_t)(&Extra);

    Result = readJmpInstruction(Bytes, Extra);
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
  VPUSubInst->setFlags(FlagVPU);
  Result = decodeInstruction(DecoderTableVectorSlot64, *VPUSubInst, InsnVPU, Address, this, STI);

  if (Result == MCDisassembler::SoftFail)
    Result = decodeInstruction(DecoderTableVectorSlot64, *VPUSubInst, InsnVPU, Address, this, STI);
  if (Result == MCDisassembler::Fail)
    return MCDisassembler::Fail;
  Instr.addOperand(MCOperand::createInst(VPUSubInst));

  MCInst *LDSubInst = new (getContext()) MCInst;
  LDSubInst->setFlags(FlagLDU);
  Result = decodeInstruction(DecoderTableLoadSlot64, *LDSubInst, InsnLoad, Address, this, STI);
  if (Result == MCDisassembler::Fail)
    return MCDisassembler::Fail;
  Instr.addOperand(MCOperand::createInst(LDSubInst));

  MCInst *STSubInst = new (getContext()) MCInst;
  STSubInst->setFlags(FlagSTU);
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
  using SymbolInfoTy = std::tuple<uint64_t, StringRef, uint8_t>;
  using SectionSymbolsTy = std::vector<SymbolInfoTy>;

  if (!IsBranch) {
    return false;
  }

  auto *Symbols = static_cast<SectionSymbolsTy *>(DisInfo);
  if (!Symbols) {
    return false;
  }

  auto Result = std::find_if(Symbols->begin(), Symbols->end(),
                             [SearchValue](const SymbolInfoTy& Val) {
                                return std::get<0>(Val) == static_cast<uint64_t>(SearchValue)
                                    && std::get<2>(Val) == ELF::STT_NOTYPE;
                             });
  if (Result != Symbols->end()) {
    auto *Sym = Ctx.getOrCreateSymbol(std::get<1>(*Result));
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

