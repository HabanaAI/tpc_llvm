// RUN: %clang -cc1as -triple tpc-none-none -filetype obj %s -o %t.o
// RUN: %disasm %t.o | FileCheck %s

NOP; NOP; UNPACK.I16 V1, V2, GROUP_0, STRIDE_2, GROUP_HALF_0
NOP; NOP; UNPACK.I16 V1, V2, GROUP_0, STRIDE_2, GROUP_HALF_0, SP4
NOP; NOP; UNPACK.I16 V1, V2, GROUP_0, STRIDE_2, GROUP_HALF_0, !SP4
NOP; NOP; UNPACK.I16 V1, V2, GROUP_0, STRIDE_2, GROUP_HALF_0, VP4
NOP; NOP; UNPACK.I16 V1, V2, GROUP_0, STRIDE_2, GROUP_HALF_0, !VP4

// CHECK: nop; 	nop; 	unpack.i16 source_group=0 element_stride=2 group_half=0 V1, V2; 	nop
// CHECK: nop; 	nop; 	unpack.i16 source_group=0 element_stride=2 group_half=0 V1, V2, SP4; 	nop
// CHECK: nop; 	nop; 	unpack.i16 source_group=0 element_stride=2 group_half=0 V1, V2, !SP4; 	nop
// CHECK: nop; 	nop; 	unpack.i16 source_group=0 element_stride=2 group_half=0 V1, V2, VP4; 	nop
// CHECK: nop; 	nop; 	unpack.i16 source_group=0 element_stride=2 group_half=0 V1, V2, !VP4; 	nop


NOP; NOP; UNPACK.I16 V1, V2, GROUP_1, STRIDE_2, GROUP_HALF_0
NOP; NOP; UNPACK.I16 V1, V2, GROUP_1, STRIDE_2, GROUP_HALF_0, SP4
NOP; NOP; UNPACK.I16 V1, V2, GROUP_1, STRIDE_2, GROUP_HALF_0, !SP4
NOP; NOP; UNPACK.I16 V1, V2, GROUP_1, STRIDE_2, GROUP_HALF_0, VP4
NOP; NOP; UNPACK.I16 V1, V2, GROUP_1, STRIDE_2, GROUP_HALF_0, !VP4

// CHECK: nop; 	nop; 	unpack.i16  source_group=1 element_stride=2 group_half=0 V1, V2; 	nop
// CHECK: nop; 	nop; 	unpack.i16  source_group=1 element_stride=2 group_half=0 V1, V2, SP4; 	nop
// CHECK: nop; 	nop; 	unpack.i16  source_group=1 element_stride=2 group_half=0 V1, V2, !SP4; 	nop
// CHECK: nop; 	nop; 	unpack.i16  source_group=1 element_stride=2 group_half=0 V1, V2, VP4; 	nop
// CHECK: nop; 	nop; 	unpack.i16  source_group=1 element_stride=2 group_half=0 V1, V2, !VP4; 	nop


NOP; NOP; UNPACK.I16 V1, V2, GROUP_0, STRIDE_4, GROUP_HALF_0
NOP; NOP; UNPACK.I16 V1, V2, GROUP_0, STRIDE_4, GROUP_HALF_0, SP4
NOP; NOP; UNPACK.I16 V1, V2, GROUP_0, STRIDE_4, GROUP_HALF_0, !SP4
NOP; NOP; UNPACK.I16 V1, V2, GROUP_0, STRIDE_4, GROUP_HALF_0, VP4
NOP; NOP; UNPACK.I16 V1, V2, GROUP_0, STRIDE_4, GROUP_HALF_0, !VP4

