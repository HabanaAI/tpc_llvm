; RUN: llc -mcpu gaudi %s -o - | FileCheck %s

target triple = "tpc"

define void @main(i32 %dest, float %x) {
entry:
  %0 = inttoptr i32 %dest to bfloat16 addrspace(1)*
  %1 = fptrunc float %x to bfloat16
  store bfloat16 %1, bfloat16 addrspace(1)* %0, align 4
  ret void
}

; CHECK: convert.f32 target_type=bf16  [[VAL:%S[0-9]+]], %S1
; CHECK: st_l        %S0, [[VAL]]
