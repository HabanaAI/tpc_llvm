#include "TPC.h"
#include "TPCSubtarget.h"
#include "TPCTargetMachine.h"
#include "TPCMachineScheduler.h"
#include "llvm/Support/TargetRegistry.h"
#include "llvm/CodeGen/ScheduleDAG.h"
#include "llvm/CodeGen/ScheduleDAGInstrs.h"


using namespace llvm;

#define DEBUG_TYPE "tpc-subtarget"
#define GET_SUBTARGETINFO_TARGET_DESC
#define GET_SUBTARGETINFO_CTOR
#include "TPCGenSubtargetInfo.inc"

static::cl::opt<bool> AllowADRFSpills("allow-adrf-spill",
        cl::Hidden, cl::ZeroOrMore, cl::init(false));

static cl::opt<bool> EnableSubRegLiveness("tpc-enable-subreg-liveness",
       cl::init(true), cl::Hidden);

//---------------------------------------------------------------------------//

// TODO: Other architectures do initalizeSubtargetDependencies for InstrInfo.
//       But we have only one subtarget, so probably don't need to do this

TPCSubtarget::TPCSubtarget(
    const Triple &TT,
    const std::string CPU,
    const std::string FS,
    const TargetMachine &TM)
  : TPCGenSubtargetInfo(TT, CPU, /* TuneCPU */ CPU, FS), InstrInfo(*this), TLInfo(TM, *this) {

    ParseSubtargetFeatures(CPU, /* TuneCPU */ CPU, FS);

    // Initialize scheduling itinerary for the specified CPU.
    InstrItins = getInstrItineraryForCPU(CPU);

    HasADRFSpills = AllowADRFSpills;
}

static bool isGEN_ADDR(SUnit * SU) {
  unsigned opc = SU->getInstr()->getOpcode();
  return (opc == TPC::GEN_ADDR_ld || opc == TPC::GEN_ADDR_st ||
          opc == TPC::GEN_ADDR_ldT || opc == TPC::GEN_ADDR_stT);
}

static bool isST_TNSR(SUnit * SU) {
  MachineInstr * MI = SU->getInstr();
  if (!MI) return false;
  const MCInstrDesc &MC = MI->getDesc();
  if (TPCII::isStoreInst(MC)) {
    unsigned opc = TPCII::getSlotOpCode(MC);
    if (opc == 11 || opc == 12 || opc == 13) { // TODO: use mnemonic names for opcodes
      return true;
    }
  }
  return false;
}

static bool isLD_TNSR(SUnit * SU) {
  MachineInstr * MI = SU->getInstr();
  if (!MI) return false;
  const MCInstrDesc &MC = MI->getDesc();
  if (TPCII::isLoadInst(MC)) {
    unsigned opc = TPCII::getSlotOpCode(MC);
    if (opc == 17 || opc == 18 || opc == 19) { // TODO: use mnemonic names for opcodes
      return true;
    }
  }
  return false;
}

static bool isMMIO_ST(SUnit * SU) {
  assert(SU->getInstr() && "Must be a instruction");
  const MCInstrDesc &MC = SU->getInstr()->getDesc();

  if (!TPCII::isStoreInst(MC) || (TPCII::getSlotOpCode(MC) != TPCII::ST_L)) {
    return false;
  }

  int mmio = 0;
  if (SU->getInstr()->getNumOperands() < 3) {
    return false;
  }
  if (SU->getInstr()->getOperand(2).isImm()) {
    mmio = SU->getInstr()->getOperand(2).getImm();
  }
  return mmio;
}

#define ROUND_CSR 0x8FC
#define CONV_ROUND_CSR 0x7F8

static bool MaybeWriteToCSR(SUnit * SU, unsigned CSR) {
  if (!isMMIO_ST(SU)) {
    return false;
  }
  if (SU->getInstr()->getOperand(0).isImm()) {
    unsigned imm = SU->getInstr()->getOperand(0).getImm();
    if (imm != CSR) {
      return false;
    }
  }
  // Conservatively assume writing to specified MMIO.
  return true;
}

