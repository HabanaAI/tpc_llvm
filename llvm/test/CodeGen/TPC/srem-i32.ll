; RUN: llc -march=tpc -mcpu=dali -O0 -ignore-mem-overflow %s -o - | FileCheck %s --check-prefix=CHECK-DALI
; RUN: llc -march=tpc -mcpu=gaudi -O0 -ignore-mem-overflow %s -o - | FileCheck %s --check-prefix=CHECK-GAUDI
; RUN: llc -march=tpc -mcpu=goya2 -O0 -ignore-mem-overflow %s -o - | FileCheck %s --check-prefix=CHECK-GOYA2

; GAUDI-2184
; XFAIL: *

; CHECK-DALI: xor.i32  %S{{[0-9]+}}, %S{{[0-9]+}}, %S{{[0-9]+}}, %SP{{[0-9]+}}
; CHECK-DALI: and.i32  %S{{[0-9]+}}, %S{{[0-9]+}}, -0x80000000, %SP{{[0-9]+}}
; CHECK-DALI: and.i32  %S{{[0-9]+}}, %S{{[0-9]+}}, -0x80000000, %SP{{[0-9]+}}
; CHECK-DALI: and.i32  %S{{[0-9]+}}, %S{{[0-9]+}}, -0x80000000, %SP{{[0-9]+}}
; CHECK-DALI: mov.u32  %S{{[0-9]+}}, %S{{[0-9]+}}, %SP{{[0-9]+}}
; CHECK-DALI: mov.u32  %S{{[0-9]+}}, 0x0, %SP{{[0-9]+}}
; CHECK-DALI: udiv_step.u32  0x1f %Z{{[0-9]+}}, %S{{[0-9]+}}, %SP{{[0-9]+}}
; CHECK-DALI: udiv_step.u32  0x1e %Z{{[0-9]+}}, %S{{[0-9]+}}, %SP{{[0-9]+}}
; CHECK-DALI: udiv_step.u32  0x1d %Z{{[0-9]+}}, %S{{[0-9]+}}, %SP{{[0-9]+}}
; CHECK-DALI: udiv_step.u32  0x1c %Z{{[0-9]+}}, %S{{[0-9]+}}, %SP{{[0-9]+}}
; CHECK-DALI: udiv_step.u32  0x1b %Z{{[0-9]+}}, %S{{[0-9]+}}, %SP{{[0-9]+}}
; CHECK-DALI: udiv_step.u32  0x1a %Z{{[0-9]+}}, %S{{[0-9]+}}, %SP{{[0-9]+}}
; CHECK-DALI: udiv_step.u32  0x19 %Z{{[0-9]+}}, %S{{[0-9]+}}, %SP{{[0-9]+}}
; CHECK-DALI: udiv_step.u32  0x18 %Z{{[0-9]+}}, %S{{[0-9]+}}, %SP{{[0-9]+}}
; CHECK-DALI: udiv_step.u32  0x17 %Z{{[0-9]+}}, %S{{[0-9]+}}, %SP{{[0-9]+}}
; CHECK-DALI: udiv_step.u32  0x16 %Z{{[0-9]+}}, %S{{[0-9]+}}, %SP{{[0-9]+}}
; CHECK-DALI: udiv_step.u32  0x15 %Z{{[0-9]+}}, %S{{[0-9]+}}, %SP{{[0-9]+}}
; CHECK-DALI: udiv_step.u32  0x14 %Z{{[0-9]+}}, %S{{[0-9]+}}, %SP{{[0-9]+}}
; CHECK-DALI: udiv_step.u32  0x13 %Z{{[0-9]+}}, %S{{[0-9]+}}, %SP{{[0-9]+}}
; CHECK-DALI: udiv_step.u32  0x12 %Z{{[0-9]+}}, %S{{[0-9]+}}, %SP{{[0-9]+}}
; CHECK-DALI: udiv_step.u32  0x11 %Z{{[0-9]+}}, %S{{[0-9]+}}, %SP{{[0-9]+}}
; CHECK-DALI: udiv_step.u32  0x10 %Z{{[0-9]+}}, %S{{[0-9]+}}, %SP{{[0-9]+}}
; CHECK-DALI: udiv_step.u32  0xf %Z{{[0-9]+}}, %S{{[0-9]+}}, %SP{{[0-9]+}}
; CHECK-DALI: udiv_step.u32  0xe %Z{{[0-9]+}}, %S{{[0-9]+}}, %SP{{[0-9]+}}
; CHECK-DALI: udiv_step.u32  0xd %Z{{[0-9]+}}, %S{{[0-9]+}}, %SP{{[0-9]+}}
; CHECK-DALI: udiv_step.u32  0xc %Z{{[0-9]+}}, %S{{[0-9]+}}, %SP{{[0-9]+}}
; CHECK-DALI: udiv_step.u32  0xb %Z{{[0-9]+}}, %S{{[0-9]+}}, %SP{{[0-9]+}}
; CHECK-DALI: udiv_step.u32  0xa %Z{{[0-9]+}}, %S{{[0-9]+}}, %SP{{[0-9]+}}
; CHECK-DALI: udiv_step.u32  0x9 %Z{{[0-9]+}}, %S{{[0-9]+}}, %SP{{[0-9]+}}
; CHECK-DALI: udiv_step.u32  0x8 %Z{{[0-9]+}}, %S{{[0-9]+}}, %SP{{[0-9]+}}
; CHECK-DALI: udiv_step.u32  0x7 %Z{{[0-9]+}}, %S{{[0-9]+}}, %SP{{[0-9]+}}
; CHECK-DALI: udiv_step.u32  0x6 %Z{{[0-9]+}}, %S{{[0-9]+}}, %SP{{[0-9]+}}
; CHECK-DALI: udiv_step.u32  0x5 %Z{{[0-9]+}}, %S{{[0-9]+}}, %SP{{[0-9]+}}
; CHECK-DALI: udiv_step.u32  0x4 %Z{{[0-9]+}}, %S{{[0-9]+}}, %SP{{[0-9]+}}
; CHECK-DALI: udiv_step.u32  0x3 %Z{{[0-9]+}}, %S{{[0-9]+}}, %SP{{[0-9]+}}
; CHECK-DALI: udiv_step.u32  0x2 %Z{{[0-9]+}}, %S{{[0-9]+}}, %SP{{[0-9]+}}
; CHECK-DALI: udiv_step.u32  0x1 %Z{{[0-9]+}}, %S{{[0-9]+}}, %SP{{[0-9]+}}
; CHECK-DALI: or.i32  %S{{[0-9]+}}, %S{{[0-9]+}}, %S{{[0-9]+}}, %SP{{[0-9]+}}

