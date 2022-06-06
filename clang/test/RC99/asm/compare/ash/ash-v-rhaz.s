// RUN: %clang -cc1as -triple tpc-none-none -target-cpu goya2 -filetype obj %s -o %t.o
// RUN: %disasm --mcpu=goya2 %t.o | FileCheck %s
// Gen3 disassembler does not exist


NOP; NOP; ASH.I32.RHAZ_RS V3, D2, V1; NOP
NOP; NOP; ASH.I32.RHAZ_RS V3, D2, V1, SP1; NOP
NOP; NOP; ASH.I32.RHAZ_RS V3, D6, V1, !SP1; NOP
NOP; NOP; ASH.I32.RHAZ_RS V3, D6, V1, VP1; NOP
NOP; NOP; ASH.I32.RHAZ_RS V3, D8, V1, !VP1; NOP

// CHECK: nop; nop; ash.i32  rhaz_rs V3, D2, V1; nop
// CHECK: nop; nop; ash.i32  rhaz_rs V3, D2, V1, SP1; nop
// CHECK: nop; nop; ash.i32  rhaz_rs V3, D6, V1, !SP1; nop
// CHECK: nop; nop; ash.i32  rhaz_rs V3, D6, V1, VP1; nop
// CHECK: nop; nop; ash.i32  rhaz_rs V3, D8, V1, !VP1; nop

NOP; NOP; ASH.U32.RHAZ_RS V3, D2, V1; NOP
NOP; NOP; ASH.U32.RHAZ_RS V3, D2, V1, SP1; NOP
NOP; NOP; ASH.U32.RHAZ_RS V3, D6, V1, !SP1; NOP
NOP; NOP; ASH.U32.RHAZ_RS V3, D6, V1, VP1; NOP
NOP; NOP; ASH.U32.RHAZ_RS V3, D8, V1, !VP1; NOP

// CHECK: nop; nop; ash.u32  rhaz_rs V3, D2, V1; nop
// CHECK: nop; nop; ash.u32  rhaz_rs V3, D2, V1, SP1; nop
// CHECK: nop; nop; ash.u32  rhaz_rs V3, D6, V1, !SP1; nop
// CHECK: nop; nop; ash.u32  rhaz_rs V3, D6, V1, VP1; nop
// CHECK: nop; nop; ash.u32  rhaz_rs V3, D8, V1, !VP1; nop

NOP; NOP; ASH.U8  V3, V2, i8_-5
NOP; NOP; ASH.I32 V3, V2, i8_10, VP1
NOP; NOP; ASH.I8  V3, V2, i8_10, !VP1

// CHECK: nop; nop; ash.u8 V3, V2, 0xfffffffb; nop
// CHECK: nop; nop; ash.i32 V3, V2, 0xa, VP1; nop
// CHECK: nop; nop; ash.i8 V3, V2, 0xa, !VP1; nop


NOP; NOP; ASH.I32 V3, V2, S1
NOP; NOP; ASH.I32 V3, V2, S1, SP6
NOP; NOP; ASH.I32 V3, V2, S1, !SP6
NOP; NOP; ASH.I32 V3, V2, S1, VP6
NOP; NOP; ASH.I32 V3, V2, S1, !VP6

// CHECK: nop; nop; ash.i32 V3, V2, S1; nop
// CHECK: nop; nop; ash.i32 V3, V2, S1, SP6; nop
// CHECK: nop; nop; ash.i32 V3, V2, S1, !SP6; nop
// CHECK: nop; nop; ash.i32 V3, V2, S1, VP6; nop
// CHECK: nop; nop; ash.i32 V3, V2, S1, !VP6; nop


NOP; NOP; ASH.I32 V3, V2, V1, RHU
NOP; NOP; ASH.I32 V3, V2, V1, RHU, SP6
NOP; NOP; ASH.I32 V3, V2, V1, RHU, !SP6
NOP; NOP; ASH.I32 V3, V2, V1, RHU, VP6
NOP; NOP; ASH.I32 V3, V2, V1, RHU, !VP6

// CHECK: nop; nop; ash.i32 rhu V3, V2, V1; nop
// CHECK: nop; nop; ash.i32 rhu V3, V2, V1, SP6; nop
// CHECK: nop; nop; ash.i32 rhu V3, V2, V1, !SP6; nop
// CHECK: nop; nop; ash.i32 rhu V3, V2, V1, VP6; nop
// CHECK: nop; nop; ash.i32 rhu V3, V2, V1, !VP6; nop


NOP; NOP; ASH.I32 V3, V2, I32_1234, RHU
NOP; NOP; ASH.I32 V3, V2, I32_1234, RHU, SP6
NOP; NOP; ASH.I32 V3, V2, I32_1234, RHU, !SP6
NOP; NOP; ASH.I32 V3, V2, I32_1234, RHU, VP6
NOP; NOP; ASH.I32 V3, V2, I32_1234, RHU, !VP6

// CHECK: nop; nop; ash.i32 rhu V3, V2, 0x4d2; nop
// CHECK: nop; nop; ash.i32 rhu V3, V2, 0x4d2, SP6; nop
// CHECK: nop; nop; ash.i32 rhu V3, V2, 0x4d2, !SP6; nop
// CHECK: nop; nop; ash.i32 rhu V3, V2, 0x4d2, VP6; nop
// CHECK: nop; nop; ash.i32 rhu V3, V2, 0x4d2, !VP6; nop


NOP; NOP; ASH.I32 V3, V2, S1, RHU
NOP; NOP; ASH.I32 V3, V2, S1, RHU, SP6
NOP; NOP; ASH.I32 V3, V2, S1, RHU, !SP6
NOP; NOP; ASH.I32 V3, V2, S1, RHU, VP6
NOP; NOP; ASH.I32 V3, V2, S1, RHU, !VP6

// CHECK: nop; nop; ash.i32 rhu V3, V2, S1; nop
// CHECK: nop; nop; ash.i32 rhu V3, V2, S1, SP6; nop
// CHECK: nop; nop; ash.i32 rhu V3, V2, S1, !SP6; nop
// CHECK: nop; nop; ash.i32 rhu V3, V2, S1, VP6; nop
// CHECK: nop; nop; ash.i32 rhu V3, V2, S1, !VP6; nop