static bool IsRoundCSRConsumer(SUnit * SU) {
  const MachineInstr *MI = SU->getInstr();
  assert(MI && "Must be a instruction");
  unsigned SlotOpcode = TPCII::getSlotOpCode(MI->getDesc());

  if ((TPCII::isSPUInst(MI->getDesc()) &&
      (SlotOpcode == TPCII::spuADD ||
       SlotOpcode == TPCII::spuMAC ||
       SlotOpcode == TPCII::spuMUL ||
       SlotOpcode == TPCII::spuSUB)) ||
      (TPCII::isVPUInst(MI->getDesc()) &&
       (SlotOpcode == TPCII::vpuADD ||
        SlotOpcode == TPCII::vpuMAC ||
        SlotOpcode == TPCII::vpuMADD ||
        SlotOpcode == TPCII::vpuMUL ||
        SlotOpcode == TPCII::vpuSUB))) {
    TPCII::OpType Type = getOpType(*MI);
    switch (Type) {
    case TPCII::OpType::FP32:
    case TPCII::OpType::FP16:
    case TPCII::OpType::BF16:
    case TPCII::OpType::FP8_143:
    case TPCII::OpType::FP8_152:
      return true;
    default:
      return false;
    }
  }

  return false;
}

static bool IsConvertRoundCSRConsumer(SUnit * SU) {
  assert(SU->getInstr() && "Must be a instruction");
  const MCInstrDesc &Desc = SU->getInstr()->getDesc();
  unsigned SlotOpcode = TPCII::getSlotOpCode(Desc);

  if ((TPCII::isSPUInst(Desc) &&
       (SlotOpcode == TPCII::spuCONVERT ||
        SlotOpcode == TPCII::spuNEARBYINT)) ||
      (TPCII::isVPUInst(Desc) &&
       (SlotOpcode == TPCII::vpuCONVERT ||
        SlotOpcode == TPCII::vpuNEARBYINT))) {
    unsigned Switch = getSwitches(*SU->getInstr());
    unsigned RoundMode = Switch & TPCII::SW_GROUP_RM;
    return RoundMode == TPCII::SW_CSR;
  }

  return false;
}

#ifndef NDEBUG
static void dumpSU(ScheduleDAGInstrs *DAG, SUnit * SU) {
  if (SU->getInstr() != nullptr)
    DAG->dumpNode(*SU);
  else
    DAG->dumpNodeName(*SU);
}
#endif

static bool hasDataDep(SUnit * PSU, SUnit * SU) {
  for (auto &D : SU->Preds) {
    if (D.getKind() == SDep::Data && D.getSUnit() == PSU) {
      return true;
    }
  }
  return false;
}

#if 0
static bool canRemoveOrderDep(SUnit * SU1, SUnit * SU2) {
  // There should be no order dependences between LD_TNSR and ST_TNSR instructions
  // 
  if ((isLD_TNSR(SU1) && isLD_TNSR(SU2)) ||
      (isLD_TNSR(SU1) && isST_TNSR(SU2)) ||
      (isLD_TNSR(SU2) && isST_TNSR(SU1))
  ) {
    return true;
  }

  // There should be no order dependences between GEN_ADDR and other instructions.
  // 
  if ((isGEN_ADDR(SU1) && isGEN_ADDR(SU2))) {
    return true;
  }

  return false;
}
#endif

static void propagateDataDep(ScheduleDAGInstrs *DAG, SUnit * SU, SUnit * PSU, SmallVector<SDep, 4> * NewDeps) {
    for (auto &D : SU->Succs) {
      if (D.getKind() == SDep::Data && !hasDataDep(PSU, D.getSUnit())) {
        SDep newDep(PSU, SDep::Barrier);
        D.getSUnit()->addPred(newDep);
        LLVM_DEBUG(dbgs() << " - Add Data dep:\n");
        LLVM_DEBUG(dumpSU(DAG, PSU));
        LLVM_DEBUG(dumpSU(DAG, D.getSUnit()));
      }
    }
    for (auto &D : PSU->Preds) {
      if (D.getKind() == SDep::Data && !hasDataDep(D.getSUnit(), SU)) {
        SDep newDep(D.getSUnit(), SDep::Barrier);
        NewDeps->push_back(newDep);
        LLVM_DEBUG(dbgs() << " - Add Data dep:\n");
        LLVM_DEBUG(dumpSU(DAG, D.getSUnit()));
        LLVM_DEBUG(dumpSU(DAG, SU));
      }
    }
}

