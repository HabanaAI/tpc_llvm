// RUN: %clang -cc1as -triple tpc-none-none -filetype obj %s -o %t.o
// RUN: %disasm %t.o | FileCheck %s
// RUN: %clang -cc1as -triple tpc-none-none -target-cpu gaudi -filetype obj %s -o %t.o
// RUN: %disasm --mcpu=gaudi %t.o | FileCheck %s
// RUN: %clang -cc1as -triple tpc-none-none -target-cpu goya2 -filetype obj %s -o %t.o
// RUN: %disasm --mcpu=goya2 %t.o | FileCheck %s


nop; nop; sel2_leq.i32 D4, V5, V6, V7, V8, SP0;         nop
nop; nop; sel2_leq.i32 D4, V5, V6, V7, V8, SP1;    nop
nop; nop; sel2_leq.i32 D4, V5, V6, V7, V8, !SP1;   nop
nop; nop; sel2_leq.i32 D4, V5, V6, V7, V8, VP1;    nop
nop; nop; sel2_leq.i32 D4, V5, V6, V7, V8, !VP1;   nop

// CHECK: nop; nop; sel2_leq.i32 D4, V5, V6, V7, V8; nop
// CHECK: nop; nop; sel2_leq.i32 D4, V5, V6, V7, V8, SP1; nop
// CHECK: nop; nop; sel2_leq.i32 D4, V5, V6, V7, V8, !SP1; nop
// CHECK: nop; nop; sel2_leq.i32 D4, V5, V6, V7, V8, VP1; nop
// CHECK: nop; nop; sel2_leq.i32 D4, V5, V6, V7, V8, !VP1; nop

nop; nop; sel2_leq.i32 D4, V5, S3, V7, V8, SP0;         nop
nop; nop; sel2_leq.i32 D4, V5, S3, V7, V8, SP1;    nop
nop; nop; sel2_leq.i32 D4, V5, S3, V7, V8, !SP1;   nop
nop; nop; sel2_leq.i32 D4, V5, S3, V7, V8, VP1;    nop
nop; nop; sel2_leq.i32 D4, V5, S3, V7, V8, !VP1;   nop

// CHECK: nop; nop; sel2_leq.i32 D4, V5, S3, V7, V8; nop
// CHECK: nop; nop; sel2_leq.i32 D4, V5, S3, V7, V8, SP1; nop
// CHECK: nop; nop; sel2_leq.i32 D4, V5, S3, V7, V8, !SP1; nop
// CHECK: nop; nop; sel2_leq.i32 D4, V5, S3, V7, V8, VP1; nop
// CHECK: nop; nop; sel2_leq.i32 D4, V5, S3, V7, V8, !VP1; nop

nop; nop; sel2_leq.i32 D4, V5, 122, V7, V8, SP0;         nop
nop; nop; sel2_leq.i32 D4, V5, 122, V7, V8, SP1;    nop
nop; nop; sel2_leq.i32 D4, V5, 122, V7, V8, !SP1;   nop
nop; nop; sel2_leq.i32 D4, V5, 122, V7, V8, VP1;    nop
nop; nop; sel2_leq.i32 D4, V5, 122, V7, V8, !VP1;   nop

// CHECK: nop; nop; sel2_leq.i32 D4, V5, 0x7a, V7, V8; nop
// CHECK: nop; nop; sel2_leq.i32 D4, V5, 0x7a, V7, V8, SP1; nop
// CHECK: nop; nop; sel2_leq.i32 D4, V5, 0x7a, V7, V8, !SP1; nop
// CHECK: nop; nop; sel2_leq.i32 D4, V5, 0x7a, V7, V8, VP1; nop
// CHECK: nop; nop; sel2_leq.i32 D4, V5, 0x7a, V7, V8, !VP1; nop

nop; nop; sel2_leq.i32 D4, V5, V6, V7, S3, SP0;         nop
nop; nop; sel2_leq.i32 D4, V5, V6, V7, S3, SP1;    nop
nop; nop; sel2_leq.i32 D4, V5, V6, V7, S3, !SP1;   nop
nop; nop; sel2_leq.i32 D4, V5, V6, V7, S3, VP1;    nop
nop; nop; sel2_leq.i32 D4, V5, V6, V7, S3, !VP1;   nop

// CHECK: nop; nop; sel2_leq.i32 D4, V5, V6, V7, S3; nop
// CHECK: nop; nop; sel2_leq.i32 D4, V5, V6, V7, S3, SP1; nop
// CHECK: nop; nop; sel2_leq.i32 D4, V5, V6, V7, S3, !SP1; nop
// CHECK: nop; nop; sel2_leq.i32 D4, V5, V6, V7, S3, VP1; nop
// CHECK: nop; nop; sel2_leq.i32 D4, V5, V6, V7, S3, !VP1; nop

