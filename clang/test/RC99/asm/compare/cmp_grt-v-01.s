// RUN: %clang -cc1as -triple tpc-none-none -filetype obj %s -o %t.o
// RUN: %disasm %t.o | FileCheck %s
// RUN: %clang -cc1as -triple tpc-none-none -target-cpu gaudi -filetype obj %s -o %t.o
// RUN: %disasm -mcpu=gaudi %t.o | FileCheck %s




NOP; NOP; CMP_GRT.F32 VP1, V2, V3; NOP
NOP; NOP; CMP_GRT.F32 VP1, V2, V3, SP4; NOP
NOP; NOP; CMP_GRT.F32 VP1, V2, V3, !SP4; NOP
NOP; NOP; CMP_GRT.F32 VP1, V2, V3, VP4; NOP
NOP; NOP; CMP_GRT.F32 VP1, V2, V3, !VP4; NOP

// CHECK: nop; 	nop; 	cmp_grt.f32  VP1, V2, V3, SP0; 	nop
// CHECK: nop; 	nop; 	cmp_grt.f32  VP1, V2, V3, SP4; 	nop
// CHECK: nop; 	nop; 	cmp_grt.f32  VP1, V2, V3, !SP4; 	nop
// CHECK: nop; 	nop; 	cmp_grt.f32  VP1, V2, V3, VP4; 	nop
// CHECK: nop; 	nop; 	cmp_grt.f32  VP1, V2, V3, !VP4; 	nop


NOP; NOP; CMP_GRT.F32 VP1, V2, S3; NOP
NOP; NOP; CMP_GRT.F32 VP1, V2, S3, SP4; NOP
NOP; NOP; CMP_GRT.F32 VP1, V2, S3, !SP4; NOP
NOP; NOP; CMP_GRT.F32 VP1, V2, S3, VP4; NOP
NOP; NOP; CMP_GRT.F32 VP1, V2, S3, !VP4; NOP

// CHECK: nop; 	nop; 	cmp_grt.f32  VP1, V2, S3, SP0; 	nop
// CHECK: nop; 	nop; 	cmp_grt.f32  VP1, V2, S3, SP4; 	nop
// CHECK: nop; 	nop; 	cmp_grt.f32  VP1, V2, S3, !SP4; 	nop
// CHECK: nop; 	nop; 	cmp_grt.f32  VP1, V2, S3, VP4; 	nop
// CHECK: nop; 	nop; 	cmp_grt.f32  VP1, V2, S3, !VP4; 	nop


NOP; NOP; CMP_GRT.F32 VP1, V2, 0x3f4ccccd; NOP
NOP; NOP; CMP_GRT.F32 VP1, V2, 0x3f4ccccd, SP4; NOP
NOP; NOP; CMP_GRT.F32 VP1, V2, 0x3f4ccccd, !SP4; NOP
NOP; NOP; CMP_GRT.F32 VP1, V2, 0x3f4ccccd, VP4; NOP
NOP; NOP; CMP_GRT.F32 VP1, V2, 0x3f4ccccd, !VP4; NOP

// CHECK: nop; 	nop; 	cmp_grt.f32  VP1, V2, 0x3f4ccccd, SP0; 	nop
// CHECK: nop; 	nop; 	cmp_grt.f32  VP1, V2, 0x3f4ccccd, SP4; 	nop
// CHECK: nop; 	nop; 	cmp_grt.f32  VP1, V2, 0x3f4ccccd, !SP4; 	nop
// CHECK: nop; 	nop; 	cmp_grt.f32  VP1, V2, 0x3f4ccccd, VP4; 	nop
// CHECK: nop; 	nop; 	cmp_grt.f32  VP1, V2, 0x3f4ccccd, !VP4; 	nop


NOP; NOP; CMP_GRT.I32 VP1, V2, V3; NOP
NOP; NOP; CMP_GRT.I32 VP1, V2, V3, SP4; NOP
NOP; NOP; CMP_GRT.I32 VP1, V2, V3, !SP4; NOP
NOP; NOP; CMP_GRT.I32 VP1, V2, V3, VP4; NOP
NOP; NOP; CMP_GRT.I32 VP1, V2, V3, !VP4; NOP

// CHECK: nop; 	nop; 	cmp_grt.i32  VP1, V2, V3, SP0; 	nop
// CHECK: nop; 	nop; 	cmp_grt.i32  VP1, V2, V3, SP4; 	nop
// CHECK: nop; 	nop; 	cmp_grt.i32  VP1, V2, V3, !SP4; 	nop
// CHECK: nop; 	nop; 	cmp_grt.i32  VP1, V2, V3, VP4; 	nop
// CHECK: nop; 	nop; 	cmp_grt.i32  VP1, V2, V3, !VP4; 	nop


