// RUN: %clang -cc1as -triple tpc-none-none -filetype obj %s -o %t.o
// RUN: %disasm %t.o | FileCheck %s
// RUN: %clang -cc1as -triple tpc-none-none -target-cpu gaudi -filetype obj %s -o %t.o
// RUN: %disasm --mcpu=gaudi %t.o | FileCheck %s
// RUN: %clang -cc1as -triple tpc-none-none -target-cpu goya2 -filetype obj %s -o %t.o
// RUN: %disasm --mcpu=goya2 %t.o | FileCheck %s


NOP; CMP_NEQ.F32 SP1, S2, S3               ; NOP; NOP
NOP; CMP_NEQ.F32 SP1, S2, S3, SP4          ; NOP; NOP
NOP; CMP_NEQ.F32 SP1, S2, S3, !SP4         ; NOP; NOP
NOP; CMP_NEQ.F32 SP1, S2, 0x3f4ccccd       ; NOP; NOP
NOP; CMP_NEQ.F32 SP1, S2, 0x3f4ccccd, SP4  ; NOP; NOP
NOP; CMP_NEQ.F32 SP1, S2, 0x3f4ccccd, !SP4 ; NOP; NOP

// CHECK: nop;    cmp_neq.f32  SP1, S2, S3;   nop;    nop
// CHECK: nop;    cmp_neq.f32  SP1, S2, S3, SP4;   nop;    nop
// CHECK: nop;    cmp_neq.f32  SP1, S2, S3, !SP4;  nop;    nop
// CHECK: nop;    cmp_neq.f32  SP1, S2, 0x3f4ccccd;   nop;    nop
// CHECK: nop;    cmp_neq.f32  SP1, S2, 0x3f4ccccd, SP4;   nop;    nop
// CHECK: nop;    cmp_neq.f32  SP1, S2, 0x3f4ccccd, !SP4;  nop;    nop


NOP; CMP_NEQ.I32 SP1, S2, S3               ; NOP; NOP
NOP; CMP_NEQ.I32 SP1, S2, S3, SP4          ; NOP; NOP
NOP; CMP_NEQ.I32 SP1, S2, S3, !SP4         ; NOP; NOP
NOP; CMP_NEQ.I32 SP1, S2, 123              ; NOP; NOP
NOP; CMP_NEQ.I32 SP1, S2, 123, SP4         ; NOP; NOP
NOP; CMP_NEQ.I32 SP1, S2, 123, !SP4        ; NOP; NOP

// CHECK: nop;    cmp_neq.i32  SP1, S2, S3;   nop;    nop
// CHECK: nop;    cmp_neq.i32  SP1, S2, S3, SP4;   nop;    nop
// CHECK: nop;    cmp_neq.i32  SP1, S2, S3, !SP4;  nop;    nop
// CHECK: nop;    cmp_neq.i32  SP1, S2, 0x7b;         nop;    nop
// CHECK: nop;    cmp_neq.i32  SP1, S2, 0x7b, SP4;         nop;    nop
// CHECK: nop;    cmp_neq.i32  SP1, S2, 0x7b, !SP4;        nop;    nop


NOP; CMP_NEQ.U32 SP1, S2, S3               ; NOP; NOP
NOP; CMP_NEQ.U32 SP1, S2, S3, SP4          ; NOP; NOP
NOP; CMP_NEQ.U32 SP1, S2, S3, !SP4         ; NOP; NOP
NOP; CMP_NEQ.U32 SP1, S2, 123              ; NOP; NOP
NOP; CMP_NEQ.U32 SP1, S2, 123, SP4         ; NOP; NOP
NOP; CMP_NEQ.U32 SP1, S2, 123, !SP4        ; NOP; NOP

// CHECK: nop;    cmp_neq.u32  SP1, S2, S3;   nop;    nop
// CHECK: nop;    cmp_neq.u32  SP1, S2, S3, SP4;   nop;    nop
// CHECK: nop;    cmp_neq.u32  SP1, S2, S3, !SP4;  nop;    nop
// CHECK: nop;    cmp_neq.u32  SP1, S2, 0x7b;         nop;    nop
// CHECK: nop;    cmp_neq.u32  SP1, S2, 0x7b, SP4;         nop;    nop
// CHECK: nop;    cmp_neq.u32  SP1, S2, 0x7b, !SP4;        nop;    nop


