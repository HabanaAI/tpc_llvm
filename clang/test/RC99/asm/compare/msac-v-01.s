// RUN: %clang -cc1as -triple tpc-none-none -filetype obj %s -o %t.o
// RUN: %disasm %t.o | FileCheck %s



NOP; NOP; MSAC.I16 V4, V5, V6, V7, V8, NORMALIZE_AB;         NOP
NOP; NOP; MSAC.I16 V4, V5, V6, V7, V8, NORMALIZE_AB, SP1;    NOP
NOP; NOP; MSAC.I16 V4, V5, V6, V7, V8, NORMALIZE_AB, !SP1;   NOP
NOP; NOP; MSAC.I16 V4, V5, V6, V7, V8, NORMALIZE_AB, VP1;    NOP
NOP; NOP; MSAC.I16 V4, V5, V6, V7, V8, NORMALIZE_AB, !VP1;   NOP

// CHECK: nop; nop; msac.i16 normalize_ab V4, V5, V6, V7, V8, SP0; nop
// CHECK: nop; nop; msac.i16 normalize_ab V4, V5, V6, V7, V8, SP1; nop
// CHECK: nop; nop; msac.i16 normalize_ab V4, V5, V6, V7, V8, !SP1; nop
// CHECK: nop; nop; msac.i16 normalize_ab V4, V5, V6, V7, V8, VP1; nop
// CHECK: nop; nop; msac.i16 normalize_ab V4, V5, V6, V7, V8, !VP1; nop

NOP; NOP; MSAC.I16 V4, V5, S3, V7, V8, NORMALIZE_AB;         NOP
NOP; NOP; MSAC.I16 V4, V5, S3, V7, V8, NORMALIZE_AB, SP1;    NOP
NOP; NOP; MSAC.I16 V4, V5, S3, V7, V8, NORMALIZE_AB, !SP1;   NOP
NOP; NOP; MSAC.I16 V4, V5, S3, V7, V8, NORMALIZE_AB, VP1;    NOP
NOP; NOP; MSAC.I16 V4, V5, S3, V7, V8, NORMALIZE_AB, !VP1;   NOP

// CHECK: nop; nop; msac.i16 normalize_ab V4, V5, S3, V7, V8, SP0; nop
// CHECK: nop; nop; msac.i16 normalize_ab V4, V5, S3, V7, V8, SP1; nop
// CHECK: nop; nop; msac.i16 normalize_ab V4, V5, S3, V7, V8, !SP1; nop
// CHECK: nop; nop; msac.i16 normalize_ab V4, V5, S3, V7, V8, VP1; nop
// CHECK: nop; nop; msac.i16 normalize_ab V4, V5, S3, V7, V8, !VP1; nop

NOP; NOP; MSAC.I16 V4, V5, 122, V7, V8, NORMALIZE_AB;         NOP
NOP; NOP; MSAC.I16 V4, V5, 122, V7, V8, NORMALIZE_AB, SP1;    NOP
NOP; NOP; MSAC.I16 V4, V5, 122, V7, V8, NORMALIZE_AB, !SP1;   NOP
NOP; NOP; MSAC.I16 V4, V5, 122, V7, V8, NORMALIZE_AB, VP1;    NOP
NOP; NOP; MSAC.I16 V4, V5, 122, V7, V8, NORMALIZE_AB, !VP1;   NOP

// CHECK: nop; nop; msac.i16 normalize_ab V4, V5, 0x7a, V7, V8, SP0; nop
// CHECK: nop; nop; msac.i16 normalize_ab V4, V5, 0x7a, V7, V8, SP1; nop
// CHECK: nop; nop; msac.i16 normalize_ab V4, V5, 0x7a, V7, V8, !SP1; nop
// CHECK: nop; nop; msac.i16 normalize_ab V4, V5, 0x7a, V7, V8, VP1; nop
// CHECK: nop; nop; msac.i16 normalize_ab V4, V5, 0x7a, V7, V8, !VP1; nop

NOP; NOP; MSAC.I16 V4, V5, V6, V7, S3, NORMALIZE_AB;         NOP
NOP; NOP; MSAC.I16 V4, V5, V6, V7, S3, NORMALIZE_AB, SP1;    NOP
NOP; NOP; MSAC.I16 V4, V5, V6, V7, S3, NORMALIZE_AB, !SP1;   NOP
NOP; NOP; MSAC.I16 V4, V5, V6, V7, S3, NORMALIZE_AB, VP1;    NOP
NOP; NOP; MSAC.I16 V4, V5, V6, V7, S3, NORMALIZE_AB, !VP1;   NOP

// CHECK: nop; nop; msac.i16 normalize_ab V4, V5, V6, V7, S3, SP0; nop
// CHECK: nop; nop; msac.i16 normalize_ab V4, V5, V6, V7, S3, SP1; nop
// CHECK: nop; nop; msac.i16 normalize_ab V4, V5, V6, V7, S3, !SP1; nop
// CHECK: nop; nop; msac.i16 normalize_ab V4, V5, V6, V7, S3, VP1; nop
// CHECK: nop; nop; msac.i16 normalize_ab V4, V5, V6, V7, S3, !VP1; nop

NOP; NOP; MSAC.I16 V4, V5, V6, V7, 123, NORMALIZE_AB;         NOP
NOP; NOP; MSAC.I16 V4, V5, V6, V7, 123, NORMALIZE_AB, SP1;    NOP
NOP; NOP; MSAC.I16 V4, V5, V6, V7, 123, NORMALIZE_AB, !SP1;   NOP
NOP; NOP; MSAC.I16 V4, V5, V6, V7, 123, NORMALIZE_AB, VP1;    NOP
NOP; NOP; MSAC.I16 V4, V5, V6, V7, 123, NORMALIZE_AB, !VP1;   NOP

// CHECK: nop; nop; msac.i16 normalize_ab V4, V5, V6, V7, 0x7b, SP0; nop
// CHECK: nop; nop; msac.i16 normalize_ab V4, V5, V6, V7, 0x7b, SP1; nop
// CHECK: nop; nop; msac.i16 normalize_ab V4, V5, V6, V7, 0x7b, !SP1; nop
// CHECK: nop; nop; msac.i16 normalize_ab V4, V5, V6, V7, 0x7b, VP1; nop
// CHECK: nop; nop; msac.i16 normalize_ab V4, V5, V6, V7, 0x7b, !VP1; nop
                                                       
NOP; NOP; MSAC.I8 V4, V5, V6, V7, V8, NORMALIZE_AB;         NOP
NOP; NOP; MSAC.I8 V4, V5, V6, V7, V8, NORMALIZE_AB, SP1;    NOP
NOP; NOP; MSAC.I8 V4, V5, V6, V7, V8, NORMALIZE_AB, !SP1;   NOP
NOP; NOP; MSAC.I8 V4, V5, V6, V7, V8, NORMALIZE_AB, VP1;    NOP
NOP; NOP; MSAC.I8 V4, V5, V6, V7, V8, NORMALIZE_AB, !VP1;   NOP

// CHECK: nop; nop; msac.i8 normalize_ab V4, V5, V6, V7, V8, SP0; nop
// CHECK: nop; nop; msac.i8 normalize_ab V4, V5, V6, V7, V8, SP1; nop
// CHECK: nop; nop; msac.i8 normalize_ab V4, V5, V6, V7, V8, !SP1; nop
// CHECK: nop; nop; msac.i8 normalize_ab V4, V5, V6, V7, V8, VP1; nop
// CHECK: nop; nop; msac.i8 normalize_ab V4, V5, V6, V7, V8, !VP1; nop

NOP; NOP; MSAC.I8 V4, V5, S3, V7, V8, NORMALIZE_AB;         NOP
NOP; NOP; MSAC.I8 V4, V5, S3, V7, V8, NORMALIZE_AB, SP1;    NOP
NOP; NOP; MSAC.I8 V4, V5, S3, V7, V8, NORMALIZE_AB, !SP1;   NOP
NOP; NOP; MSAC.I8 V4, V5, S3, V7, V8, NORMALIZE_AB, VP1;    NOP
NOP; NOP; MSAC.I8 V4, V5, S3, V7, V8, NORMALIZE_AB, !VP1;   NOP

// CHECK: nop; nop; msac.i8 normalize_ab V4, V5, S3, V7, V8, SP0; nop
// CHECK: nop; nop; msac.i8 normalize_ab V4, V5, S3, V7, V8, SP1; nop
// CHECK: nop; nop; msac.i8 normalize_ab V4, V5, S3, V7, V8, !SP1; nop
// CHECK: nop; nop; msac.i8 normalize_ab V4, V5, S3, V7, V8, VP1; nop
// CHECK: nop; nop; msac.i8 normalize_ab V4, V5, S3, V7, V8, !VP1; nop

NOP; NOP; MSAC.I8 V4, V5, 122, V7, V8, NORMALIZE_AB;         NOP
NOP; NOP; MSAC.I8 V4, V5, 122, V7, V8, NORMALIZE_AB, SP1;    NOP
NOP; NOP; MSAC.I8 V4, V5, 122, V7, V8, NORMALIZE_AB, !SP1;   NOP
NOP; NOP; MSAC.I8 V4, V5, 122, V7, V8, NORMALIZE_AB, VP1;    NOP
NOP; NOP; MSAC.I8 V4, V5, 122, V7, V8, NORMALIZE_AB, !VP1;   NOP

// CHECK: nop; nop; msac.i8 normalize_ab V4, V5, 0x7a, V7, V8, SP0; nop
// CHECK: nop; nop; msac.i8 normalize_ab V4, V5, 0x7a, V7, V8, SP1; nop
// CHECK: nop; nop; msac.i8 normalize_ab V4, V5, 0x7a, V7, V8, !SP1; nop
// CHECK: nop; nop; msac.i8 normalize_ab V4, V5, 0x7a, V7, V8, VP1; nop
// CHECK: nop; nop; msac.i8 normalize_ab V4, V5, 0x7a, V7, V8, !VP1; nop

NOP; NOP; MSAC.I8 V4, V5, V6, V7, S3, NORMALIZE_AB;         NOP
NOP; NOP; MSAC.I8 V4, V5, V6, V7, S3, NORMALIZE_AB, SP1;    NOP
NOP; NOP; MSAC.I8 V4, V5, V6, V7, S3, NORMALIZE_AB, !SP1;   NOP
NOP; NOP; MSAC.I8 V4, V5, V6, V7, S3, NORMALIZE_AB, VP1;    NOP
NOP; NOP; MSAC.I8 V4, V5, V6, V7, S3, NORMALIZE_AB, !VP1;   NOP

// CHECK: nop; nop; msac.i8 normalize_ab V4, V5, V6, V7, S3, SP0; nop
// CHECK: nop; nop; msac.i8 normalize_ab V4, V5, V6, V7, S3, SP1; nop
// CHECK: nop; nop; msac.i8 normalize_ab V4, V5, V6, V7, S3, !SP1; nop
// CHECK: nop; nop; msac.i8 normalize_ab V4, V5, V6, V7, S3, VP1; nop
// CHECK: nop; nop; msac.i8 normalize_ab V4, V5, V6, V7, S3, !VP1; nop

NOP; NOP; MSAC.I8 V4, V5, V6, V7, 123, NORMALIZE_AB;         NOP
NOP; NOP; MSAC.I8 V4, V5, V6, V7, 123, NORMALIZE_AB, SP1;    NOP
NOP; NOP; MSAC.I8 V4, V5, V6, V7, 123, NORMALIZE_AB, !SP1;   NOP
NOP; NOP; MSAC.I8 V4, V5, V6, V7, 123, NORMALIZE_AB, VP1;    NOP
NOP; NOP; MSAC.I8 V4, V5, V6, V7, 123, NORMALIZE_AB, !VP1;   NOP

// CHECK: nop; nop; msac.i8 normalize_ab V4, V5, V6, V7, 0x7b, SP0; nop
// CHECK: nop; nop; msac.i8 normalize_ab V4, V5, V6, V7, 0x7b, SP1; nop
// CHECK: nop; nop; msac.i8 normalize_ab V4, V5, V6, V7, 0x7b, !SP1; nop
// CHECK: nop; nop; msac.i8 normalize_ab V4, V5, V6, V7, 0x7b, VP1; nop
// CHECK: nop; nop; msac.i8 normalize_ab V4, V5, V6, V7, 0x7b, !VP1; nop

