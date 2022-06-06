//===---------------------- TPCx2CombineUtil.cpp :  -----------------------===//
//
//
//                     The LLVM Compiler Infrastructure:
//
//              2022 - This pass is a property of Habana labs
//
// Authors : Darshan Bhat (dbhat@habana.ai)
//           Vinay Vasista (vvasista@habana.ai)
//===----------------------------------------------------------------------===//
//
// This file provides util class implementation to combine Machine Instructions
// into x2 Machine Instructions.
//
//===----------------------------------------------------------------------===//

#include "TPCx2CombineUtil.h"
#include "MCTargetDesc/InstructionDB.h"
#include "MCTargetDesc/TPCMCInstrInfo.h"
#include "MCTargetDesc/TPCMCTargetDesc.h"
#include "TPCInstrInfo.h"
#include "TPCx2CombineUtil.h"
#include "llvm/CodeGen/MachineRegisterInfo.h"
#include "llvm/CodeGen/RegisterPressure.h"

using namespace llvm;

#define DEBUG_TYPE "TPCx2CombineUtil"

std::pair<MachineInstr *, MachineInstr *> TPCx2CombineUtil::X2CandidatePair = {
    nullptr, nullptr};
SmallVector<MachineInstr *> TPCx2CombineUtil::X2MIsForReorder;

const unsigned TPCx2CombineUtil::X2BlockSizeThreshold = 800;
const unsigned TPCx2CombineUtil::X2PHIThreshold = 100;
const unsigned TPCx2CombineUtil::X2RegPressureThreshold = 32;
unsigned TPCx2CombineUtil::X2RegPressure = 0;
unsigned TPCx2CombineUtil::X2RegPressureDelta = 0;
unsigned TPCx2CombineUtil::X2DRFMaxPressure = 0;
unsigned TPCx2CombineUtil::X2VRFMaxPressure = 0;

void TPCx2CombineUtil::resetMBBRegPressure(
    MachineBasicBlock *MBB, RegisterClassInfo *RegClassInfo) const {
  X2RegPressure = 0;
  RegionPressure Pressure;
  RegPressureTracker RPTracker(Pressure);
  MachineRegisterInfo &MRI = MBB->getParent()->getRegInfo();
  MachineFunction &MF = *MBB->getParent();
  const TargetRegisterInfo &TRI = *MF.getSubtarget().getRegisterInfo();
  // Initialize the register pressure tracker.
  RPTracker.init(MBB->getParent(), RegClassInfo, nullptr, MBB, MBB->end(),
                 /*TrackLaneMasks*/ false, /*TrackUntiedDefs=*/true);

  for (MachineBasicBlock::iterator MII = MBB->instr_end(),
                                   MIE = MBB->instr_begin();
       MII != MIE; --MII) {
    MachineInstr &MI = *std::prev(MII);
    if (MI.isDebugValue() || MI.isDebugLabel())
      continue;
    RegisterOperands RegOpers;
    RegOpers.collect(MI, TRI, MRI, false, false);
    RPTracker.recedeSkipDebugValues();
    assert(&*RPTracker.getPos() == &MI && "RPTracker sync error!");
    RPTracker.recede(RegOpers);
  }

  // Close the RPTracker to finalize live ins.
  RPTracker.closeRegion();

  LLVM_DEBUG(RPTracker.getPressure().dump(&TRI));
  std::vector<unsigned int> MaxRegPressureVector =
      RPTracker.getPressure().MaxSetPressure;
  unsigned int VRF_REG_ID = 26;
  unsigned int DRF_REG_ID = 25;
  for (size_t i = 0; i < MaxRegPressureVector.size(); ++i) {
    if (!MaxRegPressureVector[i])
      continue;
    if (VRF_REG_ID == i)
      X2VRFMaxPressure = MaxRegPressureVector[i];
    else if (DRF_REG_ID == i)
      X2DRFMaxPressure = MaxRegPressureVector[i];
  }
}

