// RUN: %clang -cc1as -triple tpc-none-none -target-cpu goya -filetype obj %s -o %t.o
// RUN: %disasm %t.o | FileCheck %s

NOP; NOP; NOT.I32 V1, V2; NOP
NOP; NOP; NOT.I32 V1, V2, SP3; NOP
NOP; NOP; NOT.I32 V1, V2, !SP3; NOP
NOP; NOP; NOT.I32 V1, V2, VP3; NOP
NOP; NOP; NOT.I32 V1, V2, !VP3; NOP

// CHECK: nop;    nop;    not.i32 V1, V2;    nop
// CHECK: nop;    nop;    not.i32 V1, V2, SP3;    nop
// CHECK: nop;    nop;    not.i32 V1, V2, !SP3;   nop
// CHECK: nop;    nop;    not.i32 V1, V2, VP3;    nop
// CHECK: nop;    nop;    not.i32 V1, V2, !VP3;   nop


NOP; NOP; NOT.I16 V1, V2; NOP
NOP; NOP; NOT.I16 V1, V2, SP3; NOP
NOP; NOP; NOT.I16 V1, V2, !SP3; NOP
NOP; NOP; NOT.I16 V1, V2, VP3; NOP
NOP; NOP; NOT.I16 V1, V2, !VP3; NOP

// CHECK: nop;    nop;    not.i16 V1, V2;    nop
// CHECK: nop;    nop;    not.i16 V1, V2, SP3;    nop
// CHECK: nop;    nop;    not.i16 V1, V2, !SP3;   nop
// CHECK: nop;    nop;    not.i16 V1, V2, VP3;    nop
// CHECK: nop;    nop;    not.i16 V1, V2, !VP3;   nop


NOP; NOP; NOT.I8 V1, V2; NOP
NOP; NOP; NOT.I8 V1, V2, SP3; NOP
NOP; NOP; NOT.I8 V1, V2, !SP3; NOP
NOP; NOP; NOT.I8 V1, V2, VP3; NOP
NOP; NOP; NOT.I8 V1, V2, !VP3; NOP

// CHECK: nop;    nop;    not.i8 V1, V2;     nop
// CHECK: nop;    nop;    not.i8 V1, V2, SP3;     nop
// CHECK: nop;    nop;    not.i8 V1, V2, !SP3;    nop
// CHECK: nop;    nop;    not.i8 V1, V2, VP3;     nop
// CHECK: nop;    nop;    not.i8 V1, V2, !VP3;    nop


NOP; NOP; NOT.B VP1, VP2; NOP
NOP; NOP; NOT.B VP1, VP2, SP3; NOP
NOP; NOP; NOT.B VP1, VP2, !SP3; NOP
NOP; NOP; NOT.B VP1, VP2, VP3; NOP
NOP; NOP; NOT.B VP1, VP2, !VP3; NOP

// CHECK: nop;    nop;    not.b VP1, VP2;    nop
// CHECK: nop;    nop;    not.b VP1, VP2, SP3;    nop
// CHECK: nop;    nop;    not.b VP1, VP2, !SP3;   nop
// CHECK: nop;    nop;    not.b VP1, VP2, VP3;    nop
// CHECK: nop;    nop;    not.b VP1, VP2, !VP3;   nop
