// RUN: %clang -cc1as -triple tpc-none-none -filetype obj %s -o %t.o
// RUN: %disasm %t.o | FileCheck %s
// RUN: %clang -cc1as -triple tpc-none-none -target-cpu gaudi -filetype obj %s -o %t.o
// RUN: %disasm -mcpu=gaudi %t.o | FileCheck %s




nop; nop; sub.f32 V5, V1, V2
nop; nop; sub.f32 V5, V1, V2, SP3
nop; nop; sub.f32 V5, V1, V2, !SP3
nop; nop; sub.f32 V5, V1, V2, VP3
nop; nop; sub.f32 V5, V1, V2, !VP3

//CHECK: nop; nop; sub.f32 V5, V1, V2, SP0; nop
//CHECK: nop; nop; sub.f32 V5, V1, V2, SP3; nop
//CHECK: nop; nop; sub.f32 V5, V1, V2, !SP3; nop
//CHECK: nop; nop; sub.f32 V5, V1, V2, VP3; nop
//CHECK: nop; nop; sub.f32 V5, V1, V2, !VP3; nop

nop; nop; sub.f32 V5, V1, S2
nop; nop; sub.f32 V5, V1, S2, SP3
nop; nop; sub.f32 V5, V1, S2, !SP3
nop; nop; sub.f32 V5, V1, S2, VP3
nop; nop; sub.f32 V5, V1, S2, !VP3

//CHECK: nop; nop; sub.f32 V5, V1, S2, SP0; nop
//CHECK: nop; nop; sub.f32 V5, V1, S2, SP3; nop
//CHECK: nop; nop; sub.f32 V5, V1, S2, !SP3; nop
//CHECK: nop; nop; sub.f32 V5, V1, S2, VP3; nop
//CHECK: nop; nop; sub.f32 V5, V1, S2, !VP3; nop

nop; nop; sub.f32 V5, V1, 0x42e00000
nop; nop; sub.f32 V5, V1, 0x42e00000, SP3
nop; nop; sub.f32 V5, V1, 0x42e00000, !SP3
nop; nop; sub.f32 V5, V1, 0x42e00000, VP3
nop; nop; sub.f32 V5, V1, 0x42e00000, !VP3

//CHECK: nop; nop; sub.f32 V5, V1, 0x42e00000, SP0; nop
//CHECK: nop; nop; sub.f32 V5, V1, 0x42e00000, SP3; nop
//CHECK: nop; nop; sub.f32 V5, V1, 0x42e00000, !SP3; nop
//CHECK: nop; nop; sub.f32 V5, V1, 0x42e00000, VP3; nop
//CHECK: nop; nop; sub.f32 V5, V1, 0x42e00000, !VP3; nop

nop; nop; sub.i32 V5, V1, V2
nop; nop; sub.i32 V5, V1, V2, SP3
nop; nop; sub.i32 V5, V1, V2, !SP3
nop; nop; sub.i32 V5, V1, V2, VP3
nop; nop; sub.i32 V5, V1, V2, !VP3

//CHECK: nop; nop; sub.i32 V5, V1, V2, SP0; nop
//CHECK: nop; nop; sub.i32 V5, V1, V2, SP3; nop
//CHECK: nop; nop; sub.i32 V5, V1, V2, !SP3; nop
//CHECK: nop; nop; sub.i32 V5, V1, V2, VP3; nop
//CHECK: nop; nop; sub.i32 V5, V1, V2, !VP3; nop


nop; nop; sub.i32 V5, V1, S2
nop; nop; sub.i32 V5, V1, S2, SP3
nop; nop; sub.i32 V5, V1, S2, !SP3
nop; nop; sub.i32 V5, V1, S2, VP3
nop; nop; sub.i32 V5, V1, S2, !VP3

//CHECK: nop; nop; sub.i32 V5, V1, S2, SP0; nop
//CHECK: nop; nop; sub.i32 V5, V1, S2, SP3; nop
//CHECK: nop; nop; sub.i32 V5, V1, S2, !SP3; nop
//CHECK: nop; nop; sub.i32 V5, V1, S2, VP3; nop
//CHECK: nop; nop; sub.i32 V5, V1, S2, !VP3; nop

