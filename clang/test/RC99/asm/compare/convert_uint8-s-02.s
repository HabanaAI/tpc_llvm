// RUN: %clang -cc1as -triple tpc-none-none -filetype obj -target-cpu goya2 %s -o %t.o
// RUN: %disasm --mcpu goya2 %t.o | FileCheck %s


NOP; CONVERT_UINT8 S6, S12, S8, RNE;       NOP; NOP
NOP; CONVERT_UINT8 S6, S12, S8, RNE, SP1;  NOP; NOP
NOP; CONVERT_UINT8 S6, S12, S8, RNE, !SP1; NOP; NOP

// CHECK: nop;    convert_uint8 rhne S6, S12, S8;    nop;    nop
// CHECK: nop;    convert_uint8 rhne S6, S12, S8, SP1;    nop;    nop
// CHECK: nop;    convert_uint8 rhne S6, S12, S8, !SP1;   nop;    nop

NOP; CONVERT_UINT8 S6, S12, S8;       NOP; NOP
NOP; CONVERT_UINT8 S6, S12, S8, SP1;  NOP; NOP
NOP; CONVERT_UINT8 S6, S12, S8, !SP1; NOP; NOP

// CHECK: nop;    convert_uint8 rhne S6, S12, S8;    nop;    nop
// CHECK: nop;    convert_uint8 rhne S6, S12, S8, SP1;    nop;    nop
// CHECK: nop;    convert_uint8 rhne S6, S12, S8, !SP1;   nop;    nop


NOP; CONVERT_UINT8 S6, S12, S11, RNE;  NOP; NOP
NOP; CONVERT_UINT8 S6, S12, S11, RHNE; NOP; NOP
NOP; CONVERT_UINT8 S6, S12, S11, RZ;   NOP; NOP
NOP; CONVERT_UINT8 S6, S12, S11, RU;   NOP; NOP
NOP; CONVERT_UINT8 S6, S12, S11, RD;   NOP; NOP
NOP; CONVERT_UINT8 S6, S12, S11, SR;   NOP; NOP

// CHECK: nop;    convert_uint8 rhne S6, S12, S11;   nop;    nop
// CHECK: nop;    convert_uint8 rhne S6, S12, S11;   nop;    nop
// CHECK: nop;    convert_uint8 rz   S6, S12, S11;  nop;    nop
// CHECK: nop;    convert_uint8 ru   S6, S12, S11;  nop;    nop
// CHECK: nop;    convert_uint8 rd   S6, S12, S11;  nop;    nop
// CHECK: nop;    convert_uint8 sr   S6, S12, S11;  nop;    nop


NOP; CONVERT_UINT8 S6, S12, 6, RNE;      NOP; NOP
NOP; CONVERT_UINT8 S6, S12, 6, RD, SP1;  NOP; NOP
NOP; CONVERT_UINT8 S6, S12, 6, RU, !SP1; NOP; NOP

// CHECK: nop;    convert_uint8 rhne S6, S12, 0x6;   nop;    nop
// CHECK: nop;    convert_uint8 rd   S6, S12, 0x6, SP1;   nop;    nop
// CHECK: nop;    convert_uint8 ru   S6, S12, 0x6, !SP1;  nop;    nop
