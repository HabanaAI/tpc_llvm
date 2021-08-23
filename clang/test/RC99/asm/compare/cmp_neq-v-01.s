// RUN: %clang -cc1as -triple tpc-none-none -filetype obj %s -o %t.o
// RUN: %disasm %t.o | FileCheck %s
// RUN: %clang -cc1as -triple tpc-none-none -target-cpu gaudi -filetype obj %s -o %t.o
// RUN: %disasm -mcpu=gaudi %t.o | FileCheck %s




NOP; NOP; CMP_NEQ.F32 VP1, V2, V3; NOP
NOP; NOP; CMP_NEQ.F32 VP1, V2, V3, SP4; NOP
NOP; NOP; CMP_NEQ.F32 VP1, V2, V3, !SP4; NOP
NOP; NOP; CMP_NEQ.F32 VP1, V2, V3, VP4; NOP
NOP; NOP; CMP_NEQ.F32 VP1, V2, V3, !VP4; NOP

// CHECK: nop; 	nop; 	cmp_neq.f32  VP1, V2, V3, SP0; 	nop
// CHECK: nop; 	nop; 	cmp_neq.f32  VP1, V2, V3, SP4; 	nop
// CHECK: nop; 	nop; 	cmp_neq.f32  VP1, V2, V3, !SP4; 	nop
// CHECK: nop; 	nop; 	cmp_neq.f32  VP1, V2, V3, VP4; 	nop
// CHECK: nop; 	nop; 	cmp_neq.f32  VP1, V2, V3, !VP4; 	nop


NOP; NOP; CMP_NEQ.F32 VP1, V2, S3; NOP
NOP; NOP; CMP_NEQ.F32 VP1, V2, S3, SP4; NOP
NOP; NOP; CMP_NEQ.F32 VP1, V2, S3, !SP4; NOP
NOP; NOP; CMP_NEQ.F32 VP1, V2, S3, VP4; NOP
NOP; NOP; CMP_NEQ.F32 VP1, V2, S3, !VP4; NOP

// CHECK: nop; 	nop; 	cmp_neq.f32  VP1, V2, S3, SP0; 	nop
// CHECK: nop; 	nop; 	cmp_neq.f32  VP1, V2, S3, SP4; 	nop
// CHECK: nop; 	nop; 	cmp_neq.f32  VP1, V2, S3, !SP4; 	nop
// CHECK: nop; 	nop; 	cmp_neq.f32  VP1, V2, S3, VP4; 	nop
// CHECK: nop; 	nop; 	cmp_neq.f32  VP1, V2, S3, !VP4; 	nop


NOP; NOP; CMP_NEQ.F32 VP1, V2, 0x3f4ccccd; NOP
NOP; NOP; CMP_NEQ.F32 VP1, V2, 0x3f4ccccd, SP4; NOP
NOP; NOP; CMP_NEQ.F32 VP1, V2, 0x3f4ccccd, !SP4; NOP
NOP; NOP; CMP_NEQ.F32 VP1, V2, 0x3f4ccccd, VP4; NOP
NOP; NOP; CMP_NEQ.F32 VP1, V2, 0x3f4ccccd, !VP4; NOP

// CHECK: nop; 	nop; 	cmp_neq.f32  VP1, V2, 0x3f4ccccd, SP0; 	nop
// CHECK: nop; 	nop; 	cmp_neq.f32  VP1, V2, 0x3f4ccccd, SP4; 	nop
// CHECK: nop; 	nop; 	cmp_neq.f32  VP1, V2, 0x3f4ccccd, !SP4; 	nop
// CHECK: nop; 	nop; 	cmp_neq.f32  VP1, V2, 0x3f4ccccd, VP4; 	nop
// CHECK: nop; 	nop; 	cmp_neq.f32  VP1, V2, 0x3f4ccccd, !VP4; 	nop


NOP; NOP; CMP_NEQ.I32 VP1, V2, V3; NOP
NOP; NOP; CMP_NEQ.I32 VP1, V2, V3, SP4; NOP
NOP; NOP; CMP_NEQ.I32 VP1, V2, V3, !SP4; NOP
NOP; NOP; CMP_NEQ.I32 VP1, V2, V3, VP4; NOP
NOP; NOP; CMP_NEQ.I32 VP1, V2, V3, !VP4; NOP

