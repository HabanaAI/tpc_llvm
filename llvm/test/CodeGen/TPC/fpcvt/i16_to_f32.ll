; RUN: llc -mcpu dali %s -o - | FileCheck %s

target triple = "tpc"

define void @main(i32 %dest, i16 %x) {
entry:
  %0 = inttoptr i32 %dest to float addrspace(1)*
  %1 = sitofp i16 %x to float
  store float %1, float addrspace(1)* %0, align 4
  ret void
}

; CHECK: convert.i16 target_type=fp32 [[VAL:%S[0-9]+]], %S1
; CHECK: st_l        %S0, [[VAL]]
