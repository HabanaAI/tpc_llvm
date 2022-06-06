add_llvm_component_library(LLVMTPCDisassembler
  TPCDisassembler.cpp

  LINK_COMPONENTS
  MCDisassembler
  Support
  TPCDesc
  TPCInfo

  ADD_TO_COMPONENT
  TPC
  )
