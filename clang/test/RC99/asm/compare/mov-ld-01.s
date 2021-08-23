// RUN: %clang -cc1as -triple tpc-none-none -filetype obj %s -o %t.o
// RUN: %disasm %t.o | FileCheck %s


MOV V14, V10
MOV V14, V10, SP2
MOV V14, V10, !SP2
MOV V14, V10, VP2
MOV V14, V10, !VP2

// CHECK: mov V14, V10, SP0;      nop;    nop;    nop
// CHECK: mov V14, V10, SP2;      nop;    nop;    nop
// CHECK: mov V14, V10, !SP2;     nop;    nop;    nop
// CHECK: mov V14, V10, VP2;      nop;    nop;    nop
// CHECK: mov V14, V10, !VP2;     nop;    nop;    nop

MOV.I32 V14, V10
MOV.I16 V14, V10, SP2
MOV.I8 V14, V10, !SP2
MOV.F32 V14, V10, VP2
MOV.U32 V14, V10, !VP2

// CHECK: mov V14, V10, SP0;      nop;    nop;    nop
// CHECK: mov V14, V10, SP2;      nop;    nop;    nop
// CHECK: mov V14, V10, !SP2;     nop;    nop;    nop
// CHECK: mov V14, V10, VP2;      nop;    nop;    nop
// CHECK: mov V14, V10, !VP2;     nop;    nop;    nop


MOV VP14, VP12
MOV VP14, VP12, SP2
MOV VP14, VP12, !SP2
MOV VP14, VP12, VP2
MOV VP14, VP12, !VP2

// CHECK: mov VP14, VP12, SP0;    nop;    nop;    nop
// CHECK: mov VP14, VP12, SP2;    nop;    nop;    nop
// CHECK: mov VP14, VP12, !SP2;   nop;    nop;    nop
// CHECK: mov VP14, VP12, VP2;    nop;    nop;    nop
// CHECK: mov VP14, VP12, !VP2;   nop;    nop;    nop


MOV VP14, SP12
MOV VP14, SP12, SP2
MOV VP14, SP12, !SP2
MOV VP14, SP12, VP2
MOV VP14, SP12, !VP2
 
// CHECK: mov VP14, SP12, SP0;    nop;    nop;    nop
// CHECK: mov VP14, SP12, SP2;    nop;    nop;    nop
// CHECK: mov VP14, SP12, !SP2;   nop;    nop;    nop
// CHECK: mov VP14, SP12, VP2;    nop;    nop;    nop
// CHECK: mov VP14, SP12, !VP2;   nop;    nop;    nop


MOV.I8 V14, 123
MOV.I8 V14, 123, SP2
MOV.I8 V14, 123, !SP2
MOV.I8 V14, 123, VP2
MOV.I8 V14, 123, !VP2

// CHECK: mov.i8 V14, 0x7b, SP0;  nop;    nop;    nop
// CHECK: mov.i8 V14, 0x7b, SP2;  nop;    nop;    nop
// CHECK: mov.i8 V14, 0x7b, !SP2;         nop;    nop;    nop
// CHECK: mov.i8 V14, 0x7b, VP2;  nop;    nop;    nop
// CHECK: mov.i8 V14, 0x7b, !VP2;         nop;    nop;    nop

MOV.U8 V14, 123
MOV.U8 V14, 123, SP2
MOV.U8 V14, 123, !SP2
MOV.U8 V14, 123, VP2
MOV.U8 V14, 123, !VP2

// CHECK: mov.u8 V14, 0x7b, SP0;  nop;    nop;    nop
// CHECK: mov.u8 V14, 0x7b, SP2;  nop;    nop;    nop
// CHECK: mov.u8 V14, 0x7b, !SP2;         nop;    nop;    nop
// CHECK: mov.u8 V14, 0x7b, VP2;  nop;    nop;    nop
// CHECK: mov.u8 V14, 0x7b, !VP2;         nop;    nop;    nop

MOV.I16 V14, 123
MOV.I16 V14, 123, SP2
MOV.I16 V14, 123, !SP2
MOV.I16 V14, 123, VP2
MOV.I16 V14, 123, !VP2

// CHECK: mov.i16 V14, 0x7b, SP0;         nop;    nop;    nop
// CHECK: mov.i16 V14, 0x7b, SP2;         nop;    nop;    nop
// CHECK: mov.i16 V14, 0x7b, !SP2;        nop;    nop;    nop
// CHECK: mov.i16 V14, 0x7b, VP2;         nop;    nop;    nop
// CHECK: mov.i16 V14, 0x7b, !VP2;        nop;    nop;    nop

