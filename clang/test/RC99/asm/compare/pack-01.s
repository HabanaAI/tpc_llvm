// RUN: %clang -cc1as -triple tpc-none-none -filetype obj %s -o %t.o
// RUN: %disasm %t.o | FileCheck %s

NOP; NOP; PACK.I16 V1, V2, GROUP_0, STRIDE_2
NOP; NOP; PACK.I16 V1, V2, GROUP_0, STRIDE_2, SP4
NOP; NOP; PACK.I16 V1, V2, GROUP_0, STRIDE_2, !SP4
NOP; NOP; PACK.I16 V1, V2, GROUP_0, STRIDE_2, VP4
NOP; NOP; PACK.I16 V1, V2, GROUP_0, STRIDE_2, !VP4

// CHECK: nop; 	nop; 	pack.i16 source_group=0 element_stride=2 V1, V2, SP0; 	nop
// CHECK: nop; 	nop; 	pack.i16 source_group=0 element_stride=2 V1, V2, SP4; 	nop
// CHECK: nop; 	nop; 	pack.i16 source_group=0 element_stride=2 V1, V2, !SP4; 	nop
// CHECK: nop; 	nop; 	pack.i16 source_group=0 element_stride=2 V1, V2, VP4; 	nop
// CHECK: nop; 	nop; 	pack.i16 source_group=0 element_stride=2 V1, V2, !VP4; 	nop


NOP; NOP; PACK.I16 V1, V2, GROUP_1, STRIDE_2
NOP; NOP; PACK.I16 V1, V2, GROUP_1, STRIDE_2, SP4
NOP; NOP; PACK.I16 V1, V2, GROUP_1, STRIDE_2, !SP4
NOP; NOP; PACK.I16 V1, V2, GROUP_1, STRIDE_2, VP4
NOP; NOP; PACK.I16 V1, V2, GROUP_1, STRIDE_2, !VP4

// CHECK: nop; 	nop; 	pack.i16  source_group=1 element_stride=2 V1, V2, SP0; 	nop
// CHECK: nop; 	nop; 	pack.i16  source_group=1 element_stride=2 V1, V2, SP4; 	nop
// CHECK: nop; 	nop; 	pack.i16  source_group=1 element_stride=2 V1, V2, !SP4; 	nop
// CHECK: nop; 	nop; 	pack.i16  source_group=1 element_stride=2 V1, V2, VP4; 	nop
// CHECK: nop; 	nop; 	pack.i16  source_group=1 element_stride=2 V1, V2, !VP4; 	nop


NOP; NOP; PACK.I16 V1, V2, GROUP_0, STRIDE_4
NOP; NOP; PACK.I16 V1, V2, GROUP_0, STRIDE_4, SP4
NOP; NOP; PACK.I16 V1, V2, GROUP_0, STRIDE_4, !SP4
NOP; NOP; PACK.I16 V1, V2, GROUP_0, STRIDE_4, VP4
NOP; NOP; PACK.I16 V1, V2, GROUP_0, STRIDE_4, !VP4

// CHECK: nop; 	nop; 	pack.i16 source_group=0 element_stride=4 V1, V2, SP0; 	nop
// CHECK: nop; 	nop; 	pack.i16 source_group=0 element_stride=4 V1, V2, SP4; 	nop
// CHECK: nop; 	nop; 	pack.i16 source_group=0 element_stride=4 V1, V2, !SP4; 	nop
// CHECK: nop; 	nop; 	pack.i16 source_group=0 element_stride=4 V1, V2, VP4; 	nop
// CHECK: nop; 	nop; 	pack.i16 source_group=0 element_stride=4 V1, V2, !VP4; 	nop


NOP; NOP; PACK.I16 V1, V2, GROUP_1, STRIDE_4
NOP; NOP; PACK.I16 V1, V2, GROUP_1, STRIDE_4, SP4
NOP; NOP; PACK.I16 V1, V2, GROUP_1, STRIDE_4, !SP4
NOP; NOP; PACK.I16 V1, V2, GROUP_1, STRIDE_4, VP4
NOP; NOP; PACK.I16 V1, V2, GROUP_1, STRIDE_4, !VP4

// CHECK: nop; 	nop; 	pack.i16  source_group=1 element_stride=4 V1, V2, SP0; 	nop
// CHECK: nop; 	nop; 	pack.i16  source_group=1 element_stride=4 V1, V2, SP4; 	nop
// CHECK: nop; 	nop; 	pack.i16  source_group=1 element_stride=4 V1, V2, !SP4; 	nop
// CHECK: nop; 	nop; 	pack.i16  source_group=1 element_stride=4 V1, V2, VP4; 	nop
// CHECK: nop; 	nop; 	pack.i16  source_group=1 element_stride=4 V1, V2, !VP4; 	nop


