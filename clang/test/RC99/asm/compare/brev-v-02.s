// RUN: %clang -cc1as -triple tpc-none-none -target-cpu gaudi -filetype obj %s -o %t.o
// RUN: %disasm --mcpu gaudi %t.o | FileCheck %s


NOP; NOP; BREV.F32 V1, V2
NOP; NOP; BREV.F32 V1, V2, SP3
NOP; NOP; BREV.F32 V1, V2, !SP3
NOP; NOP; BREV.F32 V1, V2, VP3
NOP; NOP; BREV.F32 V1, V2, !VP3

// CHECK: 	nop; 	nop; 	brev.f32  V1, V2; 	nop
// CHECK: 	nop; 	nop; 	brev.f32  V1, V2, SP3; 	nop
// CHECK: 	nop; 	nop; 	brev.f32  V1, V2, !SP3; 	nop
// CHECK: 	nop; 	nop; 	brev.f32  V1, V2, VP3; 	nop
// CHECK: 	nop; 	nop; 	brev.f32  V1, V2, !VP3; 	nop


NOP; NOP; BREV.BF16 V1, V2
NOP; NOP; BREV.BF16 V1, V2, SP3
NOP; NOP; BREV.BF16 V1, V2, !SP3
NOP; NOP; BREV.BF16 V1, V2, VP3
NOP; NOP; BREV.BF16 V1, V2, !VP3

// CHECK: 	nop; 	nop; 	brev.bf16  V1, V2; 	nop
// CHECK: 	nop; 	nop; 	brev.bf16  V1, V2, SP3; 	nop
// CHECK: 	nop; 	nop; 	brev.bf16  V1, V2, !SP3; 	nop
// CHECK: 	nop; 	nop; 	brev.bf16  V1, V2, VP3; 	nop
// CHECK: 	nop; 	nop; 	brev.bf16  V1, V2, !VP3; 	nop


NOP; NOP; BREV.I32 V1, V2
NOP; NOP; BREV.I32 V1, V2, SP3
NOP; NOP; BREV.I32 V1, V2, !SP3
NOP; NOP; BREV.I32 V1, V2, VP3
NOP; NOP; BREV.I32 V1, V2, !VP3

// CHECK: 	nop; 	nop; 	brev.i32  V1, V2; 	nop
// CHECK: 	nop; 	nop; 	brev.i32  V1, V2, SP3; 	nop
// CHECK: 	nop; 	nop; 	brev.i32  V1, V2, !SP3; 	nop
// CHECK: 	nop; 	nop; 	brev.i32  V1, V2, VP3; 	nop
// CHECK: 	nop; 	nop; 	brev.i32  V1, V2, !VP3; 	nop


NOP; NOP; BREV.U32 V1, V2
NOP; NOP; BREV.U32 V1, V2, SP3
NOP; NOP; BREV.U32 V1, V2, !SP3
NOP; NOP; BREV.U32 V1, V2, VP3
NOP; NOP; BREV.U32 V1, V2, !VP3

// CHECK: 	nop; 	nop; 	brev.u32  V1, V2; 	nop
// CHECK: 	nop; 	nop; 	brev.u32  V1, V2, SP3; 	nop
// CHECK: 	nop; 	nop; 	brev.u32  V1, V2, !SP3; 	nop
// CHECK: 	nop; 	nop; 	brev.u32  V1, V2, VP3; 	nop
// CHECK: 	nop; 	nop; 	brev.u32  V1, V2, !VP3; 	nop


NOP; NOP; BREV.I16 V1, V2
NOP; NOP; BREV.I16 V1, V2, SP3
NOP; NOP; BREV.I16 V1, V2, !SP3
NOP; NOP; BREV.I16 V1, V2, VP3
NOP; NOP; BREV.I16 V1, V2, !VP3

// CHECK: 	nop; 	nop; 	brev.i16  V1, V2; 	nop
// CHECK: 	nop; 	nop; 	brev.i16  V1, V2, SP3; 	nop
// CHECK: 	nop; 	nop; 	brev.i16  V1, V2, !SP3; 	nop
// CHECK: 	nop; 	nop; 	brev.i16  V1, V2, VP3; 	nop
// CHECK: 	nop; 	nop; 	brev.i16  V1, V2, !VP3; 	nop