MOV.U16 V14, 123
MOV.U16 V14, 123, SP2
MOV.U16 V14, 123, !SP2
MOV.U16 V14, 123, VP2
MOV.U16 V14, 123, !VP2

// CHECK: mov.u16 V14, 0x7b, SP0;         nop;    nop;    nop
// CHECK: mov.u16 V14, 0x7b, SP2;         nop;    nop;    nop
// CHECK: mov.u16 V14, 0x7b, !SP2;        nop;    nop;    nop
// CHECK: mov.u16 V14, 0x7b, VP2;         nop;    nop;    nop
// CHECK: mov.u16 V14, 0x7b, !VP2;        nop;    nop;    nop

MOV.I32 V14, 123
MOV.I32 V14, 123, SP2
MOV.I32 V14, 123, !SP2
MOV.I32 V14, 123, VP2
MOV.I32 V14, 123, !VP2

// CHECK: mov.i32 V14, 0x7b, SP0;         nop;    nop;    nop
// CHECK: mov.i32 V14, 0x7b, SP2;         nop;    nop;    nop
// CHECK: mov.i32 V14, 0x7b, !SP2;        nop;    nop;    nop
// CHECK: mov.i32 V14, 0x7b, VP2;         nop;    nop;    nop
// CHECK: mov.i32 V14, 0x7b, !VP2;        nop;    nop;    nop

MOV.U32 V14, 123
MOV.U32 V14, 123, SP2
MOV.U32 V14, 123, !SP2
MOV.U32 V14, 123, VP2
MOV.U32 V14, 123, !VP2

// CHECK: mov.u32 V14, 0x7b, SP0;         nop;    nop;    nop
// CHECK: mov.u32 V14, 0x7b, SP2;         nop;    nop;    nop
// CHECK: mov.u32 V14, 0x7b, !SP2;        nop;    nop;    nop
// CHECK: mov.u32 V14, 0x7b, VP2;         nop;    nop;    nop
// CHECK: mov.u32 V14, 0x7b, !VP2;        nop;    nop;    nop

MOV.F32 V14, 0x3f800000
MOV.F32 V14, 0x3f800000, SP2
MOV.F32 V14, 0x3f800000, !SP2
MOV.F32 V14, 0x3f800000, VP2
MOV.F32 V14, 0x3f800000, !VP2

// CHECK: mov.f32 V14, 0x3f800000, SP0;   nop;    nop;    nop
// CHECK: mov.f32 V14, 0x3f800000, SP2;   nop;    nop;    nop
// CHECK: mov.f32 V14, 0x3f800000, !SP2;  nop;    nop;    nop
// CHECK: mov.f32 V14, 0x3f800000, VP2;   nop;    nop;    nop
// CHECK: mov.f32 V14, 0x3f800000, !VP2;  nop;    nop;    nop


MOV.I8 V14, S1
MOV.I8 V14, S1, SP2
MOV.I8 V14, S1, !SP2
MOV.I8 V14, S1, VP2
MOV.I8 V14, S1, !VP2

// CHECK: mov.i8 V14, S1, SP0;    nop;    nop;    nop
// CHECK: mov.i8 V14, S1, SP2;    nop;    nop;    nop
// CHECK: mov.i8 V14, S1, !SP2;   nop;    nop;    nop
// CHECK: mov.i8 V14, S1, VP2;    nop;    nop;    nop
// CHECK: mov.i8 V14, S1, !VP2;   nop;    nop;    nop

MOV.U8 V14, S1
MOV.U8 V14, S1, SP2
MOV.U8 V14, S1, !SP2
MOV.U8 V14, S1, VP2
MOV.U8 V14, S1, !VP2

// CHECK: mov.u8 V14, S1, SP0;    nop;    nop;    nop
// CHECK: mov.u8 V14, S1, SP2;    nop;    nop;    nop
// CHECK: mov.u8 V14, S1, !SP2;   nop;    nop;    nop
// CHECK: mov.u8 V14, S1, VP2;    nop;    nop;    nop
// CHECK: mov.u8 V14, S1, !VP2;   nop;    nop;    nop

MOV.I16 V14, S1
MOV.I16 V14, S1, SP2
MOV.I16 V14, S1, !SP2
MOV.I16 V14, S1, VP2
MOV.I16 V14, S1, !VP2

