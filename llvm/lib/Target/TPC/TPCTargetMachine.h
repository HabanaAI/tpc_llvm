#ifndef _TPC_TARGET_MACHINE_H_
#define _TPC_TARGET_MACHINE_H_

#include "TPCInstrInfo.h"
#include "TPCSubtarget.h"
#include "llvm/IR/DataLayout.h"
#include "llvm/CodeGen/CodeGenTargetMachineImpl.h"
#include "llvm/Target/TargetMachine.h"
#include <optional>
#include "llvm/ADT/StringMap.h"
#include "llvm/Analysis/LoopAnalysisManager.h"
#include "llvm/Analysis/LoopPass.h"
#include "llvm/IR/PassManager.h"
#include <memory>
#include <optional>

namespace llvm {

class LPMUpdater;
class Module;

/// Pass "tpcopt".
class TpcLoopOptPass : public PassInfoMixin<TpcLoopOptPass> {
public:
  PreservedAnalyses run(Loop &L, LoopAnalysisManager &AM,
                        LoopStandardAnalysisResults &AR, LPMUpdater &U);
  // Never skipped.
  static bool isRequired() { return true; }
};


/// Pass "scalar2irf".
class TPCScalarToIRFPass : public PassInfoMixin<TPCScalarToIRFPass> {
  bool FoldCmp;

public:
  explicit TPCScalarToIRFPass(bool FoldCmp = true) : FoldCmp(FoldCmp) {}
  PreservedAnalyses run(Function &F, FunctionAnalysisManager &AM);
};

/// Pass "glbresolver".
class GlobalResolverPass : public PassInfoMixin<GlobalResolverPass> {
public:
  PreservedAnalyses run(Module &M, ModuleAnalysisManager &AM);
};

/// Pass "globalizer".
class GlobalizerPass : public PassInfoMixin<GlobalizerPass> {
public:
  PreservedAnalyses run(Module &M, ModuleAnalysisManager &AM);
};

FunctionPass *createTPCIndexMap();
void initializeTPCIndexMapPass(PassRegistry&);

FunctionPass *createTPCIndexGen();
void initializeTPCIndexGenPass(PassRegistry &);

/// Keeps vectors stored to VLM at constant addresses in registers
/// (TPCLocalMemoryForwarding.cpp).
FunctionPass *createTPCLocalMemoryForwarding();
void initializeTPCLocalMemoryForwardingPass(PassRegistry &);

/// Pass "tpclegal".
class TPCModuleLegalizerPass : public PassInfoMixin<TPCModuleLegalizerPass> {
public:
  PreservedAnalyses run(Module &M, ModuleAnalysisManager &AM);
  // Runs at all optimization levels.
  static bool isRequired() { return true; }
};

/// Pass "copyelide".
class TpcCopyElisionPass : public PassInfoMixin<TpcCopyElisionPass> {
public:
  PreservedAnalyses run(Function &F, FunctionAnalysisManager &AM);
};

/// Pass "tpcmem2reg".
class TPCMemoryToRegPass : public PassInfoMixin<TPCMemoryToRegPass> {
public:
  PreservedAnalyses run(Function &F, FunctionAnalysisManager &AM);
  // Runs at -O0, where functions are optnone.
  static bool isRequired() { return true; }
};

/// Pass "adjust-attr".
class AttributeAdjusterPass : public PassInfoMixin<AttributeAdjusterPass> {
public:
  PreservedAnalyses run(Function &F, FunctionAnalysisManager &AM);
  // Must run on optnone functions: removing optnone is its whole purpose.
  static bool isRequired() { return true; }
};

struct TPCEliminateRedundantLinearityPassConfig {
  bool HasFeatureTensorStorePack32To16 = false;
  bool HasFeatureTensorStorePack32To8 = false;
  bool HasFeatureTensorStorePack16To8 = false;
  bool HasFeatureTensorLoadUnpack = false;
};

/// Pass "tpc-eliminate-redundant-linearity".
class TPCEliminateRedundantLinearityPass
    : public PassInfoMixin<TPCEliminateRedundantLinearityPass> {
  TPCEliminateRedundantLinearityPassConfig Cfg;

public:
  explicit TPCEliminateRedundantLinearityPass(
      TPCEliminateRedundantLinearityPassConfig Cfg = {})
      : Cfg(Cfg) {}
  PreservedAnalyses run(Function &F, FunctionAnalysisManager &AM);
};

/// Pass "tpc-markup-direct-mmio-access".
class TPCMarkupDirectMMIOAccessPass
    : public PassInfoMixin<TPCMarkupDirectMMIOAccessPass> {
public:
  PreservedAnalyses run(Function &F, FunctionAnalysisManager &AM);
  // .tpc_metadata depends on it at all optimization levels.
  static bool isRequired() { return true; }
};

FunctionPass *createTPCPredicateOptimizer();
void initializeTPCPredicateOptimizerPass(PassRegistry &);

FunctionPass *createTPCIncMerger();
void initializeTPCIncMergerPass(PassRegistry &);

/// Pass "tpc-trans-intr".
class TPCTransformIntrinPass : public PassInfoMixin<TPCTransformIntrinPass> {
public:
  PreservedAnalyses run(Function &F, FunctionAnalysisManager &AM);
};

/// Pass "tpc-carry-pairs": joins each carry-reading add with the add before
/// it (see TPCCarryPairs.cpp).
class TPCCarryPairsPass : public PassInfoMixin<TPCCarryPairsPass> {
public:
  PreservedAnalyses run(Function &F, FunctionAnalysisManager &AM);
  // Needed for correct code at -O0 too.
  static bool isRequired() { return true; }
};

FunctionPass *createTPCHardwareLoops();
FunctionPass *createTPCOptimizeHWLoopsGuards();
void initializeTPCOptimizeHWLoopsGuardsPass(PassRegistry &);
FunctionPass *createTPCModuloScheduleSetup();
void initializeTPCModuloScheduleSetupPass(PassRegistry &);
FunctionPass *createTPCModuloScheduleFinish();
void initializeTPCModuloScheduleFinishPass(PassRegistry &);
FunctionPass *createTPCMulAddCombine();
void initializeTPCMulAddCombinePass(PassRegistry &);
FunctionPass *createTPCLoopBoundaryImm();
void initializeTPCLoopBoundaryImmPass(PassRegistry &);
FunctionPass *createTPCCarryDeps();
void initializeTPCCarryDepsPass(PassRegistry &);
FunctionPass *createTPCPipeliner();
FunctionPass *createTPCPacketizer();
FunctionPass *createTPCExpandHWRegCopies();
FunctionPass *createTPCUnHardwareLoops();
FunctionPass *createTPCSetSpillBase();
FunctionPass *createTPCSetIndxCoalescer();
#ifdef DEPRECATED_TPC_ADDR_OPT
FunctionPass *createTPCAddrOpt();
#endif
FunctionPass *createTPCHWWA2();
FunctionPass *createTPCRegisterBalancer();
FunctionPass *createTPCRegisterCounter();
FunctionPass *createTPCElfSpecSet();
FunctionPass *createTPCLutCacheCounter();
FunctionPass *createTPCPrologueMMIO();
FunctionPass *createTPCLatencyResolver();
FunctionPass *createTPCCostModelEmitter();
FunctionPass *createEventProfilerInject();
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
FunctionPass *createTPCSwapFakePredicate();

class TPCTargetMachine : public CodeGenTargetMachineImpl {
  std::unique_ptr<TargetLoweringObjectFile> TLOF;
  TPCSubtarget *Subtarget = nullptr;
  mutable StringMap<std::unique_ptr<TPCSubtarget>> SubtargetMap;

