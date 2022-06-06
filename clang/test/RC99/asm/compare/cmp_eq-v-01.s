// RUN: %clang -cc1as -triple tpc-none-none -filetype obj %s -o %t.o
// RUN: %disasm %t.o | FileCheck %s
// RUN: %clang -cc1as -triple tpc-none-none -target-cpu gaudi -filetype obj %s -o %t.o
// RUN: %disasm --mcpu=gaudi %t.o | FileCheck %s
// RUN: %clang -cc1as -triple tpc-none-none -target-cpu goya2 -filetype obj %s -o %t.o
// RUN: %disasm --mcpu=goya2 %t.o | FileCheck %s


NOP; NOP; CMP_EQ.F32 VP1, V2, V3; NOP
NOP; NOP; CMP_EQ.F32 VP1, V2, V3, SP4; NOP
NOP; NOP; CMP_EQ.F32 VP1, V2, V3, !SP4; NOP
NOP; NOP; CMP_EQ.F32 VP1, V2, V3, VP4; NOP
NOP; NOP; CMP_EQ.F32 VP1, V2, V3, !VP4; NOP

// CHECK: nop; 	nop; 	cmp_eq.f32  VP1, V2, V3; 	nop
// CHECK: nop; 	nop; 	cmp_eq.f32  VP1, V2, V3, SP4; 	nop
// CHECK: nop; 	nop; 	cmp_eq.f32  VP1, V2, V3, !SP4; 	nop
// CHECK: nop; 	nop; 	cmp_eq.f32  VP1, V2, V3, VP4; 	nop
// CHECK: nop; 	nop; 	cmp_eq.f32  VP1, V2, V3, !VP4; 	nop


NOP; NOP; CMP_EQ.F32 VP1, V2, S3; NOP
NOP; NOP; CMP_EQ.F32 VP1, V2, S3, SP4; NOP
NOP; NOP; CMP_EQ.F32 VP1, V2, S3, !SP4; NOP
NOP; NOP; CMP_EQ.F32 VP1, V2, S3, VP4; NOP
NOP; NOP; CMP_EQ.F32 VP1, V2, S3, !VP4; NOP

// CHECK: nop; 	nop; 	cmp_eq.f32  VP1, V2, S3; 	nop
// CHECK: nop; 	nop; 	cmp_eq.f32  VP1, V2, S3, SP4; 	nop
// CHECK: nop; 	nop; 	cmp_eq.f32  VP1, V2, S3, !SP4; 	nop
// CHECK: nop; 	nop; 	cmp_eq.f32  VP1, V2, S3, VP4; 	nop
// CHECK: nop; 	nop; 	cmp_eq.f32  VP1, V2, S3, !VP4; 	nop


NOP; NOP; CMP_EQ.F32 VP1, V2, 0x3f4ccccd; NOP
NOP; NOP; CMP_EQ.F32 VP1, V2, 0x3f4ccccd, SP4; NOP
NOP; NOP; CMP_EQ.F32 VP1, V2, 0x3f4ccccd, !SP4; NOP
NOP; NOP; CMP_EQ.F32 VP1, V2, 0x3f4ccccd, VP4; NOP
NOP; NOP; CMP_EQ.F32 VP1, V2, 0x3f4ccccd, !VP4; NOP

// CHECK: nop; 	nop; 	cmp_eq.f32  VP1, V2, 0x3f4ccccd; 	nop
// CHECK: nop; 	nop; 	cmp_eq.f32  VP1, V2, 0x3f4ccccd, SP4; 	nop
// CHECK: nop; 	nop; 	cmp_eq.f32  VP1, V2, 0x3f4ccccd, !SP4; 	nop
// CHECK: nop; 	nop; 	cmp_eq.f32  VP1, V2, 0x3f4ccccd, VP4; 	nop
// CHECK: nop; 	nop; 	cmp_eq.f32  VP1, V2, 0x3f4ccccd, !VP4; 	nop


NOP; NOP; CMP_EQ.I32 VP1, V2, V3; NOP
NOP; NOP; CMP_EQ.I32 VP1, V2, V3, SP4; NOP
NOP; NOP; CMP_EQ.I32 VP1, V2, V3, !SP4; NOP
NOP; NOP; CMP_EQ.I32 VP1, V2, V3, VP4; NOP
NOP; NOP; CMP_EQ.I32 VP1, V2, V3, !VP4; NOP

