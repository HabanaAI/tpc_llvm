// RUN: %clang -cc1as -triple tpc-none-none -filetype obj %s -o %t.o
// RUN: %disasm %t.o | FileCheck %s

NOP; NOP; SEL2_GRT.I32 D4, V5, V6, V7, V8;         NOP
NOP; NOP; SEL2_GRT.I32 D4, V5, V6, V7, V8, SP1;    NOP
NOP; NOP; SEL2_GRT.I32 D4, V5, V6, V7, V8, !SP1;   NOP
NOP; NOP; SEL2_GRT.I32 D4, V5, V6, V7, V8, VP1;    NOP
NOP; NOP; SEL2_GRT.I32 D4, V5, V6, V7, V8, !VP1;   NOP

// CHECK: 	nop; 	nop; 	sel2_grt.i32  D4, V5, V6, V7, V8, SP0; 	nop
// CHECK: 	nop; 	nop; 	sel2_grt.i32  D4, V5, V6, V7, V8, SP1; 	nop
// CHECK: 	nop; 	nop; 	sel2_grt.i32  D4, V5, V6, V7, V8, !SP1; 	nop
// CHECK: 	nop; 	nop; 	sel2_grt.i32  D4, V5, V6, V7, V8, VP1; 	nop
// CHECK: 	nop; 	nop; 	sel2_grt.i32  D4, V5, V6, V7, V8, !VP1; 	nop


NOP; NOP; SEL2_GRT.I32 D4, V5, S3, V7, V8;         NOP
NOP; NOP; SEL2_GRT.I32 D4, V5, S3, V7, V8, SP1;    NOP
NOP; NOP; SEL2_GRT.I32 D4, V5, S3, V7, V8, !SP1;   NOP
NOP; NOP; SEL2_GRT.I32 D4, V5, S3, V7, V8, VP1;    NOP
NOP; NOP; SEL2_GRT.I32 D4, V5, S3, V7, V8, !VP1;   NOP

// CHECK: 	nop; 	nop; 	sel2_grt.i32  D4, V5, S3, V7, V8, SP0; 	nop
// CHECK: 	nop; 	nop; 	sel2_grt.i32  D4, V5, S3, V7, V8, SP1; 	nop
// CHECK: 	nop; 	nop; 	sel2_grt.i32  D4, V5, S3, V7, V8, !SP1; 	nop
// CHECK: 	nop; 	nop; 	sel2_grt.i32  D4, V5, S3, V7, V8, VP1; 	nop
// CHECK: 	nop; 	nop; 	sel2_grt.i32  D4, V5, S3, V7, V8, !VP1; 	nop


NOP; NOP; SEL2_GRT.I32 D4, V5, 122, V7, V8;         NOP
NOP; NOP; SEL2_GRT.I32 D4, V5, 122, V7, V8, SP1;    NOP
NOP; NOP; SEL2_GRT.I32 D4, V5, 122, V7, V8, !SP1;   NOP
NOP; NOP; SEL2_GRT.I32 D4, V5, 122, V7, V8, VP1;    NOP
NOP; NOP; SEL2_GRT.I32 D4, V5, 122, V7, V8, !VP1;   NOP

// CHECK: 	nop; 	nop; 	sel2_grt.i32  D4, V5, 0x7a, V7, V8, SP0; 	nop
// CHECK: 	nop; 	nop; 	sel2_grt.i32  D4, V5, 0x7a, V7, V8, SP1; 	nop
// CHECK: 	nop; 	nop; 	sel2_grt.i32  D4, V5, 0x7a, V7, V8, !SP1; 	nop
// CHECK: 	nop; 	nop; 	sel2_grt.i32  D4, V5, 0x7a, V7, V8, VP1; 	nop
// CHECK: 	nop; 	nop; 	sel2_grt.i32  D4, V5, 0x7a, V7, V8, !VP1; 	nop


NOP; NOP; SEL2_GRT.I32 D4, V5, V6, V7, S3;         NOP
NOP; NOP; SEL2_GRT.I32 D4, V5, V6, V7, S3, SP1;    NOP
NOP; NOP; SEL2_GRT.I32 D4, V5, V6, V7, S3, !SP1;   NOP
NOP; NOP; SEL2_GRT.I32 D4, V5, V6, V7, S3, VP1;    NOP
NOP; NOP; SEL2_GRT.I32 D4, V5, V6, V7, S3, !VP1;   NOP

// CHECK: 	nop; 	nop; 	sel2_grt.i32  D4, V5, V6, V7, S3, SP0; 	nop
// CHECK: 	nop; 	nop; 	sel2_grt.i32  D4, V5, V6, V7, S3, SP1; 	nop
// CHECK: 	nop; 	nop; 	sel2_grt.i32  D4, V5, V6, V7, S3, !SP1; 	nop
// CHECK: 	nop; 	nop; 	sel2_grt.i32  D4, V5, V6, V7, S3, VP1; 	nop
// CHECK: 	nop; 	nop; 	sel2_grt.i32  D4, V5, V6, V7, S3, !VP1; 	nop


NOP; NOP; SEL2_GRT.I32 D4, V5, V6, V7, 123;         NOP
NOP; NOP; SEL2_GRT.I32 D4, V5, V6, V7, 123, SP1;    NOP
NOP; NOP; SEL2_GRT.I32 D4, V5, V6, V7, 123, !SP1;   NOP
NOP; NOP; SEL2_GRT.I32 D4, V5, V6, V7, 123, VP1;    NOP
NOP; NOP; SEL2_GRT.I32 D4, V5, V6, V7, 123, !VP1;   NOP