static bool isFloatData(TPCII::OpType X) {
  switch (X) {
  case TPCII::OpType::BF16:
  case TPCII::OpType::FP16:
  case TPCII::OpType::FP32:
  // Gaudi2 opcodes:
  case TPCII::OpType::FP8_152:
  case TPCII::OpType::FP8_143:
    return true;
  default:
    return false;
  }
}

void TPCSubtarget::TPCDAGMutation::apply(ScheduleDAGInstrs *DAG) {
  const TPCSubtarget &Subtarget = DAG->MF.getSubtarget<TPCSubtarget>();
  LLVM_DEBUG(dbgs() << "*** Applying TPC DAG Mutation\n");
  const TPCInstrInfo *TII = Subtarget.getInstrInfo();
  unsigned DefMsk = 0;
  unsigned UseMsk = 0;

  for (auto &SU : DAG->SUnits) {
    if (!SU.isInstr())
      continue;

    //LLVM_DEBUG(dumpSU(DAG, &SU));

    SmallVector<SDep, 4> NewDeps;
    NewDeps.clear();

    SmallVector<SDep, 4> Erase;

    //
    // Process predecessor dependences
    //
    for (auto &D : SU.Preds) {
      if (D.getKind() == SDep::Data &&
          isSET_INDX(SU.getInstr()->getOpcode()) &&
          isSET_INDX(D.getSUnit()->getInstr()->getOpcode()))
      {
        if (SU.getInstr()->getOperand(0).getReg() == D.getSUnit()->getInstr()->getOperand(1).getReg()) {
          LLVM_DEBUG(dbgs() << "- Removing Data Dep between SET_INDX:\n");
          LLVM_DEBUG(dumpSU(DAG, D.getSUnit()));
          LLVM_DEBUG(dumpSU(DAG, &SU));
          propagateDataDep(DAG, &SU, D.getSUnit(), &NewDeps);
          Erase.push_back(D);
        }
      }

      if (D.getKind() == SDep::Output &&
          TII->isIRFProducerWithDimMask(*(SU.getInstr()), DefMsk) &&
          TII->isIRFProducerWithDimMask(*(D.getSUnit()->getInstr()), UseMsk)) {
        if (0 == (DefMsk & UseMsk)) {
          LLVM_DEBUG(dbgs() << "- Set zero latency for IRF writing different lanes: \n");
          LLVM_DEBUG(dumpSU(DAG, D.getSUnit()));
          LLVM_DEBUG(dumpSU(DAG, &SU));
          D.setLatency(0);
          SU.setHeightDirty();

          // Change the dependence in the opposite direction too.
          for (SDep &SI : D.getSUnit()->Succs) {
            if (SI.getSUnit() != &SU || SI.getKind() != SDep::Output)
              continue;
            SI.setLatency(0);
            D.getSUnit()->setDepthDirty();
          }
        }
      }

      // ** MOV_DUAL_GROUP **
      unsigned DefDstG = 0;
      unsigned UseDstG = 0;
      unsigned DefSrcG = 0;
      unsigned UseSrcG = 0;
      if ((D.getKind() == SDep::Output) &&
           TII->isMovDualGroup(*(SU.getInstr()), &DefSrcG, &DefDstG) &&
           TII->isMovDualGroup(*(D.getSUnit()->getInstr()), &UseSrcG, &UseDstG) &&
           SU.getInstr()->getOperand(0).isIdenticalTo(D.getSUnit()->getInstr()->getOperand(0)))
      {
        if (DefDstG != UseDstG) {
          LLVM_DEBUG(dbgs() << "- Set zero latency for MOV_DUAL_GROUP writing different lanes: \n");
          LLVM_DEBUG(dumpSU(DAG, D.getSUnit()));
          LLVM_DEBUG(dumpSU(DAG, &SU));
            D.setLatency(0);
            SU.setHeightDirty();

            // Change the dependence in the opposite direction too.
            for (SDep &SI : D.getSUnit()->Succs) {
              if (SI.getSUnit() != &SU || SI.getKind() != SDep::Output)
                continue;
              SI.setLatency(0);
              D.getSUnit()->setDepthDirty();
            }
        }
      }

      // Set latency to at least two for output dependences, due to the following
      // restrictions. Restrictions are covered in TPCLatencyResolver.cpp, however,
      // setting latency here helps scheduling more efficiently.
      // 
      // Gaudi (Gen2): The following cycle after MAC/MUL BF16/FP32, it is not allowed
      //               to schedule an instruction which writes to the same destination
      //               (or destinations) as the MAC/MUL
      // Gaudisb     : The following cycle after MAC/MUL BF16/FP32/FP16 or ADD/SUB FP16,
      //               it is not allowed to schedule an instruction which writes to the same
      //               destination(or destinations) as the MAC/MUL/ADD/SUB.
      // Goya2 (Gen3), Gaudi2 (Gen4): The following cycle after MAC/MUL BF16/FP32/FP16
      //               or ADD/SUB FP16, it is not allowed to schedule an instruction
      //               which writes to the same destination (or destinations) as
      //               the MAC/MUL/ADD/SUB.
      // Gaudi2 (Gen4): The following cycle after MAC/MUL/MADD BF16/FP32/FP16/FP8_152/FP8_143
      //               or ADD/SUB FP16/FP8_152/FP8_143 or ADD/SUB FP32 with X2, it is not allowed
      //               to schedule an instruction which writes to the same destination
      //               (or destinations) as the MAC/MUL/ADD/SUB/MADD.
      //
      if (D.getKind() == SDep::Output) {
        if (!Subtarget.hasGoyaISA()) {
          const MachineInstr* DefMI = D.getSUnit()->getInstr();
          const MCInstrDesc &DefMCID = DefMI->getDesc();
          unsigned opcDef = TPCII::getSlotOpCode(DefMCID);
          bool isDefRestrict = false;
          if ((TPCII::isVPUInst(DefMCID) && (opcDef == TPCII::vpuMAC || opcDef == TPCII::vpuMUL)) ||
              (TPCII::isSPUInst(DefMCID) && (opcDef == TPCII::spuMAC || opcDef == TPCII::spuMUL))) {
            isDefRestrict = isFloatData(getOpType(*DefMI));
          }
          if (Subtarget.hasGaudi2ISA()) {
            if (TPCII::isVPUInst(DefMCID) && (opcDef == TPCII::vpuMADD)) {
              isDefRestrict = isFloatData(getOpType(*DefMI));
            }
          }
          if (Subtarget.hasGrecoISA() ||
              Subtarget.hasGaudi2ISA() ||
              Subtarget.hasGaudiBISA()) {
            if ((TPCII::isVPUInst(DefMCID) && (opcDef == TPCII::vpuADD || opcDef == TPCII::vpuSUB)) ||
                (TPCII::isSPUInst(DefMCID) && (opcDef == TPCII::spuADD || opcDef == TPCII::spuSUB))) {
              isDefRestrict = (getOpType(*DefMI) == TPCII::OpType::FP16);
              if (!isDefRestrict && Subtarget.hasGaudi2ISA()) {
                if (getOpType(*DefMI) == TPCII::OpType::FP16) {
                  isDefRestrict = true;
                }
                if (!isDefRestrict &&
                    ((getOpType(*DefMI) == TPCII::OpType::FP32) && TPCII::isVPUInst(DefMCID)))
                {
                  // Check the switch operand for 'X2'
                  if ((DefMI->getNumOperands() > 5) && DefMI->getOperand(5).isImm()) { // Switches
                    int64_t imm = DefMI->getOperand(5).getImm();
                    int64_t X2_sw = (imm >> 4) & 1;
                    isDefRestrict = (X2_sw != 0);
                  }
                }
              }
            }
          }
          if (isDefRestrict) {
            // It is not necessary to check if the "use" instr writes
            // to the same destination as "def", because we have Output dependency,
            // which implies writing to the same destination.
            unsigned alatency = std::max(D.getLatency(), (unsigned)2);
            LLVM_DEBUG(dbgs() << "- Set latency = " << alatency << " for : \n");
            LLVM_DEBUG(dumpSU(DAG, D.getSUnit()));
            LLVM_DEBUG(dumpSU(DAG, &SU));
            D.setLatency(alatency);
            SU.setHeightDirty();

            // Change the dependence in the opposite direction too.
            for (SDep &SI : D.getSUnit()->Succs) {
              if (SI.getSUnit() != &SU || SI.getKind() != SDep::Output)
                continue;
              SI.setLatency(alatency);
              D.getSUnit()->setDepthDirty();
            }
          }
        }
      }

#if 0
      //
      // We do not care about order deps between LD_TNSR/ST_TNSR - these instrs can be safely re-ordered.
      // The only deps we should care about is dependences between LD_TNSR/ST_TNSR and *real* LD/ST
      // as the latter may change the tensor (this is done in the code below, which sets up dependences
      // between MMIO changes and tensor access instructions).
      // TODO: remove the 'if' code below when intrinsics are re-generated so that LD_TNSR/ST_TNSR/GEN_ADDR
      // are not marked 'mayLoad'.
      //
      if (D.getKind() == SDep::Order) {
        if (canRemoveOrderDep(&SU, D.getSUnit())) {
          LLVM_DEBUG(dbgs() << "- Removing Order Dep:\n");
          LLVM_DEBUG(dumpSU(DAG, D.getSUnit()));
          LLVM_DEBUG(dumpSU(DAG, &SU));
          Erase.push_back(D);
        }
      }
#endif
    }

    for (SDep &E : Erase) {
      SU.removePred(E);
    }

    for (auto &D : NewDeps) {
      SU.addPred(D);
    }
  }

  //
  // Set up dependences between MMIO changes and tensor access instructions
  //
  SmallVector<SUnit*, 4> StMMIO;
  SmallVector<SUnit*, 4> TnsrUse;
  for (auto &SU : DAG->SUnits) {
    if (!SU.isInstr())
      continue;

    if (isMMIO_ST(&SU)) {
      StMMIO.push_back(&SU);
    }
    if (isGEN_ADDR(&SU) || isST_TNSR(&SU) || isLD_TNSR(&SU)) {
      TnsrUse.push_back(&SU);
    }
  }
  for (SUnit *MSU : StMMIO) {
    for (SUnit *TSU : TnsrUse) {
      SUnit *pred;
      SUnit *succ;
      if (MSU->NodeNum > TSU->NodeNum) {
        pred = TSU;
        succ = MSU;
      }
      else {
        pred = MSU;
        succ = TSU;
      }
      SDep newDep(pred, SDep::Barrier);
      newDep.setLatency(0);
      succ->addPred(newDep);
      succ->setHeightDirty();
      LLVM_DEBUG(dbgs() << "- Add MMIO barrier:\n");
      LLVM_DEBUG(dumpSU(DAG, pred));
      LLVM_DEBUG(dumpSU(DAG, succ));
    }
  }

  //
  // Set up dependences between MMIO changes and CONV_ROUND_CSR
  //
  SmallVector<SUnit *, 4> RoundCSRDef;
  SmallVector<SUnit *, 4> ConvRoundCSRDef;

  SmallVector<SUnit*, 4> RoundCSRUse;
  SmallVector<SUnit*, 4> ConvRoundCSRUse;

  for (auto &SU : DAG->SUnits) {
    if (!SU.isInstr())
      continue;

    if (MaybeWriteToCSR(&SU, Subtarget.getRoundCSRAddr()))
      RoundCSRDef.push_back(&SU);
    if (MaybeWriteToCSR(&SU, Subtarget.getConvRoundCSRAddr()))
      ConvRoundCSRDef.push_back(&SU);

    if (IsRoundCSRConsumer(&SU))
      RoundCSRUse.push_back(&SU);
    if (IsConvertRoundCSRConsumer(&SU))
      ConvRoundCSRUse.push_back(&SU);
  }

  for (SUnit *MSU : RoundCSRDef) {
    for (SUnit *TSU : RoundCSRUse) {
      SUnit *pred;
      SUnit *succ;
      if (MSU->NodeNum > TSU->NodeNum) {
        pred = TSU;
        succ = MSU;
      }
      else {
        pred = MSU;
        succ = TSU;
      }
      SDep newDep(pred, SDep::Barrier);
      newDep.setLatency(0);
      succ->addPred(newDep);
      succ->setHeightDirty();
      LLVM_DEBUG(dbgs() << "- Add ROUND_CSR barrier:\n");
      LLVM_DEBUG(dumpSU(DAG, pred));
      LLVM_DEBUG(dumpSU(DAG, succ));
    }
  }
  for (SUnit *MSU : ConvRoundCSRDef) {
    for (SUnit *TSU : ConvRoundCSRUse) {
      SUnit *pred;
      SUnit *succ;
      if (MSU->NodeNum > TSU->NodeNum) {
        pred = TSU;
        succ = MSU;
      }
      else {
        pred = MSU;
        succ = TSU;
      }
      SDep newDep(pred, SDep::Barrier);
      newDep.setLatency(0);
      succ->addPred(newDep);
      succ->setHeightDirty();
      LLVM_DEBUG(dbgs() << "- Add CONV_ROUND_CSR barrier:\n");
      LLVM_DEBUG(dumpSU(DAG, pred));
      LLVM_DEBUG(dumpSU(DAG, succ));
    }
  }
}

