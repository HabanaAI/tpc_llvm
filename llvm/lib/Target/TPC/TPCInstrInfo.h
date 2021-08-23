//===-- TPCInstrInfo.h - TPC Instruction Information --------*- C++ -*-===//
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

#ifndef LLVM_LIB_TARGET_TPC_TPCINSTRINFO_H
#define LLVM_LIB_TARGET_TPC_TPCINSTRINFO_H

#include "TPCRegisterInfo.h"
#include "llvm/CodeGen/TargetInstrInfo.h"
#include "MCTargetDesc/TPCMCInstrInfo.h" // for TPCII enum

#define GET_INSTRINFO_HEADER
#define GET_INSTRINFO_SCHED_ENUM
#include "TPCGenInstrInfo.inc"

namespace llvm {

namespace TPCStackID {
enum StackTypes : uint8_t {
  SCRATCH = 0,
  SLM_SPILL = 1,
  VLM_SPILL = 2
};
}

class TPCSubtarget;

class TPCInstrInfo : public TPCGenInstrInfo {
  const TPCRegisterInfo RI;
  const TPCSubtarget& Subtarget;
  virtual void anchor();
public:
  explicit TPCInstrInfo(TPCSubtarget &ST);

  const TargetRegisterClass *
  getClassOfPhysicalRegister(Register Reg, const TargetRegisterInfo &TRI) const;

  /// getRegisterInfo - TargetInstrInfo is a superset of MRegister info.  As
  /// such, whenever a client has an instance of instruction info, it should
  /// always be able to get register info as well (through this method).
  ///
  const TPCRegisterInfo &getRegisterInfo() const { return RI; }

  /// isLoadFromStackSlot - If the specified machine instruction is a direct
  /// load from a stack slot, return the virtual or physical register number of
  /// the destination along with the FrameIndex of the loaded stack slot.  If
  /// not, return 0.  This predicate must return 0 if the instruction has
  /// any side effects other than loading from the stack slot.
  unsigned isLoadFromStackSlot(const MachineInstr &MI,
                               int &FrameIndex) const override;

  /// isStoreToStackSlot - If the specified machine instruction is a direct
  /// store to a stack slot, return the virtual or physical register number of
  /// the source reg along with the FrameIndex of the loaded stack slot.  If
  /// not, return 0.  This predicate must return 0 if the instruction has
  /// any side effects other than storing to the stack slot.
  unsigned isStoreToStackSlot(const MachineInstr &MI,
                              int &FrameIndex) const override;

  bool analyzeBranch(MachineBasicBlock &MBB, MachineBasicBlock *&TBB,
                     MachineBasicBlock *&FBB,
                     SmallVectorImpl<MachineOperand> &Cond,
                     bool AllowModify = false) const override;

  unsigned removeBranch(MachineBasicBlock &MBB,
                                int *BytesRemoved = nullptr) const override;

  unsigned insertBranch(MachineBasicBlock &MBB, MachineBasicBlock *TBB,
                                MachineBasicBlock *FBB,
                                ArrayRef<MachineOperand> Cond,
                                const DebugLoc &DL,
                                int *BytesAdded = nullptr) const override;

  bool
  reverseBranchCondition(SmallVectorImpl<MachineOperand> &Cond) const override;

  void copyPhysReg(MachineBasicBlock &MBB, MachineBasicBlock::iterator MI,
                   const DebugLoc &DL, MCRegister DestReg, MCRegister SrcReg,
                   bool KillSrc) const override;

  void storeRegToStackSlot(MachineBasicBlock &MBB,
                           MachineBasicBlock::iterator MBBI,
                           unsigned SrcReg, bool isKill, int FrameIndex,
                           const TargetRegisterClass *RC,
                           const TargetRegisterInfo *TRI) const override;

  void loadRegFromStackSlot(MachineBasicBlock &MBB,
                            MachineBasicBlock::iterator MBBI,
                            unsigned DestReg, int FrameIndex,
                            const TargetRegisterClass *RC,
                            const TargetRegisterInfo *TRI) const override;

  /// Target-dependent implementation for foldMemoryOperand.
  /// Target-independent code in foldMemoryOperand will
  /// take care of adding a MachineMemOperand to the newly created instruction.
  /// The instruction and any auxiliary instructions necessary will be inserted
  /// at InsertPt.
  MachineInstr *foldMemoryOperandImpl(
      MachineFunction &MF, MachineInstr &MI, ArrayRef<unsigned> Ops,
      MachineBasicBlock::iterator InsertPt, MachineInstr &LoadMI,
      LiveIntervals *LIS = nullptr) const override;
  MachineInstr *foldMemoryOperandImpl(MachineFunction &MF, MachineInstr &MI,
                                      ArrayRef<unsigned> Ops,
                                      MachineBasicBlock::iterator InsertPt,
                                      int FrameIndex,
                                      LiveIntervals *LIS = nullptr,
                                      VirtRegMap *VRM = nullptr) const override;