// CHECK: nop; 	nop; 	cmp_neq.i32  VP1, V2, V3, SP0; 	nop
// CHECK: nop; 	nop; 	cmp_neq.i32  VP1, V2, V3, SP4; 	nop
// CHECK: nop; 	nop; 	cmp_neq.i32  VP1, V2, V3, !SP4; 	nop
// CHECK: nop; 	nop; 	cmp_neq.i32  VP1, V2, V3, VP4; 	nop
// CHECK: nop; 	nop; 	cmp_neq.i32  VP1, V2, V3, !VP4; 	nop


NOP; NOP; CMP_NEQ.I32 VP1, V2, S3; NOP
NOP; NOP; CMP_NEQ.I32 VP1, V2, S3, SP4; NOP
NOP; NOP; CMP_NEQ.I32 VP1, V2, S3, !SP4; NOP
NOP; NOP; CMP_NEQ.I32 VP1, V2, S3, VP4; NOP
NOP; NOP; CMP_NEQ.I32 VP1, V2, S3, !VP4; NOP

// CHECK: nop; 	nop; 	cmp_neq.i32  VP1, V2, S3, SP0; 	nop
// CHECK: nop; 	nop; 	cmp_neq.i32  VP1, V2, S3, SP4; 	nop
// CHECK: nop; 	nop; 	cmp_neq.i32  VP1, V2, S3, !SP4; 	nop
// CHECK: nop; 	nop; 	cmp_neq.i32  VP1, V2, S3, VP4; 	nop
// CHECK: nop; 	nop; 	cmp_neq.i32  VP1, V2, S3, !VP4; 	nop


NOP; NOP; CMP_NEQ.I32 VP1, V2, 123; NOP
NOP; NOP; CMP_NEQ.I32 VP1, V2, 123, SP4; NOP
NOP; NOP; CMP_NEQ.I32 VP1, V2, 123, !SP4; NOP
NOP; NOP; CMP_NEQ.I32 VP1, V2, 123, VP4; NOP
NOP; NOP; CMP_NEQ.I32 VP1, V2, 123, !VP4; NOP

// CHECK: nop; 	nop; 	cmp_neq.i32  VP1, V2, 0x7b, SP0; 	nop
// CHECK: nop; 	nop; 	cmp_neq.i32  VP1, V2, 0x7b, SP4; 	nop
// CHECK: nop; 	nop; 	cmp_neq.i32  VP1, V2, 0x7b, !SP4; 	nop
// CHECK: nop; 	nop; 	cmp_neq.i32  VP1, V2, 0x7b, VP4; 	nop
// CHECK: nop; 	nop; 	cmp_neq.i32  VP1, V2, 0x7b, !VP4; 	nop


NOP; NOP; CMP_NEQ.U32 VP1, V2, V3; NOP
NOP; NOP; CMP_NEQ.U32 VP1, V2, V3, SP4; NOP
NOP; NOP; CMP_NEQ.U32 VP1, V2, V3, !SP4; NOP
NOP; NOP; CMP_NEQ.U32 VP1, V2, V3, VP4; NOP
NOP; NOP; CMP_NEQ.U32 VP1, V2, V3, !VP4; NOP

// CHECK: nop; 	nop; 	cmp_neq.u32  VP1, V2, V3, SP0; 	nop
// CHECK: nop; 	nop; 	cmp_neq.u32  VP1, V2, V3, SP4; 	nop
// CHECK: nop; 	nop; 	cmp_neq.u32  VP1, V2, V3, !SP4; 	nop
// CHECK: nop; 	nop; 	cmp_neq.u32  VP1, V2, V3, VP4; 	nop
// CHECK: nop; 	nop; 	cmp_neq.u32  VP1, V2, V3, !VP4; 	nop


NOP; NOP; CMP_NEQ.U32 VP1, V2, S3; NOP
NOP; NOP; CMP_NEQ.U32 VP1, V2, S3, SP4; NOP
NOP; NOP; CMP_NEQ.U32 VP1, V2, S3, !SP4; NOP
NOP; NOP; CMP_NEQ.U32 VP1, V2, S3, VP4; NOP
NOP; NOP; CMP_NEQ.U32 VP1, V2, S3, !VP4; NOP

// CHECK: nop; 	nop; 	cmp_neq.u32  VP1, V2, S3, SP0; 	nop
// CHECK: nop; 	nop; 	cmp_neq.u32  VP1, V2, S3, SP4; 	nop
// CHECK: nop; 	nop; 	cmp_neq.u32  VP1, V2, S3, !SP4; 	nop
// CHECK: nop; 	nop; 	cmp_neq.u32  VP1, V2, S3, VP4; 	nop
// CHECK: nop; 	nop; 	cmp_neq.u32  VP1, V2, S3, !VP4; 	nop