NOP; NOP; MSAC.U16 V4, V5, V6, V7, V8, NORMALIZE_AB;         NOP
NOP; NOP; MSAC.U16 V4, V5, V6, V7, V8, NORMALIZE_AB, SP1;    NOP
NOP; NOP; MSAC.U16 V4, V5, V6, V7, V8, NORMALIZE_AB, !SP1;   NOP
NOP; NOP; MSAC.U16 V4, V5, V6, V7, V8, NORMALIZE_AB, VP1;    NOP
NOP; NOP; MSAC.U16 V4, V5, V6, V7, V8, NORMALIZE_AB, !VP1;   NOP

// CHECK: nop; nop; msac.u16 normalize_ab V4, V5, V6, V7, V8, SP0; nop
// CHECK: nop; nop; msac.u16 normalize_ab V4, V5, V6, V7, V8, SP1; nop
// CHECK: nop; nop; msac.u16 normalize_ab V4, V5, V6, V7, V8, !SP1; nop
// CHECK: nop; nop; msac.u16 normalize_ab V4, V5, V6, V7, V8, VP1; nop
// CHECK: nop; nop; msac.u16 normalize_ab V4, V5, V6, V7, V8, !VP1; nop

NOP; NOP; MSAC.U16 V4, V5, S3, V7, V8, NORMALIZE_AB;         NOP
NOP; NOP; MSAC.U16 V4, V5, S3, V7, V8, NORMALIZE_AB, SP1;    NOP
NOP; NOP; MSAC.U16 V4, V5, S3, V7, V8, NORMALIZE_AB, !SP1;   NOP
NOP; NOP; MSAC.U16 V4, V5, S3, V7, V8, NORMALIZE_AB, VP1;    NOP
NOP; NOP; MSAC.U16 V4, V5, S3, V7, V8, NORMALIZE_AB, !VP1;   NOP

// CHECK: nop; nop; msac.u16 normalize_ab V4, V5, S3, V7, V8, SP0; nop
// CHECK: nop; nop; msac.u16 normalize_ab V4, V5, S3, V7, V8, SP1; nop
// CHECK: nop; nop; msac.u16 normalize_ab V4, V5, S3, V7, V8, !SP1; nop
// CHECK: nop; nop; msac.u16 normalize_ab V4, V5, S3, V7, V8, VP1; nop
// CHECK: nop; nop; msac.u16 normalize_ab V4, V5, S3, V7, V8, !VP1; nop

NOP; NOP; MSAC.U16 V4, V5, 122, V7, V8, NORMALIZE_AB;         NOP
NOP; NOP; MSAC.U16 V4, V5, 122, V7, V8, NORMALIZE_AB, SP1;    NOP
NOP; NOP; MSAC.U16 V4, V5, 122, V7, V8, NORMALIZE_AB, !SP1;   NOP
NOP; NOP; MSAC.U16 V4, V5, 122, V7, V8, NORMALIZE_AB, VP1;    NOP
NOP; NOP; MSAC.U16 V4, V5, 122, V7, V8, NORMALIZE_AB, !VP1;   NOP

// CHECK: nop; nop; msac.u16 normalize_ab V4, V5, 0x7a, V7, V8, SP0; nop
// CHECK: nop; nop; msac.u16 normalize_ab V4, V5, 0x7a, V7, V8, SP1; nop
// CHECK: nop; nop; msac.u16 normalize_ab V4, V5, 0x7a, V7, V8, !SP1; nop
// CHECK: nop; nop; msac.u16 normalize_ab V4, V5, 0x7a, V7, V8, VP1; nop
// CHECK: nop; nop; msac.u16 normalize_ab V4, V5, 0x7a, V7, V8, !VP1; nop

NOP; NOP; MSAC.U16 V4, V5, V6, V7, S3, NORMALIZE_AB;         NOP
NOP; NOP; MSAC.U16 V4, V5, V6, V7, S3, NORMALIZE_AB, SP1;    NOP
NOP; NOP; MSAC.U16 V4, V5, V6, V7, S3, NORMALIZE_AB, !SP1;   NOP
NOP; NOP; MSAC.U16 V4, V5, V6, V7, S3, NORMALIZE_AB, VP1;    NOP
NOP; NOP; MSAC.U16 V4, V5, V6, V7, S3, NORMALIZE_AB, !VP1;   NOP

// CHECK: nop; nop; msac.u16 normalize_ab V4, V5, V6, V7, S3, SP0; nop
// CHECK: nop; nop; msac.u16 normalize_ab V4, V5, V6, V7, S3, SP1; nop
// CHECK: nop; nop; msac.u16 normalize_ab V4, V5, V6, V7, S3, !SP1; nop
// CHECK: nop; nop; msac.u16 normalize_ab V4, V5, V6, V7, S3, VP1; nop
// CHECK: nop; nop; msac.u16 normalize_ab V4, V5, V6, V7, S3, !VP1; nop

NOP; NOP; MSAC.U16 V4, V5, V6, V7, 123, NORMALIZE_AB;         NOP
NOP; NOP; MSAC.U16 V4, V5, V6, V7, 123, NORMALIZE_AB, SP1;    NOP
NOP; NOP; MSAC.U16 V4, V5, V6, V7, 123, NORMALIZE_AB, !SP1;   NOP
NOP; NOP; MSAC.U16 V4, V5, V6, V7, 123, NORMALIZE_AB, VP1;    NOP
NOP; NOP; MSAC.U16 V4, V5, V6, V7, 123, NORMALIZE_AB, !VP1;   NOP

// CHECK: nop; nop; msac.u16 normalize_ab V4, V5, V6, V7, 0x7b, SP0; nop
// CHECK: nop; nop; msac.u16 normalize_ab V4, V5, V6, V7, 0x7b, SP1; nop
// CHECK: nop; nop; msac.u16 normalize_ab V4, V5, V6, V7, 0x7b, !SP1; nop
// CHECK: nop; nop; msac.u16 normalize_ab V4, V5, V6, V7, 0x7b, VP1; nop
// CHECK: nop; nop; msac.u16 normalize_ab V4, V5, V6, V7, 0x7b, !VP1; nop

NOP; NOP; MSAC.U8 V4, V5, V6, V7, V8, NORMALIZE_AB;         NOP
NOP; NOP; MSAC.U8 V4, V5, V6, V7, V8, NORMALIZE_AB, SP1;    NOP
NOP; NOP; MSAC.U8 V4, V5, V6, V7, V8, NORMALIZE_AB, !SP1;   NOP
NOP; NOP; MSAC.U8 V4, V5, V6, V7, V8, NORMALIZE_AB, VP1;    NOP
NOP; NOP; MSAC.U8 V4, V5, V6, V7, V8, NORMALIZE_AB, !VP1;   NOP

// CHECK: nop; nop; msac.u8 normalize_ab V4, V5, V6, V7, V8, SP0; nop
// CHECK: nop; nop; msac.u8 normalize_ab V4, V5, V6, V7, V8, SP1; nop
// CHECK: nop; nop; msac.u8 normalize_ab V4, V5, V6, V7, V8, !SP1; nop
// CHECK: nop; nop; msac.u8 normalize_ab V4, V5, V6, V7, V8, VP1; nop
// CHECK: nop; nop; msac.u8 normalize_ab V4, V5, V6, V7, V8, !VP1; nop

NOP; NOP; MSAC.U8 V4, V5, S3, V7, V8, NORMALIZE_AB;         NOP
NOP; NOP; MSAC.U8 V4, V5, S3, V7, V8, NORMALIZE_AB, SP1;    NOP
NOP; NOP; MSAC.U8 V4, V5, S3, V7, V8, NORMALIZE_AB, !SP1;   NOP
NOP; NOP; MSAC.U8 V4, V5, S3, V7, V8, NORMALIZE_AB, VP1;    NOP
NOP; NOP; MSAC.U8 V4, V5, S3, V7, V8, NORMALIZE_AB, !VP1;   NOP

// CHECK: nop; nop; msac.u8 normalize_ab V4, V5, S3, V7, V8, SP0; nop
// CHECK: nop; nop; msac.u8 normalize_ab V4, V5, S3, V7, V8, SP1; nop
// CHECK: nop; nop; msac.u8 normalize_ab V4, V5, S3, V7, V8, !SP1; nop
// CHECK: nop; nop; msac.u8 normalize_ab V4, V5, S3, V7, V8, VP1; nop
// CHECK: nop; nop; msac.u8 normalize_ab V4, V5, S3, V7, V8, !VP1; nop

NOP; NOP; MSAC.U8 V4, V5, 122, V7, V8, NORMALIZE_AB;         NOP
NOP; NOP; MSAC.U8 V4, V5, 122, V7, V8, NORMALIZE_AB, SP1;    NOP
NOP; NOP; MSAC.U8 V4, V5, 122, V7, V8, NORMALIZE_AB, !SP1;   NOP
NOP; NOP; MSAC.U8 V4, V5, 122, V7, V8, NORMALIZE_AB, VP1;    NOP
NOP; NOP; MSAC.U8 V4, V5, 122, V7, V8, NORMALIZE_AB, !VP1;   NOP

// CHECK: nop; nop; msac.u8 normalize_ab V4, V5, 0x7a, V7, V8, SP0; nop
// CHECK: nop; nop; msac.u8 normalize_ab V4, V5, 0x7a, V7, V8, SP1; nop
// CHECK: nop; nop; msac.u8 normalize_ab V4, V5, 0x7a, V7, V8, !SP1; nop
// CHECK: nop; nop; msac.u8 normalize_ab V4, V5, 0x7a, V7, V8, VP1; nop
// CHECK: nop; nop; msac.u8 normalize_ab V4, V5, 0x7a, V7, V8, !VP1; nop

NOP; NOP; MSAC.U8 V4, V5, V6, V7, S3, NORMALIZE_AB;         NOP
NOP; NOP; MSAC.U8 V4, V5, V6, V7, S3, NORMALIZE_AB, SP1;    NOP
NOP; NOP; MSAC.U8 V4, V5, V6, V7, S3, NORMALIZE_AB, !SP1;   NOP
NOP; NOP; MSAC.U8 V4, V5, V6, V7, S3, NORMALIZE_AB, VP1;    NOP
NOP; NOP; MSAC.U8 V4, V5, V6, V7, S3, NORMALIZE_AB, !VP1;   NOP

// CHECK: nop; nop; msac.u8 normalize_ab V4, V5, V6, V7, S3, SP0; nop
// CHECK: nop; nop; msac.u8 normalize_ab V4, V5, V6, V7, S3, SP1; nop
// CHECK: nop; nop; msac.u8 normalize_ab V4, V5, V6, V7, S3, !SP1; nop
// CHECK: nop; nop; msac.u8 normalize_ab V4, V5, V6, V7, S3, VP1; nop
// CHECK: nop; nop; msac.u8 normalize_ab V4, V5, V6, V7, S3, !VP1; nop

NOP; NOP; MSAC.U8 V4, V5, V6, V7, 123, NORMALIZE_AB;         NOP
NOP; NOP; MSAC.U8 V4, V5, V6, V7, 123, NORMALIZE_AB, SP1;    NOP
NOP; NOP; MSAC.U8 V4, V5, V6, V7, 123, NORMALIZE_AB, !SP1;   NOP
NOP; NOP; MSAC.U8 V4, V5, V6, V7, 123, NORMALIZE_AB, VP1;    NOP
NOP; NOP; MSAC.U8 V4, V5, V6, V7, 123, NORMALIZE_AB, !VP1;   NOP

// CHECK: nop; nop; msac.u8 normalize_ab V4, V5, V6, V7, 0x7b, SP0; nop
// CHECK: nop; nop; msac.u8 normalize_ab V4, V5, V6, V7, 0x7b, SP1; nop
// CHECK: nop; nop; msac.u8 normalize_ab V4, V5, V6, V7, 0x7b, !SP1; nop
// CHECK: nop; nop; msac.u8 normalize_ab V4, V5, V6, V7, 0x7b, VP1; nop
// CHECK: nop; nop; msac.u8 normalize_ab V4, V5, V6, V7, 0x7b, !VP1; nop

NOP; NOP; MSAC.I16 V4, V5, V6, V7, V8, NORMALIZE_C;         NOP
NOP; NOP; MSAC.I16 V4, V5, V6, V7, V8, NORMALIZE_C, SP1;    NOP
NOP; NOP; MSAC.I16 V4, V5, V6, V7, V8, NORMALIZE_C, !SP1;   NOP
NOP; NOP; MSAC.I16 V4, V5, V6, V7, V8, NORMALIZE_C, VP1;    NOP
NOP; NOP; MSAC.I16 V4, V5, V6, V7, V8, NORMALIZE_C, !VP1;   NOP

