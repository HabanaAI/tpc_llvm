// RUN: %clang -cc1as -triple tpc-none-none -target-cpu gaudi -filetype obj %s -o %t.o
// RUN: %disasm --mcpu=gaudi %t.o | FileCheck %s

LOOKUP V1, V2, BV32, 1; NOP; NOP; NOP
LOOKUP V1, V2, BV32, 2, SP1;  NOP; NOP; NOP
LOOKUP V1, V2, BV32, 2, !SP1; NOP; NOP; NOP
//CHECK: lookup BV32 V1, V2, 0x1;   nop;    nop;    nop
//CHECK: lookup BV32 V1, V2, 0x2, SP1;   nop;    nop;    nop
//CHECK: lookup BV32 V1, V2, 0x2, !SP1;  nop;    nop;    nop

LOOKUP UPPER_HALF V1, V2, BV32, 1; NOP; NOP; NOP
LOOKUP UPPER_HALF V1, V2, BV32, 2, SP1;  NOP; NOP; NOP
LOOKUP UPPER_HALF V1, V2, BV32, 2, !SP1; NOP; NOP; NOP
//CHECK: lookup BV32 upper_half V1, V2, 0x1;        nop;    nop;    nop
//CHECK: lookup BV32 upper_half V1, V2, 0x2, SP1;        nop;    nop;    nop
//CHECK: lookup BV32 upper_half V1, V2, 0x2, !SP1;       nop;    nop;    nop