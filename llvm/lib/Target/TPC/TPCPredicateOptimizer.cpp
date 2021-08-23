//===---- TPCPredicateOptimizer.cpp --- Optimizes predicates --------------===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//
//
// This pass:
// - replaces predicates with known value with SP0.
//
//===----------------------------------------------------------------------===//

#include "TPCInstrInfo.h"
#include "TPCSubtarget.h"
#include "llvm/CodeGen/MachineRegisterInfo.h"
#include "llvm/CodeGen/MachineInstrBuilder.h"
#include "MCTargetDesc/TPCMCTargetDesc.h"
#include "MCTargetDesc/TPCMCInstrInfo.h"
using namespace llvm;

namespace llvm {
FunctionPass *createTPCPredicateOptimizer();
void initializeTPCPredicateOptimizerPass(PassRegistry&);
}

static const char PassDescription[] = "TPC predicate optimizer";
static const char PassName[] = "tpc-pred";

// Flag to disable predicate optimization.
static cl::opt<bool>
EnablePredicateOptimizer("optimize-predicates",
                       cl::desc("Optimize use of TPC predicates (default=true)"),
                       cl::init(true), cl::Hidden);



namespace {
class TPCPredicateOptimizer : public MachineFunctionPass {
  MachineFunction *MF = nullptr;
  unsigned NumReplaced = 0;
  unsigned NumRemoved = 0;
  const TargetInstrInfo *TII = nullptr;
  const TargetRegisterInfo *TRI = nullptr;
  MachineRegisterInfo *MRI = nullptr;

public:
  static char ID;

  StringRef getPassName() const override { return PassDescription; }

  TPCPredicateOptimizer() : MachineFunctionPass(ID) {
    initializeTPCPredicateOptimizerPass(*PassRegistry::getPassRegistry());
  }

  /// \brief Loop over all of the basic blocks, replacing predicated instructions
  /// by equivalent non-predicated instructions if needed and when possible.
  ///
  bool runOnMachineFunction(MachineFunction &MF) override;

  bool isSPRFPredicated(const MachineInstr &I, unsigned &PredRegLoc,
                        unsigned &IncomeValue);
  bool isDstFullyWritten(const MachineInstr &I);
};
}

char TPCPredicateOptimizer::ID = 0;

INITIALIZE_PASS(TPCPredicateOptimizer, PassName, PassDescription, false, false)

FunctionPass *llvm::createTPCPredicateOptimizer() {
  return new TPCPredicateOptimizer();
}


bool TPCPredicateOptimizer::isSPRFPredicated(const MachineInstr &I,
                                             unsigned &PredRegLoc,
                                             unsigned &IncomeValue) {
  // Instruction is predicated if its last two arguments are of type i1, and the
  // last argument (polarity) is a constant.

  const MCInstrDesc &MCD = I.getDesc();
  if (MCD.getNumOperands() <= 2)
    return false;

  const MCOperandInfo &PredicateOp = MCD.OpInfo[MCD.getNumOperands() - 2];
  if (PredicateOp.OperandType != TPC::OperandType::OPERAND_PREDICATE)
    return false;

  const MCOperandInfo &PolarityOp = MCD.OpInfo[MCD.getNumOperands() - 1];
  if (PolarityOp.OperandType != TPC::OperandType::OPERAND_PREDICATE)
    return false;

  if (!I.getOperand(MCD.getNumOperands() - 1).isImm())
    return false;

  const MachineOperand &PredOp = I.getOperand(MCD.getNumOperands() - 2);
  if (!PredOp.isReg())
    return false;

  Register RegNo = PredOp.getReg();
  const TargetRegisterClass *RC;
  if (RegNo.isVirtual()) {
    RC = MRI->getRegClass(RegNo);
  } else {
    RC = static_cast<const TPCInstrInfo*>(TII)->getClassOfPhysicalRegister(RegNo, *TRI);
  }

  if (RC != &TPC::SPRFRegClass)
    return false;

  PredRegLoc = MCD.getNumOperands() - 2;
  IncomeValue = ~0U;

  // Instructions that do not produce values do not need income value.
  if (!I.getOperand(0).isReg() || !I.getOperand(0).isDef())
    return true;

  // Calculate income source operand. It must be of the same type as the result
  // of the instruction and be tired to the instruction result.
  unsigned NIncome;
  for (NIncome = PredRegLoc; NIncome > 0; --NIncome) {
    const MachineOperand &Op = I.getOperand(NIncome);
    if (Op.isReg()) {
      if (Op.isTied()) {
        unsigned TiredOp = I.findTiedOperandIdx(NIncome);
        if (TiredOp == 0)
          break;
      }
    }
  }
  if (NIncome == 0)
    return false;

  IncomeValue = NIncome;
  return true;
}

