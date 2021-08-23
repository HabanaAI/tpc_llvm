// RUN: %clang -cc1as -triple tpc-none-none -filetype obj %s -o %t.o
// RUN: %disasm %t.o | FileCheck %s
// RUN: %clang -cc1as -triple tpc-none-none -target-cpu gaudi -filetype obj %s -o %t.o
// RUN: %disasm -mcpu=gaudi %t.o | FileCheck %s



nop; nop; sel_less.i32 V4, V5, V6, V7, V8, SP0;         nop
nop; nop; sel_less.i32 V4, V5, V6, V7, V8, SP1;    nop
nop; nop; sel_less.i32 V4, V5, V6, V7, V8, !SP1;   nop
nop; nop; sel_less.i32 V4, V5, V6, V7, V8, VP1;    nop
nop; nop; sel_less.i32 V4, V5, V6, V7, V8, !VP1;   nop

// CHECK: nop; nop; sel_less.i32 V4, V5, V6, V7, V8, SP0; nop
// CHECK: nop; nop; sel_less.i32 V4, V5, V6, V7, V8, SP1; nop
// CHECK: nop; nop; sel_less.i32 V4, V5, V6, V7, V8, !SP1; nop
// CHECK: nop; nop; sel_less.i32 V4, V5, V6, V7, V8, VP1; nop
// CHECK: nop; nop; sel_less.i32 V4, V5, V6, V7, V8, !VP1; nop

nop; nop; sel_less.i32 V4, V5, S3, V7, V8, SP0;         nop
nop; nop; sel_less.i32 V4, V5, S3, V7, V8, SP1;    nop
nop; nop; sel_less.i32 V4, V5, S3, V7, V8, !SP1;   nop
nop; nop; sel_less.i32 V4, V5, S3, V7, V8, VP1;    nop
nop; nop; sel_less.i32 V4, V5, S3, V7, V8, !VP1;   nop

// CHECK: nop; nop; sel_less.i32 V4, V5, S3, V7, V8, SP0; nop
// CHECK: nop; nop; sel_less.i32 V4, V5, S3, V7, V8, SP1; nop
// CHECK: nop; nop; sel_less.i32 V4, V5, S3, V7, V8, !SP1; nop
// CHECK: nop; nop; sel_less.i32 V4, V5, S3, V7, V8, VP1; nop
// CHECK: nop; nop; sel_less.i32 V4, V5, S3, V7, V8, !VP1; nop

nop; nop; sel_less.i32 V4, V5, 122, V7, V8, SP0;         nop
nop; nop; sel_less.i32 V4, V5, 122, V7, V8, SP1;    nop
nop; nop; sel_less.i32 V4, V5, 122, V7, V8, !SP1;   nop
nop; nop; sel_less.i32 V4, V5, 122, V7, V8, VP1;    nop
nop; nop; sel_less.i32 V4, V5, 122, V7, V8, !VP1;   nop

// CHECK: nop; nop; sel_less.i32 V4, V5, 0x7a, V7, V8, SP0; nop
// CHECK: nop; nop; sel_less.i32 V4, V5, 0x7a, V7, V8, SP1; nop
// CHECK: nop; nop; sel_less.i32 V4, V5, 0x7a, V7, V8, !SP1; nop
// CHECK: nop; nop; sel_less.i32 V4, V5, 0x7a, V7, V8, VP1; nop
// CHECK: nop; nop; sel_less.i32 V4, V5, 0x7a, V7, V8, !VP1; nop

nop; nop; sel_less.i32 V4, V5, V6, V7, S3, SP0;         nop
nop; nop; sel_less.i32 V4, V5, V6, V7, S3, SP1;    nop
nop; nop; sel_less.i32 V4, V5, V6, V7, S3, !SP1;   nop
nop; nop; sel_less.i32 V4, V5, V6, V7, S3, VP1;    nop
nop; nop; sel_less.i32 V4, V5, V6, V7, S3, !VP1;   nop

// CHECK: nop; nop; sel_less.i32 V4, V5, V6, V7, S3, SP0; nop
// CHECK: nop; nop; sel_less.i32 V4, V5, V6, V7, S3, SP1; nop
// CHECK: nop; nop; sel_less.i32 V4, V5, V6, V7, S3, !SP1; nop
// CHECK: nop; nop; sel_less.i32 V4, V5, V6, V7, S3, VP1; nop
// CHECK: nop; nop; sel_less.i32 V4, V5, V6, V7, S3, !VP1; nop