nop; nop; sel2_leq.i32 D4, V5, V6, V7, 123;         nop
nop; nop; sel2_leq.i32 D4, V5, V6, V7, 123, SP1;    nop
nop; nop; sel2_leq.i32 D4, V5, V6, V7, 123, !SP1;   nop
nop; nop; sel2_leq.i32 D4, V5, V6, V7, 123, VP1;    nop
nop; nop; sel2_leq.i32 D4, V5, V6, V7, 123, !VP1;   nop

// CHECK: nop; nop; sel2_leq.i32 D4, V5, V6, V7, 0x7b; nop
// CHECK: nop; nop; sel2_leq.i32 D4, V5, V6, V7, 0x7b, SP1; nop
// CHECK: nop; nop; sel2_leq.i32 D4, V5, V6, V7, 0x7b, !SP1; nop
// CHECK: nop; nop; sel2_leq.i32 D4, V5, V6, V7, 0x7b, VP1; nop
// CHECK: nop; nop; sel2_leq.i32 D4, V5, V6, V7, 0x7b, !VP1; nop



nop; nop; sel2_leq.i16 D4, V5, V6, V7, V8, SP0;         nop
nop; nop; sel2_leq.i16 D4, V5, V6, V7, V8, SP1;    nop
nop; nop; sel2_leq.i16 D4, V5, V6, V7, V8, !SP1;   nop
nop; nop; sel2_leq.i16 D4, V5, V6, V7, V8, VP1;    nop
nop; nop; sel2_leq.i16 D4, V5, V6, V7, V8, !VP1;   nop

// CHECK: nop; nop; sel2_leq.i16 D4, V5, V6, V7, V8; nop
// CHECK: nop; nop; sel2_leq.i16 D4, V5, V6, V7, V8, SP1; nop
// CHECK: nop; nop; sel2_leq.i16 D4, V5, V6, V7, V8, !SP1; nop
// CHECK: nop; nop; sel2_leq.i16 D4, V5, V6, V7, V8, VP1; nop
// CHECK: nop; nop; sel2_leq.i16 D4, V5, V6, V7, V8, !VP1; nop

nop; nop; sel2_leq.i16 D4, V5, S3, V7, V8, SP0;         nop
nop; nop; sel2_leq.i16 D4, V5, S3, V7, V8, SP1;    nop
nop; nop; sel2_leq.i16 D4, V5, S3, V7, V8, !SP1;   nop
nop; nop; sel2_leq.i16 D4, V5, S3, V7, V8, VP1;    nop
nop; nop; sel2_leq.i16 D4, V5, S3, V7, V8, !VP1;   nop

// CHECK: nop; nop; sel2_leq.i16 D4, V5, S3, V7, V8; nop
// CHECK: nop; nop; sel2_leq.i16 D4, V5, S3, V7, V8, SP1; nop
// CHECK: nop; nop; sel2_leq.i16 D4, V5, S3, V7, V8, !SP1; nop
// CHECK: nop; nop; sel2_leq.i16 D4, V5, S3, V7, V8, VP1; nop
// CHECK: nop; nop; sel2_leq.i16 D4, V5, S3, V7, V8, !VP1; nop

nop; nop; sel2_leq.i16 D4, V5, 122, V7, V8, SP0;         nop
nop; nop; sel2_leq.i16 D4, V5, 122, V7, V8, SP1;    nop
nop; nop; sel2_leq.i16 D4, V5, 122, V7, V8, !SP1;   nop
nop; nop; sel2_leq.i16 D4, V5, 122, V7, V8, VP1;    nop
nop; nop; sel2_leq.i16 D4, V5, 122, V7, V8, !VP1;   nop

// CHECK: nop; nop; sel2_leq.i16 D4, V5, 0x7a, V7, V8; nop
// CHECK: nop; nop; sel2_leq.i16 D4, V5, 0x7a, V7, V8, SP1; nop
// CHECK: nop; nop; sel2_leq.i16 D4, V5, 0x7a, V7, V8, !SP1; nop
// CHECK: nop; nop; sel2_leq.i16 D4, V5, 0x7a, V7, V8, VP1; nop
// CHECK: nop; nop; sel2_leq.i16 D4, V5, 0x7a, V7, V8, !VP1; nop

nop; nop; sel2_leq.i16 D4, V5, V6, V7, S3, SP0;         nop
nop; nop; sel2_leq.i16 D4, V5, V6, V7, S3, SP1;    nop
nop; nop; sel2_leq.i16 D4, V5, V6, V7, S3, !SP1;   nop
nop; nop; sel2_leq.i16 D4, V5, V6, V7, S3, VP1;    nop
nop; nop; sel2_leq.i16 D4, V5, V6, V7, S3, !VP1;   nop

