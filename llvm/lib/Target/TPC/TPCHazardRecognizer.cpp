//===- TPCHazardRecognizer.cpp ---- Custom HazardRecognizer for TPC -------===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//
//
//===----------------------------------------------------------------------===//

#include "TPCMachineScheduler.h"
#include "TPCSubtarget.h"
#include "TPCInstrInfo.h"
#include "llvm/CodeGen/MachineScheduler.h"
#include "llvm/CodeGen/ScheduleHazardRecognizer.h"
#include "llvm/CodeGen/DFAPacketizer.h"
#include "MCTargetDesc/TPCMCTargetDesc.h"
using namespace llvm;

#define DEBUG_TYPE "tpchazardrec"

namespace llvm {
// TPC Hazard Recognizer.
//
class MachineFunction;
class MachineInstr;
class ScheduleDAG;
class TPCInstrInfo;
class TPCSubtarget;

class TPCHazardRecognizer : public ScheduleHazardRecognizer {

  bool isPostRA;
  unsigned PacketNum;
  const MachineFunction &MF;
  const TPCSubtarget &ST;
  DFAPacketizer *Resources;
  const ScheduleDAG *DAG;
  std::vector<SUnit*> CurrentPacket;
  std::vector<SUnit*> PrevPacket;

public:
  TPCHazardRecognizer(const InstrItineraryData *II, const ScheduleDAG *SchedDAG, bool postRA);
  ~TPCHazardRecognizer() override {
    if (Resources)
      delete Resources;
  }
  bool atIssueLimit() const override;
  void EmitInstruction(SUnit *SU) override;
  void EmitInstruction(MachineInstr *MI) override;
  HazardType getHazardType(SUnit *SU, int Stalls) override;
  void AdvanceCycle() override;
  void RecedeCycle() override;
  void Reset() override;

protected:
  bool canReserveResources(SUnit *SU);
  void reserveResources(SUnit *SU);
  void clearResources();
  bool isLegalToPacketizeTogether(SUnit *SUI, SUnit *SUJ);
  bool hasDeadDependence(const MachineInstr &I, const MachineInstr &J);
  bool hasTPCSpecificDependence(const MachineInstr &I, const MachineInstr &J);
  void dumpCurPacket();
};
}

