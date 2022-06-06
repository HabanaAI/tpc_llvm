; RUN: llc -mcpu gaudi2 %s -o - | FileCheck %s

target triple = "tpc"

define void @main(f8_143 %x0, f8_143 %x1, i32 %dest) {
entry:
  %0 = inttoptr i32 %dest to f8_143 addrspace(1)*
  %1 = fadd f8_143 %x0, %x1
  store f8_143 %1, f8_143 addrspace(1)* %0, align 4
  ret void
}

; CHECK: add.f8_143  [[VAL:%S[0-9]+]], %S0, %S1
; CHECK: st_l        %S2, [[VAL]]
