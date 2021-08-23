//===---- TPCInstPrinter.cpp - Convert TPC MCInst to asm syntax -----------===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//
//
// This class prints an TPC MCInst to a .s file.
//
//===----------------------------------------------------------------------===//

#include "TPCInstPrinter.h"
#include "TPC.h"
#include "MCTargetDesc/TPCMCInstrInfo.h"
#include "MCTargetDesc/InstructionDB.h"
#include "llvm/MC/MCAsmInfo.h"
#include "llvm/MC/MCExpr.h"
#include "llvm/MC/MCInst.h"
#include "llvm/MC/MCSymbol.h"
#include "llvm/MC/MCInstrInfo.h"
#include "llvm/MC/MCSubtargetInfo.h"
#include "llvm/Support/ErrorHandling.h"
#include "llvm/Support/FormattedStream.h"
#include "llvm/Support/CommandLine.h"
#include <bitset>

using namespace llvm;

const int TPCInstPrinter::InstructionPerLineNoNops = 0;
const int TPCInstPrinter::InstructionPerLine = 1;
const int TPCInstPrinter::BundlePerLine = 2;
const int TPCInstPrinter::TpcAsmParseCompatible = 3;

#define DEBUG_TYPE "asm-printer"

// Include the auto-generated portion of the assembly writer.
#define PRINT_ALIAS_INSTR
#include "TPCGenAsmWriter.inc"

void TPCInstPrinter::printRegName(raw_ostream &OS, unsigned RegNo) const {
  OS << StringRef(getRegisterName(RegNo)).lower();
}

void TPCInstPrinter::printAddrOperand(const MCInst* MI, unsigned OpNum, raw_ostream& O) {
  const MCOperand &MO1 = MI->getOperand(OpNum);

  if (MO1.isImm()) {
    O << formatHex(MO1.getImm());
  } else {
    assert(MO1.isReg() && "First part of an address should be a register or an immediate");
    if (HasPercentPrefix)
      O << "%";
    O << getRegisterName(MO1.getReg());
  }

  if (TPCII::getSubtargetInfo()->hasFeature(TPC::FeatureAddr2)) {
    const MCOperand &MO2 = MI->getOperand(OpNum + 1);
    O << ", ";
    if (MO2.isImm()) {
      O << formatHex(MO2.getImm());
    } else {
      assert(MO2.isReg() && "First part of an address should be a register or an immediate");
      if (HasPercentPrefix)
        O << "%";
      O << getRegisterName(MO2.getReg());
    }
  }
}

void TPCInstPrinter::printSPredicate(const MCInst *MI, unsigned OpNum, raw_ostream &O) {
  const MCOperand &PredReg = MI->getOperand(OpNum);
  const MCOperand &Polarity = MI->getOperand(OpNum + 1);

  assert(PredReg.isReg() && "Predicate must be a register");
  assert(Polarity.isImm() && "Polarity must be an immediate");
  assert(MRI.getRegClass(TPC::SPRFRegClassID).contains(PredReg.getReg()) &&
         "Predicate register must be from SPRF");
  if (Polarity.getImm())
    O << "!";
  if (HasPercentPrefix)
    O << "%";
  O << getRegisterName(PredReg.getReg());
}

void TPCInstPrinter::printVPredicate(const MCInst *MI, unsigned OpNum, raw_ostream &O) {
  const MCOperand &PredReg = MI->getOperand(OpNum);
  const MCOperand &Polarity = MI->getOperand(OpNum + 1);

  assert(PredReg.isReg() && "Predicate must be a register");
  assert(Polarity.isImm() && "Polarity must be an immediate");
  assert(MRI.getRegClass(TPC::VPRFRegClassID).contains(PredReg.getReg()) &&
         "Predicate register must be from VPRF");
  if (Polarity.getImm())
    O << "!";
  if (HasPercentPrefix)
    O << "%";
  O << getRegisterName(PredReg.getReg());
}