namespace llvm {

ScheduleHazardRecognizer *createTPCHazardRecognizer(const InstrItineraryData *II, const ScheduleDAG *DAG, bool postRA) {
  return new TPCHazardRecognizer(II, DAG, postRA);
}

TPCHazardRecognizer::TPCHazardRecognizer(const InstrItineraryData *II, const ScheduleDAG *SchedDAG, bool postRA) :
  isPostRA(postRA),
  PacketNum(0),
  MF(SchedDAG->MF),
  ST(SchedDAG->MF.getSubtarget<TPCSubtarget>()),
  Resources(SchedDAG->MF.getSubtarget<TPCSubtarget>().createDFAPacketizer(II)),
  DAG(SchedDAG)
{
  MaxLookAhead = 8;
}

void TPCHazardRecognizer::Reset() {
  LLVM_DEBUG(dbgs() << "  *HR* Reset Hazard Recognizer\n");
  clearResources();
  PacketNum = 0;
}

void TPCHazardRecognizer::clearResources() {
  if (CurrentPacket.size() > 0) {
    PrevPacket.clear();
    for (unsigned i=0; i<CurrentPacket.size(); i++) {
      PrevPacket.push_back(CurrentPacket[i]);
    }
  }
  Resources->clearResources();
  CurrentPacket.clear();
}

bool TPCHazardRecognizer::canReserveResources(SUnit *SU) {
  MachineInstr *MI = SU->getInstr();
  if (MI->isPseudo()) {
    return (CurrentPacket.size() < 2);
  }
  if (!Resources->canReserveResources(*MI)) {
    return false;
  }
  for (auto SUJ : CurrentPacket) {
    if (!isLegalToPacketizeTogether(SU, SUJ)) {
        return false;
    }
  }
  return true;
}

bool TPCHazardRecognizer::hasDeadDependence(const MachineInstr &I,
                                            const MachineInstr &J) {
  const TargetSubtargetInfo &STI = DAG->MF.getSubtarget();
  const TargetInstrInfo *HII = STI.getInstrInfo();

  if (HII->isPredicated(I) || HII->isPredicated(J))
    return false;

  BitVector DeadDefs(256);
  for (auto &MO : I.operands()) {
    if (!MO.isReg() || !MO.isDef() || !MO.isDead())
      continue;
    if (!MO.getReg().isPhysical())
      continue;
    DeadDefs[MO.getReg()] = true;
  }
  for (auto &MO : J.operands()) {
    if (!MO.isReg() || !MO.isDef() || !MO.isDead())
      continue;
    if (!MO.getReg().isPhysical())
      continue;
    unsigned R = MO.getReg();
    if (DeadDefs[R]) {
      return true;
    }
  }
  return false;
}

bool TPCHazardRecognizer::hasTPCSpecificDependence(const MachineInstr &I,
                                                   const MachineInstr &J) {
  const TPCInstrInfo * TII = DAG->MF.getSubtarget<TPCSubtarget>().getInstrInfo();

  // Immediate sharing
  bool hasImmI = (TII->instHasImm(I));
  bool hasImmJ = (TII->instHasImm(J));
  bool hasImmField = (TII->instHasImmField(I) || TII->instHasImmField(J));
  if (hasImmI && hasImmJ && !hasImmField) {
    uint64_t immI = TII->getInstImm(I);
    uint64_t immJ = TII->getInstImm(J);
    if (immI != immJ) {
      LLVM_DEBUG(dbgs() << "Imm field dependency between " << I << " and " << J << "\n");
      return true;
    }
  }

  // LD/ST predicate sharing
  unsigned pI = 0;
  unsigned pJ = 0;
  unsigned ppI = 0;
  unsigned ppJ = 0;
  bool ldstI = (TII->isLDSTInstrWithPredicate(I, pI, ppI));
  bool ldstJ = (TII->isLDSTInstrWithPredicate(J, pJ, ppJ));
  if (ldstI && ldstJ) {
    if ((pI != pJ) || (ppI != ppJ)) {
      LLVM_DEBUG(dbgs() << "Predicate dependency between " << I << " and " << J << "\n");
      return true;
    }
  }

  // 1.3.4. General Restrictions
  // CACHE FLUSH/INVALIDATE or ASO with Evict and LD_G cannot be scheduled in the same VLIW instruction
  //
  {
    bool restrict1 = false;
    bool restrict2 = false;
    bool r_restrict1 = false;
    bool r_restrict2 = false;
    if (TPCII::isLoadInst(I.getDesc()) && TPCII::getSlotOpCode(I.getDesc()) == TPCII::LD_G) {
      restrict1 = true;
    }
    else  if (TPCII::isLoadInst(J.getDesc()) && TPCII::getSlotOpCode(J.getDesc()) == TPCII::LD_G) {
      r_restrict1 = true;
    }
    if (restrict1) {
      switch (J.getOpcode()) {
        case TPC::CACHE_FLUSH:
        case TPC::CACHE_INVALIDATE:
        case TPC::ASO:
          restrict2 = true;
          break;
        default:;
      }
    }
    if (r_restrict1) {
      switch (I.getOpcode()) {
        case TPC::CACHE_FLUSH:
        case TPC::CACHE_INVALIDATE:
        case TPC::ASO:
          r_restrict2 = true;
          break;
        default:;
      }
    }

    if ((restrict1 && restrict2) || (r_restrict1 && r_restrict2)) {
      LLVM_DEBUG(dbgs() << "CACHE and LD_G dependency between " << I << " and " << J << "\n");
      return true;
    }
  }

  unsigned sopcI = TPCII::getSlotOpCode(I.getDesc());
  unsigned sopcJ = TPCII::getSlotOpCode(J.getDesc());


  // From PRM:
  // LOAD and STORE issue slots share the same resource (spill RAM), and cannot
  // access it simultaneously. On the LOAD issue slot, the LD_L* and LOOKUP*
  // instructions access the spill RAM. On the STORE issue slot, all ST_L*
  // instructions access this SRAM. The compiler should avoid scheduling both
  // the stated instruction on the LOAD issue slot and the stated insruction
  // on the STORE issue slot in the same VLIW instruction.
  if (TPCII::isLookupC(I.getDesc()) ||
      (TPCII::isLoadInst(I.getDesc()) && sopcI == TPCII::LOOKUP) ||
      (TPCII::isLoadInst(I.getDesc()) && (sopcI >= 11 && sopcI <= 16))   // LD_L*
  ) {
    if (TPCII::isStoreInst(J.getDesc()) && (sopcJ >= TPCII::ST_L && sopcJ <= TPCII::ST_L_V_HIGH)) { // ST_L, ST_G, LT_L_V*
      return true;
    }
  }
  if (TPCII::isLookupC(J.getDesc()) ||
      (TPCII::isLoadInst(J.getDesc()) && sopcJ == TPCII::LOOKUP) ||
      (TPCII::isLoadInst(J.getDesc()) && (sopcJ >= 11 && sopcJ <= 16))   // LD_L*
  ) {
    if (TPCII::isStoreInst(I.getDesc()) && (sopcI >= TPCII::ST_L && sopcI <= TPCII::ST_L_V_HIGH)) { // ST_L, ST_G, ST_L_V*
      return true;
    }
  }

  // 1.3.4. General Restrictions
  // All generations: ST_G and LD_G cannot be scheduled in the same VLIW instruction
  if (TPCII::isLoadInst(I.getDesc()) && TPCII::getSlotOpCode(I.getDesc()) == TPCII::LD_G &&
      TPCII::isStoreInst(J.getDesc()) && TPCII::getSlotOpCode(J.getDesc()) == TPCII::ST_G) {
      return true;
  }
  if (TPCII::isLoadInst(J.getDesc()) && TPCII::getSlotOpCode(J.getDesc()) == TPCII::LD_G &&
      TPCII::isStoreInst(I.getDesc()) && TPCII::getSlotOpCode(I.getDesc()) == TPCII::ST_G) {
      return true;
  }
  // All except Gen1 (Dali) ST_G and LD_G/PREFETCH cannot be scheduled in the same VLIW instruction
  if (!DAG->MF.getSubtarget<TPCSubtarget>().hasGoyaISA()) {
    if (TPCII::isLoadInst(I.getDesc()) && TPCII::getSlotOpCode(I.getDesc()) == TPCII::PREFETCH &&
        TPCII::isStoreInst(J.getDesc()) && TPCII::getSlotOpCode(J.getDesc()) == TPCII::ST_G) {
        return true;
    }
    if (TPCII::isLoadInst(J.getDesc()) && TPCII::getSlotOpCode(J.getDesc()) == TPCII::PREFETCH &&
        TPCII::isStoreInst(I.getDesc()) && TPCII::getSlotOpCode(I.getDesc()) == TPCII::ST_G) {
        return true;
    }
  }
  
  // 1.3.4. General Restrictions
  // Assertion 1: The maximum number of SRF or SPRF sources allowed
  // in 1 VLIW instruction which includes the following is 1:
  //   - MOV to V or VP
  //   - LD_L_V* (only for Dali)
  //   - VPU instruction
  //
  // Hilla Ben Yaacov wrote on 11/03/2020:
  //
  // Let me explain the HW mechanism:
  // The instructions are decoded in SPU, and then written to an Instruction-Queue for the VPU.
  // The instructions in the Instruction-Queue have a slightly different format
  // (see sheet Vector Pipe Instruction Encoding  in the ISA excel).
  //
  // In addition to the regular fields like VPU_OPCODE, LOAD_OPCODE etc.,
  // there is some meta-data coming as well.
  // You can see there the field LOAD_VPU_EMBEDDED_S.
  // This field (referred to as LD_VPU_EMBEDDED_S in other sheets of the ISA excel)
  // is used for transferring the required SRF/SPRF value to the vector pipe.
  //
  // In Goya, you can see that all 3 instructions are using the same field
  // LD_L_V, MOV from SRF/SPRF to VRF/VPRF, and VPU with SRF (you can see it on the
  // right hand side of the excel sheet).
  //
  // In Gaudi this restriction can be mitigated, because we added a separate field
  // (LD_VLM_ADDR) for LD_L_V.
  //
  // Therefore in Gaudi the restriction holds only for MOV S->V and VPU using SRF.

  bool ldlv_I = (TPCII::isLoadInst(I.getDesc()) &&
                (sopcI == TPCII::LD_L_V || sopcI == TPCII::LD_L_V_LOW || sopcI == TPCII::LD_L_V_HIGH));
  bool ldlv_J = (TPCII::isLoadInst(J.getDesc()) &&
                (sopcJ == TPCII::LD_L_V || sopcJ == TPCII::LD_L_V_LOW || sopcJ == TPCII::LD_L_V_HIGH));
  bool isIMovSToV = (TPCII::isLoadInst(I.getDesc()) &&
                    (sopcI == TPCII::ldMOV) && TII->isScalarToVector(I));
  bool isJMovSToV = (TPCII::isLoadInst(J.getDesc()) &&
                    (sopcJ == TPCII::ldMOV) && TII->isScalarToVector(J));
  if (DAG->MF.getSubtarget<TPCSubtarget>().hasGaudiISA()) {
    if (isIMovSToV || (TPCII::isVPUInst(I.getDesc()) && TII->hasSRFOrSPRFOperands(I))) {
      if (isJMovSToV || (TPCII::isVPUInst(J.getDesc()) && TII->hasSRFOrSPRFOperands(J))) {
        LLVM_DEBUG(dbgs() << "SRF/SPRF dependency between " << I << " and " << J << "\n");
        return true;
      }
    }
  } else { // Dali
    if (isIMovSToV || ldlv_I || (TPCII::isVPUInst(I.getDesc()) && TII->hasSRFOrSPRFOperands(I))) {
      if (isJMovSToV || ldlv_J || (TPCII::isVPUInst(J.getDesc()) && TII->hasSRFOrSPRFOperands(J))) {
        LLVM_DEBUG(dbgs() << "SRF/SPRF dependency between " << I << " and " << J << "\n");
        return true;
      }
    }
  }

  // It is not allowed to schedule a ST_TNSR with RMW in the same VLIW
  // with the following VPU FP8_143 operations:
  // o MAC/MUL/ADD/SUB/MADD
  // o NEARBYINT
  // o CONVERT TO/FROM FP8_143
  // o EXTRACT_EXP
  // o FORM_FP_NUMBER
  // TODO: There's no way to calculate RMW in compile time
  //       it either comes from tensor descriptor or as a value of a ST_RMW_REG.
  //       What we can do is to prohibit scheduling for all st_tnsr's
  if (DAG->MF.getSubtarget<TPCSubtarget>().hasGaudiISA()) {
    if (TPCII::isStoreInst(I.getDesc()) && (sopcI >= TPCII::ST_TNSR && sopcI <= TPCII::ST_TNSR_HIGH)) {
      if (TPCII::isVPUInst(J.getDesc())) {
        int Optype = TPCII::OpType::Invalid;
        switch(sopcJ) {
          case TPCII::vpuFORM_FP_NUM:
          case TPCII::vpuNEARBYINT:
          case TPCII::vpuEXTRACT_EXP:
          case TPCII::vpuCONVERT:
            Optype = J.getOperand(2).getImm();
            break;
          case TPCII::vpuADD:
          case TPCII::vpuSUB:
          case TPCII::vpuMAC:
          case TPCII::vpuMUL:
            Optype = J.getOperand(3).getImm();
            break;
          case TPCII::vpuMADD:
            Optype = J.getOperand(4).getImm();
            break;
        }

        if (sopcJ == TPCII::vpuCONVERT) {
          int SW = J.getOperand(2).getImm();
        }
      }
    }
    if (TPCII::isStoreInst(J.getDesc()) && (sopcJ >= TPCII::ST_TNSR && sopcJ <= TPCII::ST_TNSR_HIGH)) {
      if (TPCII::isVPUInst(I.getDesc())) {
        int Optype = TPCII::OpType::Invalid;
        switch(sopcI) {
          case TPCII::vpuFORM_FP_NUM:
          case TPCII::vpuNEARBYINT:
          case TPCII::vpuEXTRACT_EXP:
          case TPCII::vpuCONVERT:
            Optype = I.getOperand(2).getImm();
            break;
          case TPCII::vpuADD:
          case TPCII::vpuSUB:
          case TPCII::vpuMAC:
          case TPCII::vpuMUL:
            Optype = I.getOperand(3).getImm();
            break;
          case TPCII::vpuMADD:
            Optype = I.getOperand(4).getImm();
            break;
        }

        if (sopcI == TPCII::vpuCONVERT) {
          int SW = I.getOperand(2).getImm();
        }
      }
    }
  }


  // 1.3.4. General Restrictions
  // Assertion 1: If a VPU instruction accepts an SRF as input :
  //    - LD_L_V must not be scheduled in the same VLIW instruction.
  //    - MOV from SRF to V or VP must not be scheduled in LOAD slot in the same VLIW
  //      instruction.
  if (TII->isVPUInstrWithSRF(I)) {
    if (TPCII::isLoadInst(J.getDesc()) && (sopcJ == 14 || sopcJ == 15 || sopcJ == 16)) { // LD_L_V
      return true;
    }
    if (TII->isMovSRFtoVInstr(J)) {
      return true;
    }
  }
  if (TII->isVPUInstrWithSRF(J)) {
    if (TPCII::isLoadInst(I.getDesc()) && (sopcI == 14 || sopcI == 15 || sopcI == 16)) { // LD_L_V
      return true;
    }
    if (TII->isMovSRFtoVInstr(I)) {
      return true;
    }
  }

  return false;
}

bool TPCHazardRecognizer::isLegalToPacketizeTogether(SUnit *SUI, SUnit *SUJ) {
  assert(SUI->getInstr() && SUJ->getInstr());
  MachineInstr &I = *SUI->getInstr();
  MachineInstr &J = *SUJ->getInstr();

  LLVM_DEBUG(dbgs() << "Trying " << I);
  LLVM_DEBUG(dbgs() << "Trying " << J);

  if (I.getOpcode() == TPC::NOPv || J.getOpcode() == TPC::NOPv) {
    return false;
  }

  if (I.isTerminator()) {
    return false;
  }
  if (SUI == SUJ) {
    LLVM_DEBUG(dbgs() << "Failed because the slot is already occupied by" << J << "\n");
    return false;
  }

  bool Dependence = hasDeadDependence(I, J);
  if (Dependence) {
    LLVM_DEBUG(dbgs() << "Failed due to dead dependency with " << J << "\n");
    return false;
  }
  if (I.getDesc().isTerminator() && J.getDesc().isTerminator()) {
    return false;
  }

  Dependence = hasTPCSpecificDependence(I, J);
  if (Dependence) {
    return false;
  }

  if (SUJ->isSucc(SUI)) {
    for (unsigned i = 0, e = SUJ->Succs.size(); i < e; ++i) {
      const SDep &Dep = SUJ->Succs[i];
      if (Dep.getSUnit() != SUI) {
        continue;
      }
      if (Dep.getKind() == SDep::Anti) {
        continue;
      }
      if (Dep.getKind() == SDep::Output) {
        if (I.getOperand(0).getReg() != J.getOperand(0).getReg()) {
          continue;
        }
      }
      if (Dep.getKind() == SDep::Order) {
        // Ignore order dependences for now.
        continue;
      }
      if (Dep.getKind() == SDep::Data) {
        LLVM_DEBUG(dbgs() << "Failed due to DATA dependency with " << J << "\n");
        return false;
      }
    }
  }

  return true;
}

void TPCHazardRecognizer::reserveResources(SUnit *SU) {
  MachineInstr *MI = SU->getInstr();
  if (!MI->isPseudo())
    Resources->reserveResources(*MI);
  CurrentPacket.push_back(SU);
}

void TPCHazardRecognizer::dumpCurPacket() {
  for (unsigned i=0; i<CurrentPacket.size(); i++) {
    SUnit *SU = CurrentPacket[i];
    (void)SU;
    LLVM_DEBUG(dbgs() << "    SU[" << i << "]: " << *(SU->getInstr()));
  }
}

bool TPCHazardRecognizer::atIssueLimit() const {
  return (CurrentPacket.size() == 4);
}

void TPCHazardRecognizer::EmitInstruction(SUnit *SU) {
  MachineInstr *MI = SU->getInstr();

  if (MI) {
    LLVM_DEBUG(dbgs() << "  *HR* EmitInstruction(SUnit): " << *MI);
    if (canReserveResources(SU)) {
      reserveResources(SU);
      LLVM_DEBUG(dbgs() << "  *HR* Added to packet:\n");
      dumpCurPacket();
    }
    else {
      LLVM_DEBUG(dbgs() << "  *HR* ERROR: something went wrong, no resource available\n");
      dumpCurPacket();
    }
  }
  else {
    LLVM_DEBUG(dbgs() << "  *HR* Start new cycle\n");
  }
}

void TPCHazardRecognizer::EmitInstruction(MachineInstr *MI) {
  if (!MI) {
    LLVM_DEBUG(dbgs() << "  *** Start new cycle\n");
    return;
  }
  LLVM_DEBUG(dbgs() << "TPCHazardRecognizer :: EmitInstruction: " << *MI);

  if (MI->isPseudo()) {
    return;
  }

  if (Resources->canReserveResources(*MI)) {
    Resources->reserveResources(*MI);
    LLVM_DEBUG(dbgs() << "  *** Added to packet\n");
  }
  else {
    LLVM_DEBUG(dbgs() << "  *** Start new cycle\n");
  }
}

static bool isUsingSrcD(MachineInstr *MI) {
  unsigned idx = MI->getDesc().getSchedClass();

  if (idx == TPC::Sched::IIC_VectorComplexOp) {
    return true;
  }

  return false;
}

ScheduleHazardRecognizer::HazardType
TPCHazardRecognizer::getHazardType(SUnit *SU, int Stalls) {
  MachineInstr *MI = SU->getInstr();
  ScheduleHazardRecognizer::HazardType res = NoHazard;
  LLVM_DEBUG(dbgs() << "  *HR* getHazardType (" << Stalls << ") for " << *MI);

  if (!MI) {
    return NoHazard;
  }

  if (canReserveResources(SU)) {
      bool LookupPrev = false;
      for (unsigned i=0; i<PrevPacket.size(); i++) {
        SUnit *SU1 = PrevPacket[i];
        MachineInstr *PMI = SU1->getInstr();
        if (TPCII::isLookupC(PMI->getDesc())) {
          LookupPrev = true;
          break;
        }
      }
      for (unsigned i=0; i<CurrentPacket.size(); i++) {
        SUnit *SU1 = CurrentPacket[i];
        MachineInstr *PMI = SU1->getInstr();
        if (TPCII::isLookupC(PMI->getDesc())) {
          LookupPrev = true;
          break;
        }
      }
      if (LookupPrev && (!TPCII::isVPUInst(MI->getDesc()) || isUsingSrcD(MI) || TPCII::isLookupC(MI->getDesc()))) {
        res = Hazard;
        LLVM_DEBUG(dbgs() << "      Hazard - LOOKUP in prev packet\n");
        return res;
      }
      res = NoHazard;
      LLVM_DEBUG(dbgs() << "      NoHazard - can add to current packet\n");
  }
  else {
      res = Hazard;
      LLVM_DEBUG(dbgs() << "      Hazard - can not add to current packet\n");
  }

  return res;
}

void TPCHazardRecognizer::AdvanceCycle() {
  PacketNum++;
  LLVM_DEBUG(dbgs() << "  *HR* AdvanceCycle(" << PacketNum << ")\n");
  dumpCurPacket();
  clearResources();
}

void TPCHazardRecognizer::RecedeCycle() {
//  llvm_unreachable("TPC hazard recognizer does not support Bottom-Up scheduling");
  PacketNum++;
  LLVM_DEBUG(dbgs() << "  *HR* RecedeCycle(" << PacketNum << ")\n");
  dumpCurPacket();
  clearResources();
}

}

