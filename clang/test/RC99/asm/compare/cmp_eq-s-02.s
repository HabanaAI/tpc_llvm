// RUN: %clang -cc1as -triple tpc-none-none -target-cpu gaudi -filetype obj %s -o %t.o
// RUN: %disasm -mcpu=gaudi %t.o | FileCheck %s




NOP; CMP_EQ.F32.MASK_EQ_ZERO SP1, S2, S3               ; NOP; NOP
NOP; CMP_EQ.F32.MASK_EQ_ZERO SP1, S2, S3, SP4          ; NOP; NOP
NOP; CMP_EQ.F32.MASK_EQ_ZERO SP1, S2, S3, !SP4         ; NOP; NOP
NOP; CMP_EQ.F32.MASK_EQ_ZERO SP1, S2, 0x3f4ccccd       ; NOP; NOP
NOP; CMP_EQ.F32.MASK_EQ_ZERO SP1, S2, 0x3f4ccccd, SP4  ; NOP; NOP
NOP; CMP_EQ.F32.MASK_EQ_ZERO SP1, S2, 0x3f4ccccd, !SP4 ; NOP; NOP

// CHECK: nop;    cmp_eq.f32  mask_eq_zero SP1, S2, S3, SP0;   nop;    nop
// CHECK: nop;    cmp_eq.f32  mask_eq_zero SP1, S2, S3, SP4;   nop;    nop
// CHECK: nop;    cmp_eq.f32  mask_eq_zero SP1, S2, S3, !SP4;  nop;    nop
// CHECK: nop;    cmp_eq.f32  mask_eq_zero SP1, S2, 0x3f4ccccd, SP0;   nop;    nop
// CHECK: nop;    cmp_eq.f32  mask_eq_zero SP1, S2, 0x3f4ccccd, SP4;   nop;    nop
// CHECK: nop;    cmp_eq.f32  mask_eq_zero SP1, S2, 0x3f4ccccd, !SP4;  nop;    nop


NOP; CMP_EQ.I32.MASK_EQ_ZERO SP1, S2, S3               ; NOP; NOP
NOP; CMP_EQ.I32.MASK_EQ_ZERO SP1, S2, S3, SP4          ; NOP; NOP
NOP; CMP_EQ.I32.MASK_EQ_ZERO SP1, S2, S3, !SP4         ; NOP; NOP
NOP; CMP_EQ.I32.MASK_EQ_ZERO SP1, S2, 123              ; NOP; NOP
NOP; CMP_EQ.I32.MASK_EQ_ZERO SP1, S2, 123, SP4         ; NOP; NOP
NOP; CMP_EQ.I32.MASK_EQ_ZERO SP1, S2, 123, !SP4        ; NOP; NOP

// CHECK: nop;    cmp_eq.i32  mask_eq_zero SP1, S2, S3, SP0;   nop;    nop
// CHECK: nop;    cmp_eq.i32  mask_eq_zero SP1, S2, S3, SP4;   nop;    nop
// CHECK: nop;    cmp_eq.i32  mask_eq_zero SP1, S2, S3, !SP4;  nop;    nop
// CHECK: nop;    cmp_eq.i32  mask_eq_zero SP1, S2, 0x7b, SP0;         nop;    nop
// CHECK: nop;    cmp_eq.i32  mask_eq_zero SP1, S2, 0x7b, SP4;         nop;    nop
// CHECK: nop;    cmp_eq.i32  mask_eq_zero SP1, S2, 0x7b, !SP4;        nop;    nop


NOP; CMP_EQ.U32.MASK_EQ_ZERO SP1, S2, S3               ; NOP; NOP
NOP; CMP_EQ.U32.MASK_EQ_ZERO SP1, S2, S3, SP4          ; NOP; NOP
NOP; CMP_EQ.U32.MASK_EQ_ZERO SP1, S2, S3, !SP4         ; NOP; NOP
NOP; CMP_EQ.U32.MASK_EQ_ZERO SP1, S2, 123              ; NOP; NOP
NOP; CMP_EQ.U32.MASK_EQ_ZERO SP1, S2, 123, SP4         ; NOP; NOP
NOP; CMP_EQ.U32.MASK_EQ_ZERO SP1, S2, 123, !SP4        ; NOP; NOP

// CHECK: nop;    cmp_eq.u32  mask_eq_zero SP1, S2, S3, SP0;   nop;    nop
// CHECK: nop;    cmp_eq.u32  mask_eq_zero SP1, S2, S3, SP4;   nop;    nop
// CHECK: nop;    cmp_eq.u32  mask_eq_zero SP1, S2, S3, !SP4;  nop;    nop
// CHECK: nop;    cmp_eq.u32  mask_eq_zero SP1, S2, 0x7b, SP0;         nop;    nop
// CHECK: nop;    cmp_eq.u32  mask_eq_zero SP1, S2, 0x7b, SP4;         nop;    nop
// CHECK: nop;    cmp_eq.u32  mask_eq_zero SP1, S2, 0x7b, !SP4;        nop;    nop


NOP; CMP_EQ.I16.MASK_EQ_ZERO SP1, S2, S3               ; NOP; NOP
NOP; CMP_EQ.I16.MASK_EQ_ZERO SP1, S2, S3, SP4          ; NOP; NOP
NOP; CMP_EQ.I16.MASK_EQ_ZERO SP1, S2, S3, !SP4         ; NOP; NOP
NOP; CMP_EQ.I16.MASK_EQ_ZERO SP1, S2, 123              ; NOP; NOP
NOP; CMP_EQ.I16.MASK_EQ_ZERO SP1, S2, 123, SP4         ; NOP; NOP
NOP; CMP_EQ.I16.MASK_EQ_ZERO SP1, S2, 123, !SP4        ; NOP; NOP