// CHECK: nop; nop; sel2_leq.i16 D4, V5, V6, V7, S3; nop
// CHECK: nop; nop; sel2_leq.i16 D4, V5, V6, V7, S3, SP1; nop
// CHECK: nop; nop; sel2_leq.i16 D4, V5, V6, V7, S3, !SP1; nop
// CHECK: nop; nop; sel2_leq.i16 D4, V5, V6, V7, S3, VP1; nop
// CHECK: nop; nop; sel2_leq.i16 D4, V5, V6, V7, S3, !VP1; nop

nop; nop; sel2_leq.i16 D4, V5, V6, V7, 123;         nop
nop; nop; sel2_leq.i16 D4, V5, V6, V7, 123, SP1;    nop
nop; nop; sel2_leq.i16 D4, V5, V6, V7, 123, !SP1;   nop
nop; nop; sel2_leq.i16 D4, V5, V6, V7, 123, VP1;    nop
nop; nop; sel2_leq.i16 D4, V5, V6, V7, 123, !VP1;   nop

// CHECK: nop; nop; sel2_leq.i16 D4, V5, V6, V7, 0x7b; nop
// CHECK: nop; nop; sel2_leq.i16 D4, V5, V6, V7, 0x7b, SP1; nop
// CHECK: nop; nop; sel2_leq.i16 D4, V5, V6, V7, 0x7b, !SP1; nop
// CHECK: nop; nop; sel2_leq.i16 D4, V5, V6, V7, 0x7b, VP1; nop
// CHECK: nop; nop; sel2_leq.i16 D4, V5, V6, V7, 0x7b, !VP1; nop



nop; nop; sel2_leq.i8 D4, V5, V6, V7, V8, SP0;         nop
nop; nop; sel2_leq.i8 D4, V5, V6, V7, V8, SP1;    nop
nop; nop; sel2_leq.i8 D4, V5, V6, V7, V8, !SP1;   nop
nop; nop; sel2_leq.i8 D4, V5, V6, V7, V8, VP1;    nop
nop; nop; sel2_leq.i8 D4, V5, V6, V7, V8, !VP1;   nop

// CHECK: nop; nop; sel2_leq.i8 D4, V5, V6, V7, V8; nop
// CHECK: nop; nop; sel2_leq.i8 D4, V5, V6, V7, V8, SP1; nop
// CHECK: nop; nop; sel2_leq.i8 D4, V5, V6, V7, V8, !SP1; nop
// CHECK: nop; nop; sel2_leq.i8 D4, V5, V6, V7, V8, VP1; nop
// CHECK: nop; nop; sel2_leq.i8 D4, V5, V6, V7, V8, !VP1; nop

nop; nop; sel2_leq.i8 D4, V5, S3, V7, V8, SP0;         nop
nop; nop; sel2_leq.i8 D4, V5, S3, V7, V8, SP1;    nop
nop; nop; sel2_leq.i8 D4, V5, S3, V7, V8, !SP1;   nop
nop; nop; sel2_leq.i8 D4, V5, S3, V7, V8, VP1;    nop
nop; nop; sel2_leq.i8 D4, V5, S3, V7, V8, !VP1;   nop

// CHECK: nop; nop; sel2_leq.i8 D4, V5, S3, V7, V8; nop
// CHECK: nop; nop; sel2_leq.i8 D4, V5, S3, V7, V8, SP1; nop
// CHECK: nop; nop; sel2_leq.i8 D4, V5, S3, V7, V8, !SP1; nop
// CHECK: nop; nop; sel2_leq.i8 D4, V5, S3, V7, V8, VP1; nop
// CHECK: nop; nop; sel2_leq.i8 D4, V5, S3, V7, V8, !VP1; nop

nop; nop; sel2_leq.i8 D4, V5, 122, V7, V8, SP0;         nop
nop; nop; sel2_leq.i8 D4, V5, 122, V7, V8, SP1;    nop
nop; nop; sel2_leq.i8 D4, V5, 122, V7, V8, !SP1;   nop
nop; nop; sel2_leq.i8 D4, V5, 122, V7, V8, VP1;    nop
nop; nop; sel2_leq.i8 D4, V5, 122, V7, V8, !VP1;   nop

// CHECK: nop; nop; sel2_leq.i8 D4, V5, 0x7a, V7, V8; nop
// CHECK: nop; nop; sel2_leq.i8 D4, V5, 0x7a, V7, V8, SP1; nop
// CHECK: nop; nop; sel2_leq.i8 D4, V5, 0x7a, V7, V8, !SP1; nop
// CHECK: nop; nop; sel2_leq.i8 D4, V5, 0x7a, V7, V8, VP1; nop
// CHECK: nop; nop; sel2_leq.i8 D4, V5, 0x7a, V7, V8, !VP1; nop

