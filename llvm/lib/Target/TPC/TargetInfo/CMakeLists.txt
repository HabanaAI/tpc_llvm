add_llvm_component_library(LLVMTPCInfo
  TPCTargetInfo.cpp
  )
add_dependencies(LLVMTPCInfo TPCCommonTableGen)
