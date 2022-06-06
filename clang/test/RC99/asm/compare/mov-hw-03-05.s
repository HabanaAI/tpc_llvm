// RUN: %clang -cc1as -triple tpc-none-none -filetype obj -target-cpu goya2 %s -o %t.o
// RUN: %disasm --mcpu goya2 %t.o | FileCheck %s
// RUN: %clang -cc1as -triple tpc-none-none -filetype obj -target-cpu gaudi2 %s -o %t.o
// RUN: %disasm --mcpu gaudi2 %t.o | FileCheck %s
// RUN: %clang -cc1as -triple tpc-none-none -filetype obj -target-cpu doron1 %s -o %t.o
// RUN: %disasm --mcpu doron1 %t.o | FileCheck %s

// LOAD
MOV.TO_HW_REG 2, SP1, SP1
MOV.TO_HW_REG 2, SP1
MOV.TO_HW_REG 3, VP1, SP1
MOV.TO_HW_REG 3, VP1, VP1
MOV.TO_HW_REG 3, VP1
MOV.TO_HW_REG 4, S1
MOV.TO_HW_REG 5, S1
MOV.TO_HW_REG 6, S1
MOV.TO_HW_REG 7, S1
MOV.TO_HW_REG 8, S1
MOV.TO_HW_REG 9, S1
MOV.TO_HW_REG 10, S1
MOV.TO_HW_REG 11, S1
MOV.TO_HW_REG 12, S1
MOV.TO_HW_REG 13, V1

// CHECK: mov S_CARRY, SP1, SP1;   nop;   nop;   nop
// CHECK: mov S_CARRY, SP1;   nop;   nop;   nop
// CHECK: mov V_CARRY, VP1, SP1;   nop;   nop;   nop
// CHECK: mov V_CARRY, VP1, VP1;   nop;   nop;   nop
// CHECK: mov V_CARRY, VP1;   nop;   nop;   nop
// CHECK: mov M0, S1;   nop;   nop;   nop
// CHECK: mov M1, S1;   nop;   nop;   nop
// CHECK: mov M2, S1;   nop;   nop;   nop
// CHECK: mov M3, S1;   nop;   nop;   nop
// CHECK: mov LD_TNSR_ID_REG, S1;   nop;   nop;   nop
// CHECK: mov ST_TNSR_ID_REG, S1;   nop;   nop;   nop
// CHECK: mov ST_RMW_REG, S1;   nop;   nop;   nop
// CHECK: mov LD_PARTIAL_REG, S1;   nop;   nop;   nop
// CHECK: mov ST_PARTIAL_REG, S1;   nop;   nop;   nop
// CHECK: mov ZP_REG, V1;   nop;   nop;   nop

MOV S_CARRY, SP1, SP1
MOV S_CARRY, SP1
MOV V_CARRY, VP1, SP1
MOV V_CARRY, VP1, VP1
MOV V_CARRY, VP1
MOV M0, S1
MOV M1, S1
MOV M2, S1
MOV M3, S1
MOV LD_TNSR_ID_REG, S1
MOV ST_TNSR_ID_REG, S1
MOV ST_RMW_REG, S1
MOV LD_PARTIAL_REG, S1
MOV ST_PARTIAL_REG, S1
MOV ZP_REG, V1

// CHECK: mov S_CARRY, SP1, SP1;   nop;   nop;   nop
// CHECK: mov S_CARRY, SP1;   nop;   nop;   nop
// CHECK: mov V_CARRY, VP1, SP1;   nop;   nop;   nop
// CHECK: mov V_CARRY, VP1, VP1;   nop;   nop;   nop
// CHECK: mov V_CARRY, VP1;   nop;   nop;   nop
// CHECK: mov M0, S1;   nop;   nop;   nop
// CHECK: mov M1, S1;   nop;   nop;   nop
// CHECK: mov M2, S1;   nop;   nop;   nop
// CHECK: mov M3, S1;   nop;   nop;   nop
// CHECK: mov LD_TNSR_ID_REG, S1;   nop;   nop;   nop
// CHECK: mov ST_TNSR_ID_REG, S1;   nop;   nop;   nop
// CHECK: mov ST_RMW_REG, S1;   nop;   nop;   nop
// CHECK: mov LD_PARTIAL_REG, S1;   nop;   nop;   nop
// CHECK: mov ST_PARTIAL_REG, S1;   nop;   nop;   nop
// CHECK: mov ZP_REG, V1;   nop;   nop;   nop

