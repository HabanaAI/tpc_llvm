// RUN: %clang -cc1as -triple tpc-none-none -filetype obj %s -o %t.o
// RUN: %disasm %t.o | FileCheck %s


NOP; NOP; MOV V4, V5
NOP; NOP; MOV V4, V5, SP2
NOP; NOP; MOV V4, V5, !SP2
NOP; NOP; MOV V4, V5, VP2
NOP; NOP; MOV V4, V5, !VP2

// CHECK: nop;    nop;    mov V4, V5, SP0;    nop
// CHECK: nop;    nop;    mov V4, V5, SP2;    nop
// CHECK: nop;    nop;    mov V4, V5, !SP2;   nop
// CHECK: nop;    nop;    mov V4, V5, VP2;    nop
// CHECK: nop;    nop;    mov V4, V5, !VP2;   nop

NOP; NOP; MOV.I32 V4, V5
NOP; NOP; MOV.I32 V4, V5, SP2
NOP; NOP; MOV.I32 V4, V5, !SP2
NOP; NOP; MOV.I32 V4, V5, VP2
NOP; NOP; MOV.I32 V4, V5, !VP2

// CHECK: nop;    nop;    mov V4, V5, SP0;    nop
// CHECK: nop;    nop;    mov V4, V5, SP2;    nop
// CHECK: nop;    nop;    mov V4, V5, !SP2;   nop
// CHECK: nop;    nop;    mov V4, V5, VP2;    nop
// CHECK: nop;    nop;    mov V4, V5, !VP2;   nop


NOP; NOP; MOV VP4, VP5
NOP; NOP; MOV VP4, VP5, SP2
NOP; NOP; MOV VP4, VP5, !SP2
NOP; NOP; MOV VP4, VP5, VP2
NOP; NOP; MOV VP4, VP5, !VP2

// CHECK: nop;    nop;    mov VP4, VP5, SP0;      nop
// CHECK: nop;    nop;    mov VP4, VP5, SP2;      nop
// CHECK: nop;    nop;    mov VP4, VP5, !SP2;     nop
// CHECK: nop;    nop;    mov VP4, VP5, VP2;      nop
// CHECK: nop;    nop;    mov VP4, VP5, !VP2;     nop

NOP; NOP; MOV.B VP4, VP5
NOP; NOP; MOV.B VP4, VP5, SP2
NOP; NOP; MOV.B VP4, VP5, !SP2
NOP; NOP; MOV.B VP4, VP5, VP2
NOP; NOP; MOV.B VP4, VP5, !VP2

// CHECK: nop;    nop;    mov VP4, VP5, SP0;      nop
// CHECK: nop;    nop;    mov VP4, VP5, SP2;      nop
// CHECK: nop;    nop;    mov VP4, VP5, !SP2;     nop
// CHECK: nop;    nop;    mov VP4, VP5, VP2;      nop
// CHECK: nop;    nop;    mov VP4, VP5, !VP2;     nop


NOP; NOP; MOV VP4, SP5
NOP; NOP; MOV VP4, SP5, SP2
NOP; NOP; MOV VP4, SP5, !SP2
NOP; NOP; MOV VP4, SP5, VP2
NOP; NOP; MOV VP4, SP5, !VP2

// CHECK: nop;    nop;    mov VP4, SP5, SP0;      nop
// CHECK: nop;    nop;    mov VP4, SP5, SP2;      nop
// CHECK: nop;    nop;    mov VP4, SP5, !SP2;     nop
// CHECK: nop;    nop;    mov VP4, SP5, VP2;      nop
// CHECK: nop;    nop;    mov VP4, SP5, !VP2;     nop

NOP; NOP; MOV.B VP4, SP5
NOP; NOP; MOV.B VP4, SP5, SP2
NOP; NOP; MOV.B VP4, SP5, !SP2
NOP; NOP; MOV.B VP4, SP5, VP2
NOP; NOP; MOV.B VP4, SP5, !VP2

// CHECK: nop;    nop;    mov VP4, SP5, SP0;      nop
// CHECK: nop;    nop;    mov VP4, SP5, SP2;      nop
// CHECK: nop;    nop;    mov VP4, SP5, !SP2;     nop
// CHECK: nop;    nop;    mov VP4, SP5, VP2;      nop
// CHECK: nop;    nop;    mov VP4, SP5, !VP2;     nop