bool TPCx2CombineUtil::isX2Candidate(MachineInstr &I) const {
  // TODO : extend support for other Instructions and DataTypes
  return (I.getOpcode() == TPC::MACf32vvp &&
          getOpType(I) == TPCII::OpType::FP32);
}

// MacX2 related functions
bool TPCx2CombineUtil::findMACX2Pattern(
    MachineInstr &Root, SmallVectorImpl<MachineCombinerPattern> &Patterns,
    bool CheckVRegsPartOfOneDRFX2, bool CheckRegPressureX2,
    bool AllowReorderForX2) const {
  MachineBasicBlock *MBB = Root.getParent();
  auto RootIter = MachineBasicBlock::iterator(Root);
  // nothing to do
  if (RootIter == MBB->begin())
    return false;

  bool FoundPair = false;
  MachineRegisterInfo &MRI = Root.getParent()->getParent()->getRegInfo();

  // TODO : avoid redundant calculation
  unsigned PHICount = 0;
  for (auto &APhi : MBB->phis()) {
    (void)APhi;
    ++PHICount;
  }

  // not eligible for x2 combine
  if (!isX2Candidate(Root))
    return false;

  // TODO : remember available candidates and prefer earliest candidates
  // Iterate backwards to find any other pairable instruction in the same BB
  auto CurrItr = RootIter;
  CurrItr--;
  MachineInstr *CurrInst = nullptr;
  while (CurrItr != MBB->begin()) {
    CurrInst = &(*CurrItr);
    CurrItr--;

    if (!isX2Candidate(*CurrInst))
      continue;

    bool HasUser = false;
    if (!canPair(*CurrInst, Root, HasUser))
      continue;

    // TODO : skip COPYs and ST_TNSRs
    if (!hasPerformancePotential(MRI, *CurrInst, Root, HasUser,
                                 CheckVRegsPartOfOneDRFX2, CheckRegPressureX2,
                                 AllowReorderForX2))
      continue;

    FoundPair = true;
    break;
  }

  if (!FoundPair)
    return false;

  // BB Size Bailout
  if (MBB->size() > X2BlockSizeThreshold)
    return false;

  // PHI Threshold Bailout
  if (PHICount > X2PHIThreshold)
    return false;

  // Push the pattern
  Patterns.push_back(MachineCombinerPattern::TPCMACx2f32vvp);
  // Store the pointers to be processed during gen sequence stage
  X2CandidatePair = {CurrInst, &Root};
  X2RegPressure += X2RegPressureDelta;

  return true;
}

bool TPCx2CombineUtil::isVRegPartOfDRF(MachineRegisterInfo &MRI, Register Reg,
                                       Register &SourceDRF,
                                       unsigned &SubRegIndex) const {
  if (!MRI.hasOneDef(Reg))
    return false;
  auto *Def = MRI.getOneDef(Reg);
  if (!Def)
    return false;
  auto *DI = Def->getParent();
  if (DI->getOpcode() != TPC::COPY)
    return false;

  SourceDRF = DI->getOperand(1).getReg();
  if (!SourceDRF.isVirtual() || MRI.getRegClass(SourceDRF) != &TPC::DRFRegClass)
    return false;

  SubRegIndex = DI->getOperand(1).getSubReg();
  return true;
}

bool TPCx2CombineUtil::isVRegPartOfDRF(MachineRegisterInfo &MRI,
                                       Register Reg) const {
  Register DummyDRF;
  unsigned DummySubRegIndex;
  return isVRegPartOfDRF(MRI, Reg, DummyDRF, DummySubRegIndex);
}

