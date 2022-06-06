// RUN: %clang -cc1as -triple tpc-none-none -target-cpu goya2 -filetype obj %s -o %t.o
// RUN: %disasm --mcpu=goya2 %t.o | FileCheck %s

LD_G.EV_HINT S2, AD1; NOP; NOP; NOP
LD_G.EV_HINT S2, AD1, SP3; NOP; NOP; NOP
LD_G.EV_HINT S2, AD1, !SP3; NOP; NOP; NOP
// CHECK: ld_g ev_hint S2, AD1;      nop;    nop;    nop
// CHECK: ld_g ev_hint S2, AD1, SP3;      nop;    nop;    nop
// CHECK: ld_g ev_hint S2, AD1, !SP3;     nop;    nop;    nop

LD_G.L0CS S2, AD1; NOP; NOP; NOP
LD_G.L0CS S2, AD1, SP3; NOP; NOP; NOP
LD_G.L0CS S2, AD1, !SP3; NOP; NOP; NOP
// CHECK: ld_g l0cs S2, AD1;         nop;    nop;    nop
// CHECK: ld_g l0cs S2, AD1, SP3;         nop;    nop;    nop
// CHECK: ld_g l0cs S2, AD1, !SP3;        nop;    nop;    nop

LD_G b11001 I2, AD1; NOP; NOP; NOP
LD_G b11001 I2, AD1, SP3; NOP; NOP; NOP
LD_G b11001 I2, AD1, !SP3; NOP; NOP; NOP
// CHECK: ld_g  b11001 I2, AD1;      nop;    nop;    nop
// CHECK: ld_g  b11001 I2, AD1, SP3;      nop;    nop;    nop
// CHECK: ld_g  b11001 I2, AD1, !SP3;     nop;    nop;    nop

LD_G.BV64 b11001 I2, AD1; NOP; NOP; NOP
LD_G.BV64 b11001 I2, AD1, SP3; NOP; NOP; NOP
LD_G.BV64 b11001 I2, AD1, !SP3; NOP; NOP; NOP
// CHECK: ld_g bv64 b11001 I2, AD1;  nop;    nop;    nop
// CHECK: ld_g bv64 b11001 I2, AD1, SP3;  nop;    nop;    nop
// CHECK: ld_g bv64 b11001 I2, AD1, !SP3;         nop;    nop;    nop

LD_G M0 I2, AD1; NOP; NOP; NOP
LD_G M0 I2, AD1, SP3; NOP; NOP; NOP
LD_G M0 I2, AD1, !SP3; NOP; NOP; NOP
// CHECK: ld_g  M0 I2, AD1;  nop;    nop;    nop
// CHECK: ld_g  M0 I2, AD1, SP3;  nop;    nop;    nop
// CHECK: ld_g  M0 I2, AD1, !SP3;         nop;    nop;    nop

LD_G.BV64 M0 I2, AD1; NOP; NOP; NOP
LD_G.BV64 M0 I2, AD1, SP3; NOP; NOP; NOP
LD_G.BV64 M0 I2, AD1, !SP3; NOP; NOP; NOP
// CHECK: ld_g bv64 M0 I2, AD1;      nop;    nop;    nop
// CHECK: ld_g bv64 M0 I2, AD1, SP3;      nop;    nop;    nop
// CHECK: ld_g bv64 M0 I2, AD1, !SP3;     nop;    nop;    nop


LD_G.AUTO_INC S2, AD1; NOP; NOP; NOP
LD_G.AUTO_INC S2, AD1, SP3; NOP; NOP; NOP
LD_G.AUTO_INC S2, AD1, !SP3; NOP; NOP; NOP
// CHECK: ld_g inc_1 S2, AD1;     nop;    nop;    nop
// CHECK: ld_g inc_1 S2, AD1, SP3;     nop;    nop;    nop
// CHECK: ld_g inc_1 S2, AD1, !SP3;    nop;    nop;    nop