NOP; NOP; CMP_NEQ.U32 VP1, V2, 123; NOP
NOP; NOP; CMP_NEQ.U32 VP1, V2, 123, SP4; NOP
NOP; NOP; CMP_NEQ.U32 VP1, V2, 123, !SP4; NOP
NOP; NOP; CMP_NEQ.U32 VP1, V2, 123, VP4; NOP
NOP; NOP; CMP_NEQ.U32 VP1, V2, 123, !VP4; NOP

// CHECK: nop; 	nop; 	cmp_neq.u32  VP1, V2, 0x7b, SP0; 	nop
// CHECK: nop; 	nop; 	cmp_neq.u32  VP1, V2, 0x7b, SP4; 	nop
// CHECK: nop; 	nop; 	cmp_neq.u32  VP1, V2, 0x7b, !SP4; 	nop
// CHECK: nop; 	nop; 	cmp_neq.u32  VP1, V2, 0x7b, VP4; 	nop
// CHECK: nop; 	nop; 	cmp_neq.u32  VP1, V2, 0x7b, !VP4; 	nop


NOP; NOP; CMP_NEQ.I16 VP1, V2, V3; NOP
NOP; NOP; CMP_NEQ.I16 VP1, V2, V3, SP4; NOP
NOP; NOP; CMP_NEQ.I16 VP1, V2, V3, !SP4; NOP
NOP; NOP; CMP_NEQ.I16 VP1, V2, V3, VP4; NOP
NOP; NOP; CMP_NEQ.I16 VP1, V2, V3, !VP4; NOP

// CHECK: nop; 	nop; 	cmp_neq.i16  VP1, V2, V3, SP0; 	nop
// CHECK: nop; 	nop; 	cmp_neq.i16  VP1, V2, V3, SP4; 	nop
// CHECK: nop; 	nop; 	cmp_neq.i16  VP1, V2, V3, !SP4; 	nop
// CHECK: nop; 	nop; 	cmp_neq.i16  VP1, V2, V3, VP4; 	nop
// CHECK: nop; 	nop; 	cmp_neq.i16  VP1, V2, V3, !VP4; 	nop


NOP; NOP; CMP_NEQ.I16 VP1, V2, S3; NOP
NOP; NOP; CMP_NEQ.I16 VP1, V2, S3, SP4; NOP
NOP; NOP; CMP_NEQ.I16 VP1, V2, S3, !SP4; NOP
NOP; NOP; CMP_NEQ.I16 VP1, V2, S3, VP4; NOP
NOP; NOP; CMP_NEQ.I16 VP1, V2, S3, !VP4; NOP

// CHECK: nop; 	nop; 	cmp_neq.i16  VP1, V2, S3, SP0; 	nop
// CHECK: nop; 	nop; 	cmp_neq.i16  VP1, V2, S3, SP4; 	nop
// CHECK: nop; 	nop; 	cmp_neq.i16  VP1, V2, S3, !SP4; 	nop
// CHECK: nop; 	nop; 	cmp_neq.i16  VP1, V2, S3, VP4; 	nop
// CHECK: nop; 	nop; 	cmp_neq.i16  VP1, V2, S3, !VP4; 	nop


NOP; NOP; CMP_NEQ.I16 VP1, V2, 123; NOP
NOP; NOP; CMP_NEQ.I16 VP1, V2, 123, SP4; NOP
NOP; NOP; CMP_NEQ.I16 VP1, V2, 123, !SP4; NOP
NOP; NOP; CMP_NEQ.I16 VP1, V2, 123, VP4; NOP
NOP; NOP; CMP_NEQ.I16 VP1, V2, 123, !VP4; NOP

// CHECK: nop; 	nop; 	cmp_neq.i16  VP1, V2, 0x7b, SP0; 	nop
// CHECK: nop; 	nop; 	cmp_neq.i16  VP1, V2, 0x7b, SP4; 	nop
// CHECK: nop; 	nop; 	cmp_neq.i16  VP1, V2, 0x7b, !SP4; 	nop
// CHECK: nop; 	nop; 	cmp_neq.i16  VP1, V2, 0x7b, VP4; 	nop
// CHECK: nop; 	nop; 	cmp_neq.i16  VP1, V2, 0x7b, !VP4; 	nop


NOP; NOP; CMP_NEQ.U16 VP1, V2, V3; NOP
NOP; NOP; CMP_NEQ.U16 VP1, V2, V3, SP4; NOP
NOP; NOP; CMP_NEQ.U16 VP1, V2, V3, !SP4; NOP
NOP; NOP; CMP_NEQ.U16 VP1, V2, V3, VP4; NOP
NOP; NOP; CMP_NEQ.U16 VP1, V2, V3, !VP4; NOP

