// RUN: %clang -cc1as -triple tpc-none-none -filetype obj %s -o %t.o
// RUN: %disasm %t.o | FileCheck %s
// RUN: %clang -cc1as -triple tpc-none-none -target-cpu gaudi -filetype obj %s -o %t.o
// RUN: %disasm --mcpu=gaudi %t.o | FileCheck %s
// RUN: %clang -cc1as -triple tpc-none-none -target-cpu goya2 -filetype obj %s -o %t.o
// RUN: %disasm --mcpu=goya2 %t.o | FileCheck %s


NOP; FIND_FIRST.F32 0 START_LSB S1, S2; NOP; NOP
NOP; FIND_FIRST.F32 0 START_LSB S1, S2, SP3; NOP; NOP
NOP; FIND_FIRST.F32 0 START_LSB S1, S2, !SP3; NOP; NOP
NOP; FIND_FIRST.F32 1 START_LSB S1, S2; NOP; NOP
NOP; FIND_FIRST.F32 1 START_LSB S1, S2, SP3; NOP; NOP
NOP; FIND_FIRST.F32 1 START_LSB S1, S2, !SP3; NOP; NOP
NOP; FIND_FIRST.F32 0 START_MSB S1, S2; NOP; NOP
NOP; FIND_FIRST.F32 0 START_MSB S1, S2, SP3; NOP; NOP
NOP; FIND_FIRST.F32 0 START_MSB S1, S2, !SP3; NOP; NOP
NOP; FIND_FIRST.F32 1 START_MSB S1, S2; NOP; NOP
NOP; FIND_FIRST.F32 1 START_MSB S1, S2, SP3; NOP; NOP
NOP; FIND_FIRST.F32 1 START_MSB S1, S2, !SP3; NOP; NOP

// CHECK: nop; 	find_first.f32 set=0 start_lsb S1, S2; 	nop; 	nop
// CHECK: nop; 	find_first.f32 set=0 start_lsb S1, S2, SP3; 	nop; 	nop
// CHECK: nop; 	find_first.f32 set=0 start_lsb S1, S2, !SP3; 	nop; 	nop
// CHECK: nop; 	find_first.f32  set=1 start_lsb S1, S2; 	nop; 	nop
// CHECK: nop; 	find_first.f32  set=1 start_lsb S1, S2, SP3; 	nop; 	nop
// CHECK: nop; 	find_first.f32  set=1 start_lsb S1, S2, !SP3; 	nop; 	nop
// CHECK: nop; 	find_first.f32 set=0 start_msb S1, S2; 	nop; 	nop
// CHECK: nop; 	find_first.f32 set=0 start_msb S1, S2, SP3; 	nop; 	nop
// CHECK: nop; 	find_first.f32 set=0 start_msb S1, S2, !SP3; 	nop; 	nop
// CHECK: nop; 	find_first.f32  set=1 start_msb S1, S2; 	nop; 	nop
// CHECK: nop; 	find_first.f32  set=1 start_msb S1, S2, SP3; 	nop; 	nop
// CHECK: nop; 	find_first.f32  set=1 start_msb S1, S2, !SP3; 	nop; 	nop


NOP; FIND_FIRST.I32 0 START_LSB S1, S2; NOP; NOP
NOP; FIND_FIRST.I32 0 START_LSB S1, S2, SP3; NOP; NOP
NOP; FIND_FIRST.I32 0 START_LSB S1, S2, !SP3; NOP; NOP
NOP; FIND_FIRST.I32 1 START_LSB S1, S2; NOP; NOP
NOP; FIND_FIRST.I32 1 START_LSB S1, S2, SP3; NOP; NOP
NOP; FIND_FIRST.I32 1 START_LSB S1, S2, !SP3; NOP; NOP
NOP; FIND_FIRST.I32 0 START_MSB S1, S2; NOP; NOP
NOP; FIND_FIRST.I32 0 START_MSB S1, S2, SP3; NOP; NOP
NOP; FIND_FIRST.I32 0 START_MSB S1, S2, !SP3; NOP; NOP
NOP; FIND_FIRST.I32 1 START_MSB S1, S2; NOP; NOP
NOP; FIND_FIRST.I32 1 START_MSB S1, S2, SP3; NOP; NOP
NOP; FIND_FIRST.I32 1 START_MSB S1, S2, !SP3; NOP; NOP