// CHECK: nop; 	nop; 	unpack.i16 source_group=0 element_stride=4 group_half=0 V1, V2; 	nop
// CHECK: nop; 	nop; 	unpack.i16 source_group=0 element_stride=4 group_half=0 V1, V2, SP4; 	nop
// CHECK: nop; 	nop; 	unpack.i16 source_group=0 element_stride=4 group_half=0 V1, V2, !SP4; 	nop
// CHECK: nop; 	nop; 	unpack.i16 source_group=0 element_stride=4 group_half=0 V1, V2, VP4; 	nop
// CHECK: nop; 	nop; 	unpack.i16 source_group=0 element_stride=4 group_half=0 V1, V2, !VP4; 	nop


NOP; NOP; UNPACK.I16 V1, V2, GROUP_1, STRIDE_4, GROUP_HALF_0
NOP; NOP; UNPACK.I16 V1, V2, GROUP_1, STRIDE_4, GROUP_HALF_0, SP4
NOP; NOP; UNPACK.I16 V1, V2, GROUP_1, STRIDE_4, GROUP_HALF_0, !SP4
NOP; NOP; UNPACK.I16 V1, V2, GROUP_1, STRIDE_4, GROUP_HALF_0, VP4
NOP; NOP; UNPACK.I16 V1, V2, GROUP_1, STRIDE_4, GROUP_HALF_0, !VP4

// CHECK: nop; 	nop; 	unpack.i16  source_group=1 element_stride=4 group_half=0 V1, V2; 	nop
// CHECK: nop; 	nop; 	unpack.i16  source_group=1 element_stride=4 group_half=0 V1, V2, SP4; 	nop
// CHECK: nop; 	nop; 	unpack.i16  source_group=1 element_stride=4 group_half=0 V1, V2, !SP4; 	nop
// CHECK: nop; 	nop; 	unpack.i16  source_group=1 element_stride=4 group_half=0 V1, V2, VP4; 	nop
// CHECK: nop; 	nop; 	unpack.i16  source_group=1 element_stride=4 group_half=0 V1, V2, !VP4; 	nop


NOP; NOP; UNPACK.I16 V1, V2, GROUP_0, STRIDE_2, GROUP_HALF_1
NOP; NOP; UNPACK.I16 V1, V2, GROUP_0, STRIDE_2, GROUP_HALF_1, SP4
NOP; NOP; UNPACK.I16 V1, V2, GROUP_0, STRIDE_2, GROUP_HALF_1, !SP4
NOP; NOP; UNPACK.I16 V1, V2, GROUP_0, STRIDE_2, GROUP_HALF_1, VP4
NOP; NOP; UNPACK.I16 V1, V2, GROUP_0, STRIDE_2, GROUP_HALF_1, !VP4

// CHECK: nop; 	nop; 	unpack.i16 source_group=0 element_stride=2 group_half=1 V1, V2; 	nop
// CHECK: nop; 	nop; 	unpack.i16 source_group=0 element_stride=2 group_half=1 V1, V2, SP4; 	nop
// CHECK: nop; 	nop; 	unpack.i16 source_group=0 element_stride=2 group_half=1 V1, V2, !SP4; 	nop
// CHECK: nop; 	nop; 	unpack.i16 source_group=0 element_stride=2 group_half=1 V1, V2, VP4; 	nop
// CHECK: nop; 	nop; 	unpack.i16 source_group=0 element_stride=2 group_half=1 V1, V2, !VP4; 	nop


NOP; NOP; UNPACK.I16 V1, V2, GROUP_1, STRIDE_2, GROUP_HALF_1
NOP; NOP; UNPACK.I16 V1, V2, GROUP_1, STRIDE_2, GROUP_HALF_1, SP4
NOP; NOP; UNPACK.I16 V1, V2, GROUP_1, STRIDE_2, GROUP_HALF_1, !SP4
NOP; NOP; UNPACK.I16 V1, V2, GROUP_1, STRIDE_2, GROUP_HALF_1, VP4
NOP; NOP; UNPACK.I16 V1, V2, GROUP_1, STRIDE_2, GROUP_HALF_1, !VP4

