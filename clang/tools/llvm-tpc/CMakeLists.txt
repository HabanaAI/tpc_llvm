set(LLVM_EXPORTED_SYMBOL_FILE ${CMAKE_CURRENT_SOURCE_DIR}/llvm-tpc.exports)

add_clang_library(LLVMTPC SHARED
  Compiler.cpp
  Disassembler.cpp
  MemoryManager.cpp
  RedirectOutput.cpp
  ../../../llvm/tools/llvm-objdump/COFFDump.cpp
  ../../../llvm/tools/llvm-objdump/ELFDump.cpp
  ../../../llvm/tools/llvm-objdump/MachODump.cpp
  ../../../llvm/tools/llvm-objdump/WasmDump.cpp
  ../../../llvm/tools/llvm-objdump/TPCDump.cpp

  DEPENDS
  intrinsics_gen

  LINK_LIBS
  clangCodeGen

  LINK_COMPONENTS
  ${LLVM_TARGETS_TO_BUILD}
  Analysis
  CodeGen
  Core
  IPO
  AggressiveInstCombine
  InstCombine
  Instrumentation
  MC
  MCParser
  Option
  ScalarOpts
  Symbolize
  TransformUtils
  Vectorize
  )

target_include_directories(LLVMTPC PRIVATE
  ${LLVM_MAIN_SRC_DIR}
  ${LLVM_BINARY_DIR}
  )

set_property(TARGET LLVMTPC PROPERTY SOVERSION)
set_property(TARGET LLVMTPC PROPERTY VERSION)
