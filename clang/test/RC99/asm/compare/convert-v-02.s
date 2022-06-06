// RUN: %clang -cc1as -triple tpc-none-none -target-cpu gaudi -filetype obj %s -o %t.o
// R UN: %disasm --mcpu=gaudi %t.o | FileCheck %s
// RUN: %clang -cc1as -triple tpc-none-none -target-cpu goya2 -filetype obj %s -o %t.o
// R UN: %disasm --mcpu=goya2 %t.o | FileCheck %s

// X-FAIL: due to lane_sel=0

// f32 => bf16
NOP; NOP; CONVERT.F32 V0, D2, BF16, RHAZ; NOP
NOP; NOP; CONVERT.F32 V0, D2, BF16, RHAZ, SP1; NOP
NOP; NOP; CONVERT.F32 V0, D2, BF16, RHAZ, !SP1; NOP
NOP; NOP; CONVERT.F32 V0, D2, BF16, RHAZ, VP1; NOP
NOP; NOP; CONVERT.F32 V0, D2, BF16, RHAZ, !VP1; NOP

// CHECK: nop;    nop;    convert.f32  lane_sel=0 target_type=bf16 rhaz V0, V2;  nop
// CHECK: nop;    nop;    convert.f32  lane_sel=0 target_type=bf16 rhaz V0, V2, SP1;  nop
// CHECK: nop;    nop;    convert.f32  lane_sel=0 target_type=bf16 rhaz V0, V2, !SP1;         nop
// CHECK: nop;    nop;    convert.f32  lane_sel=0 target_type=bf16 rhaz V0, V2, VP1;  nop
// CHECK: nop;    nop;    convert.f32  lane_sel=0 target_type=bf16 rhaz V0, V2, !VP1;         nop

NOP; NOP; CONVERT.F32 V0, V1, BF16, SINGLE_LANE, RHAZ; NOP
NOP; NOP; CONVERT.F32 V0, V1, BF16, SINGLE_LANE, RHAZ, SP1; NOP
NOP; NOP; CONVERT.F32 V0, V1, BF16, SINGLE_LANE, RHAZ, !SP1; NOP
NOP; NOP; CONVERT.F32 V0, V1, BF16, SINGLE_LANE, RHAZ, VP1; NOP
NOP; NOP; CONVERT.F32 V0, V1, BF16, SINGLE_LANE, RHAZ, !VP1; NOP

// CHECK: nop;    nop;    convert.f32  lane_sel=0 target_type=bf16 rhaz V0, V1;      nop
// CHECK: nop;    nop;    convert.f32  lane_sel=0 target_type=bf16 rhaz V0, V1, SP1;      nop
// CHECK: nop;    nop;    convert.f32  lane_sel=0 target_type=bf16 rhaz V0, V1, !SP1;     nop
// CHECK: nop;    nop;    convert.f32  lane_sel=0 target_type=bf16 rhaz V0, V1, VP1;      nop
// CHECK: nop;    nop;    convert.f32  lane_sel=0 target_type=bf16 rhaz V0, V1, !VP1;     nop

NOP; NOP; CONVERT.F32 V0, V1, BF16, ALL_LANES, RHAZ; NOP
NOP; NOP; CONVERT.F32 V0, V1, BF16, ALL_LANES, RHAZ, SP1; NOP
NOP; NOP; CONVERT.F32 V0, V1, BF16, ALL_LANES, RHAZ, !SP1; NOP
NOP; NOP; CONVERT.F32 V0, V1, BF16, ALL_LANES, RHAZ, VP1; NOP
NOP; NOP; CONVERT.F32 V0, V1, BF16, ALL_LANES, RHAZ, !VP1; NOP

// CHECK: nop;    nop;    convert.f32  all_lanes target_type=bf16 rhaz V0, V1;      nop
// CHECK: nop;    nop;    convert.f32  all_lanes target_type=bf16 rhaz V0, V1, SP1;      nop
// CHECK: nop;    nop;    convert.f32  all_lanes target_type=bf16 rhaz V0, V1, !SP1;     nop
// CHECK: nop;    nop;    convert.f32  all_lanes target_type=bf16 rhaz V0, V1, VP1;      nop
// CHECK: nop;    nop;    convert.f32  all_lanes target_type=bf16 rhaz V0, V1, !VP1;     nop

// i32 => bf16
NOP; NOP; CONVERT.I32 V0, V1, BF16, LANE_SEL=1, RHNE; NOP
NOP; NOP; CONVERT.I32 V0, V1, BF16, LANE_SEL=1, RHNE, SP1; NOP
NOP; NOP; CONVERT.I32 V0, V1, BF16, LANE_SEL=1, RHNE, !SP1; NOP
NOP; NOP; CONVERT.I32 V0, V1, BF16, LANE_SEL=1, RHNE, VP1; NOP
NOP; NOP; CONVERT.I32 V0, V1, BF16, LANE_SEL=1, RHNE, !VP1; NOP

