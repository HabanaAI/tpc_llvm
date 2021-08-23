list(APPEND TPC_RUNTIME_TEST_DEPS
  FileCheck
  count
  not
  clang
  llvm-config
  llvm-objdump
  )

add_lit_testsuite(check-rc99 "Running the TPC clang tests"
  ${CMAKE_CURRENT_BINARY_DIR}
  #LIT ${LLVM_LIT}
  PARAMS ${CLANG_TEST_PARAMS}
  DEPENDS ${TPC_RUNTIME_TEST_DEPS}
  ARGS ${CLANG_TEST_EXTRA_ARGS}
  )
set_target_properties(check-rc99 PROPERTIES FOLDER "Clang tests")
