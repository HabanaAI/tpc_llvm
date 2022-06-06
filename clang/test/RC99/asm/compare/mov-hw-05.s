// RUN: %clang -cc1as -triple tpc-none-none -filetype obj -target-cpu doron1 %s -o %t.o
// RUN: %disasm --mcpu doron1 %t.o | FileCheck %s

//LOAD
MOV.TO_HW_REG 14, 3
MOV.TO_HW_REG 14, S1
MOV.TO_HW_REG 15, S1
MOV.TO_HW_REG 16, 3, SP1
MOV.TO_HW_REG 16, 3, VP1
MOV.TO_HW_REG 16, 3
MOV.TO_HW_REG 16, S1
MOV.TO_HW_REG 16, V1
MOV.TO_HW_REG 17, S1

// CHECK: mov INC_LD_DIM_REG, 0x3;   nop;   nop;   nop
// CHECK: mov INC_LD_DIM_REG, S1;   nop;   nop;   nop
// CHECK: mov INC_ST_DIM_REG, S1;   nop;   nop;   nop
// CHECK: mov VPU_LFSR, 0x3, SP1;   nop;   nop;   nop
// CHECK: mov VPU_LFSR, 0x3, VP1;   nop;   nop;   nop
// CHECK: mov VPU_LFSR, 0x3;   nop;   nop;   nop
// CHECK: mov VPU_LFSR, S1;   nop;   nop;   nop
// CHECK: mov VPU_LFSR, V1;   nop;   nop;   nop
// CHECK: mov SPU_LFSR, S1;   nop;   nop;   nop

MOV INC_LD_DIM_REG, 3
MOV INC_LD_DIM_REG, S1
MOV INC_ST_DIM_REG, S1
MOV VPU_LFSR, 3, SP1
MOV VPU_LFSR, 3, VP1
MOV VPU_LFSR, 3
MOV VPU_LFSR, S1
MOV VPU_LFSR, V1
MOV SPU_LFSR, S1

// CHECK: mov INC_LD_DIM_REG, 0x3;   nop;   nop;   nop
// CHECK: mov INC_LD_DIM_REG, S1;   nop;   nop;   nop
// CHECK: mov INC_ST_DIM_REG, S1;   nop;   nop;   nop
// CHECK: mov VPU_LFSR, 0x3, SP1;   nop;   nop;   nop
// CHECK: mov VPU_LFSR, 0x3, VP1;   nop;   nop;   nop
// CHECK: mov VPU_LFSR, 0x3;   nop;   nop;   nop
// CHECK: mov VPU_LFSR, S1;   nop;   nop;   nop
// CHECK: mov VPU_LFSR, V1;   nop;   nop;   nop
// CHECK: mov SPU_LFSR, S1;   nop;   nop;   nop

MOV.FROM_HW_REG S1, 14
MOV.FROM_HW_REG S1, 15
MOV.FROM_HW_REG V1, 16, SP1
MOV.FROM_HW_REG V1, 16, VP1
MOV.FROM_HW_REG V1, 16
MOV.FROM_HW_REG S1, 17
MOV.FROM_HW_REG V1, 18
MOV.FROM_HW_REG S1, 19
MOV.FROM_HW_REG V1, 20
MOV.FROM_HW_REG V1, 21
MOV.FROM_HW_REG V1, 22
MOV.FROM_HW_REG S1, 23

// CHECK: mov S1, INC_LD_DIM_REG;   nop;   nop;   nop
// CHECK: mov S1, INC_ST_DIM_REG;   nop;   nop;   nop
// CHECK: mov V1, VPU_LFSR, SP1;   nop;   nop;   nop
// CHECK: mov V1, VPU_LFSR, VP1;   nop;   nop;   nop
// CHECK: mov V1, VPU_LFSR;   nop;   nop;   nop
// CHECK: mov S1, SPU_LFSR;   nop;   nop;   nop
// CHECK: mov V1, VPU_LFSR_RO;   nop;   nop;   nop
// CHECK: mov S1, SPU_LFSR_RO;   nop;   nop;   nop
// CHECK: mov V1, LANE_ID_4B;   nop;   nop;   nop
// CHECK: mov V1, LANE_ID_2B;   nop;   nop;   nop
// CHECK: mov V1, LANE_ID_1B;   nop;   nop;   nop
// CHECK: mov S1, THREAD_ID;   nop;   nop;   nop

