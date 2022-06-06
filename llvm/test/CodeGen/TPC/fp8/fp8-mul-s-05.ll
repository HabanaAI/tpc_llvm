; RUN: llc -mcpu gaudi2 %s -o - | FileCheck %s

target triple = "tpc"

define void @main(f8_143 %x0, f8_143 %x1, i32 %dest) {
entry:
  %0 = inttoptr i32 %dest to float addrspace(1)*
  %x0e = fpext f8_143 %x0 to float
  %x1e = fpext f8_143 %x1 to float
  %1 = fmul float %x0e, %x1e
  store float %1, float addrspace(1)* %0, align 4
  ret void
}

; CHECK: mul.f8_143 acc_fp32 [[VAL:%S[0-9]+]], %S0, %S1
; CHECK: st_l        %S2, [[VAL]]
