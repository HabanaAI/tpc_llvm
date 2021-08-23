//===-- TPCInstrInfo.cpp - TPC Instruction Information ----------------===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//
//
// This file contains the TPC implementation of the TargetInstrInfo class.
//
//===----------------------------------------------------------------------===//

#include "TPCInstrInfo.h"
#include "TPCSubtarget.h"
#include "TPCMachineScheduler.h"
#include "MCTargetDesc/TPCMCTargetDesc.h"
#include "MCTargetDesc/TPCMCInstrInfo.h"
#include "llvm/ADT/STLExtras.h"
#include "llvm/ADT/SmallVector.h"
#include "llvm/CodeGen/MachineFrameInfo.h"
#include "llvm/CodeGen/MachineInstrBuilder.h"
#include "llvm/CodeGen/MachineMemOperand.h"
#include "llvm/CodeGen/MachineRegisterInfo.h"
#include "llvm/CodeGen/RegisterScavenging.h"
#include "llvm/CodeGen/MachineFrameInfo.h"
#include "llvm/Support/ErrorHandling.h"
#include "llvm/Support/TargetRegistry.h"
#include "latencies.h"

using namespace llvm;

#define DEBUG_TYPE "TPCInstrInfo"

#define GET_INSTRINFO_CTOR_DTOR
#define GET_INSTRMAP_INFO
#include "TPCGenInstrInfo.inc"
#include "TPCGenDFAPacketizer.inc"

static cl::opt<int> TPCDefaultLatency("tpc-default-latency",
    cl::Hidden, cl::ZeroOrMore, cl::init(7));

// Pin the vtable to this file.
void TPCInstrInfo::anchor() {}

// TODO: What is the register in InstrInfo constructor for?
TPCInstrInfo::TPCInstrInfo(TPCSubtarget &ST)
    : TPCGenInstrInfo(), RI(),
      Subtarget(ST) {
  TPCLatencyEvaluation::setDefaultLatency(TPCDefaultLatency);
}

/// isLoadFromStackSlot - If the specified machine instruction is a direct
/// load from a stack slot, return the virtual or physical register number of
/// the destination along with the FrameIndex of the loaded stack slot.  If
/// not, return 0.  This predicate must return 0 if the instruction has
/// any side effects other than loading from the stack slot.
unsigned TPCInstrInfo::isLoadFromStackSlot(const MachineInstr &MI,
                                           int &FrameIndex) const {
  switch (MI.getOpcode()) {
  case TPC::SPILL_ARF_RESTORE:
  case TPC::SPILL_DRF_RESTORE:
  case TPC::SPILL_VRF_RESTORE:
  case TPC::SPILL_VPRF_RESTORE:
  case TPC::SPILL_IRF_RESTORE:
  case TPC::SPILL_SRF_RESTORE:
  case TPC::SPILL_ZRF_RESTORE:
  case TPC::SPILL_SPRF_RESTORE:
    FrameIndex = MI.getOperand(1).getIndex();
    return MI.getOperand(0).getReg();
  }
  return 0;
}

/// isStoreToStackSlot - If the specified machine instruction is a direct
/// store to a stack slot, return the virtual or physical register number of
/// the source reg along with the FrameIndex of the loaded stack slot.  If
/// not, return 0.  This predicate must return 0 if the instruction has
/// any side effects other than storing to the stack slot.
unsigned TPCInstrInfo::isStoreToStackSlot(const MachineInstr &MI,
                                          int &FrameIndex) const {
  switch (MI.getOpcode()) {
  case TPC::SPILL_ARF_SAVE:
  case TPC::SPILL_DRF_SAVE:
  case TPC::SPILL_VRF_SAVE:
  case TPC::SPILL_VPRF_SAVE:
  case TPC::SPILL_IRF_SAVE:
  case TPC::SPILL_SRF_SAVE:
  case TPC::SPILL_ZRF_SAVE:
  case TPC::SPILL_SPRF_SAVE:
    FrameIndex = MI.getOperand(0).getIndex();
    return MI.getOperand(1).getReg();
  }
  return 0;
}


static void parseCondBranch(MachineInstr &LastInst, MachineBasicBlock *&Target,
                            SmallVectorImpl<MachineOperand> &Cond) {
  Target = LastInst.getOperand(0).getMBB();
  Cond.push_back(MachineOperand::CreateReg(LastInst.getOperand(1).getReg(), false));
  Cond.push_back(MachineOperand::CreateImm(LastInst.getOperand(2).getImm()));
}


static bool isUnconditionalBranch(const MachineInstr &MI) {
  if (MI.getOpcode() == TPC::JMPR_u)
    return true;
  return (MI.getOpcode() == TPC::JMPR || MI.getOpcode() == TPC::JMPA) &&
         MI.getOperand(1).getReg() == TPC::SP0 &&
         MI.getOperand(2).getImm() == 0;
}


static bool isDeadBranch(const MachineInstr &MI) {
  return (MI.getOpcode() == TPC::JMPR || MI.getOpcode() == TPC::JMPA) &&
         MI.getOperand(1).getReg() == TPC::SP0 &&
         MI.getOperand(2).getImm() != 0;
}


static bool isConditionalBranch(const MachineInstr &MI) {
  return (MI.getOpcode() == TPC::JMPR || MI.getOpcode() == TPC::JMPA) &&
         MI.getOperand(1).getReg() != TPC::SP0;
}


static MachineBasicBlock *getBranchTarget(const MachineInstr &MI) {
  assert(isUnconditionalBranch(MI) || isConditionalBranch(MI));
  return MI.getOperand(0).getMBB();
}


/// Analyze the branching code at the end of MBB, returning true if it cannot
/// be understood (e.g. it's a switch dispatch or isn't implemented for a
/// target).
///
/// Upon success, this returns false and returns with the following information
/// in various cases:
///
/// 1. If this block ends with no branches (it just falls through to its succ)
///    just return false, leaving TBB/FBB null.
/// 2. If this block ends with only an unconditional branch, it sets TBB to be
///    the destination block.
/// 3. If this block ends with an conditional branch and it falls through to
///    an successor block, it sets TBB to be the branch destination block and a
///    list of operands that evaluate the condition. These
///    operands can be passed to other TargetInstrInfo methods to create new
///    branches.
/// 4. If this block ends with an conditional branch and an unconditional
///    block, it returns the 'true' destination in TBB, the 'false' destination
///    in FBB, and a list of operands that evaluate the condition. These
///    operands can be passed to other TargetInstrInfo methods to create new
///    branches.
///
/// \param TrueBB is set to the destination if condition evaluates true (it is the
/// nullptr if the destination is the fall-through branch);
/// \param FalseBB is set to the destination if condition evaluates to false (it
/// is the nullptr if the branch is unconditional);
/// \Cond Cond is populated with machine operands needed to generate the branch
/// to insert in insertBranch;
///
/// Note that removeBranch and insertBranch must be implemented to support
/// cases where this method returns success.
///
bool TPCInstrInfo::analyzeBranch(MachineBasicBlock &MBB,
                                 MachineBasicBlock *&TrueBB,
                                 MachineBasicBlock *&FalseBB,
                                 SmallVectorImpl<MachineOperand> &Cond,
                                 bool AllowModify) const {
  TrueBB = nullptr;
  FalseBB = nullptr;
  Cond.clear();

  // Start from the bottom of the block and work up, examining the
  // terminator instructions.
  MachineInstr *LastTerminator = nullptr;
  MachineBasicBlock::iterator I = MBB.end();
  while (I != MBB.begin()) {
    --I;

    // Skip over debug values.
    if (I->isDebugValue())
      continue;

    // Working from the bottom, when we see a non-terminator
    // instruction, we're done.
    if (!isUnpredicatedTerminator(*I))
      break;

    switch (I->getOpcode()) {
    case TPC::HALTs:
    case TPC::HALTv:
      // These are barriers.
      return true;
    case TPC::LOOPiii:
    case TPC::LOOPiiip:
    case TPC::LOOPiis:
    case TPC::LOOPiisp:
    case TPC::LOOPisi:
    case TPC::LOOPisip:
    case TPC::LOOPiss:
    case TPC::LOOPissp:
    case TPC::LOOPsii:
    case TPC::LOOPsiip:
    case TPC::LOOPsis:
    case TPC::LOOPsisp:
    case TPC::LOOPssi:
    case TPC::LOOPssip:
    case TPC::LOOPsss:
    case TPC::LOOPsssp:

    case TPC::LOOP1iiip:
    case TPC::LOOP1iisp:
    case TPC::LOOP1isip:
    case TPC::LOOP1issp:
    case TPC::LOOP1siip:
    case TPC::LOOP1sisp:
    case TPC::LOOP1ssip:
    case TPC::LOOP1sssp:
      // Cannot analyse these instructions.
      return true;

    case TPC::LOOPEND:
      // Behaves like conditional branch. However we cannot form corresponding
      // conditions.
      return true;

    default:
      // A terminator that isn't a branch can't easily be handled
      // by this analysis.
      if (!I->isBranch()) {
        return true;
      }
    }

    // Handle particular case of unconditional branch with reversed polarity.
    if (isDeadBranch(*I)) {
      if (AllowModify) {
        MachineBasicBlock::iterator NewI = std::next(I);
        I->eraseFromParent();
        I = NewI;
      }
      continue;
    }

    // If we have found only one terminator, continue search.
    if (!LastTerminator) {
      LastTerminator = &*I;
      continue;
    }

    // If the first terminator is an unconditional branch, skip all subsequent
    // instructions. If AllowModify is true also delete them.
    if (isUnconditionalBranch(*I)) {
      if (AllowModify)
        LastTerminator->eraseFromParent();
      LastTerminator = &*I;
      continue;
    }

    // If the instruction before the first terminator is also a terminator,
    // (so we have three terminators), we don't know what sort of block this is,
    // unless the terminator before the first is an unconditional branch.
    MachineInstr *FirstTerminator = &*I;
    if (I != MBB.begin() && isUnpredicatedTerminator(*--I)) {
      if (isUnconditionalBranch(*I)) {
        if (AllowModify) {
          LastTerminator->eraseFromParent();
          FirstTerminator->eraseFromParent();
        }
        LastTerminator = &*I;
        continue;
      }
      return true;
    }

    // We get here if MBB is ended with two terminators, and the first one is
    // not an unconditional branch.

    // We process only conditional branch followed by unconditional, all other
    // cases cannot be analysed.
    if (!isUnconditionalBranch(*LastTerminator) ||
        !isConditionalBranch(*FirstTerminator))
      return true;

    parseCondBranch(*FirstTerminator, TrueBB, Cond);
    TrueBB = getBranchTarget(*FirstTerminator);
    FalseBB = getBranchTarget(*LastTerminator);
    return false;
  }

  if (!LastTerminator) {
    // This is fall-through block.
    assert(!TrueBB && !FalseBB);
    return false;
  }

  // Delete the branch if it's equivalent to a fall-through.
  if (MBB.isLayoutSuccessor(getBranchTarget(*LastTerminator))) {
    LastTerminator->eraseFromParent();
    assert(!TrueBB && !FalseBB);
    return false;
  }

  // Handle unconditional branches.
  if (isUnconditionalBranch(*LastTerminator)) {
    // TrueBlock is used to indicate the unconditional destination.
    TrueBB = getBranchTarget(*LastTerminator);
    return false;
  }

  // Handle conditional branches.
  if (isConditionalBranch(*LastTerminator)) {
    // Block ends with fall-through condbranch.
    parseCondBranch(*LastTerminator, TrueBB, Cond);
    return false;
  }

  return true; // Can't handle indirect branch.
}


unsigned TPCInstrInfo::insertBranch(MachineBasicBlock &MBB, MachineBasicBlock *TBB,
                                MachineBasicBlock *FBB,
                                ArrayRef<MachineOperand> Cond,
                                const DebugLoc &DL,
                                int *BytesAdded) const {
  assert(TBB && "insertBranch must not be told to insert a fallthrough");
  assert(!BytesAdded && "code size not handled");

  if (!FBB) {
    if (Cond.empty()) {
      // Due to a bug in TailMerging/CFG Optimization, we need to add a
      // special case handling of a predicated jump followed by an
      // unconditional jump. If not, Tail Merging and CFG Optimization go
      // into an infinite loop.
      MachineBasicBlock *NewTBB, *NewFBB;
      SmallVector<MachineOperand, 4> aCond;
      auto Term = MBB.getFirstTerminator();
      if (Term != MBB.end() && isPredicated(*Term) &&
          !analyzeBranch(MBB, NewTBB, NewFBB, aCond, false) &&
          MachineFunction::iterator(NewTBB) == ++MBB.getIterator()) {
        reverseBranchCondition(aCond);
        removeBranch(MBB);
        return insertBranch(MBB, TBB, nullptr, aCond, DL);
      }
      BuildMI(&MBB, DL, get(TPC::JMPR_u))
          .addMBB(TBB);
    } else {
      assert((Cond.size() == 2) && "Malformed cond vector");
      const MachineOperand &RO = Cond[0];
      unsigned Flags = getUndefRegState(RO.isUndef());
      BuildMI(&MBB, DL, get(TPC::JMPR))
          .addMBB(TBB)
          .addReg(RO.getReg(), Flags)
          .addImm(Cond[1].getImm());
    }
    return 1;
  }

  assert((!Cond.empty()) &&
         "Cond. cannot be empty when multiple branchings are required");
  const MachineOperand &RO = Cond[0];
  unsigned Flags = getUndefRegState(RO.isUndef());
  BuildMI(&MBB, DL, get(TPC::JMPR))
      .addMBB(TBB)
      .addReg(RO.getReg(), Flags)
      .addImm(Cond[1].getImm());
  BuildMI(&MBB, DL, get(TPC::JMPR_u))
      .addMBB(FBB);

  return 2;
}

unsigned TPCInstrInfo::removeBranch(MachineBasicBlock &MBB,
                                int *BytesRemoved) const {
  MachineBasicBlock::iterator I = MBB.end();
  unsigned Count = 0;
  while (I != MBB.begin()) {
    --I;
    if (I->isDebugValue())
      continue;
    // Only removing branches from end of MBB.
    if (!I->isBranch())
      return Count;
    MBB.erase(&MBB.back());
    I = MBB.end();
    ++Count;
  }
  return Count;
}

bool TPCInstrInfo::reverseBranchCondition(
                                  SmallVectorImpl<MachineOperand> &Cond) const {
  if (Cond.empty())
    return true;
  assert(Cond.size() == 2);
  assert(Cond[0].isReg() && "First entry in the cond vector must be a predicate register");
  assert(Cond[1].isImm() && "Second entry in the cond vector must be an immediate");
  Cond[1].setImm(!Cond[1].getImm());
  return false;
}

