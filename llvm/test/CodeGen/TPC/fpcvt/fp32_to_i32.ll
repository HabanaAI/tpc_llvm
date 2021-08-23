; RUN: llc -mcpu dali %s -o - | FileCheck %s

target triple = "tpc"

define void @main(i32 %dest, float %x) {
entry:
  %0 = inttoptr i32 %dest to i32 addrspace(1)*
  %1 = fptosi float %x to i32
  store i32 %1, i32 addrspace(1)* %0, align 4
  ret void
}

; CHECK: convert.f32 target_type=int32 [[VAL:%S[0-9]+]], %S1
; CHECK: st_l        %S0, [[VAL]]