nop; nop; sel2_leq.i8 D4, V5, V6, V7, S3, SP0;         nop
nop; nop; sel2_leq.i8 D4, V5, V6, V7, S3, SP1;    nop
nop; nop; sel2_leq.i8 D4, V5, V6, V7, S3, !SP1;   nop
nop; nop; sel2_leq.i8 D4, V5, V6, V7, S3, VP1;    nop
nop; nop; sel2_leq.i8 D4, V5, V6, V7, S3, !VP1;   nop

// CHECK: nop; nop; sel2_leq.i8 D4, V5, V6, V7, S3; nop
// CHECK: nop; nop; sel2_leq.i8 D4, V5, V6, V7, S3, SP1; nop
// CHECK: nop; nop; sel2_leq.i8 D4, V5, V6, V7, S3, !SP1; nop
// CHECK: nop; nop; sel2_leq.i8 D4, V5, V6, V7, S3, VP1; nop
// CHECK: nop; nop; sel2_leq.i8 D4, V5, V6, V7, S3, !VP1; nop

nop; nop; sel2_leq.i8 D4, V5, V6, V7, 123;         nop
nop; nop; sel2_leq.i8 D4, V5, V6, V7, 123, SP1;    nop
nop; nop; sel2_leq.i8 D4, V5, V6, V7, 123, !SP1;   nop
nop; nop; sel2_leq.i8 D4, V5, V6, V7, 123, VP1;    nop
nop; nop; sel2_leq.i8 D4, V5, V6, V7, 123, !VP1;   nop

// CHECK: nop; nop; sel2_leq.i8 D4, V5, V6, V7, 0x7b; nop
// CHECK: nop; nop; sel2_leq.i8 D4, V5, V6, V7, 0x7b, SP1; nop
// CHECK: nop; nop; sel2_leq.i8 D4, V5, V6, V7, 0x7b, !SP1; nop
// CHECK: nop; nop; sel2_leq.i8 D4, V5, V6, V7, 0x7b, VP1; nop
// CHECK: nop; nop; sel2_leq.i8 D4, V5, V6, V7, 0x7b, !VP1; nop



nop; nop; sel2_leq.u32 D4, V5, V6, V7, V8, SP0;         nop
nop; nop; sel2_leq.u32 D4, V5, V6, V7, V8, SP1;    nop
nop; nop; sel2_leq.u32 D4, V5, V6, V7, V8, !SP1;   nop
nop; nop; sel2_leq.u32 D4, V5, V6, V7, V8, VP1;    nop
nop; nop; sel2_leq.u32 D4, V5, V6, V7, V8, !VP1;   nop

// CHECK: nop; nop; sel2_leq.u32 D4, V5, V6, V7, V8; nop
// CHECK: nop; nop; sel2_leq.u32 D4, V5, V6, V7, V8, SP1; nop
// CHECK: nop; nop; sel2_leq.u32 D4, V5, V6, V7, V8, !SP1; nop
// CHECK: nop; nop; sel2_leq.u32 D4, V5, V6, V7, V8, VP1; nop
// CHECK: nop; nop; sel2_leq.u32 D4, V5, V6, V7, V8, !VP1; nop

nop; nop; sel2_leq.u32 D4, V5, S3, V7, V8, SP0;         nop
nop; nop; sel2_leq.u32 D4, V5, S3, V7, V8, SP1;    nop
nop; nop; sel2_leq.u32 D4, V5, S3, V7, V8, !SP1;   nop
nop; nop; sel2_leq.u32 D4, V5, S3, V7, V8, VP1;    nop
nop; nop; sel2_leq.u32 D4, V5, S3, V7, V8, !VP1;   nop

// CHECK: nop; nop; sel2_leq.u32 D4, V5, S3, V7, V8; nop
// CHECK: nop; nop; sel2_leq.u32 D4, V5, S3, V7, V8, SP1; nop
// CHECK: nop; nop; sel2_leq.u32 D4, V5, S3, V7, V8, !SP1; nop
// CHECK: nop; nop; sel2_leq.u32 D4, V5, S3, V7, V8, VP1; nop
// CHECK: nop; nop; sel2_leq.u32 D4, V5, S3, V7, V8, !VP1; nop

nop; nop; sel2_leq.u32 D4, V5, 122, V7, V8, SP0;         nop
nop; nop; sel2_leq.u32 D4, V5, 122, V7, V8, SP1;    nop
nop; nop; sel2_leq.u32 D4, V5, 122, V7, V8, !SP1;   nop
nop; nop; sel2_leq.u32 D4, V5, 122, V7, V8, VP1;    nop
nop; nop; sel2_leq.u32 D4, V5, 122, V7, V8, !VP1;   nop