; CHECK-GAUDI: xor.i32  %S{{[0-9]+}}, %S{{[0-9]+}}, %S{{[0-9]+}}, %SP{{[0-9]+}}
; CHECK-GAUDI: and.i32  %S{{[0-9]+}}, %S{{[0-9]+}}, -0x80000000, %SP{{[0-9]+}}
; CHECK-GAUDI: and.i32  %S{{[0-9]+}}, %S{{[0-9]+}}, -0x80000000, %SP{{[0-9]+}}
; CHECK-GAUDI: and.i32  %S{{[0-9]+}}, %S{{[0-9]+}}, -0x80000000, %SP{{[0-9]+}}
; CHECK-GAUDI: mov.u32  %S{{[0-9]+}}, %S{{[0-9]+}}, %SP{{[0-9]+}}
; CHECK-GAUDI: mov.u32  %S{{[0-9]+}}, 0x0, %SP{{[0-9]+}}
; CHECK-GAUDI: udiv_4step.u32  0x1f %Z{{[0-9]+}}, %S{{[0-9]+}}, %SP{{[0-9]+}}
; CHECK-GAUDI: udiv_4step.u32  0x1b %Z{{[0-9]+}}, %S{{[0-9]+}}, %SP{{[0-9]+}}
; CHECK-GAUDI: udiv_4step.u32  0x17 %Z{{[0-9]+}}, %S{{[0-9]+}}, %SP{{[0-9]+}}
; CHECK-GAUDI: udiv_4step.u32  0x13 %Z{{[0-9]+}}, %S{{[0-9]+}}, %SP{{[0-9]+}}
; CHECK-GAUDI: udiv_4step.u32  0xf %Z{{[0-9]+}}, %S{{[0-9]+}}, %SP{{[0-9]+}}
; CHECK-GAUDI: udiv_4step.u32  0xb %Z{{[0-9]+}}, %S{{[0-9]+}}, %SP{{[0-9]+}}
; CHECK-GAUDI: udiv_4step.u32  0x7 %Z{{[0-9]+}}, %S{{[0-9]+}}, %SP{{[0-9]+}}
; CHECK-GAUDI: udiv_4step.u32  0x3 %Z{{[0-9]+}}, %S{{[0-9]+}}, %SP{{[0-9]+}}
; CHECK-GAUDI: or.i32  %S{{[0-9]+}}, %S{{[0-9]+}}, %S{{[0-9]+}}, %SP{{[0-9]+}}