NOP; NOP; BREV.U16 V1, V2
NOP; NOP; BREV.U16 V1, V2, SP3
NOP; NOP; BREV.U16 V1, V2, !SP3
NOP; NOP; BREV.U16 V1, V2, VP3
NOP; NOP; BREV.U16 V1, V2, !VP3

// CHECK: 	nop; 	nop; 	brev.u16  V1, V2; 	nop
// CHECK: 	nop; 	nop; 	brev.u16  V1, V2, SP3; 	nop
// CHECK: 	nop; 	nop; 	brev.u16  V1, V2, !SP3; 	nop
// CHECK: 	nop; 	nop; 	brev.u16  V1, V2, VP3; 	nop
// CHECK: 	nop; 	nop; 	brev.u16  V1, V2, !VP3; 	nop


NOP; NOP; BREV.I8 V1, V2
NOP; NOP; BREV.I8 V1, V2, SP3
NOP; NOP; BREV.I8 V1, V2, !SP3
NOP; NOP; BREV.I8 V1, V2, VP3
NOP; NOP; BREV.I8 V1, V2, !VP3

// CHECK: 	nop; 	nop; 	brev.i8  V1, V2; 	nop
// CHECK: 	nop; 	nop; 	brev.i8  V1, V2, SP3; 	nop
// CHECK: 	nop; 	nop; 	brev.i8  V1, V2, !SP3; 	nop
// CHECK: 	nop; 	nop; 	brev.i8  V1, V2, VP3; 	nop
// CHECK: 	nop; 	nop; 	brev.i8  V1, V2, !VP3; 	nop


NOP; NOP; BREV.U8 V1, V2
NOP; NOP; BREV.U8 V1, V2, SP3
NOP; NOP; BREV.U8 V1, V2, !SP3
NOP; NOP; BREV.U8 V1, V2, VP3
NOP; NOP; BREV.U8 V1, V2, !VP3

// CHECK: 	nop; 	nop; 	brev.u8  V1, V2; 	nop
// CHECK: 	nop; 	nop; 	brev.u8  V1, V2, SP3; 	nop
// CHECK: 	nop; 	nop; 	brev.u8  V1, V2, !SP3; 	nop
// CHECK: 	nop; 	nop; 	brev.u8  V1, V2, VP3; 	nop
// CHECK: 	nop; 	nop; 	brev.u8  V1, V2, !VP3; 	nop


NOP; NOP; BREV.F32 V1, S2
NOP; NOP; BREV.F32 V1, S2, SP3
NOP; NOP; BREV.F32 V1, S2, !SP3
NOP; NOP; BREV.F32 V1, S2, VP3
NOP; NOP; BREV.F32 V1, S2, !VP3

// CHECK: 	nop; 	nop; 	brev.f32  V1, S2; 	nop
// CHECK: 	nop; 	nop; 	brev.f32  V1, S2, SP3; 	nop
// CHECK: 	nop; 	nop; 	brev.f32  V1, S2, !SP3; 	nop
// CHECK: 	nop; 	nop; 	brev.f32  V1, S2, VP3; 	nop
// CHECK: 	nop; 	nop; 	brev.f32  V1, S2, !VP3; 	nop


NOP; NOP; BREV.BF16 V1, S2
NOP; NOP; BREV.BF16 V1, S2, SP3
NOP; NOP; BREV.BF16 V1, S2, !SP3
NOP; NOP; BREV.BF16 V1, S2, VP3
NOP; NOP; BREV.BF16 V1, S2, !VP3

// CHECK: 	nop; 	nop; 	brev.bf16  V1, S2; 	nop
// CHECK: 	nop; 	nop; 	brev.bf16  V1, S2, SP3; 	nop
// CHECK: 	nop; 	nop; 	brev.bf16  V1, S2, !SP3; 	nop
// CHECK: 	nop; 	nop; 	brev.bf16  V1, S2, VP3; 	nop
// CHECK: 	nop; 	nop; 	brev.bf16  V1, S2, !VP3; 	nop


NOP; NOP; BREV.I32 V1, S2
NOP; NOP; BREV.I32 V1, S2, SP3
NOP; NOP; BREV.I32 V1, S2, !SP3
NOP; NOP; BREV.I32 V1, S2, VP3
NOP; NOP; BREV.I32 V1, S2, !VP3