NOP; NOP; MOV.I32 V4, 123
NOP; NOP; MOV.I32 V4, 123, SP2
NOP; NOP; MOV.I32 V4, 123, !SP2
NOP; NOP; MOV.I32 V4, 123, VP2
NOP; NOP; MOV.I32 V4, 123, !VP2

// CHECK: nop;    nop;    mov.i32 V4, 0x7b, SP0;  nop
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

// CHECK: nop;    nop;    mov.i32 V17, 0x0, SP0;  nop
// CHECK: nop;    nop;    mov.i32 V17, 0x1, SP0;  nop
// CHECK: nop;    nop;    mov.i32 V17, 0x2, SP0;  nop
// CHECK: nop;    nop;    mov.i32 V17, 0x3, SP0;  nop
// CHECK: nop;    nop;    mov.i32 V17, 0x4, SP0;  nop
// CHECK: nop;    nop;    mov.i32 V17, 0x5, SP0;  nop
// CHECK: nop;    nop;    mov.i32 V17, 0x6, SP0;  nop
// CHECK: nop;    nop;    mov.i32 V17, 0x7, SP0;  nop
// CHECK: nop;    nop;    mov.i32 V17, 0x8, SP0;  nop
// CHECK: nop;    nop;    mov.i32 V17, 0x9, SP0;  nop
// CHECK: nop;    nop;    mov.i32 V17, 0xa, SP0;  nop
// CHECK: nop;    nop;    mov.i32 V17, 0xb, SP0;  nop
// CHECK: nop;    nop;    mov.i32 V17, 0xc, SP0;  nop
// CHECK: nop;    nop;    mov.i32 V17, 0xd, SP0;  nop
// CHECK: nop;    nop;    mov.i32 V17, 0xe, SP0;  nop
// CHECK: nop;    nop;    mov.i32 V17, 0xf, SP0;  nop
// CHECK: nop;    nop;    mov.i32 V17, 0x10, SP0;         nop


NOP; NOP; MOV.I32 V4, S6
NOP; NOP; MOV.I32 V4, S6, SP2
NOP; NOP; MOV.I32 V4, S6, !SP2
NOP; NOP; MOV.I32 V4, S6, VP2
NOP; NOP; MOV.I32 V4, S6, !VP2

// CHECK: nop;    nop;    mov.i32 V4, S6, SP0;    nop
// CHECK: nop;    nop;    mov.i32 V4, S6, SP2;    nop
// CHECK: nop;    nop;    mov.i32 V4, S6, !SP2;   nop
// CHECK: nop;    nop;    mov.i32 V4, S6, VP2;    nop
// CHECK: nop;    nop;    mov.i32 V4, S6, !VP2;   nop


NOP; NOP; MOV 0 VP4, 123
NOP; NOP; MOV 1 VP4, 123, SP2
NOP; NOP; MOV 3 VP4, 123, !SP2
NOP; NOP; MOV 6 VP4, 123, VP2
NOP; NOP; MOV 7 VP4, 123, !VP2

// CHECK: nop;    nop;    mov 0x0 VP4, 0x7b, SP0;  nop
// CHECK: nop;    nop;    mov 0x1 VP4, 0x7b, SP2;  nop
// CHECK: nop;    nop;    mov 0x3 VP4, 0x7b, !SP2; nop
// CHECK: nop;    nop;    mov 0x6 VP4, 0x7b, VP2;  nop
// CHECK: nop;    nop;    mov 0x7 VP4, 0x7b, !VP2; nop

NOP; NOP; MOV 8 VP4, 123
NOP; NOP; MOV 8 VP4, 123, SP2
NOP; NOP; MOV 8 VP4, 123, !SP2
NOP; NOP; MOV 8 VP4, 123, VP2
NOP; NOP; MOV 8 VP4, 123, !VP2

// CHECK: nop;    nop;    mov VP4, 0x7b, SP0;  nop
// CHECK: nop;    nop;    mov VP4, 0x7b, SP2;  nop
// CHECK: nop;    nop;    mov VP4, 0x7b, !SP2; nop
// CHECK: nop;    nop;    mov VP4, 0x7b, VP2;  nop
// CHECK: nop;    nop;    mov VP4, 0x7b, !VP2; nop

