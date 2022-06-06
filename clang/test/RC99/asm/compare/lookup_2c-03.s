// RUN: %clang -cc1as -triple tpc-none-none -target-cpu goya2 -filetype obj %s -o %t.gen3.o
// RUN: %disasm --mcpu=goya2 %t.gen3.o | FileCheck %s
// RUN: %clang -cc1as -triple tpc-none-none -filetype obj -target-cpu gaudi2 %s -o %t.gen4.o
// RUN: %disasm --mcpu gaudi2 %t.gen4.o | FileCheck %s

LOOKUP_2C D2, V2, BV32, 1; NOP; NOP; NOP
LOOKUP_2C D2, V2, BV32, 2, SP1; NOP; NOP; NOP
LOOKUP_2C D2, V2, BV32, 2, !SP1; NOP; NOP; NOP
//CHECK: lookup_2c BV32 D2, V2, 0x1;   nop;    nop;    nop
//CHECK: lookup_2c BV32 D2, V2, 0x2, SP1;   nop;    nop;    nop
//CHECK: lookup_2c BV32 D2, V2, 0x2, !SP1;  nop;    nop;    nop

LOOKUP_2C D2, V4, BV32, 3; NOP; NOP; NOP
LOOKUP_2C D2, V4, BV32, 4, SP1; NOP; NOP; NOP
LOOKUP_2C D2, V4, BV32, 4, !SP1; NOP; NOP; NOP
//CHECK: lookup_2c BV32 D2, V4, 0x3;   nop;    nop;    nop
//CHECK: lookup_2c BV32 D2, V4, 0x4, SP1;   nop;    nop;    nop
//CHECK: lookup_2c BV32 D2, V4, 0x4, !SP1;  nop;    nop;    nop

LOOKUP_2C UPPER_HALF D2, V2, BV32, 1; NOP; NOP; NOP
LOOKUP_2C UPPER_HALF D2, V2, BV32, 2, SP1; NOP; NOP; NOP
LOOKUP_2C UPPER_HALF D2, V2, BV32, 2, !SP1; NOP; NOP; NOP
//CHECK: lookup_2c BV32 upper_half D2, V2, 0x1;   nop;    nop;    nop
//CHECK: lookup_2c BV32 upper_half D2, V2, 0x2, SP1;   nop;    nop;    nop
//CHECK: lookup_2c BV32 upper_half D2, V2, 0x2, !SP1;  nop;    nop;    nop

LOOKUP_2C UPPER_HALF D2, V4, BV32, 3; NOP; NOP; NOP
LOOKUP_2C UPPER_HALF D2, V4, BV32, 4, SP1; NOP; NOP; NOP
LOOKUP_2C UPPER_HALF D2, V4, BV32, 4, !SP1; NOP; NOP; NOP
//CHECK: lookup_2c BV32 upper_half D2, V4, 0x3;   nop;    nop;    nop
//CHECK: lookup_2c BV32 upper_half D2, V4, 0x4, SP1;   nop;    nop;    nop
//CHECK: lookup_2c BV32 upper_half D2, V4, 0x4, !SP1;  nop;    nop;    nop

LOOKUP_2C LUT_PTR D2, V2, S4, BV32; NOP; NOP; NOP
LOOKUP_2C LUT_PTR D2, V2, S4, BV32, SP1; NOP; NOP; NOP
LOOKUP_2C LUT_PTR D2, V2, S4, BV32, !SP1; NOP; NOP; NOP
//CHECK: lookup_2c BV32 lut_ptr D2, V2, S4;    nop;    nop;    nop
//CHECK: lookup_2c BV32 lut_ptr D2, V2, S4, SP1;    nop;    nop;    nop
//CHECK: lookup_2c BV32 lut_ptr D2, V2, S4, !SP1;   nop;    nop;    nop

LOOKUP_2C D2, V2, S4, BV16; NOP; NOP; NOP
LOOKUP_2C D2, V2, S4, BV16, SP1; NOP; NOP; NOP
LOOKUP_2C D2, V2, S4, BV16, !SP1; NOP; NOP; NOP
//CHECK: lookup_2c BV16 lut_ptr D2, V2, S4;   nop;    nop;    nop
//CHECK: lookup_2c BV16 lut_ptr D2, V2, S4, SP1;   nop;    nop;    nop
//CHECK: lookup_2c BV16 lut_ptr D2, V2, S4, !SP1;  nop;    nop;    nop

LOOKUP_2C D2, V2, S4, BV8_0; NOP; NOP; NOP
LOOKUP_2C D2, V2, S4, BV8_0, SP1; NOP; NOP; NOP
LOOKUP_2C D2, V2, S4, BV8_0, !SP1; NOP; NOP; NOP
//CHECK: lookup_2c BV8_0 lut_ptr D2, V2, S4;  nop;    nop;    nop
//CHECK: lookup_2c BV8_0 lut_ptr D2, V2, S4, SP1;  nop;    nop;    nop
//CHECK: lookup_2c BV8_0 lut_ptr D2, V2, S4, !SP1;         nop;    nop;    nop

LOOKUP_2C D2, V2, S4, BV8_1; NOP; NOP; NOP
LOOKUP_2C D2, V2, S4, BV8_1, SP1; NOP; NOP; NOP
LOOKUP_2C D2, V2, S4, BV8_1, !SP1; NOP; NOP; NOP
//CHECK: lookup_2c BV8_1 lut_ptr D2, V2, S4;  nop;    nop;    nop
//CHECK: lookup_2c BV8_1 lut_ptr D2, V2, S4, SP1;  nop;    nop;    nop
//CHECK: lookup_2c BV8_1 lut_ptr D2, V2, S4, !SP1;         nop;    nop;    nop

LOOKUP_2C LUT_PTR D2, V2, S4, BV8_1; NOP; NOP; NOP
LOOKUP_2C LUT_PTR D2, V2, S4, BV8_1, SP1; NOP; NOP; NOP
LOOKUP_2C LUT_PTR D2, V2, S4, BV8_1, !SP1; NOP; NOP; NOP
//CHECK: lookup_2c BV8_1 lut_ptr D2, V2, S4;  nop;    nop;    nop
//CHECK: lookup_2c BV8_1 lut_ptr D2, V2, S4, SP1;  nop;    nop;    nop
//CHECK: lookup_2c BV8_1 lut_ptr D2, V2, S4, !SP1;         nop;    nop;    nop
