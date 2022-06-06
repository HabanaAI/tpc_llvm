// RUN: %clang -cc1as -triple tpc-none-none -filetype obj -target-cpu gaudi2 %s -o %t.o
// RUN: %disasm --mcpu gaudi2 %t.o | FileCheck %s

// LOAD
MOV.TO_HW_REG 16, S1
MOV.TO_HW_REG 17, S1
MOV.TO_HW_REG 18, S1
MOV.TO_HW_REG 19, S1
MOV.TO_HW_REG 20, S1
MOV.TO_HW_REG 21, S1
MOV.TO_HW_REG 22, S1
MOV.TO_HW_REG 23, S1
MOV.TO_HW_REG 24, S1
MOV.TO_HW_REG 25, S1
MOV.TO_HW_REG 26, S1
MOV.TO_HW_REG 27, S1
MOV.TO_HW_REG 28, S1
MOV.TO_HW_REG 29, S1
MOV.TO_HW_REG 30, S1
MOV.TO_HW_REG 31, S1

// CHECK: mov  SQZ_CNTR0_LO, S1;   nop;   nop;   nop
// CHECK: mov  SQZ_CNTR0_HI, S1;   nop;   nop;   nop
// CHECK: mov  SQZ_CNTR1_LO, S1;   nop;   nop;   nop
// CHECK: mov  SQZ_CNTR1_HI, S1;   nop;   nop;   nop
// CHECK: mov  SQZ_CNTR2_LO, S1;   nop;   nop;   nop
// CHECK: mov  SQZ_CNTR2_HI, S1;   nop;   nop;   nop
// CHECK: mov  SQZ_CNTR3_LO, S1;   nop;   nop;   nop
// CHECK: mov  SQZ_CNTR3_HI, S1;   nop;   nop;   nop
// CHECK: mov  SQZ_CNTR4_LO, S1;   nop;   nop;   nop
// CHECK: mov  SQZ_CNTR4_HI, S1;   nop;   nop;   nop
// CHECK: mov  SQZ_CNTR5_LO, S1;   nop;   nop;   nop
// CHECK: mov  SQZ_CNTR5_HI, S1;   nop;   nop;   nop
// CHECK: mov  SQZ_CNTR6_LO, S1;   nop;   nop;   nop
// CHECK: mov  SQZ_CNTR6_HI, S1;   nop;   nop;   nop
// CHECK: mov  SQZ_CNTR7_LO, S1;   nop;   nop;   nop
// CHECK: mov  SQZ_CNTR7_HI, S1;   nop;   nop;   nop

MOV SQZ_CNTR0_LO, S1
MOV SQZ_CNTR0_HI, S1
MOV SQZ_CNTR1_LO, S1
MOV SQZ_CNTR1_HI, S1
MOV SQZ_CNTR2_LO, S1
MOV SQZ_CNTR2_HI, S1
MOV SQZ_CNTR3_LO, S1
MOV SQZ_CNTR3_HI, S1
MOV SQZ_CNTR4_LO, S1
MOV SQZ_CNTR4_HI, S1
MOV SQZ_CNTR5_LO, S1
MOV SQZ_CNTR5_HI, S1
MOV SQZ_CNTR6_LO, S1
MOV SQZ_CNTR6_HI, S1
MOV SQZ_CNTR7_LO, S1
MOV SQZ_CNTR7_HI, S1

// CHECK: mov  SQZ_CNTR0_LO, S1;   nop;   nop;   nop
// CHECK: mov  SQZ_CNTR0_HI, S1;   nop;   nop;   nop
// CHECK: mov  SQZ_CNTR1_LO, S1;   nop;   nop;   nop
// CHECK: mov  SQZ_CNTR1_HI, S1;   nop;   nop;   nop
// CHECK: mov  SQZ_CNTR2_LO, S1;   nop;   nop;   nop
// CHECK: mov  SQZ_CNTR2_HI, S1;   nop;   nop;   nop
// CHECK: mov  SQZ_CNTR3_LO, S1;   nop;   nop;   nop
// CHECK: mov  SQZ_CNTR3_HI, S1;   nop;   nop;   nop
// CHECK: mov  SQZ_CNTR4_LO, S1;   nop;   nop;   nop
// CHECK: mov  SQZ_CNTR4_HI, S1;   nop;   nop;   nop
// CHECK: mov  SQZ_CNTR5_LO, S1;   nop;   nop;   nop
// CHECK: mov  SQZ_CNTR5_HI, S1;   nop;   nop;   nop
// CHECK: mov  SQZ_CNTR6_LO, S1;   nop;   nop;   nop
// CHECK: mov  SQZ_CNTR6_HI, S1;   nop;   nop;   nop
// CHECK: mov  SQZ_CNTR7_LO, S1;   nop;   nop;   nop
// CHECK: mov  SQZ_CNTR7_HI, S1;   nop;   nop;   nop