bool TPCx2CombineUtil::areVRegsPartOfOneDRF(MachineRegisterInfo &MRI,
                                            MachineOperand Op1,
                                            MachineOperand Op2) const {
  // Check if the given virtual registers originate from two COPY instructions
  // that use the same DRF as source, with different subreg indices
  //   %436:vrf = COPY %431.sub_0:drf
  //   %437:vrf = COPY %431.sub_1:drf
  // Here %436 and %437 are part of drfs.
  Register Reg[2] = {Op1.getReg(), Op2.getReg()};
  Register SourceDRF[2];
  unsigned SubRegIndex[2];

  for (unsigned i = 0; i <= 1; i++)
    if (!isVRegPartOfDRF(MRI, Reg[i], SourceDRF[i], SubRegIndex[i]))
      return false;

  // 1. The VRegs should be part of same DRF
  // 2. The VRegs should not be the same SubReg of the DRF
  return ((SourceDRF[0] == SourceDRF[1]) && (SubRegIndex[0] != SubRegIndex[1]));
}

bool TPCx2CombineUtil::hasDependence(MachineInstr *I1, MachineInstr *I2,
                                     bool &HasUser) const {
  auto CurMBB = I1->getParent();
  MachineRegisterInfo &MRI = CurMBB->getParent()->getRegInfo();
  DenseSet<MachineInstr *> UseSet;

  // Initialize the set with users of I1
  auto addUsers = [&](MachineInstr *I) -> bool {
    Register DstReg = I->getOperand(0).getReg();
    for (auto &Use : MRI.use_instructions(DstReg)) {
      if (Use.getParent() != CurMBB || Use.isPHI())
        continue;
      if (&Use == I2)
        return true;
      UseSet.insert(&Use);
    }
    return false;
  };

  // if I2 was found to be a user of I1, exit
  if (addUsers(I1))
    return true;

  // Do the dependency checking
  auto CurIter = MachineBasicBlock::iterator(I1);
  auto LastIter = MachineBasicBlock::iterator(I2);
  ++CurIter;
  while (CurIter != LastIter) {
    if (UseSet.find(&(*CurIter)) == UseSet.end()) {
      ++CurIter;
      continue;
    }
    // if this instruction is a direct/indirect User of I1
    HasUser = true;
    // add the next indirect users (through *CurIter) of I1 to the set
    // if I2 was found to be a user of *CurIter, exit
    if (addUsers(&(*CurIter)))
      return true;
    ++CurIter;
  }

  return false;
}

// TODO : write generic compatibility checker
bool TPCx2CombineUtil::canPair(MachineInstr &I1, MachineInstr &I2,
                               bool &HasUser) const {
  // safety check
  if (&I1 == &I2)
    return false;

  int IncomeIdx1 = getIncome(I1);
  int IncomeIdx2 = getIncome(I2);
  if (IncomeIdx1 <= 0 || IncomeIdx2 <= 0 || IncomeIdx1 != IncomeIdx2)
    return false;

  int PredicateIdx1, PredicateIdx2;
  bool Polarity1, Polarity2;
  std::tie(PredicateIdx1, Polarity1) = TPCMCInstrInfo::getPredicatePolarity(I1);
  std::tie(PredicateIdx2, Polarity2) = TPCMCInstrInfo::getPredicatePolarity(I2);
  if (PredicateIdx1 <= 0 || PredicateIdx2 <= 0 ||
      PredicateIdx1 != PredicateIdx2)
    return false;

  // Switch, Predicate and Polarity has to be same
  // Incoming reg should be either both implicit, or both non-implicit
  // Skip Instructions with Polarity != 0, and different Polarities
  if ((getSwitches(I1) != getSwitches(I2)) ||
      (I1.getOperand(IncomeIdx1).isImplicit() !=
       I2.getOperand(IncomeIdx1).isImplicit()) ||
      (I1.getOperand(PredicateIdx1).getReg() !=
       I2.getOperand(PredicateIdx1).getReg()) ||
      (Polarity1 != Polarity2) || (Polarity1 != false))
    return false;

  // if I2 is dependent on I1, they cannot be combined
  return !hasDependence(&I1, &I2, HasUser);
}

