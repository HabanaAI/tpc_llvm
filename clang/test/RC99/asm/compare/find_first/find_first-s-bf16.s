// RUN: %clang -cc1as -triple tpc-none-none -target-cpu gaudi -filetype obj %s -o %t.o
// RUN: %disasm --mcpu=gaudi %t.o | FileCheck %s
// RUN: %clang -cc1as -triple tpc-none-none -target-cpu goya2 -filetype obj %s -o %t.o
// RUN: %disasm --mcpu=goya2 %t.o | FileCheck %s

NOP; FIND_FIRST.BF16 0 START_LSB S1, S2; NOP; NOP
NOP; FIND_FIRST.BF16 0 START_LSB S1, S2, SP3; NOP; NOP
NOP; FIND_FIRST.BF16 0 START_LSB S1, S2, !SP3; NOP; NOP
NOP; FIND_FIRST.BF16 1 START_LSB S1, S2; NOP; NOP
NOP; FIND_FIRST.BF16 1 START_LSB S1, S2, SP3; NOP; NOP
NOP; FIND_FIRST.BF16 1 START_LSB S1, S2, !SP3; NOP; NOP
NOP; FIND_FIRST.BF16 0 START_MSB S1, S2; NOP; NOP
NOP; FIND_FIRST.BF16 0 START_MSB S1, S2, SP3; NOP; NOP
NOP; FIND_FIRST.BF16 0 START_MSB S1, S2, !SP3; NOP; NOP
NOP; FIND_FIRST.BF16 1 START_MSB S1, S2; NOP; NOP
NOP; FIND_FIRST.BF16 1 START_MSB S1, S2, SP3; NOP; NOP
NOP; FIND_FIRST.BF16 1 START_MSB S1, S2, !SP3; NOP; NOP

// CHECK: nop; 	find_first.bf16 set=0 start_lsb S1, S2; 	nop; 	nop
// CHECK: nop; 	find_first.bf16 set=0 start_lsb S1, S2, SP3; 	nop; 	nop
// CHECK: nop; 	find_first.bf16 set=0 start_lsb S1, S2, !SP3; 	nop; 	nop
// CHECK: nop; 	find_first.bf16  set=1 start_lsb S1, S2; 	nop; 	nop
// CHECK: nop; 	find_first.bf16  set=1 start_lsb S1, S2, SP3; 	nop; 	nop
// CHECK: nop; 	find_first.bf16  set=1 start_lsb S1, S2, !SP3; 	nop; 	nop
// CHECK: nop; 	find_first.bf16 set=0 start_msb S1, S2; 	nop; 	nop
// CHECK: nop; 	find_first.bf16 set=0 start_msb S1, S2, SP3; 	nop; 	nop
// CHECK: nop; 	find_first.bf16 set=0 start_msb S1, S2, !SP3; 	nop; 	nop
// CHECK: nop; 	find_first.bf16  set=1 start_msb S1, S2; 	nop; 	nop
// CHECK: nop; 	find_first.bf16  set=1 start_msb S1, S2, SP3; 	nop; 	nop
// CHECK: nop; 	find_first.bf16  set=1 start_msb S1, S2, !SP3; 	nop; 	nop