// CHECK: nop; 	nop; 	cmp_neq.u16  VP1, V2, V3, SP0; 	nop
// CHECK: nop; 	nop; 	cmp_neq.u16  VP1, V2, V3, SP4; 	nop
// CHECK: nop; 	nop; 	cmp_neq.u16  VP1, V2, V3, !SP4; 	nop
// CHECK: nop; 	nop; 	cmp_neq.u16  VP1, V2, V3, VP4; 	nop
// CHECK: nop; 	nop; 	cmp_neq.u16  VP1, V2, V3, !VP4; 	nop


NOP; NOP; CMP_NEQ.U16 VP1, V2, S3; NOP
NOP; NOP; CMP_NEQ.U16 VP1, V2, S3, SP4; NOP
NOP; NOP; CMP_NEQ.U16 VP1, V2, S3, !SP4; NOP
NOP; NOP; CMP_NEQ.U16 VP1, V2, S3, VP4; NOP
NOP; NOP; CMP_NEQ.U16 VP1, V2, S3, !VP4; NOP

// CHECK: nop; 	nop; 	cmp_neq.u16  VP1, V2, S3, SP0; 	nop
// CHECK: nop; 	nop; 	cmp_neq.u16  VP1, V2, S3, SP4; 	nop
// CHECK: nop; 	nop; 	cmp_neq.u16  VP1, V2, S3, !SP4; 	nop
// CHECK: nop; 	nop; 	cmp_neq.u16  VP1, V2, S3, VP4; 	nop
// CHECK: nop; 	nop; 	cmp_neq.u16  VP1, V2, S3, !VP4; 	nop


NOP; NOP; CMP_NEQ.U16 VP1, V2, 123; NOP
NOP; NOP; CMP_NEQ.U16 VP1, V2, 123, SP4; NOP
NOP; NOP; CMP_NEQ.U16 VP1, V2, 123, !SP4; NOP
NOP; NOP; CMP_NEQ.U16 VP1, V2, 123, VP4; NOP
NOP; NOP; CMP_NEQ.U16 VP1, V2, 123, !VP4; NOP

// CHECK: nop; 	nop; 	cmp_neq.u16  VP1, V2, 0x7b, SP0; 	nop
// CHECK: nop; 	nop; 	cmp_neq.u16  VP1, V2, 0x7b, SP4; 	nop
// CHECK: nop; 	nop; 	cmp_neq.u16  VP1, V2, 0x7b, !SP4; 	nop
// CHECK: nop; 	nop; 	cmp_neq.u16  VP1, V2, 0x7b, VP4; 	nop
// CHECK: nop; 	nop; 	cmp_neq.u16  VP1, V2, 0x7b, !VP4; 	nop


NOP; NOP; CMP_NEQ.I8 VP1, V2, V3; NOP
NOP; NOP; CMP_NEQ.I8 VP1, V2, V3, SP4; NOP
NOP; NOP; CMP_NEQ.I8 VP1, V2, V3, !SP4; NOP
NOP; NOP; CMP_NEQ.I8 VP1, V2, V3, VP4; NOP
NOP; NOP; CMP_NEQ.I8 VP1, V2, V3, !VP4; NOP

// CHECK: nop; 	nop; 	cmp_neq.i8  VP1, V2, V3, SP0; 	nop
// CHECK: nop; 	nop; 	cmp_neq.i8  VP1, V2, V3, SP4; 	nop
// CHECK: nop; 	nop; 	cmp_neq.i8  VP1, V2, V3, !SP4; 	nop
// CHECK: nop; 	nop; 	cmp_neq.i8  VP1, V2, V3, VP4; 	nop
// CHECK: nop; 	nop; 	cmp_neq.i8  VP1, V2, V3, !VP4; 	nop


NOP; NOP; CMP_NEQ.I8 VP1, V2, S3; NOP
NOP; NOP; CMP_NEQ.I8 VP1, V2, S3, SP4; NOP
NOP; NOP; CMP_NEQ.I8 VP1, V2, S3, !SP4; NOP
NOP; NOP; CMP_NEQ.I8 VP1, V2, S3, VP4; NOP
NOP; NOP; CMP_NEQ.I8 VP1, V2, S3, !VP4; NOP