// CHECK: nop; 	nop; 	unpack.i16  source_group=1 element_stride=2 group_half=1 V1, V2; 	nop
// CHECK: nop; 	nop; 	unpack.i16  source_group=1 element_stride=2 group_half=1 V1, V2, SP4; 	nop
// CHECK: nop; 	nop; 	unpack.i16  source_group=1 element_stride=2 group_half=1 V1, V2, !SP4; 	nop
// CHECK: nop; 	nop; 	unpack.i16  source_group=1 element_stride=2 group_half=1 V1, V2, VP4; 	nop
// CHECK: nop; 	nop; 	unpack.i16  source_group=1 element_stride=2 group_half=1 V1, V2, !VP4; 	nop


NOP; NOP; UNPACK.I16 V1, V2, GROUP_0, STRIDE_4, GROUP_HALF_1
NOP; NOP; UNPACK.I16 V1, V2, GROUP_0, STRIDE_4, GROUP_HALF_1, SP4
NOP; NOP; UNPACK.I16 V1, V2, GROUP_0, STRIDE_4, GROUP_HALF_1, !SP4
NOP; NOP; UNPACK.I16 V1, V2, GROUP_0, STRIDE_4, GROUP_HALF_1, VP4
NOP; NOP; UNPACK.I16 V1, V2, GROUP_0, STRIDE_4, GROUP_HALF_1, !VP4

// CHECK: nop; 	nop; 	unpack.i16 source_group=0 element_stride=4 group_half=1 V1, V2; 	nop
// CHECK: nop; 	nop; 	unpack.i16 source_group=0 element_stride=4 group_half=1 V1, V2, SP4; 	nop
// CHECK: nop; 	nop; 	unpack.i16 source_group=0 element_stride=4 group_half=1 V1, V2, !SP4; 	nop
// CHECK: nop; 	nop; 	unpack.i16 source_group=0 element_stride=4 group_half=1 V1, V2, VP4; 	nop
// CHECK: nop; 	nop; 	unpack.i16 source_group=0 element_stride=4 group_half=1 V1, V2, !VP4; 	nop


NOP; NOP; UNPACK.I16 V1, V2, GROUP_1, STRIDE_4, GROUP_HALF_1
NOP; NOP; UNPACK.I16 V1, V2, GROUP_1, STRIDE_4, GROUP_HALF_1, SP4
NOP; NOP; UNPACK.I16 V1, V2, GROUP_1, STRIDE_4, GROUP_HALF_1, !SP4
NOP; NOP; UNPACK.I16 V1, V2, GROUP_1, STRIDE_4, GROUP_HALF_1, VP4
NOP; NOP; UNPACK.I16 V1, V2, GROUP_1, STRIDE_4, GROUP_HALF_1, !VP4

// CHECK: nop; 	nop; 	unpack.i16  source_group=1 element_stride=4 group_half=1 V1, V2; 	nop
// CHECK: nop; 	nop; 	unpack.i16  source_group=1 element_stride=4 group_half=1 V1, V2, SP4; 	nop
// CHECK: nop; 	nop; 	unpack.i16  source_group=1 element_stride=4 group_half=1 V1, V2, !SP4; 	nop
// CHECK: nop; 	nop; 	unpack.i16  source_group=1 element_stride=4 group_half=1 V1, V2, VP4; 	nop
// CHECK: nop; 	nop; 	unpack.i16  source_group=1 element_stride=4 group_half=1 V1, V2, !VP4; 	nop


NOP; NOP; UNPACK.I8 V1, V2, GROUP_0, STRIDE_2, GROUP_HALF_0
NOP; NOP; UNPACK.I8 V1, V2, GROUP_0, STRIDE_2, GROUP_HALF_0, SP4
NOP; NOP; UNPACK.I8 V1, V2, GROUP_0, STRIDE_2, GROUP_HALF_0, !SP4
NOP; NOP; UNPACK.I8 V1, V2, GROUP_0, STRIDE_2, GROUP_HALF_0, VP4
NOP; NOP; UNPACK.I8 V1, V2, GROUP_0, STRIDE_2, GROUP_HALF_0, !VP4

