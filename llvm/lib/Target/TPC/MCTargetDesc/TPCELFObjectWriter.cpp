//===-- TPCELFObjectWriter.cpp - Mips ELF Writer -------------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
//
//===----------------------------------------------------------------------===//

#include <algorithm>
#include <list>
#include "MCTargetDesc/TPCMCTargetDesc.h"
#include "llvm/ADT/STLExtras.h"
#include "llvm/MC/MCAssembler.h"
#include "llvm/MC/MCELFObjectWriter.h"
#include "llvm/MC/MCExpr.h"
#include "llvm/MC/MCSection.h"
#include "llvm/MC/MCSymbolELF.h"
#include "llvm/MC/MCValue.h"
#include "llvm/Support/Debug.h"
#include "llvm/Support/ErrorHandling.h"

#define DEBUG_TYPE "tpc-elf-object-writer"

using namespace llvm;

class TPCELFObjectWriter : public MCELFObjectTargetWriter {
public:
  TPCELFObjectWriter()
      : MCELFObjectTargetWriter(/*Is64Bit*/ false, 0, ELF::EM_386, // i386 for now
                                /*HasRelocationAddend*/ false) {}

  // MCELFObjectTargetWriter interface
public:
  unsigned getRelocType(MCContext& Ctx, const MCValue& Target, const MCFixup& Fixup, bool IsPCRel) const {
    MCSymbolRefExpr::VariantKind Modifier = Target.getAccessVariant();
    unsigned Kind = Fixup.getKind();
    switch (Modifier) {
    case MCSymbolRefExpr::VK_None:
      if (IsPCRel) {
        return 0;
        //llvm_unreachable("Relocatable fixups are not supported");
      } else {
        switch (Kind) {
        case FK_Data_4:
          return ELF::R_386_32;
        default:
          llvm_unreachable("Unknown ELF relocation type");
        }
      }

    default:
      llvm_unreachable("Modifier not supported");
    }
  }
};

std::unique_ptr<MCObjectTargetWriter> llvm::createTPCELFObjectWriter() {
  return std::make_unique<TPCELFObjectWriter>();
}
