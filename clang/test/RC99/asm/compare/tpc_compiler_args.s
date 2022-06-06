// RUN: %clang -cc1as -triple tpc -filetype obj -main-file-name tpc_compiler_args.s -target-cpu goya -fdebug-compilation-dir %T -dwarf-version=4 -mrelocation-model static -no-instr-compress --mrelax-relocations -o %t.o %s
// RUN: %llvm-objdump --triple tpc -s -j .tpc_compiler %t.o | FileCheck %s

NOP;

// CHECK: Contents of section .tpc_compiler:
// CHECK-NEXT:  0000 222d6363 31617322 {{[a-f0-9]+}} {{[a-f0-9]+}}  "-cc1as"


