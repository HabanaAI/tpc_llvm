// RUN: %clang -cc1as -triple tpc-none-none -filetype obj %s -o %t.o
// RUN: %disasm %t.o | FileCheck %s
// RUN: %clang -cc1as -triple tpc-none-none -target-cpu gaudi -filetype obj %s -o %t.o
// RUN: %disasm -mcpu=gaudi %t.o | FileCheck %s




// f32 => i32
NOP; CONVERT.F32 S0, S1, INT32, RZ; NOP; NOP
NOP; CONVERT.F32 S0, S1, INT32, RU, SP2; NOP; NOP
NOP; CONVERT.F32 S0, S1, INT32, RD, !SP2; NOP; NOP
NOP; CONVERT.F32 S0, S1, INT32, RHNE, SP3; NOP; NOP
NOP; CONVERT.F32 S0, S1, INT32, DEFAULT, SP3; NOP; NOP

// CHECK: nop;    convert.f32  target_type=int32 rz S0, S1, SP0;   nop;    nop
// CHECK: nop;    convert.f32  target_type=int32 ru S0, S1, SP2;   nop;    nop
// CHECK: nop;    convert.f32  target_type=int32 rd S0, S1, !SP2;  nop;    nop
// CHECK: nop;    convert.f32  target_type=int32 rhne S0, S1, SP3;         nop;    nop
// CHECK: nop;    convert.f32  target_type=int32  S0, S1, SP3;     nop;    nop


// f32 => i16
NOP; CONVERT.F32 S0, S1, INT16, RZ; NOP; NOP
NOP; CONVERT.F32 S0, S1, INT16, RZ, SP1; NOP; NOP
NOP; CONVERT.F32 S0, S1, INT16, RZ, !SP1; NOP; NOP

// CHECK: nop;    convert.f32  target_type=int16 rz S0, S1, SP0;   nop;    nop
// CHECK: nop;    convert.f32  target_type=int16 rz S0, S1, SP1;   nop;    nop
// CHECK: nop;    convert.f32  target_type=int16 rz S0, S1, !SP1;  nop;    nop


// f32 => i8
NOP; CONVERT.F32 S0, S1, INT8, RNE; NOP; NOP
NOP; CONVERT.F32 S0, S1, INT8, RNE, SP1; NOP; NOP
NOP; CONVERT.F32 S0, S1, INT8, RZ, !SP1; NOP; NOP

// CHECK: nop;    convert.f32  target_type=int8 rhne S0, S1, SP0;  nop;    nop
// CHECK: nop;    convert.f32  target_type=int8 rhne S0, S1, SP1;  nop;    nop
// CHECK: nop;    convert.f32  target_type=int8 rz S0, S1, !SP1;   nop;    nop


// i32 => fp32
NOP; CONVERT.I32 S0, S1, FP32, RHNE; NOP; NOP
NOP; CONVERT.I32 S0, S1, FP32, RNE, SP1; NOP; NOP
NOP; CONVERT.I32 S0, S1, FP32, RHNE, !SP1; NOP; NOP

// CHECK: nop;    convert.i32 target_type=fp32 rhne S0, S1, SP0;   nop;    nop
// CHECK: nop;    convert.i32 target_type=fp32 rhne S0, S1, SP1;   nop;    nop
// CHECK: nop;    convert.i32 target_type=fp32 rhne S0, S1, !SP1;  nop;    nop


// i32 => u32
NOP; CONVERT.I32 S0, S1, UINT32; NOP; NOP
NOP; CONVERT.I32 S0, S1, UINT32, SP1; NOP; NOP
NOP; CONVERT.I32 S0, S1, UINT32, !SP1; NOP; NOP

// CHECK: nop;    convert.i32  target_type=uint32  S0, S1, SP0;    nop;    nop
// CHECK: nop;    convert.i32  target_type=uint32  S0, S1, SP1;    nop;    nop
// CHECK: nop;    convert.i32  target_type=uint32  S0, S1, !SP1;   nop;    nop


// i16 => fp32
NOP; CONVERT.I16 S0, S1, FP32; NOP; NOP
NOP; CONVERT.I16 S0, S1, FP32, SP1; NOP; NOP
NOP; CONVERT.I16 S0, S1, FP32, !SP1; NOP; NOP

// CHECK: nop;    convert.i16 target_type=fp32  S0, S1, SP0;       nop;    nop
// CHECK: nop;    convert.i16 target_type=fp32  S0, S1, SP1;       nop;    nop
// CHECK: nop;    convert.i16 target_type=fp32  S0, S1, !SP1;      nop;    nop


// i16 => i32
NOP; CONVERT.I16 S0, S1, INT32; NOP; NOP
NOP; CONVERT.I16 S0, S1, INT32, SP1; NOP; NOP
NOP; CONVERT.I16 S0, S1, INT32, !SP1; NOP; NOP

// CHECK: nop;    convert.i16  target_type=int32  S0, S1, SP0;     nop;    nop
// CHECK: nop;    convert.i16  target_type=int32  S0, S1, SP1;     nop;    nop
// CHECK: nop;    convert.i16  target_type=int32  S0, S1, !SP1;    nop;    nop