// CHECK: mov.i16 V14, S1, SP0;   nop;    nop;    nop
// CHECK: mov.i16 V14, S1, SP2;   nop;    nop;    nop
// CHECK: mov.i16 V14, S1, !SP2;  nop;    nop;    nop
// CHECK: mov.i16 V14, S1, VP2;   nop;    nop;    nop
// CHECK: mov.i16 V14, S1, !VP2;  nop;    nop;    nop

MOV.U16 V14, S1
MOV.U16 V14, S1, SP2
MOV.U16 V14, S1, !SP2
MOV.U16 V14, S1, VP2
MOV.U16 V14, S1, !VP2

// CHECK: mov.u16 V14, S1, SP0;   nop;    nop;    nop
// CHECK: mov.u16 V14, S1, SP2;   nop;    nop;    nop
// CHECK: mov.u16 V14, S1, !SP2;  nop;    nop;    nop
// CHECK: mov.u16 V14, S1, VP2;   nop;    nop;    nop
// CHECK: mov.u16 V14, S1, !VP2;  nop;    nop;    nop

MOV.I32 V14, S1
MOV.I32 V14, S1, SP2
MOV.I32 V14, S1, !SP2
MOV.I32 V14, S1, VP2
MOV.I32 V14, S1, !VP2

// CHECK: mov.i32 V14, S1, SP0;   nop;    nop;    nop
// CHECK: mov.i32 V14, S1, SP2;   nop;    nop;    nop
// CHECK: mov.i32 V14, S1, !SP2;  nop;    nop;    nop
// CHECK: mov.i32 V14, S1, VP2;   nop;    nop;    nop
// CHECK: mov.i32 V14, S1, !VP2;  nop;    nop;    nop

MOV.U32 V14, S1
MOV.U32 V14, S1, SP2
MOV.U32 V14, S1, !SP2
MOV.U32 V14, S1, VP2
MOV.U32 V14, S1, !VP2

// CHECK: mov.u32 V14, S1, SP0;   nop;    nop;    nop
// CHECK: mov.u32 V14, S1, SP2;   nop;    nop;    nop
// CHECK: mov.u32 V14, S1, !SP2;  nop;    nop;    nop
// CHECK: mov.u32 V14, S1, VP2;   nop;    nop;    nop
// CHECK: mov.u32 V14, S1, !VP2;  nop;    nop;    nop

MOV.F32 V14, S1
MOV.F32 V14, S1, SP2
MOV.F32 V14, S1, !SP2
MOV.F32 V14, S1, VP2
MOV.F32 V14, S1, !VP2

// CHECK: mov.f32 V14, S1, SP0;   nop;    nop;    nop
// CHECK: mov.f32 V14, S1, SP2;   nop;    nop;    nop
// CHECK: mov.f32 V14, S1, !SP2;  nop;    nop;    nop
// CHECK: mov.f32 V14, S1, VP2;   nop;    nop;    nop
// CHECK: mov.f32 V14, S1, !VP2;  nop;    nop;    nop

MOV 3 VP14, 123
MOV 3 VP14, 123, SP2
MOV 3 VP14, 123, !SP2
MOV 3 VP14, 123, VP2
MOV 3 VP14, 123, !VP2

// CHECK: mov 0x3 VP14, 0x7b, SP0;        nop;    nop;    nop
// CHECK: mov 0x3 VP14, 0x7b, SP2;        nop;    nop;    nop
// CHECK: mov 0x3 VP14, 0x7b, !SP2;       nop;    nop;    nop
// CHECK: mov 0x3 VP14, 0x7b, VP2;        nop;    nop;    nop
// CHECK: mov 0x3 VP14, 0x7b, !VP2;       nop;    nop;    nop

MOV 8 VP14, 123
MOV 8 VP14, 123, SP2
MOV 8 VP14, 123, !SP2
MOV 8 VP14, 123, VP2
MOV 8 VP14, 123, !VP2

// CHECK: mov VP14, 0x7b, SP0;        nop;    nop;    nop
// CHECK: mov VP14, 0x7b, SP2;        nop;    nop;    nop
// CHECK: mov VP14, 0x7b, !SP2;       nop;    nop;    nop
// CHECK: mov VP14, 0x7b, VP2;        nop;    nop;    nop
// CHECK: mov VP14, 0x7b, !VP2;       nop;    nop;    nop

