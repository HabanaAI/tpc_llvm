// RUN: %clang -cc1as -triple tpc-none-none -target-cpu gaudib -filetype obj %s -o %t.o
// RUN: %disasm --mcpu gaudib %t.o | FileCheck %s
// RUN: %clang -cc1as -triple tpc-none-none -target-cpu gaudi2 -filetype obj %s -o %t.o
// RUN: %disasm --mcpu gaudi2 %t.o | FileCheck %s
// RUN: %clang -cc1as -triple tpc-none-none -target-cpu doron1 -filetype obj %s -o %t.o
// RUN: %disasm --mcpu doron1 %t.o | FileCheck %s

nop; 	nop; 	mov_dg.unpack unpack_type=0, sdg0=0, sdg1=1, sdg2=2, sdg3=3, weg0=3, weg1=2, weg2=1, weg3=0, V1, V2, 123; 	nop
nop; 	nop; 	mov_dg.unpack unpack_type=0, sdg0=1, sdg1=2, sdg2=3, sdg3=0, weg0=2, weg1=1, weg2=0, weg3=3, V1, V2, 123; 	nop
nop; 	nop; 	mov_dg.unpack unpack_type=0, sdg0=2, sdg1=3, sdg2=0, sdg3=1, weg0=1, weg1=0, weg2=3, weg3=2, V1, V2, 123; 	nop
nop; 	nop; 	mov_dg.unpack unpack_type=0, sdg0=3, sdg1=0, sdg2=1, sdg3=2, weg0=0, weg1=3, weg2=2, weg3=1, V1, V2, 123; 	nop

//CHECK: nop;    nop;    mov_dg.unpack unpack_type=0 sdg0=0 sdg1=1 sdg2=2 sdg3=3 weg0=3 weg1=2 weg2=1 weg3=0 V1, V2, 0x7b;  nop
//CHECK: nop;    nop;    mov_dg.unpack unpack_type=0 sdg0=1 sdg1=2 sdg2=3 sdg3=0 weg0=2 weg1=1 weg2=0 weg3=3 V1, V2, 0x7b;  nop
//CHECK: nop;    nop;    mov_dg.unpack unpack_type=0 sdg0=2 sdg1=3 sdg2=0 sdg3=1 weg0=1 weg1=0 weg2=3 weg3=2 V1, V2, 0x7b;  nop
//CHECK: nop;    nop;    mov_dg.unpack unpack_type=0 sdg0=3 sdg1=0 sdg2=1 sdg3=2 weg0=0 weg1=3 weg2=2 weg3=1 V1, V2, 0x7b;  nop

nop; 	nop; 	mov_dg.unpack V1, V2, sdg0=0, sdg1=1, sdg2=2, sdg3=3, weg0=3, weg1=2, weg2=1, weg3=0, unpack_type=0, 123, SP3; 	nop
nop; 	nop; 	mov_dg.unpack V1, V2, sdg0=0, sdg1=1, sdg2=2, sdg3=3, weg0=3, weg1=2, weg2=1, weg3=0, unpack_type=0, 123, !SP3; 	nop
nop; 	nop; 	mov_dg.unpack V1, V2, sdg0=0, sdg1=1, sdg2=2, sdg3=3, weg0=3, weg1=2, weg2=1, weg3=0, unpack_type=0, 123, VP3; 	nop
nop; 	nop; 	mov_dg.unpack V1, V2, sdg0=0, sdg1=1, sdg2=2, sdg3=3, weg0=3, weg1=2, weg2=1, weg3=0, unpack_type=0, 123, !VP3; 	nop

//CHECK: nop;    nop;    mov_dg.unpack unpack_type=0 sdg0=0 sdg1=1 sdg2=2 sdg3=3 weg0=3 weg1=2 weg2=1 weg3=0 V1, V2, 0x7b, SP3;  nop
//CHECK: nop;    nop;    mov_dg.unpack unpack_type=0 sdg0=0 sdg1=1 sdg2=2 sdg3=3 weg0=3 weg1=2 weg2=1 weg3=0 V1, V2, 0x7b, !SP3;         nop
//CHECK: nop;    nop;    mov_dg.unpack unpack_type=0 sdg0=0 sdg1=1 sdg2=2 sdg3=3 weg0=3 weg1=2 weg2=1 weg3=0 V1, V2, 0x7b, VP3;  nop
//CHECK: nop;    nop;    mov_dg.unpack unpack_type=0 sdg0=0 sdg1=1 sdg2=2 sdg3=3 weg0=3 weg1=2 weg2=1 weg3=0 V1, V2, 0x7b, !VP3;         nop

nop; 	nop; 	mov_dg.unpack unpack_type=0, sdg0=0, sdg1=1, sdg2=2, sdg3=3, weg0=3, weg1=2, weg2=1, weg3=0, V1, V2, 50; 	nop

//CHECK: nop;    nop;    mov_dg.unpack unpack_type=0 sdg0=0 sdg1=1 sdg2=2 sdg3=3 weg0=3 weg1=2 weg2=1 weg3=0 V1, V2, 0x32;  nop

nop; 	nop; 	mov_dg.unpack unpack_type=1, sdg0=0, sdg1=1, sdg2=2, sdg3=3, weg0=3, weg1=2, weg2=1, weg3=0, V1, V2, 123; 	nop

//CHECK: nop;    nop;    mov_dg.unpack unpack_type=1 sdg0=0 sdg1=1 sdg2=2 sdg3=3 weg0=3 weg1=2 weg2=1 weg3=0 V1, V2, 0x7b;  nop

NOP; 	NOP; 	MOV_DUAL_GROUP.UNPACK UNPACK_TYPE=0, SDG0=0, SDG1=1, SDG2=2, SDG3=3, WEG0=3, WEG1=2, WEG2=1, WEG3=0, V1, V2, 123; 	NOP

//CHECK: nop;    nop;    mov_dg.unpack unpack_type=0 sdg0=0 sdg1=1 sdg2=2 sdg3=3 weg0=3 weg1=2 weg2=1 weg3=0 V1, V2, 0x7b;  nop
