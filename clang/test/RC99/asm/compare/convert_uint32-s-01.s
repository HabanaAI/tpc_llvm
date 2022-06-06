// RUN: %clang -cc1as -triple tpc-none-none -target-cpu dali -filetype obj %s -o %t.o
// RUN: %disasm %t.o | FileCheck %s


NOP; CONVERT_UINT32 S6, S12, S8, RNE;       NOP; NOP
NOP; CONVERT_UINT32 S6, S12, S8, RNE, SP1;  NOP; NOP
NOP; CONVERT_UINT32 S6, S12, S8, RNE, !SP1; NOP; NOP

// CHECK: nop;    convert_uint32 rhne to_8 S6, S12, S8;    nop;    nop
// CHECK: nop;    convert_uint32 rhne to_8 S6, S12, S8, SP1;    nop;    nop
// CHECK: nop;    convert_uint32 rhne to_8 S6, S12, S8, !SP1;   nop;    nop

NOP; CONVERT_UINT32 S6, S12, S8;       NOP; NOP
NOP; CONVERT_UINT32 S6, S12, S8, SP1;  NOP; NOP
NOP; CONVERT_UINT32 S6, S12, S8, !SP1; NOP; NOP

// CHECK: nop;    convert_uint32 rhne to_8 S6, S12, S8;    nop;    nop
// CHECK: nop;    convert_uint32 rhne to_8 S6, S12, S8, SP1;    nop;    nop
// CHECK: nop;    convert_uint32 rhne to_8 S6, S12, S8, !SP1;   nop;    nop

NOP; CONVERT_UINT32 S6, S12, S8, RHNE; NOP; NOP
NOP; CONVERT_UINT32 S6, S12, S8, RNE;  NOP; NOP
NOP; CONVERT_UINT32 S6, S12, S8, RU;   NOP; NOP
NOP; CONVERT_UINT32 S6, S12, S8, RD;   NOP; NOP
NOP; CONVERT_UINT32 S6, S12, S8, SR;   NOP; NOP

// CHECK: nop;    convert_uint32 rhne to_8 S6, S12, S8;    nop;    nop
// CHECK: nop;    convert_uint32 rhne to_8 S6, S12, S8;    nop;    nop
// CHECK: nop;    convert_uint32 ru to_8   S6, S12, S8;    nop;    nop
// CHECK: nop;    convert_uint32 rd to_8   S6, S12, S8;    nop;    nop
// CHECK: nop;    convert_uint32 sr to_8   S6, S12, S8;    nop;    nop


NOP; CONVERT_UINT32 S6, S12, 123, RNE;      NOP; NOP
NOP; CONVERT_UINT32 S6, S12, 123, RD, SP1;  NOP; NOP
NOP; CONVERT_UINT32 S6, S12, 123, RU, !SP1; NOP; NOP

// CHECK: nop;    convert_uint32 rhne to_8 S6, S12, 0x7b;    nop;    nop
// CHECK: nop;    convert_uint32 rd to_8 S6, S12, 0x7b, SP1;    nop;    nop
// CHECK: nop;    convert_uint32 ru to_8 S6, S12, 0x7b, !SP1;   nop;    nop


NOP; CONVERT_UINT32 S6, S12, S8, TO_16;       NOP; NOP
NOP; CONVERT_UINT32 S6, S12, S8, TO_16, SP1;  NOP; NOP
NOP; CONVERT_UINT32 S6, S12, S8, TO_16, !SP1; NOP; NOP

// CHECK: nop;    convert_uint32 rhne to_16 S6, S12, S8;      nop;    nop
// CHECK: nop;    convert_uint32 rhne to_16 S6, S12, S8, SP1;      nop;    nop
// CHECK: nop;    convert_uint32 rhne to_16 S6, S12, S8, !SP1;     nop;    nop

NOP; CONVERT_UINT32 S6, S12, S8, TO_16, RHNE;     NOP; NOP
NOP; CONVERT_UINT32 S6, S12, S8, TO_16, RD, SP1;  NOP; NOP
NOP; CONVERT_UINT32 S6, S12, S8, TO_16, SR, !SP1; NOP; NOP

// CHECK: nop;    convert_uint32 rhne to_16 S6, S12, S8;      nop;    nop
// CHECK: nop;    convert_uint32  rd to_16 S6, S12, S8, SP1;       nop;    nop
// CHECK: nop;    convert_uint32  sr to_16 S6, S12, S8, !SP1;      nop;    nop

NOP; CONVERT_UINT32 S6, S12, 123, TO_16, RHNE;     NOP; NOP
NOP; CONVERT_UINT32 S6, S12, 123, TO_16, RD, SP1;  NOP; NOP
NOP; CONVERT_UINT32 S6, S12, 123, TO_16, SR, !SP1; NOP; NOP

// CHECK: nop;    convert_uint32 rhne to_16 S6, S12, 0x7b;    nop;    nop
// CHECK: nop;    convert_uint32  rd to_16 S6, S12, 0x7b, SP1;     nop;    nop
// CHECK: nop;    convert_uint32  sr to_16 S6, S12, 0x7b, !SP1;    nop;    nop


NOP; CONVERT_UINT32 S6, S12, S8, TO_8;       NOP; NOP
NOP; CONVERT_UINT32 S6, S12, S8, TO_8, SP1;  NOP; NOP
NOP; CONVERT_UINT32 S6, S12, S8, TO_8, !SP1; NOP; NOP

// CHECK: nop;    convert_uint32 rhne to_8 S6, S12, S8;    nop;    nop
// CHECK: nop;    convert_uint32 rhne to_8 S6, S12, S8, SP1;    nop;    nop
// CHECK: nop;    convert_uint32 rhne to_8 S6, S12, S8, !SP1;   nop;    nop

NOP; CONVERT_UINT32 S6, S12, S8, TO_8, RHNE;     NOP; NOP
NOP; CONVERT_UINT32 S6, S12, S8, TO_8, RD, SP1;  NOP; NOP
NOP; CONVERT_UINT32 S6, S12, S8, TO_8, SR, !SP1; NOP; NOP

// CHECK: nop;    convert_uint32 rhne to_8 S6, S12, S8;    nop;    nop
// CHECK: nop;    convert_uint32  rd to_8 S6, S12, S8, SP1;     nop;    nop
// CHECK: nop;    convert_uint32  sr to_8 S6, S12, S8, !SP1;    nop;    nop

NOP; CONVERT_UINT32 S6, S12, 123, TO_8, RHNE;     NOP; NOP
NOP; CONVERT_UINT32 S6, S12, 123, TO_8, RD, SP1;  NOP; NOP
NOP; CONVERT_UINT32 S6, S12, 123, TO_8, SR, !SP1; NOP; NOP

// CHECK: nop;    convert_uint32 rhne to_8 S6, S12, 0x7b;  nop;    nop
// CHECK: nop;    convert_uint32  rd to_8 S6, S12, 0x7b, SP1;   nop;    nop
// CHECK: nop;    convert_uint32  sr to_8 S6, S12, 0x7b, !SP1;  nop;    nop
