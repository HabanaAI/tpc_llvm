#include "TPC.h"
#include "TPCAliasAnalysis.h"
#include "TPCMachineFunctionInfo.h"
#include "TPCSubtarget.h"
#include "TPCTargetMachine.h"
#include "TPCTargetObjectFile.h"
#include "TPCMachineScheduler.h"
#include "TPCTargetTransformInfo.h"
#include "llvm/ADT/STLExtras.h"
#include "llvm/MC/TargetRegistry.h"
#include "llvm/CodeGen/Passes.h"
#include "llvm/CodeGen/TargetPassConfig.h"
#include "llvm/IR/LegacyPassManager.h"
#include "llvm/MC/MCInstrInfo.h"
#include "llvm/MC/MCRegisterInfo.h"
#include "llvm/MC/MCSubtargetInfo.h"
#include "llvm/Target/TargetInfoTPC.h"
#include "llvm/Transforms/InstCombine/InstCombine.h"
#include "llvm/Transforms/IPO.h"
#include "llvm/Transforms/Scalar.h"
#include "llvm/Support/CodeGen.h"
#include "llvm/IR/Verifier.h"
#include "llvm/Passes/PassBuilder.h"
#include "llvm/Transforms/IPO/GlobalOpt.h"
#include "llvm/Transforms/IPO/LinkTPCHeaders.h"
#include "llvm/Transforms/Scalar/ADCE.h"
#include "llvm/Transforms/Scalar/DCE.h"
#include "llvm/Transforms/Scalar/LoopPassManager.h"
#include "llvm/Transforms/Scalar/SROA.h"
#include "llvm/Transforms/Scalar/SimplifyCFG.h"
#include <optional>

using namespace llvm;

extern Target TheTPCTarget;

extern "C" void LLVMInitializeTPCTarget() {
  // Register the target.
  RegisterTargetMachine<TPCTargetMachine> X(TheTPCTarget);

  // Register the target passes.
  PassRegistry &Registry = *PassRegistry::getPassRegistry();
//  initializeTPCAAWrapperPassPass(Registry);
  initializeTPCPredicateOptimizerPass(Registry);
  initializeTPCIncMergerPass(Registry);
  initializeTPCLocalMemoryForwardingPass(Registry);
  initializeTPCOptimizeHWLoopsGuardsPass(Registry);
  initializeTPCModuloScheduleSetupPass(Registry);
  initializeTPCModuloScheduleFinishPass(Registry);
  initializeTPCMulAddCombinePass(Registry);
  initializeTPCLoopBoundaryImmPass(Registry);
  initializeTPCCarryDepsPass(Registry);
}

static Reloc::Model getEffectiveRelocModel(std::optional<Reloc::Model> RM) {
  if (!RM.has_value())
    return Reloc::Static;
  return *RM;
}

/*
static CodeModel::Model getEffectiveCodeModel(std::optional<CodeModel::Model> CM) {
  if (!CM.hasValue())
    return CodeModel::Small;
  return CM.getValue();
}
*/

static StringRef normalizeCPUName(StringRef CPU) {
  if (CPU.empty())
    return "goya";
  if (CPU == "dali")
    return "goya";
  if (CPU == "goya2")
    return "greco";
  if (CPU == "gaudi3")
    return "doron1";
  return CPU;
}

// TODO: Research more into data layout format
TPCTargetMachine::TPCTargetMachine(const Target &T, const Triple &TT, StringRef CPU,
                                   StringRef FS, const TargetOptions &Options,
                                   std::optional<Reloc::Model> RM, std::optional<CodeModel::Model> CM,
                                   CodeGenOptLevel OL, bool JIT)
    : CodeGenTargetMachineImpl(T, DataLayoutStringTPC, TT,
                               normalizeCPUName(CPU), FS, Options,
                               getEffectiveRelocModel(RM),
                               getEffectiveCodeModel(CM, CodeModel::Kernel), OL),
      TLOF(std::make_unique<TPCTargetObjectFile>()) {
  setRequiresStructuredCFG(true);
  initAsmInfo();

  // Create default subtarget, which is calculated from CPU and Features.
  Subtarget = createSubtarget(getTargetCPU(), FS);

  Subtarget->getFrameLowering()->setMaxScalarMemory(Options.LocalScalarMemory);
  int MaxVLM = Options.LocalVectorMemory ? Options.LocalVectorMemory : Subtarget->getDefaultBigVLMSize();
  Subtarget->getFrameLowering()->setMaxVectorMemory(MaxVLM);
}

