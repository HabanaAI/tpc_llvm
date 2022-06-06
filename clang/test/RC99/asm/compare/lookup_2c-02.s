// RUN: %clang -cc1as -triple tpc-none-none -target-cpu gaudi -filetype obj %s -o %t.o
// RUN: %disasm --mcpu=gaudi %t.o | FileCheck %s

LOOKUP_2C D2, V2, BV32, 1; NOP; NOP; NOP
LOOKUP_2C D2, V2, BV32, 2, SP1; NOP; NOP; NOP
LOOKUP_2C D2, V2, BV32, 2, !SP1; NOP; NOP; NOP
//CHECK: lookup_2c   BV32 D2, V2, 0x1;   nop;    nop;    nop
//CHECK: lookup_2c   BV32 D2, V2, 0x2, SP1;   nop;    nop;    nop
//CHECK: lookup_2c   BV32 D2, V2, 0x2, !SP1;  nop;    nop;    nop

LOOKUP_2C D2, V4, BV16, 3; NOP; NOP; NOP
LOOKUP_2C D2, V4, BV16, 4, SP1; NOP; NOP; NOP
LOOKUP_2C D2, V4, BV16, 4, !SP1; NOP; NOP; NOP
//CHECK: lookup_2c BV16 D2, V4, 0x3;  nop;    nop;    nop
//CHECK: lookup_2c BV16 D2, V4, 0x4, SP1;  nop;    nop;    nop
//CHECK: lookup_2c BV16 D2, V4, 0x4, !SP1;         nop;    nop;    nop

LOOKUP_2C UPPER_HALF D2, V2, BV32, 1; NOP; NOP; NOP
LOOKUP_2C UPPER_HALF D2, V2, BV32, 2, SP1; NOP; NOP; NOP
LOOKUP_2C UPPER_HALF D2, V2, BV32, 2, !SP1; NOP; NOP; NOP
//CHECK: lookup_2c BV32 upper_half D2, V2, 0x1;   nop;    nop;    nop
//CHECK: lookup_2c BV32 upper_half D2, V2, 0x2, SP1;   nop;    nop;    nop
//CHECK: lookup_2c BV32 upper_half D2, V2, 0x2, !SP1;  nop;    nop;    nop

LOOKUP_2C UPPER_HALF D2, V4, BV16, 3; NOP; NOP; NOP
LOOKUP_2C UPPER_HALF D2, V4, BV16, 4, SP1; NOP; NOP; NOP
LOOKUP_2C UPPER_HALF D2, V4, BV16, 4, !SP1; NOP; NOP; NOP
//CHECK: lookup_2c BV16 upper_half D2, V4, 0x3;  nop;    nop;    nop
//CHECK: lookup_2c BV16 upper_half D2, V4, 0x4, SP1;  nop;    nop;    nop
//CHECK: lookup_2c BV16 upper_half D2, V4, 0x4, !SP1;         nop;    nop;    nop