// CHECK: nop; nop; msac.i16 normalize_c V4, V5, V6, V7, V8, SP0; nop
// CHECK: nop; nop; msac.i16 normalize_c V4, V5, V6, V7, V8, SP1; nop
// CHECK: nop; nop; msac.i16 normalize_c V4, V5, V6, V7, V8, !SP1; nop
// CHECK: nop; nop; msac.i16 normalize_c V4, V5, V6, V7, V8, VP1; nop
// CHECK: nop; nop; msac.i16 normalize_c V4, V5, V6, V7, V8, !VP1; nop

NOP; NOP; MSAC.I16 V4, V5, S3, V7, V8, NORMALIZE_C;         NOP
NOP; NOP; MSAC.I16 V4, V5, S3, V7, V8, NORMALIZE_C, SP1;    NOP
NOP; NOP; MSAC.I16 V4, V5, S3, V7, V8, NORMALIZE_C, !SP1;   NOP
NOP; NOP; MSAC.I16 V4, V5, S3, V7, V8, NORMALIZE_C, VP1;    NOP
NOP; NOP; MSAC.I16 V4, V5, S3, V7, V8, NORMALIZE_C, !VP1;   NOP

// CHECK: nop; nop; msac.i16 normalize_c V4, V5, S3, V7, V8, SP0; nop
// CHECK: nop; nop; msac.i16 normalize_c V4, V5, S3, V7, V8, SP1; nop
// CHECK: nop; nop; msac.i16 normalize_c V4, V5, S3, V7, V8, !SP1; nop
// CHECK: nop; nop; msac.i16 normalize_c V4, V5, S3, V7, V8, VP1; nop
// CHECK: nop; nop; msac.i16 normalize_c V4, V5, S3, V7, V8, !VP1; nop

NOP; NOP; MSAC.I16 V4, V5, 122, V7, V8, NORMALIZE_C;         NOP
NOP; NOP; MSAC.I16 V4, V5, 122, V7, V8, NORMALIZE_C, SP1;    NOP
NOP; NOP; MSAC.I16 V4, V5, 122, V7, V8, NORMALIZE_C, !SP1;   NOP
NOP; NOP; MSAC.I16 V4, V5, 122, V7, V8, NORMALIZE_C, VP1;    NOP
NOP; NOP; MSAC.I16 V4, V5, 122, V7, V8, NORMALIZE_C, !VP1;   NOP

// CHECK: nop; nop; msac.i16 normalize_c V4, V5, 0x7a, V7, V8, SP0; nop
// CHECK: nop; nop; msac.i16 normalize_c V4, V5, 0x7a, V7, V8, SP1; nop
// CHECK: nop; nop; msac.i16 normalize_c V4, V5, 0x7a, V7, V8, !SP1; nop
// CHECK: nop; nop; msac.i16 normalize_c V4, V5, 0x7a, V7, V8, VP1; nop
// CHECK: nop; nop; msac.i16 normalize_c V4, V5, 0x7a, V7, V8, !VP1; nop

NOP; NOP; MSAC.I16 V4, V5, V6, V7, S3, NORMALIZE_C;         NOP
NOP; NOP; MSAC.I16 V4, V5, V6, V7, S3, NORMALIZE_C, SP1;    NOP
NOP; NOP; MSAC.I16 V4, V5, V6, V7, S3, NORMALIZE_C, !SP1;   NOP
NOP; NOP; MSAC.I16 V4, V5, V6, V7, S3, NORMALIZE_C, VP1;    NOP
NOP; NOP; MSAC.I16 V4, V5, V6, V7, S3, NORMALIZE_C, !VP1;   NOP

// CHECK: nop; nop; msac.i16 normalize_c V4, V5, V6, V7, S3, SP0; nop
// CHECK: nop; nop; msac.i16 normalize_c V4, V5, V6, V7, S3, SP1; nop
// CHECK: nop; nop; msac.i16 normalize_c V4, V5, V6, V7, S3, !SP1; nop
// CHECK: nop; nop; msac.i16 normalize_c V4, V5, V6, V7, S3, VP1; nop
// CHECK: nop; nop; msac.i16 normalize_c V4, V5, V6, V7, S3, !VP1; nop

NOP; NOP; MSAC.I16 V4, V5, V6, V7, 123, NORMALIZE_C;         NOP
NOP; NOP; MSAC.I16 V4, V5, V6, V7, 123, NORMALIZE_C, SP1;    NOP
NOP; NOP; MSAC.I16 V4, V5, V6, V7, 123, NORMALIZE_C, !SP1;   NOP
NOP; NOP; MSAC.I16 V4, V5, V6, V7, 123, NORMALIZE_C, VP1;    NOP
NOP; NOP; MSAC.I16 V4, V5, V6, V7, 123, NORMALIZE_C, !VP1;   NOP

// CHECK: nop; nop; msac.i16 normalize_c V4, V5, V6, V7, 0x7b, SP0; nop
// CHECK: nop; nop; msac.i16 normalize_c V4, V5, V6, V7, 0x7b, SP1; nop
// CHECK: nop; nop; msac.i16 normalize_c V4, V5, V6, V7, 0x7b, !SP1; nop
// CHECK: nop; nop; msac.i16 normalize_c V4, V5, V6, V7, 0x7b, VP1; nop
// CHECK: nop; nop; msac.i16 normalize_c V4, V5, V6, V7, 0x7b, !VP1; nop
                                                       
NOP; NOP; MSAC.I8 V4, V5, V6, V7, V8, NORMALIZE_C;         NOP
NOP; NOP; MSAC.I8 V4, V5, V6, V7, V8, NORMALIZE_C, SP1;    NOP
NOP; NOP; MSAC.I8 V4, V5, V6, V7, V8, NORMALIZE_C, !SP1;   NOP
NOP; NOP; MSAC.I8 V4, V5, V6, V7, V8, NORMALIZE_C, VP1;    NOP
NOP; NOP; MSAC.I8 V4, V5, V6, V7, V8, NORMALIZE_C, !VP1;   NOP

// CHECK: nop; nop; msac.i8 normalize_c V4, V5, V6, V7, V8, SP0; nop
// CHECK: nop; nop; msac.i8 normalize_c V4, V5, V6, V7, V8, SP1; nop
// CHECK: nop; nop; msac.i8 normalize_c V4, V5, V6, V7, V8, !SP1; nop
// CHECK: nop; nop; msac.i8 normalize_c V4, V5, V6, V7, V8, VP1; nop
// CHECK: nop; nop; msac.i8 normalize_c V4, V5, V6, V7, V8, !VP1; nop

NOP; NOP; MSAC.I8 V4, V5, S3, V7, V8, NORMALIZE_C;         NOP
NOP; NOP; MSAC.I8 V4, V5, S3, V7, V8, NORMALIZE_C, SP1;    NOP
NOP; NOP; MSAC.I8 V4, V5, S3, V7, V8, NORMALIZE_C, !SP1;   NOP
NOP; NOP; MSAC.I8 V4, V5, S3, V7, V8, NORMALIZE_C, VP1;    NOP
NOP; NOP; MSAC.I8 V4, V5, S3, V7, V8, NORMALIZE_C, !VP1;   NOP

// CHECK: nop; nop; msac.i8 normalize_c V4, V5, S3, V7, V8, SP0; nop
// CHECK: nop; nop; msac.i8 normalize_c V4, V5, S3, V7, V8, SP1; nop
// CHECK: nop; nop; msac.i8 normalize_c V4, V5, S3, V7, V8, !SP1; nop
// CHECK: nop; nop; msac.i8 normalize_c V4, V5, S3, V7, V8, VP1; nop
// CHECK: nop; nop; msac.i8 normalize_c V4, V5, S3, V7, V8, !VP1; nop

NOP; NOP; MSAC.I8 V4, V5, 122, V7, V8, NORMALIZE_C;         NOP
NOP; NOP; MSAC.I8 V4, V5, 122, V7, V8, NORMALIZE_C, SP1;    NOP
NOP; NOP; MSAC.I8 V4, V5, 122, V7, V8, NORMALIZE_C, !SP1;   NOP
NOP; NOP; MSAC.I8 V4, V5, 122, V7, V8, NORMALIZE_C, VP1;    NOP
NOP; NOP; MSAC.I8 V4, V5, 122, V7, V8, NORMALIZE_C, !VP1;   NOP

// CHECK: nop; nop; msac.i8 normalize_c V4, V5, 0x7a, V7, V8, SP0; nop
// CHECK: nop; nop; msac.i8 normalize_c V4, V5, 0x7a, V7, V8, SP1; nop
// CHECK: nop; nop; msac.i8 normalize_c V4, V5, 0x7a, V7, V8, !SP1; nop
// CHECK: nop; nop; msac.i8 normalize_c V4, V5, 0x7a, V7, V8, VP1; nop
// CHECK: nop; nop; msac.i8 normalize_c V4, V5, 0x7a, V7, V8, !VP1; nop

NOP; NOP; MSAC.I8 V4, V5, V6, V7, S3, NORMALIZE_C;         NOP
NOP; NOP; MSAC.I8 V4, V5, V6, V7, S3, NORMALIZE_C, SP1;    NOP
NOP; NOP; MSAC.I8 V4, V5, V6, V7, S3, NORMALIZE_C, !SP1;   NOP
NOP; NOP; MSAC.I8 V4, V5, V6, V7, S3, NORMALIZE_C, VP1;    NOP
NOP; NOP; MSAC.I8 V4, V5, V6, V7, S3, NORMALIZE_C, !VP1;   NOP

// CHECK: nop; nop; msac.i8 normalize_c V4, V5, V6, V7, S3, SP0; nop
// CHECK: nop; nop; msac.i8 normalize_c V4, V5, V6, V7, S3, SP1; nop
// CHECK: nop; nop; msac.i8 normalize_c V4, V5, V6, V7, S3, !SP1; nop
// CHECK: nop; nop; msac.i8 normalize_c V4, V5, V6, V7, S3, VP1; nop
// CHECK: nop; nop; msac.i8 normalize_c V4, V5, V6, V7, S3, !VP1; nop

NOP; NOP; MSAC.I8 V4, V5, V6, V7, 123, NORMALIZE_C;         NOP
NOP; NOP; MSAC.I8 V4, V5, V6, V7, 123, NORMALIZE_C, SP1;    NOP
NOP; NOP; MSAC.I8 V4, V5, V6, V7, 123, NORMALIZE_C, !SP1;   NOP
NOP; NOP; MSAC.I8 V4, V5, V6, V7, 123, NORMALIZE_C, VP1;    NOP
NOP; NOP; MSAC.I8 V4, V5, V6, V7, 123, NORMALIZE_C, !VP1;   NOP

// CHECK: nop; nop; msac.i8 normalize_c V4, V5, V6, V7, 0x7b, SP0; nop
// CHECK: nop; nop; msac.i8 normalize_c V4, V5, V6, V7, 0x7b, SP1; nop
// CHECK: nop; nop; msac.i8 normalize_c V4, V5, V6, V7, 0x7b, !SP1; nop
// CHECK: nop; nop; msac.i8 normalize_c V4, V5, V6, V7, 0x7b, VP1; nop
// CHECK: nop; nop; msac.i8 normalize_c V4, V5, V6, V7, 0x7b, !VP1; nop


NOP; NOP; MSAC.U16 V4, V5, V6, V7, V8, NORMALIZE_C;         NOP
NOP; NOP; MSAC.U16 V4, V5, V6, V7, V8, NORMALIZE_C, SP1;    NOP
NOP; NOP; MSAC.U16 V4, V5, V6, V7, V8, NORMALIZE_C, !SP1;   NOP
NOP; NOP; MSAC.U16 V4, V5, V6, V7, V8, NORMALIZE_C, VP1;    NOP
NOP; NOP; MSAC.U16 V4, V5, V6, V7, V8, NORMALIZE_C, !VP1;   NOP

// CHECK: nop; nop; msac.u16 normalize_c V4, V5, V6, V7, V8, SP0; nop
// CHECK: nop; nop; msac.u16 normalize_c V4, V5, V6, V7, V8, SP1; nop
// CHECK: nop; nop; msac.u16 normalize_c V4, V5, V6, V7, V8, !SP1; nop
// CHECK: nop; nop; msac.u16 normalize_c V4, V5, V6, V7, V8, VP1; nop
// CHECK: nop; nop; msac.u16 normalize_c V4, V5, V6, V7, V8, !VP1; nop