bool TPCSubtarget::enableSubRegLiveness() const {
  return EnableSubRegLiveness;
}

unsigned TPCSubtarget::getRoundCSRAddr() const {
  if (hasGoyaISA())
    return 0x7FC;
  else if (hasGaudiISA())
    return 0x8FC;
  else if (hasGaudiBISA())
    return 0x8FC;
  else if (hasGrecoISA())
    return 0x8FC;
  else if (hasGaudi2ISA())
    return 0xD68;
  else if (hasDoron1ISA())
    return 0xD68;
  else
    llvm_unreachable("Unknown arch for getRoundCSRAddr");
}

unsigned TPCSubtarget::getConvRoundCSRAddr() const {
  if (hasGoyaISA())
    return 0x7FC;
  else if (hasGaudiISA())
    return 0x8FC;
  else if (hasGaudiBISA())
    return 0x8FC;
  else if (hasGrecoISA())
    return 0xCA8;
  else if (hasGaudi2ISA())
    return 0xCA8;
  else if (hasDoron1ISA())
    return 0xCA8;
  else
    llvm_unreachable("Unknown arch for GetConvRoundCSR");
}

Register TPCSubtarget::getHWLoopStartReg() const {
  return hasDoron1() ? TPC::S12 : TPC::S32;
}

Register TPCSubtarget::getHWLoopFinalReg() const {
  return hasDoron1() ? TPC::S15 : TPC::S35;
}

SmallVector<Register, 4> TPCSubtarget::getHWLoopRegs() const {
  return hasDoron1() ? SmallVector<Register, 4>(
                           {TPC::S12, TPC::S13, TPC::S14, TPC::S15})
                       : SmallVector<Register, 4>(
                           {TPC::S32, TPC::S33, TPC::S34, TPC::S35});
}

bool TPCSubtarget::isHWLoopReg(const Register Reg) const {
  const unsigned StartId = getHWLoopStartReg().id();
  const unsigned FinalId = getHWLoopFinalReg().id();
  return StartId <= Reg.id() && Reg.id() <= FinalId;
}