void TPCInstPrinter::printDataType(const MCInst *MI, unsigned OpNum, raw_ostream &O) {
  const MCOperand &Op = MI->getOperand(OpNum);

  assert(Op.isImm() && "TypeOp must be an immediate");
  static const StringRef OpTypes[] = {
    ".f32",
    ".bf16",
    ".i32",
    ".u32",
    ".i8",
    ".u8",
    ".b",
    ".i16",
    ".u16",
    ".i4",
    ".u4",
    ".f16",
    ".f8_152",
    ".f8_143",
    ".i64"
  };
  unsigned Ndx = Op.getImm();
  assert(Ndx < array_lengthof(OpTypes));
  StringRef DTName = OpTypes[Ndx];

  // Don't print datatype operand if instruction mnemonic already contains it.
  // FIXME: getOpcodeName returns a TableGen's record name, but essentially we want an AsmString.
  // However there is no API to get the AsmString...
  StringRef InsnName = getOpcodeName(MI->getOpcode());
  if (InsnName.contains(DTName.drop_front(1)))
    return;

  O << DTName;
}

void TPCInstPrinter::printDimMask(const MCInst *MI, unsigned OpNum, raw_ostream &O) {
  const MCOperand &Op = MI->getOperand(OpNum);

  assert(Op.isImm() && "DimMask must be an immediate");
  unsigned Mask = Op.getImm();
  assert(Mask < 32);

  O << 'b' << std::bitset<5>(Mask).to_string();
}

void TPCInstPrinter::printSwitchSet(const MCInst *MI, unsigned OpNum, raw_ostream &O) {
  const MCOperand &Op = MI->getOperand(OpNum);

  assert(Op.isImm() && "Switches must be an immediate");
  O << TPCII::spellSwitchSet(Op.getImm(), MI, OpNum, MII.get(MI->getOpcode()), MRI);
}

void TPCInstPrinter::printJmpLoopTarget(const MCInst *MI, unsigned OpNum, raw_ostream &O) {
  const MCOperand &Op = MI->getOperand(OpNum);
  if (Op.isImm()) {
    O << Op.getImm();  
  } else if (Op.isExpr()) {
    const MCExpr *Exp = Op.getExpr();
    Exp->print(O, &MAI);
  }
}

void TPCInstPrinter::printComparison(const MCInst *MI, unsigned OpNum, raw_ostream &O) {
  const MCOperand &Op = MI->getOperand(OpNum);
  static const StringRef LoopCompareText[] = {
    ">", ">=", "<", "<=", "==", "!="
  };

  assert(Op.isImm() && "Switches must be an immediate");
  unsigned Cmp = Op.getImm();
  assert(Cmp < array_lengthof(LoopCompareText));
  O << LoopCompareText[Cmp];

}

void TPCInstPrinter::printLoopImm(const MCInst *MI, unsigned OpNum, raw_ostream &O) {
  const MCOperand &Op = MI->getOperand(OpNum);
  if (!Op.isImm()) {
    return;
  }
  assert(Op.isImm() && "LoopImm must be an immediate");
  O << Op.getImm();
}

void TPCInstPrinter::printAccumulator(const MCInst *, unsigned, raw_ostream &) {
}

void TPCInstPrinter::printRhaz(const MCInst *, unsigned, raw_ostream &) {
}

void TPCInstPrinter::printRhazRs(const MCInst *, unsigned, raw_ostream &) {
}

void TPCInstPrinter::printRhu(const MCInst *MI, unsigned OpNum, raw_ostream &O) {
  const MCOperand &Op = MI->getOperand(OpNum);
  assert(Op.isImm() && "RHU must be an immediate");

  switch(Op.getImm()) {
  default: llvm_unreachable("Unknown RHU value");
    case 2:  O << "RHU";      break;
  }
}

void TPCInstPrinter::printBothDivMod(const MCInst *, unsigned, raw_ostream &) {
}
void TPCInstPrinter::printX2(const MCInst *, unsigned, raw_ostream &) {
}

void TPCInstPrinter::printMovDGAll(const MCInst *, unsigned, raw_ostream &O) {
  O << "all";
}

void TPCInstPrinter::printMovDGPack(const MCInst *, unsigned, raw_ostream &O) {
  O << "pack";
}

void TPCInstPrinter::printMovDGUnpack(const MCInst *, unsigned, raw_ostream &O) {
  O << "unpack";
}

