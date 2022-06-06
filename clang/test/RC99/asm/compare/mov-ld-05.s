// RUN: %clang -cc1as -triple tpc-none-none -filetype obj -target-cpu doron1 %s -o %t.o
// RUN: %disasm --mcpu doron1 %t.o | FileCheck %s

MOV ZP_REG, V4
MOV ZP_REG, 1
MOV ZP_REG, 100
MOV ZP_REG, S4
MOV V4, ZP_REG
//CHECK: mov ZP_REG, V4; nop; nop; nop
//CHECK: mov ZP_REG, 0x1; nop; nop; nop
//CHECK: mov ZP_REG, 0x64; nop; nop; nop
//CHECK: mov ZP_REG, S4; nop; nop; nop
//CHECK: mov V4, ZP_REG; nop; nop; nop

MOV V_CARRY, VP4
MOV VP4, V_CARRY
//CHECK: mov V_CARRY, VP4; nop; nop; nop
//CHECK: mov VP4, V_CARRY; nop; nop; nop

MOV VPU_LFSR, V4
MOV VPU_LFSR, 1
MOV VPU_LFSR, 100
MOV VPU_LFSR, S4
MOV V4, VPU_LFSR
//CHECK: mov VPU_LFSR, V4; nop; nop; nop
//CHECK: mov VPU_LFSR, 0x1; nop; nop; nop
//CHECK: mov VPU_LFSR, 0x64; nop; nop; nop
//CHECK: mov VPU_LFSR, S4; nop; nop; nop
//CHECK: mov V4, VPU_LFSR; nop; nop; nop

MOV SPU_LFSR, S4
MOV SPU_LFSR, 1
MOV SPU_LFSR, 100
MOV S4, SPU_LFSR
//CHECK: mov SPU_LFSR, S4; nop; nop; nop
//CHECK: mov SPU_LFSR, 0x1; nop; nop; nop
//CHECK: mov SPU_LFSR, 0x64; nop; nop; nop
//CHECK: mov S4, SPU_LFSR; nop; nop; nop

MOV S4, SPU_LFSR_RO
MOV V4, VPU_LFSR_RO
//CHECK: mov S4, SPU_LFSR_RO; nop; nop; nop
//CHECK: mov V4, VPU_LFSR_RO; nop; nop; nop

MOV V4, LANE_ID_4B
MOV V4, LANE_ID_2B
MOV V4, LANE_ID_1B
//CHECK: mov V4, LANE_ID_4B; nop; nop; nop
//CHECK: mov V4, LANE_ID_2B; nop; nop; nop
//CHECK: mov V4, LANE_ID_1B; nop; nop; nop

MOV S4, THREAD_ID
//CHECK: mov S4, THREAD_ID; nop; nop; nop