NOP; NOP; MSAC.U16 V4, V5, S3, V7, V8, NORMALIZE_C;         NOP
NOP; NOP; MSAC.U16 V4, V5, S3, V7, V8, NORMALIZE_C, SP1;    NOP
NOP; NOP; MSAC.U16 V4, V5, S3, V7, V8, NORMALIZE_C, !SP1;   NOP
NOP; NOP; MSAC.U16 V4, V5, S3, V7, V8, NORMALIZE_C, VP1;    NOP
NOP; NOP; MSAC.U16 V4, V5, S3, V7, V8, NORMALIZE_C, !VP1;   NOP

// CHECK: nop; nop; msac.u16 normalize_c V4, V5, S3, V7, V8, SP0; nop
// CHECK: nop; nop; msac.u16 normalize_c V4, V5, S3, V7, V8, SP1; nop
// CHECK: nop; nop; msac.u16 normalize_c V4, V5, S3, V7, V8, !SP1; nop
// CHECK: nop; nop; msac.u16 normalize_c V4, V5, S3, V7, V8, VP1; nop
// CHECK: nop; nop; msac.u16 normalize_c V4, V5, S3, V7, V8, !VP1; nop

NOP; NOP; MSAC.U16 V4, V5, 122, V7, V8, NORMALIZE_C;         NOP
NOP; NOP; MSAC.U16 V4, V5, 122, V7, V8, NORMALIZE_C, SP1;    NOP
NOP; NOP; MSAC.U16 V4, V5, 122, V7, V8, NORMALIZE_C, !SP1;   NOP
NOP; NOP; MSAC.U16 V4, V5, 122, V7, V8, NORMALIZE_C, VP1;    NOP
NOP; NOP; MSAC.U16 V4, V5, 122, V7, V8, NORMALIZE_C, !VP1;   NOP

// CHECK: nop; nop; msac.u16 normalize_c V4, V5, 0x7a, V7, V8, SP0; nop
// CHECK: nop; nop; msac.u16 normalize_c V4, V5, 0x7a, V7, V8, SP1; nop
// CHECK: nop; nop; msac.u16 normalize_c V4, V5, 0x7a, V7, V8, !SP1; nop
// CHECK: nop; nop; msac.u16 normalize_c V4, V5, 0x7a, V7, V8, VP1; nop
// CHECK: nop; nop; msac.u16 normalize_c V4, V5, 0x7a, V7, V8, !VP1; nop

NOP; NOP; MSAC.U16 V4, V5, V6, V7, S3, NORMALIZE_C;         NOP
NOP; NOP; MSAC.U16 V4, V5, V6, V7, S3, NORMALIZE_C, SP1;    NOP
NOP; NOP; MSAC.U16 V4, V5, V6, V7, S3, NORMALIZE_C, !SP1;   NOP
NOP; NOP; MSAC.U16 V4, V5, V6, V7, S3, NORMALIZE_C, VP1;    NOP
NOP; NOP; MSAC.U16 V4, V5, V6, V7, S3, NORMALIZE_C, !VP1;   NOP

// CHECK: nop; nop; msac.u16 normalize_c V4, V5, V6, V7, S3, SP0; nop
// CHECK: nop; nop; msac.u16 normalize_c V4, V5, V6, V7, S3, SP1; nop
// CHECK: nop; nop; msac.u16 normalize_c V4, V5, V6, V7, S3, !SP1; nop
// CHECK: nop; nop; msac.u16 normalize_c V4, V5, V6, V7, S3, VP1; nop
// CHECK: nop; nop; msac.u16 normalize_c V4, V5, V6, V7, S3, !VP1; nop

NOP; NOP; MSAC.U16 V4, V5, V6, V7, 123, NORMALIZE_C;         NOP
NOP; NOP; MSAC.U16 V4, V5, V6, V7, 123, NORMALIZE_C, SP1;    NOP
NOP; NOP; MSAC.U16 V4, V5, V6, V7, 123, NORMALIZE_C, !SP1;   NOP
NOP; NOP; MSAC.U16 V4, V5, V6, V7, 123, NORMALIZE_C, VP1;    NOP
NOP; NOP; MSAC.U16 V4, V5, V6, V7, 123, NORMALIZE_C, !VP1;   NOP

// CHECK: nop; nop; msac.u16 normalize_c V4, V5, V6, V7, 0x7b, SP0; nop
// CHECK: nop; nop; msac.u16 normalize_c V4, V5, V6, V7, 0x7b, SP1; nop
// CHECK: nop; nop; msac.u16 normalize_c V4, V5, V6, V7, 0x7b, !SP1; nop
// CHECK: nop; nop; msac.u16 normalize_c V4, V5, V6, V7, 0x7b, VP1; nop
// CHECK: nop; nop; msac.u16 normalize_c V4, V5, V6, V7, 0x7b, !VP1; nop

NOP; NOP; MSAC.U8 V4, V5, V6, V7, V8, NORMALIZE_C;         NOP
NOP; NOP; MSAC.U8 V4, V5, V6, V7, V8, NORMALIZE_C, SP1;    NOP
NOP; NOP; MSAC.U8 V4, V5, V6, V7, V8, NORMALIZE_C, !SP1;   NOP
NOP; NOP; MSAC.U8 V4, V5, V6, V7, V8, NORMALIZE_C, VP1;    NOP
NOP; NOP; MSAC.U8 V4, V5, V6, V7, V8, NORMALIZE_C, !VP1;   NOP

// CHECK: nop; nop; msac.u8 normalize_c V4, V5, V6, V7, V8, SP0; nop
// CHECK: nop; nop; msac.u8 normalize_c V4, V5, V6, V7, V8, SP1; nop
// CHECK: nop; nop; msac.u8 normalize_c V4, V5, V6, V7, V8, !SP1; nop
// CHECK: nop; nop; msac.u8 normalize_c V4, V5, V6, V7, V8, VP1; nop
// CHECK: nop; nop; msac.u8 normalize_c V4, V5, V6, V7, V8, !VP1; nop

NOP; NOP; MSAC.U8 V4, V5, S3, V7, V8, NORMALIZE_C;         NOP
NOP; NOP; MSAC.U8 V4, V5, S3, V7, V8, NORMALIZE_C, SP1;    NOP
NOP; NOP; MSAC.U8 V4, V5, S3, V7, V8, NORMALIZE_C, !SP1;   NOP
NOP; NOP; MSAC.U8 V4, V5, S3, V7, V8, NORMALIZE_C, VP1;    NOP
NOP; NOP; MSAC.U8 V4, V5, S3, V7, V8, NORMALIZE_C, !VP1;   NOP

// CHECK: nop; nop; msac.u8 normalize_c V4, V5, S3, V7, V8, SP0; nop
// CHECK: nop; nop; msac.u8 normalize_c V4, V5, S3, V7, V8, SP1; nop
// CHECK: nop; nop; msac.u8 normalize_c V4, V5, S3, V7, V8, !SP1; nop
// CHECK: nop; nop; msac.u8 normalize_c V4, V5, S3, V7, V8, VP1; nop
// CHECK: nop; nop; msac.u8 normalize_c V4, V5, S3, V7, V8, !VP1; nop

NOP; NOP; MSAC.U8 V4, V5, 122, V7, V8, NORMALIZE_C;         NOP
NOP; NOP; MSAC.U8 V4, V5, 122, V7, V8, NORMALIZE_C, SP1;    NOP
NOP; NOP; MSAC.U8 V4, V5, 122, V7, V8, NORMALIZE_C, !SP1;   NOP
NOP; NOP; MSAC.U8 V4, V5, 122, V7, V8, NORMALIZE_C, VP1;    NOP
NOP; NOP; MSAC.U8 V4, V5, 122, V7, V8, NORMALIZE_C, !VP1;   NOP

// CHECK: nop; nop; msac.u8 normalize_c V4, V5, 0x7a, V7, V8, SP0; nop
// CHECK: nop; nop; msac.u8 normalize_c V4, V5, 0x7a, V7, V8, SP1; nop
// CHECK: nop; nop; msac.u8 normalize_c V4, V5, 0x7a, V7, V8, !SP1; nop
// CHECK: nop; nop; msac.u8 normalize_c V4, V5, 0x7a, V7, V8, VP1; nop
// CHECK: nop; nop; msac.u8 normalize_c V4, V5, 0x7a, V7, V8, !VP1; nop

NOP; NOP; MSAC.U8 V4, V5, V6, V7, S3, NORMALIZE_C;         NOP
NOP; NOP; MSAC.U8 V4, V5, V6, V7, S3, NORMALIZE_C, SP1;    NOP
NOP; NOP; MSAC.U8 V4, V5, V6, V7, S3, NORMALIZE_C, !SP1;   NOP
NOP; NOP; MSAC.U8 V4, V5, V6, V7, S3, NORMALIZE_C, VP1;    NOP
NOP; NOP; MSAC.U8 V4, V5, V6, V7, S3, NORMALIZE_C, !VP1;   NOP

// CHECK: nop; nop; msac.u8 normalize_c V4, V5, V6, V7, S3, SP0; nop
// CHECK: nop; nop; msac.u8 normalize_c V4, V5, V6, V7, S3, SP1; nop
// CHECK: nop; nop; msac.u8 normalize_c V4, V5, V6, V7, S3, !SP1; nop
// CHECK: nop; nop; msac.u8 normalize_c V4, V5, V6, V7, S3, VP1; nop
// CHECK: nop; nop; msac.u8 normalize_c V4, V5, V6, V7, S3, !VP1; nop

NOP; NOP; MSAC.U8 V4, V5, V6, V7, 123, NORMALIZE_C;         NOP
NOP; NOP; MSAC.U8 V4, V5, V6, V7, 123, NORMALIZE_C, SP1;    NOP
NOP; NOP; MSAC.U8 V4, V5, V6, V7, 123, NORMALIZE_C, !SP1;   NOP
NOP; NOP; MSAC.U8 V4, V5, V6, V7, 123, NORMALIZE_C, VP1;    NOP
NOP; NOP; MSAC.U8 V4, V5, V6, V7, 123, NORMALIZE_C, !VP1;   NOP

// CHECK: nop; nop; msac.u8 normalize_c V4, V5, V6, V7, 0x7b, SP0; nop
// CHECK: nop; nop; msac.u8 normalize_c V4, V5, V6, V7, 0x7b, SP1; nop
// CHECK: nop; nop; msac.u8 normalize_c V4, V5, V6, V7, 0x7b, !SP1; nop
// CHECK: nop; nop; msac.u8 normalize_c V4, V5, V6, V7, 0x7b, VP1; nop
// CHECK: nop; nop; msac.u8 normalize_c V4, V5, V6, V7, 0x7b, !VP1; nop


NOP; NOP; MSAC.I16 V4, V5, V6, V7, V8, RHU, NORMALIZE_AB;         NOP
NOP; NOP; MSAC.I16 V4, V5, V6, V7, V8, RHU, NORMALIZE_AB, SP1;    NOP
NOP; NOP; MSAC.I16 V4, V5, V6, V7, V8, RHU, NORMALIZE_AB, !SP1;   NOP
NOP; NOP; MSAC.I16 V4, V5, V6, V7, V8, RHU, NORMALIZE_AB, VP1;    NOP
NOP; NOP; MSAC.I16 V4, V5, V6, V7, V8, RHU, NORMALIZE_AB, !VP1;   NOP

// CHECK: nop; nop; msac.i16 rhu normalize_ab V4, V5, V6, V7, V8, SP0; nop
// CHECK: nop; nop; msac.i16 rhu normalize_ab V4, V5, V6, V7, V8, SP1; nop
// CHECK: nop; nop; msac.i16 rhu normalize_ab V4, V5, V6, V7, V8, !SP1; nop
// CHECK: nop; nop; msac.i16 rhu normalize_ab V4, V5, V6, V7, V8, VP1; nop
// CHECK: nop; nop; msac.i16 rhu normalize_ab V4, V5, V6, V7, V8, !VP1; nop

