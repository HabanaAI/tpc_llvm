//===-- TPCRegisterInfo.cpp - TPC Register Information ----------------===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//
//
// This file contains the TPC implementation of the TargetRegisterInfo class.
//
//===----------------------------------------------------------------------===//

#include "TPCRegisterInfo.h"
#include "TPCSubtarget.h"
#include "MCTargetDesc/TPCMCTargetDesc.h"
#include "llvm/ADT/BitVector.h"
#include "llvm/ADT/SmallSet.h"
#include "llvm/ADT/STLExtras.h"
#include "llvm/CodeGen/MachineFrameInfo.h"
#include "llvm/CodeGen/MachineFunction.h"
#include "llvm/CodeGen/MachineInstrBuilder.h"
#include "llvm/CodeGen/MachineRegisterInfo.h"
#include "llvm/IR/Type.h"
#include "llvm/Support/CommandLine.h"
#include "llvm/Support/ErrorHandling.h"
#include "llvm/CodeGen/TargetInstrInfo.h"
#include "llvm/CodeGen/MachineModuleInfo.h"

using namespace llvm;

#define GET_REGINFO_TARGET_DESC
#include "TPCGenRegisterInfo.inc"


TPCRegisterInfo::TPCRegisterInfo() : TPCGenRegisterInfo(TPC::S0) {}

const MCPhysReg*
TPCRegisterInfo::getCalleeSavedRegs(const MachineFunction *MF) const {
  return CSR_AllRegs_SaveList;
}

const uint32_t *
TPCRegisterInfo::getCallPreservedMask(const MachineFunction &MF,
                                        CallingConv::ID CC) const {
  return CSR_AllRegs_RegMask;
}

const uint32_t*
TPCRegisterInfo::getRTCallPreservedMask(CallingConv::ID CC) const {
  return CSR_AllRegs_RegMask;
}

BitVector TPCRegisterInfo::getReservedRegs(const MachineFunction &MF) const {
  BitVector Reserved(getNumRegs());

  markSuperRegs(Reserved, TPC::LFSR);
  markSuperRegs(Reserved, TPC::LFSR_NO_CHANGE);
  markSuperRegs(Reserved, TPC::V_LANE_ID_32);
  markSuperRegs(Reserved, TPC::V_LANE_ID_16);
  markSuperRegs(Reserved, TPC::V_LANE_ID_8);

  Reserved.set(TPC::SP0);
  Reserved.set(TPC::VP0);

  if (MF.getSubtarget<TPCSubtarget>().hasGoyaISA()) {
    Reserved.set(TPC::S31);
  }

  Reserved.set(TPC::S32);
  Reserved.set(TPC::S33);
  Reserved.set(TPC::S34);
  Reserved.set(TPC::S35);

  markSuperRegs(Reserved, TPC::S_LFSR);
  markSuperRegs(Reserved, TPC::S_LFSR_NO_CHANGE);

  Reserved.set(TPC::I0);
  Reserved.set(TPC::I1);
  
  const Module *MM = MF.getMMI().getModule();
  Metadata *PrintfModule = MM->getModuleFlag("tpc-printf");
  if (PrintfModule) {
    Reserved.set(TPC::I2);
    MachineInstrBuilder MIB;
    const TargetInstrInfo *TII = MF.getSubtarget().getInstrInfo();
    auto MBB = MF.getBlockNumbered(0);
    auto firstInst = MF.getBlockNumbered(0)->instr_begin();
    auto FI = &*firstInst;
    MIB = BuildMI(*MBB, firstInst, FI->getDebugLoc(), TII->get(TPC::SET_INDX_spu_ip),
                  TPC::I2)
              .addReg(TPC::I2, RegState::Undef)
              .addImm(0)
              .addImm(31)
              .addImm(0)
              .addReg(TPC::SP0)
              .addImm(0);
  
  }
  return Reserved;
}

const TargetRegisterClass*
TPCRegisterInfo::getPointerRegClass(const MachineFunction &MF,
                                      unsigned Kind) const {
  return &TPC::SRFRegClass;
}

