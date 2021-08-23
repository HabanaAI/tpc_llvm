//===---- TPCTargetMachine.cpp -----------------------------------------------------===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===-------------------------------------------------------------------------------===//
//
//===-------------------------------------------------------------------------------===//
#include "TPC.h"
#include "TPCAliasAnalysis.h"
#include "TPCSubtarget.h"
#include "TPCTargetMachine.h"
#include "TPCTargetObjectFile.h"
#include "TPCMachineScheduler.h"
#include "TPCTargetTransformInfo.h"
#include "llvm/ADT/STLExtras.h"
#include "llvm/Support/TargetRegistry.h"
#include "llvm/CodeGen/Passes.h"
#include "llvm/CodeGen/TargetPassConfig.h"
#include "llvm/IR/LegacyPassManager.h"
#include "llvm/MC/MCInstrInfo.h"
#include "llvm/MC/MCRegisterInfo.h"
#include "llvm/MC/MCSubtargetInfo.h"
#include "llvm/Target/TargetInfoTPC.h"
#include "llvm/Transforms/InstCombine/InstCombine.h"
#include "llvm/Transforms/IPO.h"
#include "llvm/Transforms/IPO/PassManagerBuilder.h"
#include "llvm/Transforms/Scalar.h"
#include "llvm/Support/CodeGen.h"
#include "llvm/IR/Verifier.h"

using namespace llvm;

extern Target TheTPCTarget;

extern "C" void LLVMInitializeTPCTarget() {
  // Register the target.
  RegisterTargetMachine<TPCTargetMachine> X(TheTPCTarget);

  // Register the target passes.
  PassRegistry &Registry = *PassRegistry::getPassRegistry();
  initializeScalarToIRFPassPass(Registry);
  initializeGlobalResolverPass(Registry);
  initializeGlobalizerPass(Registry);
  initializeNodePreLegalizerPass(Registry);
  initializeAttributeAdjusterPass(Registry);
  initializePromoteMemoryToRegsPass(Registry);
  initializeTpcLoopOptPass(Registry);
//  initializeTPCAAWrapperPassPass(Registry);
  initializeTpcCopyElisionPass(Registry);
}

static Reloc::Model getEffectiveRelocModel(Optional<Reloc::Model> RM) {
  if (!RM.hasValue())
    return Reloc::Static;
  return *RM;
}

static CodeModel::Model getEffectiveCodeModel(Optional<CodeModel::Model> CM) {
  if (!CM.hasValue())
    return CodeModel::Small;
  return CM.getValue();
}

static StringRef normalizeCPUName(StringRef CPU) {
  if (CPU.empty())
    return "goya";
  if (CPU == "dali")
    return "goya";
  return CPU;
}

// TODO: Research more into data layout format
TPCTargetMachine::TPCTargetMachine(const Target &T, const Triple &TT, StringRef CPU,
                                   StringRef FS, const TargetOptions &Options,
                                   Optional<Reloc::Model> RM, Optional<CodeModel::Model> CM,
                                   CodeGenOpt::Level OL, bool JIT)
  :  LLVMTargetMachine(T, DataLayoutStringTPC, TT, normalizeCPUName(CPU), FS,
                       Options, getEffectiveRelocModel(RM), getEffectiveCodeModel(CM, CodeModel::Kernel), OL),
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
    I = std::make_unique<TPCSubtarget>(TargetTriple, CPU, FS, *this);
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


TargetTransformInfo TPCTargetMachine::getTargetTransformInfo(const Function &F) {
  return TargetTransformInfo(TPCTTIImpl(this, F));
}

static ImmutablePass *createTPCExternalAAWrapperPass() {
  return createExternalAAWrapperPass([](Pass &P, Function &, AAResults &AAR) {
      if (auto *WrapperPass = P.getAnalysisIfAvailable<TPCAAWrapperPass>())
        AAR.addAAResult(WrapperPass->getResult());
      });
}

