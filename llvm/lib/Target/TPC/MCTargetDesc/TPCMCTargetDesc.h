//===-- TPCMCTargetDesc.h - TPC Target Descriptions ---------*- C++ -*-===//
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

#ifndef LLVM_LIB_TARGET_TPC_MCTARGETDESC_TPCMCTARGETDESC_H
#define LLVM_LIB_TARGET_TPC_MCTARGETDESC_TPCMCTARGETDESC_H

#include "llvm/MC/MCObjectWriter.h"
#include "llvm/MC/MCInstrDesc.h"
#include "llvm/Support/DataTypes.h"
#include <memory>

namespace llvm {
class MCAsmBackend;
class MCCodeEmitter;
class MCContext;
class MCInstrInfo;
class MCRegisterInfo;
class MCSubtargetInfo;
class MCTargetOptions;
class Target;
class Triple;
class StringRef;
class raw_pwrite_stream;
class raw_ostream;

Target &getTheTPCTarget();

namespace TPC {
enum OperandType {
  // an immediate operand that can be encoded differently depending on its value
  OPERAND_TPC_IMM = MCOI::OPERAND_FIRST_TARGET,
  // Marks predicate part.
  OPERAND_PREDICATE,
  // Immediate operand that represents type of values involved in operation.
  OPERAND_DATATYPE,
  OPERAND_DIMMASK
};
} // end namespace TPC

MCCodeEmitter *createTPCMCCodeEmitter(const MCInstrInfo &MCII,
                                        const MCRegisterInfo &MRI,
                                        MCContext &Ctx);
MCAsmBackend *createTPCAsmBackend(const Target &T,
                                  const MCRegisterInfo &MRI,
                                  const Triple &TheTriple, StringRef CPU,
                                  const MCTargetOptions &Options);
MCAsmBackend *createTPCAsmBackend(const Target &T,
                                  const MCSubtargetInfo &STI,
                                  const MCRegisterInfo &MRI,
                                  const MCTargetOptions &Options);
std::unique_ptr<MCObjectTargetWriter> createTPCELFObjectWriter();

} // End llvm namespace

// Defines symbolic names for TPC registers.  This defines a mapping from
// register name to register number.
//
#define GET_REGINFO_ENUM
#include "TPCGenRegisterInfo.inc"

// Defines symbolic names for the TPC instructions.
//
#define GET_INSTRINFO_ENUM
#include "TPCGenInstrInfo.inc"

#define GET_SUBTARGETINFO_ENUM
#include "TPCGenSubtargetInfo.inc"

#endif