// CHECK: nop;    nop;    convert.i32  lane_sel=1 target_type=bf16 rhne V0, V1;       nop
// CHECK: nop;    nop;    convert.i32  lane_sel=1 target_type=bf16 rhne V0, V1, SP1;       nop
// CHECK: nop;    nop;    convert.i32  lane_sel=1 target_type=bf16 rhne V0, V1, !SP1;      nop
// CHECK: nop;    nop;    convert.i32  lane_sel=1 target_type=bf16 rhne V0, V1, VP1;       nop
// CHECK: nop;    nop;    convert.i32  lane_sel=1 target_type=bf16 rhne V0, V1, !VP1;      nop


// i16 => bf16
NOP; NOP; CONVERT.I16 V0, V1, BF16, RHNE; NOP
NOP; NOP; CONVERT.I16 V0, V1, BF16, RHNE, SP1; NOP
NOP; NOP; CONVERT.I16 V0, V1, BF16, RHNE, !SP1; NOP
NOP; NOP; CONVERT.I16 V0, V1, BF16, RHNE, VP1; NOP
NOP; NOP; CONVERT.I16 V0, V1, BF16, RHNE, !VP1; NOP

// CHECK: nop;    nop;    convert.i16  lane_sel=0 target_type=bf16 rhne V0, V1;  nop
// CHECK: nop;    nop;    convert.i16  lane_sel=0 target_type=bf16 rhne V0, V1, SP1;  nop
// CHECK: nop;    nop;    convert.i16  lane_sel=0 target_type=bf16 rhne V0, V1, !SP1;         nop
// CHECK: nop;    nop;    convert.i16  lane_sel=0 target_type=bf16 rhne V0, V1, VP1;  nop
// CHECK: nop;    nop;    convert.i16  lane_sel=0 target_type=bf16 rhne V0, V1, !VP1;         nop


// u16 => bf16
NOP; NOP; CONVERT.U16 V0, V1, BF16, RHNE; NOP
NOP; NOP; CONVERT.U16 V0, V1, BF16, RHNE, SP1; NOP
NOP; NOP; CONVERT.U16 V0, V1, BF16, RHNE, !SP1; NOP
NOP; NOP; CONVERT.U16 V0, V1, BF16, RHNE, VP1; NOP
NOP; NOP; CONVERT.U16 V0, V1, BF16, RHNE, !VP1; NOP

// CHECK: nop;    nop;    convert.u16  lane_sel=0 target_type=bf16 rhne V0, V1;  nop
// CHECK: nop;    nop;    convert.u16  lane_sel=0 target_type=bf16 rhne V0, V1, SP1;  nop
// CHECK: nop;    nop;    convert.u16  lane_sel=0 target_type=bf16 rhne V0, V1, !SP1;         nop
// CHECK: nop;    nop;    convert.u16  lane_sel=0 target_type=bf16 rhne V0, V1, VP1;  nop
// CHECK: nop;    nop;    convert.u16  lane_sel=0 target_type=bf16 rhne V0, V1, !VP1;         nop


// bf16 => fp32
NOP; NOP; CONVERT.BF16 V0, V1, FP32; NOP
NOP; NOP; CONVERT.BF16 V0, V1, FP32, SP1; NOP
NOP; NOP; CONVERT.BF16 V0, V1, FP32, !SP1; NOP
NOP; NOP; CONVERT.BF16 V0, V1, FP32, VP1; NOP
NOP; NOP; CONVERT.BF16 V0, V1, FP32, !VP1; NOP

// CHECK: nop;    nop;    convert.bf16 lane_sel=0 target_type=fp32  V0, V1;      nop
// CHECK: nop;    nop;    convert.bf16 lane_sel=0 target_type=fp32  V0, V1, SP1;      nop
// CHECK: nop;    nop;    convert.bf16 lane_sel=0 target_type=fp32  V0, V1, !SP1;     nop
// CHECK: nop;    nop;    convert.bf16 lane_sel=0 target_type=fp32  V0, V1, VP1;      nop
// CHECK: nop;    nop;    convert.bf16 lane_sel=0 target_type=fp32  V0, V1, !VP1;     nop


// bf16 => i16
NOP; NOP; CONVERT.BF16 V0, V1, INT16, RU; NOP
NOP; NOP; CONVERT.BF16 V0, V1, INT16, RU, SP1; NOP
NOP; NOP; CONVERT.BF16 V0, V1, INT16, RU, !SP1; NOP
NOP; NOP; CONVERT.BF16 V0, V1, INT16, RU, VP1; NOP
NOP; NOP; CONVERT.BF16 V0, V1, INT16, RU, !VP1; NOP

// CHECK: nop;    nop;    convert.bf16  lane_sel=0 target_type=int16 ru V0, V1;  nop
// CHECK: nop;    nop;    convert.bf16  lane_sel=0 target_type=int16 ru V0, V1, SP1;  nop
// CHECK: nop;    nop;    convert.bf16  lane_sel=0 target_type=int16 ru V0, V1, !SP1;         nop
// CHECK: nop;    nop;    convert.bf16  lane_sel=0 target_type=int16 ru V0, V1, VP1;  nop
// CHECK: nop;    nop;    convert.bf16  lane_sel=0 target_type=int16 ru V0, V1, !VP1;         nop
