add_llvm_component_group(TPC)

set(LLVM_TARGET_DEFINITIONS TPC.td)

tablegen(LLVM TPCGenAsmWriter.inc -gen-asm-writer)
tablegen(LLVM TPCGenAsmMatcher.inc -gen-asm-matcher)
tablegen(LLVM TPCGenRegisterInfo.inc -gen-register-info)
tablegen(LLVM TPCGenInstrInfo.inc -gen-instr-info)
tablegen(LLVM TPCGenSubtargetInfo.inc -gen-subtarget)
tablegen(LLVM TPCGenDAGISel.inc -gen-dag-isel)
tablegen(LLVM TPCGenDFAPacketizer.inc -gen-dfa-packetizer)
tablegen(LLVM TPCGenMCCodeEmitter.inc -gen-emitter)
tablegen(LLVM TPCGenCallingConv.inc -gen-callingconv)
tablegen(LLVM TPCGenDisassemblerTables.inc -gen-disassembler)

add_public_tablegen_target(TPCCommonTableGen)

set(sources
  AttributeAdjuster.cpp
  Globalizer.cpp
  GlobalResolver.cpp
  latencies.cpp
  latenciesGen1.cpp
  latenciesGen2.cpp
  latenciesGen3.cpp
  latenciesGen4.cpp
  latenciesDoron1.cpp
  MemoryToReg.cpp
  TPCModuleLegalizer.cpp
  ScalarToIRF.cpp
  TPCTargetMachine.cpp
  TPCTools.cpp
  TPCSubtarget.cpp
  TPCAsmPrinter.cpp
  TPCEliminateRedundantLinearity.cpp
  TPCFrameLowering.cpp
  TPCInstrInfo.cpp
  TPCAddrOpt.cpp
  TPCFMAOpt.cpp
  TPCUnbranch.cpp
  TPCHWWA2.cpp
  TPCHwWaGeneral.cpp
  TPCTransformIntrin.cpp
  TPCSetIndxCoalescer.cpp
  TPCISelDAGToDAG.cpp
  TPCISelLowering.cpp
  TPCRegisterInfo.cpp
  TPCRegisterCounter.cpp
  TPCLutCacheCounter.cpp
  TPCSelectionDAGInfo.cpp
  TPCTargetObjectFile.cpp
  TPCHardwareLoops.cpp
  TPCSelectorPreshaper.cpp
  TPCSingleUseScalarOptimizer.cpp
  TPCIndexSpaceGen.cpp
  TPCSoftwareLoopPipelining.cpp
  TPCMachineScheduler.cpp
  TPCHazardRecognizer.cpp
  TPCVLIWPacketizer.cpp
  TPCExpandHWRegCopies.cpp
  TPCUnHardwareLoops.cpp
  TPCSetSpillBase.cpp
  TPCExpandHWRegCopies.cpp
  TPCUnHardwareLoops.cpp
  TPCPredicateOptimizer.cpp
  TPCRegisterBalancer.cpp
  TPCLatencyResolver.cpp
  TPCSubregInitElimination.cpp
  TPCMapCompoundInst.cpp
  TPCTargetTransformInfo.cpp
  TPCInstrCompress.cpp
  TPCPipelineRegs.cpp
  TPCoptimization.cpp
  TPCMemorySize.cpp
  TPCAliasAnalysis.cpp
  TPCHwWa.cpp
  TPCElfSet.cpp
  TPCIndexSpace.cpp
  SCEVParser.cpp
  TPCCostModelEmitter.cpp
  TPCEventProfiler.cpp
  TPCLoopData.cpp
  TPCMachineInstrTools.cpp
  TPCMachineLoopDependencyAnalysis.cpp
  TPCMachineLoopLinearIV.cpp
  TPCMarkupDirectMMIOAccess.cpp
  TPCMovCoalescer.cpp
  TPCScalarSink.cpp
  TPCImmToReg.cpp
  TPCCopyElision.cpp
  TPCSwapFakePredicate.cpp
  TPCIncMerger.cpp
  TPCx2CombineUtil.cpp
  )

add_llvm_target(TPCCodeGen
  ${sources}

  LINK_COMPONENTS
  Analysis
  AsmPrinter
  CodeGen
  Core
  IPO
  MC
  SelectionDAG
  Support
  Target
  TPCDesc
  TPCInfo

  ADD_TO_COMPONENT
  TPC
  )

add_subdirectory(MCTargetDesc)
add_subdirectory(TargetInfo)
add_subdirectory(AsmParser)
add_subdirectory(Disassembler)
