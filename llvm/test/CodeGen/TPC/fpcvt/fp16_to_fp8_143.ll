; RUN: llc -mcpu gaudi2 %s -o - | FileCheck %s

target triple = "tpc"

define void @main(i32 %dest, half %x, f8_143 %y) {
entry:
  %0 = inttoptr i32 %dest to f8_143 addrspace(1)*
  %1 = fptrunc half %x to f8_143
  %2 = fadd f8_143 %1, %y
  store f8_143 %2, f8_143 addrspace(1)* %0, align 4
  ret void
}

; CHECK: convert.f16 target_type=f8_143  [[VAL:%S[0-9]+]], %S1
; CHECK: add.f8_143  [[RES:%S[0-9]+]], [[VAL]], %S2
; CHECK: st_l        %S0, [[RES]]
