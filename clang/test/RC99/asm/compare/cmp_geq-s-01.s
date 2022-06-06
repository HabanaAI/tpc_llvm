// RUN: %clang -cc1as -triple tpc-none-none -filetype obj %s -o %t.o
// RUN: %disasm %t.o | FileCheck %s
// RUN: %clang -cc1as -triple tpc-none-none -target-cpu gaudi -filetype obj %s -o %t.o
// RUN: %disasm --mcpu=gaudi %t.o | FileCheck %s
// RUN: %clang -cc1as -triple tpc-none-none -target-cpu goya2 -filetype obj %s -o %t.o
// RUN: %disasm --mcpu=goya2 %t.o | FileCheck %s


NOP; CMP_GEQ.F32 SP1, S2, S3               ; NOP; NOP
NOP; CMP_GEQ.F32 SP1, S2, S3, SP4          ; NOP; NOP
NOP; CMP_GEQ.F32 SP1, S2, S3, !SP4         ; NOP; NOP
NOP; CMP_GEQ.F32 SP1, S2, 0x3f4ccccd       ; NOP; NOP
NOP; CMP_GEQ.F32 SP1, S2, 0x3f4ccccd, SP4  ; NOP; NOP
NOP; CMP_GEQ.F32 SP1, S2, 0x3f4ccccd, !SP4 ; NOP; NOP

// CHECK: nop;    cmp_geq.f32  SP1, S2, S3;   nop;    nop
// CHECK: nop;    cmp_geq.f32  SP1, S2, S3, SP4;   nop;    nop
// CHECK: nop;    cmp_geq.f32  SP1, S2, S3, !SP4;  nop;    nop
// CHECK: nop;    cmp_geq.f32  SP1, S2, 0x3f4ccccd;   nop;    nop
// CHECK: nop;    cmp_geq.f32  SP1, S2, 0x3f4ccccd, SP4;   nop;    nop
// CHECK: nop;    cmp_geq.f32  SP1, S2, 0x3f4ccccd, !SP4;  nop;    nop


NOP; CMP_GEQ.I32 SP1, S2, S3               ; NOP; NOP
NOP; CMP_GEQ.I32 SP1, S2, S3, SP4          ; NOP; NOP
NOP; CMP_GEQ.I32 SP1, S2, S3, !SP4         ; NOP; NOP
NOP; CMP_GEQ.I32 SP1, S2, 123              ; NOP; NOP
NOP; CMP_GEQ.I32 SP1, S2, 123, SP4         ; NOP; NOP
NOP; CMP_GEQ.I32 SP1, S2, 123, !SP4        ; NOP; NOP

// CHECK: nop;    cmp_geq.i32  SP1, S2, S3;   nop;    nop
// CHECK: nop;    cmp_geq.i32  SP1, S2, S3, SP4;   nop;    nop
// CHECK: nop;    cmp_geq.i32  SP1, S2, S3, !SP4;  nop;    nop
// CHECK: nop;    cmp_geq.i32  SP1, S2, 0x7b;         nop;    nop
// CHECK: nop;    cmp_geq.i32  SP1, S2, 0x7b, SP4;         nop;    nop
// CHECK: nop;    cmp_geq.i32  SP1, S2, 0x7b, !SP4;        nop;    nop


NOP; CMP_GEQ.U32 SP1, S2, S3               ; NOP; NOP
NOP; CMP_GEQ.U32 SP1, S2, S3, SP4          ; NOP; NOP
NOP; CMP_GEQ.U32 SP1, S2, S3, !SP4         ; NOP; NOP
NOP; CMP_GEQ.U32 SP1, S2, 123              ; NOP; NOP
NOP; CMP_GEQ.U32 SP1, S2, 123, SP4         ; NOP; NOP
NOP; CMP_GEQ.U32 SP1, S2, 123, !SP4        ; NOP; NOP

// CHECK: nop;    cmp_geq.u32  SP1, S2, S3;   nop;    nop
// CHECK: nop;    cmp_geq.u32  SP1, S2, S3, SP4;   nop;    nop
// CHECK: nop;    cmp_geq.u32  SP1, S2, S3, !SP4;  nop;    nop
// CHECK: nop;    cmp_geq.u32  SP1, S2, 0x7b;         nop;    nop
// CHECK: nop;    cmp_geq.u32  SP1, S2, 0x7b, SP4;         nop;    nop
// CHECK: nop;    cmp_geq.u32  SP1, S2, 0x7b, !SP4;        nop;    nop


