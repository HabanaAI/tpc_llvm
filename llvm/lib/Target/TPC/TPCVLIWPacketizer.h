//===-- TPCVLIWPacketizer.h -------------------------------------*- C++ -*-===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//
//
//
//===----------------------------------------------------------------------===//

#ifndef TPCVLIWPACKETIZER_H
#define TPCVLIWPACKETIZER_H

#include "llvm/CodeGen/DFAPacketizer.h"
#include "llvm/CodeGen/MachineBranchProbabilityInfo.h"
#include "llvm/CodeGen/ScheduleDAG.h"
#include "llvm/CodeGen/ScheduleDAGInstrs.h"

namespace llvm {
class TPCPacketizerList : public VLIWPacketizerList {

  // Check if there is a dependence between some instruction already in this
  // packet and this instruction.
  bool Dependence;

protected:
  const MachineBranchProbabilityInfo *MBPI;
  const MachineLoopInfo *MLI;

private:
  const TPCInstrInfo *HII;
  const TPCRegisterInfo *HRI;

public:
  // Ctor.
  TPCPacketizerList(MachineFunction &MF, MachineLoopInfo &MLI,
                    AAResults *AA, const MachineBranchProbabilityInfo *MBPI);

  // initPacketizerState - initialize some internal flags.
  void initPacketizerState() override;

  // ignorePseudoInstruction - Ignore bundling of pseudo instructions.
  bool ignorePseudoInstruction(const MachineInstr &MI,
                               const MachineBasicBlock *MBB) override;

  // isSoloInstruction - return true if instruction MI can not be packetized
  // with any other instruction, which means that MI itself is a packet.
  bool isSoloInstruction(const MachineInstr &MI) override;

  // isLegalToPacketizeTogether - Is it legal to packetize SUI and SUJ
  // together.
  bool isLegalToPacketizeTogether(SUnit *SUI, SUnit *SUJ) override;

  // isLegalToPruneDependencies - Is it legal to prune dependece between SUI
  // and SUJ.
  bool isLegalToPruneDependencies(SUnit *SUI, SUnit *SUJ) override;

  MachineBasicBlock::iterator addToPacket(MachineInstr &MI) override;
  void endPacket(MachineBasicBlock *MBB,
                 MachineBasicBlock::iterator MI) override;
  bool shouldAddToPacket(const MachineInstr &MI) override;

  void unpacketizeSoloInstrs(MachineFunction &MF);

  void addNops(MachineBasicBlock *MBB, MachineBasicBlock::iterator MI);

  bool hasDeadDependence(const MachineInstr &I, const MachineInstr &J);
  bool hasControlDependence(const MachineInstr &I, const MachineInstr &J);
  bool hasTPCSpecificDependence(const MachineInstr &I, const MachineInstr &J);
  int produceLatencyHazard(const MachineInstr &I);
  int PacketNum;

protected:
};
} // namespace llvm
#endif // TPCVLIWPACKETIZER_H

