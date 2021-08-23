set(LLVM_LINK_COMPONENTS
  TPCDesc
  )

add_llvm_component_library(LLVMTPCAsmParser
  TPCAsmParser.cpp
  TPCAsmInstCompress.cpp
  )

add_dependencies(LLVMTPCAsmParser TPCCommonTableGen)
