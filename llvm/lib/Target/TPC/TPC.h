//===-- TPC.h - Top-level interface for TPC representation --*- C++ -*-===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
//
// This file contains the entry points for global functions defined in the LLVM
// TPC back-end.
//
//===----------------------------------------------------------------------===//

#ifndef LLVM_LIB_TARGET_TPC_TPC_H
#define LLVM_LIB_TARGET_TPC_TPC_H

#include "MCTargetDesc/TPCMCTargetDesc.h"
#include "llvm/Support/ErrorHandling.h"
#include "llvm/Target/TargetMachine.h"

namespace llvm {
  class FunctionPass;
  class TPCTargetMachine;
  class formatted_raw_ostream;
  class AsmPrinter;
  class MCInst;
  class MachineInstr;

  enum AddressSpace {
    GENERIC = 0,
    LOCAL_SCALAR = 1,
    LOCAL_VECTOR = 2,
    GLOBAL = 3
  };

  FunctionPass *createTPCISelDag(TPCTargetMachine &TM, CodeGenOpt::Level OptLevel);
//  FunctionPass *createTPCDelaySlotFillerPass(TargetMachine &TM);

//  void LowerTPCMachineInstrToMCInst(const MachineInstr *MI,
//                                      MCInst &OutMI,
//                                      AsmPrinter &AP);
} // end namespace llvm;


#endif
