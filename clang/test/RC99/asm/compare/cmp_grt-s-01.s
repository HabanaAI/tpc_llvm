// RUN: %clang -cc1as -triple tpc-none-none -filetype obj %s -o %t.o
// RUN: %disasm %t.o | FileCheck %s
// RUN: %clang -cc1as -triple tpc-none-none -target-cpu gaudi -filetype obj %s -o %t.o
// RUN: %disasm --mcpu=gaudi %t.o | FileCheck %s
// RUN: %clang -cc1as -triple tpc-none-none -target-cpu goya2 -filetype obj %s -o %t.o
// RUN: %disasm --mcpu=goya2 %t.o | FileCheck %s


NOP; CMP_GRT.F32 SP1, S2, S3               ; NOP; NOP
NOP; CMP_GRT.F32 SP1, S2, S3, SP4          ; NOP; NOP
NOP; CMP_GRT.F32 SP1, S2, S3, !SP4         ; NOP; NOP
NOP; CMP_GRT.F32 SP1, S2, 0x3f4ccccd       ; NOP; NOP
NOP; CMP_GRT.F32 SP1, S2, 0x3f4ccccd, SP4  ; NOP; NOP
NOP; CMP_GRT.F32 SP1, S2, 0x3f4ccccd, !SP4 ; NOP; NOP

// CHECK: nop;    cmp_grt.f32  SP1, S2, S3;   nop;    nop
// CHECK: nop;    cmp_grt.f32  SP1, S2, S3, SP4;   nop;    nop
// CHECK: nop;    cmp_grt.f32  SP1, S2, S3, !SP4;  nop;    nop
// CHECK: nop;    cmp_grt.f32  SP1, S2, 0x3f4ccccd;   nop;    nop
// CHECK: nop;    cmp_grt.f32  SP1, S2, 0x3f4ccccd, SP4;   nop;    nop
// CHECK: nop;    cmp_grt.f32  SP1, S2, 0x3f4ccccd, !SP4;  nop;    nop


NOP; CMP_GRT.I32 SP1, S2, S3               ; NOP; NOP
NOP; CMP_GRT.I32 SP1, S2, S3, SP4          ; NOP; NOP
NOP; CMP_GRT.I32 SP1, S2, S3, !SP4         ; NOP; NOP
NOP; CMP_GRT.I32 SP1, S2, 123              ; NOP; NOP
NOP; CMP_GRT.I32 SP1, S2, 123, SP4         ; NOP; NOP
NOP; CMP_GRT.I32 SP1, S2, 123, !SP4        ; NOP; NOP

// CHECK: nop;    cmp_grt.i32  SP1, S2, S3;   nop;    nop
// CHECK: nop;    cmp_grt.i32  SP1, S2, S3, SP4;   nop;    nop
// CHECK: nop;    cmp_grt.i32  SP1, S2, S3, !SP4;  nop;    nop
// CHECK: nop;    cmp_grt.i32  SP1, S2, 0x7b;         nop;    nop
// CHECK: nop;    cmp_grt.i32  SP1, S2, 0x7b, SP4;         nop;    nop
// CHECK: nop;    cmp_grt.i32  SP1, S2, 0x7b, !SP4;        nop;    nop


NOP; CMP_GRT.U32 SP1, S2, S3               ; NOP; NOP
NOP; CMP_GRT.U32 SP1, S2, S3, SP4          ; NOP; NOP
NOP; CMP_GRT.U32 SP1, S2, S3, !SP4         ; NOP; NOP
NOP; CMP_GRT.U32 SP1, S2, 123              ; NOP; NOP
NOP; CMP_GRT.U32 SP1, S2, 123, SP4         ; NOP; NOP
NOP; CMP_GRT.U32 SP1, S2, 123, !SP4        ; NOP; NOP

// CHECK: nop;    cmp_grt.u32  SP1, S2, S3;   nop;    nop
// CHECK: nop;    cmp_grt.u32  SP1, S2, S3, SP4;   nop;    nop
// CHECK: nop;    cmp_grt.u32  SP1, S2, S3, !SP4;  nop;    nop
// CHECK: nop;    cmp_grt.u32  SP1, S2, 0x7b;         nop;    nop
// CHECK: nop;    cmp_grt.u32  SP1, S2, 0x7b, SP4;         nop;    nop
// CHECK: nop;    cmp_grt.u32  SP1, S2, 0x7b, !SP4;        nop;    nop