// CHECK: 	nop; 	nop; 	sel2_grt.i32  D4, V5, V6, V7, 0x7b, SP0; 	nop
// CHECK: 	nop; 	nop; 	sel2_grt.i32  D4, V5, V6, V7, 0x7b, SP1; 	nop
// CHECK: 	nop; 	nop; 	sel2_grt.i32  D4, V5, V6, V7, 0x7b, !SP1; 	nop
// CHECK: 	nop; 	nop; 	sel2_grt.i32  D4, V5, V6, V7, 0x7b, VP1; 	nop
// CHECK: 	nop; 	nop; 	sel2_grt.i32  D4, V5, V6, V7, 0x7b, !VP1; 	nop


NOP; NOP; SEL2_GRT.I16 D4, V5, V6, V7, V8;         NOP
NOP; NOP; SEL2_GRT.I16 D4, V5, V6, V7, V8, SP1;    NOP
NOP; NOP; SEL2_GRT.I16 D4, V5, V6, V7, V8, !SP1;   NOP
NOP; NOP; SEL2_GRT.I16 D4, V5, V6, V7, V8, VP1;    NOP
NOP; NOP; SEL2_GRT.I16 D4, V5, V6, V7, V8, !VP1;   NOP

// CHECK: 	nop; 	nop; 	sel2_grt.i16  D4, V5, V6, V7, V8, SP0; 	nop
// CHECK: 	nop; 	nop; 	sel2_grt.i16  D4, V5, V6, V7, V8, SP1; 	nop
// CHECK: 	nop; 	nop; 	sel2_grt.i16  D4, V5, V6, V7, V8, !SP1; 	nop
// CHECK: 	nop; 	nop; 	sel2_grt.i16  D4, V5, V6, V7, V8, VP1; 	nop
// CHECK: 	nop; 	nop; 	sel2_grt.i16  D4, V5, V6, V7, V8, !VP1; 	nop


NOP; NOP; SEL2_GRT.I16 D4, V5, S3, V7, V8;         NOP
NOP; NOP; SEL2_GRT.I16 D4, V5, S3, V7, V8, SP1;    NOP
NOP; NOP; SEL2_GRT.I16 D4, V5, S3, V7, V8, !SP1;   NOP
NOP; NOP; SEL2_GRT.I16 D4, V5, S3, V7, V8, VP1;    NOP
NOP; NOP; SEL2_GRT.I16 D4, V5, S3, V7, V8, !VP1;   NOP

// CHECK: 	nop; 	nop; 	sel2_grt.i16  D4, V5, S3, V7, V8, SP0; 	nop
// CHECK: 	nop; 	nop; 	sel2_grt.i16  D4, V5, S3, V7, V8, SP1; 	nop
// CHECK: 	nop; 	nop; 	sel2_grt.i16  D4, V5, S3, V7, V8, !SP1; 	nop
// CHECK: 	nop; 	nop; 	sel2_grt.i16  D4, V5, S3, V7, V8, VP1; 	nop
// CHECK: 	nop; 	nop; 	sel2_grt.i16  D4, V5, S3, V7, V8, !VP1; 	nop


NOP; NOP; SEL2_GRT.I16 D4, V5, 122, V7, V8;         NOP
NOP; NOP; SEL2_GRT.I16 D4, V5, 122, V7, V8, SP1;    NOP
NOP; NOP; SEL2_GRT.I16 D4, V5, 122, V7, V8, !SP1;   NOP
NOP; NOP; SEL2_GRT.I16 D4, V5, 122, V7, V8, VP1;    NOP
NOP; NOP; SEL2_GRT.I16 D4, V5, 122, V7, V8, !VP1;   NOP

// CHECK: 	nop; 	nop; 	sel2_grt.i16  D4, V5, 0x7a, V7, V8, SP0; 	nop
// CHECK: 	nop; 	nop; 	sel2_grt.i16  D4, V5, 0x7a, V7, V8, SP1; 	nop
// CHECK: 	nop; 	nop; 	sel2_grt.i16  D4, V5, 0x7a, V7, V8, !SP1; 	nop
// CHECK: 	nop; 	nop; 	sel2_grt.i16  D4, V5, 0x7a, V7, V8, VP1; 	nop
// CHECK: 	nop; 	nop; 	sel2_grt.i16  D4, V5, 0x7a, V7, V8, !VP1; 	nop


NOP; NOP; SEL2_GRT.I16 D4, V5, V6, V7, S3;         NOP
NOP; NOP; SEL2_GRT.I16 D4, V5, V6, V7, S3, SP1;    NOP
NOP; NOP; SEL2_GRT.I16 D4, V5, V6, V7, S3, !SP1;   NOP
NOP; NOP; SEL2_GRT.I16 D4, V5, V6, V7, S3, VP1;    NOP
NOP; NOP; SEL2_GRT.I16 D4, V5, V6, V7, S3, !VP1;   NOP