MOV.FROM_HW_REG S1, 0, SP1
MOV.FROM_HW_REG S1, 0
MOV.FROM_HW_REG SP1, 2
MOV.FROM_HW_REG VP1, 3, SP1
MOV.FROM_HW_REG VP1, 3, VP1
MOV.FROM_HW_REG VP1, 3
MOV.FROM_HW_REG S1, 4
MOV.FROM_HW_REG S1, 5
MOV.FROM_HW_REG S1, 6
MOV.FROM_HW_REG S1, 7
MOV.FROM_HW_REG S1, 8
MOV.FROM_HW_REG S1, 9
MOV.FROM_HW_REG S1, 10
MOV.FROM_HW_REG S1, 11
MOV.FROM_HW_REG S1, 12
MOV.FROM_HW_REG V1, 13

// CHECK: mov S1, PC, SP1;   nop;   nop;   nop
// CHECK: mov S1, PC;   nop;   nop;   nop
// CHECK: mov SP1, S_CARRY;   nop;   nop;   nop
// CHECK: mov VP1, V_CARRY, SP1;   nop;   nop;   nop
// CHECK: mov VP1, V_CARRY, VP1;   nop;   nop;   nop
// CHECK: mov VP1, V_CARRY;   nop;   nop;   nop
// CHECK: mov S1, M0;   nop;   nop;   nop
// CHECK: mov S1, M1;   nop;   nop;   nop
// CHECK: mov S1, M2;   nop;   nop;   nop
// CHECK: mov S1, M3;   nop;   nop;   nop
// CHECK: mov S1, LD_TNSR_ID_REG;   nop;   nop;   nop
// CHECK: mov S1, ST_TNSR_ID_REG;   nop;   nop;   nop
// CHECK: mov S1, ST_RMW_REG;   nop;   nop;   nop
// CHECK: mov S1, LD_PARTIAL_REG;   nop;   nop;   nop
// CHECK: mov S1, ST_PARTIAL_REG;   nop;   nop;   nop
// CHECK: mov V1, ZP_REG;   nop;   nop;   nop

MOV S1, PC, SP1
MOV S1, PC
MOV SP1, S_CARRY
MOV VP1, V_CARRY, SP1
MOV VP1, V_CARRY, VP1
MOV VP1, V_CARRY
MOV S1, M0
MOV S1, M1
MOV S1, M2
MOV S1, M3
MOV S1, LD_TNSR_ID_REG
MOV S1, ST_TNSR_ID_REG
MOV S1, ST_RMW_REG
MOV S1, LD_PARTIAL_REG
MOV S1, ST_PARTIAL_REG
MOV V1, ZP_REG

// CHECK: mov S1, PC, SP1;   nop;   nop;   nop
// CHECK: mov S1, PC;   nop;   nop;   nop
// CHECK: mov SP1, S_CARRY;   nop;   nop;   nop
// CHECK: mov VP1, V_CARRY, SP1;   nop;   nop;   nop
// CHECK: mov VP1, V_CARRY, VP1;   nop;   nop;   nop
// CHECK: mov VP1, V_CARRY;   nop;   nop;   nop
// CHECK: mov S1, M0;   nop;   nop;   nop
// CHECK: mov S1, M1;   nop;   nop;   nop
// CHECK: mov S1, M2;   nop;   nop;   nop
// CHECK: mov S1, M3;   nop;   nop;   nop
// CHECK: mov S1, LD_TNSR_ID_REG;   nop;   nop;   nop
// CHECK: mov S1, ST_TNSR_ID_REG;   nop;   nop;   nop
// CHECK: mov S1, ST_RMW_REG;   nop;   nop;   nop
// CHECK: mov S1, LD_PARTIAL_REG;   nop;   nop;   nop
// CHECK: mov S1, ST_PARTIAL_REG;   nop;   nop;   nop
// CHECK: mov V1, ZP_REG;   nop;   nop;   nop

