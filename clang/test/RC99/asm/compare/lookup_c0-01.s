// RUN: %clang -cc1as -triple tpc-none-none -filetype obj %s -o %t.o
// RUN: %disasm %t.o | FileCheck %s

LOOKUP_C0 V1, V2, BV32, 1; NOP; NOP; NOP
LOOKUP_C0 V1, V2, BV32, 1, SP3; NOP; NOP; NOP
LOOKUP_C0 V1, V2, BV32, 1, !SP3; NOP; NOP; NOP
//CHECK: lookup_c0   BV32 V1, V2, 0x1;   nop;    nop;    nop
//CHECK: lookup_c0   BV32 V1, V2, 0x1, SP3;   nop;    nop;    nop
//CHECK: lookup_c0   BV32 V1, V2, 0x1, !SP3;  nop;    nop;    nop

LOOKUP_C0 V1, V2, BV16_HIGH, 1; NOP; NOP; NOP
LOOKUP_C0 V1, V2, BV16_HIGH, 1, SP3; NOP; NOP; NOP
LOOKUP_C0 V1, V2, BV16_HIGH, 1, !SP3; NOP; NOP; NOP
//CHECK: lookup_c0    BV16_HIGH V1, V2, 0x1;     nop;    nop;    nop
//CHECK: lookup_c0    BV16_HIGH V1, V2, 0x1, SP3;     nop;    nop;    nop
//CHECK: lookup_c0    BV16_HIGH V1, V2, 0x1, !SP3;    nop;    nop;    nop

LOOKUP_C0 V1, V2, BV16_LOW, 1; NOP; NOP; NOP
LOOKUP_C0 V1, V2, BV16_LOW, 1, SP3; NOP; NOP; NOP
LOOKUP_C0 V1, V2, BV16_LOW, 1, !SP3; NOP; NOP; NOP
//CHECK: lookup_c0    BV16_LOW V1, V2, 0x1;      nop;    nop;    nop
//CHECK: lookup_c0    BV16_LOW V1, V2, 0x1, SP3;      nop;    nop;    nop
//CHECK: lookup_c0    BV16_LOW V1, V2, 0x1, !SP3;     nop;    nop;    nop

LOOKUP_C0 V1, V2, BV8_ELEMENT_0, 1; NOP; NOP; NOP
LOOKUP_C0 V1, V2, BV8_ELEMENT_0, 1, SP3; NOP; NOP; NOP
LOOKUP_C0 V1, V2, BV8_ELEMENT_0, 1, !SP3; NOP; NOP; NOP
//CHECK: lookup_c0    BV8_0 V1, V2, 0x1;         nop;    nop;    nop
//CHECK: lookup_c0    BV8_0 V1, V2, 0x1, SP3;         nop;    nop;    nop
//CHECK: lookup_c0    BV8_0 V1, V2, 0x1, !SP3;        nop;    nop;    nop

LOOKUP_C0 V1, V2, BV8_ELEMENT_1, 1; NOP; NOP; NOP
LOOKUP_C0 V1, V2, BV8_ELEMENT_1, 1, SP3; NOP; NOP; NOP
LOOKUP_C0 V1, V2, BV8_ELEMENT_1, 1, !SP3; NOP; NOP; NOP
//CHECK: lookup_c0    BV8_1 V1, V2, 0x1;         nop;    nop;    nop
//CHECK: lookup_c0    BV8_1 V1, V2, 0x1, SP3;         nop;    nop;    nop
//CHECK: lookup_c0    BV8_1 V1, V2, 0x1, !SP3;        nop;    nop;    nop

LOOKUP_C0 V1, V2, BV8_ELEMENT_2, 1; NOP; NOP; NOP
LOOKUP_C0 V1, V2, BV8_ELEMENT_2, 1, SP3; NOP; NOP; NOP
LOOKUP_C0 V1, V2, BV8_ELEMENT_2, 1, !SP3; NOP; NOP; NOP
//CHECK: lookup_c0    BV8_2 V1, V2, 0x1;         nop;    nop;    nop
//CHECK: lookup_c0    BV8_2 V1, V2, 0x1, SP3;         nop;    nop;    nop
//CHECK: lookup_c0    BV8_2 V1, V2, 0x1, !SP3;        nop;    nop;    nop

LOOKUP_C0 V1, V2, BV8_ELEMENT_3, 1; NOP; NOP; NOP
LOOKUP_C0 V1, V2, BV8_ELEMENT_3, 1, SP3; NOP; NOP; NOP
LOOKUP_C0 V1, V2, BV8_ELEMENT_3, 1, !SP3; NOP; NOP; NOP
//CHECK: lookup_c0    BV8_3 V1, V2, 0x1;         nop;    nop;    nop
//CHECK: lookup_c0    BV8_3 V1, V2, 0x1, SP3;         nop;    nop;    nop
//CHECK: lookup_c0    BV8_3 V1, V2, 0x1, !SP3;        nop;    nop;    nop

LOOKUP_C0 V0, V39, BV32, 1, SP1; NOP; NOP; NOP
LOOKUP_C0 V1, V38, BV32, 2, SP1; NOP; NOP; NOP
LOOKUP_C0 V2, V37, BV32, 3, SP1; NOP; NOP; NOP
LOOKUP_C0 V3, V36, BV32, 4, SP1; NOP; NOP; NOP
//CHECK: lookup_c0   BV32 V0, V39, 0x1, SP1;  nop;    nop;    nop
//CHECK: lookup_c0   BV32 V1, V38, 0x2, SP1;  nop;    nop;    nop
//CHECK: lookup_c0   BV32 V2, V37, 0x3, SP1;  nop;    nop;    nop
//CHECK: lookup_c0   BV32 V3, V36, 0x4, SP1;  nop;    nop;    nop

LOOKUP_C0 V36, V3, BV32, 5, SP1; NOP; NOP; NOP
LOOKUP_C0 V37, V2, BV32, 6, SP1; NOP; NOP; NOP
LOOKUP_C0 V38, V1, BV32, 7, SP1; NOP; NOP; NOP
LOOKUP_C0 V39, V0, BV32, 8, SP1; NOP; NOP; NOP
//CHECK: lookup_c0   BV32 V36, V3, 0x5, SP1;  nop;    nop;    nop
//CHECK: lookup_c0   BV32 V37, V2, 0x6, SP1;  nop;    nop;    nop
//CHECK: lookup_c0   BV32 V38, V1, 0x7, SP1;  nop;    nop;    nop
//CHECK: lookup_c0   BV32 V39, V0, 0x8, SP1;  nop;    nop;    nop