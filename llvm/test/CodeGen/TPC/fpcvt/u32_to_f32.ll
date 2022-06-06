; RUN: llc -mcpu gaudi2 %s -o - | FileCheck %s

target triple = "tpc"

define void @main(i32 %dest, i32 %x) {
entry:
  %0 = inttoptr i32 %dest to float addrspace(1)*
  %1 = uitofp i32 %x to float
  store float %1, float addrspace(1)* %0, align 4
  ret void
}

; CHECK: convert.u32 target_type=fp32 [[VAL:%S[0-9]+]], %S1
; CHECK: st_l        %S0, [[VAL]]
