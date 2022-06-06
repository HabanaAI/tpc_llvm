// RUN: %clang -cc1as -triple tpc-none-none -target-cpu gaudi -filetype obj %s -o %t.o
// RUN: %disasm --mcpu gaudi %t.o | FileCheck %s


NOP; NOP; CONVERT_INT16 V6, V12, V8, LANE_SEL=0, RNE;         NOP
NOP; NOP; CONVERT_INT16 V6, V12, V8, LANE_SEL=1, RNE, SP1;    NOP
NOP; NOP; CONVERT_INT16 V6, V12, V8, LANE_SEL=2, RNE, !SP1;   NOP
NOP; NOP; CONVERT_INT16 V6, V12, V8, LANE_SEL=3, RNE, VP1;    NOP
NOP; NOP; CONVERT_INT16 V6, V12, V8, LANE_SEL=2, RNE, !VP1;   NOP

// CHECK: nop;    nop;    convert_int16 lane_sel=0 rhne V6, V12, V8;        nop
// CHECK: nop;    nop;    convert_int16 lane_sel=1 rhne V6, V12, V8, SP1;        nop
// CHECK: nop;    nop;    convert_int16 lane_sel=2 rhne V6, V12, V8, !SP1;       nop
// CHECK: nop;    nop;    convert_int16 lane_sel=3 rhne V6, V12, V8, VP1;        nop
// CHECK: nop;    nop;    convert_int16 lane_sel=2 rhne V6, V12, V8, !VP1;       nop

NOP; NOP; CONVERT_INT16 V6, V12, V8, LANE_SEL=0;         NOP
NOP; NOP; CONVERT_INT16 V6, V12, V8, LANE_SEL=1, SP1;    NOP
NOP; NOP; CONVERT_INT16 V6, V12, V8, LANE_SEL=2, !SP1;   NOP
NOP; NOP; CONVERT_INT16 V6, V12, V8, LANE_SEL=3, VP1;    NOP
NOP; NOP; CONVERT_INT16 V6, V12, V8, LANE_SEL=2, !VP1;   NOP

// CHECK: nop;    nop;    convert_int16 lane_sel=0 rhne V6, V12, V8;        nop
// CHECK: nop;    nop;    convert_int16 lane_sel=1 rhne V6, V12, V8, SP1;        nop
// CHECK: nop;    nop;    convert_int16 lane_sel=2 rhne V6, V12, V8, !SP1;       nop
// CHECK: nop;    nop;    convert_int16 lane_sel=3 rhne V6, V12, V8, VP1;        nop
// CHECK: nop;    nop;    convert_int16 lane_sel=2 rhne V6, V12, V8, !VP1;       nop


NOP; NOP; CONVERT_INT16 V6, V12, V8, LANE_SEL=0, RNE;   NOP
NOP; NOP; CONVERT_INT16 V6, V12, V8, LANE_SEL=0, RHNE;  NOP
NOP; NOP; CONVERT_INT16 V6, V12, V8, LANE_SEL=0, RZ;    NOP
NOP; NOP; CONVERT_INT16 V6, V12, V8, LANE_SEL=0, RD;    NOP
NOP; NOP; CONVERT_INT16 V6, V12, V8, LANE_SEL=0, RU;    NOP
NOP; NOP; CONVERT_INT16 V6, V12, V8, LANE_SEL=0, SR;    NOP

// CHECK: nop;    nop;    convert_int16 lane_sel=0 rhne V6, V12, V8;         nop
// CHECK: nop;    nop;    convert_int16 lane_sel=0 rhne V6, V12, V8;         nop
// CHECK: nop;    nop;    convert_int16 lane_sel=0 rz V6, V12, V8;   nop
// CHECK: nop;    nop;    convert_int16 lane_sel=0 rd V6, V12, V8;   nop
// CHECK: nop;    nop;    convert_int16 lane_sel=0 ru V6, V12, V8;   nop
// CHECK: nop;    nop;    convert_int16 lane_sel=0 sr V6, V12, V8;   nop
