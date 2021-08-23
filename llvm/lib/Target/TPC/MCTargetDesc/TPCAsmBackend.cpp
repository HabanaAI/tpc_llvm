//===-- TPCAsmBackend.cpp - TPC Assembler Backend -------------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
//
//===----------------------------------------------------------------------===//

#include "MCTargetDesc/TPCMCTargetDesc.h"
#include "MCTargetDesc/TPCAsmBackend.h"
#include "MCTargetDesc/TPCMCInstrInfo.h"
#include "MCTargetDesc/TPCMCTargetDesc.h"
#include "llvm/ADT/StringSwitch.h"
#include "llvm/ADT/APInt.h"
#include "llvm/BinaryFormat/ELF.h"
#include "llvm/MC/MCAsmBackend.h"
#include "llvm/MC/MCAssembler.h"
#include "llvm/MC/MCContext.h"
#include "llvm/MC/MCDirectives.h"
#include "llvm/MC/MCExpr.h"
#include "llvm/MC/MCFixupKindInfo.h"
#include "llvm/MC/MCMachObjectWriter.h"
#include "llvm/MC/MCELFObjectWriter.h"
#include "llvm/MC/MCObjectWriter.h"
#include "llvm/MC/MCRegisterInfo.h"
#include "llvm/MC/MCSectionELF.h"
#include "llvm/MC/MCSectionMachO.h"
#include "llvm/MC/MCSubtargetInfo.h"
#include "llvm/MC/MCValue.h"
#include "llvm/Support/Debug.h"
#include "llvm/Support/ErrorHandling.h"
#include "llvm/Support/Format.h"
#include "llvm/Support/TargetParser.h"
#include "llvm/Support/raw_ostream.h"
using namespace llvm;

#define DEBUG_TYPE "mccodeemitter"

namespace llvm {
MCAsmBackend *createTPCAsmBackend(const Target &T,
                                  const MCSubtargetInfo &STI,
                                  const MCRegisterInfo &MRI,
                                  const MCTargetOptions &Options) {
  const Triple &TheTriple = STI.getTargetTriple();
  return new TPCAsmBackend(T, TheTriple);
}
} //namespace llvm

TPCAsmBackend::TPCAsmBackend(const Target& T, const Triple& TT)
  : MCAsmBackend(support::endianness::little) {
}

std::unique_ptr<MCObjectTargetWriter> TPCAsmBackend::createObjectTargetWriter() const {
  return createTPCELFObjectWriter();
}

unsigned TPCAsmBackend::getNumFixupKinds() const {
  return TPC::NumTargetFixupKinds;
}

const MCFixupKindInfo &TPCAsmBackend::getFixupKindInfo(MCFixupKind Kind) const {
  const static MCFixupKindInfo InfosLE[TPC::NumTargetFixupKinds] = {
    {"fixup_loop", 0, 16, MCFixupKindInfo::FKF_IsPCRel},
    {"fixup_loop", 0, 16, MCFixupKindInfo::FKF_IsPCRel}
  };
  if (Kind < FirstTargetFixupKind)
    return MCAsmBackend::getFixupKindInfo(Kind);

  assert(unsigned(Kind - FirstTargetFixupKind) < getNumFixupKinds() &&
         "Invalid kind!");
  return InfosLE[Kind - FirstTargetFixupKind];
}

void TPCAsmBackend::applyFixup(const MCAssembler &Asm, const MCFixup &Fixup,
                               const MCValue &Target, MutableArrayRef<char> Data,
                               uint64_t Value, bool IsResolved,
                               const MCSubtargetInfo *STI) const {
  unsigned Offset = Fixup.getOffset();
  if (Fixup.getKind() == FK_Data_4) {
    void *Ptr = &Data[Fixup.getOffset()];
    memcpy(Ptr, &Value, sizeof(uint32_t));
  } else if (Fixup.getKind() == FK_PCRel_4) {
    int RelOffset = Value & 0xffffffff;
    unsigned InstrSize = TPCII::InstructionSize;
    LLVM_DEBUG( fprintf(stderr, "applyFixup offset=%d reloc=%d comp=%d\n", Offset, RelOffset, (Data[Offset] & 3)); );

    assert(InstrSize % 64 == 0 && "Instruction is not aligned to 64 bits anymore, fix relocations");
    APInt Instruction(InstrSize, ArrayRef<uint64_t>((uint64_t*)(&Data[Offset]), InstrSize / 64));
    APInt ImmSlot(TPCII::ImmSize, RelOffset);
    Instruction |= ImmSlot.zext(InstrSize).shl(TPCII::ImmStart);

    const char* RawInstrucion = (const char*) Instruction.getRawData();
    for (unsigned i = 0; i < InstrSize / 8; ++i) {
      Data[Offset + i] = RawInstrucion[i];
    }
  } else if (Fixup.getKind() == MCFixupKind(TPC::FK_LOOP)) {
    assert(Value != 0 && "Too short LOOP");
    if ((Value & 0xffff) != Value) {
      report_fatal_error("Too many instructions in the LOOP - END_PC offset does not fit in 16 bits");
    }
    unsigned InstrSize = TPCII::InstructionSize;
    int RelOffset = (Value & 0xffff) - TPCII::InstructionSize/8; // LoopEnd + 1
    LLVM_DEBUG( fprintf(stderr, "applyFixup LOOP offset=%d reloc=%d comp=%d\n", Offset, RelOffset, (Data[Offset] & 3)); );

    APInt Instruction(TPCII::InstructionSize, ArrayRef<uint64_t>((uint64_t*)(&Data[Offset]), TPCII::InstructionSize / 64));
    APInt ImmSlot(TPCII::ImmSize, RelOffset);
    Instruction |= ImmSlot.zext(TPCII::InstructionSize).shl(TPCII::LoopOffsetStart);

    const char* RawInstrucion = (const char*) Instruction.getRawData();
    for (unsigned i = 0; i < InstrSize / 8; ++i) {
      Data[Offset + i] = RawInstrucion[i];
    }
  }

}

bool TPCAsmBackend::mayNeedRelaxation(const MCInst& Inst, const MCSubtargetInfo &STI) const {
  return false;
}

bool TPCAsmBackend::fixupNeedsRelaxation(const MCFixup& Fixup, uint64_t Value, const MCRelaxableFragment* DF, const MCAsmLayout& Layout) const {
  return false;
}

void TPCAsmBackend::relaxInstruction(const MCInst& Inst, const MCSubtargetInfo& STI, MCInst& Res) const {

}

bool TPCAsmBackend::writeNopData(raw_ostream &OS, uint64_t Count) const {
  //This should not be called, nops are inserted into bundles
  return true;
}
