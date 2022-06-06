// RUN: %clang -cc1as -triple tpc-none-none -target-cpu goya2 -filetype obj %s -o %t.o
// RUN: %disasm --mcpu goya2 %t.o | FileCheck %s
// RUN: %clang -cc1as -triple tpc-none-none -target-cpu gaudi2 -filetype obj %s -o %t.o
// RUN: %disasm --mcpu gaudi2 %t.o | FileCheck %s
// RUN: %clang -cc1as -triple tpc-none-none -target-cpu doron1 -filetype obj %s -o %t.o
// RUN: %disasm --mcpu doron1 %t.o | FileCheck %s

nop; nop; mov_dg.pack pack_TYPE=0 V1, V2; nop
nop; nop; mov_dg.pack pack_TYPE=1 V1, V2; nop

//CHECK: nop; 	nop; 	mov_dg.pack pack_type=0 V1, V2; 	nop
//CHECK: nop; 	nop; 	mov_dg.pack pack_type=1 V1, V2; 	nop

nop; nop; mov_dg.pack pack_TYPE=0 V1, V2, SP3; nop
nop; nop; mov_dg.pack pack_TYPE=0 V1, V2, !SP3; nop
nop; nop; mov_dg.pack pack_TYPE=0 V1, V2, VP3; nop
nop; nop; mov_dg.pack pack_TYPE=0 V1, V2, !VP3; nop

//CHECK: nop; 	nop; 	mov_dg.pack pack_type=0 V1, V2, SP3; 	nop
//CHECK: nop; 	nop; 	mov_dg.pack pack_type=0 V1, V2, !SP3; 	nop
//CHECK: nop; 	nop; 	mov_dg.pack pack_type=0 V1, V2, VP3; 	nop
//CHECK: nop; 	nop; 	mov_dg.pack pack_type=0 V1, V2, !VP3; 	nop

NOP; NOP; MOV_DUAL_GROUP.PACK PACK_TYPE=0 V1, V2; NOP

//CHECK: nop; 	nop; 	mov_dg.pack pack_type=0 V1, V2; 	nop
