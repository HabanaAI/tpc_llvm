// RUN: %clang -cc1as -triple tpc-none-none -filetype obj -target-cpu gaudi %s -o %t.gen2.o
// RUN: %disasm -mcpu gaudi %t.gen2.o | FileCheck %s







NOP; NOP; SHUFFLE.BF16 V1, V2, V3
NOP; NOP; SHUFFLE.BF16 V1, V2, V3, SP4
NOP; NOP; SHUFFLE.BF16 V1, V2, V3, !SP4
NOP; NOP; SHUFFLE.BF16 V1, V2, V3, VP4
NOP; NOP; SHUFFLE.BF16 V1, V2, V3, !VP4

// CHECK: nop;    nop;    shuffle.bf16  V1, V2, V3, SP0;   nop
// CHECK: nop;    nop;    shuffle.bf16  V1, V2, V3, SP4;   nop
// CHECK: nop;    nop;    shuffle.bf16  V1, V2, V3, !SP4;  nop
// CHECK: nop;    nop;    shuffle.bf16  V1, V2, V3, VP4;   nop
// CHECK: nop;    nop;    shuffle.bf16  V1, V2, V3, !VP4;  nop