bool TPCx2CombineUtil::hasPackedRegs(MachineRegisterInfo &MRI, MachineInstr &I1,
                                     MachineInstr &I2) const {
  bool IsPacked = areVRegsPartOfOneDRF(MRI, I1.getOperand(1), I2.getOperand(1));
  X2RegPressureDelta += (IsPacked ? 0 : 2);

  // TODO : extend accordingly for other MAC variants
  if (I1.getOpcode() != TPC::MACf32vvp)
    return IsPacked;

  // for MACf32vvp, consider at least Income accumulator packing
  int IncomeIdx = getIncome(I1);
  bool IsAccumPacked = areVRegsPartOfOneDRF(MRI, I1.getOperand(IncomeIdx),
                                            I2.getOperand(IncomeIdx));
  X2RegPressureDelta += (IsAccumPacked ? 0 : 2);

  return (IsPacked || IsAccumPacked);
}

bool TPCx2CombineUtil::hasSafeRegPressure() const {
  // TODO : revise register pressure heuristic
  unsigned RegPressure = (2 * X2DRFMaxPressure + X2VRFMaxPressure) +
                         X2RegPressure + X2RegPressureDelta;
  return (RegPressure <= X2RegPressureThreshold);
}

bool TPCx2CombineUtil::hasPerformancePotential(MachineRegisterInfo &MRI,
                                               MachineInstr &I1,
                                               MachineInstr &I2, bool HasUser,
                                               bool CheckVRegsPartOfOneDRFX2,
                                               bool CheckRegPressureX2,
                                               bool AllowReorderForX2) const {
  X2RegPressureDelta = 0;
  // if required, check whether the individual MIs' operands are packed in a
  // single DRF
  if (CheckVRegsPartOfOneDRFX2 && !hasPackedRegs(MRI, I1, I2))
    return false;

  // if required, check if register pressure worsens
  if (CheckRegPressureX2 && !hasSafeRegPressure())
    return false;

  // if reordering is not allowed, bailout if there is a user in between the
  // x2 candidate pair instructions
  if (!AllowReorderForX2 && HasUser)
    return false;

  return true;
}

MachineInstrBuilder
TPCx2CombineUtil::createDRFfromVRFs(MachineFunction &MF, DebugLoc Loc,
                                    Register &DRFReg, MachineOperand &VRF0,
                                    MachineOperand &VRF1) const {
  auto *TII = MF.getSubtarget().getInstrInfo();
  MachineRegisterInfo &MRI = MF.getRegInfo();

  DRFReg = MRI.createVirtualRegister(&TPC::DRFRegClass);
  auto MIB = BuildMI(MF, Loc, TII->get(TPC::REG_SEQUENCE), DRFReg)
                 .addReg(VRF0.getReg(), getKillRegState(VRF0.isKill()))
                 .addImm(TPC::sub_0)
                 .addReg(VRF1.getReg(), getKillRegState(VRF1.isKill()))
                 .addImm(TPC::sub_1);
  return MIB;
}

MachineInstrBuilder TPCx2CombineUtil::createMACx2(
    MachineFunction &MF, DebugLoc Loc, Register &ResultDRFReg, Register &DRF0,
    MachineOperand VRF1, MachineOperand VRF2, unsigned SwitchVal,
    Register &AccumDRFReg, Register &PredReg) const {
  const TargetInstrInfo *TII = MF.getSubtarget().getInstrInfo();
  MachineRegisterInfo &MRI = MF.getRegInfo();

  ResultDRFReg = MRI.createVirtualRegister(&TPC::DRFRegClass);
  auto MIB = BuildMI(MF, Loc, TII->get(TPC::MACx2f32vvvp), ResultDRFReg)
                 .addReg(DRF0, RegState::Kill)
                 .addReg(VRF1.getReg(), getKillRegState(VRF1.isKill()))
                 .addReg(VRF2.getReg(), getKillRegState(VRF2.isKill()))
                 .addImm(0)
                 .addImm(SwitchVal)
                 .addReg(AccumDRFReg, RegState::Kill)
                 .addReg(PredReg)
                 .addImm(0);
  return MIB;
}

