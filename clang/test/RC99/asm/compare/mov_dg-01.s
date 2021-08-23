// RUN: %clang -cc1as -triple tpc-none-none -filetype obj %s -o %t.o
// RUN: %disasm %t.o | FileCheck %s
// RUN: %clang -cc1as -triple tpc-none-none -target-cpu gaudi -filetype obj %s -o %t.o
// RUN: %disasm -mcpu gaudi %t.o | FileCheck %s







nop; nop; mov_dg V1, V2, src=0, dst=0, wr_lg=0, wr_ug=0, 123; nop
nop; nop; mov_dg V1, V2, src=1, dst=0, wr_lg=0, wr_ug=0, 123; nop
nop; nop; mov_dg V1, V2, src=2, dst=0, wr_lg=0, wr_ug=0, 123; nop
nop; nop; mov_dg V1, V2, src=3, dst=0, wr_lg=0, wr_ug=0, 123; nop

//CHECK: nop; 	nop; 	mov_dg src=0 dst=0 wr_lg=0x0 wr_ug=0x0 V1, V2, 0x7b, SP0; 	nop
//CHECK: nop; 	nop; 	mov_dg src=1 dst=0 wr_lg=0x0 wr_ug=0x0 V1, V2, 0x7b, SP0; 	nop
//CHECK: nop; 	nop; 	mov_dg src=2 dst=0 wr_lg=0x0 wr_ug=0x0 V1, V2, 0x7b, SP0; 	nop
//CHECK: nop; 	nop; 	mov_dg src=3 dst=0 wr_lg=0x0 wr_ug=0x0 V1, V2, 0x7b, SP0; 	nop

nop; nop; mov_dg V1, V2, src=0, dst=0, wr_lg=0, wr_ug=0, 123; nop
nop; nop; mov_dg V1, V2, src=0, dst=1, wr_lg=0, wr_ug=0, 123; nop
nop; nop; mov_dg V1, V2, src=0, dst=2, wr_lg=0, wr_ug=0, 123; nop
nop; nop; mov_dg V1, V2, src=0, dst=3, wr_lg=0, wr_ug=0, 123; nop

//CHECK: nop; 	nop; 	mov_dg src=0 dst=0 wr_lg=0x0 wr_ug=0x0 V1, V2, 0x7b, SP0; 	nop
//CHECK: nop; 	nop; 	mov_dg src=0 dst=1 wr_lg=0x0 wr_ug=0x0 V1, V2, 0x7b, SP0; 	nop
//CHECK: nop; 	nop; 	mov_dg src=0 dst=2 wr_lg=0x0 wr_ug=0x0 V1, V2, 0x7b, SP0; 	nop
//CHECK: nop; 	nop; 	mov_dg src=0 dst=3 wr_lg=0x0 wr_ug=0x0 V1, V2, 0x7b, SP0; 	nop

nop; nop; mov_dg V1, V2, src=0, dst=0, wr_lg=1, wr_ug=0, 123; nop
nop; nop; mov_dg V1, V2, src=0, dst=0, wr_lg=0, wr_ug=1, 123; nop
nop; nop; mov_dg V1, V2, src=0, dst=0, wr_lg=1, wr_ug=1, 123; nop

//CHECK: nop; 	nop; 	mov_dg src=0 dst=0 wr_lg=0x1 wr_ug=0x0 V1, V2, 0x7b, SP0; 	nop
//CHECK: nop; 	nop; 	mov_dg src=0 dst=0 wr_lg=0x0 wr_ug=0x1 V1, V2, 0x7b, SP0; 	nop
//CHECK: nop; 	nop; 	mov_dg src=0 dst=0 wr_lg=0x1 wr_ug=0x1 V1, V2, 0x7b, SP0; 	nop

nop; nop; mov_dg V1, V2, src=0, dst=1, wr_lg=1, wr_ug=1, 123; nop
nop; nop; mov_dg V1, V2, src=0, dst=1, wr_lg=1, wr_ug=1, 123, SP3; nop
nop; nop; mov_dg V1, V2, src=0, dst=1, wr_lg=1, wr_ug=1, 123, !SP3; nop
nop; nop; mov_dg V1, V2, src=0, dst=1, wr_lg=1, wr_ug=1, 123, VP3; nop
nop; nop; mov_dg V1, V2, src=0, dst=1, wr_lg=1, wr_ug=1, 123, !VP3; nop

//CHECK: nop; 	nop; 	mov_dg src=0 dst=1 wr_lg=0x1 wr_ug=0x1 V1, V2, 0x7b, SP0; 	nop
//CHECK: nop; 	nop; 	mov_dg src=0 dst=1 wr_lg=0x1 wr_ug=0x1 V1, V2, 0x7b, SP3; 	nop
//CHECK: nop; 	nop; 	mov_dg src=0 dst=1 wr_lg=0x1 wr_ug=0x1 V1, V2, 0x7b, !SP3; 	nop
//CHECK: nop; 	nop; 	mov_dg src=0 dst=1 wr_lg=0x1 wr_ug=0x1 V1, V2, 0x7b, VP3; 	nop
//CHECK: nop; 	nop; 	mov_dg src=0 dst=1 wr_lg=0x1 wr_ug=0x1 V1, V2, 0x7b, !VP3; 	nop

nop; nop; mov_dg V1, V2, src=0, dst=0, wr_lg=0, wr_ug=0, 50; nop

//CHECK: nop; 	nop; 	mov_dg src=0 dst=0 wr_lg=0x0 wr_ug=0x0 V1, V2, 0x32, SP0; 	nop

NOP; NOP; MOV_DUAL_GROUP V1, V2, SRC_DUAL_GROUP=0, DST_DUAL_GROUP=0, WR_LOWER_GROUP=0, WR_UPPER_GROUP=0, 123; NOP

//CHECK: nop; 	nop; 	mov_dg src=0 dst=0 wr_lg=0x0 wr_ug=0x0 V1, V2, 0x7b, SP0; 	nop