// CHECK: 	nop; 	nop; 	sel2_grt.i16  D4, V5, V6, V7, S3, SP0; 	nop
// CHECK: 	nop; 	nop; 	sel2_grt.i16  D4, V5, V6, V7, S3, SP1; 	nop
// CHECK: 	nop; 	nop; 	sel2_grt.i16  D4, V5, V6, V7, S3, !SP1; 	nop
// CHECK: 	nop; 	nop; 	sel2_grt.i16  D4, V5, V6, V7, S3, VP1; 	nop
// CHECK: 	nop; 	nop; 	sel2_grt.i16  D4, V5, V6, V7, S3, !VP1; 	nop


NOP; NOP; SEL2_GRT.I16 D4, V5, V6, V7, 123;         NOP
NOP; NOP; SEL2_GRT.I16 D4, V5, V6, V7, 123, SP1;    NOP
NOP; NOP; SEL2_GRT.I16 D4, V5, V6, V7, 123, !SP1;   NOP
NOP; NOP; SEL2_GRT.I16 D4, V5, V6, V7, 123, VP1;    NOP
NOP; NOP; SEL2_GRT.I16 D4, V5, V6, V7, 123, !VP1;   NOP

// CHECK: 	nop; 	nop; 	sel2_grt.i16  D4, V5, V6, V7, 0x7b, SP0; 	nop
// CHECK: 	nop; 	nop; 	sel2_grt.i16  D4, V5, V6, V7, 0x7b, SP1; 	nop
// CHECK: 	nop; 	nop; 	sel2_grt.i16  D4, V5, V6, V7, 0x7b, !SP1; 	nop
// CHECK: 	nop; 	nop; 	sel2_grt.i16  D4, V5, V6, V7, 0x7b, VP1; 	nop
// CHECK: 	nop; 	nop; 	sel2_grt.i16  D4, V5, V6, V7, 0x7b, !VP1; 	nop


NOP; NOP; SEL2_GRT.I8 D4, V5, V6, V7, V8;         NOP
NOP; NOP; SEL2_GRT.I8 D4, V5, V6, V7, V8, SP1;    NOP
NOP; NOP; SEL2_GRT.I8 D4, V5, V6, V7, V8, !SP1;   NOP
NOP; NOP; SEL2_GRT.I8 D4, V5, V6, V7, V8, VP1;    NOP
NOP; NOP; SEL2_GRT.I8 D4, V5, V6, V7, V8, !VP1;   NOP

// CHECK: 	nop; 	nop; 	sel2_grt.i8  D4, V5, V6, V7, V8, SP0; 	nop
// CHECK: 	nop; 	nop; 	sel2_grt.i8  D4, V5, V6, V7, V8, SP1; 	nop
// CHECK: 	nop; 	nop; 	sel2_grt.i8  D4, V5, V6, V7, V8, !SP1; 	nop
// CHECK: 	nop; 	nop; 	sel2_grt.i8  D4, V5, V6, V7, V8, VP1; 	nop
// CHECK: 	nop; 	nop; 	sel2_grt.i8  D4, V5, V6, V7, V8, !VP1; 	nop


NOP; NOP; SEL2_GRT.I8 D4, V5, S3, V7, V8;         NOP
NOP; NOP; SEL2_GRT.I8 D4, V5, S3, V7, V8, SP1;    NOP
NOP; NOP; SEL2_GRT.I8 D4, V5, S3, V7, V8, !SP1;   NOP
NOP; NOP; SEL2_GRT.I8 D4, V5, S3, V7, V8, VP1;    NOP
NOP; NOP; SEL2_GRT.I8 D4, V5, S3, V7, V8, !VP1;   NOP

// CHECK: 	nop; 	nop; 	sel2_grt.i8  D4, V5, S3, V7, V8, SP0; 	nop
// CHECK: 	nop; 	nop; 	sel2_grt.i8  D4, V5, S3, V7, V8, SP1; 	nop
// CHECK: 	nop; 	nop; 	sel2_grt.i8  D4, V5, S3, V7, V8, !SP1; 	nop
// CHECK: 	nop; 	nop; 	sel2_grt.i8  D4, V5, S3, V7, V8, VP1; 	nop
// CHECK: 	nop; 	nop; 	sel2_grt.i8  D4, V5, S3, V7, V8, !VP1; 	nop


NOP; NOP; SEL2_GRT.I8 D4, V5, 122, V7, V8;         NOP
NOP; NOP; SEL2_GRT.I8 D4, V5, 122, V7, V8, SP1;    NOP
NOP; NOP; SEL2_GRT.I8 D4, V5, 122, V7, V8, !SP1;   NOP
NOP; NOP; SEL2_GRT.I8 D4, V5, 122, V7, V8, VP1;    NOP
NOP; NOP; SEL2_GRT.I8 D4, V5, 122, V7, V8, !VP1;   NOP

// CHECK: 	nop; 	nop; 	sel2_grt.i8  D4, V5, 0x7a, V7, V8, SP0; 	nop
// CHECK: 	nop; 	nop; 	sel2_grt.i8  D4, V5, 0x7a, V7, V8, SP1; 	nop
// CHECK: 	nop; 	nop; 	sel2_grt.i8  D4, V5, 0x7a, V7, V8, !SP1; 	nop
// CHECK: 	nop; 	nop; 	sel2_grt.i8  D4, V5, 0x7a, V7, V8, VP1; 	nop
// CHECK: 	nop; 	nop; 	sel2_grt.i8  D4, V5, 0x7a, V7, V8, !VP1; 	nop