NOP; NOP; MOV VP4, 123
NOP; NOP; MOV VP4, 123, SP2
NOP; NOP; MOV VP4, 123, !SP2
NOP; NOP; MOV VP4, 123, VP2
NOP; NOP; MOV VP4, 123, !VP2

// CHECK: nop;    nop;    mov VP4, 0x7b, SP0;  nop
// CHECK: nop;    nop;    mov VP4, 0x7b, SP2;  nop
// CHECK: nop;    nop;    mov VP4, 0x7b, !SP2; nop
// CHECK: nop;    nop;    mov VP4, 0x7b, VP2;  nop
// CHECK: nop;    nop;    mov VP4, 0x7b, !VP2; nop

NOP; NOP; MOV 0 VP4, S0
NOP; NOP; MOV 1 VP4, S1, SP2
NOP; NOP; MOV 3 VP4, S20, !SP2
NOP; NOP; MOV 6 VP4, S21, VP2
NOP; NOP; MOV 7 VP4, S22, !VP2

// CHECK: nop;    nop;    mov 0x0 VP4, S0, SP0;  nop
// CHECK: nop;    nop;    mov 0x1 VP4, S1, SP2;  nop
// CHECK: nop;    nop;    mov 0x3 VP4, S20, !SP2; nop
// CHECK: nop;    nop;    mov 0x6 VP4, S21, VP2;  nop
// CHECK: nop;    nop;    mov 0x7 VP4, S22, !VP2; nop

NOP; NOP; MOV 8 VP4, S0
NOP; NOP; MOV 8 VP4, S1, SP2
NOP; NOP; MOV 8 VP4, S20, !SP2
NOP; NOP; MOV 8 VP4, S21, VP2
NOP; NOP; MOV 8 VP4, S22, !VP2

// CHECK: nop;    nop;    mov VP4, S0, SP0;  nop
// CHECK: nop;    nop;    mov VP4, S1, SP2;  nop
// CHECK: nop;    nop;    mov VP4, S20, !SP2; nop
// CHECK: nop;    nop;    mov VP4, S21, VP2;  nop
// CHECK: nop;    nop;    mov VP4, S22, !VP2; nop

NOP; NOP; MOV VP4, S0
NOP; NOP; MOV VP4, S1, SP2
NOP; NOP; MOV VP4, S20, !SP2
NOP; NOP; MOV VP4, S21, VP2
NOP; NOP; MOV VP4, S22, !VP2

// CHECK: nop;    nop;    mov VP4, S0, SP0;  nop
// CHECK: nop;    nop;    mov VP4, S1, SP2;  nop
// CHECK: nop;    nop;    mov VP4, S20, !SP2; nop
// CHECK: nop;    nop;    mov VP4, S21, VP2;  nop
// CHECK: nop;    nop;    mov VP4, S22, !VP2; nop

NOP; NOP; MOV.U32 0 VP4, 123
NOP; NOP; MOV.U32 1 VP4, 123, SP2
NOP; NOP; MOV.U32 5 VP4, 123, !SP2
NOP; NOP; MOV.U32 6 VP4, 123, VP2
NOP; NOP; MOV.U32 7 VP4, 123, !VP2

// CHECK: nop; nop; mov 0x0 VP4, 0x7b, SP0; nop
// CHECK: nop; nop; mov 0x1 VP4, 0x7b, SP2; nop
// CHECK: nop; nop; mov 0x5 VP4, 0x7b, !SP2; nop
// CHECK: nop; nop; mov 0x6 VP4, 0x7b, VP2; nop
// CHECK: nop; nop; mov 0x7 VP4, 0x7b, !VP2; nop

NOP; NOP; MOV.U32 0 VP4, S0
NOP; NOP; MOV.U32 1 VP4, S1, SP2
NOP; NOP; MOV.U32 3 VP4, S20, !SP2
NOP; NOP; MOV.U32 6 VP4, S21, VP2
NOP; NOP; MOV.U32 7 VP4, S22, !VP2

// CHECK: nop;    nop;    mov 0x0 VP4, S0, SP0;  nop
// CHECK: nop;    nop;    mov 0x1 VP4, S1, SP2;  nop
// CHECK: nop;    nop;    mov 0x3 VP4, S20, !SP2; nop
// CHECK: nop;    nop;    mov 0x6 VP4, S21, VP2;  nop
// CHECK: nop;    nop;    mov 0x7 VP4, S22, !VP2; nop