// TODO : extend to switch between various x2 Insts
void TPCx2CombineUtil::getMACX2Sequence(
    MachineInstr &Root, SmallVectorImpl<MachineInstr *> &InsInstrs,
    SmallVectorImpl<MachineInstr *> &DelInstrs) const {
  MachineBasicBlock &MBB = *Root.getParent();
  MachineRegisterInfo &MRI = MBB.getParent()->getRegInfo();
  MachineFunction &MF = *MBB.getParent();
  const TargetInstrInfo *TII = MF.getSubtarget().getInstrInfo();
  const TargetRegisterInfo &TRI = *MF.getSubtarget().getRegisterInfo();

  // Get back the instructions stored during pattern matching.
  MachineInstr *I[2] = {X2CandidatePair.first, X2CandidatePair.second};

  // Below code will construct MIR sequence like :
  // %194:drf = REG_SEQUENCE %57:vrf, %subreg.sub_0, %59:vrf, %subreg.sub_1
  // %195:drf = REG_SEQUENCE %70:vrf, %subreg.sub_0, %71:vrf, %subreg.sub_1
  // %193:drf = MACx2vvvp killed %194:drf, %58:vrf, %60:vrf, 0, 16,
  //            %195:drf,%sprf,0
  // %168:vrf = COPY killed %193.sub_0:drf
  // %172:vrf = COPY killed %193.sub_1:drf

  SmallVector<SmallVector<MachineOperand, 2>, 8> InstOp;
  for (unsigned i = 0; i <= 1; i++) {
    InstOp.push_back(SmallVector<MachineOperand, 8>());
    for (unsigned Op = 0; Op <= 7; Op++)
      InstOp[i].push_back(I[i]->getOperand(Op));
  }
  Register InputDRFReg;
  auto MIB = createDRFfromVRFs(MF, Root.getDebugLoc(), InputDRFReg,
                               InstOp[0][1], InstOp[1][1]);
  InsInstrs.push_back(MIB);

  Register AccumDRFReg;
  if (InstOp[0][5].isImplicit() && InstOp[1][5].isImplicit()) {
    MIB = BuildMI(MF, Root.getDebugLoc(), TII->get(TPC::IMPLICIT_DEF),
                  AccumDRFReg);
  } else {
    MIB = createDRFfromVRFs(MF, Root.getDebugLoc(), AccumDRFReg, InstOp[0][5],
                            InstOp[1][5]);
  }
  InsInstrs.push_back(MIB);

  int64_t SwitchVal = InstOp[0][4].getImm() | TPCII::SW_X2_ARITHMETIC;
  Register PredReg = InstOp[0][6].getReg();
  Register ResultDRFReg;
  MIB =
      createMACx2(MF, Root.getDebugLoc(), ResultDRFReg, InputDRFReg,
                  InstOp[0][2], InstOp[1][2], SwitchVal, AccumDRFReg, PredReg);
  InsInstrs.push_back(MIB);

  MIB = BuildMI(MF, Root.getDebugLoc(), TII->get(TPC::COPY),
                InstOp[0][0].getReg())
            .addReg(ResultDRFReg, RegState::Kill, TPC::sub_0);
  InsInstrs.push_back(MIB);

  MIB = BuildMI(MF, Root.getDebugLoc(), TII->get(TPC::COPY),
                InstOp[1][0].getReg())
            .addReg(ResultDRFReg, RegState::Kill, TPC::sub_1);
  InsInstrs.push_back(MIB);

  LLVM_DEBUG(dbgs() << "Pairing instructions for macx2 : \n");
  LLVM_DEBUG(I[0]->dump());
  LLVM_DEBUG(I[1]->dump());
  DelInstrs.push_back(I[0]);
  DelInstrs.push_back(I[1]);
}