//
// Helper function for printing the bundle in one row, sorted by slots
//
void getSortedInstrsFromBundle(const MCInst *bundle, MCInst** array_p, MCInstrInfo MII) {
    int idx;
    for (auto &I : TPCMCInstrInfo::bundleInstructions(*bundle)) {
      MCInst *MI = const_cast<MCInst*>(I.getInst());
      const MCInstrDesc& MCI = MII.get(MI->getOpcode());
      if (TPCII::isLoadInst(MCI)) {
        idx = 0;
      }
      else if (TPCII::isSPUInst(MCI)) {
        idx = 1;
      }
      else if (TPCII::isVPUInst(MCI)) {
        idx = 2;
      }
      else if (TPCII::isStoreInst(MCI)) {
        idx = 3;
      }
      else {
        assert(0 && "Unknown instruction kind in bundle");
      }
      array_p[idx] = MI;
    }
}

static bool isNOP(const MCInst & Inst) {
  switch (Inst.getOpcode()) {
  case TPC::NOPld:
  case TPC::NOPs:
  case TPC::NOPv:
  case TPC::NOPst:
    return true;
  default:
    return false;
  }
}

void TPCInstPrinter::printInst(const MCInst *MI, uint64_t Address,
                               StringRef Annotation, const MCSubtargetInfo &STI,
                               raw_ostream &OS) {
  TPCII::setSubTargetInfo(&STI);

  if (MI->getOpcode() == TPC::BUNDLE) {
    if (getFormat() == InstructionPerLine ||
        getFormat() == InstructionPerLineNoNops) {
      int icount = 0; // how many instrs in the bundle are going to be really printed
      for (auto &I : TPCMCInstrInfo::bundleInstructions(*MI)) {
        MCInst &Bundle_MI = const_cast<MCInst &>(*I.getInst());
        // Do not print NOP instructions inside bundle.
        if (getFormat() == InstructionPerLineNoNops && isNOP(Bundle_MI))
          continue;
        icount++;
      }

      if (icount > 1) {
        OS << "{\n";
      } else if (icount == 0) {
        OS << "\tNOP";
      }
      for (auto &I : TPCMCInstrInfo::bundleInstructions(*MI)) {
        const MCInst &Bundle_MI = *I.getInst();
        // Do not print NOP instructions inside bundle.
        if (getFormat() == InstructionPerLineNoNops && isNOP(Bundle_MI))
          continue;
        printInstruction(&Bundle_MI, Address, OS);
        if (icount > 1) {
          OS << "\n";
        }
      }
      if (icount > 1) {
        OS << "}";
      }
    } else { // BundlePerLine
      MCInst *array[4] = {nullptr,nullptr,nullptr,nullptr};
      getSortedInstrsFromBundle(MI, (MCInst**)(&array[0]), MII);
      for (int i=0; i<4; i++) {
        if (array[i]) {
          printInstruction(array[i], Address, OS);
        } else {
          OS << "\t";
        }
        if (i < 3) {
          OS << "; ";
        }
      }
    }
  } else {
    printInstruction(MI, Address, OS);
  }
  printAnnotation(OS, Annotation);
}

bool TPCInstPrinter::printInst(const MCInst *MI, raw_ostream &OS,
                                 StringRef Alias, unsigned OpNo0,
                                 unsigned OpNo1) {
  OS << "\t" << Alias << " ";
  printOperand(MI, OpNo0, OS);
  OS << ", ";
  printOperand(MI, OpNo1, OS);
  return true;
}

void TPCInstPrinter::printOperand(const MCInst *MI, unsigned OpNo,
                                  raw_ostream &OS) {
  const MCOperand &Op = MI->getOperand(OpNo);
  if (Op.isReg()) {
    if (HasPercentPrefix)
      OS << "%";
    OS << getRegisterName(Op.getReg());
  } else if (Op.isImm()) {
    OS << formatHex(Op.getImm());
  } else if (Op.isFPImm()) {
    float Val = (float)Op.getFPImm();
    uint64_t ival = 0l;
    // Using this two-step static_cast via void * instead of cast
    // silences a -Wstrict-aliasing false positive from GCC6 and earlier.
    //((unsigned*)&ival) [0] = *((unsigned*)&Val);
    void *Storage = static_cast<void *>(&Val);
    ival= *static_cast<uint64_t *>(Storage);
    OS << formatHex(ival);
  }
  else {
    assert(Op.isExpr() && "Expected an expression");
    Op.getExpr()->print(OS, &MAI);
  }
}
