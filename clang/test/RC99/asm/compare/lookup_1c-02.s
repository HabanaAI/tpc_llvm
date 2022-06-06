// RUN: %clang -cc1as -triple tpc-none-none -target-cpu gaudi -filetype obj %s -o %t.o
// RUN: %disasm --mcpu=gaudi %t.o | FileCheck %s

LOOKUP_1C V1, V2, BV32, 1; NOP; NOP; NOP
LOOKUP_1C V1, V2, BV32, 2, SP1; NOP; NOP; NOP
LOOKUP_1C V1, V2, BV32, 2, !SP1; NOP; NOP; NOP
//CHECK: lookup_1c  BV32 V1, V2, 0x1;   nop;    nop;    nop
//CHECK: lookup_1c  BV32 V1, V2, 0x2, SP1;   nop;    nop;    nop
//CHECK: lookup_1c  BV32 V1, V2, 0x2, !SP1;  nop;    nop;    nop

LOOKUP_1C V3, V4, BV16, 3; NOP; NOP; NOP
LOOKUP_1C V3, V4, BV16, 4, SP1; NOP; NOP; NOP
LOOKUP_1C V3, V4, BV16, 4, !SP1; NOP; NOP; NOP
//CHECK: lookup_1c  BV16 V3, V4, 0x3;  nop;    nop;    nop
//CHECK: lookup_1c  BV16 V3, V4, 0x4, SP1;  nop;    nop;    nop
//CHECK: lookup_1c  BV16 V3, V4, 0x4, !SP1;         nop;    nop;    nop

LOOKUP_1C UPPER_HALF V1, V2, BV32, 1; NOP; NOP; NOP
LOOKUP_1C UPPER_HALF V1, V2, BV32, 2, SP1; NOP; NOP; NOP
LOOKUP_1C UPPER_HALF V1, V2, BV32, 2, !SP1; NOP; NOP; NOP
//CHECK: lookup_1c BV32 upper_half V1, V2, 0x1;   nop;    nop;    nop
//CHECK: lookup_1c BV32 upper_half V1, V2, 0x2, SP1;   nop;    nop;    nop
//CHECK: lookup_1c BV32 upper_half V1, V2, 0x2, !SP1;  nop;    nop;    nop

LOOKUP_1C UPPER_HALF V3, V4, BV16, 3; NOP; NOP; NOP
LOOKUP_1C UPPER_HALF V3, V4, BV16, 4, SP1; NOP; NOP; NOP
LOOKUP_1C UPPER_HALF V3, V4, BV16, 4, !SP1; NOP; NOP; NOP
//CHECK: lookup_1c BV16 upper_half V3, V4, 0x3;  nop;    nop;    nop
//CHECK: lookup_1c BV16 upper_half V3, V4, 0x4, SP1;  nop;    nop;    nop
//CHECK: lookup_1c BV16 upper_half V3, V4, 0x4, !SP1;         nop;    nop;    nop