// CHECK: nop; 	nop; 	unpack.i8 source_group=0 element_stride=2 group_half=0 V1, V2; 	nop
// CHECK: nop; 	nop; 	unpack.i8 source_group=0 element_stride=2 group_half=0 V1, V2, SP4; 	nop
// CHECK: nop; 	nop; 	unpack.i8 source_group=0 element_stride=2 group_half=0 V1, V2, !SP4; 	nop
// CHECK: nop; 	nop; 	unpack.i8 source_group=0 element_stride=2 group_half=0 V1, V2, VP4; 	nop
// CHECK: nop; 	nop; 	unpack.i8 source_group=0 element_stride=2 group_half=0 V1, V2, !VP4; 	nop


NOP; NOP; UNPACK.I8 V1, V2, GROUP_1, STRIDE_2, GROUP_HALF_0
NOP; NOP; UNPACK.I8 V1, V2, GROUP_1, STRIDE_2, GROUP_HALF_0, SP4
NOP; NOP; UNPACK.I8 V1, V2, GROUP_1, STRIDE_2, GROUP_HALF_0, !SP4
NOP; NOP; UNPACK.I8 V1, V2, GROUP_1, STRIDE_2, GROUP_HALF_0, VP4
NOP; NOP; UNPACK.I8 V1, V2, GROUP_1, STRIDE_2, GROUP_HALF_0, !VP4

// CHECK: nop; 	nop; 	unpack.i8  source_group=1 element_stride=2 group_half=0 V1, V2; 	nop
// CHECK: nop; 	nop; 	unpack.i8  source_group=1 element_stride=2 group_half=0 V1, V2, SP4; 	nop
// CHECK: nop; 	nop; 	unpack.i8  source_group=1 element_stride=2 group_half=0 V1, V2, !SP4; 	nop
// CHECK: nop; 	nop; 	unpack.i8  source_group=1 element_stride=2 group_half=0 V1, V2, VP4; 	nop
// CHECK: nop; 	nop; 	unpack.i8  source_group=1 element_stride=2 group_half=0 V1, V2, !VP4; 	nop


NOP; NOP; UNPACK.I8 V1, V2, GROUP_0, STRIDE_4, GROUP_HALF_0
NOP; NOP; UNPACK.I8 V1, V2, GROUP_0, STRIDE_4, GROUP_HALF_0, SP4
NOP; NOP; UNPACK.I8 V1, V2, GROUP_0, STRIDE_4, GROUP_HALF_0, !SP4
NOP; NOP; UNPACK.I8 V1, V2, GROUP_0, STRIDE_4, GROUP_HALF_0, VP4
NOP; NOP; UNPACK.I8 V1, V2, GROUP_0, STRIDE_4, GROUP_HALF_0, !VP4

// CHECK: nop; 	nop; 	unpack.i8 source_group=0 element_stride=4 group_half=0 V1, V2; 	nop
// CHECK: nop; 	nop; 	unpack.i8 source_group=0 element_stride=4 group_half=0 V1, V2, SP4; 	nop
// CHECK: nop; 	nop; 	unpack.i8 source_group=0 element_stride=4 group_half=0 V1, V2, !SP4; 	nop
// CHECK: nop; 	nop; 	unpack.i8 source_group=0 element_stride=4 group_half=0 V1, V2, VP4; 	nop
// CHECK: nop; 	nop; 	unpack.i8 source_group=0 element_stride=4 group_half=0 V1, V2, !VP4; 	nop


NOP; NOP; UNPACK.I8 V1, V2, GROUP_1, STRIDE_4, GROUP_HALF_0
NOP; NOP; UNPACK.I8 V1, V2, GROUP_1, STRIDE_4, GROUP_HALF_0, SP4
NOP; NOP; UNPACK.I8 V1, V2, GROUP_1, STRIDE_4, GROUP_HALF_0, !SP4
NOP; NOP; UNPACK.I8 V1, V2, GROUP_1, STRIDE_4, GROUP_HALF_0, VP4
NOP; NOP; UNPACK.I8 V1, V2, GROUP_1, STRIDE_4, GROUP_HALF_0, !VP4