NOP; NOP; SEL2_GRT.I8 D4, V5, V6, V7, S3;         NOP
NOP; NOP; SEL2_GRT.I8 D4, V5, V6, V7, S3, SP1;    NOP
NOP; NOP; SEL2_GRT.I8 D4, V5, V6, V7, S3, !SP1;   NOP
NOP; NOP; SEL2_GRT.I8 D4, V5, V6, V7, S3, VP1;    NOP
NOP; NOP; SEL2_GRT.I8 D4, V5, V6, V7, S3, !VP1;   NOP

// CHECK: 	nop; 	nop; 	sel2_grt.i8  D4, V5, V6, V7, S3, SP0; 	nop
// CHECK: 	nop; 	nop; 	sel2_grt.i8  D4, V5, V6, V7, S3, SP1; 	nop
// CHECK: 	nop; 	nop; 	sel2_grt.i8  D4, V5, V6, V7, S3, !SP1; 	nop
// CHECK: 	nop; 	nop; 	sel2_grt.i8  D4, V5, V6, V7, S3, VP1; 	nop
// CHECK: 	nop; 	nop; 	sel2_grt.i8  D4, V5, V6, V7, S3, !VP1; 	nop


NOP; NOP; SEL2_GRT.I8 D4, V5, V6, V7, 123;         NOP
NOP; NOP; SEL2_GRT.I8 D4, V5, V6, V7, 123, SP1;    NOP
NOP; NOP; SEL2_GRT.I8 D4, V5, V6, V7, 123, !SP1;   NOP
NOP; NOP; SEL2_GRT.I8 D4, V5, V6, V7, 123, VP1;    NOP
NOP; NOP; SEL2_GRT.I8 D4, V5, V6, V7, 123, !VP1;   NOP

// CHECK: 	nop; 	nop; 	sel2_grt.i8  D4, V5, V6, V7, 0x7b, SP0; 	nop
// CHECK: 	nop; 	nop; 	sel2_grt.i8  D4, V5, V6, V7, 0x7b, SP1; 	nop
// CHECK: 	nop; 	nop; 	sel2_grt.i8  D4, V5, V6, V7, 0x7b, !SP1; 	nop
// CHECK: 	nop; 	nop; 	sel2_grt.i8  D4, V5, V6, V7, 0x7b, VP1; 	nop
// CHECK: 	nop; 	nop; 	sel2_grt.i8  D4, V5, V6, V7, 0x7b, !VP1; 	nop


NOP; NOP; SEL2_GRT.U32 D4, V5, V6, V7, V8;         NOP
NOP; NOP; SEL2_GRT.U32 D4, V5, V6, V7, V8, SP1;    NOP
NOP; NOP; SEL2_GRT.U32 D4, V5, V6, V7, V8, !SP1;   NOP
NOP; NOP; SEL2_GRT.U32 D4, V5, V6, V7, V8, VP1;    NOP
NOP; NOP; SEL2_GRT.U32 D4, V5, V6, V7, V8, !VP1;   NOP

// CHECK: 	nop; 	nop; 	sel2_grt.u32  D4, V5, V6, V7, V8, SP0; 	nop
// CHECK: 	nop; 	nop; 	sel2_grt.u32  D4, V5, V6, V7, V8, SP1; 	nop
// CHECK: 	nop; 	nop; 	sel2_grt.u32  D4, V5, V6, V7, V8, !SP1; 	nop
// CHECK: 	nop; 	nop; 	sel2_grt.u32  D4, V5, V6, V7, V8, VP1; 	nop
// CHECK: 	nop; 	nop; 	sel2_grt.u32  D4, V5, V6, V7, V8, !VP1; 	nop


NOP; NOP; SEL2_GRT.U32 D4, V5, S3, V7, V8;         NOP
NOP; NOP; SEL2_GRT.U32 D4, V5, S3, V7, V8, SP1;    NOP
NOP; NOP; SEL2_GRT.U32 D4, V5, S3, V7, V8, !SP1;   NOP
NOP; NOP; SEL2_GRT.U32 D4, V5, S3, V7, V8, VP1;    NOP
NOP; NOP; SEL2_GRT.U32 D4, V5, S3, V7, V8, !VP1;   NOP

// CHECK: 	nop; 	nop; 	sel2_grt.u32  D4, V5, S3, V7, V8, SP0; 	nop
// CHECK: 	nop; 	nop; 	sel2_grt.u32  D4, V5, S3, V7, V8, SP1; 	nop
// CHECK: 	nop; 	nop; 	sel2_grt.u32  D4, V5, S3, V7, V8, !SP1; 	nop
// CHECK: 	nop; 	nop; 	sel2_grt.u32  D4, V5, S3, V7, V8, VP1; 	nop
// CHECK: 	nop; 	nop; 	sel2_grt.u32  D4, V5, S3, V7, V8, !VP1; 	nop


NOP; NOP; SEL2_GRT.U32 D4, V5, 122, V7, V8;         NOP
NOP; NOP; SEL2_GRT.U32 D4, V5, 122, V7, V8, SP1;    NOP
NOP; NOP; SEL2_GRT.U32 D4, V5, 122, V7, V8, !SP1;   NOP
NOP; NOP; SEL2_GRT.U32 D4, V5, 122, V7, V8, VP1;    NOP
NOP; NOP; SEL2_GRT.U32 D4, V5, 122, V7, V8, !VP1;   NOP