TPCTargetMachine::~TPCTargetMachine() {}


TPCSubtarget *TPCTargetMachine::createSubtarget(StringRef CPU,
                                                StringRef FS) const {
  // If subtarget object object has already been created, return it instead of
  // creating a new object. The subtarget object created with target machine
  // has correct properties, obtained from codegen options.
  //
  // NB! If TPC get function support some day, this method probably should be
  // updated to support per-function subtargets.
  if (Subtarget)
    return Subtarget;

  // For now we ignore features, they represent particular CPU cores.
  auto &I = SubtargetMap[CPU];
  if (!I) {
    // This needs to be done before we create a new subtarget since any
    // creation will depend on the TM and the code generation flags on the
    // function that reside in TargetOptions.
    I = std::make_unique<TPCSubtarget>(TargetTriple, CPU.str(), FS.str(), *this);
  }
  return I.get();
}


const TPCSubtarget *TPCTargetMachine::getSubtargetImpl(const Function& F) const {
  Attribute CPUAttr = F.getFnAttribute("target-cpu");
  Attribute FSAttr = F.getFnAttribute("target-features");

  std::string CPU = !CPUAttr.hasAttribute(Attribute::None)
                        ? CPUAttr.getValueAsString().str()
                        : TargetCPU;
  std::string FS = !FSAttr.hasAttribute(Attribute::None)
                       ? FSAttr.getValueAsString().str()
                       : TargetFS;
  return createSubtarget(CPU, FS);
}


TargetTransformInfo TPCTargetMachine::getTargetTransformInfo(const Function &F) const {
  return TargetTransformInfo(std::make_unique<TPCTTIImpl>(this, F));
}

ScheduleDAGInstrs *
TPCTargetMachine::createMachineScheduler(MachineSchedContext *C) const {
  return createTPCMachineScheduler(C);
}

ScheduleDAGInstrs *
TPCTargetMachine::createPostMachineScheduler(MachineSchedContext *C) const {
  return createTPCPostMachineScheduler(C);
}

static ImmutablePass *createTPCExternalAAWrapperPass() {
  return createExternalAAWrapperPass([](Pass &P, Function &, AAResults &AAR) {
      if (auto *WrapperPass = P.getAnalysisIfAvailable<TPCAAWrapperPass>())
        AAR.addAAResult(WrapperPass->getResult());
      });
}

namespace {
/// TPC Code Generator Pass Configuration Options.
class TPCPassConfig : public TargetPassConfig {
public:
  TPCPassConfig(TPCTargetMachine *TM, PassManagerBase &PM)
    : TargetPassConfig(*TM, PM) {
    substitutePass(&PostRASchedulerID, &PostMachineSchedulerID);
  }

  TPCTargetMachine &getTPCTargetMachine() const {
    return getTM<TPCTargetMachine>();
  }

  FunctionPass *createTargetRegisterAllocator(bool Optimized) override {
    // Use Greedy RA even for -O0.
    return createGreedyRegisterAllocator();
  }

  void addFastRegAlloc() override {
    addPass(createTPCExpandHWRegCopies());
    // We have to use optimizing RA even for -O0.
    addPass(createTPCExpandHWRegCopies());
    addPass(&LiveVariablesID);
    addPass(&PHIEliminationID);
    addPass(&TwoAddressInstructionPassID);
    addPass(&RegisterCoalescerID);

    // Add the selected register allocation pass.
    addPass(createRegAllocPass(true));

    // Allow targets to change the register assignments before rewriting.
    addPreRewrite();

    // Finally rewrite virtual registers.
    addPass(&VirtRegRewriterID);
  }

  void addOptimizedRegAlloc() override { 
    addPass(createTPCExpandHWRegCopies());
    TargetPassConfig::addOptimizedRegAlloc();
  }

  void addIRPasses() override;
  void addMachineSSAOptimization() override;
  bool addPreISel() override;
  bool addInstSelector() override;
  bool addILPOpts() override;
  void addPreRegAlloc() override;
  //void addPostRegAlloc() override;
  void addPreSched2() override;
  bool addGCPasses() override { return false; }

  void addPreEmitPass() override;
};
} // namespace

