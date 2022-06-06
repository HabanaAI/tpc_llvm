add_llvm_component_library(LLVMTPCAsmParser
  TPCAsmParser.cpp
  TPCAsmInstCompress.cpp

  LINK_COMPONENTS
  MC
  MCParser
  Support
  TPCDesc
  TPCInfo

  ADD_TO_COMPONENT
  TPC
  )