// CHECK: 	nop; 	nop; 	sel2_grt.u32  D4, V5, 0x7a, V7, V8, SP0; 	nop
// CHECK: 	nop; 	nop; 	sel2_grt.u32  D4, V5, 0x7a, V7, V8, SP1; 	nop
// CHECK: 	nop; 	nop; 	sel2_grt.u32  D4, V5, 0x7a, V7, V8, !SP1; 	nop
// CHECK: 	nop; 	nop; 	sel2_grt.u32  D4, V5, 0x7a, V7, V8, VP1; 	nop
// CHECK: 	nop; 	nop; 	sel2_grt.u32  D4, V5, 0x7a, V7, V8, !VP1; 	nop


NOP; NOP; SEL2_GRT.U32 D4, V5, V6, V7, S3;         NOP
NOP; NOP; SEL2_GRT.U32 D4, V5, V6, V7, S3, SP1;    NOP
NOP; NOP; SEL2_GRT.U32 D4, V5, V6, V7, S3, !SP1;   NOP
NOP; NOP; SEL2_GRT.U32 D4, V5, V6, V7, S3, VP1;    NOP
NOP; NOP; SEL2_GRT.U32 D4, V5, V6, V7, S3, !VP1;   NOP

// CHECK: 	nop; 	nop; 	sel2_grt.u32  D4, V5, V6, V7, S3, SP0; 	nop
// CHECK: 	nop; 	nop; 	sel2_grt.u32  D4, V5, V6, V7, S3, SP1; 	nop
// CHECK: 	nop; 	nop; 	sel2_grt.u32  D4, V5, V6, V7, S3, !SP1; 	nop
// CHECK: 	nop; 	nop; 	sel2_grt.u32  D4, V5, V6, V7, S3, VP1; 	nop
// CHECK: 	nop; 	nop; 	sel2_grt.u32  D4, V5, V6, V7, S3, !VP1; 	nop


NOP; NOP; SEL2_GRT.U32 D4, V5, V6, V7, 123;         NOP
NOP; NOP; SEL2_GRT.U32 D4, V5, V6, V7, 123, SP1;    NOP
NOP; NOP; SEL2_GRT.U32 D4, V5, V6, V7, 123, !SP1;   NOP
NOP; NOP; SEL2_GRT.U32 D4, V5, V6, V7, 123, VP1;    NOP
NOP; NOP; SEL2_GRT.U32 D4, V5, V6, V7, 123, !VP1;   NOP

// CHECK: 	nop; 	nop; 	sel2_grt.u32  D4, V5, V6, V7, 0x7b, SP0; 	nop
// CHECK: 	nop; 	nop; 	sel2_grt.u32  D4, V5, V6, V7, 0x7b, SP1; 	nop
// CHECK: 	nop; 	nop; 	sel2_grt.u32  D4, V5, V6, V7, 0x7b, !SP1; 	nop
// CHECK: 	nop; 	nop; 	sel2_grt.u32  D4, V5, V6, V7, 0x7b, VP1; 	nop
// CHECK: 	nop; 	nop; 	sel2_grt.u32  D4, V5, V6, V7, 0x7b, !VP1; 	nop


NOP; NOP; SEL2_GRT.U16 D4, V5, V6, V7, V8;         NOP
NOP; NOP; SEL2_GRT.U16 D4, V5, V6, V7, V8, SP1;    NOP
NOP; NOP; SEL2_GRT.U16 D4, V5, V6, V7, V8, !SP1;   NOP
NOP; NOP; SEL2_GRT.U16 D4, V5, V6, V7, V8, VP1;    NOP
NOP; NOP; SEL2_GRT.U16 D4, V5, V6, V7, V8, !VP1;   NOP

// CHECK: 	nop; 	nop; 	sel2_grt.u16  D4, V5, V6, V7, V8, SP0; 	nop
// CHECK: 	nop; 	nop; 	sel2_grt.u16  D4, V5, V6, V7, V8, SP1; 	nop
// CHECK: 	nop; 	nop; 	sel2_grt.u16  D4, V5, V6, V7, V8, !SP1; 	nop
// CHECK: 	nop; 	nop; 	sel2_grt.u16  D4, V5, V6, V7, V8, VP1; 	nop
// CHECK: 	nop; 	nop; 	sel2_grt.u16  D4, V5, V6, V7, V8, !VP1; 	nop


NOP; NOP; SEL2_GRT.U16 D4, V5, S3, V7, V8;         NOP
NOP; NOP; SEL2_GRT.U16 D4, V5, S3, V7, V8, SP1;    NOP
NOP; NOP; SEL2_GRT.U16 D4, V5, S3, V7, V8, !SP1;   NOP
NOP; NOP; SEL2_GRT.U16 D4, V5, S3, V7, V8, VP1;    NOP
NOP; NOP; SEL2_GRT.U16 D4, V5, S3, V7, V8, !VP1;   NOP

// CHECK: 	nop; 	nop; 	sel2_grt.u16  D4, V5, S3, V7, V8, SP0; 	nop
// CHECK: 	nop; 	nop; 	sel2_grt.u16  D4, V5, S3, V7, V8, SP1; 	nop
// CHECK: 	nop; 	nop; 	sel2_grt.u16  D4, V5, S3, V7, V8, !SP1; 	nop
// CHECK: 	nop; 	nop; 	sel2_grt.u16  D4, V5, S3, V7, V8, VP1; 	nop
// CHECK: 	nop; 	nop; 	sel2_grt.u16  D4, V5, S3, V7, V8, !VP1; 	nop