// CHECK: nop; 	nop; 	unpack.i8  source_group=1 element_stride=4 group_half=0 V1, V2; 	nop
// CHECK: nop; 	nop; 	unpack.i8  source_group=1 element_stride=4 group_half=0 V1, V2, SP4; 	nop
// CHECK: nop; 	nop; 	unpack.i8  source_group=1 element_stride=4 group_half=0 V1, V2, !SP4; 	nop
// CHECK: nop; 	nop; 	unpack.i8  source_group=1 element_stride=4 group_half=0 V1, V2, VP4; 	nop
// CHECK: nop; 	nop; 	unpack.i8  source_group=1 element_stride=4 group_half=0 V1, V2, !VP4; 	nop


NOP; NOP; UNPACK.U16 V1, V2, GROUP_0, STRIDE_2, GROUP_HALF_0
NOP; NOP; UNPACK.U16 V1, V2, GROUP_0, STRIDE_2, GROUP_HALF_0, SP4
NOP; NOP; UNPACK.U16 V1, V2, GROUP_0, STRIDE_2, GROUP_HALF_0, !SP4
NOP; NOP; UNPACK.U16 V1, V2, GROUP_0, STRIDE_2, GROUP_HALF_0, VP4
NOP; NOP; UNPACK.U16 V1, V2, GROUP_0, STRIDE_2, GROUP_HALF_0, !VP4

// CHECK: nop; 	nop; 	unpack.u16 source_group=0 element_stride=2 group_half=0 V1, V2; 	nop
// CHECK: nop; 	nop; 	unpack.u16 source_group=0 element_stride=2 group_half=0 V1, V2, SP4; 	nop
// CHECK: nop; 	nop; 	unpack.u16 source_group=0 element_stride=2 group_half=0 V1, V2, !SP4; 	nop
// CHECK: nop; 	nop; 	unpack.u16 source_group=0 element_stride=2 group_half=0 V1, V2, VP4; 	nop
// CHECK: nop; 	nop; 	unpack.u16 source_group=0 element_stride=2 group_half=0 V1, V2, !VP4; 	nop


NOP; NOP; UNPACK.U16 V1, V2, GROUP_1, STRIDE_2, GROUP_HALF_0
NOP; NOP; UNPACK.U16 V1, V2, GROUP_1, STRIDE_2, GROUP_HALF_0, SP4
NOP; NOP; UNPACK.U16 V1, V2, GROUP_1, STRIDE_2, GROUP_HALF_0, !SP4
NOP; NOP; UNPACK.U16 V1, V2, GROUP_1, STRIDE_2, GROUP_HALF_0, VP4
NOP; NOP; UNPACK.U16 V1, V2, GROUP_1, STRIDE_2, GROUP_HALF_0, !VP4

// CHECK: nop; 	nop; 	unpack.u16  source_group=1 element_stride=2 group_half=0 V1, V2; 	nop
// CHECK: nop; 	nop; 	unpack.u16  source_group=1 element_stride=2 group_half=0 V1, V2, SP4; 	nop
// CHECK: nop; 	nop; 	unpack.u16  source_group=1 element_stride=2 group_half=0 V1, V2, !SP4; 	nop
// CHECK: nop; 	nop; 	unpack.u16  source_group=1 element_stride=2 group_half=0 V1, V2, VP4; 	nop
// CHECK: nop; 	nop; 	unpack.u16  source_group=1 element_stride=2 group_half=0 V1, V2, !VP4; 	nop


NOP; NOP; UNPACK.U16 V1, V2, GROUP_0, STRIDE_4, GROUP_HALF_0
NOP; NOP; UNPACK.U16 V1, V2, GROUP_0, STRIDE_4, GROUP_HALF_0, SP4
NOP; NOP; UNPACK.U16 V1, V2, GROUP_0, STRIDE_4, GROUP_HALF_0, !SP4
NOP; NOP; UNPACK.U16 V1, V2, GROUP_0, STRIDE_4, GROUP_HALF_0, VP4
NOP; NOP; UNPACK.U16 V1, V2, GROUP_0, STRIDE_4, GROUP_HALF_0, !VP4

