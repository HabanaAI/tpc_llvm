//===-- TPCMachineScheduler.h ---- Custom TPC MI scheduler ----------------===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//
//
// Interface to custom TPC MI scheduler.
//
//===----------------------------------------------------------------------===//

#ifndef LLVM_LIB_TARGET_TPC_TPCMACHINESCHEDULER_H
#define LLVM_LIB_TARGET_TPC_TPCMACHINESCHEDULER_H

#include "llvm/CodeGen/MachineScheduler.h"
#include "llvm/CodeGen/ScheduleHazardRecognizer.h"
#include "llvm/CodeGen/DFAPacketizer.h"
#include "llvm/CodeGen/TargetInstrInfo.h"
#include "TPCSubtarget.h"
#include "TPCInstrInfo.h"

namespace llvm {
ScheduleDAGInstrs *createTPCMachineScheduler(MachineSchedContext *C);
ScheduleDAGInstrs *createTPCPostMachineScheduler(MachineSchedContext *C);
ScheduleHazardRecognizer *createTPCHazardRecognizer(const InstrItineraryData *II, const ScheduleDAG *DAG, bool postRA);
}

#endif