NOP; NOP; SEL2_GRT.U16 D4, V5, 122, V7, V8;         NOP
NOP; NOP; SEL2_GRT.U16 D4, V5, 122, V7, V8, SP1;    NOP
NOP; NOP; SEL2_GRT.U16 D4, V5, 122, V7, V8, !SP1;   NOP
NOP; NOP; SEL2_GRT.U16 D4, V5, 122, V7, V8, VP1;    NOP
NOP; NOP; SEL2_GRT.U16 D4, V5, 122, V7, V8, !VP1;   NOP

// CHECK: 	nop; 	nop; 	sel2_grt.u16  D4, V5, 0x7a, V7, V8, SP0; 	nop
// CHECK: 	nop; 	nop; 	sel2_grt.u16  D4, V5, 0x7a, V7, V8, SP1; 	nop
// CHECK: 	nop; 	nop; 	sel2_grt.u16  D4, V5, 0x7a, V7, V8, !SP1; 	nop
// CHECK: 	nop; 	nop; 	sel2_grt.u16  D4, V5, 0x7a, V7, V8, VP1; 	nop
// CHECK: 	nop; 	nop; 	sel2_grt.u16  D4, V5, 0x7a, V7, V8, !VP1; 	nop


NOP; NOP; SEL2_GRT.U16 D4, V5, V6, V7, S3;         NOP
NOP; NOP; SEL2_GRT.U16 D4, V5, V6, V7, S3, SP1;    NOP
NOP; NOP; SEL2_GRT.U16 D4, V5, V6, V7, S3, !SP1;   NOP
NOP; NOP; SEL2_GRT.U16 D4, V5, V6, V7, S3, VP1;    NOP
NOP; NOP; SEL2_GRT.U16 D4, V5, V6, V7, S3, !VP1;   NOP

// CHECK: 	nop; 	nop; 	sel2_grt.u16  D4, V5, V6, V7, S3, SP0; 	nop
// CHECK: 	nop; 	nop; 	sel2_grt.u16  D4, V5, V6, V7, S3, SP1; 	nop
// CHECK: 	nop; 	nop; 	sel2_grt.u16  D4, V5, V6, V7, S3, !SP1; 	nop
// CHECK: 	nop; 	nop; 	sel2_grt.u16  D4, V5, V6, V7, S3, VP1; 	nop
// CHECK: 	nop; 	nop; 	sel2_grt.u16  D4, V5, V6, V7, S3, !VP1; 	nop


NOP; NOP; SEL2_GRT.U16 D4, V5, V6, V7, 123;         NOP
NOP; NOP; SEL2_GRT.U16 D4, V5, V6, V7, 123, SP1;    NOP
NOP; NOP; SEL2_GRT.U16 D4, V5, V6, V7, 123, !SP1;   NOP
NOP; NOP; SEL2_GRT.U16 D4, V5, V6, V7, 123, VP1;    NOP
NOP; NOP; SEL2_GRT.U16 D4, V5, V6, V7, 123, !VP1;   NOP

// CHECK: 	nop; 	nop; 	sel2_grt.u16  D4, V5, V6, V7, 0x7b, SP0; 	nop
// CHECK: 	nop; 	nop; 	sel2_grt.u16  D4, V5, V6, V7, 0x7b, SP1; 	nop
// CHECK: 	nop; 	nop; 	sel2_grt.u16  D4, V5, V6, V7, 0x7b, !SP1; 	nop
// CHECK: 	nop; 	nop; 	sel2_grt.u16  D4, V5, V6, V7, 0x7b, VP1; 	nop
// CHECK: 	nop; 	nop; 	sel2_grt.u16  D4, V5, V6, V7, 0x7b, !VP1; 	nop


NOP; NOP; SEL2_GRT.U8 D4, V5, V6, V7, V8;         NOP
NOP; NOP; SEL2_GRT.U8 D4, V5, V6, V7, V8, SP1;    NOP
NOP; NOP; SEL2_GRT.U8 D4, V5, V6, V7, V8, !SP1;   NOP
NOP; NOP; SEL2_GRT.U8 D4, V5, V6, V7, V8, VP1;    NOP
NOP; NOP; SEL2_GRT.U8 D4, V5, V6, V7, V8, !VP1;   NOP

// CHECK: 	nop; 	nop; 	sel2_grt.u8  D4, V5, V6, V7, V8, SP0; 	nop
// CHECK: 	nop; 	nop; 	sel2_grt.u8  D4, V5, V6, V7, V8, SP1; 	nop
// CHECK: 	nop; 	nop; 	sel2_grt.u8  D4, V5, V6, V7, V8, !SP1; 	nop
// CHECK: 	nop; 	nop; 	sel2_grt.u8  D4, V5, V6, V7, V8, VP1; 	nop
// CHECK: 	nop; 	nop; 	sel2_grt.u8  D4, V5, V6, V7, V8, !VP1; 	nop


NOP; NOP; SEL2_GRT.U8 D4, V5, S3, V7, V8;         NOP
NOP; NOP; SEL2_GRT.U8 D4, V5, S3, V7, V8, SP1;    NOP
NOP; NOP; SEL2_GRT.U8 D4, V5, S3, V7, V8, !SP1;   NOP
NOP; NOP; SEL2_GRT.U8 D4, V5, S3, V7, V8, VP1;    NOP
NOP; NOP; SEL2_GRT.U8 D4, V5, S3, V7, V8, !VP1;   NOP