// CHECK: nop; 	nop; 	cmp_neq.i8  VP1, V2, S3, SP0; 	nop
// CHECK: nop; 	nop; 	cmp_neq.i8  VP1, V2, S3, SP4; 	nop
// CHECK: nop; 	nop; 	cmp_neq.i8  VP1, V2, S3, !SP4; 	nop
// CHECK: nop; 	nop; 	cmp_neq.i8  VP1, V2, S3, VP4; 	nop
// CHECK: nop; 	nop; 	cmp_neq.i8  VP1, V2, S3, !VP4; 	nop


NOP; NOP; CMP_NEQ.I8 VP1, V2, 123; NOP
NOP; NOP; CMP_NEQ.I8 VP1, V2, 123, SP4; NOP
NOP; NOP; CMP_NEQ.I8 VP1, V2, 123, !SP4; NOP
NOP; NOP; CMP_NEQ.I8 VP1, V2, 123, VP4; NOP
NOP; NOP; CMP_NEQ.I8 VP1, V2, 123, !VP4; NOP

// CHECK: nop; 	nop; 	cmp_neq.i8  VP1, V2, 0x7b, SP0; 	nop
// CHECK: nop; 	nop; 	cmp_neq.i8  VP1, V2, 0x7b, SP4; 	nop
// CHECK: nop; 	nop; 	cmp_neq.i8  VP1, V2, 0x7b, !SP4; 	nop
// CHECK: nop; 	nop; 	cmp_neq.i8  VP1, V2, 0x7b, VP4; 	nop
// CHECK: nop; 	nop; 	cmp_neq.i8  VP1, V2, 0x7b, !VP4; 	nop


NOP; NOP; CMP_NEQ.U8 VP1, V2, V3; NOP
NOP; NOP; CMP_NEQ.U8 VP1, V2, V3, SP4; NOP
NOP; NOP; CMP_NEQ.U8 VP1, V2, V3, !SP4; NOP
NOP; NOP; CMP_NEQ.U8 VP1, V2, V3, VP4; NOP
NOP; NOP; CMP_NEQ.U8 VP1, V2, V3, !VP4; NOP

// CHECK: nop; 	nop; 	cmp_neq.u8  VP1, V2, V3, SP0; 	nop
// CHECK: nop; 	nop; 	cmp_neq.u8  VP1, V2, V3, SP4; 	nop
// CHECK: nop; 	nop; 	cmp_neq.u8  VP1, V2, V3, !SP4; 	nop
// CHECK: nop; 	nop; 	cmp_neq.u8  VP1, V2, V3, VP4; 	nop
// CHECK: nop; 	nop; 	cmp_neq.u8  VP1, V2, V3, !VP4; 	nop


NOP; NOP; CMP_NEQ.U8 VP1, V2, S3; NOP
NOP; NOP; CMP_NEQ.U8 VP1, V2, S3, SP4; NOP
NOP; NOP; CMP_NEQ.U8 VP1, V2, S3, !SP4; NOP
NOP; NOP; CMP_NEQ.U8 VP1, V2, S3, VP4; NOP
NOP; NOP; CMP_NEQ.U8 VP1, V2, S3, !VP4; NOP

// CHECK: nop; 	nop; 	cmp_neq.u8  VP1, V2, S3, SP0; 	nop
// CHECK: nop; 	nop; 	cmp_neq.u8  VP1, V2, S3, SP4; 	nop
// CHECK: nop; 	nop; 	cmp_neq.u8  VP1, V2, S3, !SP4; 	nop
// CHECK: nop; 	nop; 	cmp_neq.u8  VP1, V2, S3, VP4; 	nop
// CHECK: nop; 	nop; 	cmp_neq.u8  VP1, V2, S3, !VP4; 	nop


NOP; NOP; CMP_NEQ.U8 VP1, V2, 123; NOP
NOP; NOP; CMP_NEQ.U8 VP1, V2, 123, SP4; NOP
NOP; NOP; CMP_NEQ.U8 VP1, V2, 123, !SP4; NOP
NOP; NOP; CMP_NEQ.U8 VP1, V2, 123, VP4; NOP
NOP; NOP; CMP_NEQ.U8 VP1, V2, 123, !VP4; NOP

// CHECK: nop; 	nop; 	cmp_neq.u8  VP1, V2, 0x7b, SP0; 	nop
// CHECK: nop; 	nop; 	cmp_neq.u8  VP1, V2, 0x7b, SP4; 	nop
// CHECK: nop; 	nop; 	cmp_neq.u8  VP1, V2, 0x7b, !SP4; 	nop
// CHECK: nop; 	nop; 	cmp_neq.u8  VP1, V2, 0x7b, VP4; 	nop
// CHECK: nop; 	nop; 	cmp_neq.u8  VP1, V2, 0x7b, !VP4; 	nop