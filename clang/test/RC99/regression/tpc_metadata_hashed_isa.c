// A kernel keeps its hash across architectures, optimization levels and -S/-c
// (only -c embeds the source text); another kernel gets another hash.
// RUN: %tpc_clang -O1 -c -x c -march=gaudi %s -o %t1.o
// RUN: %tpc_clang -O2 -c -x c -march=gaudi2 %s -o %t2.o
// RUN: %tpc_clang -O2 -S -x c -march=gaudi2 -no-instr-compress -mllvm -tpc-asm-format=1 %s -o %t4.s
// RUN: %clang -cc1as -triple tpc-none-none -target-cpu gaudi2 -filetype obj %t4.s -o %t4.o
// RUN: %tpc_clang -O1 -c -x c -march=gaudi2 %S/tpc_metadata_dnorm.c -o %t3.o
// RUN: %metadata_disasm %t1.o %t2.o %t4.o %t3.o | FileCheck %s

void main(tensor out) {
  int5 c = {0};
  int64 v = {0};
  v_i32_st_tnsr(c, out, v);
}

// CHECK:     hashedISA: DD [[HASH:[1-9][0-9]*]]
// CHECK:     hashedISA: DD [[HASH]]
// CHECK:     hashedISA: DD [[HASH]]
// CHECK-NOT: hashedISA: DD [[HASH]]
// CHECK:     hashedISA: DD {{[1-9][0-9]*}}