nop; nop; sel_less.i32 V4, V5, V6, V7, 123;         nop
nop; nop; sel_less.i32 V4, V5, V6, V7, 123, SP1;    nop
nop; nop; sel_less.i32 V4, V5, V6, V7, 123, !SP1;   nop
nop; nop; sel_less.i32 V4, V5, V6, V7, 123, VP1;    nop
nop; nop; sel_less.i32 V4, V5, V6, V7, 123, !VP1;   nop

// CHECK: nop; nop; sel_less.i32 V4, V5, V6, V7, 0x7b, SP0; nop
// CHECK: nop; nop; sel_less.i32 V4, V5, V6, V7, 0x7b, SP1; nop
// CHECK: nop; nop; sel_less.i32 V4, V5, V6, V7, 0x7b, !SP1; nop
// CHECK: nop; nop; sel_less.i32 V4, V5, V6, V7, 0x7b, VP1; nop
// CHECK: nop; nop; sel_less.i32 V4, V5, V6, V7, 0x7b, !VP1; nop



nop; nop; sel_less.i16 V4, V5, V6, V7, V8, SP0;         nop
nop; nop; sel_less.i16 V4, V5, V6, V7, V8, SP1;    nop
nop; nop; sel_less.i16 V4, V5, V6, V7, V8, !SP1;   nop
nop; nop; sel_less.i16 V4, V5, V6, V7, V8, VP1;    nop
nop; nop; sel_less.i16 V4, V5, V6, V7, V8, !VP1;   nop

// CHECK: nop; nop; sel_less.i16 V4, V5, V6, V7, V8, SP0; nop
// CHECK: nop; nop; sel_less.i16 V4, V5, V6, V7, V8, SP1; nop
// CHECK: nop; nop; sel_less.i16 V4, V5, V6, V7, V8, !SP1; nop
// CHECK: nop; nop; sel_less.i16 V4, V5, V6, V7, V8, VP1; nop
// CHECK: nop; nop; sel_less.i16 V4, V5, V6, V7, V8, !VP1; nop

nop; nop; sel_less.i16 V4, V5, S3, V7, V8, SP0;         nop
nop; nop; sel_less.i16 V4, V5, S3, V7, V8, SP1;    nop
nop; nop; sel_less.i16 V4, V5, S3, V7, V8, !SP1;   nop
nop; nop; sel_less.i16 V4, V5, S3, V7, V8, VP1;    nop
nop; nop; sel_less.i16 V4, V5, S3, V7, V8, !VP1;   nop

// CHECK: nop; nop; sel_less.i16 V4, V5, S3, V7, V8, SP0; nop
// CHECK: nop; nop; sel_less.i16 V4, V5, S3, V7, V8, SP1; nop
// CHECK: nop; nop; sel_less.i16 V4, V5, S3, V7, V8, !SP1; nop
// CHECK: nop; nop; sel_less.i16 V4, V5, S3, V7, V8, VP1; nop
// CHECK: nop; nop; sel_less.i16 V4, V5, S3, V7, V8, !VP1; nop

nop; nop; sel_less.i16 V4, V5, 122, V7, V8, SP0;         nop
nop; nop; sel_less.i16 V4, V5, 122, V7, V8, SP1;    nop
nop; nop; sel_less.i16 V4, V5, 122, V7, V8, !SP1;   nop
nop; nop; sel_less.i16 V4, V5, 122, V7, V8, VP1;    nop
nop; nop; sel_less.i16 V4, V5, 122, V7, V8, !VP1;   nop

// CHECK: nop; nop; sel_less.i16 V4, V5, 0x7a, V7, V8, SP0; nop
// CHECK: nop; nop; sel_less.i16 V4, V5, 0x7a, V7, V8, SP1; nop
// CHECK: nop; nop; sel_less.i16 V4, V5, 0x7a, V7, V8, !SP1; nop
// CHECK: nop; nop; sel_less.i16 V4, V5, 0x7a, V7, V8, VP1; nop
// CHECK: nop; nop; sel_less.i16 V4, V5, 0x7a, V7, V8, !VP1; nop

nop; nop; sel_less.i16 V4, V5, V6, V7, S3, SP0;         nop
nop; nop; sel_less.i16 V4, V5, V6, V7, S3, SP1;    nop
nop; nop; sel_less.i16 V4, V5, V6, V7, S3, !SP1;   nop
nop; nop; sel_less.i16 V4, V5, V6, V7, S3, VP1;    nop
nop; nop; sel_less.i16 V4, V5, V6, V7, S3, !VP1;   nop