// CHECK: nop; 	nop; 	cmp_eq.i32  VP1, V2, V3; 	nop
// CHECK: nop; 	nop; 	cmp_eq.i32  VP1, V2, V3, SP4; 	nop
// CHECK: nop; 	nop; 	cmp_eq.i32  VP1, V2, V3, !SP4; 	nop
// CHECK: nop; 	nop; 	cmp_eq.i32  VP1, V2, V3, VP4; 	nop
// CHECK: nop; 	nop; 	cmp_eq.i32  VP1, V2, V3, !VP4; 	nop


NOP; NOP; CMP_EQ.I32 VP1, V2, S3; NOP
NOP; NOP; CMP_EQ.I32 VP1, V2, S3, SP4; NOP
NOP; NOP; CMP_EQ.I32 VP1, V2, S3, !SP4; NOP
NOP; NOP; CMP_EQ.I32 VP1, V2, S3, VP4; NOP
NOP; NOP; CMP_EQ.I32 VP1, V2, S3, !VP4; NOP

// CHECK: nop; 	nop; 	cmp_eq.i32  VP1, V2, S3; 	nop
// CHECK: nop; 	nop; 	cmp_eq.i32  VP1, V2, S3, SP4; 	nop
// CHECK: nop; 	nop; 	cmp_eq.i32  VP1, V2, S3, !SP4; 	nop
// CHECK: nop; 	nop; 	cmp_eq.i32  VP1, V2, S3, VP4; 	nop
// CHECK: nop; 	nop; 	cmp_eq.i32  VP1, V2, S3, !VP4; 	nop


NOP; NOP; CMP_EQ.I32 VP1, V2, 123; NOP
NOP; NOP; CMP_EQ.I32 VP1, V2, 123, SP4; NOP
NOP; NOP; CMP_EQ.I32 VP1, V2, 123, !SP4; NOP
NOP; NOP; CMP_EQ.I32 VP1, V2, 123, VP4; NOP
NOP; NOP; CMP_EQ.I32 VP1, V2, 123, !VP4; NOP

// CHECK: nop; 	nop; 	cmp_eq.i32  VP1, V2, 0x7b; 	nop
// CHECK: nop; 	nop; 	cmp_eq.i32  VP1, V2, 0x7b, SP4; 	nop
// CHECK: nop; 	nop; 	cmp_eq.i32  VP1, V2, 0x7b, !SP4; 	nop
// CHECK: nop; 	nop; 	cmp_eq.i32  VP1, V2, 0x7b, VP4; 	nop
// CHECK: nop; 	nop; 	cmp_eq.i32  VP1, V2, 0x7b, !VP4; 	nop


NOP; NOP; CMP_EQ.U32 VP1, V2, V3; NOP
NOP; NOP; CMP_EQ.U32 VP1, V2, V3, SP4; NOP
NOP; NOP; CMP_EQ.U32 VP1, V2, V3, !SP4; NOP
NOP; NOP; CMP_EQ.U32 VP1, V2, V3, VP4; NOP
NOP; NOP; CMP_EQ.U32 VP1, V2, V3, !VP4; NOP

// CHECK: nop; 	nop; 	cmp_eq.u32  VP1, V2, V3; 	nop
// CHECK: nop; 	nop; 	cmp_eq.u32  VP1, V2, V3, SP4; 	nop
// CHECK: nop; 	nop; 	cmp_eq.u32  VP1, V2, V3, !SP4; 	nop
// CHECK: nop; 	nop; 	cmp_eq.u32  VP1, V2, V3, VP4; 	nop
// CHECK: nop; 	nop; 	cmp_eq.u32  VP1, V2, V3, !VP4; 	nop


NOP; NOP; CMP_EQ.U32 VP1, V2, S3; NOP
NOP; NOP; CMP_EQ.U32 VP1, V2, S3, SP4; NOP
NOP; NOP; CMP_EQ.U32 VP1, V2, S3, !SP4; NOP
NOP; NOP; CMP_EQ.U32 VP1, V2, S3, VP4; NOP
NOP; NOP; CMP_EQ.U32 VP1, V2, S3, !VP4; NOP

// CHECK: nop; 	nop; 	cmp_eq.u32  VP1, V2, S3; 	nop
// CHECK: nop; 	nop; 	cmp_eq.u32  VP1, V2, S3, SP4; 	nop
// CHECK: nop; 	nop; 	cmp_eq.u32  VP1, V2, S3, !SP4; 	nop
// CHECK: nop; 	nop; 	cmp_eq.u32  VP1, V2, S3, VP4; 	nop
// CHECK: nop; 	nop; 	cmp_eq.u32  VP1, V2, S3, !VP4; 	nop


