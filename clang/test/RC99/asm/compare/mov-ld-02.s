// RUN: %clang -cc1as -triple tpc-none-none -filetype obj -target-cpu gaudi %s -o %t.o
// RUN: %disasm --mcpu gaudi %t.o | FileCheck %s


MOV.FROM_HW_REG S4, 0
MOV.FROM_HW_REG S4, 0, SP2
MOV.FROM_HW_REG S4, 0, !SP2

MOV S4, PC
MOV S4, PC, SP2
MOV S4, PC, !SP2

// CHECK: mov S4, PC;        nop;    nop;    nop
// CHECK: mov S4, PC, SP2;        nop;    nop;    nop
// CHECK: mov S4, PC, !SP2;       nop;    nop;    nop
// CHECK: mov S4, PC;        nop;    nop;    nop
// CHECK: mov S4, PC, SP2;        nop;    nop;    nop
// CHECK: mov S4, PC, !SP2;       nop;    nop;    nop