NOP; NOP; MSAC.I16 V4, V5, S3, V7, V8, RHU, NORMALIZE_AB;         NOP
NOP; NOP; MSAC.I16 V4, V5, S3, V7, V8, RHU, NORMALIZE_AB, SP1;    NOP
NOP; NOP; MSAC.I16 V4, V5, S3, V7, V8, RHU, NORMALIZE_AB, !SP1;   NOP
NOP; NOP; MSAC.I16 V4, V5, S3, V7, V8, RHU, NORMALIZE_AB, VP1;    NOP
NOP; NOP; MSAC.I16 V4, V5, S3, V7, V8, RHU, NORMALIZE_AB, !VP1;   NOP

// CHECK: nop; nop; msac.i16 rhu normalize_ab V4, V5, S3, V7, V8, SP0; nop
// CHECK: nop; nop; msac.i16 rhu normalize_ab V4, V5, S3, V7, V8, SP1; nop
// CHECK: nop; nop; msac.i16 rhu normalize_ab V4, V5, S3, V7, V8, !SP1; nop
// CHECK: nop; nop; msac.i16 rhu normalize_ab V4, V5, S3, V7, V8, VP1; nop
// CHECK: nop; nop; msac.i16 rhu normalize_ab V4, V5, S3, V7, V8, !VP1; nop

NOP; NOP; MSAC.I16 V4, V5, 122, V7, V8, RHU, NORMALIZE_AB;         NOP
NOP; NOP; MSAC.I16 V4, V5, 122, V7, V8, RHU, NORMALIZE_AB, SP1;    NOP
NOP; NOP; MSAC.I16 V4, V5, 122, V7, V8, RHU, NORMALIZE_AB, !SP1;   NOP
NOP; NOP; MSAC.I16 V4, V5, 122, V7, V8, RHU, NORMALIZE_AB, VP1;    NOP
NOP; NOP; MSAC.I16 V4, V5, 122, V7, V8, RHU, NORMALIZE_AB, !VP1;   NOP

// CHECK: nop; nop; msac.i16 rhu normalize_ab V4, V5, 0x7a, V7, V8, SP0; nop
// CHECK: nop; nop; msac.i16 rhu normalize_ab V4, V5, 0x7a, V7, V8, SP1; nop
// CHECK: nop; nop; msac.i16 rhu normalize_ab V4, V5, 0x7a, V7, V8, !SP1; nop
// CHECK: nop; nop; msac.i16 rhu normalize_ab V4, V5, 0x7a, V7, V8, VP1; nop
// CHECK: nop; nop; msac.i16 rhu normalize_ab V4, V5, 0x7a, V7, V8, !VP1; nop

NOP; NOP; MSAC.I16 V4, V5, V6, V7, S3, RHU, NORMALIZE_AB;         NOP
NOP; NOP; MSAC.I16 V4, V5, V6, V7, S3, RHU, NORMALIZE_AB, SP1;    NOP
NOP; NOP; MSAC.I16 V4, V5, V6, V7, S3, RHU, NORMALIZE_AB, !SP1;   NOP
NOP; NOP; MSAC.I16 V4, V5, V6, V7, S3, RHU, NORMALIZE_AB, VP1;    NOP
NOP; NOP; MSAC.I16 V4, V5, V6, V7, S3, RHU, NORMALIZE_AB, !VP1;   NOP

// CHECK: nop; nop; msac.i16 rhu normalize_ab V4, V5, V6, V7, S3, SP0; nop
// CHECK: nop; nop; msac.i16 rhu normalize_ab V4, V5, V6, V7, S3, SP1; nop
// CHECK: nop; nop; msac.i16 rhu normalize_ab V4, V5, V6, V7, S3, !SP1; nop
// CHECK: nop; nop; msac.i16 rhu normalize_ab V4, V5, V6, V7, S3, VP1; nop
// CHECK: nop; nop; msac.i16 rhu normalize_ab V4, V5, V6, V7, S3, !VP1; nop

NOP; NOP; MSAC.I16 V4, V5, V6, V7, 123, RHU, NORMALIZE_AB;         NOP
NOP; NOP; MSAC.I16 V4, V5, V6, V7, 123, RHU, NORMALIZE_AB, SP1;    NOP
NOP; NOP; MSAC.I16 V4, V5, V6, V7, 123, RHU, NORMALIZE_AB, !SP1;   NOP
NOP; NOP; MSAC.I16 V4, V5, V6, V7, 123, RHU, NORMALIZE_AB, VP1;    NOP
NOP; NOP; MSAC.I16 V4, V5, V6, V7, 123, RHU, NORMALIZE_AB, !VP1;   NOP

// CHECK: nop; nop; msac.i16 rhu normalize_ab V4, V5, V6, V7, 0x7b, SP0; nop
// CHECK: nop; nop; msac.i16 rhu normalize_ab V4, V5, V6, V7, 0x7b, SP1; nop
// CHECK: nop; nop; msac.i16 rhu normalize_ab V4, V5, V6, V7, 0x7b, !SP1; nop
// CHECK: nop; nop; msac.i16 rhu normalize_ab V4, V5, V6, V7, 0x7b, VP1; nop
// CHECK: nop; nop; msac.i16 rhu normalize_ab V4, V5, V6, V7, 0x7b, !VP1; nop

NOP; NOP; MSAC.I8 V4, V5, V6, V7, V8, RHU, NORMALIZE_AB;         NOP
NOP; NOP; MSAC.I8 V4, V5, V6, V7, V8, RHU, NORMALIZE_AB, SP1;    NOP
NOP; NOP; MSAC.I8 V4, V5, V6, V7, V8, RHU, NORMALIZE_AB, !SP1;   NOP
NOP; NOP; MSAC.I8 V4, V5, V6, V7, V8, RHU, NORMALIZE_AB, VP1;    NOP
NOP; NOP; MSAC.I8 V4, V5, V6, V7, V8, RHU, NORMALIZE_AB, !VP1;   NOP

// CHECK: nop; nop; msac.i8 rhu normalize_ab V4, V5, V6, V7, V8, SP0; nop
// CHECK: nop; nop; msac.i8 rhu normalize_ab V4, V5, V6, V7, V8, SP1; nop
// CHECK: nop; nop; msac.i8 rhu normalize_ab V4, V5, V6, V7, V8, !SP1; nop
// CHECK: nop; nop; msac.i8 rhu normalize_ab V4, V5, V6, V7, V8, VP1; nop
// CHECK: nop; nop; msac.i8 rhu normalize_ab V4, V5, V6, V7, V8, !VP1; nop

NOP; NOP; MSAC.I8 V4, V5, S3, V7, V8, RHU, NORMALIZE_AB;         NOP
NOP; NOP; MSAC.I8 V4, V5, S3, V7, V8, RHU, NORMALIZE_AB, SP1;    NOP
NOP; NOP; MSAC.I8 V4, V5, S3, V7, V8, RHU, NORMALIZE_AB, !SP1;   NOP
NOP; NOP; MSAC.I8 V4, V5, S3, V7, V8, RHU, NORMALIZE_AB, VP1;    NOP
NOP; NOP; MSAC.I8 V4, V5, S3, V7, V8, RHU, NORMALIZE_AB, !VP1;   NOP

// CHECK: nop; nop; msac.i8 rhu normalize_ab V4, V5, S3, V7, V8, SP0; nop
// CHECK: nop; nop; msac.i8 rhu normalize_ab V4, V5, S3, V7, V8, SP1; nop
// CHECK: nop; nop; msac.i8 rhu normalize_ab V4, V5, S3, V7, V8, !SP1; nop
// CHECK: nop; nop; msac.i8 rhu normalize_ab V4, V5, S3, V7, V8, VP1; nop
// CHECK: nop; nop; msac.i8 rhu normalize_ab V4, V5, S3, V7, V8, !VP1; nop

NOP; NOP; MSAC.I8 V4, V5, 122, V7, V8, RHU, NORMALIZE_AB;         NOP
NOP; NOP; MSAC.I8 V4, V5, 122, V7, V8, RHU, NORMALIZE_AB, SP1;    NOP
NOP; NOP; MSAC.I8 V4, V5, 122, V7, V8, RHU, NORMALIZE_AB, !SP1;   NOP
NOP; NOP; MSAC.I8 V4, V5, 122, V7, V8, RHU, NORMALIZE_AB, VP1;    NOP
NOP; NOP; MSAC.I8 V4, V5, 122, V7, V8, RHU, NORMALIZE_AB, !VP1;   NOP

// CHECK: nop; nop; msac.i8 rhu normalize_ab V4, V5, 0x7a, V7, V8, SP0; nop
// CHECK: nop; nop; msac.i8 rhu normalize_ab V4, V5, 0x7a, V7, V8, SP1; nop
// CHECK: nop; nop; msac.i8 rhu normalize_ab V4, V5, 0x7a, V7, V8, !SP1; nop
// CHECK: nop; nop; msac.i8 rhu normalize_ab V4, V5, 0x7a, V7, V8, VP1; nop
// CHECK: nop; nop; msac.i8 rhu normalize_ab V4, V5, 0x7a, V7, V8, !VP1; nop

NOP; NOP; MSAC.I8 V4, V5, V6, V7, S3, RHU, NORMALIZE_AB;         NOP
NOP; NOP; MSAC.I8 V4, V5, V6, V7, S3, RHU, NORMALIZE_AB, SP1;    NOP
NOP; NOP; MSAC.I8 V4, V5, V6, V7, S3, RHU, NORMALIZE_AB, !SP1;   NOP
NOP; NOP; MSAC.I8 V4, V5, V6, V7, S3, RHU, NORMALIZE_AB, VP1;    NOP
NOP; NOP; MSAC.I8 V4, V5, V6, V7, S3, RHU, NORMALIZE_AB, !VP1;   NOP

// CHECK: nop; nop; msac.i8 rhu normalize_ab V4, V5, V6, V7, S3, SP0; nop
// CHECK: nop; nop; msac.i8 rhu normalize_ab V4, V5, V6, V7, S3, SP1; nop
// CHECK: nop; nop; msac.i8 rhu normalize_ab V4, V5, V6, V7, S3, !SP1; nop
// CHECK: nop; nop; msac.i8 rhu normalize_ab V4, V5, V6, V7, S3, VP1; nop
// CHECK: nop; nop; msac.i8 rhu normalize_ab V4, V5, V6, V7, S3, !VP1; nop

NOP; NOP; MSAC.I8 V4, V5, V6, V7, 123, RHU, NORMALIZE_AB;         NOP
NOP; NOP; MSAC.I8 V4, V5, V6, V7, 123, RHU, NORMALIZE_AB, SP1;    NOP
NOP; NOP; MSAC.I8 V4, V5, V6, V7, 123, RHU, NORMALIZE_AB, !SP1;   NOP
NOP; NOP; MSAC.I8 V4, V5, V6, V7, 123, RHU, NORMALIZE_AB, VP1;    NOP
NOP; NOP; MSAC.I8 V4, V5, V6, V7, 123, RHU, NORMALIZE_AB, !VP1;   NOP

// CHECK: nop; nop; msac.i8 rhu normalize_ab V4, V5, V6, V7, 0x7b, SP0; nop
// CHECK: nop; nop; msac.i8 rhu normalize_ab V4, V5, V6, V7, 0x7b, SP1; nop
// CHECK: nop; nop; msac.i8 rhu normalize_ab V4, V5, V6, V7, 0x7b, !SP1; nop
// CHECK: nop; nop; msac.i8 rhu normalize_ab V4, V5, V6, V7, 0x7b, VP1; nop
// CHECK: nop; nop; msac.i8 rhu normalize_ab V4, V5, V6, V7, 0x7b, !VP1; nop

NOP; NOP; MSAC.U16 V4, V5, V6, V7, V8, RHU, NORMALIZE_AB;         NOP
NOP; NOP; MSAC.U16 V4, V5, V6, V7, V8, RHU, NORMALIZE_AB, SP1;    NOP
NOP; NOP; MSAC.U16 V4, V5, V6, V7, V8, RHU, NORMALIZE_AB, !SP1;   NOP
NOP; NOP; MSAC.U16 V4, V5, V6, V7, V8, RHU, NORMALIZE_AB, VP1;    NOP
NOP; NOP; MSAC.U16 V4, V5, V6, V7, V8, RHU, NORMALIZE_AB, !VP1;   NOP

// CHECK: nop; nop; msac.u16 rhu normalize_ab V4, V5, V6, V7, V8, SP0; nop
// CHECK: nop; nop; msac.u16 rhu normalize_ab V4, V5, V6, V7, V8, SP1; nop
// CHECK: nop; nop; msac.u16 rhu normalize_ab V4, V5, V6, V7, V8, !SP1; nop
// CHECK: nop; nop; msac.u16 rhu normalize_ab V4, V5, V6, V7, V8, VP1; nop
// CHECK: nop; nop; msac.u16 rhu normalize_ab V4, V5, V6, V7, V8, !VP1; nop

