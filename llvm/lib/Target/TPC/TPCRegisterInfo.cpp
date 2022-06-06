//===-- TPCRegisterInfo.cpp - TPC Register Information ----------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
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
#include "llvm/CodeGen/MachineModuleInfo.h"
#include "llvm/CodeGen/MachineRegisterInfo.h"
#include "llvm/CodeGen/TargetInstrInfo.h"
#include "llvm/IR/Type.h"
#include "llvm/Support/CommandLine.h"
#include "llvm/Support/ErrorHandling.h"
#include "llvm/Target/TargetMachine.h"

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

static bool IsPriorDoron1 = false;

BitVector TPCRegisterInfo::getReservedRegs(const MachineFunction &MF) const {
  BitVector Reserved(getNumRegs());
  IsPriorDoron1 = MF.getSubtarget<TPCSubtarget>().isPriorDoron1();

  markSuperRegs(Reserved, TPC::LFSR);
  markSuperRegs(Reserved, TPC::LFSR_NO_CHANGE);
  markSuperRegs(Reserved, TPC::V_LANE_ID_32);
  markSuperRegs(Reserved, TPC::V_LANE_ID_16);
  markSuperRegs(Reserved, TPC::V_LANE_ID_8);

  if (IsPriorDoron1) {
    Reserved.set(TPC::SP0);
    Reserved.set(TPC::VP0);
  }
  
  // Reserve the fake registers
  Reserved.set(TPC::SPRF_TRUE);
  Reserved.set(TPC::VPRF_TRUE);

  if (MF.getSubtarget<TPCSubtarget>().hasGoyaISA()) {
    Reserved.set(TPC::S31);
  }
  
  if (MF.getSubtarget<TPCSubtarget>().hasDoron1ISA()) {
    markSuperRegs(Reserved, TPC::VPU_LFSR);
    markSuperRegs(Reserved, TPC::VPU_LFSR_RO);
    
    markSuperRegs(Reserved, TPC::SPU_LFSR);
    markSuperRegs(Reserved, TPC::SPU_LFSR_RO);
    
    markSuperRegs(Reserved, TPC::LANE_ID_4B);
    markSuperRegs(Reserved, TPC::LANE_ID_2B);
    markSuperRegs(Reserved, TPC::LANE_ID_1B);
    
    markSuperRegs(Reserved, TPC::THREAD_ID);
  }

  if (MF.getSubtarget<TPCSubtarget>().hasADRFMov()) {
    Reserved.set(TPC::Z30);
  }

  for (const Register Reg : MF.getSubtarget<TPCSubtarget>().getHWLoopRegs())
    Reserved.set(Reg.id());

  markSuperRegs(Reserved, TPC::S_LFSR);
  markSuperRegs(Reserved, TPC::S_LFSR_NO_CHANGE);

  Reserved.set(TPC::I0);
  Reserved.set(TPC::I1);

  if (IsPriorDoron1) {
    Reserved.set(TPC::S36);
    Reserved.set(TPC::S37);
    Reserved.set(TPC::S38);
    Reserved.set(TPC::S39);
    Reserved.set(TPC::S40);
    Reserved.set(TPC::S41);
    Reserved.set(TPC::S42);
    Reserved.set(TPC::S43);
    Reserved.set(TPC::S44);
    Reserved.set(TPC::S45);
    Reserved.set(TPC::S46);
    Reserved.set(TPC::S47);
    Reserved.set(TPC::S48);
    Reserved.set(TPC::S49);
    Reserved.set(TPC::S50);
    Reserved.set(TPC::S51);
    Reserved.set(TPC::S52);
    Reserved.set(TPC::S53);
    Reserved.set(TPC::S54);
    Reserved.set(TPC::S55);
    Reserved.set(TPC::S56);
    Reserved.set(TPC::S57);
    Reserved.set(TPC::S58);
    Reserved.set(TPC::S59);
    Reserved.set(TPC::S60);
    Reserved.set(TPC::S61);
    Reserved.set(TPC::S62);
    Reserved.set(TPC::S63);

    Reserved.set(TPC::Z36);
    Reserved.set(TPC::Z38);
    Reserved.set(TPC::Z40);
    Reserved.set(TPC::Z42);
    Reserved.set(TPC::Z44);
    Reserved.set(TPC::Z46);
    Reserved.set(TPC::Z48);
    Reserved.set(TPC::Z50);
    Reserved.set(TPC::Z52);
    Reserved.set(TPC::Z54);
    Reserved.set(TPC::Z56);
    Reserved.set(TPC::Z58);
    Reserved.set(TPC::Z60);
    Reserved.set(TPC::Z62);

    Reserved.set(TPC::I32);
    Reserved.set(TPC::I33);
    Reserved.set(TPC::I34);
    Reserved.set(TPC::I35);
    Reserved.set(TPC::I36);
    Reserved.set(TPC::I37);
    Reserved.set(TPC::I38);
    Reserved.set(TPC::I39);
    Reserved.set(TPC::I40);
    Reserved.set(TPC::I41);
    Reserved.set(TPC::I42);
    Reserved.set(TPC::I43);
    Reserved.set(TPC::I44);
    Reserved.set(TPC::I45);
    Reserved.set(TPC::I46);
    Reserved.set(TPC::I47);
    Reserved.set(TPC::I48);
    Reserved.set(TPC::I49);
    Reserved.set(TPC::I50);
    Reserved.set(TPC::I51);
    Reserved.set(TPC::I52);
    Reserved.set(TPC::I53);
    Reserved.set(TPC::I54);
    Reserved.set(TPC::I55);
    Reserved.set(TPC::I56);
    Reserved.set(TPC::I57);
    Reserved.set(TPC::I58);
    Reserved.set(TPC::I59);
    Reserved.set(TPC::I60);
    Reserved.set(TPC::I61);
    Reserved.set(TPC::I62);
    Reserved.set(TPC::I63);

    Reserved.set(TPC::AD8);
    Reserved.set(TPC::AD9);
    Reserved.set(TPC::AD10);
    Reserved.set(TPC::AD11);
    Reserved.set(TPC::AD12);
    Reserved.set(TPC::AD13);
    Reserved.set(TPC::AD14);
    Reserved.set(TPC::AD15);
  }
  
  const Module *MM = MF.getFunction().getParent();
  Metadata *PrintfModule = MM->getModuleFlag("tpc-printf");
  if (PrintfModule) {
    Reserved.set(TPC::I2);
    static bool registers_already_initiated = false;
    if (!registers_already_initiated) {
      const auto F = &MF;
      auto MRI = &F->getRegInfo();
      MachineRegisterInfo *mriLocal = (MachineRegisterInfo *)MRI;
      MachineInstrBuilder MIB;
      const TargetInstrInfo *TII = MF.getSubtarget().getInstrInfo();
      auto MBB = MF.getBlockNumbered(0);
      auto firstInst = MF.getBlockNumbered(0)->instr_begin();
      auto FI = &*firstInst;
      bool is_doron1 = MF.getSubtarget<TPCSubtarget>().hasDoron1ISA();
      unsigned i2 = (is_doron1)? mriLocal->createVirtualRegister(&TPC::IRFRegClass)
                        : TPC::I2;

      MIB = BuildMI(*MBB, firstInst, FI->getDebugLoc(),
                    TII->get(TPC::SET_INDX_spu_ip), i2)
                .addReg(TPC::I2, RegState::Undef)
                .addImm(0)
                .addImm(31)
                .addImm(0)
                .addReg(TPC::SPRF_TRUE)
                .addImm(0);

      if (is_doron1) { // writing THREAD_ID to dim 2
        unsigned srf = mriLocal->createVirtualRegister(&TPC::SRFRegClass);
        const MCInstrDesc &DeskMOVinit = TII->get(TPC::MOV_ld_shp);
        MIB =
            BuildMI(*MBB, firstInst, FI->getDebugLoc(), DeskMOVinit, srf);
        MIB.addReg(TPC::THREAD_ID);
        MIB.addImm(0);
        MIB.addReg(srf, RegState::Undef);
        MIB.addReg(TPC::SPRF_TRUE);
        MIB.addImm(0);

        MIB = BuildMI(*MBB, firstInst, FI->getDebugLoc(),
                      TII->get(TPC::SET_INDX_ld_rp), TPC::I2)
                  .addReg(i2)
                  .addReg(srf /*THREAD_ID*/)
                  .addImm(4) // dimension 2    100
                  .addImm(0)
                  .addReg(TPC::SPRF_TRUE)
                  .addImm(0);
      }
      registers_already_initiated = true;
    }
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
  case TPC::SPILL_ADRF_RESTORE:
    RC = &TPC::ADRFRegClass;
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
  case TPC::SPILL_ADRF_SAVE:
    RC = &TPC::ADRFRegClass;
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

bool TPCRegisterInfo::getRegAllocationHints(Register VirtReg,
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

bool TPCRegisterInfo::isConstantPhysReg(MCRegister PhysReg) const {
  switch (PhysReg) {
  default:
    return false;
  case TPC::SP0:
  case TPC::VP0:
    return IsPriorDoron1;
  case TPC::V_LANE_ID_32:
  case TPC::V_LANE_ID_16:
  case TPC::V_LANE_ID_8:
  case TPC::LANE_ID_4B:
  case TPC::LANE_ID_2B:
  case TPC::LANE_ID_1B:
  case TPC::THREAD_ID:
  case TPC::SPRF_TRUE:
  case TPC::VPRF_TRUE:
    return true;
  }
}
