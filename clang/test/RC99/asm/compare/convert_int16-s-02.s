// RUN: %clang -cc1as -triple tpc-none-none -filetype obj -target-cpu gaudi %s -o %t.o
// RUN: %disasm -mcpu gaudi %t.o | FileCheck %s


NOP; CONVERT_INT16 S6, S12, S8, RNE;       NOP; NOP
NOP; CONVERT_INT16 S6, S12, S8, RNE, SP1;  NOP; NOP
NOP; CONVERT_INT16 S6, S12, S8, RNE, !SP1; NOP; NOP

// CHECK: nop;    convert_int16 rhne S6, S12, S8, SP0;    nop;    nop
// CHECK: nop;    convert_int16 rhne S6, S12, S8, SP1;    nop;    nop
// CHECK: nop;    convert_int16 rhne S6, S12, S8, !SP1;   nop;    nop

NOP; CONVERT_INT16 S6, S12, S8;       NOP; NOP
NOP; CONVERT_INT16 S6, S12, S8, SP1;  NOP; NOP
NOP; CONVERT_INT16 S6, S12, S8, !SP1; NOP; NOP

// CHECK: nop;    convert_int16 rhne S6, S12, S8, SP0;    nop;    nop
// CHECK: nop;    convert_int16 rhne S6, S12, S8, SP1;    nop;    nop
// CHECK: nop;    convert_int16 rhne S6, S12, S8, !SP1;   nop;    nop


NOP; CONVERT_INT16 S6, S12, S11, RNE;  NOP; NOP
NOP; CONVERT_INT16 S6, S12, S11, RHNE; NOP; NOP
NOP; CONVERT_INT16 S6, S12, S11, RZ;   NOP; NOP
NOP; CONVERT_INT16 S6, S12, S11, RU;   NOP; NOP
NOP; CONVERT_INT16 S6, S12, S11, RD;   NOP; NOP
NOP; CONVERT_INT16 S6, S12, S11, SR;   NOP; NOP

// CHECK: nop;    convert_int16 rhne S6, S12, S11, SP0;   nop;    nop
// CHECK: nop;    convert_int16 rhne S6, S12, S11, SP0;   nop;    nop
// CHECK: nop;    convert_int16 rz   S6, S12, S11, SP0;  nop;    nop
// CHECK: nop;    convert_int16 ru   S6, S12, S11, SP0;  nop;    nop
// CHECK: nop;    convert_int16 rd   S6, S12, S11, SP0;  nop;    nop
// CHECK: nop;    convert_int16 sr   S6, S12, S11, SP0;  nop;    nop


NOP; CONVERT_INT16 S6, S12, 6, RNE;      NOP; NOP
NOP; CONVERT_INT16 S6, S12, 6, RD, SP1;  NOP; NOP
NOP; CONVERT_INT16 S6, S12, 6, RU, !SP1; NOP; NOP

// CHECK: nop;    convert_int16 rhne S6, S12, 0x6, SP0;   nop;    nop
// CHECK: nop;    convert_int16 rd   S6, S12, 0x6, SP1;   nop;    nop
// CHECK: nop;    convert_int16 ru   S6, S12, 0x6, !SP1;  nop;    nop
