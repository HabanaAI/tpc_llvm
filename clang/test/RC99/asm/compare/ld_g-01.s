// RUN: %clang -cc1as -triple tpc-none-none -filetype obj %s -o %t.o
// RUN: %disasm %t.o | FileCheck %s

LD_G S3, AD1; NOP; NOP; NOP
LD_G S3, AD1, SP4; NOP; NOP; NOP
LD_G S3, AD1, !SP4; NOP; NOP; NOP

// CHECK: ld_g S3, AD1, SP0; nop; nop; nop
// CHECK: ld_g S3, AD1, SP4; nop; nop; nop
// CHECK: ld_g S3, AD1, !SP4; nop; nop; nop

LD_G SP3, AD1; NOP; NOP; NOP
LD_G SP3, AD1, SP4; NOP; NOP; NOP
LD_G SP3, AD1, !SP4; NOP; NOP; NOP

// CHECK: ld_g SP3, AD1, SP0; nop; nop; nop
// CHECK: ld_g SP3, AD1, SP4; nop; nop; nop
// CHECK: ld_g SP3, AD1, !SP4; nop; nop; nop

LD_G V1, AD1; NOP; NOP; NOP
LD_G V1, AD1, SP4; NOP; NOP; NOP
LD_G V1, AD1, !SP4; NOP; NOP; NOP

// CHECK: ld_g V1, AD1, SP0; nop; nop; nop
// CHECK: ld_g V1, AD1, SP4; nop; nop; nop
// CHECK: ld_g V1, AD1, !SP4; nop; nop; nop
