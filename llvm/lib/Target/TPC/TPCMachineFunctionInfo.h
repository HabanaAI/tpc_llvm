//===- TPCMachineFunctionInfo.h - TPC per-function state --------*- C++ -*-===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

#ifndef LLVM_LIB_TARGET_TPC_TPCMACHINEFUNCTIONINFO_H
#define LLVM_LIB_TARGET_TPC_TPCMACHINEFUNCTIONINFO_H

#include "llvm/ADT/SmallPtrSet.h"
#include "llvm/CodeGen/MachineFunction.h"

namespace llvm {

/// What one TPC pass hands on to a later one about a function.
class TPCMachineFunctionInfo : public MachineFunctionInfo {
  /// The loops the MachinePipeliner keeps for the iterations the pipelined
  /// loop leaves (TPCModuloScheduling.cpp). They run a few times at most, so
  /// the machine unroller leaves them alone.
  SmallPtrSet<const MachineBasicBlock *, 4> PipelineRemainders;

public:
  TPCMachineFunctionInfo(const Function &F, const TargetSubtargetInfo *STI) {}

  void addPipelineRemainder(const MachineBasicBlock *MBB) {
    PipelineRemainders.insert(MBB);
  }
  bool isPipelineRemainder(const MachineBasicBlock *MBB) const {
    return PipelineRemainders.contains(MBB);
  }
};

} // namespace llvm

#endif
