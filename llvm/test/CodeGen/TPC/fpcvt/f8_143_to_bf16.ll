; RUN: llc -mcpu gaudi2 %s -o - | FileCheck %s

target triple = "tpc"

define void @main(i32 %dest, bfloat16 %x, f8_143 %y) {
entry:
  %0 = inttoptr i32 %dest to bfloat16 addrspace(1)*
  %1 = fpext f8_143 %y to bfloat16
  %2 = fadd bfloat16 %x, %1
  store bfloat16 %2, bfloat16 addrspace(1)* %0, align 4
  ret void
}

; CHECK: convert.f8_143 target_type=bf16 [[VAL:%S[0-9]+]], %S2
; CHECK: add.bf16       [[RES:%S[0-9]+]], %S1, [[VAL]]
; CHECK: st_l           %S0, [[RES]]
