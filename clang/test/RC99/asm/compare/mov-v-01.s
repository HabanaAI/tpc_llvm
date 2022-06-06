// RUN: %clang -cc1as -triple tpc-none-none -filetype obj %s -o %t.o
// RUN: %disasm %t.o | FileCheck %s


NOP; NOP; MOV V4, V5
NOP; NOP; MOV V4, V5, SP2
NOP; NOP; MOV V4, V5, !SP2
NOP; NOP; MOV V4, V5, VP2
NOP; NOP; MOV V4, V5, !VP2

// CHECK: nop;    nop;    mov V4, V5;    nop
// CHECK: nop;    nop;    mov V4, V5, SP2;    nop
// CHECK: nop;    nop;    mov V4, V5, !SP2;   nop
// CHECK: nop;    nop;    mov V4, V5, VP2;    nop
// CHECK: nop;    nop;    mov V4, V5, !VP2;   nop

NOP; NOP; MOV.I32 V4, V5
NOP; NOP; MOV.I32 V4, V5, SP2
NOP; NOP; MOV.I32 V4, V5, !SP2
NOP; NOP; MOV.I32 V4, V5, VP2
NOP; NOP; MOV.I32 V4, V5, !VP2

// CHECK: nop;    nop;    mov V4, V5;    nop
// CHECK: nop;    nop;    mov V4, V5, SP2;    nop
// CHECK: nop;    nop;    mov V4, V5, !SP2;   nop
// CHECK: nop;    nop;    mov V4, V5, VP2;    nop
// CHECK: nop;    nop;    mov V4, V5, !VP2;   nop


NOP; NOP; MOV VP4, VP5
NOP; NOP; MOV VP4, VP5, SP2
NOP; NOP; MOV VP4, VP5, !SP2
NOP; NOP; MOV VP4, VP5, VP2
NOP; NOP; MOV VP4, VP5, !VP2

// CHECK: nop;    nop;    mov VP4, VP5;      nop
// CHECK: nop;    nop;    mov VP4, VP5, SP2;      nop
// CHECK: nop;    nop;    mov VP4, VP5, !SP2;     nop
// CHECK: nop;    nop;    mov VP4, VP5, VP2;      nop
// CHECK: nop;    nop;    mov VP4, VP5, !VP2;     nop

NOP; NOP; MOV.B VP4, VP5
NOP; NOP; MOV.B VP4, VP5, SP2
NOP; NOP; MOV.B VP4, VP5, !SP2
NOP; NOP; MOV.B VP4, VP5, VP2
NOP; NOP; MOV.B VP4, VP5, !VP2

// CHECK: nop;    nop;    mov VP4, VP5;      nop
// CHECK: nop;    nop;    mov VP4, VP5, SP2;      nop
// CHECK: nop;    nop;    mov VP4, VP5, !SP2;     nop
// CHECK: nop;    nop;    mov VP4, VP5, VP2;      nop
// CHECK: nop;    nop;    mov VP4, VP5, !VP2;     nop


NOP; NOP; MOV VP4, SP5
NOP; NOP; MOV VP4, SP5, SP2
NOP; NOP; MOV VP4, SP5, !SP2
NOP; NOP; MOV VP4, SP5, VP2
NOP; NOP; MOV VP4, SP5, !VP2

// CHECK: nop;    nop;    mov VP4, SP5;      nop
// CHECK: nop;    nop;    mov VP4, SP5, SP2;      nop
// CHECK: nop;    nop;    mov VP4, SP5, !SP2;     nop
// CHECK: nop;    nop;    mov VP4, SP5, VP2;      nop
// CHECK: nop;    nop;    mov VP4, SP5, !VP2;     nop

NOP; NOP; MOV.B VP4, SP5
NOP; NOP; MOV.B VP4, SP5, SP2
NOP; NOP; MOV.B VP4, SP5, !SP2
NOP; NOP; MOV.B VP4, SP5, VP2
NOP; NOP; MOV.B VP4, SP5, !VP2

// CHECK: nop;    nop;    mov VP4, SP5;      nop
// CHECK: nop;    nop;    mov VP4, SP5, SP2;      nop
// CHECK: nop;    nop;    mov VP4, SP5, !SP2;     nop
// CHECK: nop;    nop;    mov VP4, SP5, VP2;      nop
// CHECK: nop;    nop;    mov VP4, SP5, !VP2;     nop





NOP; NOP; MOV.I32 V4, 123
NOP; NOP; MOV.I32 V4, 123, SP2
NOP; NOP; MOV.I32 V4, 123, !SP2
NOP; NOP; MOV.I32 V4, 123, VP2
NOP; NOP; MOV.I32 V4, 123, !VP2