LD_G.AUTO_INC S2, AD1, INC_1; NOP; NOP; NOP
LD_G.AUTO_INC S2, AD1, INC_1, SP3; NOP; NOP; NOP
LD_G.AUTO_INC S2, AD1, INC_1, !SP3; NOP; NOP; NOP
// CHECK: ld_g inc_1 S2, AD1;     nop;    nop;    nop
// CHECK: ld_g inc_1 S2, AD1, SP3;     nop;    nop;    nop
// CHECK: ld_g inc_1 S2, AD1, !SP3;    nop;    nop;    nop

LD_G.AUTO_INC S2, AD1, INC_2; NOP; NOP; NOP
LD_G.AUTO_INC S2, AD1, INC_2, SP3; NOP; NOP; NOP
LD_G.AUTO_INC S2, AD1, INC_2, !SP3; NOP; NOP; NOP
// CHECK: ld_g inc_2 S2, AD1;   nop;    nop;    nop
// CHECK: ld_g inc_2 S2, AD1, SP3;   nop;    nop;    nop
// CHECK: ld_g inc_2 S2, AD1, !SP3;  nop;    nop;    nop

LD_G.AUTO_INC S2, AD1, INC_4; NOP; NOP; NOP
LD_G.AUTO_INC S2, AD1, INC_4, SP3; NOP; NOP; NOP
LD_G.AUTO_INC S2, AD1, INC_4, !SP3; NOP; NOP; NOP
// CHECK: ld_g inc_4 S2, AD1;   nop;    nop;    nop
// CHECK: ld_g inc_4 S2, AD1, SP3;   nop;    nop;    nop
// CHECK: ld_g inc_4 S2, AD1, !SP3;  nop;    nop;    nop

LD_G.AUTO_INC S2, AD1, INC_8; NOP; NOP; NOP
LD_G.AUTO_INC S2, AD1, INC_8, SP3; NOP; NOP; NOP
LD_G.AUTO_INC S2, AD1, INC_8, !SP3; NOP; NOP; NOP
// CHECK: ld_g inc_8 S2, AD1;   nop;    nop;    nop
// CHECK: ld_g inc_8 S2, AD1, SP3;   nop;    nop;    nop
// CHECK: ld_g inc_8 S2, AD1, !SP3;  nop;    nop;    nop

LD_G.AUTO_INC.EV_HINT S2, AD1; NOP; NOP; NOP
LD_G.AUTO_INC.EV_HINT S2, AD1, SP3; NOP; NOP; NOP
LD_G.AUTO_INC.EV_HINT S2, AD1, !SP3; NOP; NOP; NOP
// CHECK: ld_g inc_1 ev_hint S2, AD1;     nop;    nop;    nop
// CHECK: ld_g inc_1 ev_hint S2, AD1, SP3;     nop;    nop;    nop
// CHECK: ld_g inc_1 ev_hint S2, AD1, !SP3;    nop;    nop;    nop

LD_G.AUTO_INC.EV_HINT S2, AD1, INC_8; NOP; NOP; NOP
LD_G.AUTO_INC.EV_HINT S2, AD1, INC_8, SP3; NOP; NOP; NOP
LD_G.AUTO_INC.EV_HINT S2, AD1, INC_8, !SP3; NOP; NOP; NOP
// CHECK: ld_g inc_8 ev_hint S2, AD1;   nop;    nop;    nop
// CHECK: ld_g inc_8 ev_hint S2, AD1, SP3;   nop;    nop;    nop
// CHECK: ld_g inc_8 ev_hint S2, AD1, !SP3;  nop;    nop;    nop

LD_G.AUTO_INC.EV_HINT.L0CS S2, AD1, INC_8; NOP; NOP; NOP
LD_G.AUTO_INC.EV_HINT.L0CS S2, AD1, INC_8, SP3; NOP; NOP; NOP
LD_G.AUTO_INC.EV_HINT.L0CS S2, AD1, INC_8, !SP3; NOP; NOP; NOP
// CHECK: ld_g inc_8 l0cs ev_hint S2, AD1;      nop;    nop;    nop
// CHECK: ld_g inc_8 l0cs ev_hint S2, AD1, SP3;      nop;    nop;    nop
// CHECK: ld_g inc_8 l0cs ev_hint S2, AD1, !SP3;     nop;    nop;    nop