nop; nop; sub.i32 V5, V1, i32_112
nop; nop; sub.i32 V5, V1, i32_112, SP3
nop; nop; sub.i32 V5, V1, i32_112, !SP3
nop; nop; sub.i32 V5, V1, i32_112, VP3
nop; nop; sub.i32 V5, V1, i32_112, !VP3

//CHECK: nop; nop; sub.i32 V5, V1, 0x70, SP0; nop
//CHECK: nop; nop; sub.i32 V5, V1, 0x70, SP3; nop
//CHECK: nop; nop; sub.i32 V5, V1, 0x70, !SP3; nop
//CHECK: nop; nop; sub.i32 V5, V1, 0x70, VP3; nop
//CHECK: nop; nop; sub.i32 V5, V1, 0x70, !VP3; nop

nop; nop; sub.i16 V5, V1, V2
nop; nop; sub.i16 V5, V1, V2, SP3
nop; nop; sub.i16 V5, V1, V2, !SP3
nop; nop; sub.i16 V5, V1, V2, VP3
nop; nop; sub.i16 V5, V1, V2, !VP3

//CHECK: nop; nop; sub.i16 V5, V1, V2, SP0; nop
//CHECK: nop; nop; sub.i16 V5, V1, V2, SP3; nop
//CHECK: nop; nop; sub.i16 V5, V1, V2, !SP3; nop
//CHECK: nop; nop; sub.i16 V5, V1, V2, VP3; nop
//CHECK: nop; nop; sub.i16 V5, V1, V2, !VP3; nop

nop; nop; sub.i16 V5, V1, S2
nop; nop; sub.i16 V5, V1, S2, SP3
nop; nop; sub.i16 V5, V1, S2, !SP3
nop; nop; sub.i16 V5, V1, S2, VP3
nop; nop; sub.i16 V5, V1, S2, !VP3

//CHECK: nop; nop; sub.i16 V5, V1, S2, SP0; nop
//CHECK: nop; nop; sub.i16 V5, V1, S2, SP3; nop
//CHECK: nop; nop; sub.i16 V5, V1, S2, !SP3; nop
//CHECK: nop; nop; sub.i16 V5, V1, S2, VP3; nop
//CHECK: nop; nop; sub.i16 V5, V1, S2, !VP3; nop

nop; nop; sub.i16 V5, V1, i16_112
nop; nop; sub.i16 V5, V1, i16_112, SP3
nop; nop; sub.i16 V5, V1, i16_112, !SP3
nop; nop; sub.i16 V5, V1, i16_112, VP3
nop; nop; sub.i16 V5, V1, i16_112, !VP3

//CHECK: nop; nop; sub.i16 V5, V1, 0x70, SP0; nop
//CHECK: nop; nop; sub.i16 V5, V1, 0x70, SP3; nop
//CHECK: nop; nop; sub.i16 V5, V1, 0x70, !SP3; nop
//CHECK: nop; nop; sub.i16 V5, V1, 0x70, VP3; nop
//CHECK: nop; nop; sub.i16 V5, V1, 0x70, !VP3; nop

nop; nop; sub.i8 V5, V1, V2
nop; nop; sub.i8 V5, V1, V2, SP3
nop; nop; sub.i8 V5, V1, V2, !SP3
nop; nop; sub.i8 V5, V1, V2, VP3
nop; nop; sub.i8 V5, V1, V2, !VP3

//CHECK: nop; nop; sub.i8 V5, V1, V2, SP0; nop
//CHECK: nop; nop; sub.i8 V5, V1, V2, SP3; nop
//CHECK: nop; nop; sub.i8 V5, V1, V2, !SP3; nop
//CHECK: nop; nop; sub.i8 V5, V1, V2, VP3; nop
//CHECK: nop; nop; sub.i8 V5, V1, V2, !VP3; nop

nop; nop; sub.i8 V5, V1, S2
nop; nop; sub.i8 V5, V1, S2, SP3
nop; nop; sub.i8 V5, V1, S2, !SP3
nop; nop; sub.i8 V5, V1, S2, VP3
nop; nop; sub.i8 V5, V1, S2, !VP3

