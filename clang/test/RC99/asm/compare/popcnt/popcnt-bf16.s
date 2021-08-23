// RUN: %clang -cc1as -triple tpc-none-none -target-cpu gaudi -filetype obj %s -o %t.o
// RUN: %disasm -mcpu=gaudi %t.o | FileCheck %s



NOP; NOP; POPCNT.BF16 0 V1, V2; NOP
NOP; NOP; POPCNT.BF16 0 V1, V2, SP3; NOP
NOP; NOP; POPCNT.BF16 0 V1, V2, !SP3; NOP
NOP; NOP; POPCNT.BF16 0 V1, V2, VP3; NOP
NOP; NOP; POPCNT.BF16 0 V1, V2, !VP3; NOP
NOP; NOP; POPCNT.BF16 1 V1, V2; NOP
NOP; NOP; POPCNT.BF16 1 V1, V2, SP3; NOP
NOP; NOP; POPCNT.BF16 1 V1, V2, !SP3; NOP
NOP; NOP; POPCNT.BF16 1 V1, V2, VP3; NOP
NOP; NOP; POPCNT.BF16 1 V1, V2, !VP3; NOP

// CHECK: nop; 	nop; 	popcnt.bf16 set=0 V1, V2, SP0; 	nop
// CHECK: nop; 	nop; 	popcnt.bf16 set=0 V1, V2, SP3; 	nop
// CHECK: nop; 	nop; 	popcnt.bf16 set=0 V1, V2, !SP3; 	nop
// CHECK: nop; 	nop; 	popcnt.bf16 set=0 V1, V2, VP3; 	nop
// CHECK: nop; 	nop; 	popcnt.bf16 set=0 V1, V2, !VP3; 	nop
// CHECK: nop; 	nop; 	popcnt.bf16 set=1 V1, V2, SP0; 	nop
// CHECK: nop; 	nop; 	popcnt.bf16 set=1 V1, V2, SP3; 	nop
// CHECK: nop; 	nop; 	popcnt.bf16 set=1 V1, V2, !SP3; 	nop
// CHECK: nop; 	nop; 	popcnt.bf16 set=1 V1, V2, VP3; 	nop
// CHECK: nop; 	nop; 	popcnt.bf16 set=1 V1, V2, !VP3; 	nop


NOP; NOP; POPCNT.I32 0 V1, V2; NOP
NOP; NOP; POPCNT.I32 0 V1, V2, SP3; NOP
NOP; NOP; POPCNT.I32 0 V1, V2, !SP3; NOP
NOP; NOP; POPCNT.I32 0 V1, V2, VP3; NOP
NOP; NOP; POPCNT.I32 0 V1, V2, !VP3; NOP
NOP; NOP; POPCNT.I32 1 V1, V2; NOP
NOP; NOP; POPCNT.I32 1 V1, V2, SP3; NOP
NOP; NOP; POPCNT.I32 1 V1, V2, !SP3; NOP
NOP; NOP; POPCNT.I32 1 V1, V2, VP3; NOP
NOP; NOP; POPCNT.I32 1 V1, V2, !VP3; NOP

// CHECK: nop; 	nop; 	popcnt.i32 set=0 V1, V2, SP0; 	nop
// CHECK: nop; 	nop; 	popcnt.i32 set=0 V1, V2, SP3; 	nop
// CHECK: nop; 	nop; 	popcnt.i32 set=0 V1, V2, !SP3; 	nop
// CHECK: nop; 	nop; 	popcnt.i32 set=0 V1, V2, VP3; 	nop
// CHECK: nop; 	nop; 	popcnt.i32 set=0 V1, V2, !VP3; 	nop
// CHECK: nop; 	nop; 	popcnt.i32  set=1 V1, V2, SP0; 	nop
// CHECK: nop; 	nop; 	popcnt.i32  set=1 V1, V2, SP3; 	nop
// CHECK: nop; 	nop; 	popcnt.i32  set=1 V1, V2, !SP3; 	nop
// CHECK: nop; 	nop; 	popcnt.i32  set=1 V1, V2, VP3; 	nop
// CHECK: nop; 	nop; 	popcnt.i32  set=1 V1, V2, !VP3; 	nop


