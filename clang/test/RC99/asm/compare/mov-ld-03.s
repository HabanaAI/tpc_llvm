// RUN: %clang -cc1as -triple tpc-none-none -filetype obj -target-cpu goya2 %s -o %t.o
// RUN: %disasm --mcpu goya2 %t.o | FileCheck %s


MOV VP14, V10
MOV VP14, V10, SP2
MOV VP14, V10, !SP2
MOV VP14, V10, VP2
MOV VP14, V10, !VP2

// CHECK: mov VP14, V10;     nop;    nop;    nop
// CHECK: mov VP14, V10, SP2;     nop;    nop;    nop
// CHECK: mov VP14, V10, !SP2;    nop;    nop;    nop
// CHECK: mov VP14, V10, VP2;     nop;    nop;    nop
// CHECK: mov VP14, V10, !VP2;    nop;    nop;    nop

MOV V14, VP10
MOV V14, VP10, SP2
MOV V14, VP10, !SP2
MOV V14, VP10, VP2
MOV V14, VP10, !VP2

// CHECK: mov V14, VP10;     nop;    nop;    nop
// CHECK: mov V14, VP10, SP2;     nop;    nop;    nop
// CHECK: mov V14, VP10, !SP2;    nop;    nop;    nop
// CHECK: mov V14, VP10, VP2;     nop;    nop;    nop
// CHECK: mov V14, VP10, !VP2;    nop;    nop;    nop


MOV AD3, AD4
MOV AD3, AD4, SP5
MOV AD3, AD4, !SP5

// CHECK: mov AD3, AD4; nop; nop; nop
// CHECK: mov AD3, AD4, SP5; nop; nop; nop
// CHECK: mov AD3, AD4, !SP5; nop; nop; nop


MOV.TO_HW_REG 1, S1
MOV.TO_HW_REG 1, S1, SP2
MOV.TO_HW_REG 1, S1, !SP2

// CHECK: mov DIV_STEP, S1;  nop;    nop;    nop
// CHECK: mov DIV_STEP, S1, SP2;  nop;    nop;    nop
// CHECK: mov DIV_STEP, S1, !SP2;         nop;    nop;    nop

MOV DIV_STEP, S1
MOV DIV_STEP, S1, SP2
MOV DIV_STEP, S1, !SP2

// CHECK: mov DIV_STEP, S1;  nop;    nop;    nop
// CHECK: mov DIV_STEP, S1, SP2;  nop;    nop;    nop
// CHECK: mov DIV_STEP, S1, !SP2;         nop;    nop;    nop