// CHECK: nop; 	find_first.i32 set=0 start_lsb S1, S2; 	nop; 	nop
// CHECK: nop; 	find_first.i32 set=0 start_lsb S1, S2, SP3; 	nop; 	nop
// CHECK: nop; 	find_first.i32 set=0 start_lsb S1, S2, !SP3; 	nop; 	nop
// CHECK: nop; 	find_first.i32  set=1 start_lsb S1, S2; 	nop; 	nop
// CHECK: nop; 	find_first.i32  set=1 start_lsb S1, S2, SP3; 	nop; 	nop
// CHECK: nop; 	find_first.i32  set=1 start_lsb S1, S2, !SP3; 	nop; 	nop
// CHECK: nop; 	find_first.i32 set=0 start_msb S1, S2; 	nop; 	nop
// CHECK: nop; 	find_first.i32 set=0 start_msb S1, S2, SP3; 	nop; 	nop
// CHECK: nop; 	find_first.i32 set=0 start_msb S1, S2, !SP3; 	nop; 	nop
// CHECK: nop; 	find_first.i32  set=1 start_msb S1, S2; 	nop; 	nop
// CHECK: nop; 	find_first.i32  set=1 start_msb S1, S2, SP3; 	nop; 	nop
// CHECK: nop; 	find_first.i32  set=1 start_msb S1, S2, !SP3; 	nop; 	nop


NOP; FIND_FIRST.U32 0 START_LSB S1, S2; NOP; NOP
NOP; FIND_FIRST.U32 0 START_LSB S1, S2, SP3; NOP; NOP
NOP; FIND_FIRST.U32 0 START_LSB S1, S2, !SP3; NOP; NOP
NOP; FIND_FIRST.U32 1 START_LSB S1, S2; NOP; NOP
NOP; FIND_FIRST.U32 1 START_LSB S1, S2, SP3; NOP; NOP
NOP; FIND_FIRST.U32 1 START_LSB S1, S2, !SP3; NOP; NOP
NOP; FIND_FIRST.U32 0 START_MSB S1, S2; NOP; NOP
NOP; FIND_FIRST.U32 0 START_MSB S1, S2, SP3; NOP; NOP
NOP; FIND_FIRST.U32 0 START_MSB S1, S2, !SP3; NOP; NOP
NOP; FIND_FIRST.U32 1 START_MSB S1, S2; NOP; NOP
NOP; FIND_FIRST.U32 1 START_MSB S1, S2, SP3; NOP; NOP
NOP; FIND_FIRST.U32 1 START_MSB S1, S2, !SP3; NOP; NOP

// CHECK: nop; 	find_first.u32 set=0 start_lsb S1, S2; 	nop; 	nop
// CHECK: nop; 	find_first.u32 set=0 start_lsb S1, S2, SP3; 	nop; 	nop
// CHECK: nop; 	find_first.u32 set=0 start_lsb S1, S2, !SP3; 	nop; 	nop
// CHECK: nop; 	find_first.u32  set=1 start_lsb S1, S2; 	nop; 	nop
// CHECK: nop; 	find_first.u32  set=1 start_lsb S1, S2, SP3; 	nop; 	nop
// CHECK: nop; 	find_first.u32  set=1 start_lsb S1, S2, !SP3; 	nop; 	nop
// CHECK: nop; 	find_first.u32 set=0 start_msb S1, S2; 	nop; 	nop
// CHECK: nop; 	find_first.u32 set=0 start_msb S1, S2, SP3; 	nop; 	nop
// CHECK: nop; 	find_first.u32 set=0 start_msb S1, S2, !SP3; 	nop; 	nop
// CHECK: nop; 	find_first.u32  set=1 start_msb S1, S2; 	nop; 	nop
// CHECK: nop; 	find_first.u32  set=1 start_msb S1, S2, SP3; 	nop; 	nop
// CHECK: nop; 	find_first.u32  set=1 start_msb S1, S2, !SP3; 	nop; 	nop


NOP; FIND_FIRST.I16 0 START_LSB S1, S2; NOP; NOP
NOP; FIND_FIRST.I16 0 START_LSB S1, S2, SP3; NOP; NOP
NOP; FIND_FIRST.I16 0 START_LSB S1, S2, !SP3; NOP; NOP
NOP; FIND_FIRST.I16 1 START_LSB S1, S2; NOP; NOP
NOP; FIND_FIRST.I16 1 START_LSB S1, S2, SP3; NOP; NOP
NOP; FIND_FIRST.I16 1 START_LSB S1, S2, !SP3; NOP; NOP
NOP; FIND_FIRST.I16 0 START_MSB S1, S2; NOP; NOP
NOP; FIND_FIRST.I16 0 START_MSB S1, S2, SP3; NOP; NOP
NOP; FIND_FIRST.I16 0 START_MSB S1, S2, !SP3; NOP; NOP
NOP; FIND_FIRST.I16 1 START_MSB S1, S2; NOP; NOP
NOP; FIND_FIRST.I16 1 START_MSB S1, S2, SP3; NOP; NOP
NOP; FIND_FIRST.I16 1 START_MSB S1, S2, !SP3; NOP; NOP

