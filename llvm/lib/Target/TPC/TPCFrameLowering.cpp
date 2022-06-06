//===-- TPCFrameLowering.cpp - TPC Frame Information ------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
//
// This file contains the TPC implementation of TargetFrameLowering class.
//
//===----------------------------------------------------------------------===//

#include "TPCFrameLowering.h"
#include "TPCSubtarget.h"
#include "llvm/CodeGen/MachineFrameInfo.h"
#include "llvm/CodeGen/MachineFunction.h"
#include "llvm/CodeGen/MachineInstrBuilder.h"
#include "llvm/CodeGen/MachineModuleInfo.h"
#include "llvm/CodeGen/MachineRegisterInfo.h"
#include "llvm/IR/DataLayout.h"
#include "llvm/IR/Function.h"
#include "llvm/Support/CommandLine.h"
#include "llvm/Support/TypeSize.h"
#include "llvm/Target/TargetOptions.h"

using namespace llvm;

extern cl::opt<bool> IgnoreMemOverflow;

TPCFrameLowering::TPCFrameLowering()
  : TargetFrameLowering(TargetFrameLowering::StackGrowsDown, Align(8), 0, Align(8)) {
}

void TPCFrameLowering::emitPrologue(MachineFunction &MF,
                                    MachineBasicBlock &MBB) const {
}

MachineBasicBlock::iterator TPCFrameLowering::
eliminateCallFramePseudoInstr(MachineFunction &MF, MachineBasicBlock &MBB,
                              MachineBasicBlock::iterator I) const {
  llvm_unreachable("No stack frame pseudos");
}


void TPCFrameLowering::emitEpilogue(MachineFunction &MF,
                                    MachineBasicBlock &MBB) const {
}

bool TPCFrameLowering::hasReservedCallFrame(const MachineFunction &MF) const {
  return false;
}

// hasFP - Return true if the specified function should have a dedicated frame
// pointer register.  This is true if the function has variable sized allocas or
// if frame pointer elimination is disabled.
bool TPCFrameLowering::hasFP(const MachineFunction &MF) const {
  return false;
}


StackOffset
TPCFrameLowering::getFrameIndexReference(const MachineFunction &MF, int FI,
                                         Register &FrameReg) const {
  const MachineFrameInfo &MFI = MF.getFrameInfo();

  // Return "frame register".
  const TPCTargetLowering &TL = *MF.getSubtarget<TPCSubtarget>().getTargetLowering();
  FrameReg = TL.getZeroReg();

  // Resolve spill slots.
  if (static_cast<unsigned>(FI) < FrameObjects.size() && !FrameObjects[FI].isEmpty())
    return StackOffset::getFixed(getFrameObjectOffset(FI));

  // Resolve locals.
  // TODO: synchronize with GlobalResolver.
  return StackOffset::getFixed(MFI.getObjectOffset(FI));
}


bool TPCFrameLowering::isLeafProc(MachineFunction &MF) const {
  return true;
}

void TPCFrameLowering::determineCalleeSaves(MachineFunction &MF,
                                              BitVector &SavedRegs,
                                              RegScavenger *RS) const {
//  TargetFrameLowering::determineCalleeSaves(MF, SavedRegs, RS);
//  if (!DisableLeafProc && isLeafProc(MF)) {
//    TPCMachineFunctionInfo *MFI = MF.getInfo<TPCMachineFunctionInfo>();
//    MFI->setLeafProc(true);

//    remapRegsForLeafProc(MF);
//  }

}

static bool spillsToVLM(const TargetRegisterClass *RC) {
  if (RC == &TPC::SRFRegClass ||
      RC == &TPC::ZRFRegClass ||
      RC == &TPC::SPRFRegClass ||
      RC == &TPC::IRFRegClass ||
      RC == &TPC::ADRFRegClass)
    return false;
  if (RC == &TPC::VRFRegClass ||
      RC == &TPC::VPRFRegClass ||
      RC == &TPC::DRFRegClass ||
      RC == &TPC::ARFRegClass)
    return true;
  llvm_unreachable("Unsupported register class");
}

unsigned TPCFrameLowering::getSpillObject(int FrameIndex,
                                          const TargetRegisterClass *RC,
                                          MachineFunction &MF,
                                          unsigned Sz) {
  assert(Sz > 0);
  bool UsesVLM = spillsToVLM(RC);
  if (UsesVLM) {
    assert(Sz % 256 == 0);
  } else {
    assert(Sz % 4 == 0);
  }

  // On the first call initialize pointers to memories.
  if (FrameObjects.empty()) {
    TopVectorFrame = getVectorDataSize();
    TopScalarFrame = getScalarDataSize();
  }

  FrameObject *Obj = nullptr;
  if (static_cast<unsigned>(FrameIndex) < FrameObjects.size()) {
    Obj = &FrameObjects[FrameIndex];
    if (!Obj->isEmpty()) {
      // If such frame index already exists, check if the register class fits it.
      assert(Obj->Size >= Sz);
      return Obj->Offset;
    }
  } else {
    // So such frame index yet, create new one.
    FrameObjects.resize(FrameIndex + 1);
    Obj = &FrameObjects.back();
  }

  // Initialize new frame object.
  unsigned &TopFrame = UsesVLM ? TopVectorFrame : TopScalarFrame;
  unsigned MemLimit = UsesVLM ? MaxVectorMemory : MaxScalarMemory;
  MachineFrameInfo &MFI = MF.getFrameInfo();
  Obj->Size = MFI.getObjectSize(FrameIndex);
  Obj->Offset = TopFrame;
  TopFrame += Obj->Size;

  if (UsesVLM) {
      unsigned VectorSpillSize = TopFrame-getVectorDataSize();
      if(getSpillVectorDataSize() < VectorSpillSize)
        setSpillVectorDataSize(VectorSpillSize);
  } else {
      unsigned ScalarSpillSize = TopFrame-getScalarDataSize();
      if(getSpillScalarDataSize() < ScalarSpillSize)
        setSpillScalarDataSize(ScalarSpillSize);
  }

  if (TopFrame > MemLimit) {
    StringRef Msg = UsesVLM ? "Too many vector spills\n" : "Too many scalar spills\n";
    if (!IgnoreMemOverflow) {
      report_fatal_error(Msg.str(), false);
    } else {
      errs() << "Warning: " << Msg.str();
    }
  }

  return Obj->Offset;
}

unsigned TPCFrameLowering::getFrameObjectOffset(int FrameIndex) const {
  assert(!FrameObjects[FrameIndex].isEmpty());
  return FrameObjects[FrameIndex].Offset;
}

unsigned TPCFrameLowering::getFrameObjectSize(int FrameIndex) const {
  assert(!FrameObjects[FrameIndex].isEmpty());
  return FrameObjects[FrameIndex].Size;
}
