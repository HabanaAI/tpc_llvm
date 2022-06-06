// RUN: %clang -cc1as -triple tpc-none-none -target-cpu doron1 -filetype obj %s -o %t.o
// RUN: %disasm --mcpu doron1 %t.o | FileCheck %s

nop; nop; mov_dg.all.swap_hl V2, V1, sdg0=0, sdg1=1, sdg2=2, sdg3=3, weg0=3, weg1=2, weg2=1, weg3=0, 123; nop
nop; nop; mov_dg.all swap_hl V2, V1, sdg0=1, sdg1=2, sdg2=3, sdg3=0, weg0=2, weg1=1, weg2=0, weg3=3, 123; nop
nop; nop; mov_dg all swap_hl V2, V1, sdg0=2, sdg1=3, sdg2=0, sdg3=1, weg0=1, weg1=0, weg2=3, weg3=2, 123; nop
nop; nop; mov_dg.all.swap_hl V2, V1, sdg0=3, sdg1=0, sdg2=1, sdg3=2, weg0=0, weg1=3, weg2=2, weg3=1, 123; nop

// CHECK: nop; 	nop; 	mov_dg.all swap_hl  sdg0=0 sdg1=1 sdg2=2 sdg3=3 weg0=3 weg1=2 weg2=1 weg3=0 V2, V1, 0x7b; 	nop
// CHECK: nop; 	nop; 	mov_dg.all swap_hl  sdg0=1 sdg1=2 sdg2=3 sdg3=0 weg0=2 weg1=1 weg2=0 weg3=3 V2, V1, 0x7b; 	nop
// CHECK: nop; 	nop; 	mov_dg.all swap_hl  sdg0=2 sdg1=3 sdg2=0 sdg3=1 weg0=1 weg1=0 weg2=3 weg3=2 V2, V1, 0x7b; 	nop
// CHECK: nop; 	nop; 	mov_dg.all swap_hl  sdg0=3 sdg1=0 sdg2=1 sdg3=2 weg0=0 weg1=3 weg2=2 weg3=1 V2, V1, 0x7b; 	nop

nop; nop; mov_dg.all.swap_eo V2, V1, sdg0=0, sdg1=1, sdg2=2, sdg3=3, weg0=3, weg1=2, weg2=1, weg3=0, 123, SP3; nop
nop; nop; mov_dg.all.swap_eo V2, V1, sdg0=0, sdg1=1, sdg2=2, sdg3=3, weg0=3, weg1=2, weg2=1, weg3=0, 123, !SP3; nop
nop; nop; mov_dg.all.swap_eo V2, V1, sdg0=0, sdg1=1, sdg2=2, sdg3=3, weg0=3, weg1=2, weg2=1, weg3=0, 123, VP3; nop
nop; nop; mov_dg.all.swap_eo V2, V1, sdg0=0, sdg1=1, sdg2=2, sdg3=3, weg0=3, weg1=2, weg2=1, weg3=0, 123, !VP3; nop

// CHECK: nop; 	nop; 	mov_dg.all swap_eo sdg0=0 sdg1=1 sdg2=2 sdg3=3 weg0=3 weg1=2 weg2=1 weg3=0 V2, V1, 0x7b, SP3; 	nop
// CHECK: nop; 	nop; 	mov_dg.all swap_eo sdg0=0 sdg1=1 sdg2=2 sdg3=3 weg0=3 weg1=2 weg2=1 weg3=0 V2, V1, 0x7b, !SP3; 	nop
// CHECK: nop; 	nop; 	mov_dg.all swap_eo sdg0=0 sdg1=1 sdg2=2 sdg3=3 weg0=3 weg1=2 weg2=1 weg3=0 V2, V1, 0x7b, VP3; 	nop
// CHECK: nop; 	nop; 	mov_dg.all swap_eo sdg0=0 sdg1=1 sdg2=2 sdg3=3 weg0=3 weg1=2 weg2=1 weg3=0 V2, V1, 0x7b, !VP3; 	nop

nop;    nop;    mov_dg.all.ctrl_reg V0, V1, S36, 0x39;         nop
nop;    nop;    mov_dg.all ctrl_reg V0, V1, S36, 0x39, SP3;    nop
nop;    nop;    mov_dg.all.ctrl_reg V0, V1, S36, 0x39, !SP3;   nop
nop;    nop;    mov_dg all ctrl_reg V0, V1, S36, 0x39, VP3;    nop
nop;    nop;    mov_dg.all          V0, V1, S36, 0x39, !VP3;   nop

// CHECK: nop;    nop;    mov_dg.all ctrl_reg weg0=0 weg1=0 weg2=0 weg3=0 V0, V1, 0x39, S36;      nop
// CHECK: nop;    nop;    mov_dg.all ctrl_reg weg0=0 weg1=0 weg2=0 weg3=0 V0, V1, 0x39, S36, SP3;         nop
// CHECK: nop;    nop;    mov_dg.all ctrl_reg weg0=0 weg1=0 weg2=0 weg3=0 V0, V1, 0x39, S36, !SP3;        nop
// CHECK: nop;    nop;    mov_dg.all ctrl_reg weg0=0 weg1=0 weg2=0 weg3=0 V0, V1, 0x39, S36, VP3;         nop
// CHECK: nop;    nop;    mov_dg.all ctrl_reg weg0=0 weg1=0 weg2=0 weg3=0 V0, V1, 0x39, S36, !VP3;        nop

nop;    nop;    mov_dg.all.ctrl_reg weg0=0 weg1=0 weg2=0 weg3=0 V0, V1, S36, 0x39;         nop
nop;    nop;    mov_dg.all.ctrl_reg weg0=0 weg1=0 weg2=0 weg3=0 V0, V1, S36, 0x39, SP3;    nop
nop;    nop;    mov_dg.all ctrl_reg weg0=0 weg1=0 weg2=0 weg3=0 V0, V1, S36, 0x39, !SP3;   nop
nop;    nop;    mov_dg all ctrl_reg weg0=0 weg1=0 weg2=0 weg3=0 V0, V1, S36, 0x39, VP3;    nop
nop;    nop;    mov_dg.all          weg0=0 weg1=0 weg2=0 weg3=0 V0, V1, S36, 0x39, !VP3;   nop

// CHECK: nop;    nop;    mov_dg.all ctrl_reg weg0=0 weg1=0 weg2=0 weg3=0 V0, V1, 0x39, S36;      nop
// CHECK: nop;    nop;    mov_dg.all ctrl_reg weg0=0 weg1=0 weg2=0 weg3=0 V0, V1, 0x39, S36, SP3;         nop
// CHECK: nop;    nop;    mov_dg.all ctrl_reg weg0=0 weg1=0 weg2=0 weg3=0 V0, V1, 0x39, S36, !SP3;        nop
// CHECK: nop;    nop;    mov_dg.all ctrl_reg weg0=0 weg1=0 weg2=0 weg3=0 V0, V1, 0x39, S36, VP3;         nop
// CHECK: nop;    nop;    mov_dg.all ctrl_reg weg0=0 weg1=0 weg2=0 weg3=0 V0, V1, 0x39, S36, !VP3;        nop