// CHECK: nop;    nop;    mov.i32 V4, 0x7b;  nop
// CHECK: nop;    nop;    mov.i32 V4, 0x7b, SP2;  nop
// CHECK: nop;    nop;    mov.i32 V4, 0x7b, !SP2;         nop
// CHECK: nop;    nop;    mov.i32 V4, 0x7b, VP2;  nop
// CHECK: nop;    nop;    mov.i32 V4, 0x7b, !VP2;         nop

NOP; NOP; MOV.I32 V17, 0
NOP; NOP; MOV.I32 V17, 1
NOP; NOP; MOV.I32 V17, 2
NOP; NOP; MOV.I32 V17, 3
NOP; NOP; MOV.I32 V17, 4
NOP; NOP; MOV.I32 V17, 5
NOP; NOP; MOV.I32 V17, 6
NOP; NOP; MOV.I32 V17, 7
NOP; NOP; MOV.I32 V17, 8
NOP; NOP; MOV.I32 V17, 9
NOP; NOP; MOV.I32 V17, 10
NOP; NOP; MOV.I32 V17, 11
NOP; NOP; MOV.I32 V17, 12
NOP; NOP; MOV.I32 V17, 13
NOP; NOP; MOV.I32 V17, 14
NOP; NOP; MOV.I32 V17, 15
NOP; NOP; MOV.I32 V17, 16

// CHECK: nop;    nop;    mov.i32 V17, 0x0;  nop
// CHECK: nop;    nop;    mov.i32 V17, 0x1;  nop
// CHECK: nop;    nop;    mov.i32 V17, 0x2;  nop
// CHECK: nop;    nop;    mov.i32 V17, 0x3;  nop
// CHECK: nop;    nop;    mov.i32 V17, 0x4;  nop
// CHECK: nop;    nop;    mov.i32 V17, 0x5;  nop
// CHECK: nop;    nop;    mov.i32 V17, 0x6;  nop
// CHECK: nop;    nop;    mov.i32 V17, 0x7;  nop
// CHECK: nop;    nop;    mov.i32 V17, 0x8;  nop
// CHECK: nop;    nop;    mov.i32 V17, 0x9;  nop
// CHECK: nop;    nop;    mov.i32 V17, 0xa;  nop
// CHECK: nop;    nop;    mov.i32 V17, 0xb;  nop
// CHECK: nop;    nop;    mov.i32 V17, 0xc;  nop
// CHECK: nop;    nop;    mov.i32 V17, 0xd;  nop
// CHECK: nop;    nop;    mov.i32 V17, 0xe;  nop
// CHECK: nop;    nop;    mov.i32 V17, 0xf;  nop
// CHECK: nop;    nop;    mov.i32 V17, 0x10;         nop


NOP; NOP; MOV.I32 V4, S6
NOP; NOP; MOV.I32 V4, S6, SP2
NOP; NOP; MOV.I32 V4, S6, !SP2
NOP; NOP; MOV.I32 V4, S6, VP2
NOP; NOP; MOV.I32 V4, S6, !VP2

// CHECK: nop;    nop;    mov.i32 V4, S6;    nop
// CHECK: nop;    nop;    mov.i32 V4, S6, SP2;    nop
// CHECK: nop;    nop;    mov.i32 V4, S6, !SP2;   nop
// CHECK: nop;    nop;    mov.i32 V4, S6, VP2;    nop
// CHECK: nop;    nop;    mov.i32 V4, S6, !VP2;   nop


NOP; NOP; MOV 0 VP4, 123
NOP; NOP; MOV 1 VP4, 123, SP2
NOP; NOP; MOV 3 VP4, 123, !SP2
NOP; NOP; MOV 6 VP4, 123, VP2
NOP; NOP; MOV 7 VP4, 123, !VP2

// CHECK: nop;    nop;    mov 0x0 VP4, 0x7b;  nop
// CHECK: nop;    nop;    mov 0x1 VP4, 0x7b, SP2;  nop
// CHECK: nop;    nop;    mov 0x3 VP4, 0x7b, !SP2; nop
// CHECK: nop;    nop;    mov 0x6 VP4, 0x7b, VP2;  nop
// CHECK: nop;    nop;    mov 0x7 VP4, 0x7b, !VP2; nop

NOP; NOP; MOV 8 VP4, 123
NOP; NOP; MOV 8 VP4, 123, SP2
NOP; NOP; MOV 8 VP4, 123, !SP2
NOP; NOP; MOV 8 VP4, 123, VP2
NOP; NOP; MOV 8 VP4, 123, !VP2