// CHECK: nop; 	find_first.i16 set=0 start_lsb S1, S2; 	nop; 	nop
// CHECK: nop; 	find_first.i16 set=0 start_lsb S1, S2, SP3; 	nop; 	nop
// CHECK: nop; 	find_first.i16 set=0 start_lsb S1, S2, !SP3; 	nop; 	nop
// CHECK: nop; 	find_first.i16  set=1 start_lsb S1, S2; 	nop; 	nop
// CHECK: nop; 	find_first.i16  set=1 start_lsb S1, S2, SP3; 	nop; 	nop
// CHECK: nop; 	find_first.i16  set=1 start_lsb S1, S2, !SP3; 	nop; 	nop
// CHECK: nop; 	find_first.i16 set=0 start_msb S1, S2; 	nop; 	nop
// CHECK: nop; 	find_first.i16 set=0 start_msb S1, S2, SP3; 	nop; 	nop
// CHECK: nop; 	find_first.i16 set=0 start_msb S1, S2, !SP3; 	nop; 	nop
// CHECK: nop; 	find_first.i16  set=1 start_msb S1, S2; 	nop; 	nop
// CHECK: nop; 	find_first.i16  set=1 start_msb S1, S2, SP3; 	nop; 	nop
// CHECK: nop; 	find_first.i16  set=1 start_msb S1, S2, !SP3; 	nop; 	nop


NOP; FIND_FIRST.U16 0 START_LSB S1, S2; NOP; NOP
NOP; FIND_FIRST.U16 0 START_LSB S1, S2, SP3; NOP; NOP
NOP; FIND_FIRST.U16 0 START_LSB S1, S2, !SP3; NOP; NOP
NOP; FIND_FIRST.U16 1 START_LSB S1, S2; NOP; NOP
NOP; FIND_FIRST.U16 1 START_LSB S1, S2, SP3; NOP; NOP
NOP; FIND_FIRST.U16 1 START_LSB S1, S2, !SP3; NOP; NOP
NOP; FIND_FIRST.U16 0 START_MSB S1, S2; NOP; NOP
NOP; FIND_FIRST.U16 0 START_MSB S1, S2, SP3; NOP; NOP
NOP; FIND_FIRST.U16 0 START_MSB S1, S2, !SP3; NOP; NOP
NOP; FIND_FIRST.U16 1 START_MSB S1, S2; NOP; NOP
NOP; FIND_FIRST.U16 1 START_MSB S1, S2, SP3; NOP; NOP
NOP; FIND_FIRST.U16 1 START_MSB S1, S2, !SP3; NOP; NOP

// CHECK: nop; 	find_first.u16 set=0 start_lsb S1, S2; 	nop; 	nop
// CHECK: nop; 	find_first.u16 set=0 start_lsb S1, S2, SP3; 	nop; 	nop
// CHECK: nop; 	find_first.u16 set=0 start_lsb S1, S2, !SP3; 	nop; 	nop
// CHECK: nop; 	find_first.u16  set=1 start_lsb S1, S2; 	nop; 	nop
// CHECK: nop; 	find_first.u16  set=1 start_lsb S1, S2, SP3; 	nop; 	nop
// CHECK: nop; 	find_first.u16  set=1 start_lsb S1, S2, !SP3; 	nop; 	nop
// CHECK: nop; 	find_first.u16 set=0 start_msb S1, S2; 	nop; 	nop
// CHECK: nop; 	find_first.u16 set=0 start_msb S1, S2, SP3; 	nop; 	nop
// CHECK: nop; 	find_first.u16 set=0 start_msb S1, S2, !SP3; 	nop; 	nop
// CHECK: nop; 	find_first.u16  set=1 start_msb S1, S2; 	nop; 	nop
// CHECK: nop; 	find_first.u16  set=1 start_msb S1, S2, SP3; 	nop; 	nop
// CHECK: nop; 	find_first.u16  set=1 start_msb S1, S2, !SP3; 	nop; 	nop


