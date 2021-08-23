; RUN: llc -march=tpc -mcpu=dali -O0 %s -o - | FileCheck %s

target datalayout = "e-p0:32:32:32-p1:32:32:32-p2:32:32:32-p3:64:64:64-i32:32:32-i16:32:32-i8:32:32-n8:16:32-f16:16:16-f32:32:32-v16:32:32-v32:32:32-v160:32:32-v256:2048:2048-v2048:2048:2048-v4096:2048:2048-v8192:2048:2048"
target triple = "tpc"

target triple = "tpc"
; Function Attrs: noinline nounwind
define dso_local void @func(i32 %x, i32 %dest) #0 {
entry:
  %conv = sitofp i32 %x to float
  %0 = inttoptr i32 %dest to float addrspace(1)*
  store float %conv, float addrspace(1)* %0, align 4
  ret void
}

; CHECK: convert.i32 target_type=fp32  %S{{[0-9]+}}, %S{{[0-9}+}, %SP{{[0-9]+}}
; CHECK: st_l %S{{[0-9]+}}, %S{{[0-9]+}}, %SP{{[0-9]+}}