void TPCInstrInfo::copyPhysReg(MachineBasicBlock &MBB,
                               MachineBasicBlock::iterator I,
                               const DebugLoc &DL, MCRegister DestReg,
                               MCRegister SrcReg, bool KillSrc) const {
  const TargetRegisterInfo &TRI = getRegisterInfo();

  const auto CopyRegWithDataType = [&](unsigned Opc) {
    BuildMI(MBB, I, DL, get(Opc), DestReg)
      .addReg(SrcReg, getKillRegState(KillSrc))
      .addImm(TPCII::OpType::FP32)
      .addImm(0)  // Switches
      .addReg(DestReg, RegState::Undef)
      .addReg(TPC::SP0)
      .addImm(0);
  };

  const auto CopySubRegWithDataType = [&](unsigned Opc, unsigned SubReg) {
    unsigned SrcSubReg = TRI.getSubReg(SrcReg, SubReg);
    unsigned DestSubReg = TRI.getSubReg(DestReg, SubReg);
    BuildMI(MBB, I, DL, get(Opc), DestSubReg)
      .addReg(SrcSubReg, getKillRegState(KillSrc))
      .addImm(TPCII::OpType::FP32)
      .addImm(0)  // Switches
      .addReg(DestReg, RegState::Undef)
      .addReg(TPC::SP0)
      .addImm(0);
  };

  const auto CopyReg = [&](unsigned Opc) {
    BuildMI(MBB, I, DL, get(Opc), DestReg)
      .addReg(SrcReg, getKillRegState(KillSrc))
      .addImm(0)  // Switches
      .addReg(DestReg, RegState::Undef)
      .addReg(TPC::SP0)
      .addImm(0);
  };

  const auto CopySubReg = [&](unsigned Opc, unsigned SubReg) {
    unsigned SrcSubReg = TRI.getSubReg(SrcReg, SubReg);
    unsigned DestSubReg = TRI.getSubReg(DestReg, SubReg);
    BuildMI(MBB, I, DL, get(Opc), DestSubReg)
      .addReg(SrcSubReg, getKillRegState(KillSrc))
      .addImm(0)  // Switches
      .addReg(DestReg, RegState::Undef)
      .addReg(TPC::SP0)
      .addImm(0);
  };

  bool DstVRF  = TPC::VRFRegClass.contains(DestReg);
  bool DstSRF  = TPC::SRFRegClass.contains(DestReg);
  bool DstIRF  = TPC::IRFRegClass.contains(DestReg);
  bool DstVPRF = TPC::VPRFRegClass.contains(DestReg);
  bool DstSPRF = TPC::SPRFRegClass.contains(DestReg);
  bool DstZRF = TPC::ZRFRegClass.contains(DestReg);
  bool DstARF = TPC::ARFRegClass.contains(DestReg);
  bool DstDRF = TPC::DRFRegClass.contains(DestReg);
  bool DstADRF = TPC::ADRFRegClass.contains(DestReg);
  bool DstHSRF = TPC::HSRFRegClass.contains(DestReg);

  bool SrcVRF = TPC::VRFRegClass.contains(SrcReg);
  bool SrcSRF = TPC::SRFRegClass.contains(SrcReg);
  bool SrcIRF = TPC::IRFRegClass.contains(SrcReg);
  bool SrcVPRF = TPC::VPRFRegClass.contains(SrcReg);
  bool SrcSPRF = TPC::SPRFRegClass.contains(SrcReg);
  bool SrcZRF = TPC::ZRFRegClass.contains(SrcReg);
  bool SrcARF = TPC::ARFRegClass.contains(SrcReg);
  bool SrcDRF = TPC::DRFRegClass.contains(SrcReg);
  bool SrcADRF = TPC::ADRFRegClass.contains(SrcReg);

  if (SrcSRF && DstSRF) {
    CopyRegWithDataType(TPC::MOVssp);
    return;
  }
  if (SrcZRF && DstZRF) {
    CopySubRegWithDataType(TPC::MOVssp, TPC::sub_s0);
    CopySubRegWithDataType(TPC::MOVssp, TPC::sub_s1);
    return;
  }
  if (SrcIRF && DstIRF) {
    BuildMI(MBB, I, DL, get(TPC::MOVIIp), DestReg)
      .addReg(SrcReg, getKillRegState(KillSrc))
      .addImm(31) // Mask
      .addImm(0)  // Switches
      .addReg(DestReg, RegState::Undef)
      .addReg(TPC::SP0)
      .addImm(0);
    return;
  }
  if (SrcSPRF && DstSPRF) {
    CopyReg(TPC::MOVppp);
    return;
  }

  if (SrcVPRF && DstVPRF) {
    CopyReg(TPC::MOVmmp);
    return;
  }
  if (SrcVRF && DstVRF) {
    CopyReg(TPC::MOVvvp);
    return;
  }

  if (SrcARF && DstARF) {
    CopySubReg(TPC::MOVvvp, TPC::sub_0);
    CopySubReg(TPC::MOVvvp, TPC::sub_1);
    CopySubReg(TPC::MOVvvp, TPC::sub_2);
    CopySubReg(TPC::MOVvvp, TPC::sub_3);
    return;
  }
  if (SrcDRF && DstDRF) {
    CopySubReg(TPC::MOVvvp, TPC::sub_0);
    CopySubReg(TPC::MOVvvp, TPC::sub_1);
    return;
  }
  if (SrcSRF && DstVRF) {
    CopyRegWithDataType(TPC::MOVvsp);
    return;
  }

  RegScavenger RS;
  RS.enterBasicBlock(MBB);
  RS.forward(I);

  MachineRegisterInfo &MRI = MBB.getParent()->getRegInfo();

  assert(false && "Incorrect register combination in COPY");
  report_fatal_error("Cannot copy phys reg");
}

void TPCInstrInfo::
storeRegToStackSlot(MachineBasicBlock &MBB, MachineBasicBlock::iterator I,
                    unsigned SrcReg, bool isKill, int FrameIndex,
                    const TargetRegisterClass *RC,
                    const TargetRegisterInfo *TRI) const {
  DebugLoc DL = MBB.findDebugLoc(I);
  MachineFunction *MF = MBB.getParent();
  MachineFrameInfo &FrameInfo = MF->getFrameInfo();

  // Determine save command code.
  int StoreOpCode = 0;
  uint8_t StackID = 0;
  if (TPC::SRFRegClass.hasSubClassEq(RC)) {
    StoreOpCode = TPC::SPILL_SRF_SAVE;
    StackID = TPCStackID::SLM_SPILL;
  } else if (TPC::SPRFRegClass.hasSubClassEq(RC)) {
    StoreOpCode = TPC::SPILL_SPRF_SAVE;
    StackID = TPCStackID::SLM_SPILL;
  } else if (TPC::VRFRegClass.hasSubClassEq(RC)) {
    StoreOpCode = TPC::SPILL_VRF_SAVE;
    StackID = TPCStackID::VLM_SPILL;
  } else if (TPC::VPRFRegClass.hasSubClassEq(RC)) {
    StoreOpCode = TPC::SPILL_VPRF_SAVE;
    StackID = TPCStackID::VLM_SPILL;
  } else if (TPC::ARFRegClass.hasSubClassEq(RC)) {
    StoreOpCode = TPC::SPILL_ARF_SAVE;
    StackID = TPCStackID::VLM_SPILL;
  } else if (TPC::DRFRegClass.hasSubClassEq(RC)) {
    StoreOpCode = TPC::SPILL_DRF_SAVE;
    StackID = TPCStackID::VLM_SPILL;
  } else if (TPC::ZRFRegClass.hasSubClassEq(RC)) {
    StoreOpCode = TPC::SPILL_ZRF_SAVE;
    StackID = TPCStackID::SLM_SPILL;
  } else if (TPC::IRFRegClass.hasSubClassEq(RC)) {
    if (Subtarget.getTargetLowering()->getTargetMachine().Options.LongIRF)
      report_fatal_error("IRF registers are not spillable if -long-irf is specified");
    StoreOpCode = TPC::SPILL_IRF_SAVE;
    StackID = TPCStackID::SLM_SPILL;
  } else {
    report_fatal_error("Unsupported register class in StoreToStack");
  }

  FrameInfo.setStackID(FrameIndex, StackID);
  BuildMI(MBB, I, DL, get(StoreOpCode))
    .addFrameIndex(FrameIndex)
    .addReg(SrcReg, getKillRegState(isKill));
}

void TPCInstrInfo::
loadRegFromStackSlot(MachineBasicBlock &MBB, MachineBasicBlock::iterator I,
                     unsigned DestReg, int FrameIndex,
                     const TargetRegisterClass *RC,
                     const TargetRegisterInfo *TRI) const {
  DebugLoc DL = MBB.findDebugLoc(I);
  MachineFunction *MF = MBB.getParent();
  MachineFrameInfo &FrameInfo = MF->getFrameInfo();

  // Determine load command code.
  int LoadOpCode = 0;
  uint8_t StackID = 0;
  if (TPC::SRFRegClass.hasSubClassEq(RC)) {
    LoadOpCode = TPC::SPILL_SRF_RESTORE;
    StackID = TPCStackID::SLM_SPILL;
  } else if (TPC::SPRFRegClass.hasSubClassEq(RC)) {
    LoadOpCode = TPC::SPILL_SPRF_RESTORE;
    StackID = TPCStackID::SLM_SPILL;
  } else if (TPC::VRFRegClass.hasSubClassEq(RC)) {
    LoadOpCode = TPC::SPILL_VRF_RESTORE;
    StackID = TPCStackID::VLM_SPILL;
  } else if (TPC::VPRFRegClass.hasSubClassEq(RC)) {
    LoadOpCode = TPC::SPILL_VPRF_RESTORE;
    StackID = TPCStackID::VLM_SPILL;
  } else if (TPC::ARFRegClass.hasSubClassEq(RC)) {
    LoadOpCode = TPC::SPILL_ARF_RESTORE;
    StackID = TPCStackID::VLM_SPILL;
  } else if (TPC::DRFRegClass.hasSubClassEq(RC)) {
    LoadOpCode = TPC::SPILL_DRF_RESTORE;
    StackID = TPCStackID::VLM_SPILL;
  } else if (TPC::ZRFRegClass.hasSubClassEq(RC)) {
    LoadOpCode = TPC::SPILL_ZRF_RESTORE;
    StackID = TPCStackID::SLM_SPILL;
  } else if (TPC::IRFRegClass.hasSubClassEq(RC)) {
    if (Subtarget.getTargetLowering()->getTargetMachine().Options.LongIRF)
      report_fatal_error("IRF registers are not spillable if -long-irf is specified");
    LoadOpCode = TPC::SPILL_IRF_RESTORE;
    StackID = TPCStackID::SLM_SPILL;
  } else {
    report_fatal_error("Unsupported register class in loadFromStack");
  }

  FrameInfo.setStackID(FrameIndex, StackID);
  BuildMI(MBB, I, DL, get(LoadOpCode), DestReg)
      .addFrameIndex(FrameIndex);
}

BitVector TPCInstrInfo::findAvailableReg(MachineInstr &MI, const TargetRegisterClass * RC) const {
  const TargetRegisterInfo &TRI = getRegisterInfo();
  MachineBasicBlock &MBB = *MI.getParent();
  RegScavenger RS;

  RS.enterBasicBlock(MBB);
  RS.forward(MI);

  BitVector Candidates =
      TRI.getAllocatableSet(*MBB.getParent(), RC);

  // Exclude all the registers being used by the instruction.
  for (MachineOperand &MO : MI.operands()) {
    if (MO.isReg() && MO.getReg() != 0 && !MO.isDef() &&
        !MO.getReg().isVirtual())
      Candidates.reset(MO.getReg());
  }

  BitVector Available = RS.getRegsAvailable(RC);
  Available &= Candidates;

  return Available;
}

BitVector TPCInstrInfo::findAvailableSRF(MachineInstr &MI) const {
  return findAvailableReg(MI, &TPC::SRFRegClass);
}

BitVector TPCInstrInfo::findAvailableVRF(MachineInstr &MI) const {
  return findAvailableReg(MI, &TPC::VRFRegClass);
}

