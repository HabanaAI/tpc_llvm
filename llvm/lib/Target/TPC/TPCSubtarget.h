//===---- TPCSubtarget.h ---------------------------------------------------------- -===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===-------------------------------------------------------------------------------===//
//
//===-------------------------------------------------------------------------------===//
#ifndef TPC_SUBTARGET_H
#define TPC_SUBTARGET_H

#include "TPCFrameLowering.h"
#include "TPCISelLowering.h"
#include "TPCInstrInfo.h"
#include "TPCRegisterInfo.h"
#include "TPCSelectionDAGInfo.h"
#include "llvm/CodeGen/TargetSubtargetInfo.h"
#include <string>

#define GET_SUBTARGETINFO_HEADER
#include "TPCGenSubtargetInfo.inc"

//#define TPC_DISABLE_ALL_SCHED    1

//#define TPC_DISABLE_POSTRA_SCHED 1
//#define TPC_DISABLE_MISCHED      1
//#define TPC_DISABLE_PACKETIZER   1
//#define TPC_NOPS_AFTER_ALL       1

namespace llvm {

class StringRef;

//---------------------------------------------------------------------------//

class TPCSubtarget : public TPCGenSubtargetInfo {
protected:
  TPCFrameLowering FrameLowering;
  TPCInstrInfo InstrInfo;
  TPCTargetLowering TLInfo;
  TPCSelectionDAGInfo TSInfo;

private:
  InstrItineraryData InstrItins;
  bool HasBFloat16Type = false;
  bool HasCarry = false;
  bool HasPartial = false;
  bool HasMulI8 = false;
  bool HasAddr1 = false;
  bool HasAddr2 = false;
  bool HasLdVectMask = false;
  bool HasRMW = false;
  bool HasTnsrPack = false;
  bool HasGetHSRF = false;
  bool HasGaudiISA = false;
  bool HasGoyaISA  = false;
  bool HasGen2Plus = false;

  const int DefaultBigVLMSize = 80*1024;
  const int DefaultSmallVLMSize = 16*1024;


public:
    TPCSubtarget(const Triple&, const std::string&, const std::string&, const TargetMachine&);
    void ParseSubtargetFeatures(StringRef, StringRef);
    bool is64Bit() const { return false; }

    // getInstrItins - Return the instruction itineraries based on subtarget selection.
    const InstrItineraryData *getInstrItineraryData() const override {
      return &InstrItins;
    }

    const TPCSelectionDAGInfo *getSelectionDAGInfo() const override {
      return &TSInfo;
    }
    const TPCFrameLowering *getFrameLowering() const override {
      return &FrameLowering;
    }

    const TPCTargetLowering *getTargetLowering() const override {
      return &TLInfo;
    }

    const TPCInstrInfo* getInstrInfo() const override {
      return &InstrInfo;
    }

    const TPCRegisterInfo *getRegisterInfo() const override {
      return &getInstrInfo()->getRegisterInfo();
    }

    bool enableSubRegLiveness() const override;

    // Let the DAG builder use Alias Analysis for memory disambiguation.
    bool useAA() const override { return true; }

#if defined(TPC_DISABLE_MISCHED) || defined(TPC_DISABLE_ALL_SCHED)
  bool enableMachineScheduler() const override { return false; }
#else
  bool enableMachineScheduler() const override { return true; }
#endif
  bool enableMachineSchedDefaultSched() const override { return false; }
#if defined(TPC_DISABLE_POSTRA_SCHED) || defined(TPC_DISABLE_ALL_SCHED)
  bool enablePostRAScheduler() const override { return false; }
#else
  bool enablePostRAScheduler() const override { return true; }
  CodeGenOpt::Level getOptLevelToEnablePostRAScheduler() const override { return CodeGenOpt::None; }
#endif
  class TPCDAGMutation : public ScheduleDAGMutation {
  public:
    void apply(ScheduleDAGInstrs *DAG) override;
  };
/*
  void getPostRAMutations(
      std::vector<std::unique_ptr<ScheduleDAGMutation>> &Mutations)
      const override;

  void getSMSMutations(
      std::vector<std::unique_ptr<ScheduleDAGMutation>> &Mutations)
      const override;
*/

  bool hasBFloat16() const { return HasBFloat16Type; }
  bool hasCarry() const { return HasCarry; }
  bool hasPartial() const { return HasPartial; }
  bool hasMulI8() const { return HasMulI8; }
  bool hasAddr1() const { return HasAddr1; }
  bool hasAddr2() const { return HasAddr2; }
  bool hasLdVectMask() const { return HasLdVectMask; }
  bool hasRMW() const { return HasRMW; }
  bool hasTnsrPack() const { return HasTnsrPack; }
  bool hasGetHSRF() const { return HasGetHSRF; }
  bool hasGaudiISA() const { return HasGaudiISA; }
  bool hasGoyaISA()  const { return HasGoyaISA; }
  bool hasGen2Plus() const { return HasGen2Plus; }
  int getDefaultBigVLMSize() const { return DefaultBigVLMSize; }
  int getDefaultSmallVLMSize() const { return DefaultSmallVLMSize; }

  unsigned getRoundCSRAddr() const;
  unsigned getConvRoundCSRAddr() const;
};

} // namespace llvm

#endif
