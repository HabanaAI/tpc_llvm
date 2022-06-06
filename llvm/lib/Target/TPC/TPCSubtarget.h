#ifndef TPC_SUBTARGET_H
#define TPC_SUBTARGET_H

#include "TPCFrameLowering.h"
#include "TPCISelLowering.h"
#include "TPCInstrInfo.h"
#include "TPCRegisterInfo.h"
#include "TPCSelectionDAGInfo.h"
#include "llvm/ADT/SmallVector.h"
#include "llvm/ADT/StringRef.h"
#include "llvm/CodeGen/Register.h"
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

class TPCSubtarget : public TPCGenSubtargetInfo {
protected:
  TPCFrameLowering FrameLowering;
  TPCInstrInfo InstrInfo;
  TPCTargetLowering TLInfo;
  TPCSelectionDAGInfo TSInfo;

private:
  InstrItineraryData InstrItins;
  bool HasBFloat16Type = false;
  bool HasHalfFloatType = false;
  bool HasFP8Types = false;
  bool HasADRFSpills = false;
  bool HasCarry = false;
  bool HasPartial = false;
  bool HasMulI8 = false;
  bool HasAccI16 = false;
  bool HasAccI32 = false;
  bool HasX2Mac = false;
  bool HasZP = false;
  bool HasAddr1 = false;
  bool HasAddr2 = false;
  bool HasADRFMov = false;
  bool HasCompress = false;
  bool HasShortImm = false;
  bool HasDimMaskR = false;
  bool HasMADD = false;
  bool HasLdInStore = false;
  bool HasTnsrInReg = false;
  bool HasLdVectMask = false;
  bool HasRMW = false;
  bool HasTnsrPack = false;
  bool HasTnsrPackDT = false;
  bool HasTnsrUnpack = false;
  bool HasMovDGUnpack = false;
  bool HasL0CS = false;
  bool HasDirect = false;
  bool HasI4CVT = false;
  bool HasVRF2VPRF = false;
  bool HasGetHSRF = false;
  bool HasHWMov = false;
  bool HasGaudiISA = false;
  bool HasGaudiBISA = false;
  bool HasGoyaISA  = false;
  bool HasGrecoISA = false;
  bool HasGaudi2ISA = false;
  bool HasDoron1ISA = false;
  bool HasGen2Plus = false;
  bool HasGen2BPlus = false;
  bool HasGen3Plus = false;
  bool HasGen4Plus = false;
  bool HasDoron1 = false;
  bool HasConvertFpFlex = false;
  bool HasVectStMask = false;

  const int DefaultBigVLMSize = 80*1024;
  const int DefaultSmallVLMSize = 16*1024;


public:
    TPCSubtarget(const Triple&, const std::string, const std::string, const TargetMachine&);
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
  bool hasHalfFloat() const { return HasHalfFloatType; }
  bool hasFP8Types() const { return HasFP8Types; }
  bool hasCarry() const { return HasCarry; }
  bool hasPartial() const { return HasPartial; }
  bool hasMulI8() const { return HasMulI8; }
  bool hasAccI16() const { return HasAccI16; }
  bool hasAccI32() const { return HasAccI32; }
  bool hasX2Mac() const { return HasX2Mac; }
  bool hasZP() const { return HasZP; }
  bool hasAddr1() const { return HasAddr1; }
  bool hasAddr2() const { return HasAddr2; }
  bool hasADRFMov() const { return HasADRFMov; }
  bool hasCompress() const { return HasCompress; }
  bool hasDimMaskR() const { return HasDimMaskR; }
  bool hasShortImm() const { return HasShortImm; }
  bool hasMADD() const { return HasMADD; }
  bool hasLdInStore() const { return HasLdInStore; }
  bool hasTnsrInReg() const { return HasTnsrInReg; }
  bool hasLdVectMask() const { return HasLdVectMask; }
  bool hasRMW() const { return HasRMW; }
  bool hasTnsrPack() const { return HasTnsrPack; }
  bool hasTnsrPackDT() const { return HasTnsrPackDT; }
  bool hasTnsrUnpack() const { return HasTnsrUnpack; }
  bool hasMovDGUnpack() const { return HasMovDGUnpack; }
  bool hasL0CS() const { return HasL0CS; }
  bool hasDirect() const { return HasDirect; }
  bool hasI4CVT() const { return HasI4CVT; }
  bool hasVRF2VPRF() const { return HasVRF2VPRF; }
  bool hasGetHSRF() const { return HasGetHSRF; }
  bool hasHWMov() const { return HasHWMov; }
  bool hasGaudiISA() const { return HasGaudiISA; }
  bool hasGaudiBISA() const { return HasGaudiBISA; }
  bool hasGoyaISA()  const { return HasGoyaISA; }
  bool hasGrecoISA() const { return HasGrecoISA; }
  bool hasGaudi2ISA() const { return HasGaudi2ISA; }
  bool hasDoron1ISA() const { return HasDoron1ISA; }
  bool hasGen2Plus() const { return HasGen2Plus; }
  bool hasGen2BPlus() const { return HasGen2BPlus; }
  bool hasGen3Plus() const { return HasGen3Plus; }
  bool hasGen4Plus() const { return HasGen4Plus; }
  bool hasDoron1() const { return HasDoron1; }
  bool hasConvertFpFlex() const { return HasConvertFpFlex; }
  bool isPriorGen3() const { return HasGoyaISA || HasGaudiISA || HasGaudiBISA; }
  bool isPriorGen4() const { return HasGoyaISA || HasGaudiISA || HasGaudiBISA ||
                                    HasGrecoISA; }
  bool isPriorDoron1() const { return HasGoyaISA || HasGaudiISA || HasGaudiBISA ||
                                    HasGrecoISA || HasGaudi2ISA; }

  bool isGen3TillDoron1() const { return HasGrecoISA || HasGaudi2ISA; }

  bool hasVectStMask() const { return HasVectStMask; }
  int getDefaultBigVLMSize() const { return DefaultBigVLMSize; }
  int getDefaultSmallVLMSize() const { return DefaultSmallVLMSize; }
  unsigned getRoundCSRAddr() const;
  unsigned getConvRoundCSRAddr() const;

  /// Returns hardware SRF registers used for hardware loop iteration.
  ///
  /// \details For example:
  ///   * gaudi2 uses S32, S33, S34 and S35 registers for hardware loop
  ///     iterations.
  ///       - Start Reg = S32
  ///       - Final Reg = S35
  ///       - Regs = {S32, S33, S34, S35}
  Register getHWLoopStartReg() const;
  Register getHWLoopFinalReg() const;
  SmallVector<Register, 4> getHWLoopRegs() const;
  bool isHWLoopReg(Register Reg) const;

  void ParseSubtargetFeatures(StringRef CPU, StringRef TuneCPU, StringRef FS);
};

} // namespace llvm

#endif
