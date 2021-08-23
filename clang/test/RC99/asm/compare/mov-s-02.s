// RUN: %clang -cc1as -triple tpc-none-none -filetype obj -target-cpu gaudi %s -o %t.o
// RUN: %disasm -mcpu gaudi %t.o | FileCheck %s


NOP; MOV.FROM_HW_REG S4, 0
NOP; MOV.FROM_HW_REG S4, 0, SP2
NOP; MOV.FROM_HW_REG S4, 0, !SP2

NOP; MOV S4, PC
NOP; MOV S4, PC, SP2
NOP; MOV S4, PC, !SP2

// CHECK: nop;    mov S4, PC, SP0;        nop;    nop
// CHECK: nop;    mov S4, PC, SP2;        nop;    nop
// CHECK: nop;    mov S4, PC, !SP2;       nop;    nop
// CHECK: nop;    mov S4, PC, SP0;        nop;    nop
// CHECK: nop;    mov S4, PC, SP2;        nop;    nop
// CHECK: nop;    mov S4, PC, !SP2;       nop;    nop
