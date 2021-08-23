// RUN: %clang -cc1as -triple tpc-none-none -filetype obj %s -o %t.o
// RUN: %disasm %t.o | FileCheck %s
// RUN: %clang -cc1as -triple tpc-none-none -target-cpu gaudi -filetype obj %s -o %t.o
// RUN: %disasm -mcpu=gaudi %t.o | FileCheck %s




NOP; CMP_LESS.F32 SP1, S2, S3               ; NOP; NOP
NOP; CMP_LESS.F32 SP1, S2, S3, SP4          ; NOP; NOP
NOP; CMP_LESS.F32 SP1, S2, S3, !SP4         ; NOP; NOP
NOP; CMP_LESS.F32 SP1, S2, 0x3f4ccccd       ; NOP; NOP
NOP; CMP_LESS.F32 SP1, S2, 0x3f4ccccd, SP4  ; NOP; NOP
NOP; CMP_LESS.F32 SP1, S2, 0x3f4ccccd, !SP4 ; NOP; NOP

// CHECK: nop;    cmp_less.f32  SP1, S2, S3, SP0;   nop;    nop
// CHECK: nop;    cmp_less.f32  SP1, S2, S3, SP4;   nop;    nop
// CHECK: nop;    cmp_less.f32  SP1, S2, S3, !SP4;  nop;    nop
// CHECK: nop;    cmp_less.f32  SP1, S2, 0x3f4ccccd, SP0;   nop;    nop
// CHECK: nop;    cmp_less.f32  SP1, S2, 0x3f4ccccd, SP4;   nop;    nop
// CHECK: nop;    cmp_less.f32  SP1, S2, 0x3f4ccccd, !SP4;  nop;    nop


NOP; CMP_LESS.I32 SP1, S2, S3               ; NOP; NOP
NOP; CMP_LESS.I32 SP1, S2, S3, SP4          ; NOP; NOP
NOP; CMP_LESS.I32 SP1, S2, S3, !SP4         ; NOP; NOP
NOP; CMP_LESS.I32 SP1, S2, 123              ; NOP; NOP
NOP; CMP_LESS.I32 SP1, S2, 123, SP4         ; NOP; NOP
NOP; CMP_LESS.I32 SP1, S2, 123, !SP4        ; NOP; NOP

// CHECK: nop;    cmp_less.i32  SP1, S2, S3, SP0;   nop;    nop
// CHECK: nop;    cmp_less.i32  SP1, S2, S3, SP4;   nop;    nop
// CHECK: nop;    cmp_less.i32  SP1, S2, S3, !SP4;  nop;    nop
// CHECK: nop;    cmp_less.i32  SP1, S2, 0x7b, SP0;         nop;    nop
// CHECK: nop;    cmp_less.i32  SP1, S2, 0x7b, SP4;         nop;    nop
// CHECK: nop;    cmp_less.i32  SP1, S2, 0x7b, !SP4;        nop;    nop


NOP; CMP_LESS.U32 SP1, S2, S3               ; NOP; NOP
NOP; CMP_LESS.U32 SP1, S2, S3, SP4          ; NOP; NOP
NOP; CMP_LESS.U32 SP1, S2, S3, !SP4         ; NOP; NOP
NOP; CMP_LESS.U32 SP1, S2, 123              ; NOP; NOP
NOP; CMP_LESS.U32 SP1, S2, 123, SP4         ; NOP; NOP
NOP; CMP_LESS.U32 SP1, S2, 123, !SP4        ; NOP; NOP

// CHECK: nop;    cmp_less.u32  SP1, S2, S3, SP0;   nop;    nop
// CHECK: nop;    cmp_less.u32  SP1, S2, S3, SP4;   nop;    nop
// CHECK: nop;    cmp_less.u32  SP1, S2, S3, !SP4;  nop;    nop
// CHECK: nop;    cmp_less.u32  SP1, S2, 0x7b, SP0;         nop;    nop
// CHECK: nop;    cmp_less.u32  SP1, S2, 0x7b, SP4;         nop;    nop
// CHECK: nop;    cmp_less.u32  SP1, S2, 0x7b, !SP4;        nop;    nop


NOP; CMP_LESS.I16 SP1, S2, S3               ; NOP; NOP
NOP; CMP_LESS.I16 SP1, S2, S3, SP4          ; NOP; NOP
NOP; CMP_LESS.I16 SP1, S2, S3, !SP4         ; NOP; NOP
NOP; CMP_LESS.I16 SP1, S2, 123              ; NOP; NOP
NOP; CMP_LESS.I16 SP1, S2, 123, SP4         ; NOP; NOP
NOP; CMP_LESS.I16 SP1, S2, 123, !SP4        ; NOP; NOP