// CHECK: nop;    cmp_eq.i16  mask_eq_zero SP1, S2, S3, SP0;   nop;    nop
// CHECK: nop;    cmp_eq.i16  mask_eq_zero SP1, S2, S3, SP4;   nop;    nop
// CHECK: nop;    cmp_eq.i16  mask_eq_zero SP1, S2, S3, !SP4;  nop;    nop
// CHECK: nop;    cmp_eq.i16  mask_eq_zero SP1, S2, 0x7b, SP0;         nop;    nop
// CHECK: nop;    cmp_eq.i16  mask_eq_zero SP1, S2, 0x7b, SP4;         nop;    nop
// CHECK: nop;    cmp_eq.i16  mask_eq_zero SP1, S2, 0x7b, !SP4;        nop;    nop


NOP; CMP_EQ.U16.MASK_EQ_ZERO SP1, S2, S3               ; NOP; NOP
NOP; CMP_EQ.U16.MASK_EQ_ZERO SP1, S2, S3, SP4          ; NOP; NOP
NOP; CMP_EQ.U16.MASK_EQ_ZERO SP1, S2, S3, !SP4         ; NOP; NOP
NOP; CMP_EQ.U16.MASK_EQ_ZERO SP1, S2, 123              ; NOP; NOP
NOP; CMP_EQ.U16.MASK_EQ_ZERO SP1, S2, 123, SP4         ; NOP; NOP
NOP; CMP_EQ.U16.MASK_EQ_ZERO SP1, S2, 123, !SP4        ; NOP; NOP

// CHECK: nop;    cmp_eq.u16  mask_eq_zero SP1, S2, S3, SP0;   nop;    nop
// CHECK: nop;    cmp_eq.u16  mask_eq_zero SP1, S2, S3, SP4;   nop;    nop
// CHECK: nop;    cmp_eq.u16  mask_eq_zero SP1, S2, S3, !SP4;  nop;    nop
// CHECK: nop;    cmp_eq.u16  mask_eq_zero SP1, S2, 0x7b, SP0;         nop;    nop
// CHECK: nop;    cmp_eq.u16  mask_eq_zero SP1, S2, 0x7b, SP4;         nop;    nop
// CHECK: nop;    cmp_eq.u16  mask_eq_zero SP1, S2, 0x7b, !SP4;        nop;    nop


NOP; CMP_EQ.I8.MASK_EQ_ZERO SP1, S2, S3               ; NOP; NOP
NOP; CMP_EQ.I8.MASK_EQ_ZERO SP1, S2, S3, SP4          ; NOP; NOP
NOP; CMP_EQ.I8.MASK_EQ_ZERO SP1, S2, S3, !SP4         ; NOP; NOP
NOP; CMP_EQ.I8.MASK_EQ_ZERO SP1, S2, 123              ; NOP; NOP
NOP; CMP_EQ.I8.MASK_EQ_ZERO SP1, S2, 123, SP4         ; NOP; NOP
NOP; CMP_EQ.I8.MASK_EQ_ZERO SP1, S2, 123, !SP4        ; NOP; NOP

// CHECK: nop;    cmp_eq.i8  mask_eq_zero SP1, S2, S3, SP0;    nop;    nop
// CHECK: nop;    cmp_eq.i8  mask_eq_zero SP1, S2, S3, SP4;    nop;    nop
// CHECK: nop;    cmp_eq.i8  mask_eq_zero SP1, S2, S3, !SP4;   nop;    nop
// CHECK: nop;    cmp_eq.i8  mask_eq_zero SP1, S2, 0x7b, SP0;  nop;    nop
// CHECK: nop;    cmp_eq.i8  mask_eq_zero SP1, S2, 0x7b, SP4;  nop;    nop
// CHECK: nop;    cmp_eq.i8  mask_eq_zero SP1, S2, 0x7b, !SP4;         nop;    nop


NOP; CMP_EQ.U8.MASK_EQ_ZERO SP1, S2, S3               ; NOP; NOP
NOP; CMP_EQ.U8.MASK_EQ_ZERO SP1, S2, S3, SP4          ; NOP; NOP
NOP; CMP_EQ.U8.MASK_EQ_ZERO SP1, S2, S3, !SP4         ; NOP; NOP
NOP; CMP_EQ.U8.MASK_EQ_ZERO SP1, S2, 123              ; NOP; NOP
NOP; CMP_EQ.U8.MASK_EQ_ZERO SP1, S2, 123, SP4         ; NOP; NOP
NOP; CMP_EQ.U8.MASK_EQ_ZERO SP1, S2, 123, !SP4        ; NOP; NOP

// CHECK: nop;    cmp_eq.u8  mask_eq_zero SP1, S2, S3, SP0;    nop;    nop
// CHECK: nop;    cmp_eq.u8  mask_eq_zero SP1, S2, S3, SP4;    nop;    nop
// CHECK: nop;    cmp_eq.u8  mask_eq_zero SP1, S2, S3, !SP4;   nop;    nop
// CHECK: nop;    cmp_eq.u8  mask_eq_zero SP1, S2, 0x7b, SP0;  nop;    nop
// CHECK: nop;    cmp_eq.u8  mask_eq_zero SP1, S2, 0x7b, SP4;  nop;    nop
// CHECK: nop;    cmp_eq.u8  mask_eq_zero SP1, S2, 0x7b, !SP4;         nop;    nop
