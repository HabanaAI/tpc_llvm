//===---- TPCTargetMachine.h -------------------------------------------------------===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===-------------------------------------------------------------------------------===//
//
//===-------------------------------------------------------------------------------===//
#ifndef _TPC_TARGET_MACHINE_H_
#define _TPC_TARGET_MACHINE_H_

#include "TPCInstrInfo.h"
#include "TPCSubtarget.h"
#include "llvm/IR/DataLayout.h"
#include "llvm/Target/TargetMachine.h"
#include "llvm/ADT/Optional.h"
#include "llvm/ADT/StringMap.h"
#include "llvm/Analysis/LoopPass.h"

namespace llvm {

class Module;

LoopPass *createTpcLoopOptPass();
void initializeTpcLoopOptPass(PassRegistry&);


FunctionPass *createScalarToIRFPass(bool FoldCmp);
void initializeScalarToIRFPassPass(PassRegistry &);

ModulePass *createGlobalResolver();
void initializeGlobalResolverPass(PassRegistry&);

ModulePass *createGlobalizer();
void initializeGlobalizerPass(PassRegistry&);

FunctionPass *createTPCIndexMap();
void initializeTPCIndexMapPass(PassRegistry&);

FunctionPass *createTPCIndexGen();
void initializeTPCIndexGenPass(PassRegistry &);

FunctionPass *createNodePreLegalizer();
void initializeNodePreLegalizerPass(PassRegistry&);

FunctionPass *createTpcCopyElision();
void initializeTpcCopyElisionPass(PassRegistry&);

FunctionPass *createAttributeAdjuster();
void initializeAttributeAdjusterPass(PassRegistry&);

FunctionPass *createPromoteMemoryToRegsPass();
void initializePromoteMemoryToRegsPass(PassRegistry&);

FunctionPass *createTPCTransformIntrinPass();
FunctionPass *createTPCHardwareLoops();
FunctionPass *createTPCPipeliner();
FunctionPass *createTPCPacketizer();
FunctionPass *createTPCExpandHWRegCopies();
FunctionPass *createTPCUnHardwareLoops();
FunctionPass *createTPCSetSpillBase();
FunctionPass *createTPCSetIndxCoalescer();
FunctionPass *createTPCPredicateOptimizer();
FunctionPass *createTPCAddrOpt();
FunctionPass *createTPCHWWA2();
FunctionPass *createTPCRegisterBalancer();
FunctionPass *createTPCRegisterCounter();
FunctionPass *createTPCElfSpecSet();
FunctionPass *createTPCLutCacheCounter();
FunctionPass *createTPCLatencyResolver();
FunctionPass *createTPCCostModelEmitter();
FunctionPass *createTPCFMAoptPass();
FunctionPass *createTPCUnbranchPass();
FunctionPass *createTPCSelectorPreshaper();
FunctionPass *createTPCSingleUseScalarOptimizer();
FunctionPass *createTPCSubregInitElimination();
FunctionPass *createTPCPipelineRegs();
FunctionPass *createTPCInstrCompress();
FunctionPass *createTPCMemorySize();
FunctionPass *createTPCPreRAHwWorkarounds();
FunctionPass *createTPCHWWAGeneral();
FunctionPass *createTPCMovCoalescer();
FunctionPass *createTPCImmToReg();
FunctionPass *createTPCScalarSink();

class TPCTargetMachine : public LLVMTargetMachine {
  std::unique_ptr<TargetLoweringObjectFile> TLOF;
  TPCSubtarget *Subtarget = nullptr;
  mutable StringMap<std::unique_ptr<TPCSubtarget>> SubtargetMap;

  TPCSubtarget *createSubtarget(StringRef CPU, StringRef FS) const;

public:
  TPCTargetMachine(const Target &T, const Triple &TT, StringRef CPU,
                   StringRef FS, const TargetOptions &Options,
                   Optional<Reloc::Model> RM, Optional<CodeModel::Model> CM,
                   CodeGenOpt::Level OL, bool JIT);

  ~TPCTargetMachine() override;

  const TPCSubtarget *getSubtarget() const { return Subtarget; }
  const TPCSubtarget *getSubtargetImpl(const Function &F) const override;

  void adjustPassManager(PassManagerBuilder &) override;
  TargetPassConfig *createPassConfig(PassManagerBase &PM) override;

  TargetLoweringObjectFile *getObjFileLowering() const override {
    return TLOF.get();
  }

  TargetTransformInfo getTargetTransformInfo(const Function &F) override;

  bool usesPhysRegsForPEI() const override { return false; }
};

} // end namespace llvm

#endif
