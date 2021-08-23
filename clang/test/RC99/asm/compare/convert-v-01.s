// RUN: %clang -cc1as -triple tpc-none-none -filetype obj %s -o %t.o
// RUN: %disasm %t.o | FileCheck %s
// RUN: %clang -cc1as -triple tpc-none-none -target-cpu gaudi -filetype obj %s -o %t.o
// RUN: %disasm -mcpu=gaudi %t.o | FileCheck %s




// f32 => i32
NOP; NOP; CONVERT.F32 V0, V1, INT32, RZ; NOP
NOP; NOP; CONVERT.F32 V0, V1, INT32, RZ, SP1; NOP
NOP; NOP; CONVERT.F32 V0, V1, INT32, RZ, !SP1; NOP
NOP; NOP; CONVERT.F32 V0, V1, INT32, RZ, VP1; NOP
NOP; NOP; CONVERT.F32 V0, V1, INT32, RZ, !VP1; NOP

// CHECK: nop;    nop;    convert.f32  target_type=int32 rz V0, V1, SP0;   nop
// CHECK: nop;    nop;    convert.f32  target_type=int32 rz V0, V1, SP1;   nop
// CHECK: nop;    nop;    convert.f32  target_type=int32 rz V0, V1, !SP1;  nop
// CHECK: nop;    nop;    convert.f32  target_type=int32 rz V0, V1, VP1;   nop
// CHECK: nop;    nop;    convert.f32  target_type=int32 rz V0, V1, !VP1;  nop


// f32 => i16
NOP; NOP; CONVERT.f32 V0, V1, INT16, LANE_SEL=1; NOP
NOP; NOP; CONVERT.f32 V0, V1, INT16, LANE_SEL=1, SP1; NOP
NOP; NOP; CONVERT.f32 V0, V1, INT16, LANE_SEL=2, !SP1; NOP
NOP; NOP; CONVERT.f32 V0, V1, INT16, LANE_SEL=1, VP1; NOP
NOP; NOP; CONVERT.f32 V0, V1, INT16, LANE_SEL=3, !VP1; NOP

// CHECK: nop;    nop;    convert.f32  lane_sel=1 target_type=int16  V0, V1, SP0;  nop
// CHECK: nop;    nop;    convert.f32  lane_sel=1 target_type=int16  V0, V1, SP1;  nop
// CHECK: nop;    nop;    convert.f32  lane_sel=2 target_type=int16  V0, V1, !SP1;    nop
// CHECK: nop;    nop;    convert.f32  lane_sel=1 target_type=int16  V0, V1, VP1;  nop
// CHECK: nop;    nop;    convert.f32  lane_sel=3 target_type=int16  V0, V1, !VP1;    nop


// f32 => i8
NOP; NOP; CONVERT.F32 V0, V1, INT8, LANE_SEL=3, RNE; NOP
NOP; NOP; CONVERT.F32 V0, V1, INT8, LANE_SEL=2, RZ, SP1; NOP
NOP; NOP; CONVERT.F32 V0, V1, INT8, LANE_SEL=1, RZ, !SP1; NOP
NOP; NOP; CONVERT.F32 V0, V1, INT8, LANE_SEL=2, RZ, VP1; NOP
NOP; NOP; CONVERT.F32 V0, V1, INT8, LANE_SEL=1, RZ, !VP1; NOP

// CHECK: nop;    nop;    convert.f32  lane_sel=3 target_type=int8 rhne V0, V1, SP0;       nop
// CHECK: nop;    nop;    convert.f32  lane_sel=2 target_type=int8 rz V0, V1, SP1;         nop
// CHECK: nop;    nop;    convert.f32  lane_sel=1 target_type=int8 rz V0, V1, !SP1;        nop
// CHECK: nop;    nop;    convert.f32  lane_sel=2 target_type=int8 rz V0, V1, VP1;         nop
// CHECK: nop;    nop;    convert.f32  lane_sel=1 target_type=int8 rz V0, V1, !VP1;        nop


