// RUN: %clang -cc1as -triple tpc-none-none -filetype obj %s -o %t.o
// RUN: %disasm  %t.o | FileCheck %s
// RUN: %clang -cc1as -triple tpc-none-none -target-cpu gaudi -filetype obj %s -o %t.o
// RUN: %disasm -mcpu=gaudi %t.o | FileCheck %s




NOP; NOP; ASH.I32 V3, V2, V1
NOP; NOP; ASH.U32 V3, V2, V1, SP1
NOP; NOP; ASH.I16 V3, V2, V1, !SP1
NOP; NOP; ASH.U16 V3, V2, V1, VP1
NOP; NOP; ASH.I8  V3, V2, V1, !VP1

// CHECK: nop; nop; ash.i32 V3, V2, V1, SP0; nop
// CHECK: nop; nop; ash.u32 V3, V2, V1, SP1; nop
// CHECK: nop; nop; ash.i16 V3, V2, V1, !SP1; nop
// CHECK: nop; nop; ash.u16 V3, V2, V1, VP1; nop
// CHECK: nop; nop; ash.i8 V3, V2, V1, !VP1; nop


NOP; NOP; ASH.U8  V3, V2, i8_6
NOP; NOP; ASH.F32 V3, V2, i8_2, SP1
NOP; NOP; ASH.BF16 V3, V2, i8_15, !SP1
NOP; NOP; ASH.I32 V3, V2, i8_2, VP1
NOP; NOP; ASH.I8  V3, V2, i8_4, !VP1

// CHECK: nop; nop; ash.u8 V3, V2, 0x6, SP0; nop
// CHECK: nop; nop; ash.f32 V3, V2, 0x2, SP1; nop
// CHECK: nop; nop; ash.bf16 V3, V2, 0xf, !SP1; nop
// CHECK: nop; nop; ash.i32 V3, V2, 0x2, VP1; nop
// CHECK: nop; nop; ash.i8 V3, V2, 0x4, !VP1; nop


NOP; NOP; ASH.I32 V3, V2, S1
NOP; NOP; ASH.I32 V3, V2, S1, SP6
NOP; NOP; ASH.I32 V3, V2, S1, !SP6
NOP; NOP; ASH.I32 V3, V2, S1, VP6
NOP; NOP; ASH.I32 V3, V2, S1, !VP6

// CHECK: nop; nop; ash.i32 V3, V2, S1, SP0; nop
// CHECK: nop; nop; ash.i32 V3, V2, S1, SP6; nop
// CHECK: nop; nop; ash.i32 V3, V2, S1, !SP6; nop
// CHECK: nop; nop; ash.i32 V3, V2, S1, VP6; nop
// CHECK: nop; nop; ash.i32 V3, V2, S1, !VP6; nop


NOP; NOP; ASH.I32 V3, V2, V1, RHU
NOP; NOP; ASH.I32 V3, V2, V1, RHU, SP6
NOP; NOP; ASH.I32 V3, V2, V1, RHU, !SP6
NOP; NOP; ASH.I32 V3, V2, V1, RHU, VP6
NOP; NOP; ASH.I32 V3, V2, V1, RHU, !VP6

// CHECK: nop; nop; ash.i32 rhu V3, V2, V1, SP0; nop
// CHECK: nop; nop; ash.i32 rhu V3, V2, V1, SP6; nop
// CHECK: nop; nop; ash.i32 rhu V3, V2, V1, !SP6; nop
// CHECK: nop; nop; ash.i32 rhu V3, V2, V1, VP6; nop
// CHECK: nop; nop; ash.i32 rhu V3, V2, V1, !VP6; nop


NOP; NOP; ASH.I32 V3, V2, i8_34, RHU
NOP; NOP; ASH.I32 V3, V2, i8_34, RHU, SP6
NOP; NOP; ASH.I32 V3, V2, i8_34, RHU, !SP6
NOP; NOP; ASH.I32 V3, V2, i8_-34, RHU, VP6
NOP; NOP; ASH.I32 V3, V2, i8_-34, RHU, !VP6

// CHECK: nop; nop; ash.i32 rhu V3, V2, 0x22, SP0; nop
// CHECK: nop; nop; ash.i32 rhu V3, V2, 0x22, SP6; nop
// CHECK: nop; nop; ash.i32 rhu V3, V2, 0x22, !SP6; nop
// CHECK: nop; nop; ash.i32 rhu V3, V2, 0xffffffde, VP6; nop
// CHECK: nop; nop; ash.i32 rhu V3, V2, 0xffffffde, !VP6; nop


NOP; NOP; ASH.I32 V3, V2, S1, RHU
NOP; NOP; ASH.I32 V3, V2, S1, RHU, SP6
NOP; NOP; ASH.I32 V3, V2, S1, RHU, !SP6
NOP; NOP; ASH.I32 V3, V2, S1, RHU, VP6
NOP; NOP; ASH.I32 V3, V2, S1, RHU, !VP6

// CHECK: nop; nop; ash.i32 rhu V3, V2, S1, SP0; nop
// CHECK: nop; nop; ash.i32 rhu V3, V2, S1, SP6; nop
// CHECK: nop; nop; ash.i32 rhu V3, V2, S1, !SP6; nop
// CHECK: nop; nop; ash.i32 rhu V3, V2, S1, VP6; nop
// CHECK: nop; nop; ash.i32 rhu V3, V2, S1, !VP6; nop
