add_llvm_component_library(LLVMTPCInfo
  TPCTargetInfo.cpp

  LINK_COMPONENTS
  Support

  ADD_TO_COMPONENT
  TPC
  )