bool TPCInstrInfo::expandPostRAPseudo(MachineInstr &MI) const {
  MachineBasicBlock &MBB = *MI.getParent();
  const TargetRegisterInfo &TRI = getRegisterInfo();
  DebugLoc DL = MI.getDebugLoc();

  switch (MI.getOpcode()) {
  default:
    break;

  // Process load instructions.
  case TPC::SPILL_SRF_RESTORE: {
    unsigned DestReg = MI.getOperand(0).getReg();
    unsigned Offset = MI.getOperand(1).getImm();
    BuildMI(MBB, MI, DL, get(TPC::LD_Lsip), DestReg)
        .addImm(Offset)
        .addImm(0)
        .addReg(DestReg, RegState::Undef)
        .addReg(TPC::SP0)
        .addImm(0);
    MBB.erase(MI);
    return true;
  }
  case TPC::SPILL_SPRF_RESTORE: {
    unsigned Offset = MI.getOperand(1).getImm();
    BuildMI(MBB, MI, DL, get(TPC::LD_Lpip), MI.getOperand(0).getReg())
        .addImm(Offset)
        .addImm(0)
        .addReg(MI.getOperand(0).getReg(), RegState::Undef)
        .addReg(TPC::SP0)
        .addImm(0);
    MBB.erase(MI);
    return true;
  }
  case TPC::SPILL_IRF_RESTORE: {
    BitVector Available = findAvailableSRF(MI);
    int regsFound = 0;
    unsigned sRegs[5];
    for (int i=0; i<5; i++) {
      signed Reg = Available.find_first();
      if (Reg != -1) {
        sRegs[i] = Reg;
        Available.reset(Reg);
        regsFound++;
      }
    }
    if (regsFound == 0) {
      llvm_unreachable("Cannot load IRF");
    }
    //dbgs() << "   === regsFound: " << regsFound << "\n";
    for (int k = regsFound; k < 5; k++) {
      sRegs[k] = sRegs[k-regsFound];
    }
    unsigned DestReg = MI.getOperand(0).getReg();
    unsigned Offset = MI.getOperand(1).getImm();
    for (int k = 0; k < regsFound; k++) {
      BuildMI(MBB, MI, DL, get(TPC::LD_Lsip), sRegs[k])
          .addImm(Offset + k*4)
          .addImm(0)
          .addReg(sRegs[k], RegState::Undef)
          .addReg(TPC::SP0)
          .addImm(0);
      BuildMI(MBB, MI, DL, get(TPC::SET_INDX_spu_rp), DestReg)
          .addReg(DestReg, (k == 0) ? RegState::Undef : 0)
          .addReg(sRegs[k], RegState::Kill)
          .addImm(1LL << k)
          .addImm(0)
          .addReg(TPC::SP0)
          .addImm(0);
    }
    MBB.erase(MI);
    return true;
  }
  case TPC::SPILL_VPRF_RESTORE:
    if (Subtarget.hasAddr1()) {
      unsigned DestReg = MI.getOperand(0).getReg();
      BuildMI(MBB, MI, DL, get(TPC::LD_L_Vmip), DestReg)
          .addImm(MI.getOperand(1).getImm())
          .addImm(0)
          .addReg(DestReg, RegState::Undef)
          .addReg(TPC::SP0)
          .addImm(0);
    } else {
      unsigned DestReg = MI.getOperand(0).getReg();
      BuildMI(MBB, MI, DL, get(TPC::LD_L_Vmsip), DestReg)
          .addReg(Subtarget.getTargetLowering()->getZeroReg())
          .addImm(MI.getOperand(1).getImm())
        .addImm(0)
        .addReg(DestReg, RegState::Undef)
          .addReg(TPC::SP0)
          .addImm(0);
    }
    MBB.erase(MI);
    return true;
  case TPC::SPILL_VRF_RESTORE: {
    unsigned DestReg = MI.getOperand(0).getReg();
    unsigned Offset = MI.getOperand(1).getImm();
    if (Subtarget.hasAddr1()) {
      BuildMI(MBB, MI, DL, get(TPC::LD_L_Vvip), DestReg)
          .addImm(Offset)
          .addImm(0)
          .addReg(DestReg, RegState::Undef)
          .addReg(TPC::SP0)
          .addImm(0);
    } else {
      BuildMI(MBB, MI, DL, get(TPC::LD_L_Vvsip), MI.getOperand(0).getReg())
          .addReg(Subtarget.getTargetLowering()->getZeroReg())
          .addImm(Offset)
          .addImm(0)
          .addReg(DestReg, RegState::Undef)
          .addReg(TPC::SP0)
          .addImm(0);
    }
    MBB.erase(MI);
    return true;
  }
  case TPC::SPILL_ZRF_RESTORE: {
    unsigned Offset = MI.getOperand(1).getImm();
    unsigned DestReg = MI.getOperand(0).getReg();
    unsigned SubReg = TRI.getSubReg(DestReg, TPC::sub_s0);
    BuildMI(MBB, MI, DL, get(TPC::LD_Lsip), SubReg)
        .addImm(Offset)
        .addImm(0)
        .addReg(SubReg, RegState::Undef)
        .addReg(TPC::SP0)
        .addImm(0);
    SubReg = TRI.getSubReg(DestReg, TPC::sub_s1);
    BuildMI(MBB, MI, DL, get(TPC::LD_Lsip), SubReg)
        .addImm(Offset + 4)
        .addImm(0)
        .addReg(SubReg, RegState::Undef)
        .addReg(TPC::SP0)
        .addImm(0);
    MBB.erase(MI);
    return true;
  }
  case TPC::SPILL_DRF_RESTORE: {
    unsigned Offset = MI.getOperand(1).getImm();
    unsigned DestReg = MI.getOperand(0).getReg();
    unsigned SubReg = TRI.getSubReg(DestReg, TPC::sub_0);
    if (Subtarget.hasAddr1()) {
      BuildMI(MBB, MI, DL, get(TPC::LD_L_Vvip), SubReg)
          .addImm(Offset)
          .addImm(0)
          .addReg(SubReg, RegState::Undef)
          .addReg(TPC::SP0)
          .addImm(0);
      SubReg = TRI.getSubReg(DestReg, TPC::sub_1);
      BuildMI(MBB, MI, DL, get(TPC::LD_L_Vvip), SubReg)
          .addImm(Offset + 256)
          .addImm(0)
          .addReg(SubReg, RegState::Undef)
          .addReg(TPC::SP0)
          .addImm(0);
    } else {
      BuildMI(MBB, MI, DL, get(TPC::LD_L_Vvsip), SubReg)
          .addReg(Subtarget.getTargetLowering()->getZeroReg())
          .addImm(Offset)
          .addImm(0)
          .addReg(SubReg, RegState::Undef)
          .addReg(TPC::SP0)
          .addImm(0);
      SubReg = TRI.getSubReg(DestReg, TPC::sub_1);
      BuildMI(MBB, MI, DL, get(TPC::LD_L_Vvsip), SubReg)
          .addReg(Subtarget.getTargetLowering()->getZeroReg())
          .addImm(Offset + 256)
          .addImm(0)
          .addReg(SubReg, RegState::Undef)
          .addReg(TPC::SP0)
          .addImm(0);
    }
    MBB.erase(MI);
    return true;
  }
  case TPC::SPILL_ARF_RESTORE: {
    unsigned Offset = MI.getOperand(1).getImm();
    unsigned DestReg = MI.getOperand(0).getReg();
    unsigned SubReg = TRI.getSubReg(DestReg, TPC::sub_0);
    if (Subtarget.hasAddr1()) {
      BuildMI(MBB, MI, DL, get(TPC::LD_L_Vvip), SubReg)
          .addImm(Offset)
          .addImm(0)
          .addReg(SubReg, RegState::Undef)
          .addReg(TPC::SP0)
          .addImm(0);
      SubReg = TRI.getSubReg(DestReg, TPC::sub_1);
      BuildMI(MBB, MI, DL, get(TPC::LD_L_Vvip), SubReg)
          .addImm(Offset + 256)
          .addImm(0)
          .addReg(SubReg, RegState::Undef)
          .addReg(TPC::SP0)
          .addImm(0);
      SubReg = TRI.getSubReg(DestReg, TPC::sub_2);
      BuildMI(MBB, MI, DL, get(TPC::LD_L_Vvip), SubReg)
          .addImm(Offset + 512)
          .addImm(0)
          .addReg(SubReg, RegState::Undef)
          .addReg(TPC::SP0)
          .addImm(0);
      SubReg = TRI.getSubReg(DestReg, TPC::sub_3);
      BuildMI(MBB, MI, DL, get(TPC::LD_L_Vvip), SubReg)
          .addImm(Offset + 768)
          .addImm(0)
          .addReg(SubReg, RegState::Undef)
          .addReg(TPC::SP0)
          .addImm(0);
    } else {
      BuildMI(MBB, MI, DL, get(TPC::LD_L_Vvsip), SubReg)
          .addReg(Subtarget.getTargetLowering()->getZeroReg())
          .addImm(Offset)
          .addImm(0)
          .addReg(SubReg, RegState::Undef)
          .addReg(TPC::SP0)
          .addImm(0);
      SubReg = TRI.getSubReg(DestReg, TPC::sub_1);
      BuildMI(MBB, MI, DL, get(TPC::LD_L_Vvsip), SubReg)
          .addReg(Subtarget.getTargetLowering()->getZeroReg())
          .addImm(Offset + 256)
          .addImm(0)
          .addReg(SubReg, RegState::Undef)
          .addReg(TPC::SP0)
          .addImm(0);
      SubReg = TRI.getSubReg(DestReg, TPC::sub_2);
      BuildMI(MBB, MI, DL, get(TPC::LD_L_Vvsip), SubReg)
          .addReg(Subtarget.getTargetLowering()->getZeroReg())
          .addImm(Offset + 512)
          .addImm(0)
          .addReg(SubReg, RegState::Undef)
          .addReg(TPC::SP0)
          .addImm(0);
      SubReg = TRI.getSubReg(DestReg, TPC::sub_3);
      BuildMI(MBB, MI, DL, get(TPC::LD_L_Vvsip), SubReg)
          .addReg(Subtarget.getTargetLowering()->getZeroReg())
          .addImm(Offset + 768)
          .addImm(0)
          .addReg(SubReg, RegState::Undef)
          .addReg(TPC::SP0)
          .addImm(0);
    }
    MBB.erase(MI);
    return true;
  }

  // Process save instructions.
  case TPC::SPILL_SRF_SAVE: {
    unsigned Offset = MI.getOperand(0).getImm();
    BuildMI(MBB, MI, DL, get(TPC::ST_Lisp))
        .addImm(Offset)
        .addReg(MI.getOperand(1).getReg(),
                MI.getOperand(1).isKill() ? RegState::Kill : 0)
        .addImm(0)
        .addReg(TPC::SP0)
        .addImm(0);
    MBB.erase(MI);
    return true;
  }
  case TPC::SPILL_SPRF_SAVE: {
    unsigned Offset = MI.getOperand(0).getImm();
    BuildMI(MBB, MI, DL, get(TPC::ST_Lipp))
        .addImm(Offset)
        .addReg(MI.getOperand(1).getReg(),
                MI.getOperand(1).isKill() ? RegState::Kill : 0)
        .addImm(0)
        .addReg(TPC::SP0)
        .addImm(0);
    MBB.erase(MI);
    return true;
  }
  case TPC::SPILL_IRF_SAVE: {
    BitVector Available = findAvailableSRF(MI);
    int regsFound = 0;
    unsigned sRegs[5];
    for (int i=0; i<5; i++) {
      signed Reg = Available.find_first();
      if (Reg != -1) {
        sRegs[i] = Reg;
        Available.reset(Reg);
        regsFound++;
      }
    }
    if (regsFound == 0) {
      llvm_unreachable("Cannot store IRF");
    }
    //dbgs() << "   === regsFound for store: " << regsFound << "\n";
    for (int k = regsFound; k < 5; k++) {
      sRegs[k] = sRegs[k-regsFound];
    }
    unsigned Offset = MI.getOperand(0).getImm();
    for (int k = 0; k < regsFound; k++) {
      BuildMI(MBB, MI, DL, get(TPC::MOV_IRF_DIM), sRegs[k])
          .addReg(MI.getOperand(1).getReg(), 0)
          .addImm(k) // DIM
          .addImm(0) // SW
          .addReg(sRegs[k], RegState::Undef) // income
          .addReg(TPC::SP0)
          .addImm(0);
      BuildMI(MBB, MI, DL, get(TPC::ST_Lisp))
          .addImm(Offset + k*4)
          .addReg(sRegs[k], RegState::Kill)
          .addImm(0)
          .addReg(TPC::SP0)
          .addImm(0);
    }
    MBB.erase(MI);
    return true;
  }
  case TPC::SPILL_VPRF_SAVE:
    if (Subtarget.hasAddr1()) {
      BuildMI(MBB, MI, DL, get(TPC::ST_L_Vimp))
          .addImm(MI.getOperand(0).getImm())
          .addReg(MI.getOperand(1).getReg(),
                  MI.getOperand(1).isKill() ? RegState::Kill : 0)
          .addImm(0)
          .addReg(TPC::SP0)
          .addImm(0);
    } else {
      BuildMI(MBB, MI, DL, get(TPC::ST_L_Vsimp))
          .addReg(Subtarget.getTargetLowering()->getZeroReg())
          .addImm(MI.getOperand(0).getImm())
          .addReg(MI.getOperand(1).getReg(),
                  MI.getOperand(1).isKill() ? RegState::Kill : 0)
          .addImm(0)
          .addReg(TPC::SP0)
          .addImm(0);
    }
    MBB.erase(MI);
    return true;
  case TPC::SPILL_VRF_SAVE: {
    unsigned Offset = MI.getOperand(0).getImm();
    if (Subtarget.hasAddr1()) {
      BuildMI(MBB, MI, DL, get(TPC::ST_L_Vivp))
          .addImm(Offset)
          .addReg(MI.getOperand(1).getReg(),
                  MI.getOperand(1).isKill() ? RegState::Kill : 0)
          .addImm(0)
          .addReg(TPC::SP0)
          .addImm(0);
    } else {
      BuildMI(MBB, MI, DL, get(TPC::ST_L_Vsivp))
          .addReg(Subtarget.getTargetLowering()->getZeroReg())
          .addImm(Offset)
          .addReg(MI.getOperand(1).getReg(),
                  MI.getOperand(1).isKill() ? RegState::Kill : 0)
          .addImm(0)
          .addReg(TPC::SP0)
          .addImm(0);
    }
    MBB.erase(MI);
    return true;
  }
  case TPC::SPILL_ZRF_SAVE: {
    unsigned Offset = MI.getOperand(0).getImm();
    unsigned Flags = MI.getOperand(1).isKill() ? RegState::Kill : 0;
    unsigned SrcReg = MI.getOperand(1).getReg();
    unsigned SubReg = TRI.getSubReg(SrcReg, TPC::sub_s0);
    BuildMI(MBB, MI, DL, get(TPC::ST_Lisp))
        .addImm(Offset)
        .addReg(SubReg, Flags)
        .addImm(0)
        .addReg(TPC::SP0)
        .addImm(0);
    SubReg = TRI.getSubReg(SrcReg, TPC::sub_s1);
    BuildMI(MBB, MI, DL, get(TPC::ST_Lisp))
        .addImm(Offset + 4)
        .addReg(SubReg, Flags);
    MBB.erase(MI);
    return true;
  }
  case TPC::SPILL_DRF_SAVE: {
    unsigned Offset = MI.getOperand(0).getImm();
    unsigned Flags = MI.getOperand(1).isKill() ? RegState::Kill : 0;
    unsigned SrcReg = MI.getOperand(1).getReg();
    unsigned SubReg = TRI.getSubReg(SrcReg, TPC::sub_0);
    if (Subtarget.hasAddr1()) {
      BuildMI(MBB, MI, DL, get(TPC::ST_L_Vivp))
          .addImm(Offset)
          .addReg(SubReg, Flags)
          .addImm(0)
          .addReg(TPC::SP0)
          .addImm(0);
      SubReg = TRI.getSubReg(SrcReg, TPC::sub_1);
      BuildMI(MBB, MI, DL, get(TPC::ST_L_Vivp))
        .addImm(Offset + 256)
        .addReg(SubReg, Flags)
        .addImm(0)
        .addReg(TPC::SP0)
        .addImm(0);
    } else {
      BuildMI(MBB, MI, DL, get(TPC::ST_L_Vsivp))
        .addReg(Subtarget.getTargetLowering()->getZeroReg())
        .addImm(Offset)
        .addReg(SubReg, Flags)
        .addImm(0)
        .addReg(TPC::SP0)
        .addImm(0);
      SubReg = TRI.getSubReg(SrcReg, TPC::sub_1);
      BuildMI(MBB, MI, DL, get(TPC::ST_L_Vsivp))
        .addReg(Subtarget.getTargetLowering()->getZeroReg())
        .addImm(Offset + 256)
        .addReg(SubReg, Flags)
        .addImm(0)
        .addReg(TPC::SP0)
        .addImm(0);
    }
    MBB.erase(MI);
    return true;
  }
  case TPC::SPILL_ARF_SAVE: {
    unsigned Offset = MI.getOperand(0).getImm();
    unsigned Flags = MI.getOperand(1).isKill() ? RegState::Kill : 0;
    unsigned SrcReg = MI.getOperand(1).getReg();
    unsigned SubReg = TRI.getSubReg(SrcReg, TPC::sub_0);
    if (Subtarget.hasAddr1()) {
      BuildMI(MBB, MI, DL, get(TPC::ST_L_Vivp))
          .addImm(Offset)
          .addReg(SubReg, Flags)
          .addImm(0)
          .addReg(TPC::SP0)
          .addImm(0);
      SubReg = TRI.getSubReg(SrcReg, TPC::sub_1);
      BuildMI(MBB, MI, DL, get(TPC::ST_L_Vivp))
          .addImm(Offset + 256)
          .addReg(SubReg, Flags)
          .addImm(0)
          .addReg(TPC::SP0)
          .addImm(0);
      SubReg = TRI.getSubReg(SrcReg, TPC::sub_2);
      BuildMI(MBB, MI, DL, get(TPC::ST_L_Vivp))
          .addImm(Offset + 512)
          .addReg(SubReg, Flags)
          .addImm(0)
          .addReg(TPC::SP0)
          .addImm(0);
      SubReg = TRI.getSubReg(SrcReg, TPC::sub_3);
      BuildMI(MBB, MI, DL, get(TPC::ST_L_Vivp))
          .addImm(Offset + 768)
          .addReg(SubReg, Flags)
          .addImm(0)
          .addReg(TPC::SP0)
          .addImm(0);
    } else {
      BuildMI(MBB, MI, DL, get(TPC::ST_L_Vsivp))
          .addReg(Subtarget.getTargetLowering()->getZeroReg())
          .addImm(Offset)
          .addReg(SubReg, Flags)
          .addImm(0)
          .addReg(TPC::SP0)
          .addImm(0);
      SubReg = TRI.getSubReg(SrcReg, TPC::sub_1);
      BuildMI(MBB, MI, DL, get(TPC::ST_L_Vsivp))
          .addReg(Subtarget.getTargetLowering()->getZeroReg())
          .addImm(Offset + 256)
          .addReg(SubReg, Flags)
          .addImm(0)
          .addReg(TPC::SP0)
          .addImm(0);
      SubReg = TRI.getSubReg(SrcReg, TPC::sub_2);
      BuildMI(MBB, MI, DL, get(TPC::ST_L_Vsivp))
          .addReg(Subtarget.getTargetLowering()->getZeroReg())
          .addImm(Offset + 512)
          .addReg(SubReg, Flags)
          .addImm(0)
          .addReg(TPC::SP0)
          .addImm(0);
      SubReg = TRI.getSubReg(SrcReg, TPC::sub_3);
      BuildMI(MBB, MI, DL, get(TPC::ST_L_Vsivp))
          .addReg(Subtarget.getTargetLowering()->getZeroReg())
          .addImm(Offset + 768)
          .addReg(SubReg, Flags)
          .addImm(0)
          .addReg(TPC::SP0)
          .addImm(0);
    }
    MBB.erase(MI);
    return true;
  }
  }

  return false;
}

