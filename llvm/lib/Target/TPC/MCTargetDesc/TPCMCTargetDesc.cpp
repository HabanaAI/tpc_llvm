//===-- TPCMCTargetDesc.cpp - TPC Target Descriptions -----------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
//
// This file provides TPC specific target descriptions.
//
//===----------------------------------------------------------------------===//

#include "TPCTargetMachine.h"
#include "TPCMCTargetDesc.h"
#include "TPCMCAsmInfo.h"
#include "TPCAsmBackend.h"
#include "TPCInstPrinter.h"
#include "llvm/MC/MCInstrInfo.h"
#include "llvm/MC/MCRegisterInfo.h"
#include "llvm/MC/MCSubtargetInfo.h"
#include "llvm/Support/ErrorHandling.h"
#include "llvm/Support/TargetRegistry.h"

using namespace llvm;

extern Target TheTPCTarget;

#define GET_INSTRINFO_MC_DESC
#include "TPCGenInstrInfo.inc"

#define GET_SUBTARGETINFO_MC_DESC
#include "TPCGenSubtargetInfo.inc"

#define GET_REGINFO_MC_DESC
#include "TPCGenRegisterInfo.inc"


static MCAsmInfo *createTPCMCAsmInfo(const MCRegisterInfo &MRI,
                                     const Triple &TT,
                                     const MCTargetOptions &Options) {
  MCAsmInfo *MAI = new TPCMCAsmInfo(TT);
// unsigned Reg = 0;
// MRI.getDwarfRegNum(SP::O6, true);
// DEBUG:  MCCFIInstruction Inst = MCCFIInstruction::createDefCfa(nullptr, Reg, 0);
// MAI->addInitialFrameState(Inst);
  return MAI;
}

static MCInstrInfo *createTPCMCInstrInfo() {
  MCInstrInfo *X = new MCInstrInfo();
  InitTPCMCInstrInfo(X);
  return X;
}

static MCRegisterInfo *createTPCMCRegisterInfo(const Triple &TT) {
  MCRegisterInfo *X = new MCRegisterInfo();
  InitTPCMCRegisterInfo(X, 32/*SP::O7*/);
  return X;
}

static MCSubtargetInfo *
createTPCMCSubtargetInfo(const Triple &TT, StringRef CPU, StringRef FS) {
  std::string arch;
  if (CPU.empty())
    CPU = "goya";
  else if (CPU == "dali")
    CPU = "goya";

  if (CPU.equals("goya") ||
      CPU.equals("gaudi") )
    return createTPCMCSubtargetInfoImpl(TT, CPU, FS);

  return nullptr;
}

/*static MCTargetStreamer *
createObjectTargetStreamer(MCStreamer &S, const MCSubtargetInfo &STI) {
  return new TPCTargetELFStreamer(S);
}
*/

static cl::opt<int> TPCAsmFormat(
    "tpc-asm-format", cl::Hidden, cl::ZeroOrMore,
    cl::init(TPCInstPrinter::InstructionPerLineNoNops),
    cl::desc(
        "Asm output format:\n"
        "   0 - one line per instruction, NOPs are skipped\n"
        "   1 - one line per instruction, all slots\n"
        "   2 - in a single row, separated by ';'\n"
        "   3 - in a single row, separated by ';' + TpcAsmParse Compatible"));

static MCTargetStreamer *createTargetAsmStreamer(MCStreamer &S,
                                                 formatted_raw_ostream &OS,
                                                 MCInstPrinter *InstPrint,
                                                 bool isVerboseAsm) {
  // Use this callback to tune TPCInstPrinter differently for Assembler
  // and Disassembler.
  auto TPCPrinter = static_cast<TPCInstPrinter *>(InstPrint);
  TPCPrinter->setFormat(TPCAsmFormat);
  bool hasPercentPrefix =
      (TPCAsmFormat == TPCInstPrinter::TpcAsmParseCompatible) ? false : true;
  TPCPrinter->setHasPercentPrefix(hasPercentPrefix);
  return nullptr;
}

static MCInstPrinter *createTPCMCInstPrinter(const Triple &T,
                                               unsigned SyntaxVariant,
                                               const MCAsmInfo &MAI,
                                               const MCInstrInfo &MII,
                                               const MCRegisterInfo &MRI) {
  return new TPCInstPrinter(MAI, MII, MRI);
}

extern "C" void LLVMInitializeTPCTargetMC() {
  // Register the MC asm info.
  RegisterMCAsmInfoFn X(TheTPCTarget, createTPCMCAsmInfo);

    // Register the MC instruction info.
    TargetRegistry::RegisterMCInstrInfo(TheTPCTarget, createTPCMCInstrInfo);

    // Register the MC register info.
    TargetRegistry::RegisterMCRegInfo(TheTPCTarget, createTPCMCRegisterInfo);

    // Register the MC subtarget info.
    TargetRegistry::RegisterMCSubtargetInfo(TheTPCTarget, createTPCMCSubtargetInfo);

    // Register the MC Code Emitter.
    TargetRegistry::RegisterMCCodeEmitter(TheTPCTarget, createTPCMCCodeEmitter);

    // Register the asm backend.
    TargetRegistry::RegisterMCAsmBackend(TheTPCTarget, createTPCAsmBackend);

    // Register the object target streamer.
    //TargetRegistry::RegisterObjectTargetStreamer(TheTPCTarget,
     //                                            createObjectTargetStreamer);

  // Register the asm streamer.
  TargetRegistry::RegisterAsmTargetStreamer(TheTPCTarget, createTargetAsmStreamer);

  // Register the MCInstPrinter
  TargetRegistry::RegisterMCInstPrinter(TheTPCTarget, createTPCMCInstPrinter);
}