NOP; FIND_FIRST.I8 0 START_LSB S1, S2; NOP; NOP
NOP; FIND_FIRST.I8 0 START_LSB S1, S2, SP3; NOP; NOP
NOP; FIND_FIRST.I8 0 START_LSB S1, S2, !SP3; NOP; NOP
NOP; FIND_FIRST.I8 1 START_LSB S1, S2; NOP; NOP
NOP; FIND_FIRST.I8 1 START_LSB S1, S2, SP3; NOP; NOP
NOP; FIND_FIRST.I8 1 START_LSB S1, S2, !SP3; NOP; NOP
NOP; FIND_FIRST.I8 0 START_MSB S1, S2; NOP; NOP
NOP; FIND_FIRST.I8 0 START_MSB S1, S2, SP3; NOP; NOP
NOP; FIND_FIRST.I8 0 START_MSB S1, S2, !SP3; NOP; NOP
NOP; FIND_FIRST.I8 1 START_MSB S1, S2; NOP; NOP
NOP; FIND_FIRST.I8 1 START_MSB S1, S2, SP3; NOP; NOP
NOP; FIND_FIRST.I8 1 START_MSB S1, S2, !SP3; NOP; NOP

// CHECK: nop; 	find_first.i8 set=0 start_lsb S1, S2; 	nop; 	nop
// CHECK: nop; 	find_first.i8 set=0 start_lsb S1, S2, SP3; 	nop; 	nop
// CHECK: nop; 	find_first.i8 set=0 start_lsb S1, S2, !SP3; 	nop; 	nop
// CHECK: nop; 	find_first.i8  set=1 start_lsb S1, S2; 	nop; 	nop
// CHECK: nop; 	find_first.i8  set=1 start_lsb S1, S2, SP3; 	nop; 	nop
// CHECK: nop; 	find_first.i8  set=1 start_lsb S1, S2, !SP3; 	nop; 	nop
// CHECK: nop; 	find_first.i8 set=0 start_msb S1, S2; 	nop; 	nop
// CHECK: nop; 	find_first.i8 set=0 start_msb S1, S2, SP3; 	nop; 	nop
// CHECK: nop; 	find_first.i8 set=0 start_msb S1, S2, !SP3; 	nop; 	nop
// CHECK: nop; 	find_first.i8  set=1 start_msb S1, S2; 	nop; 	nop
// CHECK: nop; 	find_first.i8  set=1 start_msb S1, S2, SP3; 	nop; 	nop
// CHECK: nop; 	find_first.i8  set=1 start_msb S1, S2, !SP3; 	nop; 	nop


NOP; FIND_FIRST.U8 0 START_LSB S1, S2; NOP; NOP
NOP; FIND_FIRST.U8 0 START_LSB S1, S2, SP3; NOP; NOP
NOP; FIND_FIRST.U8 0 START_LSB S1, S2, !SP3; NOP; NOP
NOP; FIND_FIRST.U8 1 START_LSB S1, S2; NOP; NOP
NOP; FIND_FIRST.U8 1 START_LSB S1, S2, SP3; NOP; NOP
NOP; FIND_FIRST.U8 1 START_LSB S1, S2, !SP3; NOP; NOP
NOP; FIND_FIRST.U8 0 START_MSB S1, S2; NOP; NOP
NOP; FIND_FIRST.U8 0 START_MSB S1, S2, SP3; NOP; NOP
NOP; FIND_FIRST.U8 0 START_MSB S1, S2, !SP3; NOP; NOP
NOP; FIND_FIRST.U8 1 START_MSB S1, S2; NOP; NOP
NOP; FIND_FIRST.U8 1 START_MSB S1, S2, SP3; NOP; NOP
NOP; FIND_FIRST.U8 1 START_MSB S1, S2, !SP3; NOP; NOP

// CHECK: nop; 	find_first.u8 set=0 start_lsb S1, S2; 	nop; 	nop
// CHECK: nop; 	find_first.u8 set=0 start_lsb S1, S2, SP3; 	nop; 	nop
// CHECK: nop; 	find_first.u8 set=0 start_lsb S1, S2, !SP3; 	nop; 	nop
// CHECK: nop; 	find_first.u8  set=1 start_lsb S1, S2; 	nop; 	nop
// CHECK: nop; 	find_first.u8  set=1 start_lsb S1, S2, SP3; 	nop; 	nop
// CHECK: nop; 	find_first.u8  set=1 start_lsb S1, S2, !SP3; 	nop; 	nop
// CHECK: nop; 	find_first.u8 set=0 start_msb S1, S2; 	nop; 	nop
// CHECK: nop; 	find_first.u8 set=0 start_msb S1, S2, SP3; 	nop; 	nop
// CHECK: nop; 	find_first.u8 set=0 start_msb S1, S2, !SP3; 	nop; 	nop
// CHECK: nop; 	find_first.u8  set=1 start_msb S1, S2; 	nop; 	nop
// CHECK: nop; 	find_first.u8  set=1 start_msb S1, S2, SP3; 	nop; 	nop
// CHECK: nop; 	find_first.u8  set=1 start_msb S1, S2, !SP3; 	nop; 	nop
