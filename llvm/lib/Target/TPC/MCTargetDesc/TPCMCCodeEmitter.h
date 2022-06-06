//===-- TPCMCCodeEmitter.h - Convert TPC Code to Machine Code -----------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
//
// This file defines the TPCMCCodeEmitter class.
//
//===----------------------------------------------------------------------===//
//

#ifndef LLVM_LIB_TARGET_TPC_MCTARGETDESC_TPCMCCODEEMITTER_H
#define LLVM_LIB_TARGET_TPC_MCTARGETDESC_TPCMCCODEEMITTER_H

#include "llvm/ADT/APInt.h"
#include "llvm/MC/MCCodeEmitter.h"
#include "llvm/Support/DataTypes.h"
#include <inttypes.h>

using namespace llvm;

namespace llvm {
class MCContext;
class MCExpr;
class MCInst;
class MCInstrInfo;
class MCFixup;
class MCOperand;
class MCSubtargetInfo;
class raw_ostream;

class TPCInstrBits {
public:
  uint64_t VPUInst;
  uint64_t SPUInst;
  uint64_t LDInst;
  uint64_t STInst;
  uint16_t LdSrcExtra;
  uint8_t  StSrcExtra;
  uint8_t  LdSwitch;
  uint8_t  StSwitch;
  uint64_t Imm;
  bool     immSlotBusy;
  unsigned compress;
  bool     inited;
  bool hasVPU;
  bool hasSPU;
  bool hasLD;
  bool hasST;
  unsigned SPUPredicated;
  unsigned VPUPredicated;
  unsigned StorePredicated;
  unsigned LoadPredicated;
  bool doNotCompress;
  const MCInst* MI;
public:
  TPCInstrBits()
      : VPUInst(0L), SPUInst(0L), LDInst(0L), STInst(0L),
        LdSrcExtra(0), StSrcExtra(0), LdSwitch(0), StSwitch(0),
	Imm(0L), immSlotBusy(false), compress(0), inited(false)
  {
    hasVPU = false;
    hasSPU = false;
    hasLD = false;
    hasST = false;
    doNotCompress = false;
    SPUPredicated = 0;
    VPUPredicated = 0;
    StorePredicated = 0;
    LoadPredicated = 0;
  }

  ~TPCInstrBits() {}
  
  void clear() {
    hasVPU = false;
    hasSPU = false;
    hasLD = false;
    hasST = false;

    VPUInst = 0L;
    SPUInst = 0L;
    LDInst = 0L;
    STInst = 0L;

    LdSrcExtra = 0;
    StSrcExtra = 0;
    LdSwitch = 0;
    StSwitch = 0;
    Imm = 0L;
    immSlotBusy = false;

    compress = 0;
    inited = false;
    doNotCompress = false;

    SPUPredicated = 0;
    VPUPredicated = 0;
    StorePredicated = 0;
    LoadPredicated = 0;
  }
};

class TPCMCCodeEmitter : public MCCodeEmitter {
  TPCMCCodeEmitter(const TPCMCCodeEmitter &) = delete;
  void operator=(const TPCMCCodeEmitter &) = delete;
  const MCInstrInfo &MCII;
  MCContext &Ctx;
  mutable TPCInstrBits prevBits;

public:
  TPCMCCodeEmitter(const MCInstrInfo &mcii, MCContext &Ctx_)
      : MCII(mcii), Ctx(Ctx_) {}

  ~TPCMCCodeEmitter() override {}
  
  int isVPUInst(const MCInst &MI) const;
  int isSPUInst(const MCInst &MI) const;
  int isLoadInst(const MCInst &MI) const;
  int isStoreInst(const MCInst &MI) const;
  int isLoopInst(const MCInst &MI) const;

  void EmitByte(unsigned char C, raw_ostream &OS) const;

  void EmitInstruction(APInt &Instruction, unsigned Size, raw_ostream &OS) const;

  void encodeInstruction(const MCInst &MI, raw_ostream &OS,
                         SmallVectorImpl<MCFixup> &Fixups,
                         const MCSubtargetInfo &STI) const override;

  void emitBits(raw_ostream& OS, TPCInstrBits *Bits, const MCSubtargetInfo &STI) const;

  void fillInstrBits(const MCInst& MI,
                     SmallVectorImpl<MCFixup>& Fixups,
                     TPCInstrBits& Bits,
                     const MCSubtargetInfo& STI,
                     bool ignoreNops,
                     bool isSingle = true,
                     const Optional<uint64_t>& StoreSrcC = Optional<uint64_t>()) const;

  // getBinaryCodeForInstr - TableGen'erated function for getting the
  // binary encoding for an instruction.
  uint64_t getBinaryCodeForInstr(const MCInst &MI,
                                 SmallVectorImpl<MCFixup> &Fixups,
                                 const MCSubtargetInfo &STI) const;

  /// getMachineOpValue - Return binary encoding of operand. If the machine
  /// operand requires relocation, record the relocation and return zero.
  unsigned getMachineOpValue(const MCInst &MI,const MCOperand &MO,
                             SmallVectorImpl<MCFixup> &Fixups,
                             const MCSubtargetInfo &STI) const;

  unsigned getRrMemoryOpValue(const MCInst &Inst, unsigned OpNo,
                              SmallVectorImpl<MCFixup> &Fixups,
                              const MCSubtargetInfo &SubtargetInfo) const;
  unsigned getRiMemoryOpValue(const MCInst &Inst, unsigned OpNo,
                              SmallVectorImpl<MCFixup> &Fixups,
                              const MCSubtargetInfo &SubtargetInfo) const;
  unsigned encodePredicate(const MCInst &Inst, unsigned OpNo,
                           SmallVectorImpl<MCFixup> &Fixups,
                           const MCSubtargetInfo &SubtargetInfo) const;

  void EmitConstant(uint64_t Val, unsigned Size, raw_ostream &OS) const {
    // Output the constant in little endian byte order.
    for (unsigned i = 0; i != Size; ++i) {
      unsigned Shift =  i * 8;
      EmitByte((Val >> Shift) & 0xff, OS);
    }
  }

  uint64_t getInstrBits(const MCInst& MI, SmallVectorImpl<MCFixup>& Fixups, const MCSubtargetInfo& STI) const;

  void encodeLoop(const MCInst &Inst, raw_ostream& OS, uint64_t Bin, SmallVectorImpl<MCFixup>& Fixups,
                  const MCSubtargetInfo &STI) const;

  unsigned encodeTPCImm(const MCInst &Inst, unsigned OpNo,
                           SmallVectorImpl<MCFixup> &Fixups,
                           const MCSubtargetInfo &SubtargetInfo) const;
  bool mayCompress(const MCSubtargetInfo &STI) const;
  bool isGoya2(const MCSubtargetInfo &STI) const;
  bool isGaudi2(const MCSubtargetInfo &STI) const;
  
}; // class TPCMCCodeEmitter
} // namespace llvm.

#endif
