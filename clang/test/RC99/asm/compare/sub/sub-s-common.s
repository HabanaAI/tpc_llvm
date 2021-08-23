// RUN: %clang -cc1as -triple tpc-none-none -filetype obj %s -o %t.o
// RUN: %disasm %t.o | FileCheck %s
// RUN: %clang -cc1as -triple tpc-none-none -target-cpu gaudi -filetype obj %s -o %t.o
// RUN: %disasm -mcpu=gaudi %t.o | FileCheck %s




nop; sub.f32 S5, S1, S2
nop; sub.f32 S5, S1, S2, SP3
nop; sub.f32 S5, S1, S2, !SP3
nop; sub.f32 S5, S1, 0x42e00000
nop; sub.f32 S5, S1, 0x42e00000, SP3
nop; sub.f32 S5, S1, 0x42e00000, !SP3

//CHECK: nop; sub.f32 S5, S1, S2, SP0; nop; nop
//CHECK: nop; sub.f32 S5, S1, S2, SP3; nop; nop
//CHECK: nop; sub.f32 S5, S1, S2, !SP3; nop; nop
//CHECK: nop; sub.f32 S5, S1, 0x42e00000, SP0; nop; nop
//CHECK: nop; sub.f32 S5, S1, 0x42e00000, SP3; nop; nop
//CHECK: nop; sub.f32 S5, S1, 0x42e00000, !SP3; nop; nop

nop; sub.i32 S5, S1, S2
nop; sub.i32 S5, S1, S2, SP3
nop; sub.i32 S5, S1, S2, !SP3
nop; sub.i32 S5, S1, i32_112
nop; sub.i32 S5, S1, i32_112, SP3
nop; sub.i32 S5, S1, i32_112, !SP3

//CHECK: nop; sub.i32 S5, S1, S2, SP0; nop; nop
//CHECK: nop; sub.i32 S5, S1, S2, SP3; nop; nop
//CHECK: nop; sub.i32 S5, S1, S2, !SP3; nop; nop
//CHECK: nop; sub.i32 S5, S1, 0x70, SP0; nop; nop
//CHECK: nop; sub.i32 S5, S1, 0x70, SP3; nop; nop
//CHECK: nop; sub.i32 S5, S1, 0x70, !SP3; nop; nop

nop; sub.i16 S5, S1, S2
nop; sub.i16 S5, S1, S2, SP3
nop; sub.i16 S5, S1, S2, !SP3
nop; sub.i16 S5, S1, i16_112
nop; sub.i16 S5, S1, i16_112, SP3
nop; sub.i16 S5, S1, i16_112, !SP3

//CHECK: nop; sub.i16 S5, S1, S2, SP0; nop; nop
//CHECK: nop; sub.i16 S5, S1, S2, SP3; nop; nop
//CHECK: nop; sub.i16 S5, S1, S2, !SP3; nop; nop
//CHECK: nop; sub.i16 S5, S1, 0x70, SP0; nop; nop
//CHECK: nop; sub.i16 S5, S1, 0x70, SP3; nop; nop
//CHECK: nop; sub.i16 S5, S1, 0x70, !SP3; nop; nop

nop; sub.i8 S5, S1, S2
nop; sub.i8 S5, S1, S2, SP3
nop; sub.i8 S5, S1, S2, !SP3
nop; sub.i8 S5, S1, i8_112
nop; sub.i8 S5, S1, i8_112, SP3
nop; sub.i8 S5, S1, i8_112, !SP3

//CHECK: nop; sub.i8 S5, S1, S2, SP0; nop; nop
//CHECK: nop; sub.i8 S5, S1, S2, SP3; nop; nop
//CHECK: nop; sub.i8 S5, S1, S2, !SP3; nop; nop
//CHECK: nop; sub.i8 S5, S1, 0x70, SP0; nop; nop
//CHECK: nop; sub.i8 S5, S1, 0x70, SP3; nop; nop
//CHECK: nop; sub.i8 S5, S1, 0x70, !SP3; nop; nop


nop; sub.u32 S5, S1, S2
nop; sub.u32 S5, S1, S2, SP3
nop; sub.u32 S5, S1, S2, !SP3
nop; sub.u32 S5, S1, u32_112
nop; sub.u32 S5, S1, u32_112, SP3
nop; sub.u32 S5, S1, u32_112, !SP3

//CHECK: nop; sub.u32 S5, S1, S2, SP0; nop; nop
//CHECK: nop; sub.u32 S5, S1, S2, SP3; nop; nop
//CHECK: nop; sub.u32 S5, S1, S2, !SP3; nop; nop
//CHECK: nop; sub.u32 S5, S1, 0x70, SP0; nop; nop
//CHECK: nop; sub.u32 S5, S1, 0x70, SP3; nop; nop
//CHECK: nop; sub.u32 S5, S1, 0x70, !SP3; nop; nop

