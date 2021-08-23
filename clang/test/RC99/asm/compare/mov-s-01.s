// RUN: %clang -cc1as -triple tpc-none-none -filetype obj %s -o %t.o
// RUN: %disasm %t.o | FileCheck %s

NOP; MOV.I32 S1, S2
NOP; MOV.I32 S1, S2, SP3
NOP; MOV.I32 S1, S2, !SP3

// CHECK: nop;    mov.i32 S1, S2, SP0;    nop;    nop
// CHECK: nop;    mov.i32 S1, S2, SP3;    nop;    nop
// CHECK: nop;    mov.i32 S1, S2, !SP3;   nop;    nop


NOP; MOV.I32 S1, 123
NOP; MOV.I32 S1, 123, SP3
NOP; MOV.I32 S1, 123, !SP3

// CHECK: nop;    mov.i32 S1, 0x7b, SP0;  nop;    nop
// CHECK: nop;    mov.i32 S1, 0x7b, SP3;  nop;    nop
// CHECK: nop;    mov.i32 S1, 0x7b, !SP3;         nop;    nop


NOP; MOV.U32 S1, 0
NOP; MOV.U32 S1, 1
NOP; MOV.U32 S1, 2
NOP; MOV.U32 S1, 3
NOP; MOV.U32 S1, 4
NOP; MOV.U32 S1, 5
NOP; MOV.U32 S1, 6
NOP; MOV.U32 S1, 7
NOP; MOV.U32 S1, 8
NOP; MOV.U32 S1, 9
NOP; MOV.U32 S1, 10
NOP; MOV.U32 S1, 11
NOP; MOV.U32 S1, 12
NOP; MOV.U32 S1, 13
NOP; MOV.U32 S1, 14
NOP; MOV.U32 S1, 15
NOP; MOV.U32 S1, 16

// CHECK: nop;    mov.u32 S1, 0x0, SP0;   nop;    nop
// CHECK: nop;    mov.u32 S1, 0x1, SP0;   nop;    nop
// CHECK: nop;    mov.u32 S1, 0x2, SP0;   nop;    nop
// CHECK: nop;    mov.u32 S1, 0x3, SP0;   nop;    nop
// CHECK: nop;    mov.u32 S1, 0x4, SP0;   nop;    nop
// CHECK: nop;    mov.u32 S1, 0x5, SP0;   nop;    nop
// CHECK: nop;    mov.u32 S1, 0x6, SP0;   nop;    nop
// CHECK: nop;    mov.u32 S1, 0x7, SP0;   nop;    nop
// CHECK: nop;    mov.u32 S1, 0x8, SP0;   nop;    nop
// CHECK: nop;    mov.u32 S1, 0x9, SP0;   nop;    nop
// CHECK: nop;    mov.u32 S1, 0xa, SP0;   nop;    nop
// CHECK: nop;    mov.u32 S1, 0xb, SP0;   nop;    nop
// CHECK: nop;    mov.u32 S1, 0xc, SP0;   nop;    nop
// CHECK: nop;    mov.u32 S1, 0xd, SP0;   nop;    nop
// CHECK: nop;    mov.u32 S1, 0xe, SP0;   nop;    nop
// CHECK: nop;    mov.u32 S1, 0xf, SP0;   nop;    nop
// CHECK: nop;    mov.u32 S1, 0x10, SP0;  nop;    nop


NOP; MOV SP1, S2
NOP; MOV SP1, S2, SP3
NOP; MOV SP1, S2, !SP3

// CHECK: nop;    mov SP1, S2, SP0;     nop;    nop
// CHECK: nop;    mov SP1, S2, SP3;     nop;    nop
// CHECK: nop;    mov SP1, S2, !SP3;    nop;    nop


NOP; MOV SP1, 123
NOP; MOV SP1, 123, SP3
NOP; MOV SP1, 123, !SP3

// CHECK: nop;    mov SP1, 0x7b, SP0;   nop;    nop
// CHECK: nop;    mov SP1, 0x7b, SP3;   nop;    nop
// CHECK: nop;    mov SP1, 0x7b, !SP3;  nop;    nop


NOP; MOV SP1, SP2
NOP; MOV SP1, SP2, SP3
NOP; MOV SP1, SP2, !SP3

// CHECK: nop;    mov SP1, SP2, SP0;    nop;    nop
// CHECK: nop;    mov SP1, SP2, SP3;    nop;    nop
// CHECK: nop;    mov SP1, SP2, !SP3;   nop;    nop


NOP; MOV.U32 SP1, SP2
NOP; MOV.U32 SP1, SP2, SP3
NOP; MOV.U32 SP1, SP2, !SP3

// CHECK: nop;    mov SP1, SP2, SP0;    nop;    nop
// CHECK: nop;    mov SP1, SP2, SP3;    nop;    nop
// CHECK: nop;    mov SP1, SP2, !SP3;   nop;    nop


NOP; MOV.B SP1, SP2
NOP; MOV.B SP1, SP2, SP3
NOP; MOV.B SP1, SP2, !SP3

// CHECK: nop;    mov SP1, SP2, SP0;    nop;    nop
// CHECK: nop;    mov SP1, SP2, SP3;    nop;    nop
// CHECK: nop;    mov SP1, SP2, !SP3;   nop;    nop


NOP; MOV b11111 I1, S2
NOP; MOV b00111 I1, S2, SP3
NOP; MOV b10000 I1, S2, !SP3

// CHECK: nop;    mov b11111 I1, S2, SP0;     nop;    nop
// CHECK: nop;    mov b00111 I1, S2, SP3;     nop;    nop
// CHECK: nop;    mov b10000 I1, S2, !SP3;    nop;    nop


NOP; MOV b11111 I1, 123
NOP; MOV b00111 I1, 123, SP3
NOP; MOV b10000 I1, 123, !SP3

// CHECK: nop;    mov b11111 I1, 0x7b, SP0;   nop;    nop
// CHECK: nop;    mov b00111 I1, 0x7b, SP3;   nop;    nop
// CHECK: nop;    mov b10000 I1, 0x7b, !SP3;  nop;    nop


NOP; MOV b11111 I1, I2
NOP; MOV b00111 I1, I2, SP3
NOP; MOV b10000 I1, I2, !SP3

// CHECK: nop;    mov b11111 I1, I2, SP0;     nop;    nop
// CHECK: nop;    mov b00111 I1, I2, SP3;     nop;    nop
// CHECK: nop;    mov b10000 I1, I2, !SP3;    nop;    nop