// CHECK: nop; nop; sel_less.i16 V4, V5, V6, V7, S3, SP0; nop
// CHECK: nop; nop; sel_less.i16 V4, V5, V6, V7, S3, SP1; nop
// CHECK: nop; nop; sel_less.i16 V4, V5, V6, V7, S3, !SP1; nop
// CHECK: nop; nop; sel_less.i16 V4, V5, V6, V7, S3, VP1; nop
// CHECK: nop; nop; sel_less.i16 V4, V5, V6, V7, S3, !VP1; nop

nop; nop; sel_less.i16 V4, V5, V6, V7, 123;         nop
nop; nop; sel_less.i16 V4, V5, V6, V7, 123, SP1;    nop
nop; nop; sel_less.i16 V4, V5, V6, V7, 123, !SP1;   nop
nop; nop; sel_less.i16 V4, V5, V6, V7, 123, VP1;    nop
nop; nop; sel_less.i16 V4, V5, V6, V7, 123, !VP1;   nop

// CHECK: nop; nop; sel_less.i16 V4, V5, V6, V7, 0x7b, SP0; nop
// CHECK: nop; nop; sel_less.i16 V4, V5, V6, V7, 0x7b, SP1; nop
// CHECK: nop; nop; sel_less.i16 V4, V5, V6, V7, 0x7b, !SP1; nop
// CHECK: nop; nop; sel_less.i16 V4, V5, V6, V7, 0x7b, VP1; nop
// CHECK: nop; nop; sel_less.i16 V4, V5, V6, V7, 0x7b, !VP1; nop



nop; nop; sel_less.i8 V4, V5, V6, V7, V8, SP0;         nop
nop; nop; sel_less.i8 V4, V5, V6, V7, V8, SP1;    nop
nop; nop; sel_less.i8 V4, V5, V6, V7, V8, !SP1;   nop
nop; nop; sel_less.i8 V4, V5, V6, V7, V8, VP1;    nop
nop; nop; sel_less.i8 V4, V5, V6, V7, V8, !VP1;   nop

// CHECK: nop; nop; sel_less.i8 V4, V5, V6, V7, V8, SP0; nop
// CHECK: nop; nop; sel_less.i8 V4, V5, V6, V7, V8, SP1; nop
// CHECK: nop; nop; sel_less.i8 V4, V5, V6, V7, V8, !SP1; nop
// CHECK: nop; nop; sel_less.i8 V4, V5, V6, V7, V8, VP1; nop
// CHECK: nop; nop; sel_less.i8 V4, V5, V6, V7, V8, !VP1; nop

nop; nop; sel_less.i8 V4, V5, S3, V7, V8, SP0;         nop
nop; nop; sel_less.i8 V4, V5, S3, V7, V8, SP1;    nop
nop; nop; sel_less.i8 V4, V5, S3, V7, V8, !SP1;   nop
nop; nop; sel_less.i8 V4, V5, S3, V7, V8, VP1;    nop
nop; nop; sel_less.i8 V4, V5, S3, V7, V8, !VP1;   nop

// CHECK: nop; nop; sel_less.i8 V4, V5, S3, V7, V8, SP0; nop
// CHECK: nop; nop; sel_less.i8 V4, V5, S3, V7, V8, SP1; nop
// CHECK: nop; nop; sel_less.i8 V4, V5, S3, V7, V8, !SP1; nop
// CHECK: nop; nop; sel_less.i8 V4, V5, S3, V7, V8, VP1; nop
// CHECK: nop; nop; sel_less.i8 V4, V5, S3, V7, V8, !VP1; nop

nop; nop; sel_less.i8 V4, V5, 122, V7, V8, SP0;         nop
nop; nop; sel_less.i8 V4, V5, 122, V7, V8, SP1;    nop
nop; nop; sel_less.i8 V4, V5, 122, V7, V8, !SP1;   nop
nop; nop; sel_less.i8 V4, V5, 122, V7, V8, VP1;    nop
nop; nop; sel_less.i8 V4, V5, 122, V7, V8, !VP1;   nop

// CHECK: nop; nop; sel_less.i8 V4, V5, 0x7a, V7, V8, SP0; nop
// CHECK: nop; nop; sel_less.i8 V4, V5, 0x7a, V7, V8, SP1; nop
// CHECK: nop; nop; sel_less.i8 V4, V5, 0x7a, V7, V8, !SP1; nop
// CHECK: nop; nop; sel_less.i8 V4, V5, 0x7a, V7, V8, VP1; nop
// CHECK: nop; nop; sel_less.i8 V4, V5, 0x7a, V7, V8, !VP1; nop