NOP; NOP; PACK.I8 V1, V2, GROUP_0, STRIDE_2
NOP; NOP; PACK.I8 V1, V2, GROUP_0, STRIDE_2, SP4
NOP; NOP; PACK.I8 V1, V2, GROUP_0, STRIDE_2, !SP4
NOP; NOP; PACK.I8 V1, V2, GROUP_0, STRIDE_2, VP4
NOP; NOP; PACK.I8 V1, V2, GROUP_0, STRIDE_2, !VP4

// CHECK: nop; 	nop; 	pack.i8 source_group=0 element_stride=2 V1, V2, SP0; 	nop
// CHECK: nop; 	nop; 	pack.i8 source_group=0 element_stride=2 V1, V2, SP4; 	nop
// CHECK: nop; 	nop; 	pack.i8 source_group=0 element_stride=2 V1, V2, !SP4; 	nop
// CHECK: nop; 	nop; 	pack.i8 source_group=0 element_stride=2 V1, V2, VP4; 	nop
// CHECK: nop; 	nop; 	pack.i8 source_group=0 element_stride=2 V1, V2, !VP4; 	nop


NOP; NOP; PACK.I8 V1, V2, GROUP_1, STRIDE_2
NOP; NOP; PACK.I8 V1, V2, GROUP_1, STRIDE_2, SP4
NOP; NOP; PACK.I8 V1, V2, GROUP_1, STRIDE_2, !SP4
NOP; NOP; PACK.I8 V1, V2, GROUP_1, STRIDE_2, VP4
NOP; NOP; PACK.I8 V1, V2, GROUP_1, STRIDE_2, !VP4

// CHECK: nop; 	nop; 	pack.i8  source_group=1 element_stride=2 V1, V2, SP0; 	nop
// CHECK: nop; 	nop; 	pack.i8  source_group=1 element_stride=2 V1, V2, SP4; 	nop
// CHECK: nop; 	nop; 	pack.i8  source_group=1 element_stride=2 V1, V2, !SP4; 	nop
// CHECK: nop; 	nop; 	pack.i8  source_group=1 element_stride=2 V1, V2, VP4; 	nop
// CHECK: nop; 	nop; 	pack.i8  source_group=1 element_stride=2 V1, V2, !VP4; 	nop


NOP; NOP; PACK.I8 V1, V2, GROUP_0, STRIDE_4
NOP; NOP; PACK.I8 V1, V2, GROUP_0, STRIDE_4, SP4
NOP; NOP; PACK.I8 V1, V2, GROUP_0, STRIDE_4, !SP4
NOP; NOP; PACK.I8 V1, V2, GROUP_0, STRIDE_4, VP4
NOP; NOP; PACK.I8 V1, V2, GROUP_0, STRIDE_4, !VP4

// CHECK: nop; 	nop; 	pack.i8 source_group=0 element_stride=4 V1, V2, SP0; 	nop
// CHECK: nop; 	nop; 	pack.i8 source_group=0 element_stride=4 V1, V2, SP4; 	nop
// CHECK: nop; 	nop; 	pack.i8 source_group=0 element_stride=4 V1, V2, !SP4; 	nop
// CHECK: nop; 	nop; 	pack.i8 source_group=0 element_stride=4 V1, V2, VP4; 	nop
// CHECK: nop; 	nop; 	pack.i8 source_group=0 element_stride=4 V1, V2, !VP4; 	nop


NOP; NOP; PACK.I8 V1, V2, GROUP_1, STRIDE_4
NOP; NOP; PACK.I8 V1, V2, GROUP_1, STRIDE_4, SP4
NOP; NOP; PACK.I8 V1, V2, GROUP_1, STRIDE_4, !SP4
NOP; NOP; PACK.I8 V1, V2, GROUP_1, STRIDE_4, VP4
NOP; NOP; PACK.I8 V1, V2, GROUP_1, STRIDE_4, !VP4

// CHECK: nop; 	nop; 	pack.i8  source_group=1 element_stride=4 V1, V2, SP0; 	nop
// CHECK: nop; 	nop; 	pack.i8  source_group=1 element_stride=4 V1, V2, SP4; 	nop
// CHECK: nop; 	nop; 	pack.i8  source_group=1 element_stride=4 V1, V2, !SP4; 	nop
// CHECK: nop; 	nop; 	pack.i8  source_group=1 element_stride=4 V1, V2, VP4; 	nop
// CHECK: nop; 	nop; 	pack.i8  source_group=1 element_stride=4 V1, V2, !VP4; 	nop