void TPCTargetMachine::adjustPassManager(PassManagerBuilder &PMB) {
  // Prelegalizer replaces unsupported nodes with function calls. As TPC does
  // not have functions, we must do it before inliner, it is too early, we
  // have no chance to apply generic optimizations.
  //
  // Prelegalizer must be executed at all optimization levels.
  //
  PMB.addExtension(
    PassManagerBuilder::EP_EarlyAsPossible,
    [&](const PassManagerBuilder &, legacy::PassManagerBase &PM) {
    PM.add(createNodePreLegalizer());
    PM.add(createTpcCopyElision());
  });

  // Passes specific for -O0.
  if (getOptLevel() == CodeGenOpt::None) {
    // We cannot keep global pointers in memory, so promote them to registers early.
    PMB.addExtension(
      PassManagerBuilder::EP_EnabledOnOptLevel0,
      [&](const PassManagerBuilder &, legacy::PassManagerBase &PM) {
        PM.add(createPromoteMemoryToRegsPass());
    });

    // Remove attribute 'OptimizeNone' so that instruction combining can be
    // called.
    PMB.addExtension(
      PassManagerBuilder::EP_EnabledOnOptLevel0,
      [&](const PassManagerBuilder &, legacy::PassManagerBase &PM) {
        PM.add(createAttributeAdjuster());
    });

    // Instruction combiner greatly simplifies selector job by making generic
    // transformations, although it can make generation of debug info more
    // complex.
    PMB.addExtension(
      PassManagerBuilder::EP_EnabledOnOptLevel0,
      [&](const PassManagerBuilder &, legacy::PassManagerBase &PM) {
        PM.add(createInstructionCombiningPass(false/*ExpensiveCombines*/));
    });

    // At -O0 IR pipeline is short and we must call Globalizer and GlobalResolver
    // here, as extension point EP_OptimizerLast is not available.
    PMB.addExtension(
      PassManagerBuilder::EP_EnabledOnOptLevel0,
      [&](const PassManagerBuilder &, legacy::PassManagerBase &PM) {
        PM.add(createGlobalizer());
    });
    // Globalizer leaves hanging IR fragments that have no uses. Remove them now.
    PMB.addExtension(
      PassManagerBuilder::EP_EnabledOnOptLevel0,
      [&](const PassManagerBuilder &, legacy::PassManagerBase &PM) {
        PM.add(createAggressiveDCEPass());
    });
    PMB.addExtension(
      PassManagerBuilder::EP_EnabledOnOptLevel0,
      [&](const PassManagerBuilder &, legacy::PassManagerBase &PM) {
        PM.add(createGlobalResolver());
    });
  }

  // Optimization passes.
  else {
    PMB.addExtension(
      PassManagerBuilder::EP_EarlyAsPossible,
      [&](const PassManagerBuilder &, legacy::PassManagerBase &PM) {
        PM.add(createCFGSimplificationPass());
    });
/*  Not time due to store instruction
    PMB.addExtension(
      PassManagerBuilder::EP_EarlyAsPossible,
      [&](const PassManagerBuilder &, legacy::PassManagerBase &PM) {
        PM.add(createTPCUnbranchPass());
    });
*/
    PMB.addExtension(
      PassManagerBuilder::EP_EarlyAsPossible,
      [&](const PassManagerBuilder &, legacy::PassManagerBase &PM) {
        PM.add(createTPCTransformIntrinPass());
    });

    PMB.addExtension(
      PassManagerBuilder::EP_EarlyAsPossible,
      [&](const PassManagerBuilder &, legacy::PassManagerBase &PM) {
        PM.add(createSROAPass());
    });

    PMB.addExtension(
      PassManagerBuilder::EP_EarlyAsPossible,
      [&](const PassManagerBuilder &, legacy::PassManagerBase &PM) {
        PM.add(createScalarToIRFPass(false));
    });


    PMB.addExtension(
      PassManagerBuilder::EP_CGSCCOptimizerLate,
      [&](const PassManagerBuilder &, legacy::PassManagerBase &PM) {
        PM.add(createGlobalOptimizerPass());
    });

    PMB.addExtension(
      PassManagerBuilder::EP_CGSCCOptimizerLate,
      [&](const PassManagerBuilder &, legacy::PassManagerBase &PM) {
        PM.add(createAggressiveDCEPass());
    });

    PMB.addExtension(
      PassManagerBuilder::EP_CGSCCOptimizerLate,
      [&](const PassManagerBuilder &, legacy::PassManagerBase &PM) {
        PM.add(createSROAPass());
    });

    PMB.addExtension(
            PassManagerBuilder::EP_OptimizerLast,
            [&](const PassManagerBuilder &, legacy::PassManagerBase &PM) {
                PM.add(createTpcLoopOptPass());
            });
    PMB.addExtension(
            PassManagerBuilder::EP_OptimizerLast,
            [&](const PassManagerBuilder &, legacy::PassManagerBase &PM) {
                PM.add(createDeadCodeEliminationPass());
            });

    PMB.addExtension(
      PassManagerBuilder::EP_OptimizerLast,
      [&](const PassManagerBuilder &, legacy::PassManagerBase &PM) {
        PM.add(createGlobalizer());
        //PM.add(createVerifierPass());
    });

    PMB.addExtension(
      PassManagerBuilder::EP_OptimizerLast,
      [&](const PassManagerBuilder &, legacy::PassManagerBase &PM) {
        PM.add(createGlobalResolver());
        //PM.add(createVerifierPass());
    });

  PMB.addExtension(
    PassManagerBuilder::EP_ModuleOptimizerEarly,
    [&](const PassManagerBuilder &, legacy::PassManagerBase &PM) {
        PM.add(createTPCAAWrapperPass());
        PM.add(createTPCExternalAAWrapperPass());
  });
  PMB.addExtension(
    PassManagerBuilder::EP_EarlyAsPossible,
    [&](const PassManagerBuilder &, legacy::PassManagerBase &PM) {
        PM.add(createTPCAAWrapperPass());
        PM.add(createTPCExternalAAWrapperPass());
  });

  }
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
    addPass(&LiveVariablesID, false);
    addPass(&PHIEliminationID, false);
    addPass(&TwoAddressInstructionPassID, false);
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

  ScheduleDAGInstrs *
  createMachineScheduler(MachineSchedContext *C) const override {
    return createTPCMachineScheduler(C);
  }

// TODO: Implement postRA strategy and enable this scheduler
#if 1
  ScheduleDAGInstrs *
  createPostMachineScheduler(MachineSchedContext *C) const override {
    return createTPCPostMachineScheduler(C);
  }
#endif

  void addIRPasses() override;
  void addMachineSSAOptimization() override;
  bool addPreISel() override;
  bool addInstSelector() override;
  void addPreRegAlloc() override;
  //void addPostRegAlloc() override;
  void addPreSched2() override;
  bool addGCPasses() override { return false; }

  void addPreEmitPass() override;
};
} // namespace

