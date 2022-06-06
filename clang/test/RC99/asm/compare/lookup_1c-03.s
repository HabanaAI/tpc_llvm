// RUN: %clang -cc1as -triple tpc-none-none -target-cpu goya2 -filetype obj %s -o %t.gen3.o
// RUN: %disasm --mcpu=goya2 %t.gen3.o | FileCheck %s
// RUN: %clang -cc1as -triple tpc-none-none -filetype obj -target-cpu gaudi2 %s -o %t.gen4.o
// RUN: %disasm --mcpu gaudi2 %t.gen4.o | FileCheck %s

LOOKUP_1C V1, V2, BV32, 1; NOP; NOP; NOP
LOOKUP_1C V1, V2, BV32, 2, SP1; NOP; NOP; NOP
LOOKUP_1C V1, V2, BV32, 2, !SP1; NOP; NOP; NOP
//CHECK: lookup_1c BV32 V1, V2, 0x1;   nop;    nop;    nop
//CHECK: lookup_1c BV32 V1, V2, 0x2, SP1;   nop;    nop;    nop
//CHECK: lookup_1c BV32 V1, V2, 0x2, !SP1;  nop;    nop;    nop

LOOKUP_1C V3, V4, BV32, 3; NOP; NOP; NOP
LOOKUP_1C V3, V4, BV32, 4, SP1; NOP; NOP; NOP
LOOKUP_1C V3, V4, BV32, 4, !SP1; NOP; NOP; NOP
//CHECK: lookup_1c BV32 V3, V4, 0x3;   nop;    nop;    nop
//CHECK: lookup_1c BV32 V3, V4, 0x4, SP1;   nop;    nop;    nop
//CHECK: lookup_1c BV32 V3, V4, 0x4, !SP1;  nop;    nop;    nop

LOOKUP_1C UPPER_HALF V1, V2, BV32, 1; NOP; NOP; NOP
LOOKUP_1C UPPER_HALF V1, V2, BV32, 2, SP1; NOP; NOP; NOP
LOOKUP_1C UPPER_HALF V1, V2, BV32, 2, !SP1; NOP; NOP; NOP
//CHECK: lookup_1c BV32 upper_half V1, V2, 0x1;   nop;    nop;    nop
//CHECK: lookup_1c BV32 upper_half V1, V2, 0x2, SP1;   nop;    nop;    nop
//CHECK: lookup_1c BV32 upper_half V1, V2, 0x2, !SP1;  nop;    nop;    nop

LOOKUP_1C UPPER_HALF V3, V4, BV32, 3; NOP; NOP; NOP
LOOKUP_1C UPPER_HALF V3, V4, BV32, 4, SP1; NOP; NOP; NOP
LOOKUP_1C UPPER_HALF V3, V4, BV32, 4, !SP1; NOP; NOP; NOP
//CHECK: lookup_1c BV32 upper_half V3, V4, 0x3;   nop;    nop;    nop
//CHECK: lookup_1c BV32 upper_half V3, V4, 0x4, SP1;   nop;    nop;    nop
//CHECK: lookup_1c BV32 upper_half V3, V4, 0x4, !SP1;  nop;    nop;    nop

LOOKUP_1C LUT_PTR V1, V2, S2, BV32; NOP; NOP; NOP
LOOKUP_1C LUT_PTR V1, V2, S2, BV32, SP1; NOP; NOP; NOP
LOOKUP_1C LUT_PTR V1, V2, S2, BV32, !SP1; NOP; NOP; NOP
//CHECK: lookup_1c BV32 lut_ptr V1, V2, S2;    nop;    nop;    nop
//CHECK: lookup_1c BV32 lut_ptr V1, V2, S2, SP1;    nop;    nop;    nop
//CHECK: lookup_1c BV32 lut_ptr V1, V2, S2, !SP1;   nop;    nop;    nop

LOOKUP_1C LUT_PTR V1, V2, S2, BV32; NOP; NOP; NOP
LOOKUP_1C LUT_PTR V1, V2, S2, BV32, SP1; NOP; NOP; NOP
LOOKUP_1C LUT_PTR V1, V2, S2, BV32, !SP1; NOP; NOP; NOP
//CHECK: lookup_1c BV32 lut_ptr V1, V2, S2;    nop;    nop;    nop
//CHECK: lookup_1c BV32 lut_ptr V1, V2, S2, SP1;    nop;    nop;    nop
//CHECK: lookup_1c BV32 lut_ptr V1, V2, S2, !SP1;   nop;    nop;    nop