// Uncomment this if TPC specific HazardRec is needed before RA
ScheduleHazardRecognizer *
TPCInstrInfo::CreateTargetMIHazardRecognizer(
      const InstrItineraryData *II, const ScheduleDAGMI *DAG) const {
  return createTPCHazardRecognizer(II, DAG, false);
}

ScheduleHazardRecognizer *
TPCInstrInfo::CreateTargetPostRAHazardRecognizer(
      const InstrItineraryData *II, const ScheduleDAG *DAG) const {
  return createTPCHazardRecognizer(II, DAG, true);
}

static TPCLatencyEvaluation::IssueSlot convertSlot(unsigned Slot) {
  switch (Slot) {
  case TPCII::TypeVPU: return TPCLatencyEvaluation::e_issue_slot_vpu;
  case TPCII::TypeSPU: return TPCLatencyEvaluation::e_issue_slot_spu;
  case TPCII::TypeLOAD: return TPCLatencyEvaluation::e_issue_slot_load;
  case TPCII::TypeSTORE: return TPCLatencyEvaluation::e_issue_slot_store;
  case TPCII::TypeLOOP: return TPCLatencyEvaluation::e_issue_slot_spu;
  default:
    llvm_unreachable("Unexpected slot value");
  }
}

static TPCLatencyEvaluation::RegisterFile convertReg(const TargetRegisterClass *RC) {
  if (TPC::VRFRegClass.hasSubClassEq(RC) ||
      TPC::ARFRegClass.hasSubClassEq(RC) ||
      TPC::DRFRegClass.hasSubClassEq(RC))
    return TPCLatencyEvaluation::RegisterFile::e_rf_v;
  if (TPC::VPRFRegClass.hasSubClassEq(RC))
    return TPCLatencyEvaluation::RegisterFile::e_rf_vp;
  if (TPC::SRFRegClass.hasSubClassEq(RC) || TPC::ZRFRegClass.hasSubClassEq(RC))
    return TPCLatencyEvaluation::RegisterFile::e_rf_s;
  if (TPC::SPRFRegClass.hasSubClassEq(RC))
    return TPCLatencyEvaluation::RegisterFile::e_rf_sp;
  if (TPC::IRFRegClass.hasSubClassEq(RC))
    return TPCLatencyEvaluation::RegisterFile::e_rf_i;
  if (TPC::ADRFRegClass.hasSubClassEq(RC))
    return TPCLatencyEvaluation::RegisterFile::e_rf_a;
  llvm_unreachable("Unexpected register class");
}

static bool isFloatData(TPCII::OpType X) {
  switch (X) {
  case TPCII::OpType::BF16:
  case TPCII::OpType::FP32:
    return true;
  default:
    return false;
  }
}

static bool isPredicateReg(const TargetRegisterClass *RC) {
  return RC == &TPC::VPRFRegClass || RC == &TPC::SPRFRegClass;
}

static bool isLongRegister(const TargetRegisterClass *RC) {
  return RC == &TPC::ARFRegClass || RC == &TPC::DRFRegClass || RC == &TPC::ZRFRegClass;
}


static bool is_convert_instr_with_lane(const MachineInstr &MI) {
  const MCInstrDesc &MC = MI.getDesc();
  unsigned opc = TPCII::getSlotOpCode(MC);
  if (TPCII::isSPUInst(MC)) {
    return opc == TPCII::spuCONVERT_INT32 ||
           opc == TPCII::spuCONVERT_UINT32 ||
           opc == TPCII::spuCONVERT_INT16 ||
           opc == TPCII::spuCONVERT_UINT16;
  }
  if (TPCII::isVPUInst(MC)) {
    return opc == TPCII::vpuCONVERT_INT32 ||
           opc == TPCII::vpuCONVERT_UINT32 ||
           opc == TPCII::vpuCONVERT_INT16 ||
           opc == TPCII::vpuCONVERT_UINT16;
  }
  return false;
}

static unsigned getLaneSel(const MachineInstr &MI) {
  assert(is_convert_instr_with_lane(MI));
  unsigned opNum = 3;
  const MachineOperand &MO = MI.getOperand(opNum);
  assert(MO.isImm());
  return MO.getImm() & 0x3;
}
 
static TPCLatencyEvaluation::OperandID getOperandId(const TPCSubtarget &Subtarget,
                                                    const MachineInstr &MI,
                                                    const TargetRegisterClass *UseRegClass,
                                                    unsigned UseIdx) {
  const MCInstrDesc &MC = MI.getDesc();
  assert(UseIdx < MC.getNumOperands());
  const MachineOperand &MO = MI.getOperand(UseIdx);
  assert(MO.isReg());

  // Some instructions have immediate operand. Check that we do not process it.
#ifdef _DEBUG
  if (TPCII::getHasImm(MC)) {
    unsigned ImmPos = TPCII::getImmFieldOpNum(MC);
    assert(ImmPos != UseIdx);
    const MachineOperand &MOImm = MI.getOperand(ImmPos);
    assert(MOImm.isImm() || MOImm.isFPImm() || MOImm.isMBB() ||
           MOImm.isBlockAddress());
  }
#endif

  // Process the specific case, when used operand is tied to output.
  if (MO.isTied()) {
    // Fron Ron's letter of 2017-08-24: when accumulator in MAC or MSAC is used
    // as a destination, it is encoded as SRC_C in latency database.
    if (TPCII::getSlotOpCode(MC) == TPCII::vpuMAC &&
        (TPCII::isSPUInst(MC) || TPCII::isVPUInst(MC)))
      return TPCLatencyEvaluation::OperandID::e_src_c;
    if (TPCII::getSlotOpCode(MC) == TPCII::vpuMSAC)
      return TPCLatencyEvaluation::OperandID::e_src_c;

    if (TPCII::getSlotOpCode(MC) == TPCII::spuUDIV_STEP && TPCII::isSPUInst(MC))
      return TPCLatencyEvaluation::OperandID::e_src_b;


    // If the tied operand is an IRF register, calculate the latency using
    // SRC_B or SRC_A depending on the instruction.
    if (UseRegClass == &TPC::IRFRegClass) {
      // SET_INDX is actually an assignment to the IRF register, it does not
      // require knowledge of the value, so there is no write-to-read latency.
      // Return 'e_dst' to indicate this fact.
      if ((TPCII::isSPUInst(MC) && TPCII::getSlotOpCode(MC) == TPCII::spuSET_INDX) ||
          (TPCII::isLoadInst(MC) && TPCII::getSlotOpCode(MC) == TPCII::ldSET_INDX) ||
          (TPCII::isStoreInst(MC) && TPCII::getSlotOpCode(MC) == TPCII::stSET_INDX))
        return TPCLatencyEvaluation::OperandID::e_dst;

      // In Load slot (PRMT_INDX and MOV) and in Store slot (PRMT_INDX) tied
      // operand corresponds to SRC_A.
      if (TPCII::isLoadInst(MC) || TPCII::isStoreInst(MC))
        return TPCLatencyEvaluation::OperandID::e_src_a;

      //  In SPU slot unary operations (NOT, ABS and MOV) also has tied operand
      // in SRC_A.
      if (TPCII::isSPUInst(MC))
        switch (TPCII::getSlotOpCode(MC)) {
        default:
          break;
        case TPCII::spuMOV:
        case TPCII::spuABS:
        case TPCII::spuNOT:
          return TPCLatencyEvaluation::OperandID::e_src_a;
        }

      return TPCLatencyEvaluation::OperandID::e_src_b;
    }

    return TPCLatencyEvaluation::OperandID::e_dst;
  }

  // Skip tied operands to simplify calculation of mapping arguments to operand
  // ids. Idealy we would have mapping arguments (1, 2, 3, 4) to
  // (SRC_A, SRC_B, SRC_C, SRC_D), but actually there are some complications to
  // this scheme.
  for (unsigned i = MC.getNumDefs(), e = UseIdx; i < e; i++) {
    const MachineOperand & Op = MI.getOperand(i);
    if (Op.isReg() && Op.isTied())
      --UseIdx;
  }

  // Change UseIdx so that it counts only input operands (0 - the first input, etc).
  assert(UseIdx >= MC.getNumDefs());
  UseIdx -= MC.getNumDefs();

  switch (UseIdx) {
  case 0:
    // In all cases the first input is in SRC_A.
    return TPCLatencyEvaluation::OperandID::e_src_a;

  case 1:
    // Usually the second input is in SRC_B, but in some cases it is not so.
    if (TPCII::isStoreInst(MC)) {
      switch (TPCII::getSlotOpCode(MC)) {
      case TPCII::ST_L:
        // ST_L uses SRC_B to store MMIO switch.
      case TPCII::ST_G:
        // ST_G does not use SRC_B.
        return TPCLatencyEvaluation::OperandID::e_src_c;
      default:
        switch (MC.getOpcode()) {
        case TPC::SET_INDX_st_ip:
        case TPC::SET_INDX_st_rp:
          // SET_INDX in store slot keeps DIM_MASK in SRC_B.
          return TPCLatencyEvaluation::OperandID::e_src_c;
        }
        if (MC.getOpcode() == TPC::GEN_ADDR_st) {
          return TPCLatencyEvaluation::OperandID::e_src_a;
        }
        // Check for Gaudi's ST_TNSR, where the second operand is hidden
        if (Subtarget.hasGaudiISA()) {
          if (TPCII::getSlotOpCode(MC) == 11 ||
              TPCII::getSlotOpCode(MC) == 12 ||
              TPCII::getSlotOpCode(MC) == 13) {
              return TPCLatencyEvaluation::OperandID::e_src_c;
          }
                if (TPCII::getSlotOpCode(MC) == TPCII::ST_L_V) {
              return TPCLatencyEvaluation::OperandID::e_src_c;
          }
        }
        break;
      }

      // For all other instruction in Store slot the second operand is
      // encoded into SRC_B.
      return TPCLatencyEvaluation::OperandID::e_src_b;
    }

    if (MC.getOpcode() == TPC::SET_INDX_ld_rp ||
        MC.getOpcode() == TPC::SET_INDX_ld_ip ||
        MC.getOpcode() == TPC::SET_INDX_spu_rp ||
        MC.getOpcode() == TPC::SET_INDX_spu_ip ||
        MC.getOpcode() == TPC::GEN_ADDR_ld) {
      return TPCLatencyEvaluation::OperandID::e_src_a;
    }
    return TPCLatencyEvaluation::OperandID::e_src_b;

  case 2:
    if (MC.getOpcode() == TPC::SET_INDX_st_rp ||
        MC.getOpcode() == TPC::SET_INDX_st_ip)
      return TPCLatencyEvaluation::OperandID::e_src_a;
    return TPCLatencyEvaluation::OperandID::e_src_c;
  case 3:
    return TPCLatencyEvaluation::OperandID::e_src_d;

  default:
    llvm_unreachable("Unexpected operand index");
  }
}

/// Checks if the two given instruction make up a conditional chain.
///
/// Conditional chains are composed by two instructions with the same predicate
/// and destination but opposite polarity, like:
///
/// d = INSTR a, i, pred, 0
/// d = INSTR a, d, pred, 1
///
/// In such chains the latency between the two instructions is zero.
///
bool TPCInstrInfo::isConditionalChain(const MachineInstr &DefMI,
                                      const MachineInstr &UseMI) const {
  const MachineFunction &MF = *DefMI.getParent()->getParent();
  const MachineRegisterInfo &MRI = MF.getRegInfo();

  if (UseMI.getNumDefs() != 1)
    return false;
  if (UseMI.getNumOperands() < 4)  // dest, income, pred, polarity
    return false;
  if (DefMI.getNumDefs() != 1 || DefMI.getNumOperands() < 4)
    return false;

  // Polarity

  const MachineOperand &UsePolarityOp = UseMI.getOperand(UseMI.getNumOperands() - 1);
  if (!UsePolarityOp.isImm())
    return false;
  bool UsePolarity = UsePolarityOp.getImm();

  const MachineOperand &DefPolarityOp = DefMI.getOperand(DefMI.getNumOperands() - 1);
  if (!DefPolarityOp.isImm())
    return false;
  bool DefPolarity = DefPolarityOp.getImm();

  // Polarities must be opposite.
  if (UsePolarity == DefPolarity)
    return false;

  // Predicate

  const MachineOperand &DefPredicateOp = DefMI.getOperand(DefMI.getNumOperands() - 2);
  if (!DefPredicateOp.isReg())
    return false;
  unsigned DefPredicate = DefPredicateOp.getReg();
  if (DefPredicateOp.isKill())
    return false;

  const MachineOperand &UsePredicateOp = UseMI.getOperand(UseMI.getNumOperands() - 2);
  if (!UsePredicateOp.isReg())
    return false;
  unsigned UsePredicate = UsePredicateOp.getReg();

  // Predicate must be the same.
  if (DefPredicate != UsePredicate)
    return false;
  bool UseFound = false;
  bool DefFound = false;
  for (const MachineOperand &U : MRI.use_operands(DefPredicate)) {
    if (DefFound) {
      if (U.getParent() == &UseMI) {
        UseFound = true;
        break;
      }
      if (U.isKill())
        break;
    } else {
      if (U.getParent() == &DefMI)
        DefFound = true;
    }
  }
  if (!UseFound)
    return false;

  // Income

  const MachineOperand &IncomeOp = UseMI.getOperand(UseMI.getNumOperands() - 3);
  if (!IncomeOp.isReg())
    return false;
  unsigned IncomeReg = IncomeOp.getReg();

  if (DefMI.getOperand(0).getReg() != IncomeReg)
    return false;

  LLVM_DEBUG(dbgs() << "Conditional chain recognized:"
                    << "    "; DefMI.dump();
             dbgs() << "    "; UseMI.dump(););
  return true;
}

