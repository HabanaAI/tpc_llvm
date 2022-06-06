set(LLVM_LINK_COMPONENTS
  Support
  )

if("TPC" IN_LIST LLVM_TARGETS_TO_BUILD)
    list(APPEND LLVM_LINK_COMPONENTS TPCDesc)
endif()

add_clang_unittest(LexTests
  DependencyDirectivesSourceMinimizerTest.cpp
  HeaderMapTest.cpp
  HeaderSearchTest.cpp
  LexerTest.cpp
  PPCallbacksTest.cpp
  PPConditionalDirectiveRecordTest.cpp
  )

set(CLANG_LINK_LIBS
  clangAST
  clangBasic
  clangLex
  clangParse
  clangSema
  )

if("TPC" IN_LIST LLVM_TARGETS_TO_BUILD)
    list(APPEND CLANG_LINK_LIBS LLVMTPCDesc)
endif()

clang_target_link_libraries(LexTests
  PRIVATE
  ${CLANG_LINK_LIBS}
  )