TPCEliminateRedundantLinearityPassConfig
TPCTargetMachine::getEliminateRedundantLinearityConfig() const {
  TPCEliminateRedundantLinearityPassConfig Cfg;
  Cfg.HasFeatureTensorStorePack32To16 = getSubtarget()->hasTnsrPack();
  Cfg.HasFeatureTensorStorePack32To8 = getSubtarget()->hasTnsrPackDT();
  Cfg.HasFeatureTensorStorePack16To8 = getSubtarget()->hasTnsrPackDT();
  Cfg.HasFeatureTensorLoadUnpack = getSubtarget()->hasTnsrUnpack();
  return Cfg;
}

// Pipeline integration; see llvm/docs/TPC/design/Passes.rst.
void TPCTargetMachine::registerDefaultAliasAnalyses(AAManager &AAM) {
  AAM.registerFunctionAnalysis<TPCAA>();
}

void TPCTargetMachine::registerPassBuilderCallbacks(PassBuilder &PB) {
  PB.registerAnalysisRegistrationCallback([](FunctionAnalysisManager &FAM) {
    FAM.registerPass([] { return TPCAA(); });
  });
  PB.registerParseAACallback([](StringRef Name, AAManager &AAM) {
    if (Name != "tpc-aa")
      return false;
    AAM.registerFunctionAnalysis<TPCAA>();
    return true;
  });

  // Pass names for opt -passes=..., same as the legacy pass arguments. Passes
  // created by name use the same defaults as the legacy passes created by name
  // (not the subtarget-derived settings used in the pipeline).
  PB.registerPipelineParsingCallback(
      [](StringRef Name, ModulePassManager &MPM,
         ArrayRef<PassBuilder::PipelineElement>) {
        if (Name == "tpclegal") {
          MPM.addPass(TPCModuleLegalizerPass());
          return true;
        }
        if (Name == "globalizer") {
          MPM.addPass(GlobalizerPass());
          return true;
        }
        if (Name == "glbresolver") {
          MPM.addPass(GlobalResolverPass());
          return true;
        }
        return false;
      });
  PB.registerPipelineParsingCallback(
      [](StringRef Name, FunctionPassManager &FPM,
         ArrayRef<PassBuilder::PipelineElement>) {
        if (Name == "copyelide") {
          FPM.addPass(TpcCopyElisionPass());
          return true;
        }
        if (Name == "tpc-markup-direct-mmio-access") {
          FPM.addPass(TPCMarkupDirectMMIOAccessPass());
          return true;
        }
        if (Name == "adjust-attr") {
          FPM.addPass(AttributeAdjusterPass());
          return true;
        }
        if (Name == "tpcmem2reg") {
          FPM.addPass(TPCMemoryToRegPass());
          return true;
        }
        if (Name == "tpc-trans-intr") {
          FPM.addPass(TPCTransformIntrinPass());
          return true;
        }
        if (Name == "tpc-carry-pairs") {
          FPM.addPass(TPCCarryPairsPass());
          return true;
        }
        if (Name == "scalar2irf") {
          FPM.addPass(TPCScalarToIRFPass());
          return true;
        }
        if (Name == "tpc-eliminate-redundant-linearity") {
          FPM.addPass(TPCEliminateRedundantLinearityPass());
          return true;
        }
        return false;
      });
  PB.registerPipelineParsingCallback(
      [](StringRef Name, LoopPassManager &LPM,
         ArrayRef<PassBuilder::PipelineElement>) {
        if (Name == "tpcopt") {
          LPM.addPass(TpcLoopOptPass());
          return true;
        }
        if (Name == "tpc-pragma-chain-unroll") {
          LPM.addPass(TPCPragmaChainUnrollPass());
          return true;
        }
        return false;
      });

  // Legacy EP_EarlyAsPossible.
  PB.registerPipelineStartEPCallback(
      [this](ModulePassManager &MPM, OptimizationLevel Level) {
        // TPCModuleLegalizer must be executed at all optimization levels.
        if (!Options.CompileForLibrary)
          MPM.addPass(TPCModuleLegalizerPass());
        FunctionPassManager FPM;
        if (!Options.CompileForLibrary)
          FPM.addPass(TpcCopyElisionPass());
        FPM.addPass(TPCMarkupDirectMMIOAccessPass());
        // Before any pass can separate a carry read from its add.
        FPM.addPass(TPCCarryPairsPass());
        if (Level != OptimizationLevel::O0) {
          FPM.addPass(SimplifyCFGPass());
          FPM.addPass(TPCTransformIntrinPass());
          FPM.addPass(SROAPass(SROAOptions::PreserveCFG));
          FPM.addPass(TPCScalarToIRFPass(getSubtarget()->hasGen3Plus()));
        }
        MPM.addPass(createModuleToFunctionPassAdaptor(std::move(FPM)));
#ifndef _WIN32
        // Legacy: first pass of PassManagerBuilder::populateModulePassManager.
        MPM.addPass(LinkTPCHeadersPass());
#endif
      });

  // Before the IR unrollers: a loop_unroll pragma loop the backend unroller
  // would refuse gets LLVM's unroll pragma instead.
  PB.registerLateLoopOptimizationsEPCallback(
      [](LoopPassManager &LPM, OptimizationLevel Level) {
        if (Level != OptimizationLevel::O0)
          LPM.addPass(TPCPragmaChainUnrollPass());
      });

  // Legacy EP_CGSCCOptimizerLate (GlobalOpt, ADCE, SROA) followed by
  // EP_ModuleOptimizerEarly (redundant linearity elimination).
  PB.registerOptimizerEarlyEPCallback(
      [this](ModulePassManager &MPM, OptimizationLevel Level,
             ThinOrFullLTOPhase) {
        if (Level == OptimizationLevel::O0)
          return;
        MPM.addPass(GlobalOptPass());
        FunctionPassManager FPM;
        FPM.addPass(ADCEPass());
        FPM.addPass(SROAPass(SROAOptions::PreserveCFG));
        FPM.addPass(TPCEliminateRedundantLinearityPass(
            getEliminateRedundantLinearityConfig()));
        MPM.addPass(createModuleToFunctionPassAdaptor(std::move(FPM)));
      });

  // Legacy EP_EnabledOnOptLevel0 (at -O0) and EP_OptimizerLast (otherwise).
  PB.registerOptimizerLastEPCallback(
      [this](ModulePassManager &MPM, OptimizationLevel Level,
             ThinOrFullLTOPhase) {
        FunctionPassManager FPM;
        if (Level == OptimizationLevel::O0) {
          // We cannot keep global pointers in memory, so promote them to
          // registers early.
          FPM.addPass(TPCMemoryToRegPass());
          // Remove 'optnone' so that instruction combining can run.
          FPM.addPass(AttributeAdjusterPass());
          // Instruction combining greatly simplifies instruction selection.
          FPM.addPass(InstCombinePass(InstCombineOptions().setMaxIterations(2)));
          MPM.addPass(createModuleToFunctionPassAdaptor(std::move(FPM)));
          if (!Options.CompileForLibrary) {
            MPM.addPass(GlobalizerPass());
            // The Globalizer leaves unused IR fragments behind.
            MPM.addPass(createModuleToFunctionPassAdaptor(ADCEPass()));
            MPM.addPass(GlobalResolverPass());
          }
          return;
        }
        FPM.addPass(createFunctionToLoopPassAdaptor(TpcLoopOptPass(),
                                                    /*UseMemorySSA=*/false));
        FPM.addPass(DCEPass());
        MPM.addPass(createModuleToFunctionPassAdaptor(std::move(FPM)));
        if (!Options.CompileForLibrary) {
          MPM.addPass(GlobalizerPass());
          MPM.addPass(GlobalResolverPass());
        }
      });
}