// CHECK: nop; nop; sel2_leq.u32 D4, V5, 0x7a, V7, V8; nop
// CHECK: nop; nop; sel2_leq.u32 D4, V5, 0x7a, V7, V8, SP1; nop
// CHECK: nop; nop; sel2_leq.u32 D4, V5, 0x7a, V7, V8, !SP1; nop
// CHECK: nop; nop; sel2_leq.u32 D4, V5, 0x7a, V7, V8, VP1; nop
// CHECK: nop; nop; sel2_leq.u32 D4, V5, 0x7a, V7, V8, !VP1; nop

nop; nop; sel2_leq.u32 D4, V5, V6, V7, S3, SP0;         nop
nop; nop; sel2_leq.u32 D4, V5, V6, V7, S3, SP1;    nop
nop; nop; sel2_leq.u32 D4, V5, V6, V7, S3, !SP1;   nop
nop; nop; sel2_leq.u32 D4, V5, V6, V7, S3, VP1;    nop
nop; nop; sel2_leq.u32 D4, V5, V6, V7, S3, !VP1;   nop

// CHECK: nop; nop; sel2_leq.u32 D4, V5, V6, V7, S3; nop
// CHECK: nop; nop; sel2_leq.u32 D4, V5, V6, V7, S3, SP1; nop
// CHECK: nop; nop; sel2_leq.u32 D4, V5, V6, V7, S3, !SP1; nop
// CHECK: nop; nop; sel2_leq.u32 D4, V5, V6, V7, S3, VP1; nop
// CHECK: nop; nop; sel2_leq.u32 D4, V5, V6, V7, S3, !VP1; nop

nop; nop; sel2_leq.u32 D4, V5, V6, V7, 123;         nop
nop; nop; sel2_leq.u32 D4, V5, V6, V7, 123, SP1;    nop
nop; nop; sel2_leq.u32 D4, V5, V6, V7, 123, !SP1;   nop
nop; nop; sel2_leq.u32 D4, V5, V6, V7, 123, VP1;    nop
nop; nop; sel2_leq.u32 D4, V5, V6, V7, 123, !VP1;   nop

// CHECK: nop; nop; sel2_leq.u32 D4, V5, V6, V7, 0x7b; nop
// CHECK: nop; nop; sel2_leq.u32 D4, V5, V6, V7, 0x7b, SP1; nop
// CHECK: nop; nop; sel2_leq.u32 D4, V5, V6, V7, 0x7b, !SP1; nop
// CHECK: nop; nop; sel2_leq.u32 D4, V5, V6, V7, 0x7b, VP1; nop
// CHECK: nop; nop; sel2_leq.u32 D4, V5, V6, V7, 0x7b, !VP1; nop



nop; nop; sel2_leq.u16 D4, V5, V6, V7, V8, SP0;         nop
nop; nop; sel2_leq.u16 D4, V5, V6, V7, V8, SP1;    nop
nop; nop; sel2_leq.u16 D4, V5, V6, V7, V8, !SP1;   nop
nop; nop; sel2_leq.u16 D4, V5, V6, V7, V8, VP1;    nop
nop; nop; sel2_leq.u16 D4, V5, V6, V7, V8, !VP1;   nop

// CHECK: nop; nop; sel2_leq.u16 D4, V5, V6, V7, V8; nop
// CHECK: nop; nop; sel2_leq.u16 D4, V5, V6, V7, V8, SP1; nop
// CHECK: nop; nop; sel2_leq.u16 D4, V5, V6, V7, V8, !SP1; nop
// CHECK: nop; nop; sel2_leq.u16 D4, V5, V6, V7, V8, VP1; nop
// CHECK: nop; nop; sel2_leq.u16 D4, V5, V6, V7, V8, !VP1; nop

nop; nop; sel2_leq.u16 D4, V5, S3, V7, V8, SP0;         nop
nop; nop; sel2_leq.u16 D4, V5, S3, V7, V8, SP1;    nop
nop; nop; sel2_leq.u16 D4, V5, S3, V7, V8, !SP1;   nop
nop; nop; sel2_leq.u16 D4, V5, S3, V7, V8, VP1;    nop
nop; nop; sel2_leq.u16 D4, V5, S3, V7, V8, !VP1;   nop

// CHECK: nop; nop; sel2_leq.u16 D4, V5, S3, V7, V8; nop
// CHECK: nop; nop; sel2_leq.u16 D4, V5, S3, V7, V8, SP1; nop
// CHECK: nop; nop; sel2_leq.u16 D4, V5, S3, V7, V8, !SP1; nop
// CHECK: nop; nop; sel2_leq.u16 D4, V5, S3, V7, V8, VP1; nop
// CHECK: nop; nop; sel2_leq.u16 D4, V5, S3, V7, V8, !VP1; nop