NOP; NOP; PACK.U16 V1, V2, GROUP_0, STRIDE_2
NOP; NOP; PACK.U16 V1, V2, GROUP_0, STRIDE_2, SP4
NOP; NOP; PACK.U16 V1, V2, GROUP_0, STRIDE_2, !SP4
NOP; NOP; PACK.U16 V1, V2, GROUP_0, STRIDE_2, VP4
NOP; NOP; PACK.U16 V1, V2, GROUP_0, STRIDE_2, !VP4

// CHECK: nop; 	nop; 	pack.u16 source_group=0 element_stride=2 V1, V2, SP0; 	nop
// CHECK: nop; 	nop; 	pack.u16 source_group=0 element_stride=2 V1, V2, SP4; 	nop
// CHECK: nop; 	nop; 	pack.u16 source_group=0 element_stride=2 V1, V2, !SP4; 	nop
// CHECK: nop; 	nop; 	pack.u16 source_group=0 element_stride=2 V1, V2, VP4; 	nop
// CHECK: nop; 	nop; 	pack.u16 source_group=0 element_stride=2 V1, V2, !VP4; 	nop


NOP; NOP; PACK.U16 V1, V2, GROUP_1, STRIDE_2
NOP; NOP; PACK.U16 V1, V2, GROUP_1, STRIDE_2, SP4
NOP; NOP; PACK.U16 V1, V2, GROUP_1, STRIDE_2, !SP4
NOP; NOP; PACK.U16 V1, V2, GROUP_1, STRIDE_2, VP4
NOP; NOP; PACK.U16 V1, V2, GROUP_1, STRIDE_2, !VP4

// CHECK: nop; 	nop; 	pack.u16  source_group=1 element_stride=2 V1, V2, SP0; 	nop
// CHECK: nop; 	nop; 	pack.u16  source_group=1 element_stride=2 V1, V2, SP4; 	nop
// CHECK: nop; 	nop; 	pack.u16  source_group=1 element_stride=2 V1, V2, !SP4; 	nop
// CHECK: nop; 	nop; 	pack.u16  source_group=1 element_stride=2 V1, V2, VP4; 	nop
// CHECK: nop; 	nop; 	pack.u16  source_group=1 element_stride=2 V1, V2, !VP4; 	nop


NOP; NOP; PACK.U16 V1, V2, GROUP_0, STRIDE_4
NOP; NOP; PACK.U16 V1, V2, GROUP_0, STRIDE_4, SP4
NOP; NOP; PACK.U16 V1, V2, GROUP_0, STRIDE_4, !SP4
NOP; NOP; PACK.U16 V1, V2, GROUP_0, STRIDE_4, VP4
NOP; NOP; PACK.U16 V1, V2, GROUP_0, STRIDE_4, !VP4

// CHECK: nop; 	nop; 	pack.u16 source_group=0 element_stride=4 V1, V2, SP0; 	nop
// CHECK: nop; 	nop; 	pack.u16 source_group=0 element_stride=4 V1, V2, SP4; 	nop
// CHECK: nop; 	nop; 	pack.u16 source_group=0 element_stride=4 V1, V2, !SP4; 	nop
// CHECK: nop; 	nop; 	pack.u16 source_group=0 element_stride=4 V1, V2, VP4; 	nop
// CHECK: nop; 	nop; 	pack.u16 source_group=0 element_stride=4 V1, V2, !VP4; 	nop


NOP; NOP; PACK.U16 V1, V2, GROUP_1, STRIDE_4
NOP; NOP; PACK.U16 V1, V2, GROUP_1, STRIDE_4, SP4
NOP; NOP; PACK.U16 V1, V2, GROUP_1, STRIDE_4, !SP4
NOP; NOP; PACK.U16 V1, V2, GROUP_1, STRIDE_4, VP4
NOP; NOP; PACK.U16 V1, V2, GROUP_1, STRIDE_4, !VP4

// CHECK: nop; 	nop; 	pack.u16  source_group=1 element_stride=4 V1, V2, SP0; 	nop
// CHECK: nop; 	nop; 	pack.u16  source_group=1 element_stride=4 V1, V2, SP4; 	nop
// CHECK: nop; 	nop; 	pack.u16  source_group=1 element_stride=4 V1, V2, !SP4; 	nop
// CHECK: nop; 	nop; 	pack.u16  source_group=1 element_stride=4 V1, V2, VP4; 	nop
// CHECK: nop; 	nop; 	pack.u16  source_group=1 element_stride=4 V1, V2, !VP4; 	nop


