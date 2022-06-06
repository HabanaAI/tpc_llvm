add_llvm_component_library(LLVMipo
  AlwaysInliner.cpp
  Annotation2Metadata.cpp
  ArgumentPromotion.cpp
  Attributor.cpp
  AttributorAttributes.cpp
  BarrierNoopPass.cpp
  BlockExtractor.cpp
  CalledValuePropagation.cpp
  ConstantMerge.cpp
  CrossDSOCFI.cpp
  DeadArgumentElimination.cpp
  ElimAvailExtern.cpp
  ExtractGV.cpp
  ForceFunctionAttrs.cpp
  FunctionAttrs.cpp
  FunctionImport.cpp
  GlobalDCE.cpp
  GlobalOpt.cpp
  GlobalSplit.cpp
  HotColdSplitting.cpp
  IPO.cpp
  IROutliner.cpp
  InferFunctionAttrs.cpp
  InlineSimple.cpp
  Inliner.cpp
  Internalize.cpp
  LoopExtractor.cpp
  LowerTypeTests.cpp
  MergeFunctions.cpp
  OpenMPOpt.cpp
  PartialInlining.cpp
  PassManagerBuilder.cpp
  PruneEH.cpp
  SampleContextTracker.cpp
  SampleProfile.cpp
  SampleProfileProbe.cpp
  SCCP.cpp
  StripDeadPrototypes.cpp
  StripSymbols.cpp
  SyntheticCountsPropagation.cpp
  ThinLTOBitcodeWriter.cpp
  WholeProgramDevirt.cpp
  LinkTPCFunctionsMMIOAccess.cpp
  LinkTPCFunctionsRNG.cpp
  LinkTPCHeaders.cpp

  ADDITIONAL_HEADER_DIRS
  ${LLVM_MAIN_INCLUDE_DIR}/llvm/Transforms
  ${LLVM_MAIN_INCLUDE_DIR}/llvm/Transforms/IPO

  DEPENDS
  intrinsics_gen
  omp_gen

  COMPONENT_NAME
  IPO

  LINK_COMPONENTS
  AggressiveInstCombine
  Analysis
  BitReader
  BitWriter
  Core
  FrontendOpenMP
  InstCombine
  IRReader
  Linker
  Object
  ProfileData
  Scalar
  Support
  TransformUtils
  Vectorize
  Instrumentation
  )

set(gaudi_tpc_header_files
TPCHeaders/gaudi/sin.bc
)

set(output_dir ${CMAKE_CURRENT_BINARY_DIR})
set(out_files)

function(copy_header_to_output_dir src_dir file)
  set(src ${src_dir}/${file})
  set(dst ${output_dir}/${file})
  add_custom_command(OUTPUT ${dst}
    DEPENDS ${src}
    COMMAND ${CMAKE_COMMAND} -E copy_if_different ${src} ${dst}
    COMMENT "Copying clang's ${file}...")
  list(APPEND out_files ${dst})
  set(out_files ${out_files} PARENT_SCOPE)
endfunction(copy_header_to_output_dir)

# Copy header files from the source directory to the build directory
foreach( f ${files} ${gaudi_tpc_header_files} )
  copy_header_to_output_dir(${CMAKE_CURRENT_SOURCE_DIR} ${f})
endforeach( f )