// i32 => fp32
NOP; NOP; CONVERT.I32 V0, V1, FP32, RHNE; NOP
NOP; NOP; CONVERT.I32 V0, V1, FP32, RNE, SP1; NOP
NOP; NOP; CONVERT.I32 V0, V1, FP32, RNE, !SP1; NOP
NOP; NOP; CONVERT.I32 V0, V1, FP32, RNE, VP1; NOP
NOP; NOP; CONVERT.I32 V0, V1, FP32, RNE, !VP1; NOP

// CHECK: nop;    nop;    convert.i32 target_type=fp32 rhne V0, V1, SP0;   nop
// CHECK: nop;    nop;    convert.i32 target_type=fp32 rhne V0, V1, SP1;   nop
// CHECK: nop;    nop;    convert.i32 target_type=fp32 rhne V0, V1, !SP1;  nop
// CHECK: nop;    nop;    convert.i32 target_type=fp32 rhne V0, V1, VP1;   nop
// CHECK: nop;    nop;    convert.i32 target_type=fp32 rhne V0, V1, !VP1;  nop


// i32 => u32
NOP; NOP; CONVERT.I32 V0, V1, UINT32; NOP
NOP; NOP; CONVERT.I32 V0, V1, UINT32, SP1; NOP
NOP; NOP; CONVERT.I32 V0, V1, UINT32, !SP1; NOP
NOP; NOP; CONVERT.I32 V0, V1, UINT32, VP1; NOP
NOP; NOP; CONVERT.I32 V0, V1, UINT32, !VP1; NOP

// CHECK: nop;    nop;    convert.i32  target_type=uint32  V0, V1, SP0;    nop
// CHECK: nop;    nop;    convert.i32  target_type=uint32  V0, V1, SP1;    nop
// CHECK: nop;    nop;    convert.i32  target_type=uint32  V0, V1, !SP1;   nop
// CHECK: nop;    nop;    convert.i32  target_type=uint32  V0, V1, VP1;    nop
// CHECK: nop;    nop;    convert.i32  target_type=uint32  V0, V1, !VP1;   nop


// i16 => fp32
NOP; NOP; CONVERT.I16 V0, V1, FP32; NOP
NOP; NOP; CONVERT.I16 V0, V1, FP32, SP1; NOP
NOP; NOP; CONVERT.I16 V0, V1, FP32, !SP1; NOP
NOP; NOP; CONVERT.I16 V0, V1, FP32, VP1; NOP
NOP; NOP; CONVERT.I16 V0, V1, FP32, !VP1; NOP

// CHECK: nop;    nop;    convert.i16 target_type=fp32  V0, V1, SP0;       nop
// CHECK: nop;    nop;    convert.i16 target_type=fp32  V0, V1, SP1;       nop
// CHECK: nop;    nop;    convert.i16 target_type=fp32  V0, V1, !SP1;      nop
// CHECK: nop;    nop;    convert.i16 target_type=fp32  V0, V1, VP1;       nop
// CHECK: nop;    nop;    convert.i16 target_type=fp32  V0, V1, !VP1;      nop


// i16 => i32
NOP; NOP; CONVERT.I16 V0, V1, INT32, LANE_SEL=0; NOP
NOP; NOP; CONVERT.I16 V0, V1, INT32, SP1; NOP
NOP; NOP; CONVERT.I16 V0, V1, INT32, !SP1; NOP
NOP; NOP; CONVERT.I16 V0, V1, INT32, VP1; NOP
NOP; NOP; CONVERT.I16 V0, V1, INT32, !VP1; NOP

// CHECK: nop;    nop;    convert.i16  target_type=int32  V0, V1, SP0;     nop
// CHECK: nop;    nop;    convert.i16  target_type=int32  V0, V1, SP1;     nop
// CHECK: nop;    nop;    convert.i16  target_type=int32  V0, V1, !SP1;    nop
// CHECK: nop;    nop;    convert.i16  target_type=int32  V0, V1, VP1;     nop
// CHECK: nop;    nop;    convert.i16  target_type=int32  V0, V1, !VP1;    nop


// i16 => u32
NOP; NOP; CONVERT.I16 V0, V1, UINT32; NOP
NOP; NOP; CONVERT.I16 V0, V1, UINT32, SP1; NOP
NOP; NOP; CONVERT.I16 V0, V1, UINT32, !SP1; NOP
NOP; NOP; CONVERT.I16 V0, V1, UINT32, VP1; NOP
NOP; NOP; CONVERT.I16 V0, V1, UINT32, !VP1; NOP

