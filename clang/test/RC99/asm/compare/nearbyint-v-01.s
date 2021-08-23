// RUN: %clang -cc1as -triple tpc-none-none -filetype obj %s -o %t.o
// RUN: %disasm %t.o | FileCheck %s
// RUN: %clang -cc1as -triple tpc-none-none -target-cpu gaudi -filetype obj %s -o %t.o
// RUN: %disasm -mcpu=gaudi %t.o | FileCheck %s



NOP; NOP; NEARBYINT.F32 V20, V23; NOP
NOP; NOP; NEARBYINT.F32 V20, V23, SP3; NOP
NOP; NOP; NEARBYINT.F32 V20, V23, !SP3; NOP
NOP; NOP; NEARBYINT.F32 V20, V23, VP3; NOP
NOP; NOP; NEARBYINT.F32 V20, V23, !VP3; NOP

// CHECK: nop;    nop;    nearbyint.f32   V20, V23, SP0;  nop
// CHECK: nop;    nop;    nearbyint.f32   V20, V23, SP3;  nop
// CHECK: nop;    nop;    nearbyint.f32   V20, V23, !SP3;         nop
// CHECK: nop;    nop;    nearbyint.f32   V20, V23, VP3;  nop
// CHECK: nop;    nop;    nearbyint.f32   V20, V23, !VP3;         nop


NOP; NOP; NEARBYINT.F32 V20, V23, RD; NOP
NOP; NOP; NEARBYINT.F32 V20, V23, RD, SP3; NOP
NOP; NOP; NEARBYINT.F32 V20, V23, RD, !SP3; NOP
NOP; NOP; NEARBYINT.F32 V20, V23, RD, VP3; NOP
NOP; NOP; NEARBYINT.F32 V20, V23, RD, !VP3; NOP

// CHECK: nop;    nop;    nearbyint.f32  rd V20, V23, SP0;     nop
// CHECK: nop;    nop;    nearbyint.f32  rd V20, V23, SP3;     nop
// CHECK: nop;    nop;    nearbyint.f32  rd V20, V23, !SP3;    nop
// CHECK: nop;    nop;    nearbyint.f32  rd V20, V23, VP3;     nop
// CHECK: nop;    nop;    nearbyint.f32  rd V20, V23, !VP3;    nop


NOP; NOP; NEARBYINT.F32 V20, V23, RU; NOP
NOP; NOP; NEARBYINT.F32 V20, V23, RU, SP3; NOP
NOP; NOP; NEARBYINT.F32 V20, V23, RU, !SP3; NOP
NOP; NOP; NEARBYINT.F32 V20, V23, RU, VP3; NOP
NOP; NOP; NEARBYINT.F32 V20, V23, RU, !VP3; NOP

// CHECK: nop;    nop;    nearbyint.f32  ru V20, V23, SP0;     nop
// CHECK: nop;    nop;    nearbyint.f32  ru V20, V23, SP3;     nop
// CHECK: nop;    nop;    nearbyint.f32  ru V20, V23, !SP3;    nop
// CHECK: nop;    nop;    nearbyint.f32  ru V20, V23, VP3;     nop
// CHECK: nop;    nop;    nearbyint.f32  ru V20, V23, !VP3;    nop


NOP; NOP; NEARBYINT.F32 V20, V23, RZ; NOP
NOP; NOP; NEARBYINT.F32 V20, V23, RZ, SP3; NOP
NOP; NOP; NEARBYINT.F32 V20, V23, RZ, !SP3; NOP
NOP; NOP; NEARBYINT.F32 V20, V23, RZ, VP3; NOP
NOP; NOP; NEARBYINT.F32 V20, V23, RZ, !VP3; NOP

// CHECK: nop;    nop;    nearbyint.f32  rz V20, V23, SP0;     nop
// CHECK: nop;    nop;    nearbyint.f32  rz V20, V23, SP3;     nop
// CHECK: nop;    nop;    nearbyint.f32  rz V20, V23, !SP3;    nop
// CHECK: nop;    nop;    nearbyint.f32  rz V20, V23, VP3;     nop
// CHECK: nop;    nop;    nearbyint.f32  rz V20, V23, !VP3;    nop


NOP; NOP; NEARBYINT.F32 V20, V23, RHNE; NOP
NOP; NOP; NEARBYINT.F32 V20, V23, RHNE, SP3; NOP
NOP; NOP; NEARBYINT.F32 V20, V23, RHNE, !SP3; NOP
NOP; NOP; NEARBYINT.F32 V20, V23, RHNE, VP3; NOP
NOP; NOP; NEARBYINT.F32 V20, V23, RHNE, !VP3; NOP

// CHECK: nop;    nop;    nearbyint.f32 rhne V20, V23, SP0;    nop
// CHECK: nop;    nop;    nearbyint.f32 rhne V20, V23, SP3;    nop
// CHECK: nop;    nop;    nearbyint.f32 rhne V20, V23, !SP3;   nop
// CHECK: nop;    nop;    nearbyint.f32 rhne V20, V23, VP3;    nop
// CHECK: nop;    nop;    nearbyint.f32 rhne V20, V23, !VP3;   nop