nop; nop; sel2_leq.u16 D4, V5, 122, V7, V8, SP0;         nop
nop; nop; sel2_leq.u16 D4, V5, 122, V7, V8, SP1;    nop
nop; nop; sel2_leq.u16 D4, V5, 122, V7, V8, !SP1;   nop
nop; nop; sel2_leq.u16 D4, V5, 122, V7, V8, VP1;    nop
nop; nop; sel2_leq.u16 D4, V5, 122, V7, V8, !VP1;   nop

// CHECK: nop; nop; sel2_leq.u16 D4, V5, 0x7a, V7, V8; nop
// CHECK: nop; nop; sel2_leq.u16 D4, V5, 0x7a, V7, V8, SP1; nop
// CHECK: nop; nop; sel2_leq.u16 D4, V5, 0x7a, V7, V8, !SP1; nop
// CHECK: nop; nop; sel2_leq.u16 D4, V5, 0x7a, V7, V8, VP1; nop
// CHECK: nop; nop; sel2_leq.u16 D4, V5, 0x7a, V7, V8, !VP1; nop

nop; nop; sel2_leq.u16 D4, V5, V6, V7, S3, SP0;         nop
nop; nop; sel2_leq.u16 D4, V5, V6, V7, S3, SP1;    nop
nop; nop; sel2_leq.u16 D4, V5, V6, V7, S3, !SP1;   nop
nop; nop; sel2_leq.u16 D4, V5, V6, V7, S3, VP1;    nop
nop; nop; sel2_leq.u16 D4, V5, V6, V7, S3, !VP1;   nop

// CHECK: nop; nop; sel2_leq.u16 D4, V5, V6, V7, S3; nop
// CHECK: nop; nop; sel2_leq.u16 D4, V5, V6, V7, S3, SP1; nop
// CHECK: nop; nop; sel2_leq.u16 D4, V5, V6, V7, S3, !SP1; nop
// CHECK: nop; nop; sel2_leq.u16 D4, V5, V6, V7, S3, VP1; nop
// CHECK: nop; nop; sel2_leq.u16 D4, V5, V6, V7, S3, !VP1; nop

nop; nop; sel2_leq.u16 D4, V5, V6, V7, 123;         nop
nop; nop; sel2_leq.u16 D4, V5, V6, V7, 123, SP1;    nop
nop; nop; sel2_leq.u16 D4, V5, V6, V7, 123, !SP1;   nop
nop; nop; sel2_leq.u16 D4, V5, V6, V7, 123, VP1;    nop
nop; nop; sel2_leq.u16 D4, V5, V6, V7, 123, !VP1;   nop

// CHECK: nop; nop; sel2_leq.u16 D4, V5, V6, V7, 0x7b; nop
// CHECK: nop; nop; sel2_leq.u16 D4, V5, V6, V7, 0x7b, SP1; nop
// CHECK: nop; nop; sel2_leq.u16 D4, V5, V6, V7, 0x7b, !SP1; nop
// CHECK: nop; nop; sel2_leq.u16 D4, V5, V6, V7, 0x7b, VP1; nop
// CHECK: nop; nop; sel2_leq.u16 D4, V5, V6, V7, 0x7b, !VP1; nop



nop; nop; sel2_leq.u8 D4, V5, V6, V7, V8, SP0;         nop
nop; nop; sel2_leq.u8 D4, V5, V6, V7, V8, SP1;    nop
nop; nop; sel2_leq.u8 D4, V5, V6, V7, V8, !SP1;   nop
nop; nop; sel2_leq.u8 D4, V5, V6, V7, V8, VP1;    nop
nop; nop; sel2_leq.u8 D4, V5, V6, V7, V8, !VP1;   nop

// CHECK: nop; nop; sel2_leq.u8 D4, V5, V6, V7, V8; nop
// CHECK: nop; nop; sel2_leq.u8 D4, V5, V6, V7, V8, SP1; nop
// CHECK: nop; nop; sel2_leq.u8 D4, V5, V6, V7, V8, !SP1; nop
// CHECK: nop; nop; sel2_leq.u8 D4, V5, V6, V7, V8, VP1; nop
// CHECK: nop; nop; sel2_leq.u8 D4, V5, V6, V7, V8, !VP1; nop

nop; nop; sel2_leq.u8 D4, V5, S3, V7, V8, SP0;         nop
nop; nop; sel2_leq.u8 D4, V5, S3, V7, V8, SP1;    nop
nop; nop; sel2_leq.u8 D4, V5, S3, V7, V8, !SP1;   nop
nop; nop; sel2_leq.u8 D4, V5, S3, V7, V8, VP1;    nop
nop; nop; sel2_leq.u8 D4, V5, S3, V7, V8, !VP1;   nop

