// RUN: %tpc_clang -x assembler -march=gaudi2 %s -c -o %t.o
// RUN: %llvm-objdump --triple tpc -s -j .tpc_compiler %t.o | FileCheck %s

NOP;

// CHECK: Contents of section .tpc_compiler:
// CHECK-NEXT:  0000 222d6363 31617322 {{[a-f0-9]+}} {{[a-f0-9]+}}  "-cc1as"