MOV VP14, 123
MOV VP14, 123, SP2
MOV VP14, 123, !SP2
MOV VP14, 123, VP2
MOV VP14, 123, !VP2

// CHECK: mov VP14, 0x7b, SP0;        nop;    nop;    nop
// CHECK: mov VP14, 0x7b, SP2;        nop;    nop;    nop
// CHECK: mov VP14, 0x7b, !SP2;       nop;    nop;    nop
// CHECK: mov VP14, 0x7b, VP2;        nop;    nop;    nop
// CHECK: mov VP14, 0x7b, !VP2;       nop;    nop;    nop

MOV 3 VP14, S3
MOV 3 VP14, S3, SP2
MOV 3 VP14, S3, !SP2
MOV 3 VP14, S3, VP2
MOV 3 VP14, S3, !VP2

// CHECK: mov 0x3 VP14, S3, SP0;  nop;    nop;    nop
// CHECK: mov 0x3 VP14, S3, SP2;  nop;    nop;    nop
// CHECK: mov 0x3 VP14, S3, !SP2;         nop;    nop;    nop
// CHECK: mov 0x3 VP14, S3, VP2;  nop;    nop;    nop
// CHECK: mov 0x3 VP14, S3, !VP2;         nop;    nop;    nop

MOV 8 VP14, S3
MOV 8 VP14, S3, SP2
MOV 8 VP14, S3, !SP2
MOV 8 VP14, S3, VP2
MOV 8 VP14, S3, !VP2

// CHECK: mov VP14, S3, SP0;  nop;    nop;    nop
// CHECK: mov VP14, S3, SP2;  nop;    nop;    nop
// CHECK: mov VP14, S3, !SP2;         nop;    nop;    nop
// CHECK: mov VP14, S3, VP2;  nop;    nop;    nop
// CHECK: mov VP14, S3, !VP2;         nop;    nop;    nop

MOV VP14, S3
MOV VP14, S3, SP2
MOV VP14, S3, !SP2
MOV VP14, S3, VP2
MOV VP14, S3, !VP2

// CHECK: mov VP14, S3, SP0;  nop;    nop;    nop
// CHECK: mov VP14, S3, SP2;  nop;    nop;    nop
// CHECK: mov VP14, S3, !SP2;         nop;    nop;    nop
// CHECK: mov VP14, S3, VP2;  nop;    nop;    nop
// CHECK: mov VP14, S3, !VP2;         nop;    nop;    nop

MOV.I8 S5, S4
MOV.I8 S5, S4, SP1
MOV.I8 S5, S4, !SP1

// CHECK: mov.i8 S5, S4, SP0;     nop;    nop;    nop
// CHECK: mov.i8 S5, S4, SP1;     nop;    nop;    nop
// CHECK: mov.i8 S5, S4, !SP1;    nop;    nop;    nop

MOV.U8 S5, S4
MOV.U8 S5, S4, SP1
MOV.U8 S5, S4, !SP1

// CHECK: mov.u8 S5, S4, SP0;     nop;    nop;    nop
// CHECK: mov.u8 S5, S4, SP1;     nop;    nop;    nop
// CHECK: mov.u8 S5, S4, !SP1;    nop;    nop;    nop

MOV.I16 S5, S4
MOV.I16 S5, S4, SP1
MOV.I16 S5, S4, !SP1

// CHECK: mov.i16 S5, S4, SP0;    nop;    nop;    nop
// CHECK: mov.i16 S5, S4, SP1;    nop;    nop;    nop
// CHECK: mov.i16 S5, S4, !SP1;   nop;    nop;    nop

MOV.U16 S5, S4
MOV.U16 S5, S4, SP1
MOV.U16 S5, S4, !SP1

// CHECK: mov.u16 S5, S4, SP0;    nop;    nop;    nop
// CHECK: mov.u16 S5, S4, SP1;    nop;    nop;    nop
// CHECK: mov.u16 S5, S4, !SP1;   nop;    nop;    nop

MOV.I32 S5, S4
MOV.I32 S5, S4, SP1
MOV.I32 S5, S4, !SP1

// CHECK: mov.i32 S5, S4, SP0;    nop;    nop;    nop
// CHECK: mov.i32 S5, S4, SP1;    nop;    nop;    nop
// CHECK: mov.i32 S5, S4, !SP1;   nop;    nop;    nop

MOV.U32 S5, S4
MOV.U32 S5, S4, SP1
MOV.U32 S5, S4, !SP1

