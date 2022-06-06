#include "TPCMCInstrInfo.h"

#include "TPC.h"
//#include "TPCBaseInfo.h"
//#include "TPCMCChecker.h"
#include "llvm/CodeGen/MachineInstr.h"
#include "llvm/CodeGen/MachineFunction.h"
#include "llvm/MC/MCContext.h"
#include "llvm/MC/MCExpr.h"
#include "llvm/MC/MCInstrInfo.h"
#include "llvm/MC/MCSubtargetInfo.h"
#include "llvm/Support/CommandLine.h"
#include "TPCSubtarget.h"

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
  // varlax <varlax@unipro.ru>: Operations on Load/Store slots do not have
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

template<class T> std::tuple<int, bool> GetPredicatePolarity(const T &MI, const MCInstrDesc &Desc) {
  for (int I = Desc.getNumOperands() - 2; I >= 0; --I) {
    if (Desc.OpInfo[I].OperandType == TPC::OperandType::OPERAND_PREDICATE) {
      assert(MI.getOperand(I).isReg() && "Expect OPERAND_PREDICATE is reg");
      assert(MI.getOperand(I+1).isImm() && "Expect polarity here");
      return std::make_tuple(I, MI.getOperand(I+1).getImm() & 1);
    }
  }  
  return std::make_tuple(-1, false);
}

std::tuple<int, bool> TPCMCInstrInfo::getPredicatePolarity(const MachineInstr &MI) {
  return GetPredicatePolarity(MI, MI.getDesc());
}

template <class MIT>
bool hasPredicateT(const MIT &MI, const MCInstrDesc &Desc, bool isDoron1Plus) {
  int PredicateIndex;
  bool Polarity;
  std::tie(PredicateIndex, Polarity) = GetPredicatePolarity(MI, Desc);
  if (PredicateIndex == -1)
    return false;

  assert(MI.getOperand(PredicateIndex).isReg());
  Register Predicate = MI.getOperand(PredicateIndex).getReg();
  // It means polarity is 1
  if (Polarity)
    return true;

  if (Predicate == TPC::SPRF_TRUE || Predicate == TPC::VPRF_TRUE)
    return false;

  return isDoron1Plus || Predicate != TPC::SP0;
}

bool TPCMCInstrInfo::HasPredicate(const MCInst &MI, const MCInstrDesc &MD,
                                  bool isDoron1Plus) {
  return hasPredicateT(MI, MD, isDoron1Plus);
}

bool TPCMCInstrInfo::hasPredicate(const MachineInstr &MI, bool isDoron1Plus) {
  return hasPredicateT(MI, MI.getDesc(), isDoron1Plus);
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