NOP; NOP; MSAC.U16 V4, V5, S3, V7, V8, RHU, NORMALIZE_AB;         NOP
NOP; NOP; MSAC.U16 V4, V5, S3, V7, V8, RHU, NORMALIZE_AB, SP1;    NOP
NOP; NOP; MSAC.U16 V4, V5, S3, V7, V8, RHU, NORMALIZE_AB, !SP1;   NOP
NOP; NOP; MSAC.U16 V4, V5, S3, V7, V8, RHU, NORMALIZE_AB, VP1;    NOP
NOP; NOP; MSAC.U16 V4, V5, S3, V7, V8, RHU, NORMALIZE_AB, !VP1;   NOP

// CHECK: nop; nop; msac.u16 rhu normalize_ab V4, V5, S3, V7, V8, SP0; nop
// CHECK: nop; nop; msac.u16 rhu normalize_ab V4, V5, S3, V7, V8, SP1; nop
// CHECK: nop; nop; msac.u16 rhu normalize_ab V4, V5, S3, V7, V8, !SP1; nop
// CHECK: nop; nop; msac.u16 rhu normalize_ab V4, V5, S3, V7, V8, VP1; nop
// CHECK: nop; nop; msac.u16 rhu normalize_ab V4, V5, S3, V7, V8, !VP1; nop

NOP; NOP; MSAC.U16 V4, V5, 122, V7, V8, RHU, NORMALIZE_AB;         NOP
NOP; NOP; MSAC.U16 V4, V5, 122, V7, V8, RHU, NORMALIZE_AB, SP1;    NOP
NOP; NOP; MSAC.U16 V4, V5, 122, V7, V8, RHU, NORMALIZE_AB, !SP1;   NOP
NOP; NOP; MSAC.U16 V4, V5, 122, V7, V8, RHU, NORMALIZE_AB, VP1;    NOP
NOP; NOP; MSAC.U16 V4, V5, 122, V7, V8, RHU, NORMALIZE_AB, !VP1;   NOP

// CHECK: nop; nop; msac.u16 rhu normalize_ab V4, V5, 0x7a, V7, V8, SP0; nop
// CHECK: nop; nop; msac.u16 rhu normalize_ab V4, V5, 0x7a, V7, V8, SP1; nop
// CHECK: nop; nop; msac.u16 rhu normalize_ab V4, V5, 0x7a, V7, V8, !SP1; nop
// CHECK: nop; nop; msac.u16 rhu normalize_ab V4, V5, 0x7a, V7, V8, VP1; nop
// CHECK: nop; nop; msac.u16 rhu normalize_ab V4, V5, 0x7a, V7, V8, !VP1; nop

NOP; NOP; MSAC.U16 V4, V5, V6, V7, S3, RHU, NORMALIZE_AB;         NOP
NOP; NOP; MSAC.U16 V4, V5, V6, V7, S3, RHU, NORMALIZE_AB, SP1;    NOP
NOP; NOP; MSAC.U16 V4, V5, V6, V7, S3, RHU, NORMALIZE_AB, !SP1;   NOP
NOP; NOP; MSAC.U16 V4, V5, V6, V7, S3, RHU, NORMALIZE_AB, VP1;    NOP
NOP; NOP; MSAC.U16 V4, V5, V6, V7, S3, RHU, NORMALIZE_AB, !VP1;   NOP

// CHECK: nop; nop; msac.u16 rhu normalize_ab V4, V5, V6, V7, S3, SP0; nop
// CHECK: nop; nop; msac.u16 rhu normalize_ab V4, V5, V6, V7, S3, SP1; nop
// CHECK: nop; nop; msac.u16 rhu normalize_ab V4, V5, V6, V7, S3, !SP1; nop
// CHECK: nop; nop; msac.u16 rhu normalize_ab V4, V5, V6, V7, S3, VP1; nop
// CHECK: nop; nop; msac.u16 rhu normalize_ab V4, V5, V6, V7, S3, !VP1; nop

NOP; NOP; MSAC.U16 V4, V5, V6, V7, 123, RHU, NORMALIZE_AB;         NOP
NOP; NOP; MSAC.U16 V4, V5, V6, V7, 123, RHU, NORMALIZE_AB, SP1;    NOP
NOP; NOP; MSAC.U16 V4, V5, V6, V7, 123, RHU, NORMALIZE_AB, !SP1;   NOP
NOP; NOP; MSAC.U16 V4, V5, V6, V7, 123, RHU, NORMALIZE_AB, VP1;    NOP
NOP; NOP; MSAC.U16 V4, V5, V6, V7, 123, RHU, NORMALIZE_AB, !VP1;   NOP

// CHECK: nop; nop; msac.u16 rhu normalize_ab V4, V5, V6, V7, 0x7b, SP0; nop
// CHECK: nop; nop; msac.u16 rhu normalize_ab V4, V5, V6, V7, 0x7b, SP1; nop
// CHECK: nop; nop; msac.u16 rhu normalize_ab V4, V5, V6, V7, 0x7b, !SP1; nop
// CHECK: nop; nop; msac.u16 rhu normalize_ab V4, V5, V6, V7, 0x7b, VP1; nop
// CHECK: nop; nop; msac.u16 rhu normalize_ab V4, V5, V6, V7, 0x7b, !VP1; nop

NOP; NOP; MSAC.U8 V4, V5, V6, V7, V8, RHU, NORMALIZE_AB;         NOP
NOP; NOP; MSAC.U8 V4, V5, V6, V7, V8, RHU, NORMALIZE_AB, SP1;    NOP
NOP; NOP; MSAC.U8 V4, V5, V6, V7, V8, RHU, NORMALIZE_AB, !SP1;   NOP
NOP; NOP; MSAC.U8 V4, V5, V6, V7, V8, RHU, NORMALIZE_AB, VP1;    NOP
NOP; NOP; MSAC.U8 V4, V5, V6, V7, V8, RHU, NORMALIZE_AB, !VP1;   NOP

// CHECK: nop; nop; msac.u8 rhu normalize_ab V4, V5, V6, V7, V8, SP0; nop
// CHECK: nop; nop; msac.u8 rhu normalize_ab V4, V5, V6, V7, V8, SP1; nop
// CHECK: nop; nop; msac.u8 rhu normalize_ab V4, V5, V6, V7, V8, !SP1; nop
// CHECK: nop; nop; msac.u8 rhu normalize_ab V4, V5, V6, V7, V8, VP1; nop
// CHECK: nop; nop; msac.u8 rhu normalize_ab V4, V5, V6, V7, V8, !VP1; nop

NOP; NOP; MSAC.U8 V4, V5, S3, V7, V8, RHU, NORMALIZE_AB;         NOP
NOP; NOP; MSAC.U8 V4, V5, S3, V7, V8, RHU, NORMALIZE_AB, SP1;    NOP
NOP; NOP; MSAC.U8 V4, V5, S3, V7, V8, RHU, NORMALIZE_AB, !SP1;   NOP
NOP; NOP; MSAC.U8 V4, V5, S3, V7, V8, RHU, NORMALIZE_AB, VP1;    NOP
NOP; NOP; MSAC.U8 V4, V5, S3, V7, V8, RHU, NORMALIZE_AB, !VP1;   NOP

// CHECK: nop; nop; msac.u8 rhu normalize_ab V4, V5, S3, V7, V8, SP0; nop
// CHECK: nop; nop; msac.u8 rhu normalize_ab V4, V5, S3, V7, V8, SP1; nop
// CHECK: nop; nop; msac.u8 rhu normalize_ab V4, V5, S3, V7, V8, !SP1; nop
// CHECK: nop; nop; msac.u8 rhu normalize_ab V4, V5, S3, V7, V8, VP1; nop
// CHECK: nop; nop; msac.u8 rhu normalize_ab V4, V5, S3, V7, V8, !VP1; nop

NOP; NOP; MSAC.U8 V4, V5, 122, V7, V8, RHU, NORMALIZE_AB;         NOP
NOP; NOP; MSAC.U8 V4, V5, 122, V7, V8, RHU, NORMALIZE_AB, SP1;    NOP
NOP; NOP; MSAC.U8 V4, V5, 122, V7, V8, RHU, NORMALIZE_AB, !SP1;   NOP
NOP; NOP; MSAC.U8 V4, V5, 122, V7, V8, RHU, NORMALIZE_AB, VP1;    NOP
NOP; NOP; MSAC.U8 V4, V5, 122, V7, V8, RHU, NORMALIZE_AB, !VP1;   NOP

// CHECK: nop; nop; msac.u8 rhu normalize_ab V4, V5, 0x7a, V7, V8, SP0; nop
// CHECK: nop; nop; msac.u8 rhu normalize_ab V4, V5, 0x7a, V7, V8, SP1; nop
// CHECK: nop; nop; msac.u8 rhu normalize_ab V4, V5, 0x7a, V7, V8, !SP1; nop
// CHECK: nop; nop; msac.u8 rhu normalize_ab V4, V5, 0x7a, V7, V8, VP1; nop
// CHECK: nop; nop; msac.u8 rhu normalize_ab V4, V5, 0x7a, V7, V8, !VP1; nop

NOP; NOP; MSAC.U8 V4, V5, V6, V7, S3, RHU, NORMALIZE_AB;         NOP
NOP; NOP; MSAC.U8 V4, V5, V6, V7, S3, RHU, NORMALIZE_AB, SP1;    NOP
NOP; NOP; MSAC.U8 V4, V5, V6, V7, S3, RHU, NORMALIZE_AB, !SP1;   NOP
NOP; NOP; MSAC.U8 V4, V5, V6, V7, S3, RHU, NORMALIZE_AB, VP1;    NOP
NOP; NOP; MSAC.U8 V4, V5, V6, V7, S3, RHU, NORMALIZE_AB, !VP1;   NOP

// CHECK: nop; nop; msac.u8 rhu normalize_ab V4, V5, V6, V7, S3, SP0; nop
// CHECK: nop; nop; msac.u8 rhu normalize_ab V4, V5, V6, V7, S3, SP1; nop
// CHECK: nop; nop; msac.u8 rhu normalize_ab V4, V5, V6, V7, S3, !SP1; nop
// CHECK: nop; nop; msac.u8 rhu normalize_ab V4, V5, V6, V7, S3, VP1; nop
// CHECK: nop; nop; msac.u8 rhu normalize_ab V4, V5, V6, V7, S3, !VP1; nop

NOP; NOP; MSAC.U8 V4, V5, V6, V7, 123, RHU, NORMALIZE_AB;         NOP
NOP; NOP; MSAC.U8 V4, V5, V6, V7, 123, RHU, NORMALIZE_AB, SP1;    NOP
NOP; NOP; MSAC.U8 V4, V5, V6, V7, 123, RHU, NORMALIZE_AB, !SP1;   NOP
NOP; NOP; MSAC.U8 V4, V5, V6, V7, 123, RHU, NORMALIZE_AB, VP1;    NOP
NOP; NOP; MSAC.U8 V4, V5, V6, V7, 123, RHU, NORMALIZE_AB, !VP1;   NOP

// CHECK: nop; nop; msac.u8 rhu normalize_ab V4, V5, V6, V7, 0x7b, SP0; nop
// CHECK: nop; nop; msac.u8 rhu normalize_ab V4, V5, V6, V7, 0x7b, SP1; nop
// CHECK: nop; nop; msac.u8 rhu normalize_ab V4, V5, V6, V7, 0x7b, !SP1; nop
// CHECK: nop; nop; msac.u8 rhu normalize_ab V4, V5, V6, V7, 0x7b, VP1; nop
// CHECK: nop; nop; msac.u8 rhu normalize_ab V4, V5, V6, V7, 0x7b, !VP1; nop

NOP; NOP; MSAC.I16 V4, V5, V6, V7, V8, RHU, NORMALIZE_C;         NOP
NOP; NOP; MSAC.I16 V4, V5, V6, V7, V8, RHU, NORMALIZE_C, SP1;    NOP
NOP; NOP; MSAC.I16 V4, V5, V6, V7, V8, RHU, NORMALIZE_C, !SP1;   NOP
NOP; NOP; MSAC.I16 V4, V5, V6, V7, V8, RHU, NORMALIZE_C, VP1;    NOP
NOP; NOP; MSAC.I16 V4, V5, V6, V7, V8, RHU, NORMALIZE_C, !VP1;   NOP

