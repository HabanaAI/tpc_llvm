; RUN: llc %s -o - | FileCheck %s

target datalayout = "e-p0:32:32:32-p1:32:32:32-p2:32:32:32-p3:64:64:64-i32:32:32-i16:32:32-i8:32:32-n8:16:32-f16:16:16-f32:32:32-v16:32:32-v32:32:32-v160:32:32-v256:2048:2048-v2048:2048:2048-v4096:2048:2048-v8192:2048:2048"
target triple = "tpc"

define void @func(i32 %x, i32 %dest) {
entry:
  %conv = sitofp i32 %x to bfloat16
  %0 = inttoptr i32 %dest to bfloat16 addrspace(1)*
  store bfloat16 %conv, bfloat16 addrspace(1)* %0, align 4
  ret void
}

; CHECK: convert.i32 target_type=bf16 rhne %S0, %S0, %SP0