  TPCSubtarget *createSubtarget(StringRef CPU, StringRef FS) const;

public:
  TPCTargetMachine(const Target &T, const Triple &TT, StringRef CPU,
                   StringRef FS, const TargetOptions &Options,
                   std::optional<Reloc::Model> RM, std::optional<CodeModel::Model> CM,
                   CodeGenOptLevel OL, bool JIT);

  ~TPCTargetMachine() override;

  const TPCSubtarget *getSubtarget() const { return Subtarget; }
  const TPCSubtarget *getSubtargetImpl(const Function &F) const override;
  void registerPassBuilderCallbacks(PassBuilder &PB) override;
  void registerDefaultAliasAnalyses(AAManager &AAM) override;
  /// Subtarget features used by TPCEliminateRedundantLinearity.
  TPCEliminateRedundantLinearityPassConfig
  getEliminateRedundantLinearityConfig() const;
  TargetPassConfig *createPassConfig(PassManagerBase &PM) override;

  TargetLoweringObjectFile *getObjFileLowering() const override {
    return TLOF.get();
  }

  TargetTransformInfo getTargetTransformInfo(const Function &F) const override;

  MachineFunctionInfo *
  createMachineFunctionInfo(BumpPtrAllocator &Allocator, const Function &F,
                            const TargetSubtargetInfo *STI) const override;

  ScheduleDAGInstrs *
  createMachineScheduler(MachineSchedContext *C) const override;
  ScheduleDAGInstrs *
  createPostMachineScheduler(MachineSchedContext *C) const override;

 bool usesPhysRegsForValues() const override { return false; }
};

} // end namespace llvm

#endif
