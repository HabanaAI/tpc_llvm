// RUN: %clang -cc1as -triple tpc-none-none -target-cpu gaudi -filetype obj %s -o %t.o
// RUN: %disasm --mcpu=gaudi %t.o | FileCheck %s
// RUN: %clang -cc1as -triple tpc-none-none -target-cpu goya2 -filetype obj %s -o %t.o
// RUN: %disasm --mcpu=goya2 %t.o | FileCheck %s

NOP; NOP; FIND_FIRST.BF16 0 START_LSB V1, V2; NOP
NOP; NOP; FIND_FIRST.BF16 0 START_LSB V1, V2, SP3; NOP
NOP; NOP; FIND_FIRST.BF16 0 START_LSB V1, V2, !SP3; NOP
NOP; NOP; FIND_FIRST.BF16 0 START_LSB V1, V2, VP3; NOP
NOP; NOP; FIND_FIRST.BF16 0 START_LSB V1, V2, !VP3; NOP

NOP; NOP; FIND_FIRST.BF16 1 START_LSB V1, V2; NOP
NOP; NOP; FIND_FIRST.BF16 1 START_LSB V1, V2, SP3; NOP
NOP; NOP; FIND_FIRST.BF16 1 START_LSB V1, V2, !SP3; NOP
NOP; NOP; FIND_FIRST.BF16 1 START_LSB V1, V2, VP3; NOP
NOP; NOP; FIND_FIRST.BF16 1 START_LSB V1, V2, !VP3; NOP

NOP; NOP; FIND_FIRST.BF16 0 START_MSB V1, V2; NOP
NOP; NOP; FIND_FIRST.BF16 0 START_MSB V1, V2, SP3; NOP
NOP; NOP; FIND_FIRST.BF16 0 START_MSB V1, V2, !SP3; NOP
NOP; NOP; FIND_FIRST.BF16 0 START_MSB V1, V2, VP3; NOP
NOP; NOP; FIND_FIRST.BF16 0 START_MSB V1, V2, !VP3; NOP

NOP; NOP; FIND_FIRST.BF16 1 START_MSB V1, V2; NOP
NOP; NOP; FIND_FIRST.BF16 1 START_MSB V1, V2, SP3; NOP
NOP; NOP; FIND_FIRST.BF16 1 START_MSB V1, V2, !SP3; NOP
NOP; NOP; FIND_FIRST.BF16 1 START_MSB V1, V2, VP3; NOP
NOP; NOP; FIND_FIRST.BF16 1 START_MSB V1, V2, !VP3; NOP

// CHECK: nop; 	nop; 	find_first.bf16 set=0 start_lsb V1, V2; 	nop
// CHECK: nop; 	nop; 	find_first.bf16 set=0 start_lsb V1, V2, SP3; 	nop
// CHECK: nop; 	nop; 	find_first.bf16 set=0 start_lsb V1, V2, !SP3; 	nop
// CHECK: nop; 	nop; 	find_first.bf16 set=0 start_lsb V1, V2, VP3; 	nop
// CHECK: nop; 	nop; 	find_first.bf16 set=0 start_lsb V1, V2, !VP3; 	nop
// CHECK: nop; 	nop; 	find_first.bf16  set=1 start_lsb V1, V2; 	nop
// CHECK: nop; 	nop; 	find_first.bf16  set=1 start_lsb V1, V2, SP3; 	nop
// CHECK: nop; 	nop; 	find_first.bf16  set=1 start_lsb V1, V2, !SP3; 	nop
// CHECK: nop; 	nop; 	find_first.bf16  set=1 start_lsb V1, V2, VP3; 	nop
// CHECK: nop; 	nop; 	find_first.bf16  set=1 start_lsb V1, V2, !VP3; 	nop
// CHECK: nop; 	nop; 	find_first.bf16 set=0 start_msb V1, V2; 	nop
// CHECK: nop; 	nop; 	find_first.bf16 set=0 start_msb V1, V2, SP3; 	nop
// CHECK: nop; 	nop; 	find_first.bf16 set=0 start_msb V1, V2, !SP3; 	nop
// CHECK: nop; 	nop; 	find_first.bf16 set=0 start_msb V1, V2, VP3; 	nop
// CHECK: nop; 	nop; 	find_first.bf16 set=0 start_msb V1, V2, !VP3; 	nop
// CHECK: nop; 	nop; 	find_first.bf16  set=1 start_msb V1, V2; 	nop
// CHECK: nop; 	nop; 	find_first.bf16  set=1 start_msb V1, V2, SP3; 	nop
// CHECK: nop; 	nop; 	find_first.bf16  set=1 start_msb V1, V2, !SP3; 	nop
// CHECK: nop; 	nop; 	find_first.bf16  set=1 start_msb V1, V2, VP3; 	nop
// CHECK: nop; 	nop; 	find_first.bf16  set=1 start_msb V1, V2, !VP3; 	nop