// CHECK: nop;    nop;    mov VP4, 0x7b;  nop
// CHECK: nop;    nop;    mov VP4, 0x7b, SP2;  nop
// CHECK: nop;    nop;    mov VP4, 0x7b, !SP2; nop
// CHECK: nop;    nop;    mov VP4, 0x7b, VP2;  nop
// CHECK: nop;    nop;    mov VP4, 0x7b, !VP2; nop

NOP; NOP; MOV VP4, 123
NOP; NOP; MOV VP4, 123, SP2
NOP; NOP; MOV VP4, 123, !SP2
NOP; NOP; MOV VP4, 123, VP2
NOP; NOP; MOV VP4, 123, !VP2

// CHECK: nop;    nop;    mov VP4, 0x7b;  nop
// CHECK: nop;    nop;    mov VP4, 0x7b, SP2;  nop
// CHECK: nop;    nop;    mov VP4, 0x7b, !SP2; nop
// CHECK: nop;    nop;    mov VP4, 0x7b, VP2;  nop
// CHECK: nop;    nop;    mov VP4, 0x7b, !VP2; nop

NOP; NOP; MOV 0 VP4, S0
NOP; NOP; MOV 1 VP4, S1, SP2
NOP; NOP; MOV 3 VP4, S20, !SP2
NOP; NOP; MOV 6 VP4, S21, VP2
NOP; NOP; MOV 7 VP4, S22, !VP2

// CHECK: nop;    nop;    mov 0x0 VP4, S0;  nop
// CHECK: nop;    nop;    mov 0x1 VP4, S1, SP2;  nop
// CHECK: nop;    nop;    mov 0x3 VP4, S20, !SP2; nop
// CHECK: nop;    nop;    mov 0x6 VP4, S21, VP2;  nop
// CHECK: nop;    nop;    mov 0x7 VP4, S22, !VP2; nop

NOP; NOP; MOV 8 VP4, S0
NOP; NOP; MOV 8 VP4, S1, SP2
NOP; NOP; MOV 8 VP4, S20, !SP2
NOP; NOP; MOV 8 VP4, S21, VP2
NOP; NOP; MOV 8 VP4, S22, !VP2

// CHECK: nop;    nop;    mov VP4, S0;  nop
// CHECK: nop;    nop;    mov VP4, S1, SP2;  nop
// CHECK: nop;    nop;    mov VP4, S20, !SP2; nop
// CHECK: nop;    nop;    mov VP4, S21, VP2;  nop
// CHECK: nop;    nop;    mov VP4, S22, !VP2; nop

NOP; NOP; MOV VP4, S0
NOP; NOP; MOV VP4, S1, SP2
NOP; NOP; MOV VP4, S20, !SP2
NOP; NOP; MOV VP4, S21, VP2
NOP; NOP; MOV VP4, S22, !VP2

// CHECK: nop;    nop;    mov VP4, S0;  nop
// CHECK: nop;    nop;    mov VP4, S1, SP2;  nop
// CHECK: nop;    nop;    mov VP4, S20, !SP2; nop
// CHECK: nop;    nop;    mov VP4, S21, VP2;  nop
// CHECK: nop;    nop;    mov VP4, S22, !VP2; nop

NOP; NOP; MOV.U32 0 VP4, 123
NOP; NOP; MOV.U32 1 VP4, 123, SP2
NOP; NOP; MOV.U32 5 VP4, 123, !SP2
NOP; NOP; MOV.U32 6 VP4, 123, VP2
NOP; NOP; MOV.U32 7 VP4, 123, !VP2

// CHECK: nop; nop; mov 0x0 VP4, 0x7b; nop
// CHECK: nop; nop; mov 0x1 VP4, 0x7b, SP2; nop
// CHECK: nop; nop; mov 0x5 VP4, 0x7b, !SP2; nop
// CHECK: nop; nop; mov 0x6 VP4, 0x7b, VP2; nop
// CHECK: nop; nop; mov 0x7 VP4, 0x7b, !VP2; nop

NOP; NOP; MOV.U32 0 VP4, S0
NOP; NOP; MOV.U32 1 VP4, S1, SP2
NOP; NOP; MOV.U32 3 VP4, S20, !SP2
NOP; NOP; MOV.U32 6 VP4, S21, VP2
NOP; NOP; MOV.U32 7 VP4, S22, !VP2

// CHECK: nop;    nop;    mov 0x0 VP4, S0;  nop
// CHECK: nop;    nop;    mov 0x1 VP4, S1, SP2;  nop
// CHECK: nop;    nop;    mov 0x3 VP4, S20, !SP2; nop
// CHECK: nop;    nop;    mov 0x6 VP4, S21, VP2;  nop
// CHECK: nop;    nop;    mov 0x7 VP4, S22, !VP2; nop