nop; sub.u16 S5, S1, S2
nop; sub.u16 S5, S1, S2, SP3
nop; sub.u16 S5, S1, S2, !SP3
nop; sub.u16 S5, S1, u16_112
nop; sub.u16 S5, S1, u16_112, SP3
nop; sub.u16 S5, S1, u16_112, !SP3

//CHECK: nop; sub.u16 S5, S1, S2, SP0; nop; nop
//CHECK: nop; sub.u16 S5, S1, S2, SP3; nop; nop
//CHECK: nop; sub.u16 S5, S1, S2, !SP3; nop; nop
//CHECK: nop; sub.u16 S5, S1, 0x70, SP0; nop; nop
//CHECK: nop; sub.u16 S5, S1, 0x70, SP3; nop; nop
//CHECK: nop; sub.u16 S5, S1, 0x70, !SP3; nop; nop

nop; sub.u8 S5, S1, S2
nop; sub.u8 S5, S1, S2, SP3
nop; sub.u8 S5, S1, S2, !SP3
nop; sub.u8 S5, S1, u8_112
nop; sub.u8 S5, S1, u8_112, SP3
nop; sub.u8 S5, S1, u8_112, !SP3

//CHECK: nop; sub.u8 S5, S1, S2, SP0; nop; nop
//CHECK: nop; sub.u8 S5, S1, S2, SP3; nop; nop
//CHECK: nop; sub.u8 S5, S1, S2, !SP3; nop; nop
//CHECK: nop; sub.u8 S5, S1, 0x70, SP0; nop; nop
//CHECK: nop; sub.u8 S5, S1, 0x70, SP3; nop; nop
//CHECK: nop; sub.u8 S5, S1, 0x70, !SP3; nop; nop


nop; sub.f32 neg S5, S1, S2
nop; sub.f32 neg S5, S1, S2, SP3
nop; sub.f32 neg S5, S1, S2, !SP3
nop; sub.f32 neg S5, S1, 0x42e00000
nop; sub.f32 neg S5, S1, 0x42e00000, SP3
nop; sub.f32 neg S5, S1, 0x42e00000, !SP3

//CHECK: nop; sub.f32 neg S5, S1, S2, SP0; nop; nop
//CHECK: nop; sub.f32 neg S5, S1, S2, SP3; nop; nop
//CHECK: nop; sub.f32 neg S5, S1, S2, !SP3; nop; nop
//CHECK: nop; sub.f32 neg S5, S1, 0x42e00000, SP0; nop; nop
//CHECK: nop; sub.f32 neg S5, S1, 0x42e00000, SP3; nop; nop
//CHECK: nop; sub.f32 neg S5, S1, 0x42e00000, !SP3; nop; nop


nop; sub.i32 st S5, S1, S2
nop; sub.i32 st S5, S1, S2, SP3
nop; sub.i32 st S5, S1, S2, !SP3
nop; sub.i32 st S5, S1, i32_112
nop; sub.i32 st S5, S1, i32_112, SP3
nop; sub.i32 st S5, S1, i32_112, !SP3

//CHECK: nop; sub.i32 st S5, S1, S2, SP0; nop; nop
//CHECK: nop; sub.i32 st S5, S1, S2, SP3; nop; nop
//CHECK: nop; sub.i32 st S5, S1, S2, !SP3; nop; nop
//CHECK: nop; sub.i32 st S5, S1, 0x70, SP0; nop; nop
//CHECK: nop; sub.i32 st S5, S1, 0x70, SP3; nop; nop
//CHECK: nop; sub.i32 st S5, S1, 0x70, !SP3; nop; nop

nop; sub.i16 st S5, S1, S2
nop; sub.i16 st S5, S1, S2, SP3
nop; sub.i16 st S5, S1, S2, !SP3
nop; sub.i16 st S5, S1, i16_112
nop; sub.i16 st S5, S1, i16_112, SP3
nop; sub.i16 st S5, S1, i16_112, !SP3

//CHECK: nop; sub.i16 st S5, S1, S2, SP0; nop; nop
//CHECK: nop; sub.i16 st S5, S1, S2, SP3; nop; nop
//CHECK: nop; sub.i16 st S5, S1, S2, !SP3; nop; nop
//CHECK: nop; sub.i16 st S5, S1, 0x70, SP0; nop; nop
//CHECK: nop; sub.i16 st S5, S1, 0x70, SP3; nop; nop
//CHECK: nop; sub.i16 st S5, S1, 0x70, !SP3; nop; nop

nop; sub.i8 st S5, S1, S2
nop; sub.i8 st S5, S1, S2, SP3
nop; sub.i8 st S5, S1, S2, !SP3
nop; sub.i8 st S5, S1, i8_112
nop; sub.i8 st S5, S1, i8_112, SP3
nop; sub.i8 st S5, S1, i8_112, !SP3