NOP; NOP; CMP_GRT.I32 VP1, V2, S3; NOP
NOP; NOP; CMP_GRT.I32 VP1, V2, S3, SP4; NOP
NOP; NOP; CMP_GRT.I32 VP1, V2, S3, !SP4; NOP
NOP; NOP; CMP_GRT.I32 VP1, V2, S3, VP4; NOP
NOP; NOP; CMP_GRT.I32 VP1, V2, S3, !VP4; NOP

// CHECK: nop; 	nop; 	cmp_grt.i32  VP1, V2, S3, SP0; 	nop
// CHECK: nop; 	nop; 	cmp_grt.i32  VP1, V2, S3, SP4; 	nop
// CHECK: nop; 	nop; 	cmp_grt.i32  VP1, V2, S3, !SP4; 	nop
// CHECK: nop; 	nop; 	cmp_grt.i32  VP1, V2, S3, VP4; 	nop
// CHECK: nop; 	nop; 	cmp_grt.i32  VP1, V2, S3, !VP4; 	nop


NOP; NOP; CMP_GRT.I32 VP1, V2, 123; NOP
NOP; NOP; CMP_GRT.I32 VP1, V2, 123, SP4; NOP
NOP; NOP; CMP_GRT.I32 VP1, V2, 123, !SP4; NOP
NOP; NOP; CMP_GRT.I32 VP1, V2, 123, VP4; NOP
NOP; NOP; CMP_GRT.I32 VP1, V2, 123, !VP4; NOP

// CHECK: nop; 	nop; 	cmp_grt.i32  VP1, V2, 0x7b, SP0; 	nop
// CHECK: nop; 	nop; 	cmp_grt.i32  VP1, V2, 0x7b, SP4; 	nop
// CHECK: nop; 	nop; 	cmp_grt.i32  VP1, V2, 0x7b, !SP4; 	nop
// CHECK: nop; 	nop; 	cmp_grt.i32  VP1, V2, 0x7b, VP4; 	nop
// CHECK: nop; 	nop; 	cmp_grt.i32  VP1, V2, 0x7b, !VP4; 	nop


NOP; NOP; CMP_GRT.U32 VP1, V2, V3; NOP
NOP; NOP; CMP_GRT.U32 VP1, V2, V3, SP4; NOP
NOP; NOP; CMP_GRT.U32 VP1, V2, V3, !SP4; NOP
NOP; NOP; CMP_GRT.U32 VP1, V2, V3, VP4; NOP
NOP; NOP; CMP_GRT.U32 VP1, V2, V3, !VP4; NOP

// CHECK: nop; 	nop; 	cmp_grt.u32  VP1, V2, V3, SP0; 	nop
// CHECK: nop; 	nop; 	cmp_grt.u32  VP1, V2, V3, SP4; 	nop
// CHECK: nop; 	nop; 	cmp_grt.u32  VP1, V2, V3, !SP4; 	nop
// CHECK: nop; 	nop; 	cmp_grt.u32  VP1, V2, V3, VP4; 	nop
// CHECK: nop; 	nop; 	cmp_grt.u32  VP1, V2, V3, !VP4; 	nop


NOP; NOP; CMP_GRT.U32 VP1, V2, S3; NOP
NOP; NOP; CMP_GRT.U32 VP1, V2, S3, SP4; NOP
NOP; NOP; CMP_GRT.U32 VP1, V2, S3, !SP4; NOP
NOP; NOP; CMP_GRT.U32 VP1, V2, S3, VP4; NOP
NOP; NOP; CMP_GRT.U32 VP1, V2, S3, !VP4; NOP

// CHECK: nop; 	nop; 	cmp_grt.u32  VP1, V2, S3, SP0; 	nop
// CHECK: nop; 	nop; 	cmp_grt.u32  VP1, V2, S3, SP4; 	nop
// CHECK: nop; 	nop; 	cmp_grt.u32  VP1, V2, S3, !SP4; 	nop
// CHECK: nop; 	nop; 	cmp_grt.u32  VP1, V2, S3, VP4; 	nop
// CHECK: nop; 	nop; 	cmp_grt.u32  VP1, V2, S3, !VP4; 	nop