int TPCInstrInfo::getOperandLatency(const InstrItineraryData *ItinData,
                                    const MachineInstr &DefMI, unsigned DefIdx,
                                    const MachineInstr &UseMI, unsigned UseIdx) const {
  DEBUG_WITH_TYPE("latency", dbgs() << "== Latency between op" << DefIdx << " and op" << UseIdx << "\n");
  DEBUG_WITH_TYPE("latency", dbgs() << "    " << DefMI);
  DEBUG_WITH_TYPE("latency", dbgs() << "    " << UseMI);

  // Skip debug instructions.
  if (UseMI.isDebugValue()) {
    DEBUG_WITH_TYPE("latency", dbgs() << "== Latency (use in debug value) = " << 0 << "\n");
    DEBUG_WITH_TYPE("latency", dbgs() << "=====================================================\n");
    return 0;
  }

  const llvm::MachineFunction &MF = *DefMI.getParent()->getParent();
  const MachineRegisterInfo &MRI = MF.getRegInfo();
  
  const MCInstrDesc &DefMCID = DefMI.getDesc();
  const MachineOperand &DefOperand = DefMI.getOperand(DefIdx);
  assert(DefOperand.isDef());

  const MCInstrDesc &UseMCID = UseMI.getDesc();
  const MachineOperand &UseOperand = UseMI.getOperand(UseIdx);
  // Cannot require that UseOperand is not a definition. For implicit uses of
  // long registers we can have more fancy case (def = 2, use 0):
  //
  // %V0<def> = MOVi32vi 0, %A0<imp-def>
  // %V1<def> = MOVf32vv %V0<kill>, %A0<imp-use, kill>, %A0<imp-def>
  //
  // Probably this is a bug of NoopInserter <- TODO: check it
  assert(!UseOperand.isDef() || UseOperand.isTied() || UseIdx < UseMCID.getNumDefs());
  // assert(!UseOperand.isDef());
  assert(UseOperand.isReg());

  // Determine register classes of the producer and consumer.

  Register DefReg = DefOperand.getReg();
  const TargetRegisterClass *DefRegClass;
  if (DefReg.isPhysical()) {
    DefRegClass = getRegClass(DefMCID, DefIdx, &RI, MF);
    if (!DefRegClass)
      DefRegClass = getClassOfPhysicalRegister(DefReg, RI);
  } else {
    DefRegClass = MRI.getRegClass(DefReg);
  }
  assert(DefRegClass);

  Register UseReg = UseOperand.getReg();
  const TargetRegisterClass *UseRegClass;
  if (UseReg.isPhysical()) {
    UseRegClass = getRegClass(UseMCID, UseIdx, &RI, MF);
    if (!UseRegClass)
      UseRegClass = getClassOfPhysicalRegister(UseReg, RI);
  } else {
    UseRegClass = MRI.getRegClass(UseReg);
  }
  assert(UseRegClass);

  // Skip pseudo instructions.
  if (DefMI.getOpcode() == TPC::KILL) {
    DEBUG_WITH_TYPE("latency", dbgs() << "== Latency (use in KILL) = " << 0 << "\n");
    DEBUG_WITH_TYPE("latency", dbgs() << "=====================================================\n");
    return 0;
  }

  if (DefRegClass->hasSuperClassEq(&TPC::HSRFRegClass)) {
    // LLVM-1281 Optimize ST_TNSR_ID_REG latency
    DEBUG_WITH_TYPE("latency", dbgs() << "== Latency HWReg = " << 4 << "\n");
    DEBUG_WITH_TYPE("latency", dbgs() << "=====================================================\n");
    return 4;
  }

  if (DefMI.getOpcode() == TPC::COPY && !UseMI.isPseudo()) {
    unsigned CopySrcSubReg = DefMI.getOperand(1).getSubReg();

    // Cost of IRF copy is 1.
    if (DefRegClass == &TPC::IRFRegClass) {
      assert(UseRegClass == &TPC::IRFRegClass);
      DEBUG_WITH_TYPE("latency", dbgs() << "== Latency COPY to consumer (IRF) = " << 1 << "\n");
      DEBUG_WITH_TYPE("latency", dbgs() << "=====================================================\n");
      return 1;
    }

    // Cost of copy from wide registed to short is estimated as 1 (subregister is directly used).
    if (CopySrcSubReg && UseOperand.getSubReg() == 0) {
      DEBUG_WITH_TYPE("latency", dbgs() << "== Latency COPY to consumer (short reg) = " << 1 << "\n");
      DEBUG_WITH_TYPE("latency", dbgs() << "=====================================================\n");
      return 1;
    }

    // Cost of copy between different subregisters is 4.
    if (CopySrcSubReg && CopySrcSubReg != UseOperand.getSubReg()) {
      DEBUG_WITH_TYPE("latency", dbgs() << "== Latency COPY to consumer (subreg) = " << 4 << "\n");
      DEBUG_WITH_TYPE("latency", dbgs() << "=====================================================\n");
      return 4;
    }

    // If the definition is a COPY and the use is not another copy, then compute
    // latency value as for copy instruction.
    //
    // TODO: is there more correct way to determine latency in this case?
    DEBUG_WITH_TYPE("latency", dbgs() << "== Latency COPY to consumer = " << 1 << "\n");
    DEBUG_WITH_TYPE("latency", dbgs() << "=====================================================\n");
    return 1;
  }

  if (DefMI.isPseudo() || UseMI.isPseudo()) {
    // TODO: Must be 0 but for now return copy cost for safety.
    DEBUG_WITH_TYPE("latency", dbgs() << "== Latency (use in pseudo) = " << 1 << "\n");
    DEBUG_WITH_TYPE("latency", dbgs() << "=====================================================\n");
    return 1;
  }

  // Identify implicit defs and uses.

  bool ImplicitDef = DefOperand.isImplicit();
  bool ImplicitUse = UseOperand.isImplicit();
  if (ImplicitDef) {
    // Do not expect implici definitions of short registers for now.
    //assert(isLongRegister(DefRegClass));
    DEBUG_WITH_TYPE("latency", dbgs() << "== Latency : implicit def\n");
  }
  if (ImplicitUse) {
    // Do not expect implici use of short registers for now.
    //assert(isLongRegister(UseRegClass));
    DEBUG_WITH_TYPE("latency", dbgs() << "== Latency : implicit use\n");
  }
  // Allow implicit use/def only as extra arguments, until we find a proper case.
  assert(ImplicitDef || DefIdx < DefMCID.getNumOperands());
  assert(ImplicitUse || UseIdx < UseMCID.getNumOperands());

  if (ImplicitDef && ImplicitUse) {
    if (DefReg == TPC::LFSR && Subtarget.hasGoyaISA()) {
      // SW-2006 Consecutive LFSR accesses should be at least 4 cycles apart - marked gen1-specific.
      DEBUG_WITH_TYPE("latency", dbgs() << "== Latency (reads from LFSR) = " << 4 << "\n");
      DEBUG_WITH_TYPE("latency", dbgs() << "=====================================================\n");
      return 4;
    }

    // If both def and use are implicit, then both def and use are subregisters
    // of a long register. The latency between these operands depends on whether
    // they refer to the same register. It will be calculated when explicit
    // use/def are evaluated, so skip the calculation for the implicit pair.
    DEBUG_WITH_TYPE("latency", dbgs() << "== Latency between implicit def and use = " << 0 << "\n");
    DEBUG_WITH_TYPE("latency", dbgs() << "=====================================================\n");
    return 0;
  }

  // Instructions LD_TNSR and ST_TNSR may have register operands that are not
  // encoded in the instruction. These are S27/S28 for tensor number, S30/S31
  // for size+offset and S29 for RMW. For all of them latency is 7.
  //
  // LD_TNSR          V1, S27, I2, SP3        LD_TNSRTvp   V1, I2, S27, 0, V1, SP3, 0
  // LD_TNSR.PARTIAL  V1, 0,   I1, S30, VP4   LD_TNSR_Pvm  V1, I2, 0,   S30, 0, V1, VP4, 0
  // LD_TNSR.PARTIAL  V1, S27, I2, S30, SP4   LD_TNSR_PTvp V1, I2, S27, S30, 0, V1, SP4, 0
  //
  // ST_TNSR.PARTIAL         0,   I1, V1, S31             ST_TNSR_Pvp   I2, 0,   V1, S31, SP0
  // ST_TNSR.RMW_SEL         S28, I1, V2, S29             ST_TNSR_RTvp  I1, S28, V2, S29, SP0
  // ST_TNSR.PARTIAL.RMW_SEL S28, I2, V3, S29, S31, SP6   ST_TNSR_PRTvp I2, S28, V3, S29, S31, SP6
  //
  if (TPCII::isLoadInst(UseMCID))
    switch (TPCII::getSlotOpCode(UseMCID)) {
    case TPCII::LD_TNSR:
    case TPCII::LD_TNSR_LOW:
    case TPCII::LD_TNSR_HIGH:
      if (UseIdx == 2) {
        assert(TPC::TnsrRegLdRegClass.hasSubClassEq(UseRegClass));
        DEBUG_WITH_TYPE("latency", dbgs() << "== Latency (S27 -> LD_TNSR) " << 7 << "\n");
        DEBUG_WITH_TYPE("latency", dbgs() << "=====================================================\n");
        return 7;
      }
      if (UseIdx == 3) {
        assert(TPC::OffsSizeRegLdRegClass.hasSubClassEq(UseRegClass));
        DEBUG_WITH_TYPE("latency", dbgs() << "== Latency (S30 -> LD_TNSR) " << 7 << "\n");
        DEBUG_WITH_TYPE("latency", dbgs() << "=====================================================\n");
        return 7;
      }
      break;
    case TPCII::ldGEN_ADDR:
      if (UseIdx == 1) {
        assert(TPC::TnsrRegLdRegClass.hasSubClassEq(UseRegClass));
        DEBUG_WITH_TYPE("latency", dbgs() << "== Latency (S27 -> GEN_ADDR) " << 7 << "\n");
        DEBUG_WITH_TYPE("latency", dbgs() << "=====================================================\n");
        return 7;
      }
      break;
    default:
      break;
    }
  if (TPCII::isStoreInst(UseMCID))
    switch (TPCII::getSlotOpCode(UseMCID)) {
    case TPCII::ST_TNSR:
    case TPCII::ST_TNSR_LOW:
    case TPCII::ST_TNSR_HIGH:
      if (UseIdx == 1) {
        assert(TPC::TnsrRegStRegClass.hasSubClassEq(UseRegClass));
        DEBUG_WITH_TYPE("latency", dbgs() << "== Latency (S28 -> ST_TNSR) " << 7 << "\n");
        DEBUG_WITH_TYPE("latency", dbgs() << "=====================================================\n");
        return 7;
      }
      if (UseIdx == 3) {
        assert(TPC::RMWRegRegClass.hasSubClassEq(UseRegClass) ||
                TPC::OffsSizeRegStRegClass.hasSubClassEq(UseRegClass));
        DEBUG_WITH_TYPE("latency", dbgs() << "== Latency (S29/S31 -> ST_TNSR) " << 7 << "\n");
        DEBUG_WITH_TYPE("latency", dbgs() << "=====================================================\n");
        return 7;
      }
      if (UseIdx == 4 && TPC::OffsSizeRegStRegClass.hasSubClassEq(UseRegClass)) {
        DEBUG_WITH_TYPE("latency", dbgs() << "== Latency (S31 -> ST_TNSR) " << 7 << "\n");
        DEBUG_WITH_TYPE("latency", dbgs() << "=====================================================\n");
        return 7;
      }
      break;
    case TPCII::stGEN_ADDR:
      if (UseIdx == 1) {
        assert(TPC::TnsrRegStRegClass.hasSubClassEq(UseRegClass));
        DEBUG_WITH_TYPE("latency", dbgs() << "== Latency (S28/HW -> GEN_ADDR) " << 7 << "\n");
        DEBUG_WITH_TYPE("latency", dbgs() << "=====================================================\n");
        return 7;
      }
      break;
    default:
      break;
    }

  if (ImplicitUse) {
    if (!isLongRegister(UseRegClass))
      return 4;

    // Find the operand that is a subregister of the implicitly used
    // long register.
    unsigned ShortRegIdx = ~0U;
    unsigned ShortReg = ~0U;
    for (unsigned I = UseMCID.getNumDefs(), E = UseMCID.getNumOperands(); I < E; ++I) {
      const MachineOperand &Operand = UseMI.getOperand(I);
      if (Operand.isReg()) {
        unsigned Reg = Operand.getReg();
        if (getRegisterInfo().isSubRegister(UseReg, Reg)) {
          ShortRegIdx = I;
          ShortReg = Reg;
          break;
        }
      }
    }
    if (ShortRegIdx >= UseMI.getNumOperands()) {
      // Example (sef = 0, use = 2):
      //
      // %V10<def> = MOVf32vv %V0, %A8<imp-use,kill>, %A8<imp-def>
      // %V11<def> = MOVf32vv %V0, %A8<imp-use>, %A8<imp-def>
      //
      // In this case no dependency exists.
      DEBUG_WITH_TYPE("latency", dbgs() << "== Latency between def and implicit use = " << 0 << "\n");
      DEBUG_WITH_TYPE("latency", dbgs() << "=====================================================\n");
      return 0;
    }

    if (isLongRegister(DefRegClass)) {
      // We can see false dependencies between long registers, like in the
      // example (def = 0, use = 8):
      //
      // %D0<def,tied2> = LOOKUP_C1C2i8vvep %V12<kill>, %D0<kill,tied0>, 7, 40, %SP0, 0
      // %V23<def, tied3> = CONVERT_INT32i32i8vvsem %V3, %S1, %V23<tied0>, 0, 3, %VP4, 0, %A0<imp - use>
      //
      if (isLongRegister(UseRegClass)) {
        // Return 0 in this case. If there are real dependency, it will be detected
        // when explicit operands are used.
        DEBUG_WITH_TYPE("latency", dbgs() << "== Latency : between long definition and implicit long use " << 0 << "\n");
        return 0;
      }

      // Example (def = 0, use = 3):
      //
      // %D0<def,tied2> = LOOKUP_C1C2i8vve %V2<kill>, %D0<kill,tied0>, 1, 1
      // ST_L_Vi8si %S1, 256, %V1, %D0<imp - use>; mem:ST256[%arrayidx(addrspace = 2)](tbaa = !3)
      //
      // In this case the latency between operand is calculated as for short
      // registers.
      DEBUG_WITH_TYPE("latency", dbgs() << "== Latency : replaced use op to " << ShortRegIdx << "\n");
      assert(ShortRegIdx < UseIdx);
      return getOperandLatency(ItinData, DefMI, DefIdx, UseMI, ShortRegIdx);
    }

    // The definition defines a register, which is a part of a long register,
    // which is implicitly used. For example (def = 0, use = 2):
    //
    // %V2<def> = MOVf32vv %V0, %A0<imp-use,kill>, %A0<imp-def>
    // %V3<def> = MOVf32vv %V0, %A0<imp-use>, %A0<imp-def>
    //
    if (DefReg == ShortReg) {
      DEBUG_WITH_TYPE("latency", dbgs() << "== Latency : replaced use op to " << ShortRegIdx << "\n");
      assert(ShortRegIdx < UseIdx);
      return getOperandLatency(ItinData, DefMI, DefIdx, UseMI, ShortRegIdx);
    }

    // Try to find another explicit register operand that represents a
    // subregister of the implicit use.
    for (unsigned I = ShortRegIdx + 1, E = UseMCID.getNumOperands(); I < E; ++I) {
      const MachineOperand &Operand = UseMI.getOperand(I);
      if (Operand.isReg()) {
        unsigned Reg = Operand.getReg();
        if (getRegisterInfo().isSubRegister(DefReg, Reg)) {
          if (Reg == DefReg) {
            DEBUG_WITH_TYPE("latency", dbgs() << "== Latency : replaced use op to " << I << "\n");
            assert(I < UseIdx);
            return getOperandLatency(ItinData, DefMI, DefIdx, UseMI, I);
          }
        }
      }
    }

    // It looks like no dependency exists (see example above).
    DEBUG_WITH_TYPE("latency", dbgs() << "== Latency between def and implicit use = " << 0 << "\n");
    DEBUG_WITH_TYPE("latency", dbgs() << "=====================================================\n");
    return 0;
  }

  if (ImplicitDef && isLongRegister(DefRegClass)) {

    // Find explicit operand that is a subregister of the implicitly defined
    // long register.
    unsigned ShortRegOp = ~0U;
    unsigned ShortReg = ~0U;
    for (unsigned I = 0, E = DefMCID.getNumDefs(); I < E; ++I) {
      const MachineOperand &Operand = DefMI.getOperand(I);
      if (Operand.isReg()) {
        unsigned Reg = Operand.getReg();
        if (getRegisterInfo().isSubRegister(DefReg, Reg)) {
          ShortRegOp = I;
          ShortReg = Reg;
          break;
        }
      }
    }

    // If producer writes to %V0 and consumer uses %D0, then the latency is the
    // same as if producer and consumer both used %V0 register.
    // Example (def = 3, use = 2):
    //
    // %V1<def> = MOVf32vv %V0, %D0<imp-use,kill>, %D0<imp-def>
    // %D0<def, tied2> = LOOKUP_C1C2i8vve %V2, %D0<tied0>, 1, 1
    //
    if (isLongRegister(UseRegClass) && getRegisterInfo().isSubRegister(ShortReg, DefReg)) {
      DEBUG_WITH_TYPE("latency", dbgs() << "== Latency : replaced def op to " << ShortRegOp << "\n");
      assert(ShortRegOp < DefIdx);
      return getOperandLatency(ItinData, DefMI, ShortRegOp, UseMI, UseIdx);
    }

    // Another case is when producer and consumer operate different subregisters
    // of the same long register, for example (def = 3, use = 2):
    //
    // %V3<def> = MOVf32vv %V0, %A0<imp-use,kill>, %A0<imp-def>
    // ST_L_Vi32si %S31, 0, %V0
    //
    if (UseReg == ShortReg) {
      DEBUG_WITH_TYPE("latency", dbgs() << "== Latency : replaced def op to " << ShortRegOp << "\n");
      assert(ShortRegOp < DefIdx);
      return getOperandLatency(ItinData, DefMI, ShortRegOp, UseMI, UseIdx);
    }

    // Try to find another explicit register operand that represents a
    // subregister of the implicit def.
    for (unsigned I = ShortRegOp + 1, E = UseMCID.getNumDefs(); I < E; ++I) {
      const MachineOperand &Operand = UseMI.getOperand(I);
      if (Operand.isReg()) {
        unsigned Reg = Operand.getReg();
        if (getRegisterInfo().isSubRegister(UseReg, Reg)) {
          if (Reg == UseReg)
            DEBUG_WITH_TYPE("latency", dbgs() << "== Latency : replaced use op to " << I << "\n");
            assert(I < UseIdx);
            return getOperandLatency(ItinData, DefMI, DefIdx, UseMI, I);
        }
      }
    }

    assert(DefIdx >= DefMCID.getNumOperands());
    DEBUG_WITH_TYPE("latency", dbgs() << "== Latency from implicit def = " << 0 << "\n");
    DEBUG_WITH_TYPE("latency", dbgs() << "=====================================================\n");
    return 0;
  }

  if (UseReg != DefReg) {
    // The registers used by definition and use may differ if one register
    // is a subregister of the other.
    if (!getRegisterInfo().isSubRegister(DefReg, UseReg) &&
        !getRegisterInfo().isSubRegister(UseReg, DefReg)) {
      // We can have such case if both register definition and its use are DRFs,
      // which both are different parts of some ARF.
      assert(DefRegClass->hasSuperClassEq(&TPC::DRFRegClass) &&
             UseRegClass->hasSuperClassEq(&TPC::DRFRegClass));
      DEBUG_WITH_TYPE("latency", dbgs() << "== Latency different DRF parts of an ARF = " << 0 << "\n");
      DEBUG_WITH_TYPE("latency", dbgs() << "=====================================================\n");
      return 0;
    }
  }

  // This is the guideline from Ron - for producers (dst), isVPUDest always false
  bool isDefVectorPipe = false;
  bool isAddSubFP16 = false;
  bool is2srfDst = false;
  bool isDefLFSRImplicitDst = false;
  bool isDefIRFDest = (DefRegClass == &TPC::IRFRegClass);
  bool IsDefFloat = !TPCII::isLoopInst(DefMCID) &&
                    !TPCII::isStoreInst(DefMCID) &&
                    isFloatData(getOpType(DefMI));
  if (IsDefFloat) {
    if (TPCII::isLoadInst(DefMCID) && !TPCII::isLookup(DefMCID)) {
      IsDefFloat = false;
    }
  }
  if (TPCII::isLookup(DefMCID)) {
      IsDefFloat = true;
  }

  if (TPCII::getSlotOpCode(DefMCID) == TPCII::spuLOOP && TPCII::isLoopInst(DefMCID)) {
    assert(ImplicitDef);// LOOP defines iterators.
    DEBUG_WITH_TYPE("latency", dbgs() << "== Latency = " << 1 << "\n");
    DEBUG_WITH_TYPE("latency", dbgs() << "=====================================================\n");
    return 1;
  }

  bool isUseLFSRImplicitDst = false;

  const TargetRegisterClass *UseDestRegClass;
  bool isUseIRFDest = false;
  bool isUseVectorPipe = false;
  const MachineOperand &UseDest = UseMI.getOperand(0);
  if (UseDest.isReg() && UseDest.isDef()) {
    if (UseDest.getReg().isPhysical()) {
      UseDestRegClass = getRegClass(UseMCID, 0, &RI, MF);
    } else {
      UseDestRegClass = MRI.getRegClass(UseDest.getReg());
    }
    isUseIRFDest = (UseDestRegClass == &TPC::IRFRegClass);
    isUseVectorPipe = (UseDestRegClass == &TPC::VRFRegClass) ||
                      (UseDestRegClass == &TPC::VPRFRegClass) ||
                      (UseDestRegClass == &TPC::ARFRegClass) ||
                      (UseDestRegClass == &TPC::DRFRegClass);
  }

  // Workaround for MOV to LFSR, which is represented by special instruction.
  // TODO: can the code above do it?
  if (UseMI.getOpcode() == TPC::WriteLFSR) {
    isUseVectorPipe = true;
  }

  // Latency between instructions in conditional chain is 0.
  if (isConditionalChain(DefMI, UseMI)) {
    DEBUG_WITH_TYPE("latency", dbgs() << "== Latency = " << 0 << "\n");
    DEBUG_WITH_TYPE("latency", dbgs() << "=====================================================\n");
    return 0;
  }

  // SET_INDX (with immediate operand) has operand 1, which is not actually an
  // operand. It is tied to the instruction result and defines the lanes in the
  // result, that are not touched by this instruction.
  if (isSET_INDX(UseMI.getOpcode()) && UseIdx == 1) {
    assert(UseMI.getOperand(1).isReg());
    assert(DefMI.getOperand(0).getReg() == UseMI.getOperand(1).getReg());
    DEBUG_WITH_TYPE("latency", dbgs() << "== Latency (IRF) = " << 0 << "\n");
    DEBUG_WITH_TYPE("latency", dbgs() << "=====================================================\n");
    return 0;
  }

#if 1
  // Process instruction which write into different lanes of the same register
  // ** PACK **
  if (TPCII::getSlotOpCode(DefMCID) == 45 /*PACK*/ && TPCII::isVPUInst(DefMCID) &&
      TPCII::getSlotOpCode(UseMCID) == 45 /*PACK*/ && TPCII::isVPUInst(UseMCID) &&
      UseIdx == 2 && DefIdx == 0 && UseMI.getOperand(UseIdx).isTied())
  {
    int DefMsk = 0;
    int UseMsk = 0;
    if (DefMI.getOperand(2).isImm()) {
      DefMsk = DefMI.getOperand(2).getImm();
    }
    else if (DefMI.getOperand(3).isImm()) {
      DefMsk = DefMI.getOperand(3).getImm();
    }
    if (UseMI.getOperand(2).isImm()) {
      UseMsk = UseMI.getOperand(2).getImm();
    }
    else if (UseMI.getOperand(3).isImm()) {
      UseMsk = UseMI.getOperand(3).getImm();
    }
    if (DefMsk && UseMsk && (0 == (DefMsk & UseMsk))) {
      DEBUG_WITH_TYPE("latency", dbgs() << "== Latency (different lanes) = " << 0 << "\n");
      DEBUG_WITH_TYPE("latency", dbgs() << "=====================================================\n");
      return 0;
    }
  }

  // ** MOV_DUAL_GROUP **
  unsigned DefDstG = 0;
  unsigned UseDstG = 0;
  unsigned DefSrcG = 0;
  unsigned UseSrcG = 0;
  if (isMovDualGroup(DefMI, &DefSrcG, &DefDstG) && isMovDualGroup(UseMI, &UseSrcG, &UseDstG)) {
    if ((DefDstG != UseDstG) && UseMI.getOperand(UseIdx).isTied()) {
      DEBUG_WITH_TYPE("latency", dbgs() << "== Latency (different dst lanes) = " << 0 << "\n");
      DEBUG_WITH_TYPE("latency", dbgs() << "=====================================================\n");
      return 0;
    }
    if ((DefIdx == 0) && (UseIdx == 1) && (DefDstG != UseSrcG)) {
      DEBUG_WITH_TYPE("latency", dbgs() << "== Latency (different src & dst lanes) = " << 0 << "\n");
      DEBUG_WITH_TYPE("latency", dbgs() << "=====================================================\n");
      return 0;
    }
  }

  // ** LOOKUP **
  if (TPCII::isLookupC(DefMCID) && TPCII::getSlotOpCode(DefMCID) == TPCII::LOOKUP_C1C2 &&
      TPCII::isLookupC(UseMCID) && TPCII::getSlotOpCode(UseMCID) == TPCII::LOOKUP_C1C2 &&
      ((DefIdx == 0 &&  ((UseIdx == 3 && UseMI.getOperand(UseIdx).isTied()) || (UseIdx == 0))) ||
      (DefIdx == 1 &&  ((UseIdx == 4 && UseMI.getOperand(UseIdx).isTied()) || (UseIdx == 1)))))
  {
    int DefMsk = 0;
    int UseMsk = 0;
    if (DefMI.getOperand(5).isImm()) {
      DefMsk = DefMI.getOperand(5).getImm();
    }
    if (UseMI.getOperand(5).isImm()) {
      UseMsk = UseMI.getOperand(5).getImm();
    }
//    if (DefMsk && UseMsk && (0 == (DefMsk & UseMsk))) {
    if (DefMsk && UseMsk && (DefMsk != UseMsk)) {
      DEBUG_WITH_TYPE("latency", dbgs() << "== Latency (different lanes) " << 0 << "\n");
      DEBUG_WITH_TYPE("latency", dbgs() << "=====================================================\n");
      return 0;
    }
  }

  // ** LOOKUP DRF/ARF **
  if (TPCII::isLookupC(DefMCID) && TPCII::getSlotOpCode(DefMCID) == TPCII::LOOKUP_C1C2 &&
      TPCII::isLookupC(UseMCID) && TPCII::getSlotOpCode(UseMCID) == TPCII::LOOKUP_C1C2 &&
      (DefIdx == 0 &&  (UseIdx == 2 && UseMI.getOperand(UseIdx).isTied())))
  {
    int DefMsk = 0;
    int UseMsk = 0;
    if (DefMI.getOperand(3).isImm()) {
      DefMsk = DefMI.getOperand(3).getImm();
    }
    if (UseMI.getOperand(3).isImm()) {
      UseMsk = UseMI.getOperand(3).getImm();
    }
    if (DefMsk && UseMsk && ((DefMsk != UseMsk))) {
      DEBUG_WITH_TYPE("latency", dbgs() << "== Latency (different lanes) " << 0 << "\n");
      DEBUG_WITH_TYPE("latency", dbgs() << "=====================================================\n");
      return 0;
    }
  }

  if (TPCII::isLookupC(DefMCID) && TPCII::getSlotOpCode(DefMCID) == TPCII::LOOKUP_C0 &&
      TPCII::isLookupC(UseMCID) && TPCII::getSlotOpCode(UseMCID) == TPCII::LOOKUP_C0 &&
      (DefIdx == 0 &&  ((UseIdx == 2 && UseMI.getOperand(UseIdx).isTied()) || (UseIdx == 0))))
  {
    int DefMsk = 0;
    int UseMsk = 0;
    if (DefMI.getOperand(3).isImm()) {
      DefMsk = DefMI.getOperand(3).getImm();
    }
    if (UseMI.getOperand(3).isImm()) {
      UseMsk = UseMI.getOperand(3).getImm();
    }
    if (DefMsk && UseMsk && (0 == (DefMsk & UseMsk))) {
      DEBUG_WITH_TYPE("latency", dbgs() << "== Latency (different lanes) = " << 0 << "\n");
      DEBUG_WITH_TYPE("latency", dbgs() << "=====================================================\n");
      return 0;
    }
  }

  // ** IRF arithmetic with DIM_MASK **
  if (DefRegClass == &TPC::IRFRegClass) {
    assert(UseRegClass == &TPC::IRFRegClass);
    unsigned DefMask = 0;
    unsigned UseMask = 0;
    if (isIRFProducerWithDimMask(DefMI, DefMask) && isIRFProducerWithDimMask(UseMI, UseMask)) {
      if (0 == (DefMask & UseMask)) {
        DEBUG_WITH_TYPE("latency", dbgs() << "== Latency (different lanes) " << 0 << "\n");
        DEBUG_WITH_TYPE("latency", dbgs() << "=====================================================\n");
        return 0;
      }
    }
  }
#endif

  auto UseOpId = TPCLatencyEvaluation::OperandID::e_dst;

  // Recognize predicate register as the operand right before the last one. The
  // last operand is predicate polarity and must be a constant.
  if (UseIdx == UseMCID.getNumOperands() - 2 && isPredicateReg(DefRegClass))
    UseOpId = TPCLatencyEvaluation::OperandID::e_src_p;
  // LD_TNSR may have additional argument (S30) before income value, if it has
  // switch PARTIAL. To avoid complication of getOperandId handle this case
  // before call to it.
  else if (TPCII::isLoadInst(UseMCID) &&
           TPCII::getSlotOpCode(UseMCID) == TPCII::LD_TNSR &&
           UseIdx == UseMCID.getNumOperands() - 3 &&
           UseMCID.getOperandConstraint(UseIdx, MCOI::TIED_TO) == 0)
    UseOpId = TPCLatencyEvaluation::OperandID::e_dst;
  else
    UseOpId = getOperandId(Subtarget, UseMI, UseRegClass, UseIdx);

  // See TPC PRM 1.3.2. Read After Write Latencies
  if (UseOpId == TPCLatencyEvaluation::OperandID::e_dst) {

// Table 5: Write-after-Write Restrictions
// ----------------------------------------------------------------------------
// First WriteInstruction          |   Second Write Instruction        | Latency
// ----------------------------------------------------------------------------
// LD_G/LD_L/LD_L_V*/LD_TNSR* to   |   Any instruction on the SPU/VPU  |   
// SPRF/VPRF                       |   that updates the same SPRF/VPRF |   3
// ----------------------------------------------------------------------------
// LD_G/LD_L/LD_L_V*/LD_TNSR* to   |   MOV on a LOAD issue slot that   |   
// SPRF/VPRF                       |   updates the same SPRF/VPRF      |   3
// ----------------------------------------------------------------------------
// UDIV_4STEP                      |   Any instruction that defines    |   6
//                                 |   the same def                    |
// ----------------------------------------------------------------------------

    int lat = 1;
    unsigned opc1 = TPCII::getSlotOpCode(DefMCID);
    unsigned opc2 = TPCII::getSlotOpCode(UseMCID);

    if (TPCII::isLoadInst(DefMCID)) {
      switch (opc1) {
       case TPCII::LD_L:
       case TPCII::LD_G:
       case TPCII::LD_L_V:
       case TPCII::LD_L_V_LOW:
       case TPCII::LD_L_V_HIGH:
       case TPCII::LD_TNSR:
       case TPCII::LD_TNSR_LOW:
       case TPCII::LD_TNSR_HIGH:
         if ( TPCII::isVPUInst(UseMCID) || TPCII::isSPUInst(UseMCID) ||
              (TPCII::isLoadInst(UseMCID) && opc2 == TPCII::ldMOV) ) {
            lat = 3;
         }
         break;
      }
    }

    if (is_convert_instr_with_lane(DefMI) && is_convert_instr_with_lane(UseMI)) {
      if (getLaneSel(DefMI) != getLaneSel(UseMI)) {
        lat = 0; // no latency between different lanes
      }
    }
    DEBUG_WITH_TYPE("latency", dbgs() << "== Latency (e_dst in Consumer) = " << lat << "\n");
    DEBUG_WITH_TYPE("latency", dbgs() << "=====================================================\n");
    return lat;
  }

  if (UseMCID.isBranch() && isPredicateReg(DefRegClass))
    UseOpId = TPCLatencyEvaluation::OperandID::e_src_p;
  bool IsUseFloat = !TPCII::isLoopInst(UseMCID) &&
                    !TPCII::isStoreInst(UseMCID) &&
                    isFloatData(getOpType(UseMI));
  if (IsUseFloat) {
    if (TPCII::isLoadInst(UseMCID) && !TPCII::isLookup(UseMCID)) {
      IsUseFloat = false;
    }
  }
  if (TPCII::isLookup(UseMCID)) {
      IsUseFloat = true;
  }

  //todo: This condition can be more optimal. This condition can be relaxed for only FP instruction with SRC_B.
  bool isDefSpuAddSub = (TPCII::isSPUInst(DefMCID) &&
                         ((TPCII::getSlotOpCode(DefMCID) == TPCII::spuSUB) ||
			  (TPCII::getSlotOpCode(DefMCID) == TPCII::spuADD)));
  bool isDefVpuAddSub = (TPCII::isVPUInst(DefMCID) &&
                         ((TPCII::getSlotOpCode(DefMCID) == TPCII::vpuSUB) ||
			  (TPCII::getSlotOpCode(DefMCID) == TPCII::vpuADD)));

  bool defIsAccFp32 = false;
  bool useIsAccFp32 = false;
  bool isUseFp16 = false;
  if (Subtarget.hasGen2Plus()) {
    // Only MAC and MUL have a AccFp32
    // For now only relevent BF16
    bool isMulMacSPU = (TPCII::isSPUInst(DefMCID) &&
                        (TPCII::getSlotOpCode(DefMCID) == TPCII::spuMUL ||
                         TPCII::getSlotOpCode(DefMCID) == TPCII::spuMAC) &&
                        getOpType(DefMI) == TPCII::OpType::BF16);
    bool isMulMacVPU = (TPCII::isVPUInst(DefMCID) &&
                        (TPCII::getSlotOpCode(DefMCID) == TPCII::vpuMUL ||
                         TPCII::getSlotOpCode(DefMCID) == TPCII::vpuMAC) &&
                        getOpType(DefMI) == TPCII::OpType::BF16);

    if (isMulMacSPU || isMulMacVPU)
      defIsAccFp32 = DefMI.getOperand(4).getImm() & TPCII::SW_ACC_FP32;

    isMulMacSPU = (TPCII::isSPUInst(UseMCID) &&
                   (TPCII::getSlotOpCode(UseMCID) == TPCII::spuMUL ||
                    TPCII::getSlotOpCode(UseMCID) == TPCII::spuMAC) &&
                   getOpType(UseMI) == TPCII::OpType::BF16);
    isMulMacVPU = (TPCII::isVPUInst(UseMCID) &&
                   (TPCII::getSlotOpCode(UseMCID) == TPCII::vpuMUL ||
                    TPCII::getSlotOpCode(UseMCID) == TPCII::vpuMAC) &&
                   getOpType(UseMI) == TPCII::OpType::BF16);

    if (isMulMacSPU || isMulMacVPU) {
      useIsAccFp32 = UseMI.getOperand(4).getImm() & TPCII::SW_ACC_FP32;
    }
  }

  TPCLatencyEvaluation::InstructionForLatencyDetermination DefData(
    convertSlot(TPCII::getInstrType(DefMCID)),
    TPCII::getSlotOpCode(DefMCID),
    TPCLatencyEvaluation::e_dst,
    IsDefFloat,
    isDefIRFDest,
    isDefVectorPipe,
    isDefLFSRImplicitDst,
    defIsAccFp32,	// isAccFp32
    0,	    // idxDst0
    is2srfDst,
    0,	// is2xLookupAddSub,
    isAddSubFP16,	// isAddSubFp16,
    false,  //isFp8
    convertReg(DefRegClass)
  );

  TPCLatencyEvaluation::InstructionForLatencyDetermination UseData(
    convertSlot(TPCII::getInstrType(UseMCID)),
    TPCII::getSlotOpCode(UseMCID),
    UseOpId,
    IsUseFloat,
    isUseIRFDest,
    isUseVectorPipe,
    isUseLFSRImplicitDst,
    useIsAccFp32,	// isAccFp32
    0,	    // idxDst0
    false,	// is2SrfDst
    0, // is2xLookupAddSub,
    isUseFp16,            // isAddSubFp16,
    false,  //isFp8
    convertReg(UseRegClass)
  );

  if (TPCLatencyEvaluation::latenciesDB.empty()) {
    if (Subtarget.hasGaudiISA()) {
      TPCLatencyEvaluation::gaudi_buildInstructionLatenciesDB();
    }
    else {
      TPCLatencyEvaluation::dali_buildInstructionLatenciesDB();
    }
  }


  int Latency;
  int tpc_generation = 1; // Dali
  
  if (Subtarget.hasGaudiISA()) {
    tpc_generation = 2;
  }

  Latency = static_cast<int>(TPCLatencyEvaluation::calculateLatency(DefData, UseData, tpc_generation));

  //
  // Fix latency value taking into account different PRM restrictions
  // In fact, the latencyDB interface function 'calculateLatency' should have
  // taken care of this, but this is not true for now.
  //
  if (tpc_generation > 1) {
    // Gaudi restrictions:
    //
    // 4 cycles after any instruction which write to Scalar Predicate,
    // ASO with VPU_ASO_OP switch can't be predicated using this predicate
    // (so we set latency = 5 if the conditions above are true)
    //
    if (TPCII::isStoreInst(UseMCID) &&
        TPCII::getSlotOpCode(UseMCID) == TPCII::ASO && UseIdx == 1)
    {
      const MachineOperand &MO = UseMI.getOperand(0); // Sw
      assert (MO.isImm());
      if (MO.getImm() & 2) { // VPU_ASO_OP switch
        DEBUG_WITH_TYPE("latency", dbgs() << "== Restriction: ASO and Scalar predicate\n");
        Latency = std::max(Latency, 5);
      }
    }
  }

  DEBUG_WITH_TYPE("latency", dbgs() << "== Latency = " << Latency << "\n");
  DEBUG_WITH_TYPE("latency", dbgs() << "=====================================================\n");

  return Latency;
}