// CHECK: nop; nop; sel2_leq.u8 D4, V5, S3, V7, V8; nop
// CHECK: nop; nop; sel2_leq.u8 D4, V5, S3, V7, V8, SP1; nop
// CHECK: nop; nop; sel2_leq.u8 D4, V5, S3, V7, V8, !SP1; nop
// CHECK: nop; nop; sel2_leq.u8 D4, V5, S3, V7, V8, VP1; nop
// CHECK: nop; nop; sel2_leq.u8 D4, V5, S3, V7, V8, !VP1; nop

nop; nop; sel2_leq.u8 D4, V5, 122, V7, V8, SP0;         nop
nop; nop; sel2_leq.u8 D4, V5, 122, V7, V8, SP1;    nop
nop; nop; sel2_leq.u8 D4, V5, 122, V7, V8, !SP1;   nop
nop; nop; sel2_leq.u8 D4, V5, 122, V7, V8, VP1;    nop
nop; nop; sel2_leq.u8 D4, V5, 122, V7, V8, !VP1;   nop

// CHECK: nop; nop; sel2_leq.u8 D4, V5, 0x7a, V7, V8; nop
// CHECK: nop; nop; sel2_leq.u8 D4, V5, 0x7a, V7, V8, SP1; nop
// CHECK: nop; nop; sel2_leq.u8 D4, V5, 0x7a, V7, V8, !SP1; nop
// CHECK: nop; nop; sel2_leq.u8 D4, V5, 0x7a, V7, V8, VP1; nop
// CHECK: nop; nop; sel2_leq.u8 D4, V5, 0x7a, V7, V8, !VP1; nop

nop; nop; sel2_leq.u8 D4, V5, V6, V7, S3, SP0;         nop
nop; nop; sel2_leq.u8 D4, V5, V6, V7, S3, SP1;    nop
nop; nop; sel2_leq.u8 D4, V5, V6, V7, S3, !SP1;   nop
nop; nop; sel2_leq.u8 D4, V5, V6, V7, S3, VP1;    nop
nop; nop; sel2_leq.u8 D4, V5, V6, V7, S3, !VP1;   nop

// CHECK: nop; nop; sel2_leq.u8 D4, V5, V6, V7, S3; nop
// CHECK: nop; nop; sel2_leq.u8 D4, V5, V6, V7, S3, SP1; nop
// CHECK: nop; nop; sel2_leq.u8 D4, V5, V6, V7, S3, !SP1; nop
// CHECK: nop; nop; sel2_leq.u8 D4, V5, V6, V7, S3, VP1; nop
// CHECK: nop; nop; sel2_leq.u8 D4, V5, V6, V7, S3, !VP1; nop

nop; nop; sel2_leq.u8 D4, V5, V6, V7, 123;         nop
nop; nop; sel2_leq.u8 D4, V5, V6, V7, 123, SP1;    nop
nop; nop; sel2_leq.u8 D4, V5, V6, V7, 123, !SP1;   nop
nop; nop; sel2_leq.u8 D4, V5, V6, V7, 123, VP1;    nop
nop; nop; sel2_leq.u8 D4, V5, V6, V7, 123, !VP1;   nop

// CHECK: nop; nop; sel2_leq.u8 D4, V5, V6, V7, 0x7b; nop
// CHECK: nop; nop; sel2_leq.u8 D4, V5, V6, V7, 0x7b, SP1; nop
// CHECK: nop; nop; sel2_leq.u8 D4, V5, V6, V7, 0x7b, !SP1; nop
// CHECK: nop; nop; sel2_leq.u8 D4, V5, V6, V7, 0x7b, VP1; nop
// CHECK: nop; nop; sel2_leq.u8 D4, V5, V6, V7, 0x7b, !VP1; nop



nop; nop; sel2_leq.f32 D4, V5, V6, V7, V8, SP0;         nop
nop; nop; sel2_leq.f32 D4, V5, V6, V7, V8, SP1;    nop
nop; nop; sel2_leq.f32 D4, V5, V6, V7, V8, !SP1;   nop
nop; nop; sel2_leq.f32 D4, V5, V6, V7, V8, VP1;    nop
nop; nop; sel2_leq.f32 D4, V5, V6, V7, V8, !VP1;   nop