NOP; NOP; POPCNT.U32 0 V1, V2; NOP
NOP; NOP; POPCNT.U32 0 V1, V2, SP3; NOP
NOP; NOP; POPCNT.U32 0 V1, V2, !SP3; NOP
NOP; NOP; POPCNT.U32 0 V1, V2, VP3; NOP
NOP; NOP; POPCNT.U32 0 V1, V2, !VP3; NOP
NOP; NOP; POPCNT.U32 1 V1, V2; NOP
NOP; NOP; POPCNT.U32 1 V1, V2, SP3; NOP
NOP; NOP; POPCNT.U32 1 V1, V2, !SP3; NOP
NOP; NOP; POPCNT.U32 1 V1, V2, VP3; NOP
NOP; NOP; POPCNT.U32 1 V1, V2, !VP3; NOP

// CHECK: nop; 	nop; 	popcnt.u32 set=0 V1, V2, SP0; 	nop
// CHECK: nop; 	nop; 	popcnt.u32 set=0 V1, V2, SP3; 	nop
// CHECK: nop; 	nop; 	popcnt.u32 set=0 V1, V2, !SP3; 	nop
// CHECK: nop; 	nop; 	popcnt.u32 set=0 V1, V2, VP3; 	nop
// CHECK: nop; 	nop; 	popcnt.u32 set=0 V1, V2, !VP3; 	nop
// CHECK: nop; 	nop; 	popcnt.u32  set=1 V1, V2, SP0; 	nop
// CHECK: nop; 	nop; 	popcnt.u32  set=1 V1, V2, SP3; 	nop
// CHECK: nop; 	nop; 	popcnt.u32  set=1 V1, V2, !SP3; 	nop
// CHECK: nop; 	nop; 	popcnt.u32  set=1 V1, V2, VP3; 	nop
// CHECK: nop; 	nop; 	popcnt.u32  set=1 V1, V2, !VP3; 	nop


NOP; NOP; POPCNT.I16 0 V1, V2; NOP
NOP; NOP; POPCNT.I16 0 V1, V2, SP3; NOP
NOP; NOP; POPCNT.I16 0 V1, V2, !SP3; NOP
NOP; NOP; POPCNT.I16 0 V1, V2, VP3; NOP
NOP; NOP; POPCNT.I16 0 V1, V2, !VP3; NOP
NOP; NOP; POPCNT.I16 1 V1, V2; NOP
NOP; NOP; POPCNT.I16 1 V1, V2, SP3; NOP
NOP; NOP; POPCNT.I16 1 V1, V2, !SP3; NOP
NOP; NOP; POPCNT.I16 1 V1, V2, VP3; NOP
NOP; NOP; POPCNT.I16 1 V1, V2, !VP3; NOP

// CHECK: nop; 	nop; 	popcnt.i16 set=0 V1, V2, SP0; 	nop
// CHECK: nop; 	nop; 	popcnt.i16 set=0 V1, V2, SP3; 	nop
// CHECK: nop; 	nop; 	popcnt.i16 set=0 V1, V2, !SP3; 	nop
// CHECK: nop; 	nop; 	popcnt.i16 set=0 V1, V2, VP3; 	nop
// CHECK: nop; 	nop; 	popcnt.i16 set=0 V1, V2, !VP3; 	nop
// CHECK: nop; 	nop; 	popcnt.i16  set=1 V1, V2, SP0; 	nop
// CHECK: nop; 	nop; 	popcnt.i16  set=1 V1, V2, SP3; 	nop
// CHECK: nop; 	nop; 	popcnt.i16  set=1 V1, V2, !SP3; 	nop
// CHECK: nop; 	nop; 	popcnt.i16  set=1 V1, V2, VP3; 	nop
// CHECK: nop; 	nop; 	popcnt.i16  set=1 V1, V2, !VP3; 	nop


NOP; NOP; POPCNT.U16 0 V1, V2; NOP
NOP; NOP; POPCNT.U16 0 V1, V2, SP3; NOP
NOP; NOP; POPCNT.U16 0 V1, V2, !SP3; NOP
NOP; NOP; POPCNT.U16 0 V1, V2, VP3; NOP
NOP; NOP; POPCNT.U16 0 V1, V2, !VP3; NOP
NOP; NOP; POPCNT.U16 1 V1, V2; NOP
NOP; NOP; POPCNT.U16 1 V1, V2, SP3; NOP
NOP; NOP; POPCNT.U16 1 V1, V2, !SP3; NOP
NOP; NOP; POPCNT.U16 1 V1, V2, VP3; NOP
NOP; NOP; POPCNT.U16 1 V1, V2, !VP3; NOP

