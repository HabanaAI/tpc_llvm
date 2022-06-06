// RUN: %clang -cc1as -triple tpc-none-none -target-cpu gaudi -filetype obj %s -o %t.o
// RUN: %disasm --mcpu=gaudi %t.o | FileCheck %s
// RUN: %clang -cc1as -triple tpc-none-none -target-cpu goya2 -filetype obj %s -o %t.o
// RUN: %disasm --mcpu=goya2 %t.o | FileCheck %s


// f32 => bf16
NOP; CONVERT.F32 S0, S1, BF16, RHAZ; NOP; NOP
NOP; CONVERT.F32 S0, S1, BF16, RHAZ, SP1; NOP; NOP
NOP; CONVERT.F32 S0, S1, BF16, RHAZ, !SP1; NOP; NOP

// CHECK: nop;    convert.f32  target_type=bf16 rhaz S0, S1;  nop;    nop
// CHECK: nop;    convert.f32  target_type=bf16 rhaz S0, S1, SP1;  nop;    nop
// CHECK: nop;    convert.f32  target_type=bf16 rhaz S0, S1, !SP1;         nop;    nop


// i32 => bf16
NOP; CONVERT.I32 S0, S1, BF16, RHNE; NOP; NOP
NOP; CONVERT.I32 S0, S1, BF16, RHNE, SP1; NOP; NOP
NOP; CONVERT.I32 S0, S1, BF16, RHNE, !SP1; NOP; NOP

// CHECK: nop;    convert.i32  target_type=bf16 rhne S0, S1;  nop;    nop
// CHECK: nop;    convert.i32  target_type=bf16 rhne S0, S1, SP1;  nop;    nop
// CHECK: nop;    convert.i32  target_type=bf16 rhne S0, S1, !SP1;         nop;    nop


// i16 => bf16
NOP; CONVERT.I16 S0, S1, BF16, RHNE; NOP; NOP
NOP; CONVERT.I16 S0, S1, BF16, RHNE, SP1; NOP; NOP
NOP; CONVERT.I16 S0, S1, BF16, RHNE, !SP1; NOP; NOP

// CHECK: nop;    convert.i16  target_type=bf16 rhne S0, S1;  nop;    nop
// CHECK: nop;    convert.i16  target_type=bf16 rhne S0, S1, SP1;  nop;    nop
// CHECK: nop;    convert.i16  target_type=bf16 rhne S0, S1, !SP1;         nop;    nop


// u16 => bf16
NOP; CONVERT.U16 S0, S1, BF16, RHNE; NOP; NOP
NOP; CONVERT.U16 S0, S1, BF16, RHNE, SP1; NOP; NOP
NOP; CONVERT.U16 S0, S1, BF16, RHNE, !SP1; NOP; NOP

// CHECK: nop;    convert.u16  target_type=bf16 rhne S0, S1;  nop;    nop
// CHECK: nop;    convert.u16  target_type=bf16 rhne S0, S1, SP1;  nop;    nop
// CHECK: nop;    convert.u16  target_type=bf16 rhne S0, S1, !SP1;         nop;    nop


// bf16 => fp32
NOP; CONVERT.BF16 S0, S1, FP32; NOP; NOP
NOP; CONVERT.BF16 S0, S1, FP32, SP1; NOP; NOP
NOP; CONVERT.BF16 S0, S1, FP32, !SP1; NOP; NOP

// CHECK: nop;    convert.bf16 target_type=fp32  S0, S1;      nop;    nop
// CHECK: nop;    convert.bf16 target_type=fp32  S0, S1, SP1;      nop;    nop
// CHECK: nop;    convert.bf16 target_type=fp32  S0, S1, !SP1;     nop;    nop


// bf16 => i16
NOP; CONVERT.BF16 S0, S1, INT16, RU; NOP; NOP
NOP; CONVERT.BF16 S0, S1, INT16, RU, SP1; NOP; NOP
NOP; CONVERT.BF16 S0, S1, INT16, RU, !SP1; NOP; NOP

// CHECK: nop;    convert.bf16  target_type=int16 ru S0, S1;  nop;    nop
// CHECK: nop;    convert.bf16  target_type=int16 ru S0, S1, SP1;  nop;    nop
// CHECK: nop;    convert.bf16  target_type=int16 ru S0, S1, !SP1;         nop;    nop