NOP; NOP; CMP_GRT.U32 VP1, V2, 123; NOP
NOP; NOP; CMP_GRT.U32 VP1, V2, 123, SP4; NOP
NOP; NOP; CMP_GRT.U32 VP1, V2, 123, !SP4; NOP
NOP; NOP; CMP_GRT.U32 VP1, V2, 123, VP4; NOP
NOP; NOP; CMP_GRT.U32 VP1, V2, 123, !VP4; NOP

// CHECK: nop; 	nop; 	cmp_grt.u32  VP1, V2, 0x7b, SP0; 	nop
// CHECK: nop; 	nop; 	cmp_grt.u32  VP1, V2, 0x7b, SP4; 	nop
// CHECK: nop; 	nop; 	cmp_grt.u32  VP1, V2, 0x7b, !SP4; 	nop
// CHECK: nop; 	nop; 	cmp_grt.u32  VP1, V2, 0x7b, VP4; 	nop
// CHECK: nop; 	nop; 	cmp_grt.u32  VP1, V2, 0x7b, !VP4; 	nop


NOP; NOP; CMP_GRT.I16 VP1, V2, V3; NOP
NOP; NOP; CMP_GRT.I16 VP1, V2, V3, SP4; NOP
NOP; NOP; CMP_GRT.I16 VP1, V2, V3, !SP4; NOP
NOP; NOP; CMP_GRT.I16 VP1, V2, V3, VP4; NOP
NOP; NOP; CMP_GRT.I16 VP1, V2, V3, !VP4; NOP

// CHECK: nop; 	nop; 	cmp_grt.i16  VP1, V2, V3, SP0; 	nop
// CHECK: nop; 	nop; 	cmp_grt.i16  VP1, V2, V3, SP4; 	nop
// CHECK: nop; 	nop; 	cmp_grt.i16  VP1, V2, V3, !SP4; 	nop
// CHECK: nop; 	nop; 	cmp_grt.i16  VP1, V2, V3, VP4; 	nop
// CHECK: nop; 	nop; 	cmp_grt.i16  VP1, V2, V3, !VP4; 	nop


NOP; NOP; CMP_GRT.I16 VP1, V2, S3; NOP
NOP; NOP; CMP_GRT.I16 VP1, V2, S3, SP4; NOP
NOP; NOP; CMP_GRT.I16 VP1, V2, S3, !SP4; NOP
NOP; NOP; CMP_GRT.I16 VP1, V2, S3, VP4; NOP
NOP; NOP; CMP_GRT.I16 VP1, V2, S3, !VP4; NOP

// CHECK: nop; 	nop; 	cmp_grt.i16  VP1, V2, S3, SP0; 	nop
// CHECK: nop; 	nop; 	cmp_grt.i16  VP1, V2, S3, SP4; 	nop
// CHECK: nop; 	nop; 	cmp_grt.i16  VP1, V2, S3, !SP4; 	nop
// CHECK: nop; 	nop; 	cmp_grt.i16  VP1, V2, S3, VP4; 	nop
// CHECK: nop; 	nop; 	cmp_grt.i16  VP1, V2, S3, !VP4; 	nop


NOP; NOP; CMP_GRT.I16 VP1, V2, 123; NOP
NOP; NOP; CMP_GRT.I16 VP1, V2, 123, SP4; NOP
NOP; NOP; CMP_GRT.I16 VP1, V2, 123, !SP4; NOP
NOP; NOP; CMP_GRT.I16 VP1, V2, 123, VP4; NOP
NOP; NOP; CMP_GRT.I16 VP1, V2, 123, !VP4; NOP

// CHECK: nop; 	nop; 	cmp_grt.i16  VP1, V2, 0x7b, SP0; 	nop
// CHECK: nop; 	nop; 	cmp_grt.i16  VP1, V2, 0x7b, SP4; 	nop
// CHECK: nop; 	nop; 	cmp_grt.i16  VP1, V2, 0x7b, !SP4; 	nop
// CHECK: nop; 	nop; 	cmp_grt.i16  VP1, V2, 0x7b, VP4; 	nop
// CHECK: nop; 	nop; 	cmp_grt.i16  VP1, V2, 0x7b, !VP4; 	nop


NOP; NOP; CMP_GRT.U16 VP1, V2, V3; NOP
NOP; NOP; CMP_GRT.U16 VP1, V2, V3, SP4; NOP
NOP; NOP; CMP_GRT.U16 VP1, V2, V3, !SP4; NOP
NOP; NOP; CMP_GRT.U16 VP1, V2, V3, VP4; NOP
NOP; NOP; CMP_GRT.U16 VP1, V2, V3, !VP4; NOP

