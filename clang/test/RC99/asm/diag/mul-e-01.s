// RUN: not %clang -cc1as -triple tpc-none-none %s 2>&1 | FileCheck %s

NOP; MUL.I8.UPPER32  S5, S1, S2, !SP3
// CHECK: error: switch is not valid for used operand type

NOP; MUL.U8.UPPER32  S5, S1, u8_123
// CHECK: error: switch is not valid for used operand type

NOP; MUL.I16.UPPER32 S5, S1, 123, SP3
// CHECK: error: switch is not valid for used operand type

NOP; MUL.U16.UPPER32 S5, S1, u16_222, !SP3
// CHECK: error: switch is not valid for used operand type

NOP; MUL.I32.DOUBLE_AND_ROUND32 S5, S1, u16_222, !SP3
// CHECK: error: switch is not valid for used operand type

NOP; NOP; MUL.I32 V3, V2, V1


