// RUN: %tpc_clang -c -x assembler -march=goya2 %s -o %ttest.o
// RUN: %disasm --mcpu=goya2 %ttest.o | FileCheck %s

NOP; NOP; MAC.I8.ACC_I16 D6, V1, V2, VP3; nop
// CHECK: nop; nop; mac.i8 acc_i16 D6, V1, V2, VP3; nop

nop; nop; mac.i8 acc_i16 D6, V1, V2, VP3; nop
// CHECK: nop; nop; mac.i8 acc_i16 D6, V1, V2, VP3; nop

NOP; NOP; MAC.U8.ACC_I16 D6, V1, V2, VP3; nop
// CHECK: nop; nop; mac.u8 acc_i16 D6, V1, V2, VP3; nop

nop; nop; mac.u8 acc_i16 D6, V1, V2, VP3; nop
// CHECK: nop; nop; mac.u8 acc_i16 D6, V1, V2, VP3; nop

nop; nop; mac.u16 acc_i32 D6, V1, V2, VP3; nop
// CHECK: nop; nop; mac.u16 acc_i32 D6, V1, V2, VP3; nop

