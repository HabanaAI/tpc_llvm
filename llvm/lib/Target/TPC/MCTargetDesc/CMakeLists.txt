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
  TPCInstrLayout.cpp
  TPCInstPrinter.cpp
  TPCMetadataSection.cpp
  TPCKernelInfo.cpp
  TPCClangInfo.cpp

  LINK_COMPONENTS
  MC
  MCDisassembler
  Support
  TPCInfo
  BinaryFormat

  ADD_TO_COMPONENT
  TPC
  )
