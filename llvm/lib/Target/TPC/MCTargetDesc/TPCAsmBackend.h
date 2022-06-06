//===-- TPCAsmBackend.h - TPC Assembler Backend -----------------*- C++ -*-===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

#ifndef LLVM_LIB_TARGET_TPC_TPCASMBACKEND_H
#define LLVM_LIB_TARGET_TPC_TPCASMBACKEND_H

#include "MCTargetDesc/TPCMCTargetDesc.h"
#include "llvm/MC/MCAsmBackend.h"
#include "llvm/MC/MCSubtargetInfo.h"
#include "llvm/Support/TargetRegistry.h"

namespace llvm {

namespace TPC {
enum Fixups {
  FK_Loop = FirstTargetFixupKind,
  FK_LOOP, // TODO: Remove it
  LastTargetFixupKind,
  NumTargetFixupKinds = LastTargetFixupKind - FirstTargetFixupKind
};
}

class TPCAsmBackend : public MCAsmBackend {
public:
  TPCAsmBackend(const Target &T, const Triple &TT);

  ~TPCAsmBackend() override { }

  // MCAsmBackend interface
public:
//  MCObjectWriter *createObjectWriter(raw_pwrite_stream& OS) const override;
  std::unique_ptr<MCObjectTargetWriter> createObjectTargetWriter() const override;
  unsigned getNumFixupKinds() const override;
  void applyFixup(const MCAssembler &Asm, const MCFixup &Fixup,
                  const MCValue &Target, MutableArrayRef<char> Data,
                  uint64_t Value, bool IsResolved,
                  const MCSubtargetInfo *STI) const override;
  bool mayNeedRelaxation(const MCInst &Inst, const MCSubtargetInfo &STI) const override;
  bool fixupNeedsRelaxation(const MCFixup& Fixup, uint64_t Value, const MCRelaxableFragment* DF,
                            const MCAsmLayout& Layout) const override;
  bool writeNopData(raw_ostream &OS, uint64_t Count) const override;
  const MCFixupKindInfo &getFixupKindInfo(MCFixupKind Kind) const override;
};
} // end namespace llvm

#endif
