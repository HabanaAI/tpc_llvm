// RUN: %clang -cc1as -triple tpc-none-none -filetype obj -target-cpu goya2 %s -o %t.o
// RUN: %disasm --mcpu goya2 %t.o | FileCheck %s
// RUN: %clang -cc1as -triple tpc-none-none -filetype obj -target-cpu gaudi2 %s -o %t.o
// RUN: %disasm --mcpu gaudi2 %t.o | FileCheck %s
// RUN: %clang -cc1as -triple tpc-none-none -filetype obj -target-cpu doron1 %s -o %t.o
// RUN: %disasm --mcpu doron1 %t.o | FileCheck %s

nop; 	nop; 	get_lut_entry_and_interval_start.bf16 opt D0, V3, 0x0; 	nop
nop; 	nop; 	get_lut_entry_and_interval_start.bf16 exp0 D0, V3, 0x0; 	nop
nop; 	nop; 	get_lut_entry_and_interval_start.bf16 opt exp0 D0, V3, 0x0; 	nop

// CHECK: nop; 	nop; 	get_lut_entry_and_interval_start.bf16 opt D0, V3, 0x0; 	nop
// CHECK: nop; 	nop; 	get_lut_entry_and_interval_start.bf16 exp0 D0, V3, 0x0; 	nop
// CHECK: nop; 	nop; 	get_lut_entry_and_interval_start.bf16 opt exp0 D0, V3, 0x0; 	nop

nop; 	nop; 	get_lut_entry_and_interval_start.bf16 opt D0, V3, 0x0; 	nop
nop; 	nop; 	get_lut_entry_and_interval_start.bf16 exp0 D0, V3, 0x0; 	nop
nop; 	nop; 	get_lut_entry_and_interval_start.bf16 opt exp0 D0, V3, 0x0; 	nop

// CHECK: nop; 	nop; 	get_lut_entry_and_interval_start.bf16 opt D0, V3, 0x0; 	nop
// CHECK: nop; 	nop; 	get_lut_entry_and_interval_start.bf16 exp0 D0, V3, 0x0; 	nop
// CHECK: nop; 	nop; 	get_lut_entry_and_interval_start.bf16 opt exp0 D0, V3, 0x0; 	nop

nop; 	nop; 	get_lut_entry_and_interval_start.f16 exp0 D0, V3, 0x0; 	nop

// CHECK: nop; 	nop; 	get_lut_entry_and_interval_start.f16 exp0 D0, V3, 0x0; 	nop

nop; 	nop; 	get_lut_entry_and_interval_start.f16 exp0 D0, V3, 0x0; 	nop

// CHECK: nop; 	nop; 	get_lut_entry_and_interval_start.f16 exp0 D0, V3, 0x0; 	nop
