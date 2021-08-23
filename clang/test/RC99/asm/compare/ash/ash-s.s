// RUN: %clang -cc1as -triple tpc-none-none -filetype obj %s -o %t.o
// RUN: %disasm %t.o | FileCheck %s
// RUN: %clang -cc1as -triple tpc-none-none -target-cpu gaudi -filetype obj %s -o %t.o
// RUN: %disasm -mcpu=gaudi %t.o | FileCheck %s



NOP; ASH.I32 S5, S1, S2
NOP; ASH.U32 S5, S1, S2, SP3
NOP; ASH.I8  S5, S1, S2, !SP3

// CHECK: nop; ash.i32 S5, S1, S2, SP0; nop; nop
// CHECK: nop; ash.u32 S5, S1, S2, SP3; nop; nop
// CHECK: nop; ash.i8 S5, S1, S2, !SP3; nop; nop

NOP; ASH.U8  S5, S1, i8_123
NOP; ASH.I16 S5, S1, i8_123, SP3
NOP; ASH.U16 S5, S1, i8_-15, !SP3

// CHECK: nop; ash.u8 S5, S1, 0x7b, SP0; nop; nop
// CHECK: nop; ash.i16 S5, S1, 0x7b, SP3; nop; nop
// CHECK: nop; ash.u16 S5, S1, 0xfffffff1, !SP3; nop; nop

NOP; ASH.I32 S5, S1, S2, RHU
NOP; ASH.U32 S5, S1, S2, RHU, SP3
NOP; ASH.I8  S5, S1, S2, RHU, !SP3

// CHECK: nop; ash.i32 rhu S5, S1, S2, SP0; nop; nop
// CHECK: nop; ash.u32 rhu S5, S1, S2, SP3; nop; nop
// CHECK: nop; ash.i8 rhu S5, S1, S2, !SP3; nop; nop

NOP; ASH.U8  S5, S1, i8_123, RHU
NOP; ASH.I16 S5, S1, i8_123, RHU, SP3
NOP; ASH.U16 S5, S1, i8_-15, RHU, !SP3

// CHECK: nop; ash.u8 rhu S5, S1, 0x7b, SP0; nop; nop
// CHECK: nop; ash.i16 rhu S5, S1, 0x7b, SP3; nop; nop
// CHECK: nop; ash.u16 rhu S5, S1, 0xfffffff1, !SP3; nop; nop