// CHECK: nop; 	nop; 	popcnt.u16 set=0 V1, V2, SP0; 	nop
// CHECK: nop; 	nop; 	popcnt.u16 set=0 V1, V2, SP3; 	nop
// CHECK: nop; 	nop; 	popcnt.u16 set=0 V1, V2, !SP3; 	nop
// CHECK: nop; 	nop; 	popcnt.u16 set=0 V1, V2, VP3; 	nop
// CHECK: nop; 	nop; 	popcnt.u16 set=0 V1, V2, !VP3; 	nop
// CHECK: nop; 	nop; 	popcnt.u16  set=1 V1, V2, SP0; 	nop
// CHECK: nop; 	nop; 	popcnt.u16  set=1 V1, V2, SP3; 	nop
// CHECK: nop; 	nop; 	popcnt.u16  set=1 V1, V2, !SP3; 	nop
// CHECK: nop; 	nop; 	popcnt.u16  set=1 V1, V2, VP3; 	nop
// CHECK: nop; 	nop; 	popcnt.u16  set=1 V1, V2, !VP3; 	nop


NOP; NOP; POPCNT.I8 0 V1, V2; NOP
NOP; NOP; POPCNT.I8 0 V1, V2, SP3; NOP
NOP; NOP; POPCNT.I8 0 V1, V2, !SP3; NOP
NOP; NOP; POPCNT.I8 0 V1, V2, VP3; NOP
NOP; NOP; POPCNT.I8 0 V1, V2, !VP3; NOP
NOP; NOP; POPCNT.I8 1 V1, V2; NOP
NOP; NOP; POPCNT.I8 1 V1, V2, SP3; NOP
NOP; NOP; POPCNT.I8 1 V1, V2, !SP3; NOP
NOP; NOP; POPCNT.I8 1 V1, V2, VP3; NOP
NOP; NOP; POPCNT.I8 1 V1, V2, !VP3; NOP

// CHECK: nop; 	nop; 	popcnt.i8 set=0 V1, V2, SP0; 	nop
// CHECK: nop; 	nop; 	popcnt.i8 set=0 V1, V2, SP3; 	nop
// CHECK: nop; 	nop; 	popcnt.i8 set=0 V1, V2, !SP3; 	nop
// CHECK: nop; 	nop; 	popcnt.i8 set=0 V1, V2, VP3; 	nop
// CHECK: nop; 	nop; 	popcnt.i8 set=0 V1, V2, !VP3; 	nop
// CHECK: nop; 	nop; 	popcnt.i8  set=1 V1, V2, SP0; 	nop
// CHECK: nop; 	nop; 	popcnt.i8  set=1 V1, V2, SP3; 	nop
// CHECK: nop; 	nop; 	popcnt.i8  set=1 V1, V2, !SP3; 	nop
// CHECK: nop; 	nop; 	popcnt.i8  set=1 V1, V2, VP3; 	nop
// CHECK: nop; 	nop; 	popcnt.i8  set=1 V1, V2, !VP3; 	nop


NOP; NOP; POPCNT.U8 0 V1, V2; NOP
NOP; NOP; POPCNT.U8 0 V1, V2, SP3; NOP
NOP; NOP; POPCNT.U8 0 V1, V2, !SP3; NOP
NOP; NOP; POPCNT.U8 0 V1, V2, VP3; NOP
NOP; NOP; POPCNT.U8 0 V1, V2, !VP3; NOP
NOP; NOP; POPCNT.U8 1 V1, V2; NOP
NOP; NOP; POPCNT.U8 1 V1, V2, SP3; NOP
NOP; NOP; POPCNT.U8 1 V1, V2, !SP3; NOP
NOP; NOP; POPCNT.U8 1 V1, V2, VP3; NOP
NOP; NOP; POPCNT.U8 1 V1, V2, !VP3; NOP

// CHECK: nop; 	nop; 	popcnt.u8 set=0 V1, V2, SP0; 	nop
// CHECK: nop; 	nop; 	popcnt.u8 set=0 V1, V2, SP3; 	nop
// CHECK: nop; 	nop; 	popcnt.u8 set=0 V1, V2, !SP3; 	nop
// CHECK: nop; 	nop; 	popcnt.u8 set=0 V1, V2, VP3; 	nop
// CHECK: nop; 	nop; 	popcnt.u8 set=0 V1, V2, !VP3; 	nop
// CHECK: nop; 	nop; 	popcnt.u8  set=1 V1, V2, SP0; 	nop
// CHECK: nop; 	nop; 	popcnt.u8  set=1 V1, V2, SP3; 	nop
// CHECK: nop; 	nop; 	popcnt.u8  set=1 V1, V2, !SP3; 	nop
// CHECK: nop; 	nop; 	popcnt.u8  set=1 V1, V2, VP3; 	nop
// CHECK: nop; 	nop; 	popcnt.u8  set=1 V1, V2, !VP3; 	nop
