#ifndef LLVM_LIB_TARGET_TPC_X2_COMBINE_UTIL_H
#define LLVM_LIB_TARGET_TPC_X2_COMBINE_UTIL_H

//===---------------------- TPCx2CombineUtil.h :  -------------------------===//
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
// This file provides util class definition to combine Machine Instructions
// into x2 Machine Instructions.
//
//===----------------------------------------------------------------------===//

#include "llvm/CodeGen/MachineCombinerPattern.h"
#include "llvm/CodeGen/MachineInstrBuilder.h"
#include "llvm/CodeGen/RegisterClassInfo.h"

namespace llvm {

class TPCx2CombineUtil {
public:
  void resetMBBRegPressure(MachineBasicBlock *MBB,
                           RegisterClassInfo *RegClassInfo) const;
  bool findMACX2Pattern(MachineInstr &Root,
                        SmallVectorImpl<MachineCombinerPattern> &Patterns,
                        bool CheckVRegsPartOfOneDRFX2, bool CheckRegPressureX2,
                        bool AllowReorderForX2) const;
  void getMACX2Sequence(MachineInstr &Root,
                        SmallVectorImpl<MachineInstr *> &InsInstrs,
                        SmallVectorImpl<MachineInstr *> &DelInstrs) const;

private:
  bool canPair(MachineInstr &FistInstr, MachineInstr &SecondInstr,
               bool &HasUser) const;

  bool isX2Candidate(MachineInstr &I) const;
  void trackLiveDRFs(MachineRegisterInfo &MRI, MachineInstr *RootInst) const;
  bool isVRegPartOfDRF(MachineRegisterInfo &MRI, Register Reg,
                       Register &SourceDRF, unsigned &SubRegIndex) const;
  bool isVRegPartOfDRF(MachineRegisterInfo &MRI, Register Reg) const;
  bool areVRegsPartOfOneDRF(MachineRegisterInfo &MRI, MachineOperand Op1,
                            MachineOperand Op2) const;
  bool hasPackedRegs(MachineRegisterInfo &MRI, MachineInstr &I1,
                     MachineInstr &I2) const;
  bool hasDependence(MachineInstr *I1, MachineInstr *I2, bool &HasUser) const;
  bool hasSafeRegPressure() const;
  bool hasPerformancePotential(MachineRegisterInfo &MRI, MachineInstr &I1,
                               MachineInstr &I2, bool HasUser,
                               bool CheckVRegsPartOfOneDRFX2,
                               bool CheckRegPressureX2,
                               bool AllowReorderForX2) const;

  MachineInstrBuilder createDRFfromVRFs(MachineFunction &MF, DebugLoc Loc,
                                        Register &DRFReg, MachineOperand &VRF0,
                                        MachineOperand &VRF1) const;

  MachineInstrBuilder createMACx2(MachineFunction &MF, DebugLoc Loc,
                                  Register &ResultDRFReg, Register &DRF0,
                                  MachineOperand VRF1, MachineOperand VRF2,
                                  unsigned SwitchVal, Register &AccumDRFReg,
                                  Register &PredReg) const;

  static std::pair<MachineInstr *, MachineInstr *> X2CandidatePair;
  static SmallVector<MachineInstr *> X2MIsForReorder;

  static const unsigned X2PHIThreshold, X2BlockSizeThreshold;
  static const unsigned X2RegPressureThreshold;
  static unsigned X2VRFMaxPressure, X2DRFMaxPressure;
  static unsigned X2RegPressure, X2RegPressureDelta;
};

} // namespace llvm

#endif