NOP; NOP; PACK.U8 V1, V2, GROUP_0, STRIDE_2
NOP; NOP; PACK.U8 V1, V2, GROUP_0, STRIDE_2, SP4
NOP; NOP; PACK.U8 V1, V2, GROUP_0, STRIDE_2, !SP4
NOP; NOP; PACK.U8 V1, V2, GROUP_0, STRIDE_2, VP4
NOP; NOP; PACK.U8 V1, V2, GROUP_0, STRIDE_2, !VP4

// CHECK: nop; 	nop; 	pack.u8 source_group=0 element_stride=2 V1, V2, SP0; 	nop
// CHECK: nop; 	nop; 	pack.u8 source_group=0 element_stride=2 V1, V2, SP4; 	nop
// CHECK: nop; 	nop; 	pack.u8 source_group=0 element_stride=2 V1, V2, !SP4; 	nop
// CHECK: nop; 	nop; 	pack.u8 source_group=0 element_stride=2 V1, V2, VP4; 	nop
// CHECK: nop; 	nop; 	pack.u8 source_group=0 element_stride=2 V1, V2, !VP4; 	nop


NOP; NOP; PACK.U8 V1, V2, GROUP_1, STRIDE_2
NOP; NOP; PACK.U8 V1, V2, GROUP_1, STRIDE_2, SP4
NOP; NOP; PACK.U8 V1, V2, GROUP_1, STRIDE_2, !SP4
NOP; NOP; PACK.U8 V1, V2, GROUP_1, STRIDE_2, VP4
NOP; NOP; PACK.U8 V1, V2, GROUP_1, STRIDE_2, !VP4

// CHECK: nop; 	nop; 	pack.u8  source_group=1 element_stride=2 V1, V2, SP0; 	nop
// CHECK: nop; 	nop; 	pack.u8  source_group=1 element_stride=2 V1, V2, SP4; 	nop
// CHECK: nop; 	nop; 	pack.u8  source_group=1 element_stride=2 V1, V2, !SP4; 	nop
// CHECK: nop; 	nop; 	pack.u8  source_group=1 element_stride=2 V1, V2, VP4; 	nop
// CHECK: nop; 	nop; 	pack.u8  source_group=1 element_stride=2 V1, V2, !VP4; 	nop


NOP; NOP; PACK.U8 V1, V2, GROUP_0, STRIDE_4
NOP; NOP; PACK.U8 V1, V2, GROUP_0, STRIDE_4, SP4
NOP; NOP; PACK.U8 V1, V2, GROUP_0, STRIDE_4, !SP4
NOP; NOP; PACK.U8 V1, V2, GROUP_0, STRIDE_4, VP4
NOP; NOP; PACK.U8 V1, V2, GROUP_0, STRIDE_4, !VP4

// CHECK: nop; 	nop; 	pack.u8 source_group=0 element_stride=4 V1, V2, SP0; 	nop
// CHECK: nop; 	nop; 	pack.u8 source_group=0 element_stride=4 V1, V2, SP4; 	nop
// CHECK: nop; 	nop; 	pack.u8 source_group=0 element_stride=4 V1, V2, !SP4; 	nop
// CHECK: nop; 	nop; 	pack.u8 source_group=0 element_stride=4 V1, V2, VP4; 	nop
// CHECK: nop; 	nop; 	pack.u8 source_group=0 element_stride=4 V1, V2, !VP4; 	nop


NOP; NOP; PACK.U8 V1, V2, GROUP_1, STRIDE_4
NOP; NOP; PACK.U8 V1, V2, GROUP_1, STRIDE_4, SP4
NOP; NOP; PACK.U8 V1, V2, GROUP_1, STRIDE_4, !SP4
NOP; NOP; PACK.U8 V1, V2, GROUP_1, STRIDE_4, VP4
NOP; NOP; PACK.U8 V1, V2, GROUP_1, STRIDE_4, !VP4

// CHECK: nop; 	nop; 	pack.u8  source_group=1 element_stride=4 V1, V2, SP0; 	nop
// CHECK: nop; 	nop; 	pack.u8  source_group=1 element_stride=4 V1, V2, SP4; 	nop
// CHECK: nop; 	nop; 	pack.u8  source_group=1 element_stride=4 V1, V2, !SP4; 	nop
// CHECK: nop; 	nop; 	pack.u8  source_group=1 element_stride=4 V1, V2, VP4; 	nop
// CHECK: nop; 	nop; 	pack.u8  source_group=1 element_stride=4 V1, V2, !VP4; 	nop