MOV S1, INC_LD_DIM_REG
MOV S1, INC_ST_DIM_REG
MOV V1, VPU_LFSR, SP1
MOV V1, VPU_LFSR, VP1
MOV V1, VPU_LFSR
MOV S1, SPU_LFSR
MOV V1, VPU_LFSR_RO
MOV S1, SPU_LFSR_RO
MOV V1, LANE_ID_4B
MOV V1, LANE_ID_2B
MOV V1, LANE_ID_1B
MOV S1, THREAD_ID

// CHECK: mov S1, INC_LD_DIM_REG;   nop;   nop;   nop
// CHECK: mov S1, INC_ST_DIM_REG;   nop;   nop;   nop
// CHECK: mov V1, VPU_LFSR, SP1;   nop;   nop;   nop
// CHECK: mov V1, VPU_LFSR, VP1;   nop;   nop;   nop
// CHECK: mov V1, VPU_LFSR;   nop;   nop;   nop
// CHECK: mov S1, SPU_LFSR;   nop;   nop;   nop
// CHECK: mov V1, VPU_LFSR_RO;   nop;   nop;   nop
// CHECK: mov S1, SPU_LFSR_RO;   nop;   nop;   nop
// CHECK: mov V1, LANE_ID_4B;   nop;   nop;   nop
// CHECK: mov V1, LANE_ID_2B;   nop;   nop;   nop
// CHECK: mov V1, LANE_ID_1B;   nop;   nop;   nop
// CHECK: mov S1, THREAD_ID;   nop;   nop;   nop

// SPU
NOP; MOV.TO_HW_REG 14, 3, SP1
NOP; MOV.TO_HW_REG 14, 3
NOP; MOV.TO_HW_REG 14, S1
NOP; MOV.TO_HW_REG 15, S1
NOP; MOV.TO_HW_REG 17, S1

// CHECK:nop;   mov INC_LD_DIM_REG, 0x3, SP1;   nop;   nop
// CHECK:nop;   mov INC_LD_DIM_REG, 0x3;   nop;   nop
// CHECK:nop;   mov INC_LD_DIM_REG, S1;   nop;   nop
// CHECK:nop;   mov INC_ST_DIM_REG, S1;   nop;   nop
// CHECK:nop;   mov SPU_LFSR, S1;   nop;   nop

NOP; MOV INC_LD_DIM_REG, 3, SP1
NOP; MOV INC_LD_DIM_REG, 3
NOP; MOV INC_LD_DIM_REG, S1
NOP; MOV INC_ST_DIM_REG, S1
NOP; MOV SPU_LFSR, S1

// CHECK: nop;   mov INC_LD_DIM_REG, 0x3, SP1;   nop;   nop
// CHECK: nop;   mov INC_LD_DIM_REG, 0x3;   nop;   nop
// CHECK: nop;   mov INC_LD_DIM_REG, S1;   nop;   nop
// CHECK: nop;   mov INC_ST_DIM_REG, S1;   nop;   nop
// CHECK: nop;   mov SPU_LFSR, S1;   nop;   nop

NOP; MOV.FROM_HW_REG S1, 14
NOP; MOV.FROM_HW_REG S1, 15
NOP; MOV.FROM_HW_REG S1, 17
NOP; MOV.FROM_HW_REG S1, 19
NOP; MOV.FROM_HW_REG S1, 23

// CHECK: nop;   mov S1, INC_LD_DIM_REG;   nop;   nop
// CHECK: nop;   mov S1, INC_ST_DIM_REG;   nop;   nop
// CHECK: nop;   mov S1, SPU_LFSR;   nop;   nop
// CHECK: nop;   mov S1, SPU_LFSR_RO;   nop;   nop
// CHECK: nop;   mov S1, THREAD_ID;   nop;   nop

NOP; MOV S1, INC_LD_DIM_REG
NOP; MOV S1, INC_ST_DIM_REG
NOP; MOV S1, SPU_LFSR
NOP; MOV S1, SPU_LFSR_RO
NOP; MOV S1, THREAD_ID