; CHECK-GOYA2: xor.i32  %S{{[0-9]+}}, %S{{[0-9]+}}, %S{{[0-9]+}}, %SP{{[0-9]+}}
; CHECK-GOYA2: and.i32  %S{{[0-9]+}}, %S{{[0-9]+}}, -0x80000000, %SP{{[0-9]+}}
; CHECK-GOYA2: and.i32  %S{{[0-9]+}}, %S{{[0-9]+}}, -0x80000000, %SP{{[0-9]+}}
; CHECK-GOYA2: and.i32  %S{{[0-9]+}}, %S{{[0-9]+}}, -0x80000000, %SP{{[0-9]+}}
; CHECK-GOYA2: mov.u32  %S{{[0-9]+}}, %S{{[0-9]+}}, %SP{{[0-9]+}}
; CHECK-GOYA2: mov.u32  %S{{[0-9]+}}, 0x0, %SP{{[0-9]+}}
; CHECK-GOYA2: udiv_4step.u32 x2 0x1e %Z{{[0-9]+}}, %S{{[0-9]+}}, %SP{{[0-9]+}}
; CHECK-GOYA2: udiv_4step.u32 x2 0x16 %Z{{[0-9]+}}, %S{{[0-9]+}}, %SP{{[0-9]+}}
; CHECK-GOYA2: udiv_4step.u32 x2 0xe %Z{{[0-9]+}}, %S{{[0-9]+}}, %SP{{[0-9]+}}
; CHECK-GOYA2: udiv_4step.u32 x2 0x6 %Z{{[0-9]+}}, %S{{[0-9]+}}, %SP{{[0-9]+}}
; CHECK-GOYA2: or.i32  %S{{[0-9]+}}, %S{{[0-9]+}}, %S{{[0-9]+}}, %SP{{[0-9]+}}

target datalayout = "e-p0:32:32:32-p1:32:32:32-p2:32:32:32-p3:64:64:64-i32:32:32-i16:32:32-i8:32:32-n8:16:32-f16:16:16-f32:32:32-v16:32:32-v32:32:32-v160:32:32-v256:2048:2048-v2048:2048:2048-v4096:2048:2048-v8192:2048:2048"
target triple = "tpc"

%struct._int32_t_pair_t = type { i32, i32 }

; Function Attrs: nounwind
define dso_local void @main(i32 %dest, i32 %dividend, i32 %divisor) local_unnamed_addr {
entry:
  %0 = inttoptr i32 %dest to %struct._int32_t_pair_t addrspace(1)*
  %1 = srem i32 %dividend, %divisor
  %quot_rem.sroa.0.0..sroa_idx = getelementptr inbounds %struct._int32_t_pair_t, %struct._int32_t_pair_t addrspace(1)* %0, i32 0, i32 0
  store i32 %1, i32 addrspace(1)* %quot_rem.sroa.0.0..sroa_idx, align 4
  ret void
}

; Function Attrs: nounwind readnone
declare <2 x i32> @llvm.tpc.udiv.step.v2i32.i32(i32, i32, i8, i32, <2 x i32>, i1, i1)

!llvm.module.flags = !{!0}
!llvm.ident = !{!1}
!llvm.tpc.scalar_data = !{!2}
!llvm.tpc.vector_data = !{!3}

!0 = !{i32 1, !"wchar_size", i32 4}
!1 = !{!"clang version 7.0.0 (ssh://akodnani@gerrit.habana-labs.com:29418/tpc_clang2.git f8db2bf692c43550c8b7d3f6f25aabcdfdb4198c) (ssh://akodnani@gerrit.habana-labs.com:29418/tpc_llvm2.git b4d9d8797fb5853d6c60771365e9b682b78869cb)"}
!2 = !{i32 0}
!3 = !{i32 256}