// CHECK: mov.u32 S5, S4, SP0;    nop;    nop;    nop
// CHECK: mov.u32 S5, S4, SP1;    nop;    nop;    nop
// CHECK: mov.u32 S5, S4, !SP1;   nop;    nop;    nop

MOV.F32 S5, S4
MOV.F32 S5, S4, SP1
MOV.F32 S5, S4, !SP1

// CHECK: mov.f32 S5, S4, SP0;    nop;    nop;    nop
// CHECK: mov.f32 S5, S4, SP1;    nop;    nop;    nop
// CHECK: mov.f32 S5, S4, !SP1;   nop;    nop;    nop


MOV.I8 S5, 123
MOV.I8 S5, 123, SP1
MOV.I8 S5, 123, !SP1

// CHECK: mov.i8 S5, 0x7b, SP0;   nop;    nop;    nop
// CHECK: mov.i8 S5, 0x7b, SP1;   nop;    nop;    nop
// CHECK: mov.i8 S5, 0x7b, !SP1;  nop;    nop;    nop

MOV.U8 S5, 123
MOV.U8 S5, 123, SP1
MOV.U8 S5, 123, !SP1

// CHECK: mov.u8 S5, 0x7b, SP0;   nop;    nop;    nop
// CHECK: mov.u8 S5, 0x7b, SP1;   nop;    nop;    nop
// CHECK: mov.u8 S5, 0x7b, !SP1;  nop;    nop;    nop

MOV.I16 S5, 123
MOV.I16 S5, 123, SP1
MOV.I16 S5, 123, !SP1

// CHECK: mov.i16 S5, 0x7b, SP0;  nop;    nop;    nop
// CHECK: mov.i16 S5, 0x7b, SP1;  nop;    nop;    nop
// CHECK: mov.i16 S5, 0x7b, !SP1;         nop;    nop;    nop

MOV.U16 S5, 123
MOV.U16 S5, 123, SP1
MOV.U16 S5, 123, !SP1

// CHECK: mov.u16 S5, 0x7b, SP0;  nop;    nop;    nop
// CHECK: mov.u16 S5, 0x7b, SP1;  nop;    nop;    nop
// CHECK: mov.u16 S5, 0x7b, !SP1;         nop;    nop;    nop

MOV.I32 S5, 123
MOV.I32 S5, 123, SP1
MOV.I32 S5, 123, !SP1

// CHECK: mov.i32 S5, 0x7b, SP0;  nop;    nop;    nop
// CHECK: mov.i32 S5, 0x7b, SP1;  nop;    nop;    nop
// CHECK: mov.i32 S5, 0x7b, !SP1;         nop;    nop;    nop

MOV.U32 S5, 123
MOV.U32 S5, 123, SP1
MOV.U32 S5, 123, !SP1

// CHECK: mov.u32 S5, 0x7b, SP0;  nop;    nop;    nop
// CHECK: mov.u32 S5, 0x7b, SP1;  nop;    nop;    nop
// CHECK: mov.u32 S5, 0x7b, !SP1;         nop;    nop;    nop

MOV.F32 S5, 0x3f800000
MOV.F32 S5, 0x3f800000, SP1
MOV.F32 S5, 0x3f800000, !SP1

// CHECK: mov.f32 S5, 0x3f800000, SP0;    nop;    nop;    nop
// CHECK: mov.f32 S5, 0x3f800000, SP1;    nop;    nop;    nop
// CHECK: mov.f32 S5, 0x3f800000, !SP1;   nop;    nop;    nop


MOV.U32 S5, 0
MOV.U32 S5, 1
MOV.U32 S5, 3
MOV.U32 S5, 4
MOV.U32 S5, 5
MOV.U32 S5, 6
MOV.U32 S5, 7
MOV.U32 S5, 8
MOV.U32 S5, 9
MOV.U32 S5, 10
MOV.U32 S5, 11
MOV.U32 S5, 12
MOV.U32 S5, 13
MOV.U32 S5, 14
MOV.U32 S5, 15
MOV.U32 S5, 16