TargetPassConfig *TPCTargetMachine::createPassConfig(PassManagerBase &PM) {
  return new TPCPassConfig(this, PM);
}

void TPCPassConfig::addIRPasses() {
  addPass(createTPCIndexGen());
  TargetPassConfig::addIRPasses();

      addPass(createTPCAAWrapperPass());
      addPass(createExternalAAWrapperPass([](Pass &P, Function &,
                                             AAResults &AAR) {
        if (auto *WrapperPass = P.getAnalysisIfAvailable<TPCAAWrapperPass>())
          AAR.addAAResult(WrapperPass->getResult());
        }));

}

void TPCPassConfig::addMachineSSAOptimization() {
  if (getOptLevel() > CodeGenOpt::None)
    addPass(createTPCHWWAGeneral());
  TargetPassConfig::addMachineSSAOptimization();
  if (getOptLevel() > CodeGenOpt::None) {
    addPass(createTPCPredicateOptimizer());
    addPass(createTPCSetIndxCoalescer());
    addPass(&DeadMachineInstructionElimID);
  }
}

void TPCPassConfig::addPreEmitPass() {
  bool NoOpt = (getOptLevel() == CodeGenOpt::None);

  addPass(createTPCLutCacheCounter(), false);
  // Create Packets.
#if !defined(TPC_DISABLE_PACKETIZER) && !defined(TPC_DISABLE_ALL_SCHED)
  if (!NoOpt) {
    addPass(createTPCPacketizer(), false);
  }
#endif
   addPass(createTPCUnHardwareLoops());
#if !defined(TPC_DISABLE_NOP_INSERTER) && !defined(TPC_DISABLE_ALL_SCHED)
  addPass(createTPCLatencyResolver(), false);
  addPass(createTPCPipelineRegs(), false);
  addPass(createTPCCostModelEmitter(), false);
#endif
  addPass(createTPCInstrCompress(), false);
  addPass(createTPCRegisterCounter(), false);
  addPass(createTPCElfSpecSet(), false);
}

bool TPCPassConfig::addPreISel() {
  bool NoOpt = (getOptLevel() == CodeGenOpt::None);
  if (!NoOpt) {
    addPass(createTPCFMAoptPass());
    addPass(createTPCUnbranchPass());
  }
  addPass(createTPCSelectorPreshaper());
  addPass(createTPCSingleUseScalarOptimizer());
  addPass(createAggressiveDCEPass());
  return false;
}

bool TPCPassConfig::addInstSelector() {
  addPass(createTPCISelDag(getTPCTargetMachine(), getOptLevel()));
  addPass(createTPCMemorySize());
  return false;
}

void TPCPassConfig::addPreRegAlloc() {
  addPass(createTPCScalarSink());
  if (getOptLevel() != CodeGenOpt::None)
    addBlockPlacement();
  if (getOptLevel() >= CodeGenOpt::Default) {
    addPass(createTPCImmToReg());
    addPass(createTPCAddrOpt());
    addPass(createTPCRegisterBalancer());
    addPass(&DeadMachineInstructionElimID);
    addPass(createTPCHardwareLoops(), false);
    addPass(createTPCPipeliner(), false);
    addPass(&DeadMachineInstructionElimID);
    addPass(createTPCSubregInitElimination());
    // TODO: some kernels fail because of this pass
    //addPass(createTPCMovCoalescer());
  }
  if (getOptLevel() == CodeGenOpt::None)
   addPass(createTPCHWWAGeneral());

  addPass(createTPCHWWA2());
  addPass(createTPCPreRAHwWorkarounds());
}

void TPCPassConfig::addPreSched2()  {
  addPass(createTPCSetSpillBase(), false);
}