// CHECK: 	nop; 	nop; 	sel2_grt.u8  D4, V5, S3, V7, V8, SP0; 	nop
// CHECK: 	nop; 	nop; 	sel2_grt.u8  D4, V5, S3, V7, V8, SP1; 	nop
// CHECK: 	nop; 	nop; 	sel2_grt.u8  D4, V5, S3, V7, V8, !SP1; 	nop
// CHECK: 	nop; 	nop; 	sel2_grt.u8  D4, V5, S3, V7, V8, VP1; 	nop
// CHECK: 	nop; 	nop; 	sel2_grt.u8  D4, V5, S3, V7, V8, !VP1; 	nop


NOP; NOP; SEL2_GRT.U8 D4, V5, 122, V7, V8;         NOP
NOP; NOP; SEL2_GRT.U8 D4, V5, 122, V7, V8, SP1;    NOP
NOP; NOP; SEL2_GRT.U8 D4, V5, 122, V7, V8, !SP1;   NOP
NOP; NOP; SEL2_GRT.U8 D4, V5, 122, V7, V8, VP1;    NOP
NOP; NOP; SEL2_GRT.U8 D4, V5, 122, V7, V8, !VP1;   NOP

// CHECK: 	nop; 	nop; 	sel2_grt.u8  D4, V5, 0x7a, V7, V8, SP0; 	nop
// CHECK: 	nop; 	nop; 	sel2_grt.u8  D4, V5, 0x7a, V7, V8, SP1; 	nop
// CHECK: 	nop; 	nop; 	sel2_grt.u8  D4, V5, 0x7a, V7, V8, !SP1; 	nop
// CHECK: 	nop; 	nop; 	sel2_grt.u8  D4, V5, 0x7a, V7, V8, VP1; 	nop
// CHECK: 	nop; 	nop; 	sel2_grt.u8  D4, V5, 0x7a, V7, V8, !VP1; 	nop


NOP; NOP; SEL2_GRT.U8 D4, V5, V6, V7, S3;         NOP
NOP; NOP; SEL2_GRT.U8 D4, V5, V6, V7, S3, SP1;    NOP
NOP; NOP; SEL2_GRT.U8 D4, V5, V6, V7, S3, !SP1;   NOP
NOP; NOP; SEL2_GRT.U8 D4, V5, V6, V7, S3, VP1;    NOP
NOP; NOP; SEL2_GRT.U8 D4, V5, V6, V7, S3, !VP1;   NOP

// CHECK: 	nop; 	nop; 	sel2_grt.u8  D4, V5, V6, V7, S3, SP0; 	nop
// CHECK: 	nop; 	nop; 	sel2_grt.u8  D4, V5, V6, V7, S3, SP1; 	nop
// CHECK: 	nop; 	nop; 	sel2_grt.u8  D4, V5, V6, V7, S3, !SP1; 	nop
// CHECK: 	nop; 	nop; 	sel2_grt.u8  D4, V5, V6, V7, S3, VP1; 	nop
// CHECK: 	nop; 	nop; 	sel2_grt.u8  D4, V5, V6, V7, S3, !VP1; 	nop


NOP; NOP; SEL2_GRT.U8 D4, V5, V6, V7, 123;         NOP
NOP; NOP; SEL2_GRT.U8 D4, V5, V6, V7, 123, SP1;    NOP
NOP; NOP; SEL2_GRT.U8 D4, V5, V6, V7, 123, !SP1;   NOP
NOP; NOP; SEL2_GRT.U8 D4, V5, V6, V7, 123, VP1;    NOP
NOP; NOP; SEL2_GRT.U8 D4, V5, V6, V7, 123, !VP1;   NOP

// CHECK: 	nop; 	nop; 	sel2_grt.u8  D4, V5, V6, V7, 0x7b, SP0; 	nop
// CHECK: 	nop; 	nop; 	sel2_grt.u8  D4, V5, V6, V7, 0x7b, SP1; 	nop
// CHECK: 	nop; 	nop; 	sel2_grt.u8  D4, V5, V6, V7, 0x7b, !SP1; 	nop
// CHECK: 	nop; 	nop; 	sel2_grt.u8  D4, V5, V6, V7, 0x7b, VP1; 	nop
// CHECK: 	nop; 	nop; 	sel2_grt.u8  D4, V5, V6, V7, 0x7b, !VP1; 	nop


NOP; NOP; SEL2_GRT.F32 D4, V5, V6, V7, V8;         NOP
NOP; NOP; SEL2_GRT.F32 D4, V5, V6, V7, V8, SP1;    NOP
NOP; NOP; SEL2_GRT.F32 D4, V5, V6, V7, V8, !SP1;   NOP
NOP; NOP; SEL2_GRT.F32 D4, V5, V6, V7, V8, VP1;    NOP
NOP; NOP; SEL2_GRT.F32 D4, V5, V6, V7, V8, !VP1;   NOP

// CHECK: 	nop; 	nop; 	sel2_grt.f32  D4, V5, V6, V7, V8, SP0; 	nop
// CHECK: 	nop; 	nop; 	sel2_grt.f32  D4, V5, V6, V7, V8, SP1; 	nop
// CHECK: 	nop; 	nop; 	sel2_grt.f32  D4, V5, V6, V7, V8, !SP1; 	nop
// CHECK: 	nop; 	nop; 	sel2_grt.f32  D4, V5, V6, V7, V8, VP1; 	nop
// CHECK: 	nop; 	nop; 	sel2_grt.f32  D4, V5, V6, V7, V8, !VP1; 	nop