bool TPCInstrInfo::isPredicated(const MachineInstr& MI) const {
  if (MI.getOpcode() == TPC::JMPR) {
    return true;
  }

  // Intrinsics are predicated with a predicate being the penultimate operand.
  const MCInstrDesc &MCID = MI.getDesc();
  if (MCID.getNumOperands() > 1) {
    const MachineOperand& PossiblePred = MI.getOperand(MCID.getNumOperands() - 2);
    const MachineOperand& PossiblePol = MI.getOperand(MCID.getNumOperands() - 1);
    if (!PossiblePol.isImm() || !PossiblePred.isReg())
      return false;

    Register PReg = PossiblePred.getReg();
    if (PReg == TPC::SP0 || PReg == TPC::VP0)
      return false;
    const TargetRegisterClass *RC;
    if (PReg.isPhysical()) {
      RC = getClassOfPhysicalRegister(PReg, RI);
    } else {
      // Virtual registers.
      // In this case we cannot determine if the register is SP0, so such
      // instruction is always considered predicated if the register is of
      // predicate class.
      const MachineRegisterInfo& MRI = MI.getParent()->getParent()->getRegInfo();
      RC = MRI.getRegClass(PReg);
    }
    return TPC::SPRFRegClass.hasSubClassEq(RC)
        || TPC::VPRFRegClass.hasSubClassEq(RC);
  }

  // TODO: other instructions (not intrinsics)?

  return false;
}