// CHECK: 	nop; 	nop; 	brev.i32  V1, S2; 	nop
// CHECK: 	nop; 	nop; 	brev.i32  V1, S2, SP3; 	nop
// CHECK: 	nop; 	nop; 	brev.i32  V1, S2, !SP3; 	nop
// CHECK: 	nop; 	nop; 	brev.i32  V1, S2, VP3; 	nop
// CHECK: 	nop; 	nop; 	brev.i32  V1, S2, !VP3; 	nop


NOP; NOP; BREV.U32 V1, S2
NOP; NOP; BREV.U32 V1, S2, SP3
NOP; NOP; BREV.U32 V1, S2, !SP3
NOP; NOP; BREV.U32 V1, S2, VP3
NOP; NOP; BREV.U32 V1, S2, !VP3

// CHECK: 	nop; 	nop; 	brev.u32  V1, S2; 	nop
// CHECK: 	nop; 	nop; 	brev.u32  V1, S2, SP3; 	nop
// CHECK: 	nop; 	nop; 	brev.u32  V1, S2, !SP3; 	nop
// CHECK: 	nop; 	nop; 	brev.u32  V1, S2, VP3; 	nop
// CHECK: 	nop; 	nop; 	brev.u32  V1, S2, !VP3; 	nop


NOP; NOP; BREV.I16 V1, S2
NOP; NOP; BREV.I16 V1, S2, SP3
NOP; NOP; BREV.I16 V1, S2, !SP3
NOP; NOP; BREV.I16 V1, S2, VP3
NOP; NOP; BREV.I16 V1, S2, !VP3

// CHECK: 	nop; 	nop; 	brev.i16  V1, S2; 	nop
// CHECK: 	nop; 	nop; 	brev.i16  V1, S2, SP3; 	nop
// CHECK: 	nop; 	nop; 	brev.i16  V1, S2, !SP3; 	nop
// CHECK: 	nop; 	nop; 	brev.i16  V1, S2, VP3; 	nop
// CHECK: 	nop; 	nop; 	brev.i16  V1, S2, !VP3; 	nop


NOP; NOP; BREV.U16 V1, S2
NOP; NOP; BREV.U16 V1, S2, SP3
NOP; NOP; BREV.U16 V1, S2, !SP3
NOP; NOP; BREV.U16 V1, S2, VP3
NOP; NOP; BREV.U16 V1, S2, !VP3

// CHECK: 	nop; 	nop; 	brev.u16  V1, S2; 	nop
// CHECK: 	nop; 	nop; 	brev.u16  V1, S2, SP3; 	nop
// CHECK: 	nop; 	nop; 	brev.u16  V1, S2, !SP3; 	nop
// CHECK: 	nop; 	nop; 	brev.u16  V1, S2, VP3; 	nop
// CHECK: 	nop; 	nop; 	brev.u16  V1, S2, !VP3; 	nop


NOP; NOP; BREV.I8 V1, S2
NOP; NOP; BREV.I8 V1, S2, SP3
NOP; NOP; BREV.I8 V1, S2, !SP3
NOP; NOP; BREV.I8 V1, S2, VP3
NOP; NOP; BREV.I8 V1, S2, !VP3

// CHECK: 	nop; 	nop; 	brev.i8  V1, S2; 	nop
// CHECK: 	nop; 	nop; 	brev.i8  V1, S2, SP3; 	nop
// CHECK: 	nop; 	nop; 	brev.i8  V1, S2, !SP3; 	nop
// CHECK: 	nop; 	nop; 	brev.i8  V1, S2, VP3; 	nop
// CHECK: 	nop; 	nop; 	brev.i8  V1, S2, !VP3; 	nop


NOP; NOP; BREV.U8 V1, S2
NOP; NOP; BREV.U8 V1, S2, SP3
NOP; NOP; BREV.U8 V1, S2, !SP3
NOP; NOP; BREV.U8 V1, S2, VP3
NOP; NOP; BREV.U8 V1, S2, !VP3

// CHECK: 	nop; 	nop; 	brev.u8  V1, S2; 	nop
// CHECK: 	nop; 	nop; 	brev.u8  V1, S2, SP3; 	nop
// CHECK: 	nop; 	nop; 	brev.u8  V1, S2, !SP3; 	nop
// CHECK: 	nop; 	nop; 	brev.u8  V1, S2, VP3; 	nop
// CHECK: 	nop; 	nop; 	brev.u8  V1, S2, !VP3; 	nop