// CHECK: nop;    nop;    convert.i16  target_type=uint32  V0, V1, SP0;    nop
// CHECK: nop;    nop;    convert.i16  target_type=uint32  V0, V1, SP1;    nop
// CHECK: nop;    nop;    convert.i16  target_type=uint32  V0, V1, !SP1;   nop
// CHECK: nop;    nop;    convert.i16  target_type=uint32  V0, V1, VP1;    nop
// CHECK: nop;    nop;    convert.i16  target_type=uint32  V0, V1, !VP1;   nop


// i16 => u16
NOP; NOP; CONVERT.I16 V0, V1, UINT16; NOP
NOP; NOP; CONVERT.I16 V0, V1, UINT16, SP1; NOP
NOP; NOP; CONVERT.I16 V0, V1, UINT16, !SP1; NOP
NOP; NOP; CONVERT.I16 V0, V1, UINT16, VP1; NOP
NOP; NOP; CONVERT.I16 V0, V1, UINT16, !VP1; NOP

// CHECK: nop;    nop;    convert.i16  target_type=uint16  V0, V1, SP0;    nop
// CHECK: nop;    nop;    convert.i16  target_type=uint16  V0, V1, SP1;    nop
// CHECK: nop;    nop;    convert.i16  target_type=uint16  V0, V1, !SP1;   nop
// CHECK: nop;    nop;    convert.i16  target_type=uint16  V0, V1, VP1;    nop
// CHECK: nop;    nop;    convert.i16  target_type=uint16  V0, V1, !VP1;   nop


// i16 => u8
nop; nop; convert.i16 V0, V1, UINT8, LANE_SEL=1; nop
nop; nop; convert.i16 V0, V1, UINT8, LANE_SEL=2, SP1; nop
nop; nop; convert.i16 V0, V1, UINT8, LANE_SEL=1, !SP1; nop
nop; nop; convert.i16 V0, V1, UINT8, LANE_SEL=1, VP1; nop
nop; nop; convert.i16 V0, V1, UINT8, LANE_SEL=3, !VP1; nop

// CHECK: nop;    nop;    convert.i16  lane_sel=1 target_type=uint8  V0, V1, SP0;  nop
// CHECK: nop;    nop;    convert.i16  lane_sel=2 target_type=uint8  V0, V1, SP1;     nop
// CHECK: nop;    nop;    convert.i16  lane_sel=1 target_type=uint8  V0, V1, !SP1;         nop
// CHECK: nop;    nop;    convert.i16  lane_sel=1 target_type=uint8  V0, V1, VP1;  nop
// CHECK: nop;    nop;    convert.i16  lane_sel=3 target_type=uint8  V0, V1, !VP1;    nop

// i8 => fp32
nop; nop; convert.i8 V0, V1, FP32; nop
nop; nop; convert.i8 V0, V1, FP32, SP1; nop
nop; nop; convert.i8 V0, V1, FP32, !SP1; nop
nop; nop; convert.i8 V0, V1, FP32, VP1; nop
nop; nop; convert.i8 V0, V1, FP32, !VP1; nop

// CHECK: nop;    nop;    convert.i8 target_type=fp32  V0, V1, SP0;        nop
// CHECK: nop;    nop;    convert.i8 target_type=fp32  V0, V1, SP1;        nop
// CHECK: nop;    nop;    convert.i8 target_type=fp32  V0, V1, !SP1;       nop
// CHECK: nop;    nop;    convert.i8 target_type=fp32  V0, V1, VP1;        nop
// CHECK: nop;    nop;    convert.i8 target_type=fp32  V0, V1, !VP1;       nop


// i8 => i32
nop; nop; convert.i8 V0, V1, INT32; nop
nop; nop; convert.i8 V0, V1, INT32, SP1; nop
nop; nop; convert.i8 V0, V1, INT32, !SP1; nop
nop; nop; convert.i8 V0, V1, INT32, VP1; nop
nop; nop; convert.i8 V0, V1, INT32, !VP1; nop