void TPCRegisterInfo::eliminateFrameIndex(MachineBasicBlock::iterator II,
                                          int SPAdj, unsigned FIOperandNum,
                                          RegScavenger *RS) const {
  MachineInstr &MI = *II;
  const TargetRegisterClass *RC = nullptr;
  bool IsSpill = false;
  bool IsFill = false;

  switch (MI.getOpcode()) {
  case TPC::SPILL_ARF_RESTORE:
    RC = &TPC::ARFRegClass;
    IsFill = true;
    break;
  case TPC::SPILL_DRF_RESTORE:
    RC = &TPC::DRFRegClass;
    IsFill = true;
    break;
  case TPC::SPILL_ZRF_RESTORE:
    RC = &TPC::ZRFRegClass;
    IsFill = true;
    break;
  case TPC::SPILL_VRF_RESTORE:
    RC = &TPC::VRFRegClass;
    IsFill = true;
    break;
  case TPC::SPILL_VPRF_RESTORE:
    RC = &TPC::VPRFRegClass;
    IsFill = true;
    break;
  case TPC::SPILL_IRF_RESTORE:
    RC = &TPC::IRFRegClass;
    IsFill = true;
    break;
  case TPC::SPILL_SRF_RESTORE:
    RC = &TPC::SRFRegClass;
    IsFill = true;
    break;
  case TPC::SPILL_SPRF_RESTORE:
    RC = &TPC::SPRFRegClass;
    IsFill = true;
    break;
  case TPC::SPILL_ARF_SAVE:
    RC = &TPC::ARFRegClass;
    IsSpill = true;
    break;
  case TPC::SPILL_DRF_SAVE:
    RC = &TPC::DRFRegClass;
    IsSpill = true;
    break;
  case TPC::SPILL_ZRF_SAVE:
    RC = &TPC::ZRFRegClass;
    IsSpill = true;
    break;
  case TPC::SPILL_VRF_SAVE:
    RC = &TPC::VRFRegClass;
    IsSpill = true;
    break;
  case TPC::SPILL_VPRF_SAVE:
    RC = &TPC::VPRFRegClass;
    IsSpill = true;
    break;
  case TPC::SPILL_IRF_SAVE:
    RC = &TPC::IRFRegClass;
    IsSpill = true;
    break;
  case TPC::SPILL_SRF_SAVE:
    RC = &TPC::SRFRegClass;
    IsSpill = true;
    break;
  case TPC::SPILL_SPRF_SAVE:
    RC = &TPC::SPRFRegClass;
    IsSpill = true;
    break;

  default:
    llvm_unreachable("Unexpected frame index");
  }

  if (IsSpill || IsFill) {
    MachineBasicBlock &MBB = *MI.getParent();
    MachineFunction &MF = *MBB.getParent();
    MachineFrameInfo &MFI = MF.getFrameInfo();
    TPCFrameLowering &FL = *const_cast<TPCFrameLowering *>(
        MF.getSubtarget<TPCSubtarget>().getFrameLowering());
    int FI = MI.getOperand(FIOperandNum).getIndex();
    unsigned Offset;
    Offset = FL.getSpillObject(FI, RC, MF, MFI.getObjectSize(FI));
    MI.getOperand(FIOperandNum).ChangeToImmediate(Offset);
  }
}

Register TPCRegisterInfo::getFrameRegister(const MachineFunction &MF) const {
  return Register();
}

// TPC has no architectural need for stack realignment support,
// except that LLVM unfortunately currently implements overaligned
// stack objects by depending upon stack realignment support.
// If that ever changes, this can probably be deleted.
bool TPCRegisterInfo::canRealignStack(const MachineFunction &MF) const {
  return false;
}

bool TPCRegisterInfo::getRegAllocationHints(unsigned VirtReg,
                                            ArrayRef<MCPhysReg> Order,
                                            SmallVectorImpl<MCPhysReg> &Hints,
                                            const MachineFunction &MF,
                                            const VirtRegMap *VRM,
                                            const LiveRegMatrix *Matrix) const {
  const MachineRegisterInfo *MRI = &MF.getRegInfo();
  const TargetRegisterInfo *TRI = MF.getSubtarget().getRegisterInfo();

  // Hint all physregs that are connected with VirtReg via COPYs.
  const TargetRegisterClass *RC = MRI->getRegClass(VirtReg);
  SmallSet<unsigned, 4> ConnectedPhysRegs;
  for (auto &Use : MRI->use_instructions(VirtReg)) {
    if (Use.isCopy()) {
      Register PhysReg;
      MachineOperand &DstMO = Use.getOperand(0);
      MachineOperand &SrcMO = Use.getOperand(1);
      MachineOperand *VirtRegMO = nullptr;
      // Get the connected physreg, if any.
      if (DstMO.getReg().isPhysical()) {
        PhysReg = DstMO.getReg();
        if (DstMO.getSubReg())
          PhysReg = TRI->getSubReg(PhysReg, DstMO.getSubReg());
        VirtRegMO = &SrcMO;
      } else if (SrcMO.getReg().isPhysical()) {
        PhysReg = SrcMO.getReg();
        if (SrcMO.getSubReg())
          PhysReg = TRI->getSubReg(PhysReg, SrcMO.getSubReg());
        VirtRegMO = &DstMO;
      }
      if (!PhysReg)
        continue;

      if (RC->contains(PhysReg)) {
        ConnectedPhysRegs.insert(PhysReg);
        continue;
      }

      // Check if the subreg index match so that a super register of PhysReg
      // could be hinted.
      if (VirtRegMO->getSubReg() != TPC::NoSubRegister) {
        if (unsigned SuperReg =
            TRI->getMatchingSuperReg(PhysReg, VirtRegMO->getSubReg(), RC))
          ConnectedPhysRegs.insert(SuperReg);
      }
    }
  }

  if (!ConnectedPhysRegs.empty()) {
    // Add the connected physregs sorted by the allocation order.
    for (MCPhysReg Reg : Order)
      if (ConnectedPhysRegs.count(Reg) && !MRI->isReserved(Reg))
        Hints.push_back(Reg);
  }

  return TargetRegisterInfo::getRegAllocationHints(VirtReg, Order, Hints, MF, VRM);
}

bool TPCRegisterInfo::isConstantPhysReg(unsigned PhysReg) const {
  return PhysReg == TPC::SP0
      || PhysReg == TPC::VP0
      || PhysReg == TPC::V_LANE_ID_32
      || PhysReg == TPC::V_LANE_ID_16
      || PhysReg == TPC::V_LANE_ID_8;
}