MOV.FROM_HW_REG S1, 16
MOV.FROM_HW_REG S1, 17
MOV.FROM_HW_REG S1, 18
MOV.FROM_HW_REG S1, 19
MOV.FROM_HW_REG S1, 20
MOV.FROM_HW_REG S1, 21
MOV.FROM_HW_REG S1, 22
MOV.FROM_HW_REG S1, 23
MOV.FROM_HW_REG S1, 24
MOV.FROM_HW_REG S1, 25
MOV.FROM_HW_REG S1, 26
MOV.FROM_HW_REG S1, 27
MOV.FROM_HW_REG S1, 28
MOV.FROM_HW_REG S1, 29
MOV.FROM_HW_REG S1, 30
MOV.FROM_HW_REG S1, 31

// CHECK: mov  S1, SQZ_CNTR0_LO;   nop;   nop;   nop
// CHECK: mov  S1, SQZ_CNTR0_HI;   nop;   nop;   nop
// CHECK: mov  S1, SQZ_CNTR1_LO;   nop;   nop;   nop
// CHECK: mov  S1, SQZ_CNTR1_HI;   nop;   nop;   nop
// CHECK: mov  S1, SQZ_CNTR2_LO;   nop;   nop;   nop
// CHECK: mov  S1, SQZ_CNTR2_HI;   nop;   nop;   nop
// CHECK: mov  S1, SQZ_CNTR3_LO;   nop;   nop;   nop
// CHECK: mov  S1, SQZ_CNTR3_HI;   nop;   nop;   nop
// CHECK: mov  S1, SQZ_CNTR4_LO;   nop;   nop;   nop
// CHECK: mov  S1, SQZ_CNTR4_HI;   nop;   nop;   nop
// CHECK: mov  S1, SQZ_CNTR5_LO;   nop;   nop;   nop
// CHECK: mov  S1, SQZ_CNTR5_HI;   nop;   nop;   nop
// CHECK: mov  S1, SQZ_CNTR6_LO;   nop;   nop;   nop
// CHECK: mov  S1, SQZ_CNTR6_HI;   nop;   nop;   nop
// CHECK: mov  S1, SQZ_CNTR7_LO;   nop;   nop;   nop
// CHECK: mov  S1, SQZ_CNTR7_HI; 	 nop; 	nop; 	 nop

MOV  S1, SQZ_CNTR0_LO
MOV  S1, SQZ_CNTR0_HI
MOV  S1, SQZ_CNTR1_LO
MOV  S1, SQZ_CNTR1_HI
MOV  S1, SQZ_CNTR2_LO
MOV  S1, SQZ_CNTR2_HI
MOV  S1, SQZ_CNTR3_LO
MOV  S1, SQZ_CNTR3_HI
MOV  S1, SQZ_CNTR4_LO
MOV  S1, SQZ_CNTR4_HI
MOV  S1, SQZ_CNTR5_LO
MOV  S1, SQZ_CNTR5_HI
MOV  S1, SQZ_CNTR6_LO
MOV  S1, SQZ_CNTR6_HI
MOV  S1, SQZ_CNTR7_LO
MOV  S1, SQZ_CNTR7_HI

// CHECK: mov  S1, SQZ_CNTR0_LO;   nop;   nop;   nop
// CHECK: mov  S1, SQZ_CNTR0_HI;   nop;   nop;   nop
// CHECK: mov  S1, SQZ_CNTR1_LO;   nop;   nop;   nop
// CHECK: mov  S1, SQZ_CNTR1_HI;   nop;   nop;   nop
// CHECK: mov  S1, SQZ_CNTR2_LO;   nop;   nop;   nop
// CHECK: mov  S1, SQZ_CNTR2_HI;   nop;   nop;   nop
// CHECK: mov  S1, SQZ_CNTR3_LO;   nop;   nop;   nop
// CHECK: mov  S1, SQZ_CNTR3_HI;   nop;   nop;   nop
// CHECK: mov  S1, SQZ_CNTR4_LO;   nop;   nop;   nop
// CHECK: mov  S1, SQZ_CNTR4_HI;   nop;   nop;   nop
// CHECK: mov  S1, SQZ_CNTR5_LO;   nop;   nop;   nop
// CHECK: mov  S1, SQZ_CNTR5_HI;   nop;   nop;   nop
// CHECK: mov  S1, SQZ_CNTR6_LO;   nop;   nop;   nop
// CHECK: mov  S1, SQZ_CNTR6_HI;   nop;   nop;   nop
// CHECK: mov  S1, SQZ_CNTR7_LO;   nop;   nop;   nop
// CHECK: mov  S1, SQZ_CNTR7_HI;   nop;   nop;   nop

