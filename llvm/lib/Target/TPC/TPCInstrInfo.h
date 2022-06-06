//===-- TPCInstrInfo.h - TPC Instruction Information --------*- C++ -*-===//
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

#ifndef LLVM_LIB_TARGET_TPC_TPCINSTRINFO_H
#define LLVM_LIB_TARGET_TPC_TPCINSTRINFO_H

#include "MCTargetDesc/TPCMCInstrInfo.h"
#include "TPCRegisterInfo.h"
#include "TPCx2CombineUtil.h"
#include "llvm/ADT/SmallVector.h"
#include "llvm/CodeGen/MachineInstrBuilder.h"
#include "llvm/CodeGen/TargetInstrInfo.h"

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
  TPCx2CombineUtil m_tpcx2Util;
  virtual void anchor();
public:
  static MachineFunction *MachineFunctionX2;

  explicit TPCInstrInfo(TPCSubtarget &ST);

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
                           Register SrcReg, bool isKill, int FrameIndex,
                           const TargetRegisterClass *RC,
                           const TargetRegisterInfo *TRI) const override;

  void loadRegFromStackSlot(MachineBasicBlock &MBB,
                            MachineBasicBlock::iterator MBBI,
                            Register DestReg, int FrameIndex,
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

  bool useMachineCombiner() const override;
  bool isThroughputPattern(MachineCombinerPattern Pattern) const override;
  bool
  shouldReduceRegisterPressure(MachineBasicBlock *MBB,
                               RegisterClassInfo *RegClassInfo) const override;
  bool
  getMachineCombinerPatterns(MachineInstr &Root,
                             SmallVectorImpl<MachineCombinerPattern> &Patterns,
                             bool DoRegPressureReduce) const override;

  void genAlternativeCodeSequence(
      MachineInstr &Root, MachineCombinerPattern Pattern,
      SmallVectorImpl<MachineInstr *> &InsInstrs,
      SmallVectorImpl<MachineInstr *> &DelInstrs,
      DenseMap<unsigned, unsigned> &InstrIdxForVirtReg) const override;
  // Lower pseudo instructions after register allocation.
  bool expandPostRAPseudo(MachineInstr &MI) const override;

  bool isProfitableToHoist(MachineInstr &MI) const override;

  bool isReallyTriviallyReMaterializable(const MachineInstr &MI,
                                         AAResults *AA) const override;

  void reMaterialize(MachineBasicBlock &MBB,
                     MachineBasicBlock::iterator MI, Register DestReg,
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
                        const MachineInstr &UseMI,
                        unsigned UseIdx) const override;
  
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
  bool isRegisterClassUnspillable(const TargetRegisterClass *RC) const;

  unsigned convertHWRegToMemAddress(unsigned HWReg) const;
};


TPCII::OpType getOpType(const MachineInstr &MI);
bool useImmSlotForImm(const MachineInstr &MI, int64_t Val);

bool isSET_INDX(unsigned opc);

/// Whether CONVERT instruction has ALL_LANES switch.
bool isAllLanesVpuConvert(const MachineInstr &MI, const TPCSubtarget &TST);

/// Whether CONVERT_INT32 / CONVERT_UINT32 instruction has ALL_LANES switch.
bool isAllLanesVpuConvertI32(const MachineInstr &MI, const TPCSubtarget &TST);

/// Whether CONVERT_INT16 / CONVERT_UINT16 instruction has ALL_LANES switch.
bool isAllLanesVpuConvertI16(const MachineInstr &MI, const TPCSubtarget &TST);

const TargetRegisterClass *
getRegisterClass(Register Reg, const MachineRegisterInfo &MRI);

bool looksLikeHavingAPredicate(const MachineInstr &MI, int &PredicateIdx, bool &Polarity);
int getIncome(const MachineInstr &MI);
unsigned getSwitches(const MachineInstr &MI);
}

#endif