//CHECK: nop; nop; sub.i8 V5, V1, S2, SP0; nop
//CHECK: nop; nop; sub.i8 V5, V1, S2, SP3; nop
//CHECK: nop; nop; sub.i8 V5, V1, S2, !SP3; nop
//CHECK: nop; nop; sub.i8 V5, V1, S2, VP3; nop
//CHECK: nop; nop; sub.i8 V5, V1, S2, !VP3; nop

nop; nop; sub.i8 V5, V1, i8_112
nop; nop; sub.i8 V5, V1, i8_112, SP3
nop; nop; sub.i8 V5, V1, i8_112, !SP3
nop; nop; sub.i8 V5, V1, i8_112, VP3
nop; nop; sub.i8 V5, V1, i8_112, !VP3

//CHECK: nop; nop; sub.i8 V5, V1, 0x70, SP0; nop
//CHECK: nop; nop; sub.i8 V5, V1, 0x70, SP3; nop
//CHECK: nop; nop; sub.i8 V5, V1, 0x70, !SP3; nop
//CHECK: nop; nop; sub.i8 V5, V1, 0x70, VP3; nop
//CHECK: nop; nop; sub.i8 V5, V1, 0x70, !VP3; nop

nop; nop; sub.u32 V5, V1, V2
nop; nop; sub.u32 V5, V1, V2, SP3
nop; nop; sub.u32 V5, V1, V2, !SP3
nop; nop; sub.u32 V5, V1, V2, VP3
nop; nop; sub.u32 V5, V1, V2, !VP3

//CHECK: nop; nop; sub.u32 V5, V1, V2, SP0; nop
//CHECK: nop; nop; sub.u32 V5, V1, V2, SP3; nop
//CHECK: nop; nop; sub.u32 V5, V1, V2, !SP3; nop
//CHECK: nop; nop; sub.u32 V5, V1, V2, VP3; nop
//CHECK: nop; nop; sub.u32 V5, V1, V2, !VP3; nop

nop; nop; sub.u32 V5, V1, S2
nop; nop; sub.u32 V5, V1, S2, SP3
nop; nop; sub.u32 V5, V1, S2, !SP3
nop; nop; sub.u32 V5, V1, S2, VP3
nop; nop; sub.u32 V5, V1, S2, !VP3

//CHECK: nop; nop; sub.u32 V5, V1, S2, SP0; nop
//CHECK: nop; nop; sub.u32 V5, V1, S2, SP3; nop
//CHECK: nop; nop; sub.u32 V5, V1, S2, !SP3; nop
//CHECK: nop; nop; sub.u32 V5, V1, S2, VP3; nop
//CHECK: nop; nop; sub.u32 V5, V1, S2, !VP3; nop

nop; nop; sub.u32 V5, V1, u32_112
nop; nop; sub.u32 V5, V1, u32_112, SP3
nop; nop; sub.u32 V5, V1, u32_112, !SP3
nop; nop; sub.u32 V5, V1, u32_112, VP3
nop; nop; sub.u32 V5, V1, u32_112, !VP3

//CHECK: nop; nop; sub.u32 V5, V1, 0x70, SP0; nop
//CHECK: nop; nop; sub.u32 V5, V1, 0x70, SP3; nop
//CHECK: nop; nop; sub.u32 V5, V1, 0x70, !SP3; nop
//CHECK: nop; nop; sub.u32 V5, V1, 0x70, VP3; nop
//CHECK: nop; nop; sub.u32 V5, V1, 0x70, !VP3; nop

nop; nop; sub.u16 V5, V1, V2
nop; nop; sub.u16 V5, V1, V2, SP3
nop; nop; sub.u16 V5, V1, V2, !SP3
nop; nop; sub.u16 V5, V1, V2, VP3
nop; nop; sub.u16 V5, V1, V2, !VP3

//CHECK: nop; nop; sub.u16 V5, V1, V2, SP0; nop
//CHECK: nop; nop; sub.u16 V5, V1, V2, SP3; nop
//CHECK: nop; nop; sub.u16 V5, V1, V2, !SP3; nop
//CHECK: nop; nop; sub.u16 V5, V1, V2, VP3; nop
//CHECK: nop; nop; sub.u16 V5, V1, V2, !VP3; nop