MachineFunctionInfo *TPCTargetMachine::createMachineFunctionInfo(
    BumpPtrAllocator &Allocator, const Function &F,
    const TargetSubtargetInfo *STI) const {
  return TPCMachineFunctionInfo::create<TPCMachineFunctionInfo>(Allocator, F,
                                                                STI);
}

TargetPassConfig *TPCTargetMachine::createPassConfig(PassManagerBase &PM) {
  return new TPCPassConfig(this, PM);
}

void TPCPassConfig::addIRPasses() {
  addPass(createTPCIndexGen());
  TargetPassConfig::addIRPasses();
  if (getOptLevel() != CodeGenOptLevel::None)
    addPass(createTPCLocalMemoryForwarding());

      addPass(createTPCAAWrapperPass());
      addPass(createExternalAAWrapperPass([](Pass &P, Function &,
                                             AAResults &AAR) {
        if (auto *WrapperPass = P.getAnalysisIfAvailable<TPCAAWrapperPass>())
          AAR.addAAResult(WrapperPass->getResult());
        }));

}

void TPCPassConfig::addMachineSSAOptimization() {
  if (getOptLevel() > CodeGenOptLevel::None)
    addPass(createTPCHWWAGeneral());
  TargetPassConfig::addMachineSSAOptimization();
  if (getOptLevel() > CodeGenOptLevel::None) {
    addPass(createTPCPredicateOptimizer());
    addPass(createTPCSetIndxCoalescer());
    addPass(createTPCIncMerger());
    addPass(&DeadMachineInstructionElimID);
  }
}