NOP; NOP; CMP_EQ.U32 VP1, V2, 123; NOP
NOP; NOP; CMP_EQ.U32 VP1, V2, 123, SP4; NOP
NOP; NOP; CMP_EQ.U32 VP1, V2, 123, !SP4; NOP
NOP; NOP; CMP_EQ.U32 VP1, V2, 123, VP4; NOP
NOP; NOP; CMP_EQ.U32 VP1, V2, 123, !VP4; NOP

// CHECK: nop; 	nop; 	cmp_eq.u32  VP1, V2, 0x7b; 	nop
// CHECK: nop; 	nop; 	cmp_eq.u32  VP1, V2, 0x7b, SP4; 	nop
// CHECK: nop; 	nop; 	cmp_eq.u32  VP1, V2, 0x7b, !SP4; 	nop
// CHECK: nop; 	nop; 	cmp_eq.u32  VP1, V2, 0x7b, VP4; 	nop
// CHECK: nop; 	nop; 	cmp_eq.u32  VP1, V2, 0x7b, !VP4; 	nop


NOP; NOP; CMP_EQ.I16 VP1, V2, V3; NOP
NOP; NOP; CMP_EQ.I16 VP1, V2, V3, SP4; NOP
NOP; NOP; CMP_EQ.I16 VP1, V2, V3, !SP4; NOP
NOP; NOP; CMP_EQ.I16 VP1, V2, V3, VP4; NOP
NOP; NOP; CMP_EQ.I16 VP1, V2, V3, !VP4; NOP

// CHECK: nop; 	nop; 	cmp_eq.i16  VP1, V2, V3; 	nop
// CHECK: nop; 	nop; 	cmp_eq.i16  VP1, V2, V3, SP4; 	nop
// CHECK: nop; 	nop; 	cmp_eq.i16  VP1, V2, V3, !SP4; 	nop
// CHECK: nop; 	nop; 	cmp_eq.i16  VP1, V2, V3, VP4; 	nop
// CHECK: nop; 	nop; 	cmp_eq.i16  VP1, V2, V3, !VP4; 	nop


NOP; NOP; CMP_EQ.I16 VP1, V2, S3; NOP
NOP; NOP; CMP_EQ.I16 VP1, V2, S3, SP4; NOP
NOP; NOP; CMP_EQ.I16 VP1, V2, S3, !SP4; NOP
NOP; NOP; CMP_EQ.I16 VP1, V2, S3, VP4; NOP
NOP; NOP; CMP_EQ.I16 VP1, V2, S3, !VP4; NOP

// CHECK: nop; 	nop; 	cmp_eq.i16  VP1, V2, S3; 	nop
// CHECK: nop; 	nop; 	cmp_eq.i16  VP1, V2, S3, SP4; 	nop
// CHECK: nop; 	nop; 	cmp_eq.i16  VP1, V2, S3, !SP4; 	nop
// CHECK: nop; 	nop; 	cmp_eq.i16  VP1, V2, S3, VP4; 	nop
// CHECK: nop; 	nop; 	cmp_eq.i16  VP1, V2, S3, !VP4; 	nop


NOP; NOP; CMP_EQ.I16 VP1, V2, 123; NOP
NOP; NOP; CMP_EQ.I16 VP1, V2, 123, SP4; NOP
NOP; NOP; CMP_EQ.I16 VP1, V2, 123, !SP4; NOP
NOP; NOP; CMP_EQ.I16 VP1, V2, 123, VP4; NOP
NOP; NOP; CMP_EQ.I16 VP1, V2, 123, !VP4; NOP

// CHECK: nop; 	nop; 	cmp_eq.i16  VP1, V2, 0x7b; 	nop
// CHECK: nop; 	nop; 	cmp_eq.i16  VP1, V2, 0x7b, SP4; 	nop
// CHECK: nop; 	nop; 	cmp_eq.i16  VP1, V2, 0x7b, !SP4; 	nop
// CHECK: nop; 	nop; 	cmp_eq.i16  VP1, V2, 0x7b, VP4; 	nop
// CHECK: nop; 	nop; 	cmp_eq.i16  VP1, V2, 0x7b, !VP4; 	nop


NOP; NOP; CMP_EQ.U16 VP1, V2, V3; NOP
NOP; NOP; CMP_EQ.U16 VP1, V2, V3, SP4; NOP
NOP; NOP; CMP_EQ.U16 VP1, V2, V3, !SP4; NOP
NOP; NOP; CMP_EQ.U16 VP1, V2, V3, VP4; NOP
NOP; NOP; CMP_EQ.U16 VP1, V2, V3, !VP4; NOP

