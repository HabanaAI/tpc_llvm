// RUN: %clang -cc1as -triple tpc-none-none -target-cpu goya -filetype obj %s -o %t.o
// RUN: %disasm %t.o | FileCheck %s

NOP; NOT.I32 S1, S2; NOP; NOP
NOP; NOT.I32 S1, S2, SP3; NOP; NOP
NOP; NOT.I32 S1, S2, !SP3; NOP; NOP

// CHECK: nop;    not.i32  S1, S2, SP0;   nop;    nop
// CHECK: nop;    not.i32  S1, S2, SP3;   nop;    nop
// CHECK: nop;    not.i32  S1, S2, !SP3;  nop;    nop


NOP; NOT.I16 S1, S2; NOP; NOP
NOP; NOT.I16 S1, S2, SP3; NOP; NOP
NOP; NOT.I16 S1, S2, !SP3; NOP; NOP

// CHECK: nop;    not.i16  S1, S2, SP0;   nop;    nop
// CHECK: nop;    not.i16  S1, S2, SP3;   nop;    nop
// CHECK: nop;    not.i16  S1, S2, !SP3;  nop;    nop


NOP; NOT.I8 S1, S2; NOP; NOP
NOP; NOT.I8 S1, S2, SP3; NOP; NOP
NOP; NOT.I8 S1, S2, !SP3; NOP; NOP

// CHECK: nop;    not.i8  S1, S2, SP0;    nop;    nop
// CHECK: nop;    not.i8  S1, S2, SP3;    nop;    nop
// CHECK: nop;    not.i8  S1, S2, !SP3;   nop;    nop


NOP; NOT.B SP1, SP2; NOP; NOP
NOP; NOT.B SP1, SP2, SP3; NOP; NOP
NOP; NOT.B SP1, SP2, !SP3; NOP; NOP

// CHECK: nop;    not.b  SP1, SP2, SP0;   nop;    nop
// CHECK: nop;    not.b  SP1, SP2, SP3;   nop;    nop
// CHECK: nop;    not.b  SP1, SP2, !SP3;  nop;    nop


NOP; NOT.I32 b10011 I4, I5; NOP; NOP
NOP; NOT.I32 b10011 I4, I5, SP3; NOP; NOP
NOP; NOT.I32 b10011 I4, I5, !SP3; NOP; NOP

// CHECK: nop;    not.i32  b10011 I4, I5, SP0;    nop;    nop
// CHECK: nop;    not.i32  b10011 I4, I5, SP3;    nop;    nop
// CHECK: nop;    not.i32  b10011 I4, I5, !SP3;   nop;    nop


NOP; NOT.I32 b10011 I4, S6; NOP; NOP
NOP; NOT.I32 b10011 I4, S6, SP3; NOP; NOP
NOP; NOT.I32 b10011 I4, S6, !SP3; NOP; NOP

// CHECK: nop;    not.i32  b10011 I4, S6, SP0;    nop;    nop
// CHECK: nop;    not.i32  b10011 I4, S6, SP3;    nop;    nop
// CHECK: nop;    not.i32  b10011 I4, S6, !SP3;   nop;    nop