nop; nop; sel_less.i8 V4, V5, V6, V7, S3, SP0;         nop
nop; nop; sel_less.i8 V4, V5, V6, V7, S3, SP1;    nop
nop; nop; sel_less.i8 V4, V5, V6, V7, S3, !SP1;   nop
nop; nop; sel_less.i8 V4, V5, V6, V7, S3, VP1;    nop
nop; nop; sel_less.i8 V4, V5, V6, V7, S3, !VP1;   nop

// CHECK: nop; nop; sel_less.i8 V4, V5, V6, V7, S3, SP0; nop
// CHECK: nop; nop; sel_less.i8 V4, V5, V6, V7, S3, SP1; nop
// CHECK: nop; nop; sel_less.i8 V4, V5, V6, V7, S3, !SP1; nop
// CHECK: nop; nop; sel_less.i8 V4, V5, V6, V7, S3, VP1; nop
// CHECK: nop; nop; sel_less.i8 V4, V5, V6, V7, S3, !VP1; nop

nop; nop; sel_less.i8 V4, V5, V6, V7, 123;         nop
nop; nop; sel_less.i8 V4, V5, V6, V7, 123, SP1;    nop
nop; nop; sel_less.i8 V4, V5, V6, V7, 123, !SP1;   nop
nop; nop; sel_less.i8 V4, V5, V6, V7, 123, VP1;    nop
nop; nop; sel_less.i8 V4, V5, V6, V7, 123, !VP1;   nop

// CHECK: nop; nop; sel_less.i8 V4, V5, V6, V7, 0x7b, SP0; nop
// CHECK: nop; nop; sel_less.i8 V4, V5, V6, V7, 0x7b, SP1; nop
// CHECK: nop; nop; sel_less.i8 V4, V5, V6, V7, 0x7b, !SP1; nop
// CHECK: nop; nop; sel_less.i8 V4, V5, V6, V7, 0x7b, VP1; nop
// CHECK: nop; nop; sel_less.i8 V4, V5, V6, V7, 0x7b, !VP1; nop



nop; nop; sel_less.u32 V4, V5, V6, V7, V8, SP0;         nop
nop; nop; sel_less.u32 V4, V5, V6, V7, V8, SP1;    nop
nop; nop; sel_less.u32 V4, V5, V6, V7, V8, !SP1;   nop
nop; nop; sel_less.u32 V4, V5, V6, V7, V8, VP1;    nop
nop; nop; sel_less.u32 V4, V5, V6, V7, V8, !VP1;   nop

// CHECK: nop; nop; sel_less.u32 V4, V5, V6, V7, V8, SP0; nop
// CHECK: nop; nop; sel_less.u32 V4, V5, V6, V7, V8, SP1; nop
// CHECK: nop; nop; sel_less.u32 V4, V5, V6, V7, V8, !SP1; nop
// CHECK: nop; nop; sel_less.u32 V4, V5, V6, V7, V8, VP1; nop
// CHECK: nop; nop; sel_less.u32 V4, V5, V6, V7, V8, !VP1; nop

nop; nop; sel_less.u32 V4, V5, S3, V7, V8, SP0;         nop
nop; nop; sel_less.u32 V4, V5, S3, V7, V8, SP1;    nop
nop; nop; sel_less.u32 V4, V5, S3, V7, V8, !SP1;   nop
nop; nop; sel_less.u32 V4, V5, S3, V7, V8, VP1;    nop
nop; nop; sel_less.u32 V4, V5, S3, V7, V8, !VP1;   nop

// CHECK: nop; nop; sel_less.u32 V4, V5, S3, V7, V8, SP0; nop
// CHECK: nop; nop; sel_less.u32 V4, V5, S3, V7, V8, SP1; nop
// CHECK: nop; nop; sel_less.u32 V4, V5, S3, V7, V8, !SP1; nop
// CHECK: nop; nop; sel_less.u32 V4, V5, S3, V7, V8, VP1; nop
// CHECK: nop; nop; sel_less.u32 V4, V5, S3, V7, V8, !VP1; nop

nop; nop; sel_less.u32 V4, V5, 122, V7, V8, SP0;         nop
nop; nop; sel_less.u32 V4, V5, 122, V7, V8, SP1;    nop
nop; nop; sel_less.u32 V4, V5, 122, V7, V8, !SP1;   nop
nop; nop; sel_less.u32 V4, V5, 122, V7, V8, VP1;    nop
nop; nop; sel_less.u32 V4, V5, 122, V7, V8, !VP1;   nop

