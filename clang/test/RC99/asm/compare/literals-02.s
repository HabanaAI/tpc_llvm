// RUN: %clang -cc1as -triple tpc-none-none -target-cpu gaudi -filetype obj %s -o %t.o
// RUN: %disasm --mcpu gaudi %t.o | FileCheck %s


NOP; CMP_EQ.I32 SP1, S1, i32_122       ; NOP; NOP
NOP; CMP_EQ.I32 SP1, S1, i32_-122      ; NOP; NOP
NOP; CMP_EQ.I32 SP1, S1, i32_x12C      ; NOP; NOP
NOP; CMP_EQ.U32 SP1, S1, u32_122       ; NOP; NOP
NOP; CMP_EQ.I32 SP1, S1, u32_x12C      ; NOP; NOP
NOP; CMP_EQ.I16 SP1, S1, i16_122       ; NOP; NOP
NOP; CMP_EQ.I16 SP1, S1, i16_-122      ; NOP; NOP
NOP; CMP_EQ.I16 SP1, S1, i16_x12C      ; NOP; NOP
NOP; CMP_EQ.U16 SP1, S1, u16_122       ; NOP; NOP
NOP; CMP_EQ.I8  SP1, S1, i8_122        ; NOP; NOP
NOP; CMP_EQ.I8  SP1, S1, i8_-122       ; NOP; NOP
NOP; CMP_EQ.U8  SP1, S1, u8_122        ; NOP; NOP
NOP; CMP_EQ.F32 SP1, S1, f32_1.25      ; NOP; NOP

// CHECK: nop;    cmp_eq.i32  SP1, S1, 0x7a;         nop;    nop
// CHECK: nop;    cmp_eq.i32  SP1, S1, 0xffffff86;   nop;    nop
// CHECK: nop;    cmp_eq.i32  SP1, S1, 0x12c;        nop;    nop
// CHECK: nop;    cmp_eq.u32  SP1, S1, 0x7a;         nop;    nop
// CHECK: nop;    cmp_eq.i32  SP1, S1, 0x12c;        nop;    nop
// CHECK: nop;    cmp_eq.i16  SP1, S1, 0x7a;         nop;    nop
// CHECK: nop;    cmp_eq.i16  SP1, S1, 0xffffff86;   nop;    nop
// CHECK: nop;    cmp_eq.i16  SP1, S1, 0x12c;        nop;    nop
// CHECK: nop;    cmp_eq.u16  SP1, S1, 0x7a;         nop;    nop
// CHECK: nop;    cmp_eq.i8  SP1, S1, 0x7a;  nop;    nop
// CHECK: nop;    cmp_eq.i8  SP1, S1, 0xffffff86;    nop;    nop
// CHECK: nop;    cmp_eq.u8  SP1, S1, 0x7a;  nop;    nop
// CHECK: nop;    cmp_eq.f32  SP1, S1, 0x3fa00000;   nop;    nop


NOP; MOV b11111 I1, 123

// CHECK: nop;    mov b11111 I1, 0x7b;       nop;    nop

NOP; MOV.F32 S1, 10.1
NOP; MOV.F32 S1, .1
NOP; MOV.F32 S1, 10.

// CHECK: nop; 	mov.f32  S1, 0x4121999a; 	nop; 	nop
// CHECK: nop; 	mov.f32  S1, 0x3dcccccd; 	nop; 	nop
// CHECK: nop; 	mov.f32  S1, 0x41200000; 	nop; 	nop

NOP; MOV.F32 S1, 1e5
NOP; MOV.F32 S1, 1E5

// CHECK: nop; 	mov.f32  S1, 0x47c35000; 	nop; 	nop
// CHECK: nop; 	mov.f32  S1, 0x47c35000; 	nop; 	nop

NOP; MOV.F32 S1, 1e+5
NOP; MOV.F32 S1, 1e-5
NOP; MOV.F32 S1, 1.0e5
NOP; MOV.F32 S1, .1e5
NOP; MOV.F32 S1, 1.e5
NOP; MOV.F32 S1, 1e005

// CHECK: nop; 	mov.f32  S1, 0x47c35000; 	nop; 	nop
// CHECK: nop; 	mov.f32  S1, 0x3727c5ac; 	nop; 	nop
// CHECK: nop; 	mov.f32  S1, 0x47c35000; 	nop; 	nop
// CHECK: nop; 	mov.f32  S1, 0x461c4000; 	nop; 	nop
// CHECK: nop; 	mov.f32  S1, 0x47c35000; 	nop; 	nop
// CHECK: nop; 	mov.f32  S1, 0x47c35000; 	nop; 	nop

NOP; MOV.F32 S1, 1.0f
NOP; MOV.F32 S1, 1.0F
NOP; MOV.F32 S1, 1.0bf
NOP; MOV.F32 S1, 1.0BF
NOP; MOV.F32 S1, 1.0h
NOP; MOV.F32 S1, 1.0H

// CHECK: nop; 	mov.f32  S1, 0x3f800000; 	nop; 	nop
// CHECK: nop; 	mov.f32  S1, 0x3f800000; 	nop; 	nop
// CHECK: nop; 	mov.f32  S1, 0x3f80; 	nop; 	nop
// CHECK: nop; 	mov.f32  S1, 0x3f80; 	nop; 	nop
// CHECK: nop; 	mov.f32  S1, 0x3c00; 	nop; 	nop
// CHECK: nop; 	mov.f32  S1, 0x3c00; 	nop; 	nop