// CHECK: nop; 	nop; 	cmp_eq.u16  VP1, V2, V3; 	nop
// CHECK: nop; 	nop; 	cmp_eq.u16  VP1, V2, V3, SP4; 	nop
// CHECK: nop; 	nop; 	cmp_eq.u16  VP1, V2, V3, !SP4; 	nop
// CHECK: nop; 	nop; 	cmp_eq.u16  VP1, V2, V3, VP4; 	nop
// CHECK: nop; 	nop; 	cmp_eq.u16  VP1, V2, V3, !VP4; 	nop


NOP; NOP; CMP_EQ.U16 VP1, V2, S3; NOP
NOP; NOP; CMP_EQ.U16 VP1, V2, S3, SP4; NOP
NOP; NOP; CMP_EQ.U16 VP1, V2, S3, !SP4; NOP
NOP; NOP; CMP_EQ.U16 VP1, V2, S3, VP4; NOP
NOP; NOP; CMP_EQ.U16 VP1, V2, S3, !VP4; NOP

// CHECK: nop; 	nop; 	cmp_eq.u16  VP1, V2, S3; 	nop
// CHECK: nop; 	nop; 	cmp_eq.u16  VP1, V2, S3, SP4; 	nop
// CHECK: nop; 	nop; 	cmp_eq.u16  VP1, V2, S3, !SP4; 	nop
// CHECK: nop; 	nop; 	cmp_eq.u16  VP1, V2, S3, VP4; 	nop
// CHECK: nop; 	nop; 	cmp_eq.u16  VP1, V2, S3, !VP4; 	nop


NOP; NOP; CMP_EQ.U16 VP1, V2, 123; NOP
NOP; NOP; CMP_EQ.U16 VP1, V2, 123, SP4; NOP
NOP; NOP; CMP_EQ.U16 VP1, V2, 123, !SP4; NOP
NOP; NOP; CMP_EQ.U16 VP1, V2, 123, VP4; NOP
NOP; NOP; CMP_EQ.U16 VP1, V2, 123, !VP4; NOP

// CHECK: nop; 	nop; 	cmp_eq.u16  VP1, V2, 0x7b; 	nop
// CHECK: nop; 	nop; 	cmp_eq.u16  VP1, V2, 0x7b, SP4; 	nop
// CHECK: nop; 	nop; 	cmp_eq.u16  VP1, V2, 0x7b, !SP4; 	nop
// CHECK: nop; 	nop; 	cmp_eq.u16  VP1, V2, 0x7b, VP4; 	nop
// CHECK: nop; 	nop; 	cmp_eq.u16  VP1, V2, 0x7b, !VP4; 	nop


NOP; NOP; CMP_EQ.I8 VP1, V2, V3; NOP
NOP; NOP; CMP_EQ.I8 VP1, V2, V3, SP4; NOP
NOP; NOP; CMP_EQ.I8 VP1, V2, V3, !SP4; NOP
NOP; NOP; CMP_EQ.I8 VP1, V2, V3, VP4; NOP
NOP; NOP; CMP_EQ.I8 VP1, V2, V3, !VP4; NOP

// CHECK: nop; 	nop; 	cmp_eq.i8  VP1, V2, V3; 	nop
// CHECK: nop; 	nop; 	cmp_eq.i8  VP1, V2, V3, SP4; 	nop
// CHECK: nop; 	nop; 	cmp_eq.i8  VP1, V2, V3, !SP4; 	nop
// CHECK: nop; 	nop; 	cmp_eq.i8  VP1, V2, V3, VP4; 	nop
// CHECK: nop; 	nop; 	cmp_eq.i8  VP1, V2, V3, !VP4; 	nop


NOP; NOP; CMP_EQ.I8 VP1, V2, S3; NOP
NOP; NOP; CMP_EQ.I8 VP1, V2, S3, SP4; NOP
NOP; NOP; CMP_EQ.I8 VP1, V2, S3, !SP4; NOP
NOP; NOP; CMP_EQ.I8 VP1, V2, S3, VP4; NOP
NOP; NOP; CMP_EQ.I8 VP1, V2, S3, !VP4; NOP

