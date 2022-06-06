// RUN: %clang -cc1as -triple tpc-none-none  -filetype obj %s -o %t.o
// RUN: %disasm %t.o | FileCheck %s

NOP; NOP; GET_LUT_ENTRY_AND_INTERVAL_START.F32 V4, V2, 1, 1; NOP
NOP; NOP; GET_LUT_ENTRY_AND_INTERVAL_START.F32 V4, V2, 1, 1, SP3; NOP
NOP; NOP; GET_LUT_ENTRY_AND_INTERVAL_START.F32 V4, V2, 1, 1, !SP3; NOP
NOP; NOP; GET_LUT_ENTRY_AND_INTERVAL_START.F32 V4, V2, 1, 1, VP3; NOP
NOP; NOP; GET_LUT_ENTRY_AND_INTERVAL_START.F32 V4, V2, 1, 1, !VP3; NOP

// CHECK: nop;    nop;    get_lut_entry_and_interval_start.f32  tanh D4, V2, 0x1;     nop
// CHECK: nop;    nop;    get_lut_entry_and_interval_start.f32  tanh D4, V2, 0x1, SP3;     nop
// CHECK: nop;    nop;    get_lut_entry_and_interval_start.f32  tanh D4, V2, 0x1, !SP3;    nop
// CHECK: nop;    nop;    get_lut_entry_and_interval_start.f32  tanh D4, V2, 0x1, VP3;     nop
// CHECK: nop;    nop;    get_lut_entry_and_interval_start.f32  tanh D4, V2, 0x1, !VP3;    nop


NOP; NOP; GET_LUT_ENTRY_AND_INTERVAL_START.F32 V4, V2, 0, 1; NOP
NOP; NOP; GET_LUT_ENTRY_AND_INTERVAL_START.F32 V4, V2, 1, 1; NOP
NOP; NOP; GET_LUT_ENTRY_AND_INTERVAL_START.F32 V4, V2, 2, 1; NOP
NOP; NOP; GET_LUT_ENTRY_AND_INTERVAL_START.F32 V4, V2, 3, 1; NOP
NOP; NOP; GET_LUT_ENTRY_AND_INTERVAL_START.F32 V4, V2, 28, 1; NOP
NOP; NOP; GET_LUT_ENTRY_AND_INTERVAL_START.F32 V4, V2, 29, 1; NOP
NOP; NOP; GET_LUT_ENTRY_AND_INTERVAL_START.F32 V4, V2, 30, 1; NOP
NOP; NOP; GET_LUT_ENTRY_AND_INTERVAL_START.F32 V4, V2, 31, 1; NOP

// CHECK: nop;    nop;    get_lut_entry_and_interval_start.f32  tanh D4, V2, 0x0;     nop
// CHECK: nop;    nop;    get_lut_entry_and_interval_start.f32  tanh D4, V2, 0x1;     nop
// CHECK: nop;    nop;    get_lut_entry_and_interval_start.f32  tanh D4, V2, 0x2;     nop
// CHECK: nop;    nop;    get_lut_entry_and_interval_start.f32  tanh D4, V2, 0x3;     nop
// CHECK: nop;    nop;    get_lut_entry_and_interval_start.f32  tanh D4, V2, 0x1c;    nop
// CHECK: nop;    nop;    get_lut_entry_and_interval_start.f32  tanh D4, V2, 0x1d;    nop
// CHECK: nop;    nop;    get_lut_entry_and_interval_start.f32  tanh D4, V2, 0x1e;    nop
// CHECK: nop;    nop;    get_lut_entry_and_interval_start.f32  tanh D4, V2, 0x1f;    nop


NOP; NOP; GET_LUT_ENTRY_AND_INTERVAL_START.F32 V4, V2, 4, 0; NOP
NOP; NOP; GET_LUT_ENTRY_AND_INTERVAL_START.F32 V4, V2, 4, 1; NOP
NOP; NOP; GET_LUT_ENTRY_AND_INTERVAL_START.F32 V4, V2, 4, 2; NOP
NOP; NOP; GET_LUT_ENTRY_AND_INTERVAL_START.F32 V4, V2, 4, 3; NOP

// CHECK: nop;    nop;    get_lut_entry_and_interval_start.f32  D4, V2, 0x4;         nop
// CHECK: nop;    nop;    get_lut_entry_and_interval_start.f32  tanh D4, V2, 0x4;     nop
// CHECK: nop;    nop;    get_lut_entry_and_interval_start.f32  sqrt_rsqrt D4, V2, 0x4;       nop
// CHECK: nop;    nop;    get_lut_entry_and_interval_start.f32  sin_cos D4, V2, 0x4;  nop


NOP; NOP; GET_LUT_ENTRY_AND_INTERVAL_START.F32 V4, V2, 4; NOP
NOP; NOP; GET_LUT_ENTRY_AND_INTERVAL_START.F32 V4, V2, 4, tanh; NOP
NOP; NOP; GET_LUT_ENTRY_AND_INTERVAL_START.F32 V4, V2, 4, sqrt_rsqrt; NOP
NOP; NOP; GET_LUT_ENTRY_AND_INTERVAL_START.F32 V4, V2, 4, sin_cos; NOP

// CHECK: nop;    nop;    get_lut_entry_and_interval_start.f32  D4, V2, 0x4;         nop
// CHECK: nop;    nop;    get_lut_entry_and_interval_start.f32  tanh D4, V2, 0x4;     nop
// CHECK: nop;    nop;    get_lut_entry_and_interval_start.f32  sqrt_rsqrt D4, V2, 0x4;       nop
// CHECK: nop;    nop;    get_lut_entry_and_interval_start.f32  sin_cos D4, V2, 0x4;  nop