// SPU
NOP; MOV.TO_HW_REG 2, SP1, SP1
NOP; MOV.TO_HW_REG 2, SP1
NOP; MOV.TO_HW_REG 4, S1
NOP; MOV.TO_HW_REG 5, S1
NOP; MOV.TO_HW_REG 6, S1
NOP; MOV.TO_HW_REG 7, S1
NOP; MOV.TO_HW_REG 8, S1
NOP; MOV.TO_HW_REG 9, S1
NOP; MOV.TO_HW_REG 10, S1
NOP; MOV.TO_HW_REG 11, S1
NOP; MOV.TO_HW_REG 12, S1

// CHECK: nop;   mov S_CARRY, SP1, SP1;   nop;   nop
// CHECK: nop;   mov S_CARRY, SP1;   nop;   nop
// CHECK: nop;   mov M0, S1;   nop;   nop
// CHECK: nop;   mov M1, S1;   nop;   nop
// CHECK: nop;   mov M2, S1;   nop;   nop
// CHECK: nop;   mov M3, S1;   nop;   nop
// CHECK: nop;   mov LD_TNSR_ID_REG, S1;   nop;   nop
// CHECK: nop;   mov ST_TNSR_ID_REG, S1;   nop;   nop
// CHECK: nop;   mov ST_RMW_REG, S1;   nop;   nop
// CHECK: nop;   mov LD_PARTIAL_REG, S1;   nop;   nop
// CHECK: nop;   mov ST_PARTIAL_REG, S1;   nop;   nop

NOP; MOV S_CARRY, SP1, SP1
NOP; MOV S_CARRY, SP1
NOP; MOV M0, S1
NOP; MOV M1, S1
NOP; MOV M2, S1
NOP; MOV M3, S1
NOP; MOV LD_TNSR_ID_REG, S1
NOP; MOV ST_TNSR_ID_REG, S1
NOP; MOV ST_RMW_REG, S1
NOP; MOV LD_PARTIAL_REG, S1
NOP; MOV ST_PARTIAL_REG, S1

// CHECK: nop;   mov S_CARRY, SP1, SP1;   nop;   nop
// CHECK: nop;   mov S_CARRY, SP1;   nop;   nop
// CHECK: nop;   mov M0, S1;   nop;   nop
// CHECK: nop;   mov M1, S1;   nop;   nop
// CHECK: nop;   mov M2, S1;   nop;   nop
// CHECK: nop;   mov M3, S1;   nop;   nop
// CHECK: nop;   mov LD_TNSR_ID_REG, S1;   nop;   nop
// CHECK: nop;   mov ST_TNSR_ID_REG, S1;   nop;   nop
// CHECK: nop;   mov ST_RMW_REG, S1;   nop;   nop
// CHECK: nop;   mov LD_PARTIAL_REG, S1;   nop;   nop
// CHECK: nop;   mov ST_PARTIAL_REG, S1;   nop;   nop

NOP; MOV.FROM_HW_REG S1, 0, SP1
NOP; MOV.FROM_HW_REG S1, 0
NOP; MOV.FROM_HW_REG SP1, 2
NOP; MOV.FROM_HW_REG S1, 4
NOP; MOV.FROM_HW_REG S1, 5
NOP; MOV.FROM_HW_REG S1, 6
NOP; MOV.FROM_HW_REG S1, 7
NOP; MOV.FROM_HW_REG S1, 8
NOP; MOV.FROM_HW_REG S1, 9
NOP; MOV.FROM_HW_REG S1, 10
NOP; MOV.FROM_HW_REG S1, 11
NOP; MOV.FROM_HW_REG S1, 12

// CHECK: nop;   mov S1, PC, SP1;   nop;   nop
// CHECK: nop;   mov S1, PC;   nop;   nop
// CHECK: nop;   mov SP1, S_CARRY;   nop;   nop
// CHECK: nop;   mov S1, M0;   nop;   nop
// CHECK: nop;   mov S1, M1;   nop;   nop
// CHECK: nop;   mov S1, M2;   nop;   nop
// CHECK: nop;   mov S1, M3;   nop;   nop
// CHECK: nop;   mov S1, LD_TNSR_ID_REG;   nop;   nop
// CHECK: nop;   mov S1, ST_TNSR_ID_REG;   nop;   nop
// CHECK: nop;   mov S1, ST_RMW_REG;   nop;   nop
// CHECK: nop;   mov S1, LD_PARTIAL_REG;   nop;   nop
// CHECK: nop;   mov S1, ST_PARTIAL_REG;   nop;   nop