// CHECK: nop; 	nop; 	cmp_grt.u16  VP1, V2, V3, SP0; 	nop
// CHECK: nop; 	nop; 	cmp_grt.u16  VP1, V2, V3, SP4; 	nop
// CHECK: nop; 	nop; 	cmp_grt.u16  VP1, V2, V3, !SP4; 	nop
// CHECK: nop; 	nop; 	cmp_grt.u16  VP1, V2, V3, VP4; 	nop
// CHECK: nop; 	nop; 	cmp_grt.u16  VP1, V2, V3, !VP4; 	nop


NOP; NOP; CMP_GRT.U16 VP1, V2, S3; NOP
NOP; NOP; CMP_GRT.U16 VP1, V2, S3, SP4; NOP
NOP; NOP; CMP_GRT.U16 VP1, V2, S3, !SP4; NOP
NOP; NOP; CMP_GRT.U16 VP1, V2, S3, VP4; NOP
NOP; NOP; CMP_GRT.U16 VP1, V2, S3, !VP4; NOP

// CHECK: nop; 	nop; 	cmp_grt.u16  VP1, V2, S3, SP0; 	nop
// CHECK: nop; 	nop; 	cmp_grt.u16  VP1, V2, S3, SP4; 	nop
// CHECK: nop; 	nop; 	cmp_grt.u16  VP1, V2, S3, !SP4; 	nop
// CHECK: nop; 	nop; 	cmp_grt.u16  VP1, V2, S3, VP4; 	nop
// CHECK: nop; 	nop; 	cmp_grt.u16  VP1, V2, S3, !VP4; 	nop


NOP; NOP; CMP_GRT.U16 VP1, V2, 123; NOP
NOP; NOP; CMP_GRT.U16 VP1, V2, 123, SP4; NOP
NOP; NOP; CMP_GRT.U16 VP1, V2, 123, !SP4; NOP
NOP; NOP; CMP_GRT.U16 VP1, V2, 123, VP4; NOP
NOP; NOP; CMP_GRT.U16 VP1, V2, 123, !VP4; NOP

// CHECK: nop; 	nop; 	cmp_grt.u16  VP1, V2, 0x7b, SP0; 	nop
// CHECK: nop; 	nop; 	cmp_grt.u16  VP1, V2, 0x7b, SP4; 	nop
// CHECK: nop; 	nop; 	cmp_grt.u16  VP1, V2, 0x7b, !SP4; 	nop
// CHECK: nop; 	nop; 	cmp_grt.u16  VP1, V2, 0x7b, VP4; 	nop
// CHECK: nop; 	nop; 	cmp_grt.u16  VP1, V2, 0x7b, !VP4; 	nop


NOP; NOP; CMP_GRT.I8 VP1, V2, V3; NOP
NOP; NOP; CMP_GRT.I8 VP1, V2, V3, SP4; NOP
NOP; NOP; CMP_GRT.I8 VP1, V2, V3, !SP4; NOP
NOP; NOP; CMP_GRT.I8 VP1, V2, V3, VP4; NOP
NOP; NOP; CMP_GRT.I8 VP1, V2, V3, !VP4; NOP

// CHECK: nop; 	nop; 	cmp_grt.i8  VP1, V2, V3, SP0; 	nop
// CHECK: nop; 	nop; 	cmp_grt.i8  VP1, V2, V3, SP4; 	nop
// CHECK: nop; 	nop; 	cmp_grt.i8  VP1, V2, V3, !SP4; 	nop
// CHECK: nop; 	nop; 	cmp_grt.i8  VP1, V2, V3, VP4; 	nop
// CHECK: nop; 	nop; 	cmp_grt.i8  VP1, V2, V3, !VP4; 	nop


NOP; NOP; CMP_GRT.I8 VP1, V2, S3; NOP
NOP; NOP; CMP_GRT.I8 VP1, V2, S3, SP4; NOP
NOP; NOP; CMP_GRT.I8 VP1, V2, S3, !SP4; NOP
NOP; NOP; CMP_GRT.I8 VP1, V2, S3, VP4; NOP
NOP; NOP; CMP_GRT.I8 VP1, V2, S3, !VP4; NOP

