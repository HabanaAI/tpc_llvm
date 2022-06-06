//===-- TPCFrameLowering.h - Define frame lowering for TPC --*- C++ -*-===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
//
//
//
//===----------------------------------------------------------------------===//

#ifndef LLVM_LIB_TARGET_TPC_TPCFRAMELOWERING_H
#define LLVM_LIB_TARGET_TPC_TPCFRAMELOWERING_H

#include "llvm/CodeGen/TargetFrameLowering.h"

namespace llvm {

class TPCSubtarget;
class TPCFrameLowering : public TargetFrameLowering {
  mutable unsigned MaxScalarMemory = 0;
  mutable unsigned MaxVectorMemory = 0;
  mutable unsigned ScalarDataSize = 0;
  mutable unsigned VectorDataSize = 0;
  mutable unsigned SpillScalarDataSize = 0;
  mutable unsigned SpillVectorDataSize = 0;

  // Describes a 'stack' slot.
  struct FrameObject {
    unsigned Size = 0;
    unsigned Offset;    // Start of the slot in local memory

    bool isEmpty() const { return Size == 0; }
  };

  // Array of frame objects. Index into this array is the 'FrameIndex' provided
  // for loadRegFromStackSlot/storeRegToStackSlot.
  SmallVector<FrameObject, 16> FrameObjects;

  // Offset for the next frame record in local memory.
  unsigned TopVectorFrame = 0;
  unsigned TopScalarFrame = 0;

public:
  explicit TPCFrameLowering();

  unsigned getMaxScalarMemory() const { return MaxScalarMemory; }
  unsigned getMaxVectorMemory() const { return MaxVectorMemory; }
  unsigned getScalarDataSize() const { return ScalarDataSize; }
  unsigned getVectorDataSize() const { return VectorDataSize; }
  unsigned getSpillScalarDataSize() const { return SpillScalarDataSize; }
  unsigned getSpillVectorDataSize() const { return SpillVectorDataSize; }

  void setMaxScalarMemory(unsigned Sz) const { MaxScalarMemory = Sz; }
  void setMaxVectorMemory(unsigned Sz) const { MaxVectorMemory = Sz; }
  void setScalarDataSize(unsigned Sz) const { ScalarDataSize = Sz; }
  void setVectorDataSize(unsigned Sz) const { VectorDataSize = Sz; }
  void setSpillScalarDataSize(unsigned Sz) const { SpillScalarDataSize = Sz; }
  void setSpillVectorDataSize(unsigned Sz) const { SpillVectorDataSize = Sz; }


  unsigned getSpillObject(int FrameIndex, const TargetRegisterClass *RC,
                          MachineFunction &MF, unsigned Sz);
  unsigned getFrameObjectOffset(int FrameIndex) const;
  unsigned getFrameObjectSize(int FrameIndex) const;

  /// emitProlog/emitEpilog - These methods insert prolog and epilog code into
  /// the function.
  void emitPrologue(MachineFunction &MF, MachineBasicBlock &MBB) const override;
  void emitEpilogue(MachineFunction &MF, MachineBasicBlock &MBB) const override;

  MachineBasicBlock::iterator
  eliminateCallFramePseudoInstr(MachineFunction &MF,
                                MachineBasicBlock &MBB,
                                MachineBasicBlock::iterator I) const override;

  bool hasReservedCallFrame(const MachineFunction &MF) const override;
  bool hasFP(const MachineFunction &MF) const override;
  void determineCalleeSaves(MachineFunction &MF, BitVector &SavedRegs,
                            RegScavenger *RS = nullptr) const override;

  StackOffset getFrameIndexReference(const MachineFunction &MF, int FI,
                                     Register &FrameReg) const override;

  /// targetHandlesStackFrameRounding - Returns true if the target is
  /// responsible for rounding up the stack frame (probably at emitPrologue
  /// time).
  bool targetHandlesStackFrameRounding() const override { return true; }

private:

  // Returns true if MF is a leaf procedure.
  bool isLeafProc(MachineFunction &MF) const;


};

} // End llvm namespace

#endif