NOP; MOV S1, PC, SP1
NOP; MOV S1, PC
NOP; MOV SP1, S_CARRY
NOP; MOV S1, M0
NOP; MOV S1, M1
NOP; MOV S1, M2
NOP; MOV S1, M3
NOP; MOV S1, LD_TNSR_ID_REG
NOP; MOV S1, ST_TNSR_ID_REG
NOP; MOV S1, ST_RMW_REG
NOP; MOV S1, LD_PARTIAL_REG
NOP; MOV S1, ST_PARTIAL_REG

// CHECK: nop;   mov S1, PC, SP1;   nop;   nop
// CHECK: nop;   mov S1, PC;   nop;   nop
// CHECK: nop;   mov SP1, S_CARRY;   nop;   nop
// CHECK: nop;   mov S1, M0;   nop;   nop
// CHECK: nop;   mov S1, M1;   nop;   nop
// CHECK: nop;   mov S1, M2;   nop;   nop
// CHECK: nop;   mov S1, M3;   nop;   nop
// CHECK: nop;   mov S1, LD_TNSR_ID_REG;   nop;   nop
// CHECK: nop;   mov S1, ST_TNSR_ID_REG;   nop;   nop
// CHECK: nop;   mov S1, ST_RMW_REG;   nop;   nop
// CHECK: nop;   mov S1, LD_PARTIAL_REG;   nop;   nop
// CHECK: nop;   mov S1, ST_PARTIAL_REG;   nop;   nop

// VPU
NOP; NOP; MOV.TO_HW_REG 3, VP1, SP1
NOP; NOP; MOV.TO_HW_REG 3, VP1, VP1
NOP; NOP; MOV.TO_HW_REG 3, VP1
NOP; NOP; MOV.TO_HW_REG 13, V1

// CHECK: nop;   nop;   mov V_CARRY, VP1, SP1;   nop
// CHECK: nop;   nop;   mov V_CARRY, VP1, VP1;   nop
// CHECK: nop;   nop;   mov V_CARRY, VP1;   nop
// CHECK: nop;   nop;   mov ZP_REG, V1;   nop

NOP; NOP; MOV V_CARRY, VP1, SP1
NOP; NOP; MOV V_CARRY, VP1, VP1
NOP; NOP; MOV V_CARRY, VP1
NOP; NOP; MOV ZP_REG, V1

// CHECK: nop;   nop;   mov V_CARRY, VP1, SP1;   nop
// CHECK: nop;   nop;   mov V_CARRY, VP1, VP1;   nop
// CHECK: nop;   nop;   mov V_CARRY, VP1;   nop
// CHECK: nop;   nop;   mov ZP_REG, V1;   nop

NOP; NOP; MOV.FROM_HW_REG VP1, 3, SP1
NOP; NOP; MOV.FROM_HW_REG VP1, 3, VP1
NOP; NOP; MOV.FROM_HW_REG VP1, 3
NOP; NOP; MOV.FROM_HW_REG V1, 13

// CHECK: nop;   nop;   mov VP1, V_CARRY, SP1;   nop
// CHECK: nop;   nop;   mov VP1, V_CARRY, VP1;   nop
// CHECK: nop;   nop;   mov VP1, V_CARRY;   nop
// CHECK: nop;   nop;   mov V1, ZP_REG;   nop

NOP; NOP; MOV VP1, V_CARRY, SP1
NOP; NOP; MOV VP1, V_CARRY, VP1
NOP; NOP; MOV VP1, V_CARRY
NOP; NOP; MOV V1, ZP_REG

// CHECK: nop;   nop;   mov VP1, V_CARRY, SP1;   nop
// CHECK: nop;   nop;   mov VP1, V_CARRY, VP1;   nop
// CHECK: nop;   nop;   mov VP1, V_CARRY;   nop
// CHECK: nop;   nop;   mov V1, ZP_REG;   nop
