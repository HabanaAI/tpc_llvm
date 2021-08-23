// RUN: %clang -cc1as -triple tpc-none-none -target-cpu goya -filetype obj %s -o %t.o
// RUN: %disasm %t.o | FileCheck %s

NOP; ABS.I32 S1, S2; NOP; NOP
NOP; ABS.I32 S1, S2, SP3; NOP; NOP
NOP; ABS.I32 S1, S2, !SP3; NOP; NOP

// CHECK: nop;    abs.i32  S1, S2, SP0;   nop;    nop
// CHECK: nop;    abs.i32  S1, S2, SP3;   nop;    nop
// CHECK: nop;    abs.i32  S1, S2, !SP3;  nop;    nop


NOP; ABS.I16 S1, S2; NOP; NOP
NOP; ABS.I16 S1, S2, SP3; NOP; NOP
NOP; ABS.I16 S1, S2, !SP3; NOP; NOP

// CHECK: nop;    abs.i16  S1, S2, SP0;   nop;    nop
// CHECK: nop;    abs.i16  S1, S2, SP3;   nop;    nop
// CHECK: nop;    abs.i16  S1, S2, !SP3;  nop;    nop


NOP; ABS.I8 S1, S2; NOP; NOP
NOP; ABS.I8 S1, S2, SP3; NOP; NOP
NOP; ABS.I8 S1, S2, !SP3; NOP; NOP

// CHECK: nop;    abs.i8  S1, S2, SP0;    nop;    nop
// CHECK: nop;    abs.i8  S1, S2, SP3;    nop;    nop
// CHECK: nop;    abs.i8  S1, S2, !SP3;   nop;    nop


NOP; ABS.I32 b10011 I4, I5; NOP; NOP
NOP; ABS.I32 b10011 I4, I5, SP3; NOP; NOP
NOP; ABS.I32 b10011 I4, I5, !SP3; NOP; NOP

// CHECK: nop;    abs.i32  b10011 I4, I5, SP0;    nop;    nop
// CHECK: nop;    abs.i32  b10011 I4, I5, SP3;    nop;    nop
// CHECK: nop;    abs.i32  b10011 I4, I5, !SP3;   nop;    nop


NOP; ABS.I32 b10011 I4, S6; NOP; NOP
NOP; ABS.I32 b10011 I4, S6, SP3; NOP; NOP
NOP; ABS.I32 b10011 I4, S6, !SP3; NOP; NOP

// CHECK: nop;    abs.i32  b10011 I4, S6, SP0;    nop;    nop
// CHECK: nop;    abs.i32  b10011 I4, S6, SP3;    nop;    nop
// CHECK: nop;    abs.i32  b10011 I4, S6, !SP3;   nop;    nop