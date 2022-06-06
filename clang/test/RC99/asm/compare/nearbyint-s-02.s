// RUN: %clang -cc1as -triple tpc-none-none -target-cpu gaudi -filetype obj %s -o %t.o
// RUN: %disasm --mcpu=gaudi %t.o | FileCheck %s
// RUN: %clang -cc1as -triple tpc-none-none -target-cpu goya2 -filetype obj %s -o %t.o
// RUN: %disasm --mcpu=goya2 %t.o | FileCheck %s


NOP; NEARBYINT.BF16 S0, S1; NOP; NOP
NOP; NEARBYINT.BF16 S0, S1, SP3; NOP; NOP
NOP; NEARBYINT.BF16 S0, S1, !SP3; NOP; NOP

// CHECK: nop;    nearbyint.bf16   S0, S1;    nop;    nop
// CHECK: nop;    nearbyint.bf16   S0, S1, SP3;    nop;    nop
// CHECK: nop;    nearbyint.bf16   S0, S1, !SP3;   nop;    nop


NOP; NEARBYINT.BF16 S0, S1, RD; NOP; NOP
NOP; NEARBYINT.BF16 S0, S1, RD, SP3; NOP; NOP
NOP; NEARBYINT.BF16 S0, S1, RD, !SP3; NOP; NOP

// CHECK: nop;    nearbyint.bf16  rd S0, S1;       nop;    nop
// CHECK: nop;    nearbyint.bf16  rd S0, S1, SP3;       nop;    nop
// CHECK: nop;    nearbyint.bf16  rd S0, S1, !SP3;      nop;    nop


NOP; NEARBYINT.BF16 S0, S1, RU; NOP; NOP
NOP; NEARBYINT.BF16 S0, S1, RU, SP3; NOP; NOP
NOP; NEARBYINT.BF16 S0, S1, RU, !SP3; NOP; NOP

// CHECK: nop;    nearbyint.bf16  ru S0, S1;       nop;    nop
// CHECK: nop;    nearbyint.bf16  ru S0, S1, SP3;       nop;    nop
// CHECK: nop;    nearbyint.bf16  ru S0, S1, !SP3;      nop;    nop


NOP; NEARBYINT.BF16 S0, S1, RZ; NOP; NOP
NOP; NEARBYINT.BF16 S0, S1, RZ, SP3; NOP; NOP
NOP; NEARBYINT.BF16 S0, S1, RZ, !SP3; NOP; NOP

// CHECK: nop;    nearbyint.bf16  rz S0, S1;       nop;    nop
// CHECK: nop;    nearbyint.bf16  rz S0, S1, SP3;       nop;    nop
// CHECK: nop;    nearbyint.bf16  rz S0, S1, !SP3;      nop;    nop


NOP; NEARBYINT.BF16 S0, S1, RHNE; NOP; NOP
NOP; NEARBYINT.BF16 S0, S1, RHNE, SP3; NOP; NOP
NOP; NEARBYINT.BF16 S0, S1, RHNE, !SP3; NOP; NOP

// CHECK: nop;    nearbyint.bf16 rhne S0, S1;      nop;    nop
// CHECK: nop;    nearbyint.bf16 rhne S0, S1, SP3;      nop;    nop
// CHECK: nop;    nearbyint.bf16 rhne S0, S1, !SP3;     nop;    nop