// i16 => u32
NOP; CONVERT.I16 S0, S1, UINT32; NOP; NOP
NOP; CONVERT.I16 S0, S1, UINT32, SP1; NOP; NOP
NOP; CONVERT.I16 S0, S1, UINT32, !SP1; NOP; NOP

// CHECK: nop;    convert.i16  target_type=uint32  S0, S1, SP0;    nop;    nop
// CHECK: nop;    convert.i16  target_type=uint32  S0, S1, SP1;    nop;    nop
// CHECK: nop;    convert.i16  target_type=uint32  S0, S1, !SP1;   nop;    nop


// i16 => u16
NOP; CONVERT.I16 S0, S1, UINT16; NOP; NOP
NOP; CONVERT.I16 S0, S1, UINT16, SP1; NOP; NOP
NOP; CONVERT.I16 S0, S1, UINT16, !SP1; NOP; NOP

// CHECK: nop;    convert.i16  target_type=uint16  S0, S1, SP0;    nop;    nop
// CHECK: nop;    convert.i16  target_type=uint16  S0, S1, SP1;    nop;    nop
// CHECK: nop;    convert.i16  target_type=uint16  S0, S1, !SP1;   nop;    nop


// i16 => u8
NOP; CONVERT.I16 S0, S1, UINT8; NOP; NOP
NOP; CONVERT.I16 S0, S1, UINT8, SP1; NOP; NOP
NOP; CONVERT.I16 S0, S1, UINT8, !SP1; NOP; NOP

// CHECK: nop;    convert.i16  target_type=uint8  S0, S1, SP0;     nop;    nop
// CHECK: nop;    convert.i16  target_type=uint8  S0, S1, SP1;     nop;    nop
// CHECK: nop;    convert.i16  target_type=uint8  S0, S1, !SP1;    nop;    nop


// i8 => fp32
nop; convert.i8 S0, S1, FP32; nop; nop
nop; convert.i8 S0, S1, FP32, SP1; nop; nop
nop; convert.i8 S0, S1, FP32, !SP1; nop; nop

// CHECK: nop;    convert.i8 target_type=fp32  S0, S1, SP0;        nop;    nop
// CHECK: nop;    convert.i8 target_type=fp32  S0, S1, SP1;        nop;    nop
// CHECK: nop;    convert.i8 target_type=fp32  S0, S1, !SP1;       nop;    nop


// i8 => i32
nop; convert.i8 S0, S1, INT32; nop; nop
nop; convert.i8 S0, S1, INT32, SP1; nop; nop
nop; convert.i8 S0, S1, INT32, !SP1; nop; nop

// CHECK: nop;    convert.i8  target_type=int32  S0, S1, SP0;      nop;    nop
// CHECK: nop;    convert.i8  target_type=int32  S0, S1, SP1;      nop;    nop
// CHECK: nop;    convert.i8  target_type=int32  S0, S1, !SP1;     nop;    nop


// i8 => u32
nop; convert.i8 S0, S1, UINT32; nop; nop
nop; convert.i8 S0, S1, UINT32, SP1; nop; nop
nop; convert.i8 S0, S1, UINT32, !SP1; nop; nop

// CHECK: nop;    convert.i8  target_type=uint32  S0, S1, SP0;     nop;    nop
// CHECK: nop;    convert.i8  target_type=uint32  S0, S1, SP1;     nop;    nop
// CHECK: nop;    convert.i8  target_type=uint32  S0, S1, !SP1;    nop;    nop


// i8 => i16
nop; convert.i8 S0, S1, INT16; nop; nop
nop; convert.i8 S0, S1, INT16, SP1; nop; nop
nop; convert.i8 S0, S1, INT16, !SP1; nop; nop

// CHECK: nop;    convert.i8  target_type=int16  S0, S1, SP0;      nop;    nop
// CHECK: nop;    convert.i8  target_type=int16  S0, S1, SP1;      nop;    nop
// CHECK: nop;    convert.i8  target_type=int16  S0, S1, !SP1;     nop;    nop


// i8 => u16
nop; convert.i8 S0, S1, UINT16; nop; nop
nop; convert.i8 S0, S1, UINT16, SP1; nop; nop
nop; convert.i8 S0, S1, UINT16, !SP1; nop; nop

// CHECK: nop;    convert.i8  target_type=uint16  S0, S1, SP0;     nop;    nop
// CHECK: nop;    convert.i8  target_type=uint16  S0, S1, SP1;     nop;    nop
// CHECK: nop;    convert.i8  target_type=uint16  S0, S1, !SP1;    nop;    nop


// i8 => u8
nop; convert.i8 S0, S1, UINT8; nop; nop
nop; convert.i8 S0, S1, UINT8, SP1; nop; nop
nop; convert.i8 S0, S1, UINT8, !SP1; nop; nop

// CHECK: nop;    convert.i8  target_type=uint8  S0, S1, SP0;      nop;    nop
// CHECK: nop;    convert.i8  target_type=uint8  S0, S1, SP1;      nop;    nop
// CHECK: nop;    convert.i8  target_type=uint8  S0, S1, !SP1;     nop;    nop
