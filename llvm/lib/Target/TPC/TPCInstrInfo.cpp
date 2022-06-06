//===-- TPCInstrInfo.cpp - TPC Instruction Information ----------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
//
// This file contains the TPC implementation of the TargetInstrInfo class.
//
//===----------------------------------------------------------------------===//

#include "TPCInstrInfo.h"
#include "MCTargetDesc/InstructionDB.h"
#include "MCTargetDesc/TPCMCInstrInfo.h"
#include "MCTargetDesc/TPCMCTargetDesc.h"
#include "TPCMachineScheduler.h"
#include "TPCSubtarget.h"
#include "TPCTargetMachine.h"
#include "latencies.h"
#include "llvm/ADT/DenseSet.h"
#include "llvm/ADT/STLExtras.h"
#include "llvm/ADT/SmallVector.h"
#include "llvm/CodeGen/MachineFrameInfo.h"
#include "llvm/CodeGen/MachineInstrBuilder.h"
#include "llvm/CodeGen/MachineMemOperand.h"
#include "llvm/CodeGen/MachineOperand.h"
#include "llvm/CodeGen/MachineRegisterInfo.h"
#include "llvm/CodeGen/Register.h"
#include "llvm/CodeGen/RegisterScavenging.h"
#include "llvm/MC/MCRegisterInfo.h"
#include "llvm/Support/ErrorHandling.h"
#include "llvm/Support/TargetRegistry.h"

using namespace llvm;

#define DEBUG_TYPE "TPCInstrInfo"

#define GET_INSTRINFO_CTOR_DTOR
#define GET_INSTRMAP_INFO
#include "TPCGenInstrInfo.inc"
#include "TPCGenDFAPacketizer.inc"

static cl::opt<int> TPCDefaultLatency("tpc-default-latency",
    cl::Hidden, cl::ZeroOrMore, cl::init(7));

// This flag is used to enable/disable x2 combine at Machine IR
static cl::opt<bool> TPCEnableX2("tpc-enable-x2",
                                 cl::desc("Enable x2 combine at Machine IR"),
                                 cl::Hidden, cl::ZeroOrMore, cl::init(true));

// This flag is used to enable aggressive x2 combine at Machine IR
static cl::opt<bool>
    AggressiveX2Combine("aggressive-x2-combine",
                        cl::desc("Enable aggressive x2 combine at machine IR"),
                        cl::Hidden, cl::ZeroOrMore, cl::init(false));

// This flag is used to consider register pressure while combining for x2
static cl::opt<bool> CheckRegPressureX2(
    "x2-check-regpressure",
    cl::desc("Consider register pressure while combining for x2"), cl::Hidden,
    cl::ZeroOrMore, cl::init(true));

// This flag is used to allow x2 combine only if candidate MI regs are packed in
// a DRF
static cl::opt<bool> CheckVRegsPartOfOneDRFX2(
    "x2-check-drf",
    cl::desc("Combine for x2 only if candidate MI regs are packed in a DRF"),
    cl::Hidden, cl::ZeroOrMore, cl::init(true));

// This flag is used to allow reordering while combining for x2
static cl::opt<bool> AllowReorderForX2(
    "x2-allow-reorder",
    cl::desc("Allow instruction reorder while combining for x2"), cl::Hidden,
    cl::ZeroOrMore, cl::init(false));

// Pin the vtable to this file.
void TPCInstrInfo::anchor() {}

// TODO: What is the register in InstrInfo constructor for?
TPCInstrInfo::TPCInstrInfo(TPCSubtarget &ST)
    : TPCGenInstrInfo(), RI(), Subtarget(ST), m_tpcx2Util() {
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
  case TPC::SPILL_ADRF_RESTORE:
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
  case TPC::SPILL_ADRF_SAVE:
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
         MI.getOperand(1).getReg() == TPC::SPRF_TRUE &&
         MI.getOperand(2).getImm() == 0;
}


static bool isDeadBranch(const MachineInstr &MI) {
  return (MI.getOpcode() == TPC::JMPR || MI.getOpcode() == TPC::JMPA) &&
         MI.getOperand(1).getReg() == TPC::SPRF_TRUE &&
         MI.getOperand(2).getImm() != 0;
}


