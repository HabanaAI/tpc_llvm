add_subdirectory(Rewrite)

set(LLVM_LINK_COMPONENTS
  BitReader
  BitstreamReader
  Option
  ProfileData
  Support
  )

set(optional_deps intrinsics_gen)
if (CLANG_BUILT_STANDALONE)
  set(optional_deps)
endif()

# Original files are kept here.
set(embeddable_files_dir ${CLANG_SOURCE_DIR}/lib/Headers)

# Set of files to embed.
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
    DEPENDS ${file_to_embed} ${CMAKE_CURRENT_SOURCE_DIR}/embed_file.py
    COMMAND
      ${PYTHON_EXECUTABLE} ${CMAKE_CURRENT_SOURCE_DIR}/embed_file.py "${embedded_file}" "${file_to_embed}"
  )
endforeach()
set_source_files_properties(${embedded_files_list}
  PROPERTIES GENERATED TRUE
)

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
  FrontendTiming.cpp
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

  DEPENDS
  ClangDriverOptions
  ${optional_deps}

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
