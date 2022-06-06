; RUN: llc -mcpu gaudi2 %s -o - | FileCheck %s

target triple = "tpc"

define void @main(i32 %dest, float %x, f8_143 %y) {
entry:
  %0 = inttoptr i32 %dest to float addrspace(1)*
  %1 = fpext f8_143 %y to float
  %2 = fadd float %x, %1
  store float %2, float addrspace(1)* %0, align 4
  ret void
}

; CHECK: add.f8_143     [[RES:%S[0-9]+]], %S1, %S2
; CHECK: st_l           %S0, [[RES]]