NOP; MOV.F32 S1, -1.0e00
NOP; MOV.F32 S1, - 1.0e00
NOP; MOV.F32 S1, +1.0e00
NOP; MOV.F32 S1, + 1.0e00

// CHECK: nop; 	mov.f32  S1, 0xbf800000; 	nop; 	nop
// CHECK: nop; 	mov.f32  S1, 0xbf800000; 	nop; 	nop
// CHECK: nop; 	mov.f32  S1, 0x3f800000; 	nop; 	nop
// CHECK: nop; 	mov.f32  S1, 0x3f800000; 	nop; 	nop

NOP; MOV.F32 S1, 0.0
NOP; MOV.F32 S1, +0.0
NOP; MOV.F32 S1, -0.0

// CHECK: nop; 	mov.f32  S1, 0x0; 	nop; 	nop
// CHECK: nop; 	mov.f32  S1, 0x0; 	nop; 	nop
// CHECK: nop; 	mov.f32  S1, 0x80000000; 	nop; 	nop

NOP; MOV.I32 S1, 45876u
NOP; MOV.I32 S1, 45876U

// CHECK: nop; 	mov.i32  S1, 0xb334; 	nop; 	nop
// CHECK: nop; 	mov.i32  S1, 0xb334; 	nop; 	nop

NOP; MOV.I32 S1, 0b101
NOP; MOV.I32 S1, 0B101

// CHECK: nop; 	mov.i32  S1, 0x5; 	nop; 	nop
// CHECK: nop; 	mov.i32  S1, 0x5; 	nop; 	nop

NOP; MOV.I32 S1, 0b00101

// CHECK: nop; 	mov.i32  S1, 0x5; 	nop; 	nop

NOP; MOV.I32 S1, -0b101
NOP; MOV.I32 S1, - 0b101
NOP; MOV.I32 S1, +0b101
NOP; MOV.I32 S1, +0b101

// CHECK: nop; 	mov.i32  S1, 0xfffffffb; 	nop; 	nop
// CHECK: nop; 	mov.i32  S1, 0xfffffffb; 	nop; 	nop
// CHECK: nop; 	mov.i32  S1, 0x5; 	nop; 	nop
// CHECK: nop; 	mov.i32  S1, 0x5; 	nop; 	nop

NOP; MOV.I32 S1, 45876
NOP; MOV.I32 S1, +45876
NOP; MOV.I32 S1, + 45876

// CHECK: nop; 	mov.i32  S1, 0xb334; 	nop; 	nop
// CHECK: nop; 	mov.i32  S1, 0xb334; 	nop; 	nop
// CHECK: nop; 	mov.i32  S1, 0xb334; 	nop; 	nop

NOP; MOV.I32 S1, -45876
NOP; MOV.I32 S1, - 45876

// CHECK: nop; 	mov.i32  S1, 0xffff4ccc; 	nop; 	nop
// CHECK: nop; 	mov.i32  S1, 0xffff4ccc; 	nop; 	nop

NOP; MOV.I32 S1, 0xA12B
NOP; MOV.I32 S1, 0XA12B

// CHECK: nop; 	mov.i32  S1, 0xa12b; 	nop; 	nop
// CHECK: nop; 	mov.i32  S1, 0xa12b; 	nop; 	nop

NOP; MOV.I32 S1, 0xA12B
NOP; MOV.I32 S1, 0xa12b
NOP; MOV.I32 S1, 0xA12b
NOP; MOV.I32 S1, 0xa12B

// CHECK: nop; 	mov.i32  S1, 0xa12b; 	nop; 	nop
// CHECK: nop; 	mov.i32  S1, 0xa12b; 	nop; 	nop
// CHECK: nop; 	mov.i32  S1, 0xa12b; 	nop; 	nop
// CHECK: nop; 	mov.i32  S1, 0xa12b; 	nop; 	nop

NOP; MOV.I32 S1, -0xA12B
NOP; MOV.I32 S1, - 0xA12B
NOP; MOV.I32 S1, +0xA12B
NOP; MOV.I32 S1, + 0xA12B

// CHECK: nop; 	mov.i32  S1, 0xffff5ed5; 	nop; 	nop
// CHECK: nop; 	mov.i32  S1, 0xffff5ed5; 	nop; 	nop
// CHECK: nop; 	mov.i32  S1, 0xa12b; 	nop; 	nop
// CHECK: nop; 	mov.i32  S1, 0xa12b; 	nop; 	nop

NOP; MOV.I32 S1, 010
NOP; MOV.I32 S1, 00010

// CHECK: nop; 	mov.i32  S1, 0x8; 	nop; 	nop
// CHECK: nop; 	mov.i32  S1, 0x8; 	nop; 	nop


NOP; MOV.I32 S1, -010
NOP; MOV.I32 S1, - 010
NOP; MOV.I32 S1, +010
NOP; MOV.I32 S1, + 010

// CHECK: nop; 	mov.i32  S1, 0xfffffff8; 	nop; 	nop
// CHECK: nop; 	mov.i32  S1, 0xfffffff8; 	nop; 	nop
// CHECK: nop; 	mov.i32  S1, 0x8; 	nop; 	nop
// CHECK: nop; 	mov.i32  S1, 0x8; 	nop; 	nop


