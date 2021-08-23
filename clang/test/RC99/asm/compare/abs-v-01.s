// RUN: %clang -cc1as -triple tpc-none-none -target-cpu goya -filetype obj %s -o %t.o
// RUN: %disasm %t.o | FileCheck %s

NOP; NOP; ABS.I32 V1, V2; NOP
NOP; NOP; ABS.I32 V1, V2, SP3; NOP
NOP; NOP; ABS.I32 V1, V2, !SP3; NOP
NOP; NOP; ABS.I32 V1, V2, VP3; NOP
NOP; NOP; ABS.I32 V1, V2, !VP3; NOP

// CHECK: nop;    nop;    abs.i32  V1, V2, SP0;   nop
// CHECK: nop;    nop;    abs.i32  V1, V2, SP3;   nop
// CHECK: nop;    nop;    abs.i32  V1, V2, !SP3;  nop
// CHECK: nop;    nop;    abs.i32  V1, V2, VP3;   nop
// CHECK: nop;    nop;    abs.i32  V1, V2, !VP3;  nop


NOP; NOP; ABS.I16 V1, V2; NOP
NOP; NOP; ABS.I16 V1, V2, SP3; NOP
NOP; NOP; ABS.I16 V1, V2, !SP3; NOP
NOP; NOP; ABS.I16 V1, V2, VP3; NOP
NOP; NOP; ABS.I16 V1, V2, !VP3; NOP

// CHECK: nop;    nop;    abs.i16  V1, V2, SP0;   nop
// CHECK: nop;    nop;    abs.i16  V1, V2, SP3;   nop
// CHECK: nop;    nop;    abs.i16  V1, V2, !SP3;  nop
// CHECK: nop;    nop;    abs.i16  V1, V2, VP3;   nop
// CHECK: nop;    nop;    abs.i16  V1, V2, !VP3;  nop


NOP; NOP; ABS.I8 V1, V2; NOP
NOP; NOP; ABS.I8 V1, V2, SP3; NOP
NOP; NOP; ABS.I8 V1, V2, !SP3; NOP
NOP; NOP; ABS.I8 V1, V2, VP3; NOP
NOP; NOP; ABS.I8 V1, V2, !VP3; NOP

// CHECK: nop;    nop;    abs.i8  V1, V2, SP0;    nop
// CHECK: nop;    nop;    abs.i8  V1, V2, SP3;    nop
// CHECK: nop;    nop;    abs.i8  V1, V2, !SP3;   nop
// CHECK: nop;    nop;    abs.i8  V1, V2, VP3;    nop
// CHECK: nop;    nop;    abs.i8  V1, V2, !VP3;   nop