NOP; CMP_NEQ.I16 SP1, S2, S3               ; NOP; NOP
NOP; CMP_NEQ.I16 SP1, S2, S3, SP4          ; NOP; NOP
NOP; CMP_NEQ.I16 SP1, S2, S3, !SP4         ; NOP; NOP
NOP; CMP_NEQ.I16 SP1, S2, 123              ; NOP; NOP
NOP; CMP_NEQ.I16 SP1, S2, 123, SP4         ; NOP; NOP
NOP; CMP_NEQ.I16 SP1, S2, 123, !SP4        ; NOP; NOP

// CHECK: nop;    cmp_neq.i16  SP1, S2, S3;   nop;    nop
// CHECK: nop;    cmp_neq.i16  SP1, S2, S3, SP4;   nop;    nop
// CHECK: nop;    cmp_neq.i16  SP1, S2, S3, !SP4;  nop;    nop
// CHECK: nop;    cmp_neq.i16  SP1, S2, 0x7b;         nop;    nop
// CHECK: nop;    cmp_neq.i16  SP1, S2, 0x7b, SP4;         nop;    nop
// CHECK: nop;    cmp_neq.i16  SP1, S2, 0x7b, !SP4;        nop;    nop


NOP; CMP_NEQ.U16 SP1, S2, S3               ; NOP; NOP
NOP; CMP_NEQ.U16 SP1, S2, S3, SP4          ; NOP; NOP
NOP; CMP_NEQ.U16 SP1, S2, S3, !SP4         ; NOP; NOP
NOP; CMP_NEQ.U16 SP1, S2, 123              ; NOP; NOP
NOP; CMP_NEQ.U16 SP1, S2, 123, SP4         ; NOP; NOP
NOP; CMP_NEQ.U16 SP1, S2, 123, !SP4        ; NOP; NOP

// CHECK: nop;    cmp_neq.u16  SP1, S2, S3;   nop;    nop
// CHECK: nop;    cmp_neq.u16  SP1, S2, S3, SP4;   nop;    nop
// CHECK: nop;    cmp_neq.u16  SP1, S2, S3, !SP4;  nop;    nop
// CHECK: nop;    cmp_neq.u16  SP1, S2, 0x7b;         nop;    nop
// CHECK: nop;    cmp_neq.u16  SP1, S2, 0x7b, SP4;         nop;    nop
// CHECK: nop;    cmp_neq.u16  SP1, S2, 0x7b, !SP4;        nop;    nop


NOP; CMP_NEQ.I8 SP1, S2, S3               ; NOP; NOP
NOP; CMP_NEQ.I8 SP1, S2, S3, SP4          ; NOP; NOP
NOP; CMP_NEQ.I8 SP1, S2, S3, !SP4         ; NOP; NOP
NOP; CMP_NEQ.I8 SP1, S2, 123              ; NOP; NOP
NOP; CMP_NEQ.I8 SP1, S2, 123, SP4         ; NOP; NOP
NOP; CMP_NEQ.I8 SP1, S2, 123, !SP4        ; NOP; NOP

// CHECK: nop;    cmp_neq.i8  SP1, S2, S3;    nop;    nop
// CHECK: nop;    cmp_neq.i8  SP1, S2, S3, SP4;    nop;    nop
// CHECK: nop;    cmp_neq.i8  SP1, S2, S3, !SP4;   nop;    nop
// CHECK: nop;    cmp_neq.i8  SP1, S2, 0x7b;  nop;    nop
// CHECK: nop;    cmp_neq.i8  SP1, S2, 0x7b, SP4;  nop;    nop
// CHECK: nop;    cmp_neq.i8  SP1, S2, 0x7b, !SP4;         nop;    nop


NOP; CMP_NEQ.U8 SP1, S2, S3               ; NOP; NOP
NOP; CMP_NEQ.U8 SP1, S2, S3, SP4          ; NOP; NOP
NOP; CMP_NEQ.U8 SP1, S2, S3, !SP4         ; NOP; NOP
NOP; CMP_NEQ.U8 SP1, S2, 123              ; NOP; NOP
NOP; CMP_NEQ.U8 SP1, S2, 123, SP4         ; NOP; NOP
NOP; CMP_NEQ.U8 SP1, S2, 123, !SP4        ; NOP; NOP

// CHECK: nop;    cmp_neq.u8  SP1, S2, S3;    nop;    nop
// CHECK: nop;    cmp_neq.u8  SP1, S2, S3, SP4;    nop;    nop
// CHECK: nop;    cmp_neq.u8  SP1, S2, S3, !SP4;   nop;    nop
// CHECK: nop;    cmp_neq.u8  SP1, S2, 0x7b;  nop;    nop
// CHECK: nop;    cmp_neq.u8  SP1, S2, 0x7b, SP4;  nop;    nop
// CHECK: nop;    cmp_neq.u8  SP1, S2, 0x7b, !SP4;         nop;    nop