DFAPacketizer *TPCInstrInfo::CreateTargetScheduleState(
    const TargetSubtargetInfo &STI) const {
  const InstrItineraryData *II = STI.getInstrItineraryData();
  return static_cast<const TPCSubtarget&>(STI).createDFAPacketizer(II);
}


void TPCInstrInfo::insertNoop(int opcode, MachineBasicBlock &MBB,
                             MachineBasicBlock::iterator MI) {
  DebugLoc DL;
  BuildMI(MBB, MI, DL, get(opcode));
}

/// Insert a NOP instruction before MI instruction.
/// We insert only NOPv (VPU slot) - the packetizer
/// will not bundle it with any instruction, so that
/// we have a full VLIW NOP.
void TPCInstrInfo::insertNoop(MachineBasicBlock &MBB,
                             MachineBasicBlock::iterator MI) const {
  DebugLoc DL;
  //BuildMI(MBB, MI, DL, get(TPC::NOPld));
  //BuildMI(MBB, MI, DL, get(TPC::NOPs));
  BuildMI(MBB, MI, DL, get(TPC::NOPv));
  //BuildMI(MBB, MI, DL, get(TPC::NOPst));
}

LLVM_READONLY int getAltOpcodeLoad (uint16_t Opcode, enum TPC::Slot inSlot);
LLVM_READONLY int getAltOpcodeStore(uint16_t Opcode, enum TPC::Slot inSlot);
LLVM_READONLY int getAltOpcodeSpu  (uint16_t Opcode, enum TPC::Slot inSlot);
LLVM_READONLY int getAltOpcodeVpu  (uint16_t Opcode, enum TPC::Slot inSlot);

bool TPCInstrInfo::getOtherSlotOpcodes(MachineInstr* MI, std::vector<unsigned>& opcodes) const {
  int altOpc;
  bool found = false;
  int (*getAltOpcode)(uint16_t, TPC::Slot);
  int (*getAltOpcodeGen)(uint16_t, TPC::Slot) = nullptr;
  int (*getAltOpcodeGenEx)(uint16_t, TPC::Slot) = nullptr;
  if (TPCII::isVPUInst(MI->getDesc())) {
    getAltOpcode = llvm::TPC::getAltOpcodeVpu;
  }
  else if (TPCII::isSPUInst(MI->getDesc())) {
    getAltOpcode = llvm::TPC::getAltOpcodeSpu;
  }
  else if (TPCII::isLoadInst(MI->getDesc())) {
    getAltOpcode = llvm::TPC::getAltOpcodeLoad;
  }
  else if (TPCII::isStoreInst(MI->getDesc())) {
    getAltOpcode = llvm::TPC::getAltOpcodeStore;
  }
  else {
    return found;
  }

  TPC::Slot slots[4] = {TPC::Slot_SpuSlot, TPC::Slot_VpuSlot, TPC::Slot_LoadSlot, TPC::Slot_StoreSlot};
  for (int i = 0; i < 4; i++) {
    altOpc = getAltOpcode(MI->getOpcode(), slots[i]);
    if (altOpc >= 0 && (altOpc != (uint16_t)-1U)) {
      opcodes.push_back(altOpc);
      found = true;
    }
    if (getAltOpcodeGen != nullptr) {
      altOpc = getAltOpcodeGen(MI->getOpcode(), slots[i]);
      if (altOpc >= 0 && (altOpc != (uint16_t)-1U)) {
        opcodes.push_back(altOpc);
        found = true;
      }
    }
    if (getAltOpcodeGenEx != nullptr) {
      altOpc = getAltOpcodeGenEx(MI->getOpcode(), slots[i]);
      if (altOpc >= 0 && (altOpc != (uint16_t)-1U)) {
        opcodes.push_back(altOpc);
        found = true;
      }
    }
  }

  return found;
}

bool TPCInstrInfo::instHasImm(const MachineInstr &MI) const {
  return TPCII::getHasImm(MI.getDesc());
}

bool TPCInstrInfo::instHasImmField(const MachineInstr &MI) const {
  return TPCII::getHasImmField(MI.getDesc());
}

static bool isRealPredicateReg(unsigned Reg) {
  return (TPC::SPRFRegClass.contains(Reg) && (Reg != TPC::SP0));
}

bool TPCInstrInfo::isScalarToVector(const MachineInstr &MI) const {
    unsigned Reg;
    unsigned numOp = MI.getDesc().getNumOperands();
    if (numOp <= 1) return false;
    const MachineOperand &MO = MI.getOperand(0);
    if (!MO.isReg())
      return false;
    if (!MO.isDef())
      return false;
    Reg = MO.getReg();
    if (!TPC::VRFRegClass.contains(Reg) &&
        !TPC::VPRFRegClass.contains(Reg))
      return false;
    
    for (unsigned i = 1, e = numOp; i != e; ++i) {
      const MachineOperand &MO = MI.getOperand(i);
      if (!MO.isReg()) continue;
      Reg = MO.getReg();
      if (TPC::SRFRegClass.contains(Reg) ||
          isRealPredicateReg(Reg))
        return true;
      }
    return false;
}

bool TPCInstrInfo::hasSRFOrSPRFOperands(const MachineInstr &MI) const {
    unsigned Reg;
    unsigned numOp = MI.getDesc().getNumOperands();
    for (unsigned i = 0, e = numOp; i != e; ++i) {
      const MachineOperand &MO = MI.getOperand(i);
      if (!MO.isReg()) continue;
      Reg = MO.getReg();
      if (TPC::SRFRegClass.contains(Reg))
        return true;
      if (isRealPredicateReg(Reg))
        return true;
    }
    return false;
}

bool TPCInstrInfo::isVPUInstrWithSRF(const MachineInstr &MI) const {
    if (!TPCII::isVPUInst(MI.getDesc())) {
      return false;
    }
    unsigned Reg;
    unsigned numOp = MI.getDesc().getNumOperands();
    for (unsigned i = 0, e = numOp; i != e; ++i) {
      const MachineOperand &MO = MI.getOperand(i);
      if (!MO.isReg()) continue;
      Reg = MO.getReg();
      if (TPC::SRFRegClass.contains(Reg))
        return true;
      if (isRealPredicateReg(Reg))
        return true;
    }
    return false;
}

bool TPCInstrInfo::isMovSRFtoVInstr(const MachineInstr &MI) const {
    unsigned sopc = TPCII::getSlotOpCode(MI.getDesc());
    if (TPCII::isLoadInst(MI.getDesc()) && (sopc == TPCII::ldMOV)) {
      unsigned Reg;
      unsigned numOp = MI.getDesc().getNumOperands();
      if (numOp <= 1) return false;
      const MachineOperand &MO = MI.getOperand(0);
      if (!MO.isReg()) return false;
      Reg = MO.getReg();
      if (TPC::VRFRegClass.contains(Reg) || TPC::VPRFRegClass.contains(Reg)) {
        const MachineOperand &M1 = MI.getOperand(1);
        if (!M1.isReg()) return false;
        Reg = M1.getReg();
        if (TPC::SRFRegClass.contains(Reg)) {
          return true;
        }
      }
    }
    return false;
}