// CHECK: nop; nop; sel_less.u32 V4, V5, 0x7a, V7, V8, SP0; nop
// CHECK: nop; nop; sel_less.u32 V4, V5, 0x7a, V7, V8, SP1; nop
// CHECK: nop; nop; sel_less.u32 V4, V5, 0x7a, V7, V8, !SP1; nop
// CHECK: nop; nop; sel_less.u32 V4, V5, 0x7a, V7, V8, VP1; nop
// CHECK: nop; nop; sel_less.u32 V4, V5, 0x7a, V7, V8, !VP1; nop

nop; nop; sel_less.u32 V4, V5, V6, V7, S3, SP0;         nop
nop; nop; sel_less.u32 V4, V5, V6, V7, S3, SP1;    nop
nop; nop; sel_less.u32 V4, V5, V6, V7, S3, !SP1;   nop
nop; nop; sel_less.u32 V4, V5, V6, V7, S3, VP1;    nop
nop; nop; sel_less.u32 V4, V5, V6, V7, S3, !VP1;   nop

// CHECK: nop; nop; sel_less.u32 V4, V5, V6, V7, S3, SP0; nop
// CHECK: nop; nop; sel_less.u32 V4, V5, V6, V7, S3, SP1; nop
// CHECK: nop; nop; sel_less.u32 V4, V5, V6, V7, S3, !SP1; nop
// CHECK: nop; nop; sel_less.u32 V4, V5, V6, V7, S3, VP1; nop
// CHECK: nop; nop; sel_less.u32 V4, V5, V6, V7, S3, !VP1; nop

nop; nop; sel_less.u32 V4, V5, V6, V7, 123;         nop
nop; nop; sel_less.u32 V4, V5, V6, V7, 123, SP1;    nop
nop; nop; sel_less.u32 V4, V5, V6, V7, 123, !SP1;   nop
nop; nop; sel_less.u32 V4, V5, V6, V7, 123, VP1;    nop
nop; nop; sel_less.u32 V4, V5, V6, V7, 123, !VP1;   nop

// CHECK: nop; nop; sel_less.u32 V4, V5, V6, V7, 0x7b, SP0; nop
// CHECK: nop; nop; sel_less.u32 V4, V5, V6, V7, 0x7b, SP1; nop
// CHECK: nop; nop; sel_less.u32 V4, V5, V6, V7, 0x7b, !SP1; nop
// CHECK: nop; nop; sel_less.u32 V4, V5, V6, V7, 0x7b, VP1; nop
// CHECK: nop; nop; sel_less.u32 V4, V5, V6, V7, 0x7b, !VP1; nop



nop; nop; sel_less.u16 V4, V5, V6, V7, V8, SP0;         nop
nop; nop; sel_less.u16 V4, V5, V6, V7, V8, SP1;    nop
nop; nop; sel_less.u16 V4, V5, V6, V7, V8, !SP1;   nop
nop; nop; sel_less.u16 V4, V5, V6, V7, V8, VP1;    nop
nop; nop; sel_less.u16 V4, V5, V6, V7, V8, !VP1;   nop

// CHECK: nop; nop; sel_less.u16 V4, V5, V6, V7, V8, SP0; nop
// CHECK: nop; nop; sel_less.u16 V4, V5, V6, V7, V8, SP1; nop
// CHECK: nop; nop; sel_less.u16 V4, V5, V6, V7, V8, !SP1; nop
// CHECK: nop; nop; sel_less.u16 V4, V5, V6, V7, V8, VP1; nop
// CHECK: nop; nop; sel_less.u16 V4, V5, V6, V7, V8, !VP1; nop

nop; nop; sel_less.u16 V4, V5, S3, V7, V8, SP0;         nop
nop; nop; sel_less.u16 V4, V5, S3, V7, V8, SP1;    nop
nop; nop; sel_less.u16 V4, V5, S3, V7, V8, !SP1;   nop
nop; nop; sel_less.u16 V4, V5, S3, V7, V8, VP1;    nop
nop; nop; sel_less.u16 V4, V5, S3, V7, V8, !VP1;   nop

// CHECK: nop; nop; sel_less.u16 V4, V5, S3, V7, V8, SP0; nop
// CHECK: nop; nop; sel_less.u16 V4, V5, S3, V7, V8, SP1; nop
// CHECK: nop; nop; sel_less.u16 V4, V5, S3, V7, V8, !SP1; nop
// CHECK: nop; nop; sel_less.u16 V4, V5, S3, V7, V8, VP1; nop
// CHECK: nop; nop; sel_less.u16 V4, V5, S3, V7, V8, !VP1; nop

