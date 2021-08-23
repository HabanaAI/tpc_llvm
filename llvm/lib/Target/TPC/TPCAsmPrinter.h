//===-- TPCAsmPrinter.h - TPC implementation of AsmPrinter ------*- C++ -*-===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

#ifndef LLVM_LIB_TARGET_TPC_TPCASMPRINTER_H
#define LLVM_LIB_TARGET_TPC_TPCASMPRINTER_H

#include "TPCSubtarget.h"
#include "llvm/CodeGen/AsmPrinter.h"
#include "llvm/CodeGen/FaultMaps.h"
#include "llvm/CodeGen/StackMaps.h"
#include "llvm/Target/TargetMachine.h"

// Implemented in TPCMCInstLower.cpp
namespace {
  class TPCMCInstLower;
}

namespace llvm {
class MCStreamer;
class MCSymbol;

class LLVM_LIBRARY_VISIBILITY TPCAsmPrinter : public AsmPrinter {
public:
  explicit TPCAsmPrinter(TargetMachine &TM,
                         std::unique_ptr<MCStreamer> Streamer)
      : AsmPrinter(TM, std::move(Streamer)) {}

  void EmitInstruction(const MachineInstr *MI) override;
  bool PrintAsmOperand(const MachineInstr *MI, unsigned OpNo,
                       const char *ExtraCode, raw_ostream &OS) override;
  bool isBlockOnlyReachableByFallthrough(
    const MachineBasicBlock *MBB) const override;

private:
  bool isLoop(const MachineInstr& MI) const;
};

} // end namespace llvm

#endif

