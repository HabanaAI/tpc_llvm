// RUN: %clang -cc1as -triple tpc-none-none -target-cpu doron1 -filetype obj %s -o %t.o
// RUN: %disasm --mcpu doron1 %t.o | FileCheck %s

FCLASS.F32 V6, V7; NOP; NOP; NOP;
FCLASS.F32 V6, S7; NOP; NOP; NOP;

// CHECK: fclass.f32  V6, V7;   nop;   nop;   nop
// CHECK: fclass.f32  V6, S7;   nop;   nop;   nop

FCLASS.F32 V6, V7, SP1; NOP; NOP; NOP;
FCLASS.F32 V6, V7, !SP1; NOP; NOP; NOP;

// CHECK: fclass.f32  V6, V7, SP1;   nop;   nop;   nop
// CHECK: fclass.f32  V6, V7, !SP1;   nop;   nop;   nop

FCLASS.F32 V6, V7, VP1; NOP; NOP; NOP;
FCLASS.F32 V6, V7, !VP1; NOP; NOP; NOP;

// CHECK: fclass.f32  V6, V7, VP1;   nop;   nop;   nop
// CHECK: fclass.f32  V6, V7, !VP1;   nop;   nop;   nop