nop; nop; sel_less.u16 V4, V5, 122, V7, V8, SP0;         nop
nop; nop; sel_less.u16 V4, V5, 122, V7, V8, SP1;    nop
nop; nop; sel_less.u16 V4, V5, 122, V7, V8, !SP1;   nop
nop; nop; sel_less.u16 V4, V5, 122, V7, V8, VP1;    nop
nop; nop; sel_less.u16 V4, V5, 122, V7, V8, !VP1;   nop

// CHECK: nop; nop; sel_less.u16 V4, V5, 0x7a, V7, V8, SP0; nop
// CHECK: nop; nop; sel_less.u16 V4, V5, 0x7a, V7, V8, SP1; nop
// CHECK: nop; nop; sel_less.u16 V4, V5, 0x7a, V7, V8, !SP1; nop
// CHECK: nop; nop; sel_less.u16 V4, V5, 0x7a, V7, V8, VP1; nop
// CHECK: nop; nop; sel_less.u16 V4, V5, 0x7a, V7, V8, !VP1; nop

nop; nop; sel_less.u16 V4, V5, V6, V7, S3, SP0;         nop
nop; nop; sel_less.u16 V4, V5, V6, V7, S3, SP1;    nop
nop; nop; sel_less.u16 V4, V5, V6, V7, S3, !SP1;   nop
nop; nop; sel_less.u16 V4, V5, V6, V7, S3, VP1;    nop
nop; nop; sel_less.u16 V4, V5, V6, V7, S3, !VP1;   nop

// CHECK: nop; nop; sel_less.u16 V4, V5, V6, V7, S3, SP0; nop
// CHECK: nop; nop; sel_less.u16 V4, V5, V6, V7, S3, SP1; nop
// CHECK: nop; nop; sel_less.u16 V4, V5, V6, V7, S3, !SP1; nop
// CHECK: nop; nop; sel_less.u16 V4, V5, V6, V7, S3, VP1; nop
// CHECK: nop; nop; sel_less.u16 V4, V5, V6, V7, S3, !VP1; nop

nop; nop; sel_less.u16 V4, V5, V6, V7, 123;         nop
nop; nop; sel_less.u16 V4, V5, V6, V7, 123, SP1;    nop
nop; nop; sel_less.u16 V4, V5, V6, V7, 123, !SP1;   nop
nop; nop; sel_less.u16 V4, V5, V6, V7, 123, VP1;    nop
nop; nop; sel_less.u16 V4, V5, V6, V7, 123, !VP1;   nop

// CHECK: nop; nop; sel_less.u16 V4, V5, V6, V7, 0x7b, SP0; nop
// CHECK: nop; nop; sel_less.u16 V4, V5, V6, V7, 0x7b, SP1; nop
// CHECK: nop; nop; sel_less.u16 V4, V5, V6, V7, 0x7b, !SP1; nop
// CHECK: nop; nop; sel_less.u16 V4, V5, V6, V7, 0x7b, VP1; nop
// CHECK: nop; nop; sel_less.u16 V4, V5, V6, V7, 0x7b, !VP1; nop



nop; nop; sel_less.u8 V4, V5, V6, V7, V8, SP0;         nop
nop; nop; sel_less.u8 V4, V5, V6, V7, V8, SP1;    nop
nop; nop; sel_less.u8 V4, V5, V6, V7, V8, !SP1;   nop
nop; nop; sel_less.u8 V4, V5, V6, V7, V8, VP1;    nop
nop; nop; sel_less.u8 V4, V5, V6, V7, V8, !VP1;   nop

// CHECK: nop; nop; sel_less.u8 V4, V5, V6, V7, V8, SP0; nop
// CHECK: nop; nop; sel_less.u8 V4, V5, V6, V7, V8, SP1; nop
// CHECK: nop; nop; sel_less.u8 V4, V5, V6, V7, V8, !SP1; nop
// CHECK: nop; nop; sel_less.u8 V4, V5, V6, V7, V8, VP1; nop
// CHECK: nop; nop; sel_less.u8 V4, V5, V6, V7, V8, !VP1; nop

nop; nop; sel_less.u8 V4, V5, S3, V7, V8, SP0;         nop
nop; nop; sel_less.u8 V4, V5, S3, V7, V8, SP1;    nop
nop; nop; sel_less.u8 V4, V5, S3, V7, V8, !SP1;   nop
nop; nop; sel_less.u8 V4, V5, S3, V7, V8, VP1;    nop
nop; nop; sel_less.u8 V4, V5, S3, V7, V8, !VP1;   nop