// CHECK: nop;    cmp_less.i16  SP1, S2, S3, SP0;   nop;    nop
// CHECK: nop;    cmp_less.i16  SP1, S2, S3, SP4;   nop;    nop
// CHECK: nop;    cmp_less.i16  SP1, S2, S3, !SP4;  nop;    nop
// CHECK: nop;    cmp_less.i16  SP1, S2, 0x7b, SP0;         nop;    nop
// CHECK: nop;    cmp_less.i16  SP1, S2, 0x7b, SP4;         nop;    nop
// CHECK: nop;    cmp_less.i16  SP1, S2, 0x7b, !SP4;        nop;    nop


NOP; CMP_LESS.U16 SP1, S2, S3               ; NOP; NOP
NOP; CMP_LESS.U16 SP1, S2, S3, SP4          ; NOP; NOP
NOP; CMP_LESS.U16 SP1, S2, S3, !SP4         ; NOP; NOP
NOP; CMP_LESS.U16 SP1, S2, 123              ; NOP; NOP
NOP; CMP_LESS.U16 SP1, S2, 123, SP4         ; NOP; NOP
NOP; CMP_LESS.U16 SP1, S2, 123, !SP4        ; NOP; NOP

// CHECK: nop;    cmp_less.u16  SP1, S2, S3, SP0;   nop;    nop
// CHECK: nop;    cmp_less.u16  SP1, S2, S3, SP4;   nop;    nop
// CHECK: nop;    cmp_less.u16  SP1, S2, S3, !SP4;  nop;    nop
// CHECK: nop;    cmp_less.u16  SP1, S2, 0x7b, SP0;         nop;    nop
// CHECK: nop;    cmp_less.u16  SP1, S2, 0x7b, SP4;         nop;    nop
// CHECK: nop;    cmp_less.u16  SP1, S2, 0x7b, !SP4;        nop;    nop


NOP; CMP_LESS.I8 SP1, S2, S3               ; NOP; NOP
NOP; CMP_LESS.I8 SP1, S2, S3, SP4          ; NOP; NOP
NOP; CMP_LESS.I8 SP1, S2, S3, !SP4         ; NOP; NOP
NOP; CMP_LESS.I8 SP1, S2, 123              ; NOP; NOP
NOP; CMP_LESS.I8 SP1, S2, 123, SP4         ; NOP; NOP
NOP; CMP_LESS.I8 SP1, S2, 123, !SP4        ; NOP; NOP

// CHECK: nop;    cmp_less.i8  SP1, S2, S3, SP0;    nop;    nop
// CHECK: nop;    cmp_less.i8  SP1, S2, S3, SP4;    nop;    nop
// CHECK: nop;    cmp_less.i8  SP1, S2, S3, !SP4;   nop;    nop
// CHECK: nop;    cmp_less.i8  SP1, S2, 0x7b, SP0;  nop;    nop
// CHECK: nop;    cmp_less.i8  SP1, S2, 0x7b, SP4;  nop;    nop
// CHECK: nop;    cmp_less.i8  SP1, S2, 0x7b, !SP4;         nop;    nop


NOP; CMP_LESS.U8 SP1, S2, S3               ; NOP; NOP
NOP; CMP_LESS.U8 SP1, S2, S3, SP4          ; NOP; NOP
NOP; CMP_LESS.U8 SP1, S2, S3, !SP4         ; NOP; NOP
NOP; CMP_LESS.U8 SP1, S2, 123              ; NOP; NOP
NOP; CMP_LESS.U8 SP1, S2, 123, SP4         ; NOP; NOP
NOP; CMP_LESS.U8 SP1, S2, 123, !SP4        ; NOP; NOP

// CHECK: nop;    cmp_less.u8  SP1, S2, S3, SP0;    nop;    nop
// CHECK: nop;    cmp_less.u8  SP1, S2, S3, SP4;    nop;    nop
// CHECK: nop;    cmp_less.u8  SP1, S2, S3, !SP4;   nop;    nop
// CHECK: nop;    cmp_less.u8  SP1, S2, 0x7b, SP0;  nop;    nop
// CHECK: nop;    cmp_less.u8  SP1, S2, 0x7b, SP4;  nop;    nop
// CHECK: nop;    cmp_less.u8  SP1, S2, 0x7b, !SP4;         nop;    nop