nop; nop; sub.u16 V5, V1, S2
nop; nop; sub.u16 V5, V1, S2, SP3
nop; nop; sub.u16 V5, V1, S2, !SP3
nop; nop; sub.u16 V5, V1, S2, VP3
nop; nop; sub.u16 V5, V1, S2, !VP3

//CHECK: nop; nop; sub.u16 V5, V1, S2, SP0; nop
//CHECK: nop; nop; sub.u16 V5, V1, S2, SP3; nop
//CHECK: nop; nop; sub.u16 V5, V1, S2, !SP3; nop
//CHECK: nop; nop; sub.u16 V5, V1, S2, VP3; nop
//CHECK: nop; nop; sub.u16 V5, V1, S2, !VP3; nop

nop; nop; sub.u16 V5, V1, u16_112
nop; nop; sub.u16 V5, V1, u16_112, SP3
nop; nop; sub.u16 V5, V1, u16_112, !SP3
nop; nop; sub.u16 V5, V1, u16_112, VP3
nop; nop; sub.u16 V5, V1, u16_112, !VP3

//CHECK: nop; nop; sub.u16 V5, V1, 0x70, SP0; nop
//CHECK: nop; nop; sub.u16 V5, V1, 0x70, SP3; nop
//CHECK: nop; nop; sub.u16 V5, V1, 0x70, !SP3; nop
//CHECK: nop; nop; sub.u16 V5, V1, 0x70, VP3; nop
//CHECK: nop; nop; sub.u16 V5, V1, 0x70, !VP3; nop

nop; nop; sub.u8 V5, V1, V2
nop; nop; sub.u8 V5, V1, V2, SP3
nop; nop; sub.u8 V5, V1, V2, !SP3
nop; nop; sub.u8 V5, V1, V2, VP3
nop; nop; sub.u8 V5, V1, V2, !VP3

//CHECK: nop; nop; sub.u8 V5, V1, V2, SP0; nop
//CHECK: nop; nop; sub.u8 V5, V1, V2, SP3; nop
//CHECK: nop; nop; sub.u8 V5, V1, V2, !SP3; nop
//CHECK: nop; nop; sub.u8 V5, V1, V2, VP3; nop
//CHECK: nop; nop; sub.u8 V5, V1, V2, !VP3; nop

nop; nop; sub.u8 V5, V1, S2
nop; nop; sub.u8 V5, V1, S2, SP3
nop; nop; sub.u8 V5, V1, S2, !SP3
nop; nop; sub.u8 V5, V1, S2, VP3
nop; nop; sub.u8 V5, V1, S2, !VP3

//CHECK: nop; nop; sub.u8 V5, V1, S2, SP0; nop
//CHECK: nop; nop; sub.u8 V5, V1, S2, SP3; nop
//CHECK: nop; nop; sub.u8 V5, V1, S2, !SP3; nop
//CHECK: nop; nop; sub.u8 V5, V1, S2, VP3; nop
//CHECK: nop; nop; sub.u8 V5, V1, S2, !VP3; nop

nop; nop; sub.u8 V5, V1, u8_112
nop; nop; sub.u8 V5, V1, u8_112, SP3
nop; nop; sub.u8 V5, V1, u8_112, !SP3
nop; nop; sub.u8 V5, V1, u8_112, VP3
nop; nop; sub.u8 V5, V1, u8_112, !VP3

//CHECK: nop; nop; sub.u8 V5, V1, 0x70, SP0; nop
//CHECK: nop; nop; sub.u8 V5, V1, 0x70, SP3; nop
//CHECK: nop; nop; sub.u8 V5, V1, 0x70, !SP3; nop
//CHECK: nop; nop; sub.u8 V5, V1, 0x70, VP3; nop
//CHECK: nop; nop; sub.u8 V5, V1, 0x70, !VP3; nop