// CHECK: nop;    nop;    convert.i8  target_type=int32  V0, V1, SP0;      nop
// CHECK: nop;    nop;    convert.i8  target_type=int32  V0, V1, SP1;      nop
// CHECK: nop;    nop;    convert.i8  target_type=int32  V0, V1, !SP1;     nop
// CHECK: nop;    nop;    convert.i8  target_type=int32  V0, V1, VP1;      nop
// CHECK: nop;    nop;    convert.i8  target_type=int32  V0, V1, !VP1;     nop


// i8 => u32
nop; nop; convert.i8 V0, V1, UINT32; nop
nop; nop; convert.i8 V0, V1, UINT32, SP1; nop
nop; nop; convert.i8 V0, V1, UINT32, !SP1; nop
nop; nop; convert.i8 V0, V1, UINT32, VP1; nop
nop; nop; convert.i8 V0, V1, UINT32, !VP1; nop

// CHECK: nop;    nop;    convert.i8  target_type=uint32  V0, V1, SP0;     nop
// CHECK: nop;    nop;    convert.i8  target_type=uint32  V0, V1, SP1;     nop
// CHECK: nop;    nop;    convert.i8  target_type=uint32  V0, V1, !SP1;    nop
// CHECK: nop;    nop;    convert.i8  target_type=uint32  V0, V1, VP1;     nop
// CHECK: nop;    nop;    convert.i8  target_type=uint32  V0, V1, !VP1;    nop


// i8 => i16
nop; nop; convert.i8 V0, V1, INT16; nop
nop; nop; convert.i8 V0, V1, INT16, SP1; nop
nop; nop; convert.i8 V0, V1, INT16, !SP1; nop
nop; nop; convert.i8 V0, V1, INT16, VP1; nop
nop; nop; convert.i8 V0, V1, INT16, !VP1; nop

// CHECK: nop;    nop;    convert.i8  target_type=int16  V0, V1, SP0;      nop
// CHECK: nop;    nop;    convert.i8  target_type=int16  V0, V1, SP1;      nop
// CHECK: nop;    nop;    convert.i8  target_type=int16  V0, V1, !SP1;     nop
// CHECK: nop;    nop;    convert.i8  target_type=int16  V0, V1, VP1;      nop
// CHECK: nop;    nop;    convert.i8  target_type=int16  V0, V1, !VP1;     nop


// i8 => u16
nop; nop; convert.i8 V0, V1, UINT16; nop
nop; nop; convert.i8 V0, V1, UINT16, SP1; nop
nop; nop; convert.i8 V0, V1, UINT16, !SP1; nop
nop; nop; convert.i8 V0, V1, UINT16, VP1; nop
nop; nop; convert.i8 V0, V1, UINT16, !VP1; nop

// CHECK: nop;    nop;    convert.i8  target_type=uint16  V0, V1, SP0;     nop
// CHECK: nop;    nop;    convert.i8  target_type=uint16  V0, V1, SP1;     nop
// CHECK: nop;    nop;    convert.i8  target_type=uint16  V0, V1, !SP1;    nop
// CHECK: nop;    nop;    convert.i8  target_type=uint16  V0, V1, VP1;     nop
// CHECK: nop;    nop;    convert.i8  target_type=uint16  V0, V1, !VP1;    nop


// i8 => u8
nop; nop; convert.i8 V0, V1, UINT8; nop
nop; nop; convert.i8 V0, V1, UINT8, SP1; nop
nop; nop; convert.i8 V0, V1, UINT8, !SP1; nop
nop; nop; convert.i8 V0, V1, UINT8, VP1; nop
nop; nop; convert.i8 V0, V1, UINT8, !VP1; nop

// CHECK: nop;    nop;    convert.i8  target_type=uint8  V0, V1, SP0;      nop
// CHECK: nop;    nop;    convert.i8  target_type=uint8  V0, V1, SP1;      nop
// CHECK: nop;    nop;    convert.i8  target_type=uint8  V0, V1, !SP1;     nop
// CHECK: nop;    nop;    convert.i8  target_type=uint8  V0, V1, VP1;      nop
// CHECK: nop;    nop;    convert.i8  target_type=uint8  V0, V1, !VP1;     nop