NOP; NOP; SEL2_GRT.F32 D4, V5, S3, V7, V8;         NOP
NOP; NOP; SEL2_GRT.F32 D4, V5, S3, V7, V8, SP1;    NOP
NOP; NOP; SEL2_GRT.F32 D4, V5, S3, V7, V8, !SP1;   NOP
NOP; NOP; SEL2_GRT.F32 D4, V5, S3, V7, V8, VP1;    NOP
NOP; NOP; SEL2_GRT.F32 D4, V5, S3, V7, V8, !VP1;   NOP

// CHECK: 	nop; 	nop; 	sel2_grt.f32  D4, V5, S3, V7, V8, SP0; 	nop
// CHECK: 	nop; 	nop; 	sel2_grt.f32  D4, V5, S3, V7, V8, SP1; 	nop
// CHECK: 	nop; 	nop; 	sel2_grt.f32  D4, V5, S3, V7, V8, !SP1; 	nop
// CHECK: 	nop; 	nop; 	sel2_grt.f32  D4, V5, S3, V7, V8, VP1; 	nop
// CHECK: 	nop; 	nop; 	sel2_grt.f32  D4, V5, S3, V7, V8, !VP1; 	nop


NOP; NOP; SEL2_GRT.F32 D4, V5, 0x3f800000, V7, V8;         NOP
NOP; NOP; SEL2_GRT.F32 D4, V5, 0x3f800000, V7, V8, SP1;    NOP
NOP; NOP; SEL2_GRT.F32 D4, V5, 0x3f800000, V7, V8, !SP1;   NOP
NOP; NOP; SEL2_GRT.F32 D4, V5, 0x3f800000, V7, V8, VP1;    NOP
NOP; NOP; SEL2_GRT.F32 D4, V5, 0x3f800000, V7, V8, !VP1;   NOP

// CHECK: 	nop; 	nop; 	sel2_grt.f32  D4, V5, 0x3f800000, V7, V8, SP0; 	nop
// CHECK: 	nop; 	nop; 	sel2_grt.f32  D4, V5, 0x3f800000, V7, V8, SP1; 	nop
// CHECK: 	nop; 	nop; 	sel2_grt.f32  D4, V5, 0x3f800000, V7, V8, !SP1; 	nop
// CHECK: 	nop; 	nop; 	sel2_grt.f32  D4, V5, 0x3f800000, V7, V8, VP1; 	nop
// CHECK: 	nop; 	nop; 	sel2_grt.f32  D4, V5, 0x3f800000, V7, V8, !VP1; 	nop


NOP; NOP; SEL2_GRT.F32 D4, V5, V6, V7, S3;         NOP
NOP; NOP; SEL2_GRT.F32 D4, V5, V6, V7, S3, SP1;    NOP
NOP; NOP; SEL2_GRT.F32 D4, V5, V6, V7, S3, !SP1;   NOP
NOP; NOP; SEL2_GRT.F32 D4, V5, V6, V7, S3, VP1;    NOP
NOP; NOP; SEL2_GRT.F32 D4, V5, V6, V7, S3, !VP1;   NOP

// CHECK: 	nop; 	nop; 	sel2_grt.f32  D4, V5, V6, V7, S3, SP0; 	nop
// CHECK: 	nop; 	nop; 	sel2_grt.f32  D4, V5, V6, V7, S3, SP1; 	nop
// CHECK: 	nop; 	nop; 	sel2_grt.f32  D4, V5, V6, V7, S3, !SP1; 	nop
// CHECK: 	nop; 	nop; 	sel2_grt.f32  D4, V5, V6, V7, S3, VP1; 	nop
// CHECK: 	nop; 	nop; 	sel2_grt.f32  D4, V5, V6, V7, S3, !VP1; 	nop


NOP; NOP; SEL2_GRT.F32 D4, V5, V6, V7, 0x3f800000;         NOP
NOP; NOP; SEL2_GRT.F32 D4, V5, V6, V7, 0x3f800000, SP1;    NOP
NOP; NOP; SEL2_GRT.F32 D4, V5, V6, V7, 0x3f800000, !SP1;   NOP
NOP; NOP; SEL2_GRT.F32 D4, V5, V6, V7, 0x3f800000, VP1;    NOP
NOP; NOP; SEL2_GRT.F32 D4, V5, V6, V7, 0x3f800000, !VP1;   NOP

// CHECK: 	nop; 	nop; 	sel2_grt.f32  D4, V5, V6, V7, 0x3f800000, SP0; 	nop
// CHECK: 	nop; 	nop; 	sel2_grt.f32  D4, V5, V6, V7, 0x3f800000, SP1; 	nop
// CHECK: 	nop; 	nop; 	sel2_grt.f32  D4, V5, V6, V7, 0x3f800000, !SP1; 	nop
// CHECK: 	nop; 	nop; 	sel2_grt.f32  D4, V5, V6, V7, 0x3f800000, VP1; 	nop
// CHECK: 	nop; 	nop; 	sel2_grt.f32  D4, V5, V6, V7, 0x3f800000, !VP1; 	nop
