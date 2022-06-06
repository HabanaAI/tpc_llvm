add_subdirectory(Rewrite)

set(LLVM_LINK_COMPONENTS
  BitReader
  BitstreamReader
  Option
  ProfileData
  Support
  )

# LLVM_TPC_COMPILER
#

if("TPC" IN_LIST LLVM_TARGETS_TO_BUILD)
    list(APPEND LLVM_LINK_COMPONENTS TPCDesc)
endif()

set(converter ${LLVM_MAIN_SRC_DIR}/utils/TPC/embed_file.py)

# Original header files are kept here.
set(embeddable_files_dir ${CLANG_SOURCE_DIR}/lib/Headers)

# Set of header files to embed.
set(embedded_files
  tpc-defs.h
  tpc-reduction_functions_core.h
  tpc-special.h
)

# Create files with embedded sources.
set(embedded_files_list)
foreach(file ${embedded_files})
  set(embedded_file "${CMAKE_CURRENT_BINARY_DIR}/${file}.inc")
  set(file_to_embed "${embeddable_files_dir}/${file}")
  list(APPEND embedded_files_list "${embedded_file}")
  add_custom_command(
    OUTPUT ${embedded_file}
    DEPENDS ${file_to_embed} ${converter}
    COMMAND
      ${PYTHON_EXECUTABLE} "${converter}" "${embedded_file}" "${file_to_embed}"
  )
endforeach()
set_source_files_properties(${embedded_files_list}
  PROPERTIES GENERATED TRUE
)

# Create files with IR libraries.
foreach(file ${tpclib_bitcode_files})
  set(filename)
  get_filename_component(filename ${file} NAME)
  set(embedded_file "${CMAKE_CURRENT_BINARY_DIR}/${filename}.inc")
  list(APPEND embedded_libraries_list "${embedded_file}")
  add_custom_command(
    OUTPUT "${embedded_file}"
    COMMAND ${CMAKE_COMMAND} -E touch ${embedded_file}
  )
endforeach()
set_source_files_properties(${embedded_libraries_list}
  PROPERTIES GENERATED TRUE
)
#
# /LLVM_TPC_COMPILER

add_clang_library(clangFrontend
  ASTConsumers.cpp
  ASTMerge.cpp
  ASTUnit.cpp
  ChainedDiagnosticConsumer.cpp
  ChainedIncludesSource.cpp
  CompilerInstance.cpp
  CompilerInvocation.cpp
  CreateInvocationFromCommandLine.cpp
  DependencyFile.cpp
  DependencyGraph.cpp
  DiagnosticRenderer.cpp
  EmbeddedTpcHeaders.cpp
  FrontendAction.cpp
  FrontendActions.cpp
  FrontendOptions.cpp
  HeaderIncludeGen.cpp
  InitHeaderSearch.cpp
  InitPreprocessor.cpp
  LayoutOverrideSource.cpp
  LogDiagnosticPrinter.cpp
  ModuleDependencyCollector.cpp
  MultiplexConsumer.cpp
  PrecompiledPreamble.cpp
  PrintPreprocessedOutput.cpp
  SerializedDiagnosticPrinter.cpp
  SerializedDiagnosticReader.cpp
  TestModuleFileExtension.cpp
  TextDiagnostic.cpp
  TextDiagnosticBuffer.cpp
  TextDiagnosticPrinter.cpp
  VerifyDiagnosticConsumer.cpp
  InterfaceStubFunctionsConsumer.cpp
  ${embedded_files_list}
  ${embedded_libraries_list}

  DEPENDS
  ClangDriverOptions
  intrinsics_gen

  LINK_LIBS
  clangAST
  clangBasic
  clangDriver
  clangEdit
  clangLex
  clangParse
  clangSema
  clangSerialization
  )
