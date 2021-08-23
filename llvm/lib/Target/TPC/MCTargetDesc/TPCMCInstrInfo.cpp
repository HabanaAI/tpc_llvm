//===--------TPCMCInstrInfo.cpp---------------------------------------------===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//
//
//
//===----------------------------------------------------------------------===//
#include "TPCMCInstrInfo.h"

#include "TPC.h"
//#include "TPCBaseInfo.h"
//#include "TPCMCChecker.h"
#include "llvm/CodeGen/MachineInstr.h"
#include "llvm/MC/MCContext.h"
#include "llvm/MC/MCExpr.h"
#include "llvm/MC/MCInstrInfo.h"
#include "llvm/MC/MCSubtargetInfo.h"
#include "llvm/Support/CommandLine.h"

namespace llvm {

static cl::opt<bool> DisableShortImm("tpc-disable-short-imm", cl::Hidden,
    cl::ZeroOrMore, cl::init(false),
    cl::desc("Disable new encoding for short immediate"));

static cl::opt<bool> NegativeShortImm("tpc-negative-short-imm", cl::Hidden,
  cl::ZeroOrMore, cl::init(false),
  cl::desc("Enable new encoding for negative short immediate"));

static cl::opt<bool> ExtraShortImm("tpc-extra-short-imm", cl::Hidden,
  cl::ZeroOrMore, cl::init(false),
  cl::desc("Use 3 bit encoding for short immediate (i.e. allowed range is 0..7 regardless of signedness)"));

MCInstrDesc const &TPCMCInstrInfo::getDesc(MCInstrInfo const &MCII,
                                           MCInst const &MCI) {
  return (MCII.get(MCI.getOpcode()));
}

StringRef const TPCMCInstrInfo::getName(MCInstrInfo const &MCII,
                                        MCInst const &MCI) {
  return MCII.getName(MCI.getOpcode());
}

TPCII::IType TPCMCInstrInfo::getType(MCInstrInfo const &MCII,
                                     MCInst const &MCI) {
  return static_cast<TPCII::IType>(TPCII::getInstrType(getDesc(MCII, MCI)));
}

bool TPCMCInstrInfo::hasImm(MCInstrInfo const &MCII,
                            MCInst const &MCI) {
  return TPCII::getHasImm(getDesc(MCII, MCI));
}

bool TPCMCInstrInfo::hasCompositeImm(MCInstrInfo const &MCII,
                            MCInst const &MCI) {
  return TPCII::getHasCompositeImm(getDesc(MCII, MCI));
}

bool TPCMCInstrInfo::isPredicated(MCInstrInfo const &MCII,
                                  MCInst const &MCI) {
  return TPCII::getIsPredicated(getDesc(MCII, MCI));
}

bool TPCMCInstrInfo::hasOutOfSlotData(MCInstrInfo const &MCII,
                                      MCInst const &MCI) {
  return TPCII::getHasOutOfSlotData(getDesc(MCII, MCI));
}

bool TPCMCInstrInfo::hasImmField(MCInstrInfo const &MCII,
                                 MCInst const &MCI) {
  return TPCII::getHasImmField(getDesc(MCII, MCI));
}

unsigned TPCMCInstrInfo::getImmFieldStart(MCInstrInfo const &MCII,
                                          MCInst const &MCI) {
  return TPCII::getImmStart(getDesc(MCII, MCI));
}

unsigned TPCMCInstrInfo::getImmFieldEnd(MCInstrInfo const &MCII,
                                MCInst const &MCI) {
  return TPCII::getImmEnd(getDesc(MCII, MCI));
}

unsigned TPCMCInstrInfo::getImmFieldOpNum(MCInstrInfo const &MCII,
                                MCInst const &MCI) {
  return TPCII::getImmFieldOpNum(getDesc(MCII, MCI));
}

unsigned TPCMCInstrInfo::getImmOpCount(MCInstrInfo const &MCII,
                                       MCInst const &MCI) {
  return TPCII::getImmOpCount(getDesc(MCII, MCI));
}

unsigned TPCMCInstrInfo::getSecondImmOp(MCInstrInfo const &MCII,
                                        MCInst const &MCI) {
  return TPCII::getSecondImmOp(getDesc(MCII, MCI));
}

unsigned TPCMCInstrInfo::getThirdImmOp(MCInstrInfo const &MCII,
                                       MCInst const &MCI) {
  return TPCII::getThirdImmOp(getDesc(MCII, MCI));
}

unsigned TPCMCInstrInfo::getFirstImmBits(MCInstrInfo const &MCII,
                                         MCInst const &MCI) {
  return TPCII::getFirstImmBits(getDesc(MCII, MCI));
}

unsigned TPCMCInstrInfo::getSecondImmBits(MCInstrInfo const &MCII,
                                          MCInst const &MCI) {
  return TPCII::getSecondImmBits(getDesc(MCII, MCI));
}

unsigned TPCMCInstrInfo::getThirdImmBits(MCInstrInfo const &MCII,
                                         MCInst const &MCI) {
  return TPCII::getThirdImmBits(getDesc(MCII, MCI));
}

TPCII::OpType TPCMCInstrInfo::getOpType(const MCInstrDesc &Desc, const MCInst &Inst) {
  for (unsigned i = 0; i < Inst.getNumOperands(); ++i) {
    const MCOperandInfo &Info = Desc.OpInfo[i];
    if (Info.OperandType == TPC::OPERAND_DATATYPE) {
      auto Op = Inst.getOperand(i);
      return (TPCII::OpType)Op.getImm();
    }
  }
  return TPCII::OpType::INT32; // ? is it correct ?
}

TPCII::OpType TPCMCInstrInfo::getOpType(const MCInstrDesc &Desc, const MachineInstr &Inst) {
  for (unsigned i = 0; i < Inst.getNumOperands(); ++i) {
    const MCOperandInfo &Info = Desc.OpInfo[i];
    if (Info.OperandType == TPC::OPERAND_DATATYPE) {
      auto Op = Inst.getOperand(i);
      return (TPCII::OpType)Op.getImm();
    }
  }
  return TPCII::OpType::INT32; // ? is it correct ?
}

static bool checkDTSpecial(const MCInstrDesc &Desc, bool *matched) {
  //
  // varlax: Operations on Load/Store slots do not have
  //   datatype encoding (except MOV) - what DT is assumed for them?
  // Hilla Ben Yaacov <hbyaacov@habana.ai>: On load/slot we always assume unsigned,
  //   unless it is MOV or SET_INDX and then we check the explicit DT to decide.
  //   For load/store:
  //     Signed only if opcode is MOV/SET_INDX and DT indicates signed data-type
  //     (INT32/INT16/INT8/INT4). For all other opcodes - unsigned
  //   For spu/vpu:
  //     IRF destination (SPU only) opType is always INT32
  //     MOV_IRF_DIM/JMPA/JMPR/LOOP (SPU only) opType is always INT32
  //     CONVERT_INT*/CONVERT_UINT* (SPU/VPU) opType is determined by opcode name
  //     (e.g. UINT8 for CONVERT_UINT8), but SrcB (shift value) will always
  //     be treated as signed.
  //     shift values always unsigned for SHL/SHR
  //
  *matched = true;
  unsigned opc = TPCII::getSlotOpCode(Desc);
  if (TPCII::isLoadInst(Desc) && (opc != TPCII::ldMOV) && (opc != TPCII::ldSET_INDX)) {
    return false; // unsigned
  }
  if (TPCII::isStoreInst(Desc) && (opc != TPCII::stSET_INDX)) {
    return false; // unsigned
  }
  if (TPCII::isSPUInst(Desc) && (opc == TPCII::spuCONVERT_UINT32 ||
    opc == TPCII::spuCONVERT_UINT16 || opc == TPCII::spuCONVERT_UINT8))
    return true; // signed
  if (TPCII::isVPUInst(Desc) && (opc == TPCII::vpuCONVERT_UINT32 ||
    opc == TPCII::vpuCONVERT_UINT16 || opc == TPCII::vpuCONVERT_UINT8))
    return true; // signed
  if (TPCII::isSPUInst(Desc) && (opc == TPCII::spuSHL || opc == TPCII::spuSHR))
    return false; // unsigned
  if (TPCII::isVPUInst(Desc) && (opc == TPCII::vpuSHL || opc == TPCII::vpuSHR))
    return false; // unsigned
  *matched = false;
  return false;
}

bool TPCMCInstrInfo::isInstTypeSigned(const MCInstrDesc &Desc, const MachineInstr& MI) {
  // First check if intruction is treated specially
  bool isSpecial;
  bool isSigned = checkDTSpecial(Desc, &isSpecial);
  if (isSpecial)
    return isSigned;
  // Now check the DT operang of the instruction
  TPCII::OpType optype = getOpType(Desc, MI);
  isSigned = (optype == TPCII::OpType::INT32 ||
           optype == TPCII::OpType::INT16 ||
           optype == TPCII::OpType::INT8 ||
           optype == TPCII::OpType::INT4 );
  return isSigned;
}

bool TPCMCInstrInfo::isInstTypeSigned(const MCInstrDesc &Desc, const MCInst& MI) {
  // First check if intruction is treated specially
  bool isSpecial;
  bool isSigned = checkDTSpecial(Desc, &isSpecial);
  if (isSpecial)
    return isSigned;
  // Now check the DT operang of the instruction
  TPCII::OpType optype = getOpType(Desc, MI);
  isSigned = (optype == TPCII::OpType::INT32 ||
    optype == TPCII::OpType::INT16 ||
    optype == TPCII::OpType::INT8 ||
    optype == TPCII::OpType::INT4 );
  return isSigned;
}

bool TPCMCInstrInfo::useImmSlotForImm(const MCOperandInfo &Info, int64_t imm, bool isSigned) {
  if (DisableShortImm || Info.OperandType != TPC::OPERAND_TPC_IMM)
    return true;
  if (!NegativeShortImm && imm < 0)
    return true;
  if (ExtraShortImm)
    return imm > 7 || imm < 0;
  return ((imm > (isSigned ? 7 : 15)) || (imm < (isSigned ? -8 : 0)));
}

bool TPCMCInstrInfo::isVpuInstrWithSrcCD(unsigned opcode) {
  switch (opcode) {
  default:
    return false;

  // New formats for SEL*
  case TPC::SEL_EQvvvvp:
  case TPC::SEL_EQvivvp:
  case TPC::SEL_EQvvvip:
  case TPC::SEL_EQvsvvp:
  case TPC::SEL_EQvvvsp:
  case TPC::SEL_EQvvvvm:
  case TPC::SEL_EQvivvm:
  case TPC::SEL_EQvvvim:
  case TPC::SEL_EQvsvvm:
  case TPC::SEL_EQvvvsm:
  case TPC::SEL_NEQvvvvp :
  case TPC::SEL_NEQvivvp :
  case TPC::SEL_NEQvvvip :
  case TPC::SEL_NEQvsvvp :
  case TPC::SEL_NEQvvvsp :
  case TPC::SEL_NEQvvvvm :
  case TPC::SEL_NEQvivvm :
  case TPC::SEL_NEQvvvim :
  case TPC::SEL_NEQvsvvm :
  case TPC::SEL_NEQvvvsm :
  case TPC::SEL_LESSvvvvp :
  case TPC::SEL_LESSvivvp :
  case TPC::SEL_LESSvvvip :
  case TPC::SEL_LESSvsvvp :
  case TPC::SEL_LESSvvvsp :
  case TPC::SEL_LESSvvvvm :
  case TPC::SEL_LESSvivvm :
  case TPC::SEL_LESSvvvim :
  case TPC::SEL_LESSvsvvm :
  case TPC::SEL_LESSvvvsm :
  case TPC::SEL_LEQvvvvp :
  case TPC::SEL_LEQvivvp :
  case TPC::SEL_LEQvvvip :
  case TPC::SEL_LEQvsvvp :
  case TPC::SEL_LEQvvvsp :
  case TPC::SEL_LEQvvvvm :
  case TPC::SEL_LEQvivvm :
  case TPC::SEL_LEQvvvim :
  case TPC::SEL_LEQvsvvm :
  case TPC::SEL_LEQvvvsm :
  case TPC::SEL_GRTvvvvp :
  case TPC::SEL_GRTvivvp :
  case TPC::SEL_GRTvvvip :
  case TPC::SEL_GRTvsvvp :
  case TPC::SEL_GRTvvvsp :
  case TPC::SEL_GRTvvvvm :
  case TPC::SEL_GRTvivvm :
  case TPC::SEL_GRTvvvim :
  case TPC::SEL_GRTvsvvm :
  case TPC::SEL_GRTvvvsm :
  case TPC::SEL_GEQvvvvp :
  case TPC::SEL_GEQvivvp :
  case TPC::SEL_GEQvvvip :
  case TPC::SEL_GEQvsvvp :
  case TPC::SEL_GEQvvvsp :
  case TPC::SEL_GEQvvvvm :
  case TPC::SEL_GEQvivvm :
  case TPC::SEL_GEQvvvim :
  case TPC::SEL_GEQvsvvm :
  case TPC::SEL_GEQvvvsm :

  case TPC::SEL2_LESSvvvvp :
  case TPC::SEL2_LESSvivvp :
  case TPC::SEL2_LESSvvvip :
  case TPC::SEL2_LESSvsvvp :
  case TPC::SEL2_LESSvvvsp :
  case TPC::SEL2_LESSvvvvm :
  case TPC::SEL2_LESSvivvm :
  case TPC::SEL2_LESSvvvim :
  case TPC::SEL2_LESSvsvvm :
  case TPC::SEL2_LESSvvvsm :
  case TPC::SEL2_LEQvvvvp  :
  case TPC::SEL2_LEQvivvp  :
  case TPC::SEL2_LEQvvvip  :
  case TPC::SEL2_LEQvsvvp  :
  case TPC::SEL2_LEQvvvsp  :
  case TPC::SEL2_LEQvvvvm  :
  case TPC::SEL2_LEQvivvm  :
  case TPC::SEL2_LEQvvvim  :
  case TPC::SEL2_LEQvsvvm  :
  case TPC::SEL2_LEQvvvsm  :
  case TPC::SEL2_GRTvvvvp  :
  case TPC::SEL2_GRTvivvp  :
  case TPC::SEL2_GRTvvvip  :
  case TPC::SEL2_GRTvsvvp  :
  case TPC::SEL2_GRTvvvsp  :
  case TPC::SEL2_GRTvvvvm  :
  case TPC::SEL2_GRTvivvm  :
  case TPC::SEL2_GRTvvvim  :
  case TPC::SEL2_GRTvsvvm  :
  case TPC::SEL2_GRTvvvsm  :
  case TPC::SEL2_GEQvvvvp  :
  case TPC::SEL2_GEQvivvp  :
  case TPC::SEL2_GEQvvvip  :
  case TPC::SEL2_GEQvsvvp  :
  case TPC::SEL2_GEQvvvsp  :
  case TPC::SEL2_GEQvvvvm  :
  case TPC::SEL2_GEQvivvm  :
  case TPC::SEL2_GEQvvvim  :
  case TPC::SEL2_GEQvsvvm  :
  case TPC::SEL2_GEQvvvsm  :

  case TPC::FORM_FP_NUMvvvp:
  case TPC::FORM_FP_NUMvvvm:
  case TPC::FORM_FP_NUMsvvp:
  case TPC::FORM_FP_NUMsvvm:
  case TPC::FORM_FP_NUMivvp:
  case TPC::FORM_FP_NUMivvm:

  case TPC::MOV_DUAL_GROUPp:
  case TPC::MOV_DUAL_GROUPm:
  case TPC::MOV_DUAL_GROUP_ALLp:
  case TPC::MOV_DUAL_GROUP_ALLm:

  // New formats for MSAC
  case TPC::MSACvvvvp :
  case TPC::MSACvivvp :
  case TPC::MSACvvvip :
  case TPC::MSACvsvvp :
  case TPC::MSACvvvsp :
  case TPC::MSACvvvvm :
  case TPC::MSACvivvm :
  case TPC::MSACvvvim :
  case TPC::MSACvsvvm :
  case TPC::MSACvvvsm :
    return true;  
  }
}
 
//
// Bundle support
//
bool TPCMCInstrInfo::isBundle(MCInst const &MCI) {
  auto Result = (TPC::BUNDLE == MCI.getOpcode());
  return Result;
}

iterator_range<MCInst::const_iterator>
TPCMCInstrInfo::bundleInstructions(MCInst const &MCI) {
  assert(isBundle(MCI));
  return make_range(MCI.begin() + bundleInstructionsOffset, MCI.end());
}

size_t TPCMCInstrInfo::bundleSize(MCInst const &MCI) {
  if (TPCMCInstrInfo::isBundle(MCI))
    return (MCI.size() - bundleInstructionsOffset);
  else
    return (1);
}

MCInst TPCMCInstrInfo::createBundle() {
  MCInst Result;
  Result.setOpcode(TPC::BUNDLE);
  Result.addOperand(MCOperand::createImm(0));
  return Result;
}
//
// End of Bundle support
//

}
