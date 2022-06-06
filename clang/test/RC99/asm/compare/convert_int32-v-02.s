// RUN: %clang -cc1as -triple tpc-none-none -target-cpu gaudi -filetype obj %s -o %t.o
// RUN: %disasm --mcpu gaudi %t.o | FileCheck %s


NOP; NOP; CONVERT_INT32 V6, V12, V8, LANE_SEL=0, RNE;         NOP
NOP; NOP; CONVERT_INT32 V6, V12, V8, LANE_SEL=1, RNE, SP1;    NOP
NOP; NOP; CONVERT_INT32 V6, V12, V8, LANE_SEL=2, RNE, !SP1;   NOP
NOP; NOP; CONVERT_INT32 V6, V12, V8, LANE_SEL=3, RNE, VP1;    NOP
NOP; NOP; CONVERT_INT32 V6, V12, V8, LANE_SEL=2, RNE, !VP1;   NOP

// CHECK: nop;    nop;    convert_int32 lane_sel=0 rhne to_8 V6, V12, V8;        nop
// CHECK: nop;    nop;    convert_int32 lane_sel=1 rhne to_8 V6, V12, V8, SP1;        nop
// CHECK: nop;    nop;    convert_int32 lane_sel=2 rhne to_8 V6, V12, V8, !SP1;       nop
// CHECK: nop;    nop;    convert_int32 lane_sel=3 rhne to_8 V6, V12, V8, VP1;        nop
// CHECK: nop;    nop;    convert_int32 lane_sel=2 rhne to_8 V6, V12, V8, !VP1;       nop

NOP; NOP; CONVERT_INT32 V6, V12, V8, LANE_SEL=0;         NOP
NOP; NOP; CONVERT_INT32 V6, V12, V8, LANE_SEL=1, SP1;    NOP
NOP; NOP; CONVERT_INT32 V6, V12, V8, LANE_SEL=2, !SP1;   NOP
NOP; NOP; CONVERT_INT32 V6, V12, V8, LANE_SEL=3, VP1;    NOP
NOP; NOP; CONVERT_INT32 V6, V12, V8, LANE_SEL=2, !VP1;   NOP

// CHECK: nop;    nop;    convert_int32 lane_sel=0 rhne to_8 V6, V12, V8;        nop
// CHECK: nop;    nop;    convert_int32 lane_sel=1 rhne to_8 V6, V12, V8, SP1;        nop
// CHECK: nop;    nop;    convert_int32 lane_sel=2 rhne to_8 V6, V12, V8, !SP1;       nop
// CHECK: nop;    nop;    convert_int32 lane_sel=3 rhne to_8 V6, V12, V8, VP1;        nop
// CHECK: nop;    nop;    convert_int32 lane_sel=2 rhne to_8 V6, V12, V8, !VP1;       nop


NOP; NOP; CONVERT_INT32 V6, V12, V8, LANE_SEL=0, RNE;   NOP
NOP; NOP; CONVERT_INT32 V6, V12, V8, LANE_SEL=0, RHNE;  NOP
NOP; NOP; CONVERT_INT32 V6, V12, V8, LANE_SEL=0, RZ;    NOP
NOP; NOP; CONVERT_INT32 V6, V12, V8, LANE_SEL=0, RD;    NOP
NOP; NOP; CONVERT_INT32 V6, V12, V8, LANE_SEL=0, RU;    NOP
NOP; NOP; CONVERT_INT32 V6, V12, V8, LANE_SEL=0, SR;    NOP

// CHECK: nop;    nop;    convert_int32 lane_sel=0 rhne to_8 V6, V12, V8;         nop
// CHECK: nop;    nop;    convert_int32 lane_sel=0 rhne to_8 V6, V12, V8;         nop
// CHECK: nop;    nop;    convert_int32 lane_sel=0 rz to_8 V6, V12, V8;   nop
// CHECK: nop;    nop;    convert_int32 lane_sel=0 rd to_8 V6, V12, V8;   nop
// CHECK: nop;    nop;    convert_int32 lane_sel=0 ru to_8 V6, V12, V8;   nop
// CHECK: nop;    nop;    convert_int32 lane_sel=0 sr to_8 V6, V12, V8;   nop


NOP; NOP; CONVERT_INT32 V6, V12, V8, TO_8, LANE_SEL=0, RNE;   NOP
NOP; NOP; CONVERT_INT32 V6, V12, V8, TO_8, LANE_SEL=1, RHNE;  NOP
NOP; NOP; CONVERT_INT32 V6, V12, V8, TO_8, LANE_SEL=2, RD;    NOP
NOP; NOP; CONVERT_INT32 V6, V12, V8, TO_8, LANE_SEL=3, RU;    NOP
NOP; NOP; CONVERT_INT32 V6, V12, V8, TO_8, LANE_SEL=0, SR;    NOP

// CHECK: nop;    nop;    convert_int32 lane_sel=0 rhne to_8 V6, V12, V8;        nop
// CHECK: nop;    nop;    convert_int32 lane_sel=1 rhne to_8 V6, V12, V8;        nop
// CHECK: nop;    nop;    convert_int32 lane_sel=2 rd to_8 V6, V12, V8;   nop
// CHECK: nop;    nop;    convert_int32 lane_sel=3 ru to_8 V6, V12, V8;   nop
// CHECK: nop;    nop;    convert_int32 lane_sel=0 sr to_8 V6, V12, V8;   nop


NOP; NOP; CONVERT_INT32 V6, V12, V8, TO_16, LANE_SEL=0, RNE;   NOP
NOP; NOP; CONVERT_INT32 V6, V12, V8, TO_16, LANE_SEL=1, RHNE;  NOP
NOP; NOP; CONVERT_INT32 V6, V12, V8, TO_16, LANE_SEL=2, RD;    NOP
NOP; NOP; CONVERT_INT32 V6, V12, V8, TO_16, LANE_SEL=3, RU;    NOP
NOP; NOP; CONVERT_INT32 V6, V12, V8, TO_16, LANE_SEL=0, SR;    NOP

// CHECK: nop;    nop;    convert_int32 lane_sel=0 rhne to_16 V6, V12, V8;   nop
// CHECK: nop;    nop;    convert_int32 lane_sel=1 rhne to_16 V6, V12, V8;   nop
// CHECK: nop;    nop;    convert_int32 lane_sel=2 rd to_16 V6, V12, V8;     nop
// CHECK: nop;    nop;    convert_int32 lane_sel=3 ru to_16 V6, V12, V8;     nop
// CHECK: nop;    nop;    convert_int32 lane_sel=0 sr to_16 V6, V12, V8;     nop