// CHECK: nop; nop; sel2_leq.f32 D4, V5, V6, V7, V8; nop
// CHECK: nop; nop; sel2_leq.f32 D4, V5, V6, V7, V8, SP1; nop
// CHECK: nop; nop; sel2_leq.f32 D4, V5, V6, V7, V8, !SP1; nop
// CHECK: nop; nop; sel2_leq.f32 D4, V5, V6, V7, V8, VP1; nop
// CHECK: nop; nop; sel2_leq.f32 D4, V5, V6, V7, V8, !VP1; nop

nop; nop; sel2_leq.f32 D4, V5, S3, V7, V8, SP0;         nop
nop; nop; sel2_leq.f32 D4, V5, S3, V7, V8, SP1;    nop
nop; nop; sel2_leq.f32 D4, V5, S3, V7, V8, !SP1;   nop
nop; nop; sel2_leq.f32 D4, V5, S3, V7, V8, VP1;    nop
nop; nop; sel2_leq.f32 D4, V5, S3, V7, V8, !VP1;   nop

// CHECK: nop; nop; sel2_leq.f32 D4, V5, S3, V7, V8; nop
// CHECK: nop; nop; sel2_leq.f32 D4, V5, S3, V7, V8, SP1; nop
// CHECK: nop; nop; sel2_leq.f32 D4, V5, S3, V7, V8, !SP1; nop
// CHECK: nop; nop; sel2_leq.f32 D4, V5, S3, V7, V8, VP1; nop
// CHECK: nop; nop; sel2_leq.f32 D4, V5, S3, V7, V8, !VP1; nop

nop; nop; sel2_leq.f32 D4, V5, 0x3f800000, V7, V8, SP0;         nop
nop; nop; sel2_leq.f32 D4, V5, 0x3f800000, V7, V8, SP1;    nop
nop; nop; sel2_leq.f32 D4, V5, 0x3f800000, V7, V8, !SP1;   nop
nop; nop; sel2_leq.f32 D4, V5, 0x3f800000, V7, V8, VP1;    nop
nop; nop; sel2_leq.f32 D4, V5, 0x3f800000, V7, V8, !VP1;   nop

// CHECK: nop; nop; sel2_leq.f32 D4, V5, 0x3f800000, V7, V8; nop
// CHECK: nop; nop; sel2_leq.f32 D4, V5, 0x3f800000, V7, V8, SP1; nop
// CHECK: nop; nop; sel2_leq.f32 D4, V5, 0x3f800000, V7, V8, !SP1; nop
// CHECK: nop; nop; sel2_leq.f32 D4, V5, 0x3f800000, V7, V8, VP1; nop
// CHECK: nop; nop; sel2_leq.f32 D4, V5, 0x3f800000, V7, V8, !VP1; nop

nop; nop; sel2_leq.f32 D4, V5, V6, V7, S3, SP0;         nop
nop; nop; sel2_leq.f32 D4, V5, V6, V7, S3, SP1;    nop
nop; nop; sel2_leq.f32 D4, V5, V6, V7, S3, !SP1;   nop
nop; nop; sel2_leq.f32 D4, V5, V6, V7, S3, VP1;    nop
nop; nop; sel2_leq.f32 D4, V5, V6, V7, S3, !VP1;   nop

// CHECK: nop; nop; sel2_leq.f32 D4, V5, V6, V7, S3; nop
// CHECK: nop; nop; sel2_leq.f32 D4, V5, V6, V7, S3, SP1; nop
// CHECK: nop; nop; sel2_leq.f32 D4, V5, V6, V7, S3, !SP1; nop
// CHECK: nop; nop; sel2_leq.f32 D4, V5, V6, V7, S3, VP1; nop
// CHECK: nop; nop; sel2_leq.f32 D4, V5, V6, V7, S3, !VP1; nop

nop; nop; sel2_leq.f32 D4, V5, V6, V7, 0x3f800000;         nop
nop; nop; sel2_leq.f32 D4, V5, V6, V7, 0x3f800000, SP1;    nop
nop; nop; sel2_leq.f32 D4, V5, V6, V7, 0x3f800000, !SP1;   nop
nop; nop; sel2_leq.f32 D4, V5, V6, V7, 0x3f800000, VP1;    nop
nop; nop; sel2_leq.f32 D4, V5, V6, V7, 0x3f800000, !VP1;   nop

// CHECK: nop; nop; sel2_leq.f32 D4, V5, V6, V7, 0x3f800000; nop
// CHECK: nop; nop; sel2_leq.f32 D4, V5, V6, V7, 0x3f800000, SP1; nop
// CHECK: nop; nop; sel2_leq.f32 D4, V5, V6, V7, 0x3f800000, !SP1; nop
// CHECK: nop; nop; sel2_leq.f32 D4, V5, V6, V7, 0x3f800000, VP1; nop
// CHECK: nop; nop; sel2_leq.f32 D4, V5, V6, V7, 0x3f800000, !VP1; nop