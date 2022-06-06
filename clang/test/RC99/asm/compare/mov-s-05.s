// RUN: %clang -cc1as -triple tpc-none-none -filetype obj -target-cpu doron1 %s -o %t.o
// RUN: %disasm --mcpu doron1 %t.o | FileCheck %s

NOP; MOV M1 I5, S3
NOP; MOV M1 I5, S3, SP1
NOP; MOV M1 I5, S3, !SP1

// CHECK: nop;    mov M1 I5, S3;     nop;    nop
// CHECK: nop;    mov M1 I5, S3, SP1;     nop;    nop
// CHECK: nop;    mov M1 I5, S3, !SP1;    nop;    nop


NOP; MOV M1 I5, 123
NOP; MOV M1 I5, 123, SP1
NOP; MOV M1 I5, 123, !SP1

// CHECK: nop;    mov M1 I5, 0x7b;   nop;    nop
// CHECK: nop;    mov M1 I5, 0x7b, SP1;   nop;    nop
// CHECK: nop;    mov M1 I5, 0x7b, !SP1;  nop;    nop


NOP; MOV M1 I5, 0
NOP; MOV M1 I5, 1
NOP; MOV M1 I5, 2
NOP; MOV M1 I5, 3
NOP; MOV M1 I5, 4
NOP; MOV M1 I5, 5
NOP; MOV M1 I5, 6
NOP; MOV M1 I5, 7
NOP; MOV M1 I5, 8
NOP; MOV M1 I5, 9
NOP; MOV M1 I5, 10
NOP; MOV M1 I5, 11
NOP; MOV M1 I5, 12
NOP; MOV M1 I5, 13
NOP; MOV M1 I5, 14
NOP; MOV M1 I5, 15
NOP; MOV M1 I5, 16

// CHECK: nop;    mov M1 I5, 0x0;    nop;    nop
// CHECK: nop;    mov M1 I5, 0x1;    nop;    nop
// CHECK: nop;    mov M1 I5, 0x2;    nop;    nop
// CHECK: nop;    mov M1 I5, 0x3;    nop;    nop
// CHECK: nop;    mov M1 I5, 0x4;    nop;    nop
// CHECK: nop;    mov M1 I5, 0x5;    nop;    nop
// CHECK: nop;    mov M1 I5, 0x6;    nop;    nop
// CHECK: nop;    mov M1 I5, 0x7;    nop;    nop
// CHECK: nop;    mov M1 I5, 0x8;    nop;    nop
// CHECK: nop;    mov M1 I5, 0x9;    nop;    nop
// CHECK: nop;    mov M1 I5, 0xa;    nop;    nop
// CHECK: nop;    mov M1 I5, 0xb;    nop;    nop
// CHECK: nop;    mov M1 I5, 0xc;    nop;    nop
// CHECK: nop;    mov M1 I5, 0xd;    nop;    nop
// CHECK: nop;    mov M1 I5, 0xe;    nop;    nop
// CHECK: nop;    mov M1 I5, 0xf;    nop;    nop
// CHECK: nop;    mov M1 I5, 0x10;   nop;    nop


NOP; MOV M1 I5, I3
NOP; MOV M1 I5, I3, SP1
NOP; MOV M1 I5, I3, !SP1

// CHECK: nop;    mov M1 I5, I3;     nop;    nop
// CHECK: nop;    mov M1 I5, I3, SP1;     nop;    nop
// CHECK: nop;    mov M1 I5, I3, !SP1;    nop;    nop


NOP; MOV AD1, AD2; NOP; NOP
NOP; MOV AD1, AD2, SP2; NOP; NOP
NOP; MOV AD1, AD2, !SP2; NOP; NOP

// CHECK: nop;    mov AD1, AD2;      nop;    nop
// CHECK: nop;    mov AD1, AD2, SP2;      nop;    nop
// CHECK: nop;    mov AD1, AD2, !SP2;     nop;    nop


NOP; MOV AD1, Z0
NOP; MOV AD2, Z2, SP2
NOP; MOV AD2, Z4, !SP2

// CHECK: nop;    mov AD1, Z0;       nop;    nop
// CHECK: nop;    mov AD2, Z2, SP2;       nop;    nop
// CHECK: nop;    mov AD2, Z4, !SP2;      nop;    nop


NOP; MOV Z2, AD1
NOP; MOV Z0, AD1, SP2
NOP; MOV Z2, AD2, !SP2

// CHECK: nop;    mov Z2, AD1;       nop;    nop
// CHECK: nop;    mov Z0, AD1, SP2;       nop;    nop
// CHECK: nop;    mov Z2, AD2, !SP2;      nop;    nop