// SPU
NOP; MOV.TO_HW_REG 16, S1
NOP; MOV.TO_HW_REG 17, S1
NOP; MOV.TO_HW_REG 18, S1
NOP; MOV.TO_HW_REG 19, S1
NOP; MOV.TO_HW_REG 20, S1
NOP; MOV.TO_HW_REG 21, S1
NOP; MOV.TO_HW_REG 22, S1
NOP; MOV.TO_HW_REG 23, S1
NOP; MOV.TO_HW_REG 24, S1
NOP; MOV.TO_HW_REG 25, S1
NOP; MOV.TO_HW_REG 26, S1
NOP; MOV.TO_HW_REG 27, S1
NOP; MOV.TO_HW_REG 28, S1
NOP; MOV.TO_HW_REG 29, S1
NOP; MOV.TO_HW_REG 30, S1
NOP; MOV.TO_HW_REG 31, S1

// CHECK: nop;   mov SQZ_CNTR0_LO, S1;   nop;   nop
// CHECK: nop;   mov SQZ_CNTR0_HI, S1;   nop;   nop
// CHECK: nop;   mov SQZ_CNTR1_LO, S1;   nop;   nop
// CHECK: nop;   mov SQZ_CNTR1_HI, S1;   nop;   nop
// CHECK: nop;   mov SQZ_CNTR2_LO, S1;   nop;   nop
// CHECK: nop;   mov SQZ_CNTR2_HI, S1;   nop;   nop
// CHECK: nop;   mov SQZ_CNTR3_LO, S1;   nop;   nop
// CHECK: nop;   mov SQZ_CNTR3_HI, S1;   nop;   nop
// CHECK: nop;   mov SQZ_CNTR4_LO, S1;   nop;   nop
// CHECK: nop;   mov SQZ_CNTR4_HI, S1;   nop;   nop
// CHECK: nop;   mov SQZ_CNTR5_LO, S1;   nop;   nop
// CHECK: nop;   mov SQZ_CNTR5_HI, S1;   nop;   nop
// CHECK: nop;   mov SQZ_CNTR6_LO, S1;   nop;   nop
// CHECK: nop;   mov SQZ_CNTR6_HI, S1;   nop;   nop
// CHECK: nop;   mov SQZ_CNTR7_LO, S1;   nop;   nop
// CHECK: nop;   mov SQZ_CNTR7_HI, S1;   nop;   nop

NOP; MOV SQZ_CNTR0_LO, S1
NOP; MOV SQZ_CNTR0_HI, S1
NOP; MOV SQZ_CNTR1_LO, S1
NOP; MOV SQZ_CNTR1_HI, S1
NOP; MOV SQZ_CNTR2_LO, S1
NOP; MOV SQZ_CNTR2_HI, S1
NOP; MOV SQZ_CNTR3_LO, S1
NOP; MOV SQZ_CNTR3_HI, S1
NOP; MOV SQZ_CNTR4_LO, S1
NOP; MOV SQZ_CNTR4_HI, S1
NOP; MOV SQZ_CNTR5_LO, S1
NOP; MOV SQZ_CNTR5_HI, S1
NOP; MOV SQZ_CNTR6_LO, S1
NOP; MOV SQZ_CNTR6_HI, S1
NOP; MOV SQZ_CNTR7_LO, S1
NOP; MOV SQZ_CNTR7_HI, S1

// CHECK: nop;   mov SQZ_CNTR0_LO, S1;   nop;   nop
// CHECK: nop;   mov SQZ_CNTR0_HI, S1;   nop;   nop
// CHECK: nop;   mov SQZ_CNTR1_LO, S1;   nop;   nop
// CHECK: nop;   mov SQZ_CNTR1_HI, S1;   nop;   nop
// CHECK: nop;   mov SQZ_CNTR2_LO, S1;   nop;   nop
// CHECK: nop;   mov SQZ_CNTR2_HI, S1;   nop;   nop
// CHECK: nop;   mov SQZ_CNTR3_LO, S1;   nop;   nop
// CHECK: nop;   mov SQZ_CNTR3_HI, S1;   nop;   nop
// CHECK: nop;   mov SQZ_CNTR4_LO, S1;   nop;   nop
// CHECK: nop;   mov SQZ_CNTR4_HI, S1;   nop;   nop
// CHECK: nop;   mov SQZ_CNTR5_LO, S1;   nop;   nop
// CHECK: nop;   mov SQZ_CNTR5_HI, S1;   nop;   nop
// CHECK: nop;   mov SQZ_CNTR6_LO, S1;   nop;   nop
// CHECK: nop;   mov SQZ_CNTR6_HI, S1;   nop;   nop
// CHECK: nop;   mov SQZ_CNTR7_LO, S1;   nop;   nop
// CHECK: nop;   mov SQZ_CNTR7_HI, S1;   nop;   nop