nop; nop; sub.f32 neg V5, V1, V2
nop; nop; sub.f32 neg V5, V1, V2, SP3
nop; nop; sub.f32 neg V5, V1, V2, !SP3
nop; nop; sub.f32 neg V5, V1, V2, VP3
nop; nop; sub.f32 neg V5, V1, V2, !VP3

//CHECK: nop; nop; sub.f32 neg V5, V1, V2, SP0; nop
//CHECK: nop; nop; sub.f32 neg V5, V1, V2, SP3; nop
//CHECK: nop; nop; sub.f32 neg V5, V1, V2, !SP3; nop
//CHECK: nop; nop; sub.f32 neg V5, V1, V2, VP3; nop
//CHECK: nop; nop; sub.f32 neg V5, V1, V2, !VP3; nop

nop; nop; sub.f32 neg V5, V1, S2
nop; nop; sub.f32 neg V5, V1, S2, SP3
nop; nop; sub.f32 neg V5, V1, S2, !SP3
nop; nop; sub.f32 neg V5, V1, S2, VP3
nop; nop; sub.f32 neg V5, V1, S2, !VP3

//CHECK: nop; nop; sub.f32 neg V5, V1, S2, SP0; nop
//CHECK: nop; nop; sub.f32 neg V5, V1, S2, SP3; nop
//CHECK: nop; nop; sub.f32 neg V5, V1, S2, !SP3; nop
//CHECK: nop; nop; sub.f32 neg V5, V1, S2, VP3; nop
//CHECK: nop; nop; sub.f32 neg V5, V1, S2, !VP3; nop

nop; nop; sub.f32 neg V5, V1, 0x42e00000
nop; nop; sub.f32 neg V5, V1, 0x42e00000, SP3
nop; nop; sub.f32 neg V5, V1, 0x42e00000, !SP3
nop; nop; sub.f32 neg V5, V1, 0x42e00000, VP3
nop; nop; sub.f32 neg V5, V1, 0x42e00000, !VP3

//CHECK: nop; nop; sub.f32 neg V5, V1, 0x42e00000, SP0; nop
//CHECK: nop; nop; sub.f32 neg V5, V1, 0x42e00000, SP3; nop
//CHECK: nop; nop; sub.f32 neg V5, V1, 0x42e00000, !SP3; nop
//CHECK: nop; nop; sub.f32 neg V5, V1, 0x42e00000, VP3; nop
//CHECK: nop; nop; sub.f32 neg V5, V1, 0x42e00000, !VP3; nop

nop; nop; sub.i32 st V5, V1, V2
nop; nop; sub.i32 st V5, V1, V2, SP3
nop; nop; sub.i32 st V5, V1, V2, !SP3
nop; nop; sub.i32 st V5, V1, V2, VP3
nop; nop; sub.i32 st V5, V1, V2, !VP3

//CHECK: nop; nop; sub.i32 st V5, V1, V2, SP0; nop
//CHECK: nop; nop; sub.i32 st V5, V1, V2, SP3; nop
//CHECK: nop; nop; sub.i32 st V5, V1, V2, !SP3; nop
//CHECK: nop; nop; sub.i32 st V5, V1, V2, VP3; nop
//CHECK: nop; nop; sub.i32 st V5, V1, V2, !VP3; nop


nop; nop; sub.i32 st V5, V1, S2
nop; nop; sub.i32 st V5, V1, S2, SP3
nop; nop; sub.i32 st V5, V1, S2, !SP3
nop; nop; sub.i32 st V5, V1, S2, VP3
nop; nop; sub.i32 st V5, V1, S2, !VP3

//CHECK: nop; nop; sub.i32 st V5, V1, S2, SP0; nop
//CHECK: nop; nop; sub.i32 st V5, V1, S2, SP3; nop
//CHECK: nop; nop; sub.i32 st V5, V1, S2, !SP3; nop
//CHECK: nop; nop; sub.i32 st V5, V1, S2, VP3; nop
//CHECK: nop; nop; sub.i32 st V5, V1, S2, !VP3; nop

nop; nop; sub.i32 st V5, V1, i32_112
nop; nop; sub.i32 st V5, V1, i32_112, SP3
nop; nop; sub.i32 st V5, V1, i32_112, !SP3
nop; nop; sub.i32 st V5, V1, i32_112, VP3
nop; nop; sub.i32 st V5, V1, i32_112, !VP3