LD_G.AUTO_INC b11001 I2, AD1
LD_G.AUTO_INC b11001 I2, AD1, SP3
LD_G.AUTO_INC b11001 I2, AD1, !SP3
// CHECK: ld_g inc_1 b11001 I2, AD1;      nop;    nop;    nop
// CHECK: ld_g inc_1 b11001 I2, AD1, SP3;      nop;    nop;    nop
// CHECK: ld_g inc_1 b11001 I2, AD1, !SP3;     nop;    nop;    nop

LD_G.BV64.AUTO_INC.EV_HINT.L0CS b11001 I2, AD1, INC_8; NOP; NOP; NOP
LD_G.BV64.AUTO_INC.EV_HINT.L0CS b11001 I2, AD1, INC_8, SP3; NOP; NOP; NOP
LD_G.BV64.AUTO_INC.EV_HINT.L0CS b11001 I2, AD1, INC_8, !SP3; NOP; NOP; NOP
// CHECK: ld_g inc_8 bv64 l0cs ev_hint b11001 I2, AD1;  nop;    nop;    nop
// CHECK: ld_g inc_8 bv64 l0cs ev_hint b11001 I2, AD1, SP3;  nop;    nop;    nop
// CHECK: ld_g inc_8 bv64 l0cs ev_hint b11001 I2, AD1, !SP3;         nop;    nop;    nop

LD_G.AUTO_INC M0 I2, AD1
LD_G.AUTO_INC M0 I2, AD1, SP3
LD_G.AUTO_INC M0 I2, AD1, !SP3
// CHECK: ld_g inc_1 M0 I2, AD1;  nop;    nop;    nop
// CHECK: ld_g inc_1 M0 I2, AD1, SP3;  nop;    nop;    nop
// CHECK: ld_g inc_1 M0 I2, AD1, !SP3;         nop;    nop;    nop

LD_G.BV64.AUTO_INC.EV_HINT.L0CS M0 I2, AD1, INC_8; NOP; NOP; NOP
LD_G.BV64.AUTO_INC.EV_HINT.L0CS M0 I2, AD1, INC_8, SP3; NOP; NOP; NOP
LD_G.BV64.AUTO_INC.EV_HINT.L0CS M0 I2, AD1, INC_8, !SP3; NOP; NOP; NOP
// CHECK: ld_g inc_8 bv64 l0cs ev_hint M0 I2, AD1;      nop;    nop;    nop
// CHECK: ld_g inc_8 bv64 l0cs ev_hint M0 I2, AD1, SP3;      nop;    nop;    nop
// CHECK: ld_g inc_8 bv64 l0cs ev_hint M0 I2, AD1, !SP3;     nop;    nop;    nop

LD_G.AUTO_INC V2, AD1; NOP; NOP; NOP
// CHECK: ld_g inc_1  V2, AD1; 	nop; 	nop; 	nop

LD_G.AUTO_INC.PARTIAL.L0CS V2, AD1, LD_PARTIAL_REG; NOP; NOP; NOP
LD_G.AUTO_INC.PARTIAL V2, AD1, INC_8, LD_PARTIAL_REG; NOP; NOP; NOP
LD_G.AUTO_INC.PARTIAL V2, AD1, LD_PARTIAL_REG, SP3; NOP; NOP; NOP
LD_G.PARTIAL V2, AD1, LD_PARTIAL_REG, !SP3; NOP; NOP; NOP
// CHECK: ld_g inc_1 l0cs partial V2, AD1, LD_PARTIAL_REG; 	nop; 	nop; 	nop
// CHECK: ld_g inc_8  partial V2, AD1, LD_PARTIAL_REG; 	nop; 	nop; 	nop
// CHECK: ld_g inc_1  partial V2, AD1, LD_PARTIAL_REG, SP3; 	nop; 	nop; 	nop
// CHECK: ld_g partial V2, AD1, LD_PARTIAL_REG, !SP3; 	nop; 	nop; 	nop

LD_G S2, AD1, INC_2; NOP; NOP; NOP
// CHECK: ld_g inc_2 S2, AD1;   nop;    nop;    nop
