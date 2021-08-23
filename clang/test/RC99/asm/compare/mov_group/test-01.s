// RUN: %clang -cc1as -triple tpc-none-none -target-cpu gaudi -filetype obj %s -o %t.o
// RUN: %disasm -mcpu=gaudi %t.o | FileCheck %s

// GROUP_EN = 0, DUAL_GROUP = 15
nop; nop; mov_group V1, V2, group_en=0, dual_group_en=15, 32; nop
// CHECK: nop; nop; mov_g g_en=0b0 dg_en=0b1111 V1, V2, 0x20, SP0; nop

nop; nop; mov_group V1, V2, group_en=0, dual_group_en=15, 32, SP1; nop
// CHECK: nop; nop; mov_g g_en=0b0 dg_en=0b1111 V1, V2, 0x20, SP1; nop

nop; nop; mov_group V1, V2, group_en=0, dual_group_en=15, 32, !SP1; nop
// CHECK: nop; nop; mov_g g_en=0b0 dg_en=0b1111 V1, V2, 0x20, !SP1; nop

nop; nop; mov_group V1, V2, group_en=0, dual_group_en=15, 32, VP1; nop
// CHECK: nop; nop; mov_g g_en=0b0 dg_en=0b1111 V1, V2, 0x20, VP1; nop

nop; nop; mov_group V1, V2, group_en=0, dual_group_en=15, 32, !VP1; nop
// CHECK: nop; nop; mov_g g_en=0b0 dg_en=0b1111 V1, V2, 0x20, !VP1; nop


// GROUP_EN = 3, DUAL_GROUP = 15
nop; nop; mov_group V1, V2, group_en=3, dual_group_en=15, 32; nop
// CHECK: nop; nop; mov_g g_en=0b11 dg_en=0b1111 V1, V2, 0x20, SP0; nop

nop; nop; mov_group V1, V2, group_en=3, dual_group_en=15, 32, SP1; nop
// CHECK: nop; nop; mov_g g_en=0b11 dg_en=0b1111 V1, V2, 0x20, SP1; nop

nop; nop; mov_group V1, V2, group_en=3, dual_group_en=15, 32, !SP1; nop
// CHECK: nop; nop; mov_g g_en=0b11 dg_en=0b1111 V1, V2, 0x20, !SP1; nop

nop; nop; mov_group V1, V2, group_en=3, dual_group_en=15, 32, VP1; nop
// CHECK: nop; nop; mov_g g_en=0b11 dg_en=0b1111 V1, V2, 0x20, VP1; nop

nop; nop; mov_group V1, V2, group_en=3, dual_group_en=15, 32, !VP1; nop
// CHECK: nop; nop; mov_g g_en=0b11 dg_en=0b1111 V1, V2, 0x20, !VP1; nop