  // Lower pseudo instructions after register allocation.
  bool expandPostRAPseudo(MachineInstr &MI) const override;

  bool isProfitableToHoist(MachineInstr &MI) const override;

  bool isReallyTriviallyReMaterializable(const MachineInstr &MI,
                                         AAResults *AA) const override;

  void reMaterialize(MachineBasicBlock &MBB,
                     MachineBasicBlock::iterator MI, unsigned DestReg,
                     unsigned SubIdx, const MachineInstr &Orig,
                     const TargetRegisterInfo &TRI) const override;


  // Create machine specific model for scheduling.
  DFAPacketizer *
  CreateTargetScheduleState(const TargetSubtargetInfo &STI) const override;

  /// Allocate and return a hazard recognizer to use for this target when
  /// scheduling the machine instructions before register allocation.
// Uncomment this if TPC specific HazardRec is needed before RA
  ScheduleHazardRecognizer *
  CreateTargetMIHazardRecognizer(const InstrItineraryData *,
                                 const ScheduleDAGMI *DAG) const override;


  /// Allocate and return a hazard recognizer to use for this target when
  /// scheduling the machine instructions after register allocation.
  ScheduleHazardRecognizer *
  CreateTargetPostRAHazardRecognizer(const InstrItineraryData*,
                                     const ScheduleDAG *DAG) const override;

  /// Compute and return the use operand latency of a given pair of def and use.
  ///
  /// This is a raw interface to the itinerary that may be directly overridden
  /// by a target. Use computeOperandLatency to get the best estimate of
  /// latency.
  int getOperandLatency(const InstrItineraryData *ItinData,
                        const MachineInstr &DefMI, unsigned DefIdx,
                        const MachineInstr &UseMI, unsigned UseIdx) const override;

  bool isPredicated(const MachineInstr &MI) const override;
  unsigned getMachineCSELookAheadLimit() const override { return 20; } // default=5 is too small

  /// Insert non-pairable Noop
  void insertNoop(MachineBasicBlock &MBB,
                  MachineBasicBlock::iterator MI) const override;

  void insertNoop(int opcode, MachineBasicBlock &MBB,
                  MachineBasicBlock::iterator MI);

  /// Retutns 'true' if 'MI' has an equivalent in other slot(s).
  /// Fills in the 'opcodes' vector with alternative opcodes.
  bool getOtherSlotOpcodes(MachineInstr* MI, std::vector<unsigned>& opcodes) const;

  bool instHasImm(const MachineInstr &MI) const;
  bool instHasImmField(const MachineInstr &MI) const;
  bool isScalarToVector(const MachineInstr &MI) const;
  bool hasSRFOrSPRFOperands(const MachineInstr &MI) const;
  bool isVPUInstrWithSRF(const MachineInstr &MI) const;
  bool isMovSRFtoVInstr(const MachineInstr &MI) const;
  bool isIRFProducerWithDimMask(const MachineInstr &MI, unsigned &Mask) const;
  bool isMovDualGroup(const MachineInstr &MI, unsigned * pSrcG, unsigned * pDstG) const;
  uint64_t getInstImm(const MachineInstr &MI) const;
  bool isLDSTInstrWithPredicate(const MachineInstr &MI, unsigned &pred, unsigned &polarity) const;
  bool isConditionalChain(const MachineInstr &DefMI, const MachineInstr &UseMI) const;
  bool isGlobalScalarLoad(const MachineInstr &MI) const;
  bool isGenAddr(const MachineInstr &MI) const;

  static bool isMMIOAccess(const MachineInstr &MI);

  BitVector findAvailableReg(MachineInstr &MI, const TargetRegisterClass* RC) const;
  BitVector findAvailableSRF(MachineInstr &MI) const;
  BitVector findAvailableVRF(MachineInstr &MI) const;

  bool instrProducesUnspillableReg(const MachineInstr &MI) const;
};


TPCII::OpType getOpType(const MachineInstr &MI);
bool useImmSlotForImm(const MachineInstr &MI, int64_t Val);

bool isSET_INDX(unsigned opc);

}

#endif