NOP; CMP_GEQ.I16 SP1, S2, S3               ; NOP; NOP
NOP; CMP_GEQ.I16 SP1, S2, S3, SP4          ; NOP; NOP
NOP; CMP_GEQ.I16 SP1, S2, S3, !SP4         ; NOP; NOP
NOP; CMP_GEQ.I16 SP1, S2, 123              ; NOP; NOP
NOP; CMP_GEQ.I16 SP1, S2, 123, SP4         ; NOP; NOP
NOP; CMP_GEQ.I16 SP1, S2, 123, !SP4        ; NOP; NOP

// CHECK: nop;    cmp_geq.i16  SP1, S2, S3;   nop;    nop
// CHECK: nop;    cmp_geq.i16  SP1, S2, S3, SP4;   nop;    nop
// CHECK: nop;    cmp_geq.i16  SP1, S2, S3, !SP4;  nop;    nop
// CHECK: nop;    cmp_geq.i16  SP1, S2, 0x7b;         nop;    nop
// CHECK: nop;    cmp_geq.i16  SP1, S2, 0x7b, SP4;         nop;    nop
// CHECK: nop;    cmp_geq.i16  SP1, S2, 0x7b, !SP4;        nop;    nop


NOP; CMP_GEQ.U16 SP1, S2, S3               ; NOP; NOP
NOP; CMP_GEQ.U16 SP1, S2, S3, SP4          ; NOP; NOP
NOP; CMP_GEQ.U16 SP1, S2, S3, !SP4         ; NOP; NOP
NOP; CMP_GEQ.U16 SP1, S2, 123              ; NOP; NOP
NOP; CMP_GEQ.U16 SP1, S2, 123, SP4         ; NOP; NOP
NOP; CMP_GEQ.U16 SP1, S2, 123, !SP4        ; NOP; NOP

// CHECK: nop;    cmp_geq.u16  SP1, S2, S3;   nop;    nop
// CHECK: nop;    cmp_geq.u16  SP1, S2, S3, SP4;   nop;    nop
// CHECK: nop;    cmp_geq.u16  SP1, S2, S3, !SP4;  nop;    nop
// CHECK: nop;    cmp_geq.u16  SP1, S2, 0x7b;         nop;    nop
// CHECK: nop;    cmp_geq.u16  SP1, S2, 0x7b, SP4;         nop;    nop
// CHECK: nop;    cmp_geq.u16  SP1, S2, 0x7b, !SP4;        nop;    nop


NOP; CMP_GEQ.I8 SP1, S2, S3               ; NOP; NOP
NOP; CMP_GEQ.I8 SP1, S2, S3, SP4          ; NOP; NOP
NOP; CMP_GEQ.I8 SP1, S2, S3, !SP4         ; NOP; NOP
NOP; CMP_GEQ.I8 SP1, S2, 123              ; NOP; NOP
NOP; CMP_GEQ.I8 SP1, S2, 123, SP4         ; NOP; NOP
NOP; CMP_GEQ.I8 SP1, S2, 123, !SP4        ; NOP; NOP

// CHECK: nop;    cmp_geq.i8  SP1, S2, S3;    nop;    nop
// CHECK: nop;    cmp_geq.i8  SP1, S2, S3, SP4;    nop;    nop
// CHECK: nop;    cmp_geq.i8  SP1, S2, S3, !SP4;   nop;    nop
// CHECK: nop;    cmp_geq.i8  SP1, S2, 0x7b;  nop;    nop
// CHECK: nop;    cmp_geq.i8  SP1, S2, 0x7b, SP4;  nop;    nop
// CHECK: nop;    cmp_geq.i8  SP1, S2, 0x7b, !SP4;         nop;    nop


NOP; CMP_GEQ.U8 SP1, S2, S3               ; NOP; NOP
NOP; CMP_GEQ.U8 SP1, S2, S3, SP4          ; NOP; NOP
NOP; CMP_GEQ.U8 SP1, S2, S3, !SP4         ; NOP; NOP
NOP; CMP_GEQ.U8 SP1, S2, 123              ; NOP; NOP
NOP; CMP_GEQ.U8 SP1, S2, 123, SP4         ; NOP; NOP
NOP; CMP_GEQ.U8 SP1, S2, 123, !SP4        ; NOP; NOP

// CHECK: nop;    cmp_geq.u8  SP1, S2, S3;    nop;    nop
// CHECK: nop;    cmp_geq.u8  SP1, S2, S3, SP4;    nop;    nop
// CHECK: nop;    cmp_geq.u8  SP1, S2, S3, !SP4;   nop;    nop
// CHECK: nop;    cmp_geq.u8  SP1, S2, 0x7b;  nop;    nop
// CHECK: nop;    cmp_geq.u8  SP1, S2, 0x7b, SP4;  nop;    nop
// CHECK: nop;    cmp_geq.u8  SP1, S2, 0x7b, !SP4;         nop;    nop