// CHECK: nop; 	nop; 	unpack.u16 source_group=0 element_stride=4 group_half=0 V1, V2; 	nop
// CHECK: nop; 	nop; 	unpack.u16 source_group=0 element_stride=4 group_half=0 V1, V2, SP4; 	nop
// CHECK: nop; 	nop; 	unpack.u16 source_group=0 element_stride=4 group_half=0 V1, V2, !SP4; 	nop
// CHECK: nop; 	nop; 	unpack.u16 source_group=0 element_stride=4 group_half=0 V1, V2, VP4; 	nop
// CHECK: nop; 	nop; 	unpack.u16 source_group=0 element_stride=4 group_half=0 V1, V2, !VP4; 	nop


NOP; NOP; UNPACK.U16 V1, V2, GROUP_1, STRIDE_4, GROUP_HALF_0
NOP; NOP; UNPACK.U16 V1, V2, GROUP_1, STRIDE_4, GROUP_HALF_0, SP4
NOP; NOP; UNPACK.U16 V1, V2, GROUP_1, STRIDE_4, GROUP_HALF_0, !SP4
NOP; NOP; UNPACK.U16 V1, V2, GROUP_1, STRIDE_4, GROUP_HALF_0, VP4
NOP; NOP; UNPACK.U16 V1, V2, GROUP_1, STRIDE_4, GROUP_HALF_0, !VP4

// CHECK: nop; 	nop; 	unpack.u16  source_group=1 element_stride=4 group_half=0 V1, V2; 	nop
// CHECK: nop; 	nop; 	unpack.u16  source_group=1 element_stride=4 group_half=0 V1, V2, SP4; 	nop
// CHECK: nop; 	nop; 	unpack.u16  source_group=1 element_stride=4 group_half=0 V1, V2, !SP4; 	nop
// CHECK: nop; 	nop; 	unpack.u16  source_group=1 element_stride=4 group_half=0 V1, V2, VP4; 	nop
// CHECK: nop; 	nop; 	unpack.u16  source_group=1 element_stride=4 group_half=0 V1, V2, !VP4; 	nop


NOP; NOP; UNPACK.U8 V1, V2, GROUP_0, STRIDE_2, GROUP_HALF_0
NOP; NOP; UNPACK.U8 V1, V2, GROUP_0, STRIDE_2, GROUP_HALF_0, SP4
NOP; NOP; UNPACK.U8 V1, V2, GROUP_0, STRIDE_2, GROUP_HALF_0, !SP4
NOP; NOP; UNPACK.U8 V1, V2, GROUP_0, STRIDE_2, GROUP_HALF_0, VP4
NOP; NOP; UNPACK.U8 V1, V2, GROUP_0, STRIDE_2, GROUP_HALF_0, !VP4

// CHECK: nop; 	nop; 	unpack.u8 source_group=0 element_stride=2 group_half=0 V1, V2; 	nop
// CHECK: nop; 	nop; 	unpack.u8 source_group=0 element_stride=2 group_half=0 V1, V2, SP4; 	nop
// CHECK: nop; 	nop; 	unpack.u8 source_group=0 element_stride=2 group_half=0 V1, V2, !SP4; 	nop
// CHECK: nop; 	nop; 	unpack.u8 source_group=0 element_stride=2 group_half=0 V1, V2, VP4; 	nop
// CHECK: nop; 	nop; 	unpack.u8 source_group=0 element_stride=2 group_half=0 V1, V2, !VP4; 	nop