bool TPCPredicateOptimizer::isDstFullyWritten(const MachineInstr &I) {
  Register ireg = I.getOperand(0).getReg();
  const TargetRegisterClass *RC;
  if (ireg.isVirtual()) {
    RC = MRI->getRegClass(ireg);
  } else {
    RC = static_cast<const TPCInstrInfo*>(TII)->getClassOfPhysicalRegister(ireg, *TRI);
  }
  // Currently, we optimize only for SRF and VRF dest
  if (RC != &TPC::SRFRegClass && RC != &TPC::VRFRegClass) {
    return false;
  }
  if (TPCII::isVPUInst(I.getDesc())) {
    switch (TPCII::getSlotOpCode(I.getDesc())) {

    // Accumulators
    case TPCII::vpuMAC:
    case TPCII::vpuMSAC:
      return false;

    case TPCII::vpuCONVERT:
    {
      //  "f32", "bf16", "i32", "u32", "i8", "u8", "b", "i16", "u16", "f16"
      int isDown[12][12]{
          //  f32, bf16, i32, u32, i8, u8,   b, i16, u16, i4, u4, f16
          {0, 1, 0, 0, 1, 1, 1, 1, 1, 1, 1, 1}, // f32
          {0, 0, 0, 0, 1, 1, 1, 0, 0, 1, 1, 0}, // bf16
          {0, 1, 0, 0, 1, 1, 1, 1, 1, 1, 1, 1}, // i32
          {0, 1, 0, 0, 1, 1, 1, 1, 1, 1, 1, 1}, // u32
          {0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 0}, // i8
          {0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 0}, // u8
          {0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 0}, // b
          {0, 0, 0, 0, 1, 1, 1, 0, 0, 1, 1, 0}, // i16
          {0, 0, 0, 0, 1, 1, 1, 0, 0, 1, 1, 0}, // u16
          {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0}, // i4
          {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0}, // u4
          {0, 0, 0, 0, 1, 1, 1, 0, 0, 1, 1, 0}  // f16
      };

      // Check if it is a down-convert
      const MachineOperand &SwOp = I.getOperand(3);
      if (!SwOp.isImm()) {
        return false;
      }
      unsigned SwVal = SwOp.getImm();
      // extract 'To' data type from the switch
      SwVal = (SwVal >> 8) & 0xF;
      const MachineOperand &DTOp = I.getOperand(2);
      if (!DTOp.isImm()) {
        return false;
      }
      unsigned DTVal = DTOp.getImm();
      if (isDown[DTVal][SwVal]) {
        return false;
      }
      return true;
    }
    case TPCII::vpuCONVERT_INT16:
    case TPCII::vpuCONVERT_UINT16:
    case TPCII::vpuCONVERT_INT32:
    case TPCII::vpuCONVERT_UINT32:
    case TPCII::vpuCALC_FP_SPECIAL:
      // CALC_FP_SPECIAL leaves DST unchanged for some input values
    case TPCII::vpuPACK:
    case TPCII::vpuUNPACK:
      // UNPACK requires zero initialization of the dst in TPC kernels
      return false;

    case TPCII::vpuMOV_DUAL_GROUP:
    {
      bool hasAllSw = false;
      unsigned Sw = 0;
      if (MF->getSubtarget<TPCSubtarget>().hasGaudiISA()) {
        const MachineOperand &SwOp = I.getOperand(3);
        if (!SwOp.isImm()) {
          return false;
        }
        Sw = SwOp.getImm();
        if ((Sw & 0x01) == 0x01) { // ALL switch
          hasAllSw = true;
        }
      }
      if (!hasAllSw) {
        return false;
      }
      // Extract SrcC from Sw operand (sw{23-16} according to InstrFormat)
      unsigned SrcC = (Sw >> 16) & 0xFF;

      if (SrcC != 0xFF)
        return false;
      return true;
    }
    case TPCII::vpuMOV_GROUP:
    {
      const MachineOperand &ImmOp = I.getOperand(2);
      if (!ImmOp.isImm()) {
        return false;
      }
      unsigned Imm = ImmOp.getImm();
      if (Imm != 0xFFFFFFFF) {
        return false;
      }
      const MachineOperand &SwOp = I.getOperand(3);
      if (!SwOp.isImm()) {
        return false;
      }
      unsigned Sw = SwOp.getImm();
      if ((Sw & 0x3F) != 0x3F) {
        return false;
      }
      return true;
    }
    case TPCII::vpuSHUFFLE:
      // TODO: it is required to recursively check operands for
      //       known values - it is probably better to do this
      //       at intrinsics level, using ValueTracking feature.
      return false;
    }
  }
  else if (TPCII::isSPUInst(I.getDesc())) {
    switch (TPCII::getSlotOpCode(I.getDesc())) {
    case TPCII::spuMAC:
    case TPCII::spuCONVERT:
    case TPCII::spuCALC_FP_SPECIAL:
      // CALC_FP_SPECIAL leaves DST unchanged for some input values
      return false;
    default:;
      return true;
    }
  }
  else  if (TPCII::isLoadInst(I.getDesc())) {
    // TODO
    return false;
  }
  else  if (TPCII::isStoreInst(I.getDesc())) {
    // TODO
    return false;
  }
  return true;
}