//CHECK: nop; nop; sub.i32 st V5, V1, 0x70, SP0; nop
//CHECK: nop; nop; sub.i32 st V5, V1, 0x70, SP3; nop
//CHECK: nop; nop; sub.i32 st V5, V1, 0x70, !SP3; nop
//CHECK: nop; nop; sub.i32 st V5, V1, 0x70, VP3; nop
//CHECK: nop; nop; sub.i32 st V5, V1, 0x70, !VP3; nop

nop; nop; sub.i16 st V5, V1, V2
nop; nop; sub.i16 st V5, V1, V2, SP3
nop; nop; sub.i16 st V5, V1, V2, !SP3
nop; nop; sub.i16 st V5, V1, V2, VP3
nop; nop; sub.i16 st V5, V1, V2, !VP3

//CHECK: nop; nop; sub.i16 st V5, V1, V2, SP0; nop
//CHECK: nop; nop; sub.i16 st V5, V1, V2, SP3; nop
//CHECK: nop; nop; sub.i16 st V5, V1, V2, !SP3; nop
//CHECK: nop; nop; sub.i16 st V5, V1, V2, VP3; nop
//CHECK: nop; nop; sub.i16 st V5, V1, V2, !VP3; nop

nop; nop; sub.i16 st V5, V1, S2
nop; nop; sub.i16 st V5, V1, S2, SP3
nop; nop; sub.i16 st V5, V1, S2, !SP3
nop; nop; sub.i16 st V5, V1, S2, VP3
nop; nop; sub.i16 st V5, V1, S2, !VP3

//CHECK: nop; nop; sub.i16 st V5, V1, S2, SP0; nop
//CHECK: nop; nop; sub.i16 st V5, V1, S2, SP3; nop
//CHECK: nop; nop; sub.i16 st V5, V1, S2, !SP3; nop
//CHECK: nop; nop; sub.i16 st V5, V1, S2, VP3; nop
//CHECK: nop; nop; sub.i16 st V5, V1, S2, !VP3; nop

nop; nop; sub.i16 st V5, V1, i16_112
nop; nop; sub.i16 st V5, V1, i16_112, SP3
nop; nop; sub.i16 st V5, V1, i16_112, !SP3
nop; nop; sub.i16 st V5, V1, i16_112, VP3
nop; nop; sub.i16 st V5, V1, i16_112, !VP3

//CHECK: nop; nop; sub.i16 st V5, V1, 0x70, SP0; nop
//CHECK: nop; nop; sub.i16 st V5, V1, 0x70, SP3; nop
//CHECK: nop; nop; sub.i16 st V5, V1, 0x70, !SP3; nop
//CHECK: nop; nop; sub.i16 st V5, V1, 0x70, VP3; nop
//CHECK: nop; nop; sub.i16 st V5, V1, 0x70, !VP3; nop

nop; nop; sub.i8 st V5, V1, V2
nop; nop; sub.i8 st V5, V1, V2, SP3
nop; nop; sub.i8 st V5, V1, V2, !SP3
nop; nop; sub.i8 st V5, V1, V2, VP3
nop; nop; sub.i8 st V5, V1, V2, !VP3

//CHECK: nop; nop; sub.i8 st V5, V1, V2, SP0; nop
//CHECK: nop; nop; sub.i8 st V5, V1, V2, SP3; nop
//CHECK: nop; nop; sub.i8 st V5, V1, V2, !SP3; nop
//CHECK: nop; nop; sub.i8 st V5, V1, V2, VP3; nop
//CHECK: nop; nop; sub.i8 st V5, V1, V2, !VP3; nop

nop; nop; sub.i8 st V5, V1, S2
nop; nop; sub.i8 st V5, V1, S2, SP3
nop; nop; sub.i8 st V5, V1, S2, !SP3
nop; nop; sub.i8 st V5, V1, S2, VP3
nop; nop; sub.i8 st V5, V1, S2, !VP3