// CHECK: nop;   mov S1, INC_LD_DIM_REG;   nop;   nop
// CHECK: nop;   mov S1, INC_ST_DIM_REG;   nop;   nop
// CHECK: nop;   mov S1, SPU_LFSR;   nop;   nop
// CHECK: nop;   mov S1, SPU_LFSR_RO;   nop;   nop
// CHECK: nop;   mov S1, THREAD_ID;   nop;   nop

// VPU
NOP; NOP; MOV.TO_HW_REG 16, 3, SP1
NOP; NOP; MOV.TO_HW_REG 16, 3, VP1
NOP; NOP; MOV.TO_HW_REG 16, 3
NOP; NOP; MOV.TO_HW_REG 16, S1
NOP; NOP; MOV.TO_HW_REG 16, V1

// CHECK: nop;   nop;   mov VPU_LFSR, 0x3, SP1;   nop
// CHECK: nop;   nop;   mov VPU_LFSR, 0x3, VP1;   nop
// CHECK: nop;   nop;   mov VPU_LFSR, 0x3;   nop
// CHECK: nop;   nop;   mov VPU_LFSR, S1;   nop
// CHECK: nop;   nop;   mov VPU_LFSR, V1;   nop

NOP; NOP; MOV VPU_LFSR, 3, SP1
NOP; NOP; MOV VPU_LFSR, 3, VP1
NOP; NOP; MOV VPU_LFSR, 3
NOP; NOP; MOV VPU_LFSR, S1
NOP; NOP; MOV VPU_LFSR, V1

// CHECK: nop;   nop;   mov VPU_LFSR, 0x3, SP1;   nop
// CHECK: nop;   nop;   mov VPU_LFSR, 0x3, VP1;   nop
// CHECK: nop;   nop;   mov VPU_LFSR, 0x3;   nop
// CHECK: nop;   nop;   mov VPU_LFSR, S1;   nop
// CHECK: nop;   nop;   mov VPU_LFSR, V1;   nop

NOP; NOP; MOV.FROM_HW_REG V1, 16, SP1
NOP; NOP; MOV.FROM_HW_REG V1, 16, VP1
NOP; NOP; MOV.FROM_HW_REG V1, 16
NOP; NOP; MOV.FROM_HW_REG V1, 18
NOP; NOP; MOV.FROM_HW_REG V1, 20
NOP; NOP; MOV.FROM_HW_REG V1, 21
NOP; NOP; MOV.FROM_HW_REG V1, 22

// CHECK: nop;   nop;   mov V1, VPU_LFSR, SP1;   nop
// CHECK: nop;   nop;   mov V1, VPU_LFSR, VP1;   nop
// CHECK: nop;   nop;   mov V1, VPU_LFSR;   nop
// CHECK: nop;   nop;   mov V1, VPU_LFSR_RO;   nop
// CHECK: nop;   nop;   mov V1, LANE_ID_4B;   nop
// CHECK: nop;   nop;   mov V1, LANE_ID_2B;   nop
// CHECK: nop;   nop;   mov V1, LANE_ID_1B;   nop

NOP; NOP; MOV V1, VPU_LFSR, SP1
NOP; NOP; MOV V1, VPU_LFSR, VP1
NOP; NOP; MOV V1, VPU_LFSR
NOP; NOP; MOV V1, VPU_LFSR_RO
NOP; NOP; MOV V1, LANE_ID_4B
NOP; NOP; MOV V1, LANE_ID_2B
NOP; NOP; MOV V1, LANE_ID_1B

// CHECK: nop;   nop;   mov V1, VPU_LFSR, SP1;   nop
// CHECK: nop;   nop;   mov V1, VPU_LFSR, VP1;   nop
// CHECK: nop;   nop;   mov V1, VPU_LFSR;   nop
// CHECK: nop;   nop;   mov V1, VPU_LFSR_RO;   nop
// CHECK: nop;   nop;   mov V1, LANE_ID_4B;   nop
// CHECK: nop;   nop;   mov V1, LANE_ID_2B;   nop
// CHECK: nop;   nop;   mov V1, LANE_ID_1B;   nop