bool TPCPredicateOptimizer::runOnMachineFunction(MachineFunction &Func) {
  if (skipFunction(Func.getFunction()))
    return false;

  if (!EnablePredicateOptimizer)
    return false;

  MF = &Func;
  TRI = MF->getSubtarget().getRegisterInfo();
  TII = MF->getSubtarget().getInstrInfo();
  MRI = &MF->getRegInfo();
  NumReplaced = NumRemoved = 0;

  for (auto &BB : Func) {
    for (auto IPtr = BB.begin(), E = BB.end(); IPtr != E;) {
      MachineInstr &I = *IPtr;
      ++IPtr;
      unsigned PredRegLoc;
      unsigned IncomeArg;
      if (isSPRFPredicated(I, PredRegLoc, IncomeArg)) {
        // Get polarity.
        bool InvertedPolarity = I.getOperand(PredRegLoc + 1).getImm() != 0;

        // Get predicate.
        const MachineOperand &PredOp = I.getOperand(PredRegLoc);
        assert(PredOp.isReg());
        unsigned PredReg = PredOp.getReg();

        // Try to evaluate predicate value.
        bool PredicateValue;
        if (PredReg == TPC::SP0) {
          PredicateValue = true;
        } else {
          // Get instruction that defines the predicate.
          MachineInstr* PredDef = MRI->getVRegDef(PredReg);
          unsigned DefiningValueNo; // N of immediate operand that defines the pred
          switch (PredDef->getOpcode()) {
          case TPC::COPY:
            DefiningValueNo = 1;
            break;
          case TPC::MOVpip:
            if (PredDef->getOperand(PredDef->getNumOperands() - 2).getReg() == TPC::SP0) {
              DefiningValueNo = 1;
              break;
            }
            LLVM_FALLTHROUGH;
          default:
            continue;
          }
          if (PredDef->getOperand(DefiningValueNo).isReg()) {
            unsigned DefReg = PredDef->getOperand(DefiningValueNo).getReg();
            if (DefReg == TPC::SP0) {
              PredicateValue = true;
            } else {
              continue;
            }
          }
          else if (!PredDef->getOperand(DefiningValueNo).isImm())
            continue;
          else
            PredicateValue = PredDef->getOperand(DefiningValueNo).getImm() != 0;
        }

        // Predicate value evaluated. Transform the instruction.
        if (PredicateValue != InvertedPolarity) {
          // Replace register with SP0
          I.getOperand(PredRegLoc).setReg(TPC::SP0);
          I.getOperand(PredRegLoc + 1).setImm(0);

          // TODO: Replace income operand with undef for scalar data but only
          // if it is not used as accumulator.
          if (I.getOperand(0).isReg() && I.getOperand(0).isDef() &&
              isDstFullyWritten(I)) {
            unsigned IncomeReg = I.getOperand(IncomeArg).getReg();
            MachineInstr* IncomeDef = MRI->getVRegDef(IncomeReg);
            if (IncomeDef->getOpcode() != TPC::IMPLICIT_DEF) {
              Register ireg = I.getOperand(0).getReg();
              const TargetRegisterClass *RC;
              if (ireg.isVirtual()) {
                RC = MRI->getRegClass(ireg);
              } else {
                RC = static_cast<const TPCInstrInfo*>(TII)->getClassOfPhysicalRegister(ireg, *TRI);
              }
              unsigned Undef = MRI->createVirtualRegister(RC);
              const DebugLoc &DL = I.getDebugLoc();
              BuildMI(*(I.getParent()), &I, DL, TII->get(TPC::IMPLICIT_DEF), Undef);
              I.getOperand(IncomeArg).setReg(Undef);
              //I.getOperand(IncomeArg).setIsUndef();
            }
          }
          ++NumReplaced;
        } else {
          // Remove instruction.
          unsigned NIn = IncomeArg;
          for (MachineOperand &D : I.defs()) {
            assert(D.isReg());
            unsigned OldReg = D.getReg();
            unsigned NewReg = I.getOperand(NIn).getReg();
            MRI->replaceRegWith(OldReg, NewReg);
            ++NIn;
          }
          I.eraseFromParent();
          ++NumRemoved;
        }
      }
    }
  }

  return NumReplaced > 0 || NumRemoved > 0;
}