NOP; NOP; UNPACK.U8 V1, V2, GROUP_1, STRIDE_2, GROUP_HALF_0
NOP; NOP; UNPACK.U8 V1, V2, GROUP_1, STRIDE_2, GROUP_HALF_0, SP4
NOP; NOP; UNPACK.U8 V1, V2, GROUP_1, STRIDE_2, GROUP_HALF_0, !SP4
NOP; NOP; UNPACK.U8 V1, V2, GROUP_1, STRIDE_2, GROUP_HALF_0, VP4
NOP; NOP; UNPACK.U8 V1, V2, GROUP_1, STRIDE_2, GROUP_HALF_0, !VP4

// CHECK: nop; 	nop; 	unpack.u8  source_group=1 element_stride=2 group_half=0 V1, V2; 	nop
// CHECK: nop; 	nop; 	unpack.u8  source_group=1 element_stride=2 group_half=0 V1, V2, SP4; 	nop
// CHECK: nop; 	nop; 	unpack.u8  source_group=1 element_stride=2 group_half=0 V1, V2, !SP4; 	nop
// CHECK: nop; 	nop; 	unpack.u8  source_group=1 element_stride=2 group_half=0 V1, V2, VP4; 	nop
// CHECK: nop; 	nop; 	unpack.u8  source_group=1 element_stride=2 group_half=0 V1, V2, !VP4; 	nop


NOP; NOP; UNPACK.U8 V1, V2, GROUP_0, STRIDE_4, GROUP_HALF_0
NOP; NOP; UNPACK.U8 V1, V2, GROUP_0, STRIDE_4, GROUP_HALF_0, SP4
NOP; NOP; UNPACK.U8 V1, V2, GROUP_0, STRIDE_4, GROUP_HALF_0, !SP4
NOP; NOP; UNPACK.U8 V1, V2, GROUP_0, STRIDE_4, GROUP_HALF_0, VP4
NOP; NOP; UNPACK.U8 V1, V2, GROUP_0, STRIDE_4, GROUP_HALF_0, !VP4

// CHECK: nop; 	nop; 	unpack.u8 source_group=0 element_stride=4 group_half=0 V1, V2; 	nop
// CHECK: nop; 	nop; 	unpack.u8 source_group=0 element_stride=4 group_half=0 V1, V2, SP4; 	nop
// CHECK: nop; 	nop; 	unpack.u8 source_group=0 element_stride=4 group_half=0 V1, V2, !SP4; 	nop
// CHECK: nop; 	nop; 	unpack.u8 source_group=0 element_stride=4 group_half=0 V1, V2, VP4; 	nop
// CHECK: nop; 	nop; 	unpack.u8 source_group=0 element_stride=4 group_half=0 V1, V2, !VP4; 	nop


NOP; NOP; UNPACK.U8 V1, V2, GROUP_1, STRIDE_4, GROUP_HALF_0
NOP; NOP; UNPACK.U8 V1, V2, GROUP_1, STRIDE_4, GROUP_HALF_0, SP4
NOP; NOP; UNPACK.U8 V1, V2, GROUP_1, STRIDE_4, GROUP_HALF_0, !SP4
NOP; NOP; UNPACK.U8 V1, V2, GROUP_1, STRIDE_4, GROUP_HALF_0, VP4
NOP; NOP; UNPACK.U8 V1, V2, GROUP_1, STRIDE_4, GROUP_HALF_0, !VP4

// CHECK: nop; 	nop; 	unpack.u8  source_group=1 element_stride=4 group_half=0 V1, V2; 	nop
// CHECK: nop; 	nop; 	unpack.u8  source_group=1 element_stride=4 group_half=0 V1, V2, SP4; 	nop
// CHECK: nop; 	nop; 	unpack.u8  source_group=1 element_stride=4 group_half=0 V1, V2, !SP4; 	nop
// CHECK: nop; 	nop; 	unpack.u8  source_group=1 element_stride=4 group_half=0 V1, V2, VP4; 	nop
// CHECK: nop; 	nop; 	unpack.u8  source_group=1 element_stride=4 group_half=0 V1, V2, !VP4; 	nop