// CHECK: nop; nop; msac.i16 rhu normalize_c V4, V5, V6, V7, V8, SP0; nop
// CHECK: nop; nop; msac.i16 rhu normalize_c V4, V5, V6, V7, V8, SP1; nop
// CHECK: nop; nop; msac.i16 rhu normalize_c V4, V5, V6, V7, V8, !SP1; nop
// CHECK: nop; nop; msac.i16 rhu normalize_c V4, V5, V6, V7, V8, VP1; nop
// CHECK: nop; nop; msac.i16 rhu normalize_c V4, V5, V6, V7, V8, !VP1; nop

NOP; NOP; MSAC.I16 V4, V5, S3, V7, V8, RHU, NORMALIZE_C;         NOP
NOP; NOP; MSAC.I16 V4, V5, S3, V7, V8, RHU, NORMALIZE_C, SP1;    NOP
NOP; NOP; MSAC.I16 V4, V5, S3, V7, V8, RHU, NORMALIZE_C, !SP1;   NOP
NOP; NOP; MSAC.I16 V4, V5, S3, V7, V8, RHU, NORMALIZE_C, VP1;    NOP
NOP; NOP; MSAC.I16 V4, V5, S3, V7, V8, RHU, NORMALIZE_C, !VP1;   NOP

// CHECK: nop; nop; msac.i16 rhu normalize_c V4, V5, S3, V7, V8, SP0; nop
// CHECK: nop; nop; msac.i16 rhu normalize_c V4, V5, S3, V7, V8, SP1; nop
// CHECK: nop; nop; msac.i16 rhu normalize_c V4, V5, S3, V7, V8, !SP1; nop
// CHECK: nop; nop; msac.i16 rhu normalize_c V4, V5, S3, V7, V8, VP1; nop
// CHECK: nop; nop; msac.i16 rhu normalize_c V4, V5, S3, V7, V8, !VP1; nop

NOP; NOP; MSAC.I16 V4, V5, 122, V7, V8, RHU, NORMALIZE_C;         NOP
NOP; NOP; MSAC.I16 V4, V5, 122, V7, V8, RHU, NORMALIZE_C, SP1;    NOP
NOP; NOP; MSAC.I16 V4, V5, 122, V7, V8, RHU, NORMALIZE_C, !SP1;   NOP
NOP; NOP; MSAC.I16 V4, V5, 122, V7, V8, RHU, NORMALIZE_C, VP1;    NOP
NOP; NOP; MSAC.I16 V4, V5, 122, V7, V8, RHU, NORMALIZE_C, !VP1;   NOP

// CHECK: nop; nop; msac.i16 rhu normalize_c V4, V5, 0x7a, V7, V8, SP0; nop
// CHECK: nop; nop; msac.i16 rhu normalize_c V4, V5, 0x7a, V7, V8, SP1; nop
// CHECK: nop; nop; msac.i16 rhu normalize_c V4, V5, 0x7a, V7, V8, !SP1; nop
// CHECK: nop; nop; msac.i16 rhu normalize_c V4, V5, 0x7a, V7, V8, VP1; nop
// CHECK: nop; nop; msac.i16 rhu normalize_c V4, V5, 0x7a, V7, V8, !VP1; nop

NOP; NOP; MSAC.I16 V4, V5, V6, V7, S3, RHU, NORMALIZE_C;         NOP
NOP; NOP; MSAC.I16 V4, V5, V6, V7, S3, RHU, NORMALIZE_C, SP1;    NOP
NOP; NOP; MSAC.I16 V4, V5, V6, V7, S3, RHU, NORMALIZE_C, !SP1;   NOP
NOP; NOP; MSAC.I16 V4, V5, V6, V7, S3, RHU, NORMALIZE_C, VP1;    NOP
NOP; NOP; MSAC.I16 V4, V5, V6, V7, S3, RHU, NORMALIZE_C, !VP1;   NOP

// CHECK: nop; nop; msac.i16 rhu normalize_c V4, V5, V6, V7, S3, SP0; nop
// CHECK: nop; nop; msac.i16 rhu normalize_c V4, V5, V6, V7, S3, SP1; nop
// CHECK: nop; nop; msac.i16 rhu normalize_c V4, V5, V6, V7, S3, !SP1; nop
// CHECK: nop; nop; msac.i16 rhu normalize_c V4, V5, V6, V7, S3, VP1; nop
// CHECK: nop; nop; msac.i16 rhu normalize_c V4, V5, V6, V7, S3, !VP1; nop

NOP; NOP; MSAC.I16 V4, V5, V6, V7, 123, RHU, NORMALIZE_C;         NOP
NOP; NOP; MSAC.I16 V4, V5, V6, V7, 123, RHU, NORMALIZE_C, SP1;    NOP
NOP; NOP; MSAC.I16 V4, V5, V6, V7, 123, RHU, NORMALIZE_C, !SP1;   NOP
NOP; NOP; MSAC.I16 V4, V5, V6, V7, 123, RHU, NORMALIZE_C, VP1;    NOP
NOP; NOP; MSAC.I16 V4, V5, V6, V7, 123, RHU, NORMALIZE_C, !VP1;   NOP

// CHECK: nop; nop; msac.i16 rhu normalize_c V4, V5, V6, V7, 0x7b, SP0; nop
// CHECK: nop; nop; msac.i16 rhu normalize_c V4, V5, V6, V7, 0x7b, SP1; nop
// CHECK: nop; nop; msac.i16 rhu normalize_c V4, V5, V6, V7, 0x7b, !SP1; nop
// CHECK: nop; nop; msac.i16 rhu normalize_c V4, V5, V6, V7, 0x7b, VP1; nop
// CHECK: nop; nop; msac.i16 rhu normalize_c V4, V5, V6, V7, 0x7b, !VP1; nop

NOP; NOP; MSAC.I8 V4, V5, V6, V7, V8, RHU, NORMALIZE_C;         NOP
NOP; NOP; MSAC.I8 V4, V5, V6, V7, V8, RHU, NORMALIZE_C, SP1;    NOP
NOP; NOP; MSAC.I8 V4, V5, V6, V7, V8, RHU, NORMALIZE_C, !SP1;   NOP
NOP; NOP; MSAC.I8 V4, V5, V6, V7, V8, RHU, NORMALIZE_C, VP1;    NOP
NOP; NOP; MSAC.I8 V4, V5, V6, V7, V8, RHU, NORMALIZE_C, !VP1;   NOP

// CHECK: nop; nop; msac.i8 rhu normalize_c V4, V5, V6, V7, V8, SP0; nop
// CHECK: nop; nop; msac.i8 rhu normalize_c V4, V5, V6, V7, V8, SP1; nop
// CHECK: nop; nop; msac.i8 rhu normalize_c V4, V5, V6, V7, V8, !SP1; nop
// CHECK: nop; nop; msac.i8 rhu normalize_c V4, V5, V6, V7, V8, VP1; nop
// CHECK: nop; nop; msac.i8 rhu normalize_c V4, V5, V6, V7, V8, !VP1; nop

NOP; NOP; MSAC.I8 V4, V5, S3, V7, V8, RHU, NORMALIZE_C;         NOP
NOP; NOP; MSAC.I8 V4, V5, S3, V7, V8, RHU, NORMALIZE_C, SP1;    NOP
NOP; NOP; MSAC.I8 V4, V5, S3, V7, V8, RHU, NORMALIZE_C, !SP1;   NOP
NOP; NOP; MSAC.I8 V4, V5, S3, V7, V8, RHU, NORMALIZE_C, VP1;    NOP
NOP; NOP; MSAC.I8 V4, V5, S3, V7, V8, RHU, NORMALIZE_C, !VP1;   NOP

// CHECK: nop; nop; msac.i8 rhu normalize_c V4, V5, S3, V7, V8, SP0; nop
// CHECK: nop; nop; msac.i8 rhu normalize_c V4, V5, S3, V7, V8, SP1; nop
// CHECK: nop; nop; msac.i8 rhu normalize_c V4, V5, S3, V7, V8, !SP1; nop
// CHECK: nop; nop; msac.i8 rhu normalize_c V4, V5, S3, V7, V8, VP1; nop
// CHECK: nop; nop; msac.i8 rhu normalize_c V4, V5, S3, V7, V8, !VP1; nop

NOP; NOP; MSAC.I8 V4, V5, 122, V7, V8, RHU, NORMALIZE_C;         NOP
NOP; NOP; MSAC.I8 V4, V5, 122, V7, V8, RHU, NORMALIZE_C, SP1;    NOP
NOP; NOP; MSAC.I8 V4, V5, 122, V7, V8, RHU, NORMALIZE_C, !SP1;   NOP
NOP; NOP; MSAC.I8 V4, V5, 122, V7, V8, RHU, NORMALIZE_C, VP1;    NOP
NOP; NOP; MSAC.I8 V4, V5, 122, V7, V8, RHU, NORMALIZE_C, !VP1;   NOP

// CHECK: nop; nop; msac.i8 rhu normalize_c V4, V5, 0x7a, V7, V8, SP0; nop
// CHECK: nop; nop; msac.i8 rhu normalize_c V4, V5, 0x7a, V7, V8, SP1; nop
// CHECK: nop; nop; msac.i8 rhu normalize_c V4, V5, 0x7a, V7, V8, !SP1; nop
// CHECK: nop; nop; msac.i8 rhu normalize_c V4, V5, 0x7a, V7, V8, VP1; nop
// CHECK: nop; nop; msac.i8 rhu normalize_c V4, V5, 0x7a, V7, V8, !VP1; nop

NOP; NOP; MSAC.I8 V4, V5, V6, V7, S3, RHU, NORMALIZE_C;         NOP
NOP; NOP; MSAC.I8 V4, V5, V6, V7, S3, RHU, NORMALIZE_C, SP1;    NOP
NOP; NOP; MSAC.I8 V4, V5, V6, V7, S3, RHU, NORMALIZE_C, !SP1;   NOP
NOP; NOP; MSAC.I8 V4, V5, V6, V7, S3, RHU, NORMALIZE_C, VP1;    NOP
NOP; NOP; MSAC.I8 V4, V5, V6, V7, S3, RHU, NORMALIZE_C, !VP1;   NOP

// CHECK: nop; nop; msac.i8 rhu normalize_c V4, V5, V6, V7, S3, SP0; nop
// CHECK: nop; nop; msac.i8 rhu normalize_c V4, V5, V6, V7, S3, SP1; nop
// CHECK: nop; nop; msac.i8 rhu normalize_c V4, V5, V6, V7, S3, !SP1; nop
// CHECK: nop; nop; msac.i8 rhu normalize_c V4, V5, V6, V7, S3, VP1; nop
// CHECK: nop; nop; msac.i8 rhu normalize_c V4, V5, V6, V7, S3, !VP1; nop

NOP; NOP; MSAC.I8 V4, V5, V6, V7, 123, RHU, NORMALIZE_C;         NOP
NOP; NOP; MSAC.I8 V4, V5, V6, V7, 123, RHU, NORMALIZE_C, SP1;    NOP
NOP; NOP; MSAC.I8 V4, V5, V6, V7, 123, RHU, NORMALIZE_C, !SP1;   NOP
NOP; NOP; MSAC.I8 V4, V5, V6, V7, 123, RHU, NORMALIZE_C, VP1;    NOP
NOP; NOP; MSAC.I8 V4, V5, V6, V7, 123, RHU, NORMALIZE_C, !VP1;   NOP

// CHECK: nop; nop; msac.i8 rhu normalize_c V4, V5, V6, V7, 0x7b, SP0; nop
// CHECK: nop; nop; msac.i8 rhu normalize_c V4, V5, V6, V7, 0x7b, SP1; nop
// CHECK: nop; nop; msac.i8 rhu normalize_c V4, V5, V6, V7, 0x7b, !SP1; nop
// CHECK: nop; nop; msac.i8 rhu normalize_c V4, V5, V6, V7, 0x7b, VP1; nop
// CHECK: nop; nop; msac.i8 rhu normalize_c V4, V5, V6, V7, 0x7b, !VP1; nop