bool TPCInstrInfo::isIRFProducerWithDimMask(const MachineInstr &MI,
                                            unsigned &Mask) const {
  const llvm::MachineFunction &MF = *MI.getParent()->getParent();
  const MachineRegisterInfo &MRI = MF.getRegInfo();
  const MCInstrDesc &MCID = MI.getDesc();

  if (MCID.getNumDefs() != 1)
    return false;
  if (MI.isPseudo())  // COPY
    return false;

  // Determine destination register class.
  Register DefReg = MI.getOperand(0).getReg();
  const TargetRegisterClass *DefRegClass;
  if (DefReg.isPhysical()) {
    DefRegClass = getRegClass(MCID, 0, &RI, MF);
    if (!DefRegClass)
      DefRegClass = getClassOfPhysicalRegister(DefReg, RI);
  } else {
    DefRegClass = MRI.getRegClass(DefReg);
  }
  assert(DefRegClass);

  if (DefRegClass != &TPC::IRFRegClass)
    // It can be LD_TNSR, - it consumes IRF but does not produce such.
    return false;

  // Determine mask operand position.
  unsigned MaskOpNo = ~0U;
  if (TPCII::isLoadInst(MCID)) {
    switch (TPCII::getSlotOpCode(MCID)) {
    case TPCII::ldPRMT_INDX:
      // PRMT_INDX does not use mask.
      return false;
    case TPCII::ldSET_INDX:
      MaskOpNo = 3;
      break;
    case TPCII::LD_G:
      MaskOpNo = 2;
      break;
    case TPCII::ldMOV:
      MaskOpNo = 2;
      break;
    default:
      return false;
    }
  }
  else if (TPCII::isStoreInst(MCID)) {
    switch (TPCII::getSlotOpCode(MCID)) {
    case TPCII::stPRMT_INDX:
      // PRMT_INDX does not use mask.
      return false;
    case TPCII::stSET_INDX:
      MaskOpNo = 3;
      break;
    default:
      return false;
    }
  }
  else if (TPCII::isSPUInst(MCID)) {
    if (TPCII::getSlotOpCode(MCID) == TPCII::spuSET_INDX) {
      MaskOpNo = 3;
    } else {
      // DIM_MASK is the first immediate operand except the case when one of
      // instruction operands is immediate, in this case it is the second.
      bool HasImmOp = TPCII::getHasImm(MCID);
      unsigned NumImmFound = 0;
      for (unsigned I = MCID.getNumDefs(), E = MCID.getNumOperands(); I < E; ++I) {
        const MCOperandInfo &Op = MCID.OpInfo[I];
        if (Op.OperandType == MCOI::OperandType::OPERAND_IMMEDIATE) {
          ++NumImmFound;
          if (NumImmFound == 2 || !HasImmOp) {
            MaskOpNo = I;
            break;
          }
        }
      }
    }
  }

  if (MaskOpNo == ~0U)
    return false;

  const MachineOperand &MO = MI.getOperand(MaskOpNo);
  if (!MO.isImm())
    // It can be MRF.
    return false;
  int64_t V = MO.getImm();
  assert(V > 0 && V < (1 << 5));
  Mask = static_cast<unsigned>(V);
  return true;
}

bool TPCInstrInfo::isMovDualGroup(const MachineInstr &MI, unsigned *pSrcG, unsigned *pDstG) const {
  if (MI.getOpcode() == TPC::MOV_DUAL_GROUPm || MI.getOpcode() == TPC::MOV_DUAL_GROUPp) {
    const MachineOperand &Op = MI.getOperand(3);
    unsigned Sw = Op.getImm();

    if (Sw != TPCII::SW_MDG_TYPE_SINGLE)
      return false;

    *pSrcG = (Sw & TPCII::SW_SRC_DUAL_GROUP) >> TPCII::SW_SRC_DUAL_GROUP_SHIFT;
    *pDstG = (Sw & TPCII::SW_DST_DUAL_GROUP) >> TPCII::SW_DST_DUAL_GROUP_SHIFT;
    return true;
  }
  return false;
}

MachineInstr *TPCInstrInfo::foldMemoryOperandImpl(
    MachineFunction &MF, MachineInstr &MI, ArrayRef<unsigned> Ops,
    MachineBasicBlock::iterator InsertPt, MachineInstr &LoadMI,
    LiveIntervals *LIS) const {
  return nullptr;
}


MachineInstr *TPCInstrInfo::foldMemoryOperandImpl(
    MachineFunction &MF, MachineInstr &MI, ArrayRef<unsigned> Ops,
    MachineBasicBlock::iterator InsertPt, int FrameIndex,
    LiveIntervals *LIS, VirtRegMap *VRM) const {
  const TargetRegisterInfo &TRI = *MF.getSubtarget().getRegisterInfo();
  const MachineRegisterInfo &MRI = MF.getRegInfo();
  MachineBasicBlock &MBB = *MI.getParent();

  if (MI.isCopy()) {
    if (Ops.size() == 1 && (Ops[0] == 0 || Ops[0] == 1)) {
      bool IsSpill = Ops[0] == 0;
      const MachineOperand &DstMO = MI.getOperand(0);
      const MachineOperand &SrcMO = MI.getOperand(1);
      Register DstReg = DstMO.getReg();
      Register SrcReg = SrcMO.getReg();
      Register FoldReg = IsSpill ? DstReg : SrcReg;
      Register LiveReg = IsSpill ? SrcReg : DstReg;
      assert(FoldReg.isVirtual() && "Cannot fold physregs");
      const TargetRegisterClass *RC = MRI.getRegClass(FoldReg);
      if (LiveReg.isPhysical())
        if (!RC->contains(LiveReg)) {
          llvm_unreachable("Unrelated regclasses");
        }

      MachineInstr *SpillMI = nullptr;
      if (IsSpill) {
        storeRegToStackSlot(MBB, InsertPt, SrcReg, DstMO.isKill(), FrameIndex, RC, &TRI);
        SpillMI = &*--InsertPt;
        if (SrcMO.getSubReg() != 0) {
          SpillMI->getOperand(1).setSubReg(SrcMO.getSubReg());
        }
      } else {
        loadRegFromStackSlot(MBB, InsertPt, DstReg, FrameIndex, RC, &TRI);
        SpillMI = &*--InsertPt;
        if (DstMO.getSubReg() != 0) {
          SpillMI->getOperand(0).setSubReg(DstMO.getSubReg());
        }
      }
      return SpillMI;
    }
  }
  return nullptr;
}

const TargetRegisterClass *
TPCInstrInfo::getClassOfPhysicalRegister(Register Reg, const TargetRegisterInfo &TRI) const {
  assert(Reg.isPhysical());
  for (auto ClassIt = TRI.regclass_begin(); ClassIt != TRI.regclass_end(); ++ClassIt) {
    const TargetRegisterClass *RC = *ClassIt;
    if (RC->contains(Reg))
      return RC;
  }
  llvm_unreachable("Cannot find register class");
}

bool TPCInstrInfo::isProfitableToHoist(MachineInstr &MI) const {
  if (MI.getOpcode() == TPC::MOVvip || MI.getOpcode() == TPC::MOV_ld_vip)
    return false;

  return true;
}

bool TPCInstrInfo::isReallyTriviallyReMaterializable(const MachineInstr &MI,
                                                     AAResults *AA) const {
  unsigned NumOperands = MI.getDesc().getNumOperands();
  if (NumOperands < 3)
    return false;
  const MachineOperand &PredMO = MI.getOperand(NumOperands - 2);
  if (!PredMO.isReg())
    return false;
  return PredMO.getReg() == TPC::SP0;
}

void TPCInstrInfo::reMaterialize(MachineBasicBlock &MBB,
                                 MachineBasicBlock::iterator MI, unsigned DestReg,
                                 unsigned SubIdx, const MachineInstr &Orig,
                                 const TargetRegisterInfo &TRI) const {
  //const MachineRegisterInfo &MRI = MBB.getParent()->getRegInfo();

  //TargetInstrInfo::reMaterialize(MBB, MI, DestReg, SubIdx, Orig, TRI);
  MachineInstr *DefMI = MBB.getParent()->CloneMachineInstr(&Orig);
  DefMI->substituteRegister(DefMI->getOperand(0).getReg(), DestReg, SubIdx, TRI);
  MBB.insert(MI, DefMI);

  MachineInstr &Instr = *--MI;
  MachineOperand &MO = Instr.getOperand(0);
  assert(MO.isReg() && MO.isDef());
  if (!MO.getReg().isVirtual())
    return;
  if (MO.isTied()) {
    const MCInstrDesc &MCI = Instr.getDesc();
    if (MCI.getNumOperands() < 4)
      return;
    MachineOperand &IncomeMO = Instr.getOperand(MCI.getNumOperands() - 3);
    if (!IncomeMO.isTied())
      return;
    if (IncomeMO.getSubReg()) {
      IncomeMO.setSubReg(SubIdx);
    }
  }
}

uint64_t TPCInstrInfo::getInstImm(const MachineInstr &MI) const {
  const MCInstrDesc &MC = MI.getDesc();
  if (TPCII::getHasImm(MC)) {
    unsigned ImmPos = TPCII::getImmFieldOpNum(MC);
    const MachineOperand &MOImm = MI.getOperand(ImmPos);
    assert(MOImm.isImm() || MOImm.isFPImm() || MOImm.isMBB());
    if (MOImm.isImm()) {
      return MOImm.getImm();
    }
    else if (MOImm.isFPImm()) {
      const ConstantFP *fpImm = MOImm.getFPImm();
      APFloat apf = fpImm->getValueAPF();
      uint64_t res = apf.bitcastToAPInt().getZExtValue();
      return res;
    }
  }
  return 0;
}

bool TPCInstrInfo::isLDSTInstrWithPredicate(const MachineInstr &MI,
                                            unsigned &pred,
					    unsigned &polarity) const {
  const llvm::MachineFunction &MF = *MI.getParent()->getParent();
  const MachineRegisterInfo &MRI = MF.getRegInfo();
  const MCInstrDesc &MCID = MI.getDesc();
  if (!TPCII::isLoadInst(MCID) && !TPCII::isStoreInst(MCID)) {
    return false;
  }
  pred = 0;
  polarity = 0;

  if(MCID.getNumOperands() < 2) {
    return true;
  }

  // Predicate register is always the operand right before the last one.
  // The last operand is predicate polarity and must be a constant.
  const MachineOperand &pOp  = MI.getOperand(MCID.getNumOperands() - 2);
  const MachineOperand &ppOp = MI.getOperand(MCID.getNumOperands() - 1);

  // If the operand before the last one is not a register then we assume that
  // the instruction does not use LD/ST predicate field (using default predicate reg ang polarity).
  if(!pOp.isReg()) {
    return true;
  }
  Register pReg = pOp.getReg();
  const TargetRegisterClass *pRegClass;
  if (pReg.isPhysical()) {
    pRegClass = getClassOfPhysicalRegister(pReg, RI);
  } else {
    pRegClass = MRI.getRegClass(pReg);
  }
  assert(pRegClass);

  // If the operand before the last one is not a VP or SP register then we assume that the
  // instruction does not use LD/ST predicate field (using default predicate reg ang polarity).
  if(!isPredicateReg(pRegClass)) {
    return true;
  }

  // If the last operand is not an immediate then we assume that the instruction does not use
  // LD/ST predicate field (using default predicate reg ang polarity).
  if(!ppOp.isImm()) {
    return true;
  }

  pred = pReg;
  polarity = static_cast<unsigned>(ppOp.getImm());

  return true;
}

bool TPCInstrInfo::isGenAddr(const MachineInstr &MI) const {
  const auto Opcode = MI.getOpcode();
  return (Opcode == TPC::GEN_ADDR_st || Opcode == TPC::GEN_ADDR_ld);
}

bool llvm::TPCInstrInfo::isMMIOAccess(const MachineInstr & MI)
{
  auto HasMMIOSuffix = [&MI](unsigned SuffixOpNum) -> bool {
    unsigned SuffixValue = MI.getOperand(SuffixOpNum).getImm();
    return SuffixValue & TPCII::SW_MMIO;
  };

  if (TPCII::is_ld_l(MI.getDesc()) && HasMMIOSuffix(2))
    return true;
  else if (TPCII::is_st_l(MI.getDesc())&& HasMMIOSuffix(2))
    return true;

  return false;
}

bool TPCInstrInfo::isGlobalScalarLoad(const MachineInstr &MI) const {
  const auto &MIDesc = MI.getDesc();
  if (!TPCII::isLoadInst(MIDesc) || MI.getNumOperands() < 2)
    return false;

  const MachineOperand &DestOp = MI.getOperand(0);
  const MachineOperand &SrcOp = MI.getOperand(1);
  if (!DestOp.isReg() || !SrcOp.isReg())
    return false;
  
  return (TPC::SRFRegClass.contains(DestOp.getReg()) &&
          TPC::ADRFRegClass.contains(SrcOp.getReg()));
}

bool TPCInstrInfo::instrProducesUnspillableReg(const MachineInstr &MI) const {
  if (MI.getNumOperands() < 1)
    return false;

  const MachineOperand &DestOp = MI.getOperand(0);
  if (!DestOp.isReg() || !DestOp.isDef())
    return false;

  unsigned reg = DestOp.getReg();
  const MachineFunction *MF = MI.getParent()->getParent();
  const MachineRegisterInfo &MRI = MF->getRegInfo();
  const TargetRegisterClass *RC = MRI.getRegClass(reg);

  return (TPC::HWPCRegClass.hasSubClassEq(RC) ||
          TPC::HSRFRegClass.hasSubClassEq(RC) ||
          TPC::HSPRFRegClass.hasSubClassEq(RC));
}

// TODO duplicated by TPCMCInstrInfo::getOpType
TPCII::OpType llvm::getOpType(const MachineInstr &MI) {
  // First try to find operand that represents value type.
  const MCInstrDesc &MCInstD = MI.getDesc();
  for (unsigned I = 0, E = MCInstD.getNumOperands(); I < E; ++I) {
    const MCOperandInfo &Op = MCInstD.OpInfo[I];
    if (Op.OperandType == TPC::OPERAND_DATATYPE) {
      const MachineOperand &MOp = MI.getOperand(I);
      assert(MOp.isImm());
      assert(MOp.getImm() >= 0 && MOp.getImm() <= TPCII::OpType::Max);
      return static_cast<TPCII::OpType>(MOp.getImm());
    }
  }
  // TODO: remove this:
  return TPCII::getOpType(MCInstD);
}

bool llvm::useImmSlotForImm(const MachineInstr & I, int64_t Val)
{
  const MCInstrDesc &IDesc = I.getDesc();
  const MCOperandInfo &IInfo = IDesc.OpInfo[TPCII::getImmFieldOpNum(IDesc)];
  bool isSigned = TPCMCInstrInfo::isInstTypeSigned(IDesc, I);
  return TPCMCInstrInfo::useImmSlotForImm(IInfo, Val, isSigned);
}

bool llvm::isSET_INDX(unsigned opc) {
  return (opc == TPC::SET_INDX_ld_rp || opc == TPC::SET_INDX_spu_rp || opc == TPC::SET_INDX_st_rp ||
          opc == TPC::SET_INDX_ld_ip || opc == TPC::SET_INDX_spu_ip || opc == TPC::SET_INDX_st_ip);
}