//CHECK: nop; sub.i8 st S5, S1, S2, SP0; nop; nop
//CHECK: nop; sub.i8 st S5, S1, S2, SP3; nop; nop
//CHECK: nop; sub.i8 st S5, S1, S2, !SP3; nop; nop
//CHECK: nop; sub.i8 st S5, S1, 0x70, SP0; nop; nop
//CHECK: nop; sub.i8 st S5, S1, 0x70, SP3; nop; nop
//CHECK: nop; sub.i8 st S5, S1, 0x70, !SP3; nop; nop


nop; sub.u32 st S5, S1, S2
nop; sub.u32 st S5, S1, S2, SP3
nop; sub.u32 st S5, S1, S2, !SP3
nop; sub.u32 st S5, S1, u32_112
nop; sub.u32 st S5, S1, u32_112, SP3
nop; sub.u32 st S5, S1, u32_112, !SP3

//CHECK: nop; sub.u32 st S5, S1, S2, SP0; nop; nop
//CHECK: nop; sub.u32 st S5, S1, S2, SP3; nop; nop
//CHECK: nop; sub.u32 st S5, S1, S2, !SP3; nop; nop
//CHECK: nop; sub.u32 st S5, S1, 0x70, SP0; nop; nop
//CHECK: nop; sub.u32 st S5, S1, 0x70, SP3; nop; nop
//CHECK: nop; sub.u32 st S5, S1, 0x70, !SP3; nop; nop

nop; sub.u16 st S5, S1, S2
nop; sub.u16 st S5, S1, S2, SP3
nop; sub.u16 st S5, S1, S2, !SP3
nop; sub.u16 st S5, S1, u16_112
nop; sub.u16 st S5, S1, u16_112, SP3
nop; sub.u16 st S5, S1, u16_112, !SP3

//CHECK: nop; sub.u16 st S5, S1, S2, SP0; nop; nop
//CHECK: nop; sub.u16 st S5, S1, S2, SP3; nop; nop
//CHECK: nop; sub.u16 st S5, S1, S2, !SP3; nop; nop
//CHECK: nop; sub.u16 st S5, S1, 0x70, SP0; nop; nop
//CHECK: nop; sub.u16 st S5, S1, 0x70, SP3; nop; nop
//CHECK: nop; sub.u16 st S5, S1, 0x70, !SP3; nop; nop

nop; sub.u8 st S5, S1, S2
nop; sub.u8 st S5, S1, S2, SP3
nop; sub.u8 st S5, S1, S2, !SP3
nop; sub.u8 st S5, S1, u8_112
nop; sub.u8 st S5, S1, u8_112, SP3
nop; sub.u8 st S5, S1, u8_112, !SP3

//CHECK: nop; sub.u8 st S5, S1, S2, SP0; nop; nop
//CHECK: nop; sub.u8 st S5, S1, S2, SP3; nop; nop
//CHECK: nop; sub.u8 st S5, S1, S2, !SP3; nop; nop
//CHECK: nop; sub.u8 st S5, S1, 0x70, SP0; nop; nop
//CHECK: nop; sub.u8 st S5, S1, 0x70, SP3; nop; nop
//CHECK: nop; sub.u8 st S5, S1, 0x70, !SP3; nop; nop

nop; sub.I32 b11011 I5, I4, I2
nop; sub.I32 b11011 I5, I4, I2, SP1
nop; sub.I32 b11011 I5, I4, I2, !SP1

// CHECK: nop; sub.i32 b11011 I5, I4, I2, SP0; nop; nop
// CHECK: nop; sub.i32 b11011 I5, I4, I2, SP1; nop; nop
// CHECK: nop; sub.i32 b11011 I5, I4, I2, !SP1; nop; nop


nop; sub.I32 b11111 I5, i32_1234, I2
nop; sub.I32 b11111 I5, i32_1234, I2, SP1
nop; sub.I32 b11111 I5, i32_1234, I2, !SP1

// CHECK: nop; sub.i32 b11111 I5, 0x4d2, I2, SP0; nop; nop
// CHECK: nop; sub.i32 b11111 I5, 0x4d2, I2, SP1; nop; nop
// CHECK: nop; sub.i32 b11111 I5, 0x4d2, I2, !SP1; nop; nop


nop; sub.I32 b11011 I5, S5, I2
nop; sub.I32 b11011 I5, S5, I2, SP1
nop; sub.I32 b11011 I5, S5, I2, !SP1

// CHECK: nop; sub.i32 b11011 I5, S5, I2, SP0; nop; nop
// CHECK: nop; sub.i32 b11011 I5, S5, I2, SP1; nop; nop
// CHECK: nop; sub.i32 b11011 I5, S5, I2, !SP1; nop; nop