NOP; MOV.FROM_HW_REG S1, 16
NOP; MOV.FROM_HW_REG S1, 17
NOP; MOV.FROM_HW_REG S1, 18
NOP; MOV.FROM_HW_REG S1, 19
NOP; MOV.FROM_HW_REG S1, 20
NOP; MOV.FROM_HW_REG S1, 21
NOP; MOV.FROM_HW_REG S1, 22
NOP; MOV.FROM_HW_REG S1, 23
NOP; MOV.FROM_HW_REG S1, 24
NOP; MOV.FROM_HW_REG S1, 25
NOP; MOV.FROM_HW_REG S1, 26
NOP; MOV.FROM_HW_REG S1, 27
NOP; MOV.FROM_HW_REG S1, 28
NOP; MOV.FROM_HW_REG S1, 29
NOP; MOV.FROM_HW_REG S1, 30
NOP; MOV.FROM_HW_REG S1, 31

// CHECK: nop;   mov S1, SQZ_CNTR0_LO;   nop;   nop
// CHECK: nop;   mov S1, SQZ_CNTR0_HI;   nop;   nop
// CHECK: nop;   mov S1, SQZ_CNTR1_LO;   nop;   nop
// CHECK: nop;   mov S1, SQZ_CNTR1_HI;   nop;   nop
// CHECK: nop;   mov S1, SQZ_CNTR2_LO;   nop;   nop
// CHECK: nop;   mov S1, SQZ_CNTR2_HI;   nop;   nop
// CHECK: nop;   mov S1, SQZ_CNTR3_LO;   nop;   nop
// CHECK: nop;   mov S1, SQZ_CNTR3_HI;   nop;   nop
// CHECK: nop;   mov S1, SQZ_CNTR4_LO;   nop;   nop
// CHECK: nop;   mov S1, SQZ_CNTR4_HI;   nop;   nop
// CHECK: nop;   mov S1, SQZ_CNTR5_LO;   nop;   nop
// CHECK: nop;   mov S1, SQZ_CNTR5_HI;   nop;   nop
// CHECK: nop;   mov S1, SQZ_CNTR6_LO;   nop;   nop
// CHECK: nop;   mov S1, SQZ_CNTR6_HI;   nop;   nop
// CHECK: nop;   mov S1, SQZ_CNTR7_LO;   nop;   nop
// CHECK: nop;   mov S1, SQZ_CNTR7_HI;   nop;   nop

NOP; MOV  S1, SQZ_CNTR0_LO
NOP; MOV  S1, SQZ_CNTR0_HI
NOP; MOV  S1, SQZ_CNTR1_LO
NOP; MOV  S1, SQZ_CNTR1_HI
NOP; MOV  S1, SQZ_CNTR2_LO
NOP; MOV  S1, SQZ_CNTR2_HI
NOP; MOV  S1, SQZ_CNTR3_LO
NOP; MOV  S1, SQZ_CNTR3_HI
NOP; MOV  S1, SQZ_CNTR4_LO
NOP; MOV  S1, SQZ_CNTR4_HI
NOP; MOV  S1, SQZ_CNTR5_LO
NOP; MOV  S1, SQZ_CNTR5_HI
NOP; MOV  S1, SQZ_CNTR6_LO
NOP; MOV  S1, SQZ_CNTR6_HI
NOP; MOV  S1, SQZ_CNTR7_LO
NOP; MOV  S1, SQZ_CNTR7_HI

// CHECK: nop;   mov S1, SQZ_CNTR0_LO;   nop;   nop
// CHECK: nop;   mov S1, SQZ_CNTR0_HI;   nop;   nop
// CHECK: nop;   mov S1, SQZ_CNTR1_LO;   nop;   nop
// CHECK: nop;   mov S1, SQZ_CNTR1_HI;   nop;   nop
// CHECK: nop;   mov S1, SQZ_CNTR2_LO;   nop;   nop
// CHECK: nop;   mov S1, SQZ_CNTR2_HI;   nop;   nop
// CHECK: nop;   mov S1, SQZ_CNTR3_LO;   nop;   nop
// CHECK: nop;   mov S1, SQZ_CNTR3_HI;   nop;   nop
// CHECK: nop;   mov S1, SQZ_CNTR4_LO;   nop;   nop
// CHECK: nop;   mov S1, SQZ_CNTR4_HI;   nop;   nop
// CHECK: nop;   mov S1, SQZ_CNTR5_LO;   nop;   nop
// CHECK: nop;   mov S1, SQZ_CNTR5_HI;   nop;   nop
// CHECK: nop;   mov S1, SQZ_CNTR6_LO;   nop;   nop
// CHECK: nop;   mov S1, SQZ_CNTR6_HI;   nop;   nop
// CHECK: nop;   mov S1, SQZ_CNTR7_LO;   nop;   nop
// CHECK: nop;   mov S1, SQZ_CNTR7_HI;   nop;   nop
