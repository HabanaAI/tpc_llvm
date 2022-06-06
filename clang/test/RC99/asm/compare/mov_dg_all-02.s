// RUN: %clang -cc1as -triple tpc-none-none -target-cpu gaudi -filetype obj %s -o %t.o
// RUN: %disasm --mcpu gaudi %t.o | FileCheck %s
// RUN: %clang -cc1as -triple tpc-none-none -target-cpu gaudib -filetype obj %s -o %t.o
// RUN: %disasm --mcpu gaudib %t.o | FileCheck %s
// RUN: %clang -cc1as -triple tpc-none-none -target-cpu goya2 -filetype obj %s -o %t.o
// RUN: %disasm --mcpu goya2 %t.o | FileCheck %s
// RUN: %clang -cc1as -triple tpc-none-none -target-cpu gaudi2 -filetype obj %s -o %t.o
// RUN: %disasm --mcpu gaudi2 %t.o | FileCheck %s
// RUN: %clang -cc1as -triple tpc-none-none -target-cpu doron1 -filetype obj %s -o %t.o
// RUN: %disasm --mcpu doron1 %t.o | FileCheck %s

nop; nop; mov_dg.all V2, V1, sdg0=0, sdg1=1, sdg2=2, sdg3=3, weg0=3, weg1=2, weg2=1, weg3=0, 123; nop
nop; nop; mov_dg.all V2, V1, sdg0=1, sdg1=2, sdg2=3, sdg3=0, weg0=2, weg1=1, weg2=0, weg3=3, 123; nop
nop; nop; mov_dg.all V2, V1, sdg0=2, sdg1=3, sdg2=0, sdg3=1, weg0=1, weg1=0, weg2=3, weg3=2, 123; nop
nop; nop; mov_dg.all V2, V1, sdg0=3, sdg1=0, sdg2=1, sdg3=2, weg0=0, weg1=3, weg2=2, weg3=1, 123; nop

// CHECK: nop; 	nop; 	mov_dg.all sdg0=0 sdg1=1 sdg2=2 sdg3=3 weg0=3 weg1=2 weg2=1 weg3=0 V2, V1, 0x7b; 	nop
// CHECK: nop; 	nop; 	mov_dg.all sdg0=1 sdg1=2 sdg2=3 sdg3=0 weg0=2 weg1=1 weg2=0 weg3=3 V2, V1, 0x7b; 	nop
// CHECK: nop; 	nop; 	mov_dg.all sdg0=2 sdg1=3 sdg2=0 sdg3=1 weg0=1 weg1=0 weg2=3 weg3=2 V2, V1, 0x7b; 	nop
// CHECK: nop; 	nop; 	mov_dg.all sdg0=3 sdg1=0 sdg2=1 sdg3=2 weg0=0 weg1=3 weg2=2 weg3=1 V2, V1, 0x7b; 	nop

nop; nop; mov_dg.all V2, V1, sdg0=0, sdg1=1, sdg2=2, sdg3=3, weg0=3, weg1=2, weg2=1, weg3=0, 123, SP3; nop
nop; nop; mov_dg.all V2, V1, sdg0=0, sdg1=1, sdg2=2, sdg3=3, weg0=3, weg1=2, weg2=1, weg3=0, 123, !SP3; nop
nop; nop; mov_dg.all V2, V1, sdg0=0, sdg1=1, sdg2=2, sdg3=3, weg0=3, weg1=2, weg2=1, weg3=0, 123, VP3; nop
nop; nop; mov_dg.all V2, V1, sdg0=0, sdg1=1, sdg2=2, sdg3=3, weg0=3, weg1=2, weg2=1, weg3=0, 123, !VP3; nop

// CHECK: nop; 	nop; 	mov_dg.all sdg0=0 sdg1=1 sdg2=2 sdg3=3 weg0=3 weg1=2 weg2=1 weg3=0 V2, V1, 0x7b, SP3; 	nop
// CHECK: nop; 	nop; 	mov_dg.all sdg0=0 sdg1=1 sdg2=2 sdg3=3 weg0=3 weg1=2 weg2=1 weg3=0 V2, V1, 0x7b, !SP3; 	nop
// CHECK: nop; 	nop; 	mov_dg.all sdg0=0 sdg1=1 sdg2=2 sdg3=3 weg0=3 weg1=2 weg2=1 weg3=0 V2, V1, 0x7b, VP3; 	nop
// CHECK: nop; 	nop; 	mov_dg.all sdg0=0 sdg1=1 sdg2=2 sdg3=3 weg0=3 weg1=2 weg2=1 weg3=0 V2, V1, 0x7b, !VP3; 	nop

nop; nop; mov_dg.all V2, V1, sdg0=0, sdg1=1, sdg2=2, sdg3=3, weg0=3, weg1=2, weg2=1, weg3=0, 50; nop

// CHECK: nop; 	nop; 	mov_dg.all sdg0=0 sdg1=1 sdg2=2 sdg3=3 weg0=3 weg1=2 weg2=1 weg3=0 V2, V1, 0x32; 	nop

NOP; NOP; MOV_DUAL_GROUP.ALL V2, V1, SDG0=0, SDG1=1, SDG2=2, SDG3=3, WEG0=3, WEG1=2, WEG2=1, WEG3=0, 123; NOP

// CHECK: nop; 	nop; 	mov_dg.all sdg0=0 sdg1=1 sdg2=2 sdg3=3 weg0=3 weg1=2 weg2=1 weg3=0 V2, V1, 0x7b; 	nop