// CHECK: nop; nop; sel_less.u8 V4, V5, S3, V7, V8, SP0; nop
// CHECK: nop; nop; sel_less.u8 V4, V5, S3, V7, V8, SP1; nop
// CHECK: nop; nop; sel_less.u8 V4, V5, S3, V7, V8, !SP1; nop
// CHECK: nop; nop; sel_less.u8 V4, V5, S3, V7, V8, VP1; nop
// CHECK: nop; nop; sel_less.u8 V4, V5, S3, V7, V8, !VP1; nop

nop; nop; sel_less.u8 V4, V5, 122, V7, V8, SP0;         nop
nop; nop; sel_less.u8 V4, V5, 122, V7, V8, SP1;    nop
nop; nop; sel_less.u8 V4, V5, 122, V7, V8, !SP1;   nop
nop; nop; sel_less.u8 V4, V5, 122, V7, V8, VP1;    nop
nop; nop; sel_less.u8 V4, V5, 122, V7, V8, !VP1;   nop

// CHECK: nop; nop; sel_less.u8 V4, V5, 0x7a, V7, V8, SP0; nop
// CHECK: nop; nop; sel_less.u8 V4, V5, 0x7a, V7, V8, SP1; nop
// CHECK: nop; nop; sel_less.u8 V4, V5, 0x7a, V7, V8, !SP1; nop
// CHECK: nop; nop; sel_less.u8 V4, V5, 0x7a, V7, V8, VP1; nop
// CHECK: nop; nop; sel_less.u8 V4, V5, 0x7a, V7, V8, !VP1; nop

nop; nop; sel_less.u8 V4, V5, V6, V7, S3, SP0;         nop
nop; nop; sel_less.u8 V4, V5, V6, V7, S3, SP1;    nop
nop; nop; sel_less.u8 V4, V5, V6, V7, S3, !SP1;   nop
nop; nop; sel_less.u8 V4, V5, V6, V7, S3, VP1;    nop
nop; nop; sel_less.u8 V4, V5, V6, V7, S3, !VP1;   nop

// CHECK: nop; nop; sel_less.u8 V4, V5, V6, V7, S3, SP0; nop
// CHECK: nop; nop; sel_less.u8 V4, V5, V6, V7, S3, SP1; nop
// CHECK: nop; nop; sel_less.u8 V4, V5, V6, V7, S3, !SP1; nop
// CHECK: nop; nop; sel_less.u8 V4, V5, V6, V7, S3, VP1; nop
// CHECK: nop; nop; sel_less.u8 V4, V5, V6, V7, S3, !VP1; nop

nop; nop; sel_less.u8 V4, V5, V6, V7, 123;         nop
nop; nop; sel_less.u8 V4, V5, V6, V7, 123, SP1;    nop
nop; nop; sel_less.u8 V4, V5, V6, V7, 123, !SP1;   nop
nop; nop; sel_less.u8 V4, V5, V6, V7, 123, VP1;    nop
nop; nop; sel_less.u8 V4, V5, V6, V7, 123, !VP1;   nop

// CHECK: nop; nop; sel_less.u8 V4, V5, V6, V7, 0x7b, SP0; nop
// CHECK: nop; nop; sel_less.u8 V4, V5, V6, V7, 0x7b, SP1; nop
// CHECK: nop; nop; sel_less.u8 V4, V5, V6, V7, 0x7b, !SP1; nop
// CHECK: nop; nop; sel_less.u8 V4, V5, V6, V7, 0x7b, VP1; nop
// CHECK: nop; nop; sel_less.u8 V4, V5, V6, V7, 0x7b, !VP1; nop



nop; nop; sel_less.f32 V4, V5, V6, V7, V8, SP0;         nop
nop; nop; sel_less.f32 V4, V5, V6, V7, V8, SP1;    nop
nop; nop; sel_less.f32 V4, V5, V6, V7, V8, !SP1;   nop
nop; nop; sel_less.f32 V4, V5, V6, V7, V8, VP1;    nop
nop; nop; sel_less.f32 V4, V5, V6, V7, V8, !VP1;   nop