static bool isConditionalBranch(const MachineInstr &MI) {
  return (MI.getOpcode() == TPC::JMPR || MI.getOpcode() == TPC::JMPA) &&
         MI.getOperand(1).getReg() != TPC::SPRF_TRUE;
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

unsigned TPCInstrInfo::convertHWRegToMemAddress(unsigned HWReg) const {
  unsigned RegAddress = ~0U;
  if (Subtarget.hasDoron1ISA()) {
    switch (HWReg) {
    case TPC::SQZ_CNTR0_LO:
    case TPC::SQZ_CNTR0_HI:
    case TPC::SQZ_CNTR1_LO:
    case TPC::SQZ_CNTR1_HI:
    case TPC::SQZ_CNTR2_LO:
    case TPC::SQZ_CNTR2_HI:
    case TPC::SQZ_CNTR3_LO:
    case TPC::SQZ_CNTR3_HI:
    case TPC::SQZ_CNTR4_LO:
    case TPC::SQZ_CNTR4_HI:
    case TPC::SQZ_CNTR5_LO:
    case TPC::SQZ_CNTR5_HI:
    case TPC::SQZ_CNTR6_LO:
    case TPC::SQZ_CNTR6_HI:
    case TPC::SQZ_CNTR7_LO:
    case TPC::SQZ_CNTR7_HI:
      RegAddress = 0; // TODO: use actual address from PRM
      break;
    }
  }
  return RegAddress;
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
      .addReg(TPC::SPRF_TRUE)
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
      .addReg(TPC::SPRF_TRUE)
      .addImm(0);
  };

  const auto CopyReg = [&](unsigned Opc) {
    BuildMI(MBB, I, DL, get(Opc), DestReg)
      .addReg(SrcReg, getKillRegState(KillSrc))
      .addImm(0)  // Switches
      .addReg(DestReg, RegState::Undef)
      .addReg(TPC::SPRF_TRUE)
      .addImm(0);
  };

  const auto CopySubReg = [&](unsigned Opc, unsigned SubReg) {
    unsigned SrcSubReg = TRI.getSubReg(SrcReg, SubReg);
    unsigned DestSubReg = TRI.getSubReg(DestReg, SubReg);
    BuildMI(MBB, I, DL, get(Opc), DestSubReg)
      .addReg(SrcSubReg, getKillRegState(KillSrc))
      .addImm(0)  // Switches
      .addReg(DestReg, RegState::Undef)
      .addReg(TPC::SPRF_TRUE)
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
  bool DstHVRF = TPC::HVRFRegClass.contains(DestReg);
  bool DstSQZ = TPC::HWSqzCntrRegClass.contains(DestReg);

  bool SrcVRF = TPC::VRFRegClass.contains(SrcReg);
  bool SrcSRF = TPC::SRFRegClass.contains(SrcReg);
  bool SrcIRF = TPC::IRFRegClass.contains(SrcReg);
  bool SrcVPRF = TPC::VPRFRegClass.contains(SrcReg);
  bool SrcSPRF = TPC::SPRFRegClass.contains(SrcReg);
  bool SrcZRF = TPC::ZRFRegClass.contains(SrcReg);
  bool SrcARF = TPC::ARFRegClass.contains(SrcReg);
  bool SrcDRF = TPC::DRFRegClass.contains(SrcReg);
  bool SrcADRF = TPC::ADRFRegClass.contains(SrcReg);
  bool SrcHSRF = TPC::HSRFRegClass.contains(SrcReg);
  bool SrcSQZ = TPC::HWSqzCntrRegClass.contains(SrcReg);

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
      .addReg(TPC::SPRF_TRUE)
      .addImm(0);
    return;
  }
  if (SrcSPRF && DstSPRF) {
    CopyReg(TPC::MOVppp);
    return;
  }

  if (SrcSRF && DstHSRF) {
    // On some architectures a hardware register can be mapped to memory. It is
    // considered as a hardware register but is manipulated as memory locations.
    // Copying to such register is realized as a memory write. 
    unsigned RegAddress = convertHWRegToMemAddress(DestReg);
    if (RegAddress != ~0U) {
      MachineInstr *MI = BuildMI(MBB, I, DL, get(TPC::ST_Lisp))
        .addImm(RegAddress)
        .addReg(SrcReg, getKillRegState(KillSrc))
        .addImm(TPCII::SW_MMIO)
        .addReg(TPC::SPRF_TRUE)
        .addImm(0)
        .getInstr();
      MI->addRegisterDefined(DestReg);
      return;
    }

    if (!Subtarget.hasGrecoISA() && !Subtarget.hasGaudi2ISA() &&
        !Subtarget.hasDoron1ISA())
      report_fatal_error("Cannot copy HWReg on this platform");
    CopyReg(TPC::MOV_ld_hsp);
    return;
  }

  if (SrcHSRF && DstSRF) {
    unsigned RegAddress = convertHWRegToMemAddress(SrcReg);
    if (RegAddress != ~0U) {
      MachineInstr *MI = BuildMI(MBB, I, DL, get(TPC::LD_Lsip))
        .addImm(RegAddress)
        .addImm(TPCII::SW_MMIO)
        .addReg(TPC::SPRF_TRUE)
        .addImm(0)
        .getInstr();
      MI->addRegisterDefined(DestReg);
      return;
    }
  }
  
  if (SrcVRF && DstHVRF) {
    if (!Subtarget.hasGrecoISA() && !Subtarget.hasGaudi2ISA() &&
        !Subtarget.hasDoron1ISA())
      report_fatal_error("Cannot copy HWReg on this platform");
    CopyReg(TPC::MOV_ld_hvp);
    return;
  }

  if (SrcADRF && DstADRF) {
    if (!Subtarget.hasADRFMov())
      report_fatal_error("Cannot copy ADRF on this platform");
    CopyReg(TPC::MOVaap);
    return;
  }

  if (SrcVPRF && DstVPRF) {
    CopyReg(TPC::MOVmmp);
    return;
  }
  if (SrcVRF && DstVPRF) {
    if (!Subtarget.hasGrecoISA() && !Subtarget.hasGaudi2ISA() &&
        !Subtarget.hasDoron1ISA())
      report_fatal_error("Cannot copy VRF to VPRF on this platform");
    CopyReg(TPC::MOVmvm);
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
  if (SrcSRF && DstSPRF) {
    CopyReg(TPC::MOV_ld_psp);
    return;
  }

  RegScavenger RS;
  RS.enterBasicBlock(MBB);
  RS.forward(I);

  MachineRegisterInfo &MRI = MBB.getParent()->getRegInfo();

  if (SrcSQZ && DstSQZ) {
    Register Src1 = RS.scavengeRegister(&TPC::SRFRegClass, 0);
    RS.setRegUsed(Src1);
    Register Src2 = RS.scavengeRegister(&TPC::SRFRegClass, 0);
    RS.setRegUsed(Src2);
    
    Register SrcSub1 = TRI.getSubReg(SrcReg, TPC::sub_0);
    Register SrcSub2 = TRI.getSubReg(SrcReg, TPC::sub_1);
    
    Register DstSub1 = TRI.getSubReg(DestReg, TPC::sub_0);
    Register DstSub2 = TRI.getSubReg(DestReg, TPC::sub_1);
    
    BuildMI(MBB, I, DL, get(TPC::MOVshp), Src1)
      .addReg(SrcSub1)
      .addImm(1) // Switch
      .addReg(SrcSub1, RegState::Undef)
      .addReg(TPC::SPRF_TRUE)
      .addImm(0);
    
    BuildMI(MBB, I, DL, get(TPC::MOVshp), Src2)
      .addReg(SrcSub2)
      .addImm(1) // Switch
      .addReg(SrcSub2, RegState::Undef)
      .addReg(TPC::SPRF_TRUE)
      .addImm(0);
    
    BuildMI(MBB, I, DL, get(TPC::MOVhsp), DstSub1)
      .addReg(Src1, RegState::Kill)
      .addImm(1) // Switch
      .addReg(DstSub1, RegState::Undef)
      .addReg(TPC::SPRF_TRUE)
      .addImm(0);
    
    BuildMI(MBB, I, DL, get(TPC::MOVhsp), DstSub2)
      .addReg(Src2, RegState::Kill)
      .addImm(1) // Switch
      .addReg(DstSub2, RegState::Undef)
      .addReg(TPC::SPRF_TRUE)
      .addImm(0);
    
    return;
  }
  if (SrcSRF && DstSQZ) {
    Register DstSub1 = TRI.getSubReg(DestReg, TPC::sub_0);
    Register DstSub2 = TRI.getSubReg(DestReg, TPC::sub_1);

    BuildMI(MBB, I, DL, get(TPC::MOVhsp), DstSub1)
      .addReg(SrcReg)
      .addImm(1) // Switch
      .addReg(DstSub1, RegState::Undef)
      .addReg(TPC::SPRF_TRUE)
      .addImm(0);

    Register SRFTemp = RS.scavengeRegister(&TPC::SRFRegClass, 0);
    RS.setRegUsed(SRFTemp);

    BuildMI(MBB, I, DL, get(TPC::MOVsip), SRFTemp)
      .addImm(0)
      .addImm(TPCII::OpType::INT32)
      .addImm(0) // Switch
      .addReg(SRFTemp, RegState::Undef)
      .addReg(TPC::SPRF_TRUE)
      .addImm(0);

    BuildMI(MBB, I, DL, get(TPC::MOVhsp), DstSub2)
      .addReg(SRFTemp, RegState::Kill)
      .addImm(1) // Switch
      .addReg(DstSub2, RegState::Undef)
      .addReg(TPC::SPRF_TRUE)
      .addImm(0);

    return;
  }
  if (SrcSQZ && DstSRF) {
    Register SrcSub1 = MRI.getTargetRegisterInfo()->getSubReg(SrcReg, TPC::sub_0);

    BuildMI(MBB, I, DL, get(TPC::MOVshp), DestReg)
      .addReg(SrcSub1)
      .addImm(1) // Switch
      .addReg(SrcSub1, RegState::Undef)
      .addReg(TPC::SPRF_TRUE)
      .addImm(0);

    return;
  }

  assert(false && "Incorrect register combination in COPY");
  report_fatal_error("Cannot copy phys reg");
}

void TPCInstrInfo::
storeRegToStackSlot(MachineBasicBlock &MBB, MachineBasicBlock::iterator I,
                    Register SrcReg, bool isKill, int FrameIndex,
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
  } else if (TPC::ADRFRegClass.hasSubClassEq(RC)) {
    if (Subtarget.hasADRFMov()) {
      StoreOpCode = TPC::SPILL_ADRF_SAVE;
      StackID = TPCStackID::SLM_SPILL;
    } else {
      report_fatal_error("ADRF registers are not spillable on this processor");
    }
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
                     Register DestReg, int FrameIndex,
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
  } else if (TPC::ADRFRegClass.hasSubClassEq(RC)) {
    if (Subtarget.hasADRFMov()) {
      LoadOpCode = TPC::SPILL_ADRF_RESTORE;
      StackID = TPCStackID::SLM_SPILL;
    } else {
      report_fatal_error("ADRF registers are not spillable on this processor");
    }
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
  unsigned ZeroReg = Subtarget.getTargetLowering()->getZeroReg(); 

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
        .addReg(TPC::SPRF_TRUE)
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
        .addReg(TPC::SPRF_TRUE)
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
          .addReg(TPC::SPRF_TRUE)
          .addImm(0);
      BuildMI(MBB, MI, DL, get(TPC::SET_INDX_spu_rp), DestReg)
          .addReg(DestReg, (k == 0) ? RegState::Undef : 0)
          .addReg(sRegs[k], RegState::Kill)
          .addImm(1LL << k)
          .addImm(0)
          .addReg(TPC::SPRF_TRUE)
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
          .addReg(TPC::SPRF_TRUE)
          .addImm(0);
    } else {
      unsigned DestReg = MI.getOperand(0).getReg();
      BuildMI(MBB, MI, DL, get(TPC::LD_L_Vmsip), DestReg)
          .addReg(ZeroReg)
          .addImm(MI.getOperand(1).getImm())
        .addImm(0)
        .addReg(DestReg, RegState::Undef)
          .addReg(TPC::SPRF_TRUE)
          .addImm(0);
      if (!MBB.isLiveIn(ZeroReg)) {
        MBB.addLiveIn(ZeroReg);
      }
    }
    MBB.erase(MI);
    return true;
  case TPC::SPILL_VRF_RESTORE: {
    unsigned DestReg = MI.getOperand(0).getReg();
    unsigned Offset = MI.getOperand(1).getImm();
    if (TPC::HVRFRegClass.contains(DestReg)) {
      BitVector Available = findAvailableVRF(MI);
      unsigned vReg;
      signed reg = Available.find_first();
      if (reg != -1) {
        vReg = reg;
      }
      else  {
        llvm_unreachable("Cannot load HWReg");
      }
      unsigned DestReg = MI.getOperand(0).getReg();
      unsigned Offset = MI.getOperand(1).getImm();
      BuildMI(MBB, MI, DL, get(TPC::LD_L_Vvip), vReg)
          .addImm(Offset)
          .addImm(0)
          .addReg(vReg, RegState::Undef)
          .addReg(TPC::SPRF_TRUE)
          .addImm(0);
      BuildMI(MBB, MI, DL, get(TPC::MOV_ld_hvp), DestReg)
          .addReg(vReg, 0)
          .addImm(1)
          .addReg(DestReg, RegState::Undef)
          .addReg(TPC::SPRF_TRUE)
          .addImm(0);
    } else {
      if (Subtarget.hasAddr1()) {
        BuildMI(MBB, MI, DL, get(TPC::LD_L_Vvip), DestReg)
            .addImm(Offset)
            .addImm(0)
            .addReg(DestReg, RegState::Undef)
            .addReg(TPC::SPRF_TRUE)
            .addImm(0);
      } else {
        BuildMI(MBB, MI, DL, get(TPC::LD_L_Vvsip), MI.getOperand(0).getReg())
            .addReg(ZeroReg)
            .addImm(Offset)
            .addImm(0)
            .addReg(DestReg, RegState::Undef)
            .addReg(TPC::SPRF_TRUE)
            .addImm(0);
        if (!MBB.isLiveIn(ZeroReg)) {
          MBB.addLiveIn(ZeroReg);
        }
      }
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
        .addReg(TPC::SPRF_TRUE)
        .addImm(0);
    SubReg = TRI.getSubReg(DestReg, TPC::sub_s1);
    BuildMI(MBB, MI, DL, get(TPC::LD_Lsip), SubReg)
        .addImm(Offset + 4)
        .addImm(0)
        .addReg(SubReg, RegState::Undef)
        .addReg(TPC::SPRF_TRUE)
        .addImm(0);
    MBB.erase(MI);
    return true;
  }
  case TPC::SPILL_ADRF_RESTORE: {
    unsigned Offset = MI.getOperand(1).getImm();
    unsigned DestReg = MI.getOperand(0).getReg();

    unsigned SubReg = TRI.getSubReg(TPC::Z30, TPC::sub_s0);
    BuildMI(MBB, MI, DL, get(TPC::LD_Lsip), SubReg)
        .addImm(Offset)
        .addImm(0)
        .addReg(SubReg, RegState::Undef)
        .addReg(TPC::SPRF_TRUE)
        .addImm(0);
    SubReg = TRI.getSubReg(TPC::Z30, TPC::sub_s1);
    BuildMI(MBB, MI, DL, get(TPC::LD_Lsip), SubReg)
        .addImm(Offset + 4)
        .addImm(0)
        .addReg(SubReg, RegState::Undef)
        .addReg(TPC::SPRF_TRUE)
        .addImm(0);

    BuildMI(MBB, MI, DL, get(TPC::MOVazp), DestReg)
        .addReg(TPC::Z30)
        .addImm(0)
        .addReg(DestReg, RegState::Undef)
        .addReg(TPC::SPRF_TRUE)
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
          .addReg(TPC::SPRF_TRUE)
          .addImm(0);
      SubReg = TRI.getSubReg(DestReg, TPC::sub_1);
      BuildMI(MBB, MI, DL, get(TPC::LD_L_Vvip), SubReg)
          .addImm(Offset + 256)
          .addImm(0)
          .addReg(SubReg, RegState::Undef)
          .addReg(TPC::SPRF_TRUE)
          .addImm(0);
    } else {
      BuildMI(MBB, MI, DL, get(TPC::LD_L_Vvsip), SubReg)
          .addReg(ZeroReg)
          .addImm(Offset)
          .addImm(0)
          .addReg(SubReg, RegState::Undef)
          .addReg(TPC::SPRF_TRUE)
          .addImm(0);
      SubReg = TRI.getSubReg(DestReg, TPC::sub_1);
      BuildMI(MBB, MI, DL, get(TPC::LD_L_Vvsip), SubReg)
          .addReg(ZeroReg)
          .addImm(Offset + 256)
          .addImm(0)
          .addReg(SubReg, RegState::Undef)
          .addReg(TPC::SPRF_TRUE)
          .addImm(0);
      if (!MBB.isLiveIn(ZeroReg)) {
        MBB.addLiveIn(ZeroReg);
      }
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
          .addReg(TPC::SPRF_TRUE)
          .addImm(0);
      SubReg = TRI.getSubReg(DestReg, TPC::sub_1);
      BuildMI(MBB, MI, DL, get(TPC::LD_L_Vvip), SubReg)
          .addImm(Offset + 256)
          .addImm(0)
          .addReg(SubReg, RegState::Undef)
          .addReg(TPC::SPRF_TRUE)
          .addImm(0);
      SubReg = TRI.getSubReg(DestReg, TPC::sub_2);
      BuildMI(MBB, MI, DL, get(TPC::LD_L_Vvip), SubReg)
          .addImm(Offset + 512)
          .addImm(0)
          .addReg(SubReg, RegState::Undef)
          .addReg(TPC::SPRF_TRUE)
          .addImm(0);
      SubReg = TRI.getSubReg(DestReg, TPC::sub_3);
      BuildMI(MBB, MI, DL, get(TPC::LD_L_Vvip), SubReg)
          .addImm(Offset + 768)
          .addImm(0)
          .addReg(SubReg, RegState::Undef)
          .addReg(TPC::SPRF_TRUE)
          .addImm(0);
    } else {
      BuildMI(MBB, MI, DL, get(TPC::LD_L_Vvsip), SubReg)
          .addReg(ZeroReg)
          .addImm(Offset)
          .addImm(0)
          .addReg(SubReg, RegState::Undef)
          .addReg(TPC::SPRF_TRUE)
          .addImm(0);
      SubReg = TRI.getSubReg(DestReg, TPC::sub_1);
      BuildMI(MBB, MI, DL, get(TPC::LD_L_Vvsip), SubReg)
          .addReg(ZeroReg)
          .addImm(Offset + 256)
          .addImm(0)
          .addReg(SubReg, RegState::Undef)
          .addReg(TPC::SPRF_TRUE)
          .addImm(0);
      SubReg = TRI.getSubReg(DestReg, TPC::sub_2);
      BuildMI(MBB, MI, DL, get(TPC::LD_L_Vvsip), SubReg)
          .addReg(ZeroReg)
          .addImm(Offset + 512)
          .addImm(0)
          .addReg(SubReg, RegState::Undef)
          .addReg(TPC::SPRF_TRUE)
          .addImm(0);
      SubReg = TRI.getSubReg(DestReg, TPC::sub_3);
      BuildMI(MBB, MI, DL, get(TPC::LD_L_Vvsip), SubReg)
          .addReg(ZeroReg)
          .addImm(Offset + 768)
          .addImm(0)
          .addReg(SubReg, RegState::Undef)
          .addReg(TPC::SPRF_TRUE)
          .addImm(0);
      if (!MBB.isLiveIn(ZeroReg)) {
        MBB.addLiveIn(ZeroReg);
      }
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
        .addReg(TPC::SPRF_TRUE)
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
        .addReg(TPC::SPRF_TRUE)
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
          .addReg(TPC::SPRF_TRUE)
          .addImm(0);
      BuildMI(MBB, MI, DL, get(TPC::ST_Lisp))
          .addImm(Offset + k*4)
          .addReg(sRegs[k], RegState::Kill)
          .addImm(0)
          .addReg(TPC::SPRF_TRUE)
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
          .addReg(TPC::SPRF_TRUE)
          .addImm(0);
    } else {
      BuildMI(MBB, MI, DL, get(TPC::ST_L_Vsimp))
          .addReg(ZeroReg)
          .addImm(MI.getOperand(0).getImm())
          .addReg(MI.getOperand(1).getReg(),
                  MI.getOperand(1).isKill() ? RegState::Kill : 0)
          .addImm(0)
          .addReg(TPC::SPRF_TRUE)
          .addImm(0);
      if (!MBB.isLiveIn(ZeroReg)) {
        MBB.addLiveIn(ZeroReg);
      }
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
          .addReg(TPC::SPRF_TRUE)
          .addImm(0);
    } else {
      BuildMI(MBB, MI, DL, get(TPC::ST_L_Vsivp))
          .addReg(ZeroReg)
          .addImm(Offset)
          .addReg(MI.getOperand(1).getReg(),
                  MI.getOperand(1).isKill() ? RegState::Kill : 0)
          .addImm(0)
          .addReg(TPC::SPRF_TRUE)
          .addImm(0);
      if (!MBB.isLiveIn(ZeroReg)) {
        MBB.addLiveIn(ZeroReg);
      }
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
        .addReg(TPC::SPRF_TRUE)
        .addImm(0);
    SubReg = TRI.getSubReg(SrcReg, TPC::sub_s1);
    BuildMI(MBB, MI, DL, get(TPC::ST_Lisp))
        .addImm(Offset + 4)
        .addReg(SubReg, Flags);
    MBB.erase(MI);
    return true;
  }
  case TPC::SPILL_ADRF_SAVE: {
    unsigned Offset = MI.getOperand(0).getImm();
    unsigned Flags = MI.getOperand(1).isKill() ? RegState::Kill : 0;
    unsigned SrcReg = MI.getOperand(1).getReg();

    BuildMI(MBB, MI, DL, get(TPC::MOVzap), TPC::Z30)
        .addReg(SrcReg, Flags)
        .addImm(0)
        .addReg(TPC::Z30, RegState::Undef)
        .addReg(TPC::SPRF_TRUE)
        .addImm(0);

    unsigned SubReg = TRI.getSubReg(TPC::Z30, TPC::sub_s0);
    BuildMI(MBB, MI, DL, get(TPC::ST_Lisp))
        .addImm(Offset)
        .addReg(SubReg)
        .addImm(0)
        .addReg(TPC::SPRF_TRUE)
        .addImm(0);
    SubReg = TRI.getSubReg(TPC::Z30, TPC::sub_s1);
    BuildMI(MBB, MI, DL, get(TPC::ST_Lisp))
        .addImm(Offset + 4)
        .addReg(SubReg)
        .addImm(0)
        .addReg(TPC::SPRF_TRUE)
        .addImm(0);
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
          .addReg(TPC::SPRF_TRUE)
          .addImm(0);
      SubReg = TRI.getSubReg(SrcReg, TPC::sub_1);
      BuildMI(MBB, MI, DL, get(TPC::ST_L_Vivp))
        .addImm(Offset + 256)
        .addReg(SubReg, Flags)
        .addImm(0)
        .addReg(TPC::SPRF_TRUE)
        .addImm(0);
    } else {
      BuildMI(MBB, MI, DL, get(TPC::ST_L_Vsivp))
        .addReg(ZeroReg)
        .addImm(Offset)
        .addReg(SubReg, Flags)
        .addImm(0)
        .addReg(TPC::SPRF_TRUE)
        .addImm(0);
      SubReg = TRI.getSubReg(SrcReg, TPC::sub_1);
      BuildMI(MBB, MI, DL, get(TPC::ST_L_Vsivp))
        .addReg(ZeroReg)
        .addImm(Offset + 256)
        .addReg(SubReg, Flags)
        .addImm(0)
        .addReg(TPC::SPRF_TRUE)
        .addImm(0);
      if (!MBB.isLiveIn(ZeroReg)) {
        MBB.addLiveIn(ZeroReg);
      }
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
          .addReg(TPC::SPRF_TRUE)
          .addImm(0);
      SubReg = TRI.getSubReg(SrcReg, TPC::sub_1);
      BuildMI(MBB, MI, DL, get(TPC::ST_L_Vivp))
          .addImm(Offset + 256)
          .addReg(SubReg, Flags)
          .addImm(0)
          .addReg(TPC::SPRF_TRUE)
          .addImm(0);
      SubReg = TRI.getSubReg(SrcReg, TPC::sub_2);
      BuildMI(MBB, MI, DL, get(TPC::ST_L_Vivp))
          .addImm(Offset + 512)
          .addReg(SubReg, Flags)
          .addImm(0)
          .addReg(TPC::SPRF_TRUE)
          .addImm(0);
      SubReg = TRI.getSubReg(SrcReg, TPC::sub_3);
      BuildMI(MBB, MI, DL, get(TPC::ST_L_Vivp))
          .addImm(Offset + 768)
          .addReg(SubReg, Flags)
          .addImm(0)
          .addReg(TPC::SPRF_TRUE)
          .addImm(0);
    } else {
      BuildMI(MBB, MI, DL, get(TPC::ST_L_Vsivp))
          .addReg(ZeroReg)
          .addImm(Offset)
          .addReg(SubReg, Flags)
          .addImm(0)
          .addReg(TPC::SPRF_TRUE)
          .addImm(0);
      SubReg = TRI.getSubReg(SrcReg, TPC::sub_1);
      BuildMI(MBB, MI, DL, get(TPC::ST_L_Vsivp))
          .addReg(ZeroReg)
          .addImm(Offset + 256)
          .addReg(SubReg, Flags)
          .addImm(0)
          .addReg(TPC::SPRF_TRUE)
          .addImm(0);
      SubReg = TRI.getSubReg(SrcReg, TPC::sub_2);
      BuildMI(MBB, MI, DL, get(TPC::ST_L_Vsivp))
          .addReg(ZeroReg)
          .addImm(Offset + 512)
          .addReg(SubReg, Flags)
          .addImm(0)
          .addReg(TPC::SPRF_TRUE)
          .addImm(0);
      SubReg = TRI.getSubReg(SrcReg, TPC::sub_3);
      BuildMI(MBB, MI, DL, get(TPC::ST_L_Vsivp))
          .addReg(ZeroReg)
          .addImm(Offset + 768)
          .addReg(SubReg, Flags)
          .addImm(0)
          .addReg(TPC::SPRF_TRUE)
          .addImm(0);
      if (!MBB.isLiveIn(ZeroReg)) {
        MBB.addLiveIn(ZeroReg);
      }
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
  case TPCII::OpType::FP16:
  case TPCII::OpType::FP32:
  case TPCII::OpType::FP8_143:
  case TPCII::OpType::FP8_152:
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
    // The second operand duplicate the first operand
    if (MC.getOpcode() == TPC::LD_CALC_FP_SPECIALOneArgvvp ||
        MC.getOpcode() == TPC::LD_CALC_FP_SPECIALOneArgvvm ||
        MC.getOpcode() == TPC::CALC_FP_SPECIALOneArgvvp ||
        MC.getOpcode() == TPC::CALC_FP_SPECIALOneArgvvm)
      return TPCLatencyEvaluation::OperandID::e_src_a;

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
        if (MC.getOpcode() == TPC::GEN_ADDR_st || MC.getOpcode() == TPC::GEN_ADDR_stT) {
          return TPCLatencyEvaluation::OperandID::e_src_a;
        }
        // Check for Gaudi's ST_TNSR, where the second operand is hidden
        if (Subtarget.hasGaudiISA() || Subtarget.hasGaudiBISA() ||
            Subtarget.hasGrecoISA() || Subtarget.hasGaudi2ISA() ||
            Subtarget.hasDoron1ISA()) {
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
        MC.getOpcode() == TPC::GEN_ADDR_ld ||
        MC.getOpcode() == TPC::GEN_ADDR_ldT) {
      return TPCLatencyEvaluation::OperandID::e_src_a;
    }
    return TPCLatencyEvaluation::OperandID::e_src_b;

  case 2:
    if (MC.getOpcode() == TPC::MOVsqz)
      return TPCLatencyEvaluation::OperandID::e_src_a;
    if (MC.getOpcode() == TPC::SET_INDX_st_rp ||
        MC.getOpcode() == TPC::SET_INDX_st_ip)
      return TPCLatencyEvaluation::OperandID::e_src_a;
    if (MC.getOpcode() == TPC::FCLASS_LIMITvvvvp ||
        MC.getOpcode() == TPC::FCLASS_LIMITvsvvp ||
        MC.getOpcode() == TPC::FCLASS_LIMITvvsvp ||
        MC.getOpcode() == TPC::FCLASS_LIMITvvvsp ||
        MC.getOpcode() == TPC::FCLASS_LIMITvvvvm ||
        MC.getOpcode() == TPC::FCLASS_LIMITvsvvm ||
        MC.getOpcode() == TPC::FCLASS_LIMITvvsvm ||
        MC.getOpcode() == TPC::FCLASS_LIMITvvvsm)
      return TPCLatencyEvaluation::OperandID::e_src_d;
    if (MC.getOpcode() == TPC::MULx2f32vvvp ||
        MC.getOpcode() == TPC::MULx2f32svvp ||
        MC.getOpcode() == TPC::MULx2f32vsvp ||
        MC.getOpcode() == TPC::MULx2f32vvsp ||
        MC.getOpcode() == TPC::MULx2f32vssp ||
        MC.getOpcode() == TPC::MULx2f32ivvp ||
        MC.getOpcode() == TPC::MULx2f32vivp ||
        MC.getOpcode() == TPC::MULx2f32vvip ||
        MC.getOpcode() == TPC::MULx2f32viip ||
        
        MC.getOpcode() == TPC::MULx2f32vvvm ||
        MC.getOpcode() == TPC::MULx2f32svvm ||
        MC.getOpcode() == TPC::MULx2f32vsvm ||
        MC.getOpcode() == TPC::MULx2f32vvsm ||
        MC.getOpcode() == TPC::MULx2f32vssm ||
        MC.getOpcode() == TPC::MULx2f32ivvm ||
        MC.getOpcode() == TPC::MULx2f32vivm ||
        MC.getOpcode() == TPC::MULx2f32vvim ||
        MC.getOpcode() == TPC::MULx2f32viim)
      return TPCLatencyEvaluation::OperandID::e_src_d;
    if (MC.getOpcode() == TPC::ADDx2vvvp ||
        MC.getOpcode() == TPC::ADDx2vsvp ||
        MC.getOpcode() == TPC::ADDx2vvsp ||
        MC.getOpcode() == TPC::ADDx2vssp ||
        MC.getOpcode() == TPC::ADDx2vivp ||
        MC.getOpcode() == TPC::ADDx2vvip ||
        MC.getOpcode() == TPC::ADDx2viip ||
        
        MC.getOpcode() == TPC::ADDx2vvvm ||
        MC.getOpcode() == TPC::ADDx2vsvm ||
        MC.getOpcode() == TPC::ADDx2vvsm ||
        MC.getOpcode() == TPC::ADDx2vssm ||
        MC.getOpcode() == TPC::ADDx2vivm ||
        MC.getOpcode() == TPC::ADDx2vvim ||
        MC.getOpcode() == TPC::ADDx2viim)
    return TPCLatencyEvaluation::OperandID::e_src_d;
    if (MC.getOpcode() == TPC::SUBx2vvvp ||
        MC.getOpcode() == TPC::SUBx2vsvp ||
        MC.getOpcode() == TPC::SUBx2vvsp ||
        MC.getOpcode() == TPC::SUBx2vssp ||
        MC.getOpcode() == TPC::SUBx2vivp ||
        MC.getOpcode() == TPC::SUBx2vvip ||
        MC.getOpcode() == TPC::SUBx2viip ||
        
        MC.getOpcode() == TPC::SUBx2vvvm ||
        MC.getOpcode() == TPC::SUBx2vsvm ||
        MC.getOpcode() == TPC::SUBx2vvsm ||
        MC.getOpcode() == TPC::SUBx2vssm ||
        MC.getOpcode() == TPC::SUBx2vivm ||
        MC.getOpcode() == TPC::SUBx2vvim ||
        MC.getOpcode() == TPC::SUBx2viim)
      return TPCLatencyEvaluation::OperandID::e_src_d;

    return TPCLatencyEvaluation::OperandID::e_src_c;
  case 3:
    if (TPCII::getSlotOpCode(MC) == TPCII::ST_TNSR_SQZ)
      return TPCLatencyEvaluation::OperandID::e_src_c;
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

  // dest, income, ..., pred, polarity
  if (UseMI.getNumDefs() != 1 || UseMI.getNumOperands() < 4) 
    return false;
  if (DefMI.getNumDefs() != 1 || DefMI.getNumOperands() < 4)
    return false;

  // Predicate and Polarity.
  bool DefPolarity; int DefPredIdx;
  if (!looksLikeHavingAPredicate(DefMI, DefPredIdx, DefPolarity))
    return false;
  bool UsePolarity; int UsePredIdx;
  if (!looksLikeHavingAPredicate(UseMI, UsePredIdx, UsePolarity))
    return false;

  // Polarities must be opposite.
  if (UsePolarity == DefPolarity)
    return false;

  const MachineOperand &DefPredicate = DefMI.getOperand(DefPredIdx);
  const MachineOperand &UsePredicate = UseMI.getOperand(UsePredIdx);

  // Predicate must be the same.
  if (DefPredicate.isKill())
    return false;
  if (DefPredicate.getReg() != UsePredicate.getReg())
    return false;

  bool UseFound = false;
  bool DefFound = false;
  for (const MachineOperand &U : MRI.use_operands(DefPredicate.getReg())) {
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
  int IncomeIdx = getIncome(UseMI);
  assert(IncomeIdx > 0 && "Def under predicate must have income");
  const MachineOperand &Income = UseMI.getOperand(IncomeIdx);
  if (!Income.isReg() || DefMI.getOperand(0).getReg() != Income.getReg())
    return false;

  LLVM_DEBUG(dbgs() << "Conditional chain recognized:"
                    << "    "; DefMI.dump();
             dbgs() << "    "; UseMI.dump(););
  return true;
}

static bool IsMov(const MachineInstr &MI) {
  const MCInstrDesc &Desc = MI.getDesc();
  unsigned Opcode = TPCII::getSlotOpCode(Desc);
  if (TPCII::isLoadInst(Desc) && Opcode == TPCII::ldMOV)
    return true;
  else if (TPCII::isSPUInst(Desc) && Opcode == TPCII::spuMOV)
    return true;
  else if (TPCII::isVPUInst(Desc) && Opcode == TPCII::vpuMOV)
    return true;
  else
    return false;
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

  const llvm::MachineFunction *CurrMF = nullptr;
  if (TPCInstrInfo::MachineFunctionX2 != nullptr) {
    CurrMF = TPCInstrInfo::MachineFunctionX2;
  } else {
    CurrMF = DefMI.getParent()->getParent();
  }
  const llvm::MachineFunction &MF = *CurrMF;
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
      DefRegClass = getRegisterClass(DefReg, MRI);
  } else {
    DefRegClass = MRI.getRegClass(DefReg);
  }
  assert(DefRegClass);

  Register UseReg = UseOperand.getReg();
  const TargetRegisterClass *UseRegClass;
  if (UseReg.isPhysical()) {
    UseRegClass = getRegClass(UseMCID, UseIdx, &RI, MF);
    if (!UseRegClass)
      UseRegClass = getRegisterClass(UseReg, MRI);
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

// Table 4: Read-after-Write Restrictions (With Hardware registers)
// ----------------------------------------------------------------------------
// MOV TO_HW_REG IRF_DIM_MASK[0-3] |   Any instruction that uses       |   3
//                                 |   IRF_DIM_MASK[0-3](Greco/Gaudi2) |
// ----------------------------------------------------------------------------
// MOV TO_HW_REG: VCARRY, ZP_REG,  |   ADD/MAC that uses this HW_REG   |   1
// SCARRY                          |                                   | gen4+
//                                 |                                   |   2
//                                 |                                   | greco
// ----------------------------------------------------------------------------
// MOV TO_HW_REG: LD_PARTIAL_REG,  |   LD_TNSR*/ST_TNSR* that uses     |   2
// ST_PARTIAL_REG, LD_TNSR_ID_REG, |   this HW_REG(Greco/Gaudi2)       |
// ST_TNSR_ID_REG, ST_RMW_REG      |                                   |
// ----------------------------------------------------------------------------
// MOV TO_HW_REG: SQZ_CNTR[0-7]    |   ST_TNSR_SQZ/ST_TNSR_S that uses |   1
//                                 |   this HW_REG(Gaudi2)             |
// ----------------------------------------------------------------------------

// This code work incorrect in the simulator.
// It needs two latency.
// mov  ZP_REG, V1, SP0; 	nop; 	nop; 	nop
// nop; 	nop; 	mac.i8  ZP_REG A8, V2, V0, VP1; 	nop
  if (IsMov(DefMI)) {
    if ((Subtarget.hasGrecoISA() || Subtarget.hasGaudi2ISA()) &&
        DefRegClass->hasSuperClass(&TPC::MRFRegClass))
      return 3;
    else if ((Subtarget.hasGrecoISA() || Subtarget.hasGaudi2ISA()) &&
             (DefReg == TPC::LD_PARTIAL_REG ||
              DefReg == TPC::ST_PARTIAL_REG ||
              DefReg == TPC::LD_TNSR_ID_REG ||
              DefReg == TPC::ST_TNSR_ID_REG ||
              DefReg == TPC::ST_RMW_REG) &&
             (TPCII::isLdTnsr(UseMCID, Subtarget.hasGen3Plus()) ||
              TPCII::isStTnsr(UseMCID)))
      return 2;
    else if (Subtarget.hasGen3Plus() &&
             ((TPCII::isSPUInst(UseMCID) &&
               DefReg == TPC::S_CARRY &&
               (TPCII::getSlotOpCode(UseMCID) == TPCII::spuADD ||
                TPCII::getSlotOpCode(UseMCID) == TPCII::spuMAC)) ||
              (TPCII::isVPUInst(UseMCID) &&
               (DefReg == TPC::V_CARRY || DefReg == TPC::ZP_REG) &&
               (TPCII::getSlotOpCode(UseMCID) == TPCII::vpuADD ||
                TPCII::getSlotOpCode(UseMCID) == TPCII::vpuMAC))))
      return 2;
      // FIXME: Uncomment after above comment fix in the simulator.
      //return Subtarget.hasGrecoISA() ? 2 : 1;
    else if (Subtarget.hasGaudi2ISA() &&
             UseRegClass == &TPC::HWSqzCntrRegClass &&
             TPCII::isStoreInst(UseMCID) &&
             (TPCII::getSlotOpCode(UseMCID) == TPCII::ST_TNSR_SQZ ||
              TPCII::getSlotOpCode(UseMCID) == TPCII::ST_TNSR_S))
      return 1;
  }

  // From Hilla Ben Yaacov
  // You can do consecutive ST_TNSR_SQZ cycle after cycle, even if they are
  // using the same squeeze counter.
  // MOV and ST_TNSR_SQZ - I think we can do them one after the other without
  // any instruction between them (latebcy=1).
  if (DefRegClass == &TPC::HWSqzCntrRegClass &&
      UseRegClass == &TPC::HWSqzCntrRegClass) {
    DEBUG_WITH_TYPE("latency", dbgs() << "== Latency between HWSqzCntr and HWSqzCntr = " << 1 << "\n");
    DEBUG_WITH_TYPE("latency", dbgs() << "=====================================================\n");
    return 1;
  }

  // From Hilla Ben Yaacov
  // There is no latency for LFSR/SPU_LFSR - it is reseeded in the same cycle
  // as it is read, and then ready with a new value in the next cycle.
  // Same also for MOV to LFSR - it has 1 cycle latency.
  if(DefRegClass->hasSuperClassEq(&TPC::HWSpuLFSRRegClass) ||
     DefRegClass->hasSuperClassEq(&TPC::HWVpuLFSRRegClass)) {
    DEBUG_WITH_TYPE("latency", dbgs() << "== Latency HW LFSR = " <<  1 << "\n");
    DEBUG_WITH_TYPE("latency", dbgs() << "=====================================================\n");
    return 1;
  }

  // Latency DB interface currently does not work for HW registers, such as DIM_MASK_REG.
  // Assume the latency is 4 for now.
  if (DefRegClass->hasSuperClassEq(&TPC::HWSqzCntrSubRegClass) ||
      DefRegClass->hasSuperClassEq(&TPC::HWSqzCntrRegClass) ||
      DefRegClass->hasSuperClassEq(&TPC::HWLaneIdRegClass)) {
    DEBUG_WITH_TYPE("latency", dbgs() << "== Latency HWSqzCntr = " << 4 << "\n");
    DEBUG_WITH_TYPE("latency", dbgs() << "=====================================================\n");
    return 4;
  }

  // Nowadays any HW register can be only physical
  if (DefReg.isPhysical() && TPC::HSRFRegClass.contains(DefReg)) {
    // TPCLatencyResolver may be trigered on use-use or def-def a HW register.
    // In this case latency 1 is being returned.
    int Latency = 1;
    if (DefOperand.isDef() && UseOperand.isDef()) {
      switch(DefReg) {
      default:
        Latency = 4;
        break;
      case TPC::LD_PARTIAL_REG:
      case TPC::ST_PARTIAL_REG:
      case TPC::LD_TNSR_ID_REG:
      case TPC::ST_TNSR_ID_REG:
      case TPC::ST_RMW_REG:
        assert(Subtarget.hasGen3Plus());
        Latency = 2;
        break;
      }
    }
    DEBUG_WITH_TYPE("latency", dbgs() << "== Latency HWReg = " <<
                    Latency << "\n");
    DEBUG_WITH_TYPE("latency", dbgs() <<
                    "=====================================================\n");
    return Latency;
  }

  if (DefRegClass->hasSuperClassEq(&TPC::HVRFRegClass)) {
    assert(UseRegClass->hasSuperClassEq(&TPC::HVRFRegClass));
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

  // From Hilla Ben Yaakov's communication on 2022-02-23:
  // the latency between an ADRF update in autoincrementing instruction (LD_G,
  // ST_G, PREFETCH) and a use of the address in subsequent LD_G/ST_G/PREFETCH
  // s 1, - i.e. you can perform back to back LD_G/ST_G/PREFETCH instructions
  // with AUTO_INC, and always get the latest value of ADRF.
  if (ImplicitDef && (DefRegClass == &TPC::ADRFRegClass)) {
    DEBUG_WITH_TYPE("latency", dbgs() << "== Latency between implicit ADRF def and use = " << 1 << "\n");
    DEBUG_WITH_TYPE("latency", dbgs() << "=====================================================\n");
    return 1;
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
      if (Subtarget.hasGaudiISA() || Subtarget.hasGaudiBISA()) {
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
      }
      break;
    case TPCII::ldGEN_ADDR:
      if (UseIdx == 1) {
        //assert(TPC::TnsrRegLdRegClass.hasSubClassEq(UseRegClass));
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
      if (Subtarget.hasGaudiISA() || Subtarget.hasGaudiBISA()) {
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
      }
      break;
    case TPCII::stGEN_ADDR:
      if (UseIdx == 1) {
        assert(TPC::TnsrRegStRegClass.hasSubClassEq(UseRegClass));
        DEBUG_WITH_TYPE("latency", dbgs() << "== Latency (S28 -> GEN_ADDR) " << 7 << "\n");
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
  bool IsDefFP16 = false;
  bool IsDefFp8 = false;

  const auto &DetectFp16Fp8 = [](const MachineInstr &Instr,
                                 bool &IsFp16,
                                 bool &IsFp8){
    TPCII::OpType Type = getOpType(Instr);
    IsFp16 = Type == TPCII::OpType::FP16;
    IsFp8 = (Type == TPCII::OpType::FP8_143 ||
             Type == TPCII::OpType::FP8_152);
  };

  DetectFp16Fp8(DefMI, IsDefFP16, IsDefFp8);

  bool Is2srfDst = false;
  bool isDefLFSRImplicitDst = false;
  bool isDefIRFDest = (DefRegClass == &TPC::IRFRegClass);
  bool IsDefFloat = !TPCII::isLoopInst(DefMCID) &&
                    isFloatData(getOpType(DefMI));
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
                      (UseDestRegClass == &TPC::HVRFRegClass) ||
                      (UseDestRegClass == &TPC::DRFRegClass) ||
                      (UseDestRegClass == &TPC::HWZPRegRegClass) ||
                      (UseDestRegClass == &TPC::HWVCarryRegClass);
  }

  // Workaround for MOV to LFSR, which is represented by special instruction.
  // TODO: can the code above do it?
  if (UseMI.getOpcode() == TPC::WriteLFSRp ||
      UseMI.getOpcode() == TPC::WriteLFSRm) {
    isUseVectorPipe = true;
  }

  // Skip extra arguments
  auto IsStTnsrSQZ = [](unsigned Opcode) {
    return Opcode == TPC::ST_TNSR_SQZ ||
        Opcode == TPC::ST_TNSR_SQZ_T;
  };
  auto IsStTnsrSQZRWM = [](unsigned Opcode) {
    return Opcode == TPC::ST_TNSR_SQZ_R ||
        Opcode == TPC::ST_TNSR_SQZ_R_T;
  };

  if ((IsStTnsrSQZ(UseMCID.getOpcode()) && UseIdx == 7) ||
      (IsStTnsrSQZ(DefMCID.getOpcode()) && DefIdx == 7) ||
      (IsStTnsrSQZRWM(UseMCID.getOpcode()) && UseIdx == 8) ||
      (IsStTnsrSQZRWM(DefMCID.getOpcode()) && DefIdx == 8)) {
    DEBUG_WITH_TYPE("latency", dbgs() << "== Latency for extra argument for ST_TNSR_SQZ = " << 0 << "\n");
    DEBUG_WITH_TYPE("latency", dbgs() << "=====================================================\n");
    return 0;
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
    int Lat = 1;
    unsigned Opc1 = TPCII::getSlotOpCode(DefMCID);
    unsigned Opc2 = TPCII::getSlotOpCode(UseMCID);

    if (TPCII::isLoadInst(DefMCID)) {
      switch (Opc1) {
       case TPCII::LD_L:
       case TPCII::LD_G:
       case TPCII::LD_L_V:
       case TPCII::LD_L_V_LOW:
       case TPCII::LD_L_V_HIGH:
       case TPCII::LD_TNSR:
       case TPCII::LD_TNSR_LOW:
       case TPCII::LD_TNSR_HIGH:
         if ( TPCII::isVPUInst(UseMCID) || TPCII::isSPUInst(UseMCID) ||
              (TPCII::isLoadInst(UseMCID) && Opc2 == TPCII::ldMOV) ) {
            Lat = 3;
         }
         break;
      }
    }

    if (is_convert_instr_with_lane(DefMI) && is_convert_instr_with_lane(UseMI)) {
      if (getLaneSel(DefMI) != getLaneSel(UseMI)) {
        Lat = 0; // no latency between different lanes
      }
    }

    DEBUG_WITH_TYPE("latency", dbgs() << "== Latency (e_dst in Consumer) = " << Lat << "\n");
    DEBUG_WITH_TYPE("latency", dbgs() << "=====================================================\n");
    return Lat;
  }
  
  // Dedicated SRF registers for loops are not permitted to be changed inside
  // the loop by the scalar pipe – only the PCU HW can manipulate them.
  if (TPCII::isLoopInst(UseMCID) && Subtarget.isHWLoopReg(DefReg)) {
    if (TPCII::isSPUInst(DefMCID) &&
        TPCII::getSlotOpCode(DefMCID) == TPCII::spuUDIV)
      return 10;
    else
      return 5;
  }
  

  bool IsDnorm =
      Subtarget.getTargetLowering()->getTargetMachine().Options.TpcDnorm;

  if (IsDnorm) {
    if (TPCII::isFMA(DefMCID))
      return 6;
    if (TPCII::isFMA(UseMCID))
      return 6;
  }

  if (UseMCID.isBranch() && isPredicateReg(DefRegClass))
    UseOpId = TPCLatencyEvaluation::OperandID::e_src_p;
  bool IsUseFloat = !TPCII::isLoopInst(UseMCID) &&
                    isFloatData(getOpType(UseMI));
  if (TPCII::isLookup(UseMCID)) {
      IsUseFloat = true;
  }
  
  if (TPCII::isSPUInst(DefMCID) &&
      TPCII::getSlotOpCode(DefMCID) == TPCII::spuUDIV) { // UDIV_4STEP too
    if (Subtarget.hasGrecoISA()) {
      Is2srfDst = true;
    } else if (Subtarget.hasGaudi2ISA() || Subtarget.hasDoron1ISA()) {
      Is2srfDst = getSwitches(DefMI) == TPCII::SW_DIV_MODE_BOTH;
    }
  }

  bool defIsAccFp32 = false;
  bool useIsAccFp32 = false;
  bool IsUseFp16 = false;
  bool IsUseFp8 = false;
  DetectFp16Fp8(UseMI, IsUseFp16, IsUseFp8);

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

    if (isMulMacSPU || isMulMacVPU)
      useIsAccFp32 = UseMI.getOperand(4).getImm() & TPCII::SW_ACC_FP32;
  }

  auto HasX2 = [](const MachineInstr &Instr)-> bool {
    const MCInstrDesc &Desc = Instr.getDesc();
    return TPCII::isVPUInst(Desc) &&
           ((TPCII::getSlotOpCode(Desc) == TPCII::vpuSUB ||
             TPCII::getSlotOpCode(Desc) == TPCII::vpuADD) &&
            (getSwitches(Instr) & TPCII::SW_X2_ARITHMETIC));
  };

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
    Is2srfDst,
    HasX2(DefMI),	// is2xLookupAddSub,
    IsDefFP16,	// isFp16,
    IsDefFp8,  //isFp8
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
    UseOpId == TPCLatencyEvaluation::OperandID::e_src_p ? 0 : HasX2(UseMI),	// is2xLookupAddSub,
    UseOpId == TPCLatencyEvaluation::OperandID::e_src_p ? false : IsUseFp16, // isFp16,
    UseOpId == TPCLatencyEvaluation::OperandID::e_src_p ? false : IsUseFp8, //isFp8
    convertReg(UseRegClass)
  );

  if (TPCLatencyEvaluation::latenciesDB.empty()) {
    if (Subtarget.hasGaudiISA() || Subtarget.hasGaudiBISA()) {
      TPCLatencyEvaluation::gaudi_buildInstructionLatenciesDB();
    } else if (Subtarget.hasDoron1ISA()) {
      TPCLatencyEvaluation::doron1_buildInstructionLatenciesDB();
    } else if (Subtarget.hasGaudi2ISA()) {
      TPCLatencyEvaluation::gaudi2_buildInstructionLatenciesDB();
    } else if (Subtarget.hasGrecoISA()) {
      TPCLatencyEvaluation::goya2_buildInstructionLatenciesDB();
    } else {
      TPCLatencyEvaluation::dali_buildInstructionLatenciesDB();
    }
  }


  int Latency = 0;
  int tpc_generation = 1; // Dali
  
  if (Subtarget.hasGaudiISA() || Subtarget.hasGaudiBISA()) {
    tpc_generation = 2;
  } else if (Subtarget.hasGrecoISA()) {
    tpc_generation = 3;
  } else if (Subtarget.hasGaudi2ISA()) {
    tpc_generation = 4;
  } else if (Subtarget.hasDoron1ISA()) {
    tpc_generation = 6;
  }

  auto FixRegFile = [](const MCInstrDesc &Desc,
      TPCLatencyEvaluation::OperandID Operand,
      TPCLatencyEvaluation::RegisterFile &file) {
    if (Operand == TPCLatencyEvaluation::e_dst) {
      switch (Desc.getOpcode()) {
      case TPC::LD_Gg4_INCsap:
      case TPC::LD_Gg5_INCzap:
      case TPC::LD_G_INCsap:
        file = TPCLatencyEvaluation::e_rf_s;
        break;
      case TPC::LD_Gg4_INCpap:
      case TPC::LD_G_INCpap:
        file = TPCLatencyEvaluation::e_rf_sp;
        break;
      }
    } else if (Operand == TPCLatencyEvaluation::e_src_a) {
      switch (Desc.getOpcode()) {
      case TPC::ST_G_INCs:
      case TPC::ST_G_INCz:
        file = TPCLatencyEvaluation::e_rf_s;
        break;
      case TPC::ST_G_INCp:
        file = TPCLatencyEvaluation::e_rf_sp;
        break;
      }
    }
  };

  FixRegFile(DefMCID, DefData.the_operandID, DefData.the_registerFile);
  FixRegFile(UseMCID, UseData.the_operandID, UseData.the_registerFile);
  
  // Only for physical registers
  if (TPCII::isFMA(DefMCID) && TPCII::isVPUInst(DefMCID) &&
      TPCII::isFMA(UseMCID) && TPCII::isVPUInst(UseMCID) &&
      DefReg.isPhysical() && DefReg != UseReg) {
    const TargetRegisterInfo *TRI = Subtarget.getRegisterInfo();
    LLVM_DEBUG(dbgs() << "FMA processing: " <<
               TRI->getName(DefReg) << ", " << TRI->getName(UseReg) << "\n");
    bool Visited = false;
    for (MCSubRegIterator SubReg(DefReg, TRI, true);
         SubReg.isValid(); ++SubReg) {
      // Skipping DRF/ADRF register
      if (!TPC::VRFRegClass.contains(*SubReg))
        continue;

      if (!TRI->isSubRegisterEq(UseReg, *SubReg))
        continue;

      const auto &GetIndexBySubReg = [&TRI](Register SuperReg,
                                            Register SubReg) {
        assert(SuperReg.isPhysical());
        assert(SubReg.isPhysical());
        unsigned Index = TRI->getSubRegIndex(SuperReg, SubReg);
        switch (Index) {
        default:
          llvm_unreachable("Unexpected index");
        case 0: //Self register
        case TPC::sub_0:
        case TPC::sub_s0:
          return 0;
        case TPC::sub_1:
        case TPC::sub_s1:
          return 1;
        case TPC::sub_2:
          return 2;
        case TPC::sub_3:
          return 3;
        }
      };

      uint8_t DefIdxDst0 = GetIndexBySubReg(DefReg, *SubReg);
      uint8_t UseIdxDst0 = GetIndexBySubReg(UseReg, *SubReg);

      // FMA X2 to FMA and vs versa
      // [LLVM-2305] In case the instruction is a switch with X2 return 0
      // otherwise return 1 This check only relvent for FMA operations
      // with X2.
      if (getSwitches(UseMI) & TPCII::SW_X2_ARITHMETIC &&
          DefIdxDst0 != UseIdxDst0) {
        DEBUG_WITH_TYPE("latency", dbgs() << "LLVM-2305" << 6 << "\n");
        return 6;
      }

      DefData.the_idxDst0 = DefIdxDst0;
      UseData.the_idxDst0 = UseIdxDst0;
      Visited = true;
      unsigned TempLatency =
          TPCLatencyEvaluation::calculateLatency(DefData, UseData,
                                                 tpc_generation);

      if (static_cast<int>(TempLatency) > Latency)
        Latency = static_cast<int>(TempLatency);
    }
    assert(Visited && "calculateLatency has not been called");
  } else if (TPCII::isFMA(DefMCID) && TPCII::isVPUInst(DefMCID) &&
             TPCII::isFMA(UseMCID) && TPCII::isVPUInst(UseMCID) &&
             DefReg.isVirtual()) {
    unsigned DefSubregs = DefOperand.getSubReg();
    unsigned UseSubregs = UseOperand.getSubReg();

    const auto &GetSubIndex = [](unsigned SubregsType) ->
        SmallVector<unsigned> {
      switch (SubregsType) {
      default:
        llvm_unreachable("Unexpected index");
      case 0: //Self register
      case TPC::sub_0:
      case TPC::sub_s0:
        return {0};
      case TPC::sub_1:
      case TPC::sub_s1:
        return {1};
      case TPC::sub_2:
        return {2};
      case TPC::sub_3:
        return {3};
      case TPC::sub_0_sub_1:
        return {0, 1};
      case TPC::sub_2_sub_3:
        return {2, 3};
      }
    };

    if (DefSubregs == UseSubregs) {
      Latency = static_cast<int>(TPCLatencyEvaluation::calculateLatency(DefData, UseData, tpc_generation));
    } else {
      SmallVector<unsigned> DefIdxs = GetSubIndex(DefSubregs);
      SmallVector<unsigned> UseIdxs = GetSubIndex(UseSubregs);

      for (auto DefIdx : DefIdxs) {
        for (auto UseIdx : UseIdxs) {
          // FMA X2 to FMA and vs versa
          // [LLVM-2305] In case the instruction is a switch with X2 return 0
          // otherwise return 1. This check only relvent for FMA operations
          // with X2.
          if (getSwitches(UseMI) & TPCII::SW_X2_ARITHMETIC &&
              DefIdx != UseIdx) {
            DEBUG_WITH_TYPE("latency", dbgs() << "LLVM-2305" << 6 << "\n");
            return 6;
          }

          DefData.the_idxDst0 = DefIdx;
          UseData.the_idxDst0 = UseIdx;
          unsigned TempLatency =
              TPCLatencyEvaluation::calculateLatency(DefData, UseData,
                                                     tpc_generation);

          if (static_cast<int>(TempLatency) > Latency)
            Latency = static_cast<int>(TempLatency);
        }
      }
    }
  } else {
    Latency = static_cast<int>(TPCLatencyEvaluation::calculateLatency(DefData, UseData, tpc_generation));
  }

  assert(Latency >= 0);
  
  //
  // Fix latency value taking into account different PRM restrictions
  // In fact, the latencyDB interface function 'calculateLatency' should have
  // taken care of this, but this is not true for now.
  //
  if (tpc_generation > 1) {
    // Gaudi and Goya2 restrictions:
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

  // Goya2 PRM v0.64: After doing a GEN_ADDR that changes the Tensor ID of that specific ADRF, 
  // it is not allowed to do LD_G to VRF/VPRF from that ADRF for 6 cycles.
  // Moreover you don’t know what are the leftovers in ADRF from previous kernels, so we do it always.
  // TODO: At the beginning of the loop the first LD_G suffers more than the next ones, 
  // so there is opportunity to peel the first iteration.
  if (Subtarget.hasGrecoISA()
      &&
      ((TPCII::isLoadInst(DefMCID) &&
        TPCII::getSlotOpCode(DefMCID) == TPCII::ldGEN_ADDR) ||
       (TPCII::isStoreInst(DefMCID) &&
        TPCII::getSlotOpCode(DefMCID) == TPCII::stGEN_ADDR))
      &&
      (TPCII::isLoadInst(UseMCID) &&
       TPCII::getSlotOpCode(UseMCID) == TPCII::LD_G)
      &&
      (TPC::VRFRegClass.contains(UseMI.getOperand(0).getReg()) ||
       TPC::VPRFRegClass.contains(UseMI.getOperand(0).getReg()))
      &&
      (DefMI.getOperand(0).getReg() == UseMI.getOperand(1).getReg())
     )
  {
    Latency = std::max(Latency, 7);
  }
  
  DEBUG_WITH_TYPE("latency", dbgs() << "== Latency = " << Latency << "\n");
  DEBUG_WITH_TYPE("latency", dbgs() << "=====================================================\n");

  return Latency;
}

bool TPCInstrInfo::isPredicated(const MachineInstr& MI) const {
  if (MI.getOpcode() == TPC::JMPR)
    return true;

  bool Polarity; int Predicate;
  if (!looksLikeHavingAPredicate(MI, Predicate, Polarity))
    return false;

  Register PredReg = MI.getOperand(Predicate).getReg();
  return PredReg != TPC::SPRF_TRUE && PredReg != TPC::VPRF_TRUE;
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

LLVM_READONLY int getAltOpcodeLoadGen3 (uint16_t Opcode, enum TPC::Slot inSlot);
LLVM_READONLY int getAltOpcodeStoreGen3(uint16_t Opcode, enum TPC::Slot inSlot);
LLVM_READONLY int getAltOpcodeSpuGen3  (uint16_t Opcode, enum TPC::Slot inSlot);
LLVM_READONLY int getAltOpcodeVpuGen3  (uint16_t Opcode, enum TPC::Slot inSlot);

LLVM_READONLY int getAltOpcodeLoadGen4 (uint16_t Opcode, enum TPC::Slot inSlot);
LLVM_READONLY int getAltOpcodeStoreGen4(uint16_t Opcode, enum TPC::Slot inSlot);
LLVM_READONLY int getAltOpcodeSpuGen4  (uint16_t Opcode, enum TPC::Slot inSlot);
LLVM_READONLY int getAltOpcodeVpuGen4  (uint16_t Opcode, enum TPC::Slot inSlot);

LLVM_READONLY int getAltOpcodeLoadDoron1 (uint16_t Opcode, enum TPC::Slot inSlot);
LLVM_READONLY int getAltOpcodeStoreDoron1(uint16_t Opcode, enum TPC::Slot inSlot);
LLVM_READONLY int getAltOpcodeSpuDoron1  (uint16_t Opcode, enum TPC::Slot inSlot);
LLVM_READONLY int getAltOpcodeVpuDoron1  (uint16_t Opcode, enum TPC::Slot inSlot);

bool TPCInstrInfo::getOtherSlotOpcodes(MachineInstr* MI, std::vector<unsigned>& opcodes) const {
  int altOpc;
  bool found = false;
  int (*getAltOpcode)(uint16_t, TPC::Slot);
  int (*getAltOpcodeGen)(uint16_t, TPC::Slot) = nullptr;
  int (*getAltOpcodeGenEx)(uint16_t, TPC::Slot) = nullptr;
  int (*getAltOpcodeGenExEx)(uint16_t, TPC::Slot) = nullptr;
  if (TPCII::isVPUInst(MI->getDesc())) {
    getAltOpcode = llvm::TPC::getAltOpcodeVpu;
    if (Subtarget.hasDoron1ISA()) {
      getAltOpcodeGenExEx = llvm::TPC::getAltOpcodeVpuDoron1;
    }
    if (Subtarget.hasGaudi2ISA() || Subtarget.hasDoron1ISA()) {
      getAltOpcodeGenEx = llvm::TPC::getAltOpcodeVpuGen4;
    }
    if (Subtarget.hasGrecoISA() || Subtarget.hasGaudi2ISA()) {
      getAltOpcodeGen = llvm::TPC::getAltOpcodeVpuGen3;
    }
  }
  else if (TPCII::isSPUInst(MI->getDesc())) {
    getAltOpcode = llvm::TPC::getAltOpcodeSpu;
    if (Subtarget.hasDoron1ISA()) {
      getAltOpcodeGenExEx = llvm::TPC::getAltOpcodeSpuDoron1;
    }
    if (Subtarget.hasGaudi2ISA() || Subtarget.hasDoron1ISA()) {
      getAltOpcodeGenEx = llvm::TPC::getAltOpcodeSpuGen4;
    }
    if (Subtarget.hasGrecoISA() || Subtarget.hasGaudi2ISA()) {
      getAltOpcodeGen = llvm::TPC::getAltOpcodeSpuGen3;
    }
  }
  else if (TPCII::isLoadInst(MI->getDesc())) {
    getAltOpcode = llvm::TPC::getAltOpcodeLoad;
    if (Subtarget.hasDoron1ISA()) {
      getAltOpcodeGenExEx = llvm::TPC::getAltOpcodeLoadDoron1;
    }
    if (Subtarget.hasGaudi2ISA() || Subtarget.hasDoron1ISA()) {
      getAltOpcodeGenEx = llvm::TPC::getAltOpcodeLoadGen4;
    }
    if (Subtarget.hasGrecoISA() || Subtarget.hasGaudi2ISA()) {
      getAltOpcodeGen = llvm::TPC::getAltOpcodeLoadGen3;
    }
  }
  else if (TPCII::isStoreInst(MI->getDesc())) {
    getAltOpcode = llvm::TPC::getAltOpcodeStore;
    if (Subtarget.hasDoron1ISA()) {
      getAltOpcodeGenExEx = llvm::TPC::getAltOpcodeStoreDoron1;
    }
    if (Subtarget.hasGaudi2ISA() || Subtarget.hasDoron1ISA()) {
      getAltOpcodeGenEx = llvm::TPC::getAltOpcodeStoreGen4;
    }
    if (Subtarget.hasGrecoISA() || Subtarget.hasGaudi2ISA()) {
      getAltOpcodeGen = llvm::TPC::getAltOpcodeStoreGen3;
    }
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
    if (getAltOpcodeGenExEx != nullptr) {
      altOpc = getAltOpcodeGenExEx(MI->getOpcode(), slots[i]);
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
  return (TPC::SPRFRegClass.contains(Reg) && (Reg != TPC::SPRF_TRUE));
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
  const TargetRegisterClass *DefRegClass = getRegisterClass(DefReg, MRI);
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
      const TargetRegisterClass *RC = getRegisterClass(FoldReg, MRI);
      const TargetRegisterClass *LRC = getRegisterClass(LiveReg, MRI);
      if (isRegisterClassUnspillable(LRC))
        return nullptr;

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
llvm::getRegisterClass(Register Reg, const MachineRegisterInfo &MRI) {
  const TargetRegisterInfo &TRI = *MRI.getTargetRegisterInfo();
  const TargetRegisterClass *RC = nullptr;
  if (Reg.isPhysical()) {
    if (TPC::SRFRegClass.contains(Reg))
      RC = &TPC::SRFRegClass;
    else
      for (auto TRC : TRI.regclasses())
        if (TRC->contains(Reg)) {
          if (!RC || TRC->hasSubClassEq(RC))
            RC = TRC;
        }
  } else {
    RC = MRI.getRegClass(Reg);
  }

  assert(RC);
  return RC;
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
  return PredMO.getReg() == TPC::SPRF_TRUE;
}

void TPCInstrInfo::reMaterialize(MachineBasicBlock &MBB,
                                 MachineBasicBlock::iterator MI, Register DestReg,
                                 unsigned SubIdx, const MachineInstr &Orig,
                                 const TargetRegisterInfo &TRI) const {
  const MachineRegisterInfo &MRI = MBB.getParent()->getRegInfo();

  MachineInstr *DefMI = MBB.getParent()->CloneMachineInstr(&Orig);
  DefMI->substituteRegister(DefMI->getOperand(0).getReg(), DestReg, SubIdx, TRI);

  // TODO: this is not a fix, it's just a workaround.
  //       Sometimes llvm adds subregisters to vrfs for some reason
  const TargetRegisterClass* RC = MRI.getRegClass(DefMI->getOperand(0).getReg());
  if (TPC::VRFRegClass.hasSubClassEq(RC)) {
    const MCInstrDesc &MCI = DefMI->getDesc();
    DefMI->getOperand(MCI.getNumOperands() - 3).setSubReg(0);
  }

  MBB.insert(MI, DefMI);
  if (DefMI->getOperand(0).getSubReg()) {
    DefMI->getOperand(0).setIsUndef(true);
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
  const MCInstrDesc &MCID = MI.getDesc();
  if (!TPCII::isLoadInst(MCID) && !TPCII::isStoreInst(MCID)) {
    return false;
  }

  bool Polarity;
  int Predicate;
  if (!looksLikeHavingAPredicate(MI, Predicate, Polarity))
    return false;

  pred = MI.getOperand(Predicate).getReg();
  polarity = Polarity;
  return true;
}

bool TPCInstrInfo::isGenAddr(const MachineInstr &MI) const {
  const auto Opcode = MI.getOpcode();
  return (Opcode == TPC::GEN_ADDR_st || Opcode == TPC::GEN_ADDR_stT ||
          Opcode == TPC::GEN_ADDR_ld || Opcode == TPC::GEN_ADDR_ldT);
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
  const TargetRegisterClass *RC = getRegisterClass(reg, MRI);
  return isRegisterClassUnspillable(RC);
}

bool TPCInstrInfo::isRegisterClassUnspillable(const TargetRegisterClass *RC) const {
  return (TPC::HWPCRegClass.hasSubClassEq(RC) ||
          TPC::HWTnsrRegLdRegClass.hasSubClassEq(RC) ||
          TPC::HWTnsrRegStRegClass.hasSubClassEq(RC) ||
          TPC::HWOffsSizeRegLdRegClass.hasSubClassEq(RC) ||
          TPC::HWOffsSizeRegStRegClass.hasSubClassEq(RC) ||
          TPC::HWRMWRegRegClass.hasSubClassEq(RC) ||
          TPC::HWDivStepRegClass.hasSubClassEq(RC) ||
          TPC::HWSCarryRegClass.hasSubClassEq(RC) ||
          TPC::HWVCarryRegClass.hasSubClassEq(RC) ||
          TPC::HWZPRegRegClass.hasSubClassEq(RC) ||
          TPC::HWSqzCntrSubRegClass.hasSubClassEq(RC) ||
          TPC::HWSqzCntrRegClass.hasSubClassEq(RC) ||
          TPC::HVRFRegClass.hasSubClassEq(RC) ||
          TPC::HVPRFRegClass.hasSubClassEq(RC) ||
          TPC::HSRFRegClass.hasSubClassEq(RC) ||
          TPC::HSPRFRegClass.hasSubClassEq(RC) ||
          // Also check for any RegClass contained in HSRF
          // because getRegisterClass() may return one of them.
          // For example, getRegisterClass(ST_TNST_ID_REG)
          // returns HSRFPriorDoron1RegClass
          TPC::HSRFPriorDoron1RegClass.hasSubClassEq(RC) ||
          TPC::HSRFDoron1RegClass.hasSubClassEq(RC) ||
          TPC::HSRFDoron1RWRegClass.hasSubClassEq(RC));
}

MachineFunction *TPCInstrInfo::MachineFunctionX2 = nullptr;

bool TPCInstrInfo::useMachineCombiner() const {
  return TPCEnableX2 && Subtarget.hasGaudi2ISA();
}

bool TPCInstrInfo::isThroughputPattern(MachineCombinerPattern Pattern) const {
  return false;
}

bool TPCInstrInfo::shouldReduceRegisterPressure(
    MachineBasicBlock *MBB, RegisterClassInfo *RegClassInfo) const {
  m_tpcx2Util.resetMBBRegPressure(MBB, RegClassInfo);
  return false;
}

bool TPCInstrInfo::getMachineCombinerPatterns(
    MachineInstr &Root, SmallVectorImpl<MachineCombinerPattern> &Patterns,
    bool DoRegPressureReduce) const {
  // TODO : set booleans based on findings
  AllowReorderForX2 = false;
  if (AggressiveX2Combine) {
    CheckVRegsPartOfOneDRFX2 = false;
    CheckRegPressureX2 = false;
  }
  return m_tpcx2Util.findMACX2Pattern(Root, Patterns, CheckVRegsPartOfOneDRFX2,
                                      CheckRegPressureX2, AllowReorderForX2);
}

void TPCInstrInfo::genAlternativeCodeSequence(
    MachineInstr &Root, MachineCombinerPattern Pattern,
    SmallVectorImpl<MachineInstr *> &InsInstrs,
    SmallVectorImpl<MachineInstr *> &DelInstrs,
    DenseMap<unsigned, unsigned> &InstrIdxForVirtReg) const {
  MachineFunctionX2 = Root.getParent()->getParent();
  return m_tpcx2Util.getMACX2Sequence(Root, InsInstrs, DelInstrs);
}

bool llvm::looksLikeHavingAPredicate(const MachineInstr &MI, int &PredicateIdx, bool &Polarity) {
  std::tie(PredicateIdx, Polarity) = TPCMCInstrInfo::getPredicatePolarity(MI);
  if (PredicateIdx == -1) {
    return false;
  }

  const MachineOperand &PredicateMO = MI.getOperand(PredicateIdx);
  if (!PredicateMO.isReg())
    return false;
  Register PredReg = PredicateMO.getReg();
  const MachineFunction *MF = nullptr;
  if (TPCInstrInfo::MachineFunctionX2 != nullptr) {
    MF = TPCInstrInfo::MachineFunctionX2;
  } else {
    MF = MI.getParent()->getParent();
  }
  const MachineRegisterInfo &MRI = MF->getRegInfo();
  const TargetRegisterClass *RC;
  if (PredReg.isPhysical()) {
    RC = getRegisterClass(PredReg, MRI);
  } else {
    // Virtual registers.
    // In this case we cannot determine if the register is SP0, so such
    // instruction is always considered predicated if the register is of
    // predicate class.
    RC = MRI.getRegClass(PredReg);
  }
  if (!TPC::SPRFRegClass.hasSubClassEq(RC) &&
      !TPC::VPRFRegClass.hasSubClassEq(RC)) {
    assert(false && "Unexpected class of OPERAND_PREDICATE register");
    return false;
  }
  return true;
}

int llvm::getIncome(const MachineInstr &I) {
// Instructions that do not produce values do not need income value.
  if (I.getDesc().getNumDefs() == 0)
    return -1;

  // Calculate income source operand. It must be of the same type as the result
  // of the instruction and be tied to the instruction result.
  unsigned NIncome, Ndefs;
  for (NIncome = I.getNumOperands() - 1, Ndefs = I.getNumDefs() - 1; NIncome > Ndefs; --NIncome) {
    const MachineOperand &Op = I.getOperand(NIncome);
    if (Op.isReg() && Op.isTied()) {
      unsigned TiedOp = I.findTiedOperandIdx(NIncome);
      if (TiedOp == 0)
        break;
    }
  }
  if (NIncome == 0) {
    assert(TPCII::isStoreInst(I.getDesc()) && "No income??");
    return -1;
  }

  return NIncome;
}

unsigned llvm::getSwitches(const MachineInstr &MI) {
  const MCInstrDesc &Desc = MI.getDesc();
  for (unsigned I = Desc.getNumOperands() - 1; I >= 0; --I) {
    const MCOperandInfo &OpInfo = Desc.OpInfo[I];
    if (OpInfo.OperandType == TPC::OperandType::OPERAND_SWITCH) {
      const MachineOperand &SwitchesMO = MI.getOperand(I);
      assert(SwitchesMO.isImm() && "Expect OPERAND_SWITCH is immediate");
      return SwitchesMO.getImm();
    }
  }
  
  assert(false && "Switch was not found.");
  return 0;
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

bool llvm::isAllLanesVpuConvert(const MachineInstr &MI,
                                const TPCSubtarget &TST) {
  assert(TPCII::isVPUInst(MI.getDesc()));
  assert(TPCII::getSlotOpCode(MI.getDesc()) == TPCII::vpuCONVERT);

  const unsigned Sw = getSwitches(MI);
  const unsigned SrcTy = MI.getOperand(2).getImm();
  const unsigned DstTy = (Sw >> 8) & 0xF;

  // Goya CONVERT does not support ALL_LANES
  if (TST.hasGoyaISA())
    return false;

  // Gaudi CONVERT supports ALL_LANES only for conversion:
  //    FP32 -> BF16.
  if (TST.hasGaudiISA()) {
    return
        // ALL_LANES specified.
        (Sw & TPCII::SW_NUM_LANES_SRCB) == TPCII::SW_ALL_LANES_SRCB &&
        // FP32 -> BF16 conversion.
        SrcTy == TPCII::FP32 && DstTy == TPCII::BF16;
  }

  // GaudiB CONVERT supports ALL_LANES only for conversions:
  //    BF16 -> FP32
  //    FP16 -> FP32
  if (TST.hasGaudiBISA()) {
    return
        // ALL_LANES specified.
        (Sw & TPCII::SW_NUM_LANES_SRCB) == TPCII::SW_ALL_LANES_SRCB &&
        // BF16/FP16 -> FP32
        (SrcTy == TPCII::BF16 || SrcTy == TPCII::FP16) && DstTy == TPCII::FP32;
  }

  // Greco and Gaudi2 CONVERT supports ALL_LANES for all supported conversions.
  if (TST.hasGrecoISA() || TST.hasGaudi2ISA())
    return (Sw & TPCII::SW_NUM_LANES_SRCB) == TPCII::SW_ALL_LANES_SRCB;

  // On Doron1 CONVERT NUM_LANES mask depends on conversion types.
  //
  // According to Doron1 PRM bit 7 is used in NUM_LANES mask for the following
  // conversions only:
  //    * U32 / I32 -> U8 / I8
  //    * F32 / F8* -> U8 / I8
  //    * F8*       -> U32 / U16 / U8 / I32/ I16 / I8
  //
  // Otherwise, bit 7 is used as X2 switch mask.
  //
  // ALL_LANES is prohibited for the following conversions:
  //    * U32 / I32              -> U8 / I8
  //    * U16 / I16              -> U8 / I8
  //    * F32 / F16 / BF16 / F8* -> U8 / I8
  //    * F8*                    -> U32 / I32 / U16 / I16 / U8 / I8
  //
  // So as ALL_LANES prohibition set contains set of extra bit conversions, just
  // check against SW_NUM_LANES_SRCB_G5 mask or SW_NUM_LANES_SRCB mask, it does
  // not matter.
  if (TST.hasDoron1ISA()) {
    // U32 / I32 -> U8 / I8
    if ((SrcTy == TPCII::UINT32 || SrcTy == TPCII::INT32) &&
        (DstTy == TPCII::UINT8 || DstTy == TPCII::INT8))
      return false;

    // U16 / I16 -> U8 / I8
    if ((SrcTy == TPCII::UINT16 || SrcTy == TPCII::INT16) &&
        (DstTy == TPCII::UINT8 || DstTy == TPCII::INT8))
      return false;

    // F32 / F16 / BF16 / F8* -> U8 / I8
    if ((SrcTy == TPCII::FP32 || SrcTy == TPCII::FP16 || SrcTy == TPCII::BF16 ||
         SrcTy == TPCII::FP8_143 || SrcTy == TPCII::FP8_152) &&
        (DstTy == TPCII::UINT8 || DstTy == TPCII::INT8))
      return false;

    // F8* -> U32 / I32 / U16 / I16 / U8 / I8
    if ((SrcTy == TPCII::FP8_143 || SrcTy == TPCII::FP8_152) &&
        (DstTy == TPCII::UINT32 || DstTy == TPCII::UINT16 ||
         DstTy == TPCII::UINT8 || DstTy == TPCII::INT32 ||
         DstTy == TPCII::INT16 || DstTy == TPCII::INT8))
      return false;

    return (Sw & TPCII::SW_NUM_LANES_SRCB_G5) == TPCII::SW_ALL_LANES_SRCB;
  }

  llvm_unreachable("Unexpected target architecture");
}

bool llvm::isAllLanesVpuConvertI32(const MachineInstr &MI,
                                   const TPCSubtarget &TST) {
  assert(TPCII::isVPUInst(MI.getDesc()));
  assert(TPCII::getSlotOpCode(MI.getDesc()) == TPCII::vpuCONVERT_INT32 ||
         TPCII::getSlotOpCode(MI.getDesc()) == TPCII::vpuCONVERT_UINT32);

  const unsigned Sw = getSwitches(MI);

  // Goya, Gaudi and GaudiB CONVERT_INT32 / CONVERT_UINT32 do not support
  // ALL_LANES.
  if (TST.hasGoyaISA() || TST.hasGaudiISA() || TST.hasGaudiBISA())
    return false;

  // Greco and Gaudi2 CONVERT_INT32 / CONVERT_UINT32 support ALL_LANES for any
  // type.
  if (TST.hasGrecoISA() || TST.hasGaudi2ISA())
    return (Sw & TPCII::SW_NUM_LANES) == TPCII::SW_ALL_LANES;

  // Doron1 CONVERT_INT32 / CONVERT_UINT32 supports ALL_LANES only for
  // conversions:
  //    U32 -> U16
  //    I32 -> I16
  //
  // Since Doron1 NUM_LANES mask depends on destination type.
  //
  // According to Doron1 PRM extra bit is used only for conversions:
  //    I32 -> I8
  //    U32 -> U8
  //
  // So as ALL_LANES prohibition set contains set of extra bit conversions, just
  // check against SW_NUM_LANES_G5 or SW_NUM_LANES mask, it does not matter.
  if (TST.hasDoron1ISA()) {
    return
        // TO_16
        (Sw & TPCII::SW_GROUP_TO) == TPCII::SW_TO_16 &&
        // ALL_LANES specified.
        (Sw & TPCII::SW_NUM_LANES) == TPCII::SW_ALL_LANES;
  }

  llvm_unreachable("Unexpected target architecture");
}

bool llvm::isAllLanesVpuConvertI16(const MachineInstr &MI,
                                   const TPCSubtarget &TST) {
  assert(TPCII::isVPUInst(MI.getDesc()));
  assert(TPCII::getSlotOpCode(MI.getDesc()) == TPCII::vpuCONVERT_INT16 ||
         TPCII::getSlotOpCode(MI.getDesc()) == TPCII::vpuCONVERT_UINT16);

  const unsigned Sw = getSwitches(MI);

  // Goya, Gaudi, GaudiB, Doron1 CONVERT_INT16 / CONVERT_UINT16 do not support
  // ALL_LANES.
  if (TST.hasGoyaISA() || TST.hasGaudiISA() || TST.hasGaudiBISA() ||
      TST.hasDoron1ISA())
    return false;

  // Greco and Gaudi2 CONVERT_INT16 / CONVERT_UINT16 support ALL_LANES for any
  // type.
  if (TST.hasGrecoISA() || TST.hasGaudi2ISA())
    return (Sw & TPCII::SW_NUM_LANES) == TPCII::SW_ALL_LANES;

  llvm_unreachable("Unexpected target architecture");
}