void TPCPassConfig::addPreEmitPass() {
  bool NoOpt = (getOptLevel() == CodeGenOptLevel::None);

  addPass(createTPCLutCacheCounter());
  addPass(createTPCPrologueMMIO());
  addPass(createTPCCarryDeps());
  // Create Packets.
#if !defined(TPC_DISABLE_PACKETIZER) && !defined(TPC_DISABLE_ALL_SCHED)
  if (!NoOpt) {
    addPass(createTPCPacketizer());
  }
#endif
   addPass(createTPCUnHardwareLoops());
#if !defined(TPC_DISABLE_NOP_INSERTER) && !defined(TPC_DISABLE_ALL_SCHED)
  addPass(createTPCLatencyResolver());
  if (getTM<TPCTargetMachine>().getSubtarget()->isPriorDoron1())
    addPass(createTPCPipelineRegs());
  addPass(createTPCCostModelEmitter());
  if (getTM<TPCTargetMachine>().getSubtarget()->hasGen4Plus())
    addPass(createEventProfilerInject());
#endif
  addPass(createTPCInstrCompress());
  addPass(createTPCRegisterCounter());
  addPass(createTPCElfSpecSet());
}

bool TPCPassConfig::addPreISel() {
  bool NoOpt = (getOptLevel() == CodeGenOptLevel::None);
  if (!NoOpt) {
    addPass(createTPCFMAoptPass());
    addPass(createTPCUnbranchPass());
  }
  addPass(createTPCSelectorPreshaper());
  addPass(createTPCSingleUseScalarOptimizer());
  addPass(createDeadCodeEliminationPass());
  return false;
}

bool TPCPassConfig::addInstSelector() {
  addPass(createTPCISelDag(getTPCTargetMachine(), getOptLevel()));
  addPass(createTPCCarryDeps());
  addPass(createTPCMemorySize());
  return false;
}

bool TPCPassConfig::addILPOpts() {
  // Before machine LICM and CSE, which then share and hoist the zero addends.
  if (getOptLevel() >= CodeGenOptLevel::Default)
    addPass(createTPCMulAddCombine());
  addPass(&MachineCombinerID);
  return true;
}
void TPCPassConfig::addPreRegAlloc() {
  addPass(createTPCSwapFakePredicate());
  addPass(createTPCScalarSink());
  if (getOptLevel() != CodeGenOptLevel::None)
    addBlockPlacement();
  if (getOptLevel() >= CodeGenOptLevel::Default) {
    addPass(createTPCImmToReg());
#ifdef DEPRECATED_TPC_ADDR_OPT
    addPass(createTPCAddrOpt());
#endif
    addPass(createTPCRegisterBalancer());
    addPass(&DeadMachineInstructionElimID);
    // Modulo scheduling, where TPCSubtarget::enableMachinePipeliner says.
    addPass(createTPCModuloScheduleSetup());
    addPass(&MachinePipelinerID);
    addPass(createTPCModuloScheduleFinish());
    addPass(&DeadMachineInstructionElimID);
    addPass(createTPCHardwareLoops());
    addPass(createTPCOptimizeHWLoopsGuards());
    addPass(createTPCPipeliner());
    addPass(&DeadMachineInstructionElimID);
    addPass(createTPCSubregInitElimination());
    // TODO: some kernels fail because of this pass
    addPass(createTPCMovCoalescer());
    // After every pass that makes or changes LOOPs.
    addPass(createTPCLoopBoundaryImm());
  }
  if (getOptLevel() == CodeGenOptLevel::None)
   addPass(createTPCHWWAGeneral());

  addPass(createTPCHWWA2());
  addPass(createTPCPreRAHwWorkarounds());
  // Before the pre-RA machine scheduler.
  addPass(createTPCCarryDeps());
}

void TPCPassConfig::addPreSched2()  {
  addPass(createTPCSetSpillBase());
  addPass(createTPCCarryDeps());
}