NOP; NOP; MSAC.U16 V4, V5, V6, V7, V8, RHU, NORMALIZE_C;         NOP
NOP; NOP; MSAC.U16 V4, V5, V6, V7, V8, RHU, NORMALIZE_C, SP1;    NOP
NOP; NOP; MSAC.U16 V4, V5, V6, V7, V8, RHU, NORMALIZE_C, !SP1;   NOP
NOP; NOP; MSAC.U16 V4, V5, V6, V7, V8, RHU, NORMALIZE_C, VP1;    NOP
NOP; NOP; MSAC.U16 V4, V5, V6, V7, V8, RHU, NORMALIZE_C, !VP1;   NOP

// CHECK: nop; nop; msac.u16 rhu normalize_c V4, V5, V6, V7, V8, SP0; nop
// CHECK: nop; nop; msac.u16 rhu normalize_c V4, V5, V6, V7, V8, SP1; nop
// CHECK: nop; nop; msac.u16 rhu normalize_c V4, V5, V6, V7, V8, !SP1; nop
// CHECK: nop; nop; msac.u16 rhu normalize_c V4, V5, V6, V7, V8, VP1; nop
// CHECK: nop; nop; msac.u16 rhu normalize_c V4, V5, V6, V7, V8, !VP1; nop

NOP; NOP; MSAC.U16 V4, V5, S3, V7, V8, RHU, NORMALIZE_C;         NOP
NOP; NOP; MSAC.U16 V4, V5, S3, V7, V8, RHU, NORMALIZE_C, SP1;    NOP
NOP; NOP; MSAC.U16 V4, V5, S3, V7, V8, RHU, NORMALIZE_C, !SP1;   NOP
NOP; NOP; MSAC.U16 V4, V5, S3, V7, V8, RHU, NORMALIZE_C, VP1;    NOP
NOP; NOP; MSAC.U16 V4, V5, S3, V7, V8, RHU, NORMALIZE_C, !VP1;   NOP

// CHECK: nop; nop; msac.u16 rhu normalize_c V4, V5, S3, V7, V8, SP0; nop
// CHECK: nop; nop; msac.u16 rhu normalize_c V4, V5, S3, V7, V8, SP1; nop
// CHECK: nop; nop; msac.u16 rhu normalize_c V4, V5, S3, V7, V8, !SP1; nop
// CHECK: nop; nop; msac.u16 rhu normalize_c V4, V5, S3, V7, V8, VP1; nop
// CHECK: nop; nop; msac.u16 rhu normalize_c V4, V5, S3, V7, V8, !VP1; nop

NOP; NOP; MSAC.U16 V4, V5, 122, V7, V8, RHU, NORMALIZE_C;         NOP
NOP; NOP; MSAC.U16 V4, V5, 122, V7, V8, RHU, NORMALIZE_C, SP1;    NOP
NOP; NOP; MSAC.U16 V4, V5, 122, V7, V8, RHU, NORMALIZE_C, !SP1;   NOP
NOP; NOP; MSAC.U16 V4, V5, 122, V7, V8, RHU, NORMALIZE_C, VP1;    NOP
NOP; NOP; MSAC.U16 V4, V5, 122, V7, V8, RHU, NORMALIZE_C, !VP1;   NOP

// CHECK: nop; nop; msac.u16 rhu normalize_c V4, V5, 0x7a, V7, V8, SP0; nop
// CHECK: nop; nop; msac.u16 rhu normalize_c V4, V5, 0x7a, V7, V8, SP1; nop
// CHECK: nop; nop; msac.u16 rhu normalize_c V4, V5, 0x7a, V7, V8, !SP1; nop
// CHECK: nop; nop; msac.u16 rhu normalize_c V4, V5, 0x7a, V7, V8, VP1; nop
// CHECK: nop; nop; msac.u16 rhu normalize_c V4, V5, 0x7a, V7, V8, !VP1; nop

NOP; NOP; MSAC.U16 V4, V5, V6, V7, S3, RHU, NORMALIZE_C;         NOP
NOP; NOP; MSAC.U16 V4, V5, V6, V7, S3, RHU, NORMALIZE_C, SP1;    NOP
NOP; NOP; MSAC.U16 V4, V5, V6, V7, S3, RHU, NORMALIZE_C, !SP1;   NOP
NOP; NOP; MSAC.U16 V4, V5, V6, V7, S3, RHU, NORMALIZE_C, VP1;    NOP
NOP; NOP; MSAC.U16 V4, V5, V6, V7, S3, RHU, NORMALIZE_C, !VP1;   NOP

// CHECK: nop; nop; msac.u16 rhu normalize_c V4, V5, V6, V7, S3, SP0; nop
// CHECK: nop; nop; msac.u16 rhu normalize_c V4, V5, V6, V7, S3, SP1; nop
// CHECK: nop; nop; msac.u16 rhu normalize_c V4, V5, V6, V7, S3, !SP1; nop
// CHECK: nop; nop; msac.u16 rhu normalize_c V4, V5, V6, V7, S3, VP1; nop
// CHECK: nop; nop; msac.u16 rhu normalize_c V4, V5, V6, V7, S3, !VP1; nop

NOP; NOP; MSAC.U16 V4, V5, V6, V7, 123, RHU, NORMALIZE_C;         NOP
NOP; NOP; MSAC.U16 V4, V5, V6, V7, 123, RHU, NORMALIZE_C, SP1;    NOP
NOP; NOP; MSAC.U16 V4, V5, V6, V7, 123, RHU, NORMALIZE_C, !SP1;   NOP
NOP; NOP; MSAC.U16 V4, V5, V6, V7, 123, RHU, NORMALIZE_C, VP1;    NOP
NOP; NOP; MSAC.U16 V4, V5, V6, V7, 123, RHU, NORMALIZE_C, !VP1;   NOP

// CHECK: nop; nop; msac.u16 rhu normalize_c V4, V5, V6, V7, 0x7b, SP0; nop
// CHECK: nop; nop; msac.u16 rhu normalize_c V4, V5, V6, V7, 0x7b, SP1; nop
// CHECK: nop; nop; msac.u16 rhu normalize_c V4, V5, V6, V7, 0x7b, !SP1; nop
// CHECK: nop; nop; msac.u16 rhu normalize_c V4, V5, V6, V7, 0x7b, VP1; nop
// CHECK: nop; nop; msac.u16 rhu normalize_c V4, V5, V6, V7, 0x7b, !VP1; nop

NOP; NOP; MSAC.U8 V4, V5, V6, V7, V8, RHU, NORMALIZE_C;         NOP
NOP; NOP; MSAC.U8 V4, V5, V6, V7, V8, RHU, NORMALIZE_C, SP1;    NOP
NOP; NOP; MSAC.U8 V4, V5, V6, V7, V8, RHU, NORMALIZE_C, !SP1;   NOP
NOP; NOP; MSAC.U8 V4, V5, V6, V7, V8, RHU, NORMALIZE_C, VP1;    NOP
NOP; NOP; MSAC.U8 V4, V5, V6, V7, V8, RHU, NORMALIZE_C, !VP1;   NOP

// CHECK: nop; nop; msac.u8 rhu normalize_c V4, V5, V6, V7, V8, SP0; nop
// CHECK: nop; nop; msac.u8 rhu normalize_c V4, V5, V6, V7, V8, SP1; nop
// CHECK: nop; nop; msac.u8 rhu normalize_c V4, V5, V6, V7, V8, !SP1; nop
// CHECK: nop; nop; msac.u8 rhu normalize_c V4, V5, V6, V7, V8, VP1; nop
// CHECK: nop; nop; msac.u8 rhu normalize_c V4, V5, V6, V7, V8, !VP1; nop

NOP; NOP; MSAC.U8 V4, V5, S3, V7, V8, RHU, NORMALIZE_C;         NOP
NOP; NOP; MSAC.U8 V4, V5, S3, V7, V8, RHU, NORMALIZE_C, SP1;    NOP
NOP; NOP; MSAC.U8 V4, V5, S3, V7, V8, RHU, NORMALIZE_C, !SP1;   NOP
NOP; NOP; MSAC.U8 V4, V5, S3, V7, V8, RHU, NORMALIZE_C, VP1;    NOP
NOP; NOP; MSAC.U8 V4, V5, S3, V7, V8, RHU, NORMALIZE_C, !VP1;   NOP

// CHECK: nop; nop; msac.u8 rhu normalize_c V4, V5, S3, V7, V8, SP0; nop
// CHECK: nop; nop; msac.u8 rhu normalize_c V4, V5, S3, V7, V8, SP1; nop
// CHECK: nop; nop; msac.u8 rhu normalize_c V4, V5, S3, V7, V8, !SP1; nop
// CHECK: nop; nop; msac.u8 rhu normalize_c V4, V5, S3, V7, V8, VP1; nop
// CHECK: nop; nop; msac.u8 rhu normalize_c V4, V5, S3, V7, V8, !VP1; nop

NOP; NOP; MSAC.U8 V4, V5, 122, V7, V8, RHU, NORMALIZE_C;         NOP
NOP; NOP; MSAC.U8 V4, V5, 122, V7, V8, RHU, NORMALIZE_C, SP1;    NOP
NOP; NOP; MSAC.U8 V4, V5, 122, V7, V8, RHU, NORMALIZE_C, !SP1;   NOP
NOP; NOP; MSAC.U8 V4, V5, 122, V7, V8, RHU, NORMALIZE_C, VP1;    NOP
NOP; NOP; MSAC.U8 V4, V5, 122, V7, V8, RHU, NORMALIZE_C, !VP1;   NOP

// CHECK: nop; nop; msac.u8 rhu normalize_c V4, V5, 0x7a, V7, V8, SP0; nop
// CHECK: nop; nop; msac.u8 rhu normalize_c V4, V5, 0x7a, V7, V8, SP1; nop
// CHECK: nop; nop; msac.u8 rhu normalize_c V4, V5, 0x7a, V7, V8, !SP1; nop
// CHECK: nop; nop; msac.u8 rhu normalize_c V4, V5, 0x7a, V7, V8, VP1; nop
// CHECK: nop; nop; msac.u8 rhu normalize_c V4, V5, 0x7a, V7, V8, !VP1; nop

NOP; NOP; MSAC.U8 V4, V5, V6, V7, S3, RHU, NORMALIZE_C;         NOP
NOP; NOP; MSAC.U8 V4, V5, V6, V7, S3, RHU, NORMALIZE_C, SP1;    NOP
NOP; NOP; MSAC.U8 V4, V5, V6, V7, S3, RHU, NORMALIZE_C, !SP1;   NOP
NOP; NOP; MSAC.U8 V4, V5, V6, V7, S3, RHU, NORMALIZE_C, VP1;    NOP
NOP; NOP; MSAC.U8 V4, V5, V6, V7, S3, RHU, NORMALIZE_C, !VP1;   NOP

// CHECK: nop; nop; msac.u8 rhu normalize_c V4, V5, V6, V7, S3, SP0; nop
// CHECK: nop; nop; msac.u8 rhu normalize_c V4, V5, V6, V7, S3, SP1; nop
// CHECK: nop; nop; msac.u8 rhu normalize_c V4, V5, V6, V7, S3, !SP1; nop
// CHECK: nop; nop; msac.u8 rhu normalize_c V4, V5, V6, V7, S3, VP1; nop
// CHECK: nop; nop; msac.u8 rhu normalize_c V4, V5, V6, V7, S3, !VP1; nop

NOP; NOP; MSAC.U8 V4, V5, V6, V7, 123, RHU, NORMALIZE_C;         NOP
NOP; NOP; MSAC.U8 V4, V5, V6, V7, 123, RHU, NORMALIZE_C, SP1;    NOP
NOP; NOP; MSAC.U8 V4, V5, V6, V7, 123, RHU, NORMALIZE_C, !SP1;   NOP
NOP; NOP; MSAC.U8 V4, V5, V6, V7, 123, RHU, NORMALIZE_C, VP1;    NOP
NOP; NOP; MSAC.U8 V4, V5, V6, V7, 123, RHU, NORMALIZE_C, !VP1;   NOP

// CHECK: nop; nop; msac.u8 rhu normalize_c V4, V5, V6, V7, 0x7b, SP0; nop
// CHECK: nop; nop; msac.u8 rhu normalize_c V4, V5, V6, V7, 0x7b, SP1; nop
// CHECK: nop; nop; msac.u8 rhu normalize_c V4, V5, V6, V7, 0x7b, !SP1; nop
// CHECK: nop; nop; msac.u8 rhu normalize_c V4, V5, V6, V7, 0x7b, VP1; nop
// CHECK: nop; nop; msac.u8 rhu normalize_c V4, V5, V6, V7, 0x7b, !VP1; nop