// CHECK: nop; nop; sel_less.f32 V4, V5, V6, V7, V8, SP0; nop
// CHECK: nop; nop; sel_less.f32 V4, V5, V6, V7, V8, SP1; nop
// CHECK: nop; nop; sel_less.f32 V4, V5, V6, V7, V8, !SP1; nop
// CHECK: nop; nop; sel_less.f32 V4, V5, V6, V7, V8, VP1; nop
// CHECK: nop; nop; sel_less.f32 V4, V5, V6, V7, V8, !VP1; nop

nop; nop; sel_less.f32 V4, V5, S3, V7, V8, SP0;         nop
nop; nop; sel_less.f32 V4, V5, S3, V7, V8, SP1;    nop
nop; nop; sel_less.f32 V4, V5, S3, V7, V8, !SP1;   nop
nop; nop; sel_less.f32 V4, V5, S3, V7, V8, VP1;    nop
nop; nop; sel_less.f32 V4, V5, S3, V7, V8, !VP1;   nop

// CHECK: nop; nop; sel_less.f32 V4, V5, S3, V7, V8, SP0; nop
// CHECK: nop; nop; sel_less.f32 V4, V5, S3, V7, V8, SP1; nop
// CHECK: nop; nop; sel_less.f32 V4, V5, S3, V7, V8, !SP1; nop
// CHECK: nop; nop; sel_less.f32 V4, V5, S3, V7, V8, VP1; nop
// CHECK: nop; nop; sel_less.f32 V4, V5, S3, V7, V8, !VP1; nop

nop; nop; sel_less.f32 V4, V5, 0x3f800000, V7, V8, SP0;         nop
nop; nop; sel_less.f32 V4, V5, 0x3f800000, V7, V8, SP1;    nop
nop; nop; sel_less.f32 V4, V5, 0x3f800000, V7, V8, !SP1;   nop
nop; nop; sel_less.f32 V4, V5, 0x3f800000, V7, V8, VP1;    nop
nop; nop; sel_less.f32 V4, V5, 0x3f800000, V7, V8, !VP1;   nop

// CHECK: nop; nop; sel_less.f32 V4, V5, 0x3f800000, V7, V8, SP0; nop
// CHECK: nop; nop; sel_less.f32 V4, V5, 0x3f800000, V7, V8, SP1; nop
// CHECK: nop; nop; sel_less.f32 V4, V5, 0x3f800000, V7, V8, !SP1; nop
// CHECK: nop; nop; sel_less.f32 V4, V5, 0x3f800000, V7, V8, VP1; nop
// CHECK: nop; nop; sel_less.f32 V4, V5, 0x3f800000, V7, V8, !VP1; nop

nop; nop; sel_less.f32 V4, V5, V6, V7, S3, SP0;         nop
nop; nop; sel_less.f32 V4, V5, V6, V7, S3, SP1;    nop
nop; nop; sel_less.f32 V4, V5, V6, V7, S3, !SP1;   nop
nop; nop; sel_less.f32 V4, V5, V6, V7, S3, VP1;    nop
nop; nop; sel_less.f32 V4, V5, V6, V7, S3, !VP1;   nop

// CHECK: nop; nop; sel_less.f32 V4, V5, V6, V7, S3, SP0; nop
// CHECK: nop; nop; sel_less.f32 V4, V5, V6, V7, S3, SP1; nop
// CHECK: nop; nop; sel_less.f32 V4, V5, V6, V7, S3, !SP1; nop
// CHECK: nop; nop; sel_less.f32 V4, V5, V6, V7, S3, VP1; nop
// CHECK: nop; nop; sel_less.f32 V4, V5, V6, V7, S3, !VP1; nop

nop; nop; sel_less.f32 V4, V5, V6, V7, 0x3f800000;         nop
nop; nop; sel_less.f32 V4, V5, V6, V7, 0x3f800000, SP1;    nop
nop; nop; sel_less.f32 V4, V5, V6, V7, 0x3f800000, !SP1;   nop
nop; nop; sel_less.f32 V4, V5, V6, V7, 0x3f800000, VP1;    nop
nop; nop; sel_less.f32 V4, V5, V6, V7, 0x3f800000, !VP1;   nop

// CHECK: nop; nop; sel_less.f32 V4, V5, V6, V7, 0x3f800000, SP0; nop
// CHECK: nop; nop; sel_less.f32 V4, V5, V6, V7, 0x3f800000, SP1; nop
// CHECK: nop; nop; sel_less.f32 V4, V5, V6, V7, 0x3f800000, !SP1; nop
// CHECK: nop; nop; sel_less.f32 V4, V5, V6, V7, 0x3f800000, VP1; nop
// CHECK: nop; nop; sel_less.f32 V4, V5, V6, V7, 0x3f800000, !VP1; nop