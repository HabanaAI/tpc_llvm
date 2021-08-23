add_llvm_component_library(LLVMTPCDesc
  InstructionDB.cpp
  TPCMCTargetDesc.cpp
  TPCMCAsmInfo.cpp
  TPCAsmBackend.cpp
  TPCELFObjectWriter.cpp
  TPCMCCodeEmitter.cpp
  TPCMCInstrInfo.cpp
  TPCInstrDecomposer.cpp
  TPCInstrComposer.cpp
  TPCInstPrinter.cpp
  TPCMetadataSection.cpp
  TPCKernelInfo.cpp
  )
add_dependencies(LLVMTPCDesc TPCCommonTableGen)