// CHECK: nop; 	nop; 	cmp_grt.i8  VP1, V2, S3, SP0; 	nop
// CHECK: nop; 	nop; 	cmp_grt.i8  VP1, V2, S3, SP4; 	nop
// CHECK: nop; 	nop; 	cmp_grt.i8  VP1, V2, S3, !SP4; 	nop
// CHECK: nop; 	nop; 	cmp_grt.i8  VP1, V2, S3, VP4; 	nop
// CHECK: nop; 	nop; 	cmp_grt.i8  VP1, V2, S3, !VP4; 	nop


NOP; NOP; CMP_GRT.I8 VP1, V2, 123; NOP
NOP; NOP; CMP_GRT.I8 VP1, V2, 123, SP4; NOP
NOP; NOP; CMP_GRT.I8 VP1, V2, 123, !SP4; NOP
NOP; NOP; CMP_GRT.I8 VP1, V2, 123, VP4; NOP
NOP; NOP; CMP_GRT.I8 VP1, V2, 123, !VP4; NOP

// CHECK: nop; 	nop; 	cmp_grt.i8  VP1, V2, 0x7b, SP0; 	nop
// CHECK: nop; 	nop; 	cmp_grt.i8  VP1, V2, 0x7b, SP4; 	nop
// CHECK: nop; 	nop; 	cmp_grt.i8  VP1, V2, 0x7b, !SP4; 	nop
// CHECK: nop; 	nop; 	cmp_grt.i8  VP1, V2, 0x7b, VP4; 	nop
// CHECK: nop; 	nop; 	cmp_grt.i8  VP1, V2, 0x7b, !VP4; 	nop


NOP; NOP; CMP_GRT.U8 VP1, V2, V3; NOP
NOP; NOP; CMP_GRT.U8 VP1, V2, V3, SP4; NOP
NOP; NOP; CMP_GRT.U8 VP1, V2, V3, !SP4; NOP
NOP; NOP; CMP_GRT.U8 VP1, V2, V3, VP4; NOP
NOP; NOP; CMP_GRT.U8 VP1, V2, V3, !VP4; NOP

// CHECK: nop; 	nop; 	cmp_grt.u8  VP1, V2, V3, SP0; 	nop
// CHECK: nop; 	nop; 	cmp_grt.u8  VP1, V2, V3, SP4; 	nop
// CHECK: nop; 	nop; 	cmp_grt.u8  VP1, V2, V3, !SP4; 	nop
// CHECK: nop; 	nop; 	cmp_grt.u8  VP1, V2, V3, VP4; 	nop
// CHECK: nop; 	nop; 	cmp_grt.u8  VP1, V2, V3, !VP4; 	nop


NOP; NOP; CMP_GRT.U8 VP1, V2, S3; NOP
NOP; NOP; CMP_GRT.U8 VP1, V2, S3, SP4; NOP
NOP; NOP; CMP_GRT.U8 VP1, V2, S3, !SP4; NOP
NOP; NOP; CMP_GRT.U8 VP1, V2, S3, VP4; NOP
NOP; NOP; CMP_GRT.U8 VP1, V2, S3, !VP4; NOP

// CHECK: nop; 	nop; 	cmp_grt.u8  VP1, V2, S3, SP0; 	nop
// CHECK: nop; 	nop; 	cmp_grt.u8  VP1, V2, S3, SP4; 	nop
// CHECK: nop; 	nop; 	cmp_grt.u8  VP1, V2, S3, !SP4; 	nop
// CHECK: nop; 	nop; 	cmp_grt.u8  VP1, V2, S3, VP4; 	nop
// CHECK: nop; 	nop; 	cmp_grt.u8  VP1, V2, S3, !VP4; 	nop


NOP; NOP; CMP_GRT.U8 VP1, V2, 123; NOP
NOP; NOP; CMP_GRT.U8 VP1, V2, 123, SP4; NOP
NOP; NOP; CMP_GRT.U8 VP1, V2, 123, !SP4; NOP
NOP; NOP; CMP_GRT.U8 VP1, V2, 123, VP4; NOP
NOP; NOP; CMP_GRT.U8 VP1, V2, 123, !VP4; NOP

// CHECK: nop; 	nop; 	cmp_grt.u8  VP1, V2, 0x7b, SP0; 	nop
// CHECK: nop; 	nop; 	cmp_grt.u8  VP1, V2, 0x7b, SP4; 	nop
// CHECK: nop; 	nop; 	cmp_grt.u8  VP1, V2, 0x7b, !SP4; 	nop
// CHECK: nop; 	nop; 	cmp_grt.u8  VP1, V2, 0x7b, VP4; 	nop
// CHECK: nop; 	nop; 	cmp_grt.u8  VP1, V2, 0x7b, !VP4; 	nop