//CHECK: nop; nop; sub.i8 st V5, V1, S2, SP0; nop
//CHECK: nop; nop; sub.i8 st V5, V1, S2, SP3; nop
//CHECK: nop; nop; sub.i8 st V5, V1, S2, !SP3; nop
//CHECK: nop; nop; sub.i8 st V5, V1, S2, VP3; nop
//CHECK: nop; nop; sub.i8 st V5, V1, S2, !VP3; nop

nop; nop; sub.i8 st V5, V1, i8_112
nop; nop; sub.i8 st V5, V1, i8_112, SP3
nop; nop; sub.i8 st V5, V1, i8_112, !SP3
nop; nop; sub.i8 st V5, V1, i8_112, VP3
nop; nop; sub.i8 st V5, V1, i8_112, !VP3

//CHECK: nop; nop; sub.i8 st V5, V1, 0x70, SP0; nop
//CHECK: nop; nop; sub.i8 st V5, V1, 0x70, SP3; nop
//CHECK: nop; nop; sub.i8 st V5, V1, 0x70, !SP3; nop
//CHECK: nop; nop; sub.i8 st V5, V1, 0x70, VP3; nop
//CHECK: nop; nop; sub.i8 st V5, V1, 0x70, !VP3; nop

nop; nop; sub.u32 st V5, V1, V2
nop; nop; sub.u32 st V5, V1, V2, SP3
nop; nop; sub.u32 st V5, V1, V2, !SP3
nop; nop; sub.u32 st V5, V1, V2, VP3
nop; nop; sub.u32 st V5, V1, V2, !VP3

//CHECK: nop; nop; sub.u32 st V5, V1, V2, SP0; nop
//CHECK: nop; nop; sub.u32 st V5, V1, V2, SP3; nop
//CHECK: nop; nop; sub.u32 st V5, V1, V2, !SP3; nop
//CHECK: nop; nop; sub.u32 st V5, V1, V2, VP3; nop
//CHECK: nop; nop; sub.u32 st V5, V1, V2, !VP3; nop

nop; nop; sub.u32 st V5, V1, S2
nop; nop; sub.u32 st V5, V1, S2, SP3
nop; nop; sub.u32 st V5, V1, S2, !SP3
nop; nop; sub.u32 st V5, V1, S2, VP3
nop; nop; sub.u32 st V5, V1, S2, !VP3

//CHECK: nop; nop; sub.u32 st V5, V1, S2, SP0; nop
//CHECK: nop; nop; sub.u32 st V5, V1, S2, SP3; nop
//CHECK: nop; nop; sub.u32 st V5, V1, S2, !SP3; nop
//CHECK: nop; nop; sub.u32 st V5, V1, S2, VP3; nop
//CHECK: nop; nop; sub.u32 st V5, V1, S2, !VP3; nop

nop; nop; sub.u32 st V5, V1, u32_112
nop; nop; sub.u32 st V5, V1, u32_112, SP3
nop; nop; sub.u32 st V5, V1, u32_112, !SP3
nop; nop; sub.u32 st V5, V1, u32_112, VP3
nop; nop; sub.u32 st V5, V1, u32_112, !VP3

//CHECK: nop; nop; sub.u32 st V5, V1, 0x70, SP0; nop
//CHECK: nop; nop; sub.u32 st V5, V1, 0x70, SP3; nop
//CHECK: nop; nop; sub.u32 st V5, V1, 0x70, !SP3; nop
//CHECK: nop; nop; sub.u32 st V5, V1, 0x70, VP3; nop
//CHECK: nop; nop; sub.u32 st V5, V1, 0x70, !VP3; nop

nop; nop; sub.u16 st V5, V1, V2
nop; nop; sub.u16 st V5, V1, V2, SP3
nop; nop; sub.u16 st V5, V1, V2, !SP3
nop; nop; sub.u16 st V5, V1, V2, VP3
nop; nop; sub.u16 st V5, V1, V2, !VP3

//CHECK: nop; nop; sub.u16 st V5, V1, V2, SP0; nop
//CHECK: nop; nop; sub.u16 st V5, V1, V2, SP3; nop
//CHECK: nop; nop; sub.u16 st V5, V1, V2, !SP3; nop
//CHECK: nop; nop; sub.u16 st V5, V1, V2, VP3; nop
//CHECK: nop; nop; sub.u16 st V5, V1, V2, !VP3; nop