// CHECK: nop; 	nop; 	cmp_eq.i8  VP1, V2, S3; 	nop
// CHECK: nop; 	nop; 	cmp_eq.i8  VP1, V2, S3, SP4; 	nop
// CHECK: nop; 	nop; 	cmp_eq.i8  VP1, V2, S3, !SP4; 	nop
// CHECK: nop; 	nop; 	cmp_eq.i8  VP1, V2, S3, VP4; 	nop
// CHECK: nop; 	nop; 	cmp_eq.i8  VP1, V2, S3, !VP4; 	nop


NOP; NOP; CMP_EQ.I8 VP1, V2, 123; NOP
NOP; NOP; CMP_EQ.I8 VP1, V2, 123, SP4; NOP
NOP; NOP; CMP_EQ.I8 VP1, V2, 123, !SP4; NOP
NOP; NOP; CMP_EQ.I8 VP1, V2, 123, VP4; NOP
NOP; NOP; CMP_EQ.I8 VP1, V2, 123, !VP4; NOP

// CHECK: nop; 	nop; 	cmp_eq.i8  VP1, V2, 0x7b; 	nop
// CHECK: nop; 	nop; 	cmp_eq.i8  VP1, V2, 0x7b, SP4; 	nop
// CHECK: nop; 	nop; 	cmp_eq.i8  VP1, V2, 0x7b, !SP4; 	nop
// CHECK: nop; 	nop; 	cmp_eq.i8  VP1, V2, 0x7b, VP4; 	nop
// CHECK: nop; 	nop; 	cmp_eq.i8  VP1, V2, 0x7b, !VP4; 	nop


NOP; NOP; CMP_EQ.U8 VP1, V2, V3; NOP
NOP; NOP; CMP_EQ.U8 VP1, V2, V3, SP4; NOP
NOP; NOP; CMP_EQ.U8 VP1, V2, V3, !SP4; NOP
NOP; NOP; CMP_EQ.U8 VP1, V2, V3, VP4; NOP
NOP; NOP; CMP_EQ.U8 VP1, V2, V3, !VP4; NOP

// CHECK: nop; 	nop; 	cmp_eq.u8  VP1, V2, V3; 	nop
// CHECK: nop; 	nop; 	cmp_eq.u8  VP1, V2, V3, SP4; 	nop
// CHECK: nop; 	nop; 	cmp_eq.u8  VP1, V2, V3, !SP4; 	nop
// CHECK: nop; 	nop; 	cmp_eq.u8  VP1, V2, V3, VP4; 	nop
// CHECK: nop; 	nop; 	cmp_eq.u8  VP1, V2, V3, !VP4; 	nop


NOP; NOP; CMP_EQ.U8 VP1, V2, S3; NOP
NOP; NOP; CMP_EQ.U8 VP1, V2, S3, SP4; NOP
NOP; NOP; CMP_EQ.U8 VP1, V2, S3, !SP4; NOP
NOP; NOP; CMP_EQ.U8 VP1, V2, S3, VP4; NOP
NOP; NOP; CMP_EQ.U8 VP1, V2, S3, !VP4; NOP

// CHECK: nop; 	nop; 	cmp_eq.u8  VP1, V2, S3; 	nop
// CHECK: nop; 	nop; 	cmp_eq.u8  VP1, V2, S3, SP4; 	nop
// CHECK: nop; 	nop; 	cmp_eq.u8  VP1, V2, S3, !SP4; 	nop
// CHECK: nop; 	nop; 	cmp_eq.u8  VP1, V2, S3, VP4; 	nop
// CHECK: nop; 	nop; 	cmp_eq.u8  VP1, V2, S3, !VP4; 	nop


NOP; NOP; CMP_EQ.U8 VP1, V2, 123; NOP
NOP; NOP; CMP_EQ.U8 VP1, V2, 123, SP4; NOP
NOP; NOP; CMP_EQ.U8 VP1, V2, 123, !SP4; NOP
NOP; NOP; CMP_EQ.U8 VP1, V2, 123, VP4; NOP
NOP; NOP; CMP_EQ.U8 VP1, V2, 123, !VP4; NOP

// CHECK: nop; 	nop; 	cmp_eq.u8  VP1, V2, 0x7b; 	nop
// CHECK: nop; 	nop; 	cmp_eq.u8  VP1, V2, 0x7b, SP4; 	nop
// CHECK: nop; 	nop; 	cmp_eq.u8  VP1, V2, 0x7b, !SP4; 	nop
// CHECK: nop; 	nop; 	cmp_eq.u8  VP1, V2, 0x7b, VP4; 	nop
// CHECK: nop; 	nop; 	cmp_eq.u8  VP1, V2, 0x7b, !VP4; 	nop
