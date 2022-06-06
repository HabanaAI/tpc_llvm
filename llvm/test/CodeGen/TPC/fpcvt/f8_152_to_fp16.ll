; RUN: llc -mcpu gaudi2 %s -o - | FileCheck %s

target triple = "tpc"

define void @main(i32 %dest, half %x, f8_152 %y) {
entry:
  %0 = inttoptr i32 %dest to half addrspace(1)*
  %1 = fpext f8_152 %y to half
  %2 = fadd half %x, %1
  store half %2, half addrspace(1)* %0, align 4
  ret void
}

; CHECK: convert.f8_152 target_type=f16 [[VAL:%S[0-9]+]], %S2
; CHECK: add.f16        [[RES:%S[0-9]+]], %S1, [[VAL]]
; CHECK: st_l           %S0, [[RES]]