nop; nop; sub.u16 st V5, V1, S2
nop; nop; sub.u16 st V5, V1, S2, SP3
nop; nop; sub.u16 st V5, V1, S2, !SP3
nop; nop; sub.u16 st V5, V1, S2, VP3
nop; nop; sub.u16 st V5, V1, S2, !VP3

//CHECK: nop; nop; sub.u16 st V5, V1, S2, SP0; nop
//CHECK: nop; nop; sub.u16 st V5, V1, S2, SP3; nop
//CHECK: nop; nop; sub.u16 st V5, V1, S2, !SP3; nop
//CHECK: nop; nop; sub.u16 st V5, V1, S2, VP3; nop
//CHECK: nop; nop; sub.u16 st V5, V1, S2, !VP3; nop

nop; nop; sub.u16 st V5, V1, u16_112
nop; nop; sub.u16 st V5, V1, u16_112, SP3
nop; nop; sub.u16 st V5, V1, u16_112, !SP3
nop; nop; sub.u16 st V5, V1, u16_112, VP3
nop; nop; sub.u16 st V5, V1, u16_112, !VP3

//CHECK: nop; nop; sub.u16 st V5, V1, 0x70, SP0; nop
//CHECK: nop; nop; sub.u16 st V5, V1, 0x70, SP3; nop
//CHECK: nop; nop; sub.u16 st V5, V1, 0x70, !SP3; nop
//CHECK: nop; nop; sub.u16 st V5, V1, 0x70, VP3; nop
//CHECK: nop; nop; sub.u16 st V5, V1, 0x70, !VP3; nop

nop; nop; sub.u8 st V5, V1, V2
nop; nop; sub.u8 st V5, V1, V2, SP3
nop; nop; sub.u8 st V5, V1, V2, !SP3
nop; nop; sub.u8 st V5, V1, V2, VP3
nop; nop; sub.u8 st V5, V1, V2, !VP3

//CHECK: nop; nop; sub.u8 st V5, V1, V2, SP0; nop
//CHECK: nop; nop; sub.u8 st V5, V1, V2, SP3; nop
//CHECK: nop; nop; sub.u8 st V5, V1, V2, !SP3; nop
//CHECK: nop; nop; sub.u8 st V5, V1, V2, VP3; nop
//CHECK: nop; nop; sub.u8 st V5, V1, V2, !VP3; nop

nop; nop; sub.u8 st V5, V1, S2
nop; nop; sub.u8 st V5, V1, S2, SP3
nop; nop; sub.u8 st V5, V1, S2, !SP3
nop; nop; sub.u8 st V5, V1, S2, VP3
nop; nop; sub.u8 st V5, V1, S2, !VP3

//CHECK: nop; nop; sub.u8 st V5, V1, S2, SP0; nop
//CHECK: nop; nop; sub.u8 st V5, V1, S2, SP3; nop
//CHECK: nop; nop; sub.u8 st V5, V1, S2, !SP3; nop
//CHECK: nop; nop; sub.u8 st V5, V1, S2, VP3; nop
//CHECK: nop; nop; sub.u8 st V5, V1, S2, !VP3; nop

nop; nop; sub.u8 st V5, V1, u8_112
nop; nop; sub.u8 st V5, V1, u8_112, SP3
nop; nop; sub.u8 st V5, V1, u8_112, !SP3
nop; nop; sub.u8 st V5, V1, u8_112, VP3
nop; nop; sub.u8 st V5, V1, u8_112, !VP3

//CHECK: nop; nop; sub.u8 st V5, V1, 0x70, SP0; nop
//CHECK: nop; nop; sub.u8 st V5, V1, 0x70, SP3; nop
//CHECK: nop; nop; sub.u8 st V5, V1, 0x70, !SP3; nop
//CHECK: nop; nop; sub.u8 st V5, V1, 0x70, VP3; nop
//CHECK: nop; nop; sub.u8 st V5, V1, 0x70, !VP3; nop