// RUN: %clang -cc1as -triple tpc-none-none -filetype obj -target-cpu goya2 %s -o %t.o
// RUN: %disasm --mcpu goya2 %t.o | FileCheck %s


NOP; NOP; MUL.I32 D6, V2, V1
NOP; NOP; MUL.I32 RND32=NO_ROUND D6, V2, V1
NOP; NOP; MUL.I32 NO_ROUND D6, V2, V1
NOP; NOP; MUL.I32 RND32=NO_ROUND, D6, V2, V1
NOP; NOP; MUL.I32 NO_ROUND, D6, V2, V1

// CHECK: nop;    nop;    mul.i32  D6, V2, V1;       nop
// CHECK: nop;    nop;    mul.i32  D6, V2, V1;       nop
// CHECK: nop;    nop;    mul.i32  D6, V2, V1;       nop
// CHECK: nop;    nop;    mul.i32  D6, V2, V1;       nop
// CHECK: nop;    nop;    mul.i32  D6, V2, V1;       nop


NOP; NOP; MUL.I32 RND32=DOUBLE_AND_ROUND32 V6, V2, V1
NOP; NOP; MUL.I32 DOUBLE_AND_ROUND32 V6, V2, V1

// CHECK: nop;    nop;    mul.i32  double_and_round32 V6, V2, V1;         nop
// CHECK: nop;    nop;    mul.i32  double_and_round32 V6, V2, V1;         nop

NOP; NOP; MUL.I32 RND32=KEEP_RS D6, V2, V1
NOP; NOP; MUL.I32 KEEP_RS D6, V2, V1
NOP; NOP; MUL.I32 RND32=KEEP_RS_FOR_ADD D6, V2, V1
NOP; NOP; MUL.I32 KEEP_RS_FOR_ADD D6, V2, V1


// CHECK: nop;    nop;    mul.i32  keep_rs D6, V2, V1;         nop
// CHECK: nop;    nop;    mul.i32  keep_rs D6, V2, V1;         nop
// CHECK: nop;    nop;    mul.i32  keep_rs_for_add D6, V2, V1;         nop
// CHECK: nop;    nop;    mul.i32  keep_rs_for_add D6, V2, V1;         nop