// CHECK: mov.u32 S5, 0x0, SP0;   nop;    nop;    nop
// CHECK: mov.u32 S5, 0x1, SP0;   nop;    nop;    nop
// CHECK: mov.u32 S5, 0x3, SP0;   nop;    nop;    nop
// CHECK: mov.u32 S5, 0x4, SP0;   nop;    nop;    nop
// CHECK: mov.u32 S5, 0x5, SP0;   nop;    nop;    nop
// CHECK: mov.u32 S5, 0x6, SP0;   nop;    nop;    nop
// CHECK: mov.u32 S5, 0x7, SP0;   nop;    nop;    nop
// CHECK: mov.u32 S5, 0x8, SP0;   nop;    nop;    nop
// CHECK: mov.u32 S5, 0x9, SP0;   nop;    nop;    nop
// CHECK: mov.u32 S5, 0xa, SP0;   nop;    nop;    nop
// CHECK: mov.u32 S5, 0xb, SP0;   nop;    nop;    nop
// CHECK: mov.u32 S5, 0xc, SP0;   nop;    nop;    nop
// CHECK: mov.u32 S5, 0xd, SP0;   nop;    nop;    nop
// CHECK: mov.u32 S5, 0xe, SP0;   nop;    nop;    nop
// CHECK: mov.u32 S5, 0xf, SP0;   nop;    nop;    nop
// CHECK: mov.u32 S5, 0x10, SP0;  nop;    nop;    nop

MOV SP5, S3
MOV SP5, S3, SP1
MOV SP5, S3, !SP1

// CHECK: mov SP5, S3, SP0;       nop;    nop;    nop
// CHECK: mov SP5, S3, SP1;       nop;    nop;    nop
// CHECK: mov SP5, S3, !SP1;      nop;    nop;    nop

MOV.U32 SP5, S3
MOV.U32 SP5, S3, SP1
MOV.U32 SP5, S3, !SP1

// CHECK: mov SP5, S3, SP0;       nop;    nop;    nop
// CHECK: mov SP5, S3, SP1;       nop;    nop;    nop
// CHECK: mov SP5, S3, !SP1;      nop;    nop;    nop

MOV.B SP5, S3
MOV.B SP5, S3, SP1
MOV.B SP5, S3, !SP1

// CHECK: mov SP5, S3, SP0;       nop;    nop;    nop
// CHECK: mov SP5, S3, SP1;       nop;    nop;    nop
// CHECK: mov SP5, S3, !SP1;      nop;    nop;    nop

MOV SP5, 1
MOV SP5, 1, SP2
MOV SP5, 1, !SP2

// CHECK: mov SP5, 0x1, SP0;      nop;    nop;    nop
// CHECK: mov SP5, 0x1, SP2;      nop;    nop;    nop
// CHECK: mov SP5, 0x1, !SP2;     nop;    nop;    nop

MOV.U32 SP5, 1
MOV.U32 SP5, 1, SP2
MOV.U32 SP5, 1, !SP2

// CHECK: mov SP5, 0x1, SP0;      nop;    nop;    nop
// CHECK: mov SP5, 0x1, SP2;      nop;    nop;    nop
// CHECK: mov SP5, 0x1, !SP2;     nop;    nop;    nop

MOV.B SP5, 1
MOV.B SP5, 1, SP2
MOV.B SP5, 1, !SP2

// CHECK: mov SP5, 0x1, SP0;      nop;    nop;    nop
// CHECK: mov SP5, 0x1, SP2;      nop;    nop;    nop
// CHECK: mov SP5, 0x1, !SP2;     nop;    nop;    nop


MOV b00001 I5, S3
MOV b01010 I5, S3, SP1
MOV b00011 I5, S3, !SP1

// CHECK: mov b00001 I5, S3, SP0;         nop;    nop;    nop
// CHECK: mov b01010 I5, S3, SP1;         nop;    nop;    nop
// CHECK: mov b00011 I5, S3, !SP1;        nop;    nop;    nop


MOV b00001 I5, 123
MOV b01010 I5, 123, SP1
MOV b00011 I5, 123, !SP1

// CHECK: mov b00001 I5, 0x7b, SP0;       nop;    nop;    nop
// CHECK: mov b01010 I5, 0x7b, SP1;       nop;    nop;    nop
// CHECK: mov b00011 I5, 0x7b, !SP1;      nop;    nop;    nop

MOV b00001 I5, I3
MOV b01010 I5, I3, SP1
MOV b00011 I5, I3, !SP1

// CHECK: mov b00001 I5, I3, SP0;         nop;    nop;    nop
// CHECK: mov b01010 I5, I3, SP1;         nop;    nop;    nop
// CHECK: mov b00011 I5, I3, !SP1;        nop;    nop;    nop