NOP; CMP_GRT.I16 SP1, S2, S3               ; NOP; NOP
NOP; CMP_GRT.I16 SP1, S2, S3, SP4          ; NOP; NOP
NOP; CMP_GRT.I16 SP1, S2, S3, !SP4         ; NOP; NOP
NOP; CMP_GRT.I16 SP1, S2, 123              ; NOP; NOP
NOP; CMP_GRT.I16 SP1, S2, 123, SP4         ; NOP; NOP
NOP; CMP_GRT.I16 SP1, S2, 123, !SP4        ; NOP; NOP

// CHECK: nop;    cmp_grt.i16  SP1, S2, S3;   nop;    nop
// CHECK: nop;    cmp_grt.i16  SP1, S2, S3, SP4;   nop;    nop
// CHECK: nop;    cmp_grt.i16  SP1, S2, S3, !SP4;  nop;    nop
// CHECK: nop;    cmp_grt.i16  SP1, S2, 0x7b;         nop;    nop
// CHECK: nop;    cmp_grt.i16  SP1, S2, 0x7b, SP4;         nop;    nop
// CHECK: nop;    cmp_grt.i16  SP1, S2, 0x7b, !SP4;        nop;    nop


NOP; CMP_GRT.U16 SP1, S2, S3               ; NOP; NOP
NOP; CMP_GRT.U16 SP1, S2, S3, SP4          ; NOP; NOP
NOP; CMP_GRT.U16 SP1, S2, S3, !SP4         ; NOP; NOP
NOP; CMP_GRT.U16 SP1, S2, 123              ; NOP; NOP
NOP; CMP_GRT.U16 SP1, S2, 123, SP4         ; NOP; NOP
NOP; CMP_GRT.U16 SP1, S2, 123, !SP4        ; NOP; NOP

// CHECK: nop;    cmp_grt.u16  SP1, S2, S3;   nop;    nop
// CHECK: nop;    cmp_grt.u16  SP1, S2, S3, SP4;   nop;    nop
// CHECK: nop;    cmp_grt.u16  SP1, S2, S3, !SP4;  nop;    nop
// CHECK: nop;    cmp_grt.u16  SP1, S2, 0x7b;         nop;    nop
// CHECK: nop;    cmp_grt.u16  SP1, S2, 0x7b, SP4;         nop;    nop
// CHECK: nop;    cmp_grt.u16  SP1, S2, 0x7b, !SP4;        nop;    nop


NOP; CMP_GRT.I8 SP1, S2, S3               ; NOP; NOP
NOP; CMP_GRT.I8 SP1, S2, S3, SP4          ; NOP; NOP
NOP; CMP_GRT.I8 SP1, S2, S3, !SP4         ; NOP; NOP
NOP; CMP_GRT.I8 SP1, S2, 123              ; NOP; NOP
NOP; CMP_GRT.I8 SP1, S2, 123, SP4         ; NOP; NOP
NOP; CMP_GRT.I8 SP1, S2, 123, !SP4        ; NOP; NOP

// CHECK: nop;    cmp_grt.i8  SP1, S2, S3;    nop;    nop
// CHECK: nop;    cmp_grt.i8  SP1, S2, S3, SP4;    nop;    nop
// CHECK: nop;    cmp_grt.i8  SP1, S2, S3, !SP4;   nop;    nop
// CHECK: nop;    cmp_grt.i8  SP1, S2, 0x7b;  nop;    nop
// CHECK: nop;    cmp_grt.i8  SP1, S2, 0x7b, SP4;  nop;    nop
// CHECK: nop;    cmp_grt.i8  SP1, S2, 0x7b, !SP4;         nop;    nop


NOP; CMP_GRT.U8 SP1, S2, S3               ; NOP; NOP
NOP; CMP_GRT.U8 SP1, S2, S3, SP4          ; NOP; NOP
NOP; CMP_GRT.U8 SP1, S2, S3, !SP4         ; NOP; NOP
NOP; CMP_GRT.U8 SP1, S2, 123              ; NOP; NOP
NOP; CMP_GRT.U8 SP1, S2, 123, SP4         ; NOP; NOP
NOP; CMP_GRT.U8 SP1, S2, 123, !SP4        ; NOP; NOP

// CHECK: nop;    cmp_grt.u8  SP1, S2, S3;    nop;    nop
// CHECK: nop;    cmp_grt.u8  SP1, S2, S3, SP4;    nop;    nop
// CHECK: nop;    cmp_grt.u8  SP1, S2, S3, !SP4;   nop;    nop
// CHECK: nop;    cmp_grt.u8  SP1, S2, 0x7b;  nop;    nop
// CHECK: nop;    cmp_grt.u8  SP1, S2, 0x7b, SP4;  nop;    nop
// CHECK: nop;    cmp_grt.u8  SP1, S2, 0x7b, !SP4;         nop;    nop
