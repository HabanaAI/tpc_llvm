; RUN: llc -mcpu gaudi2 %s -o - | FileCheck %s

target triple = "tpc"

define void @main(f8_152 %x0, f8_152 %x1, i32 %dest) {
entry:
  %0 = inttoptr i32 %dest to f8_152 addrspace(1)*
  %1 = fsub f8_152 %x0, %x1
  store f8_152 %1, f8_152 addrspace(1)* %0, align 4
  ret void
}

; CHECK: sub.f8_152  [[VAL:%S[0-9]+]], %S0, %S1
; CHECK: st_l        %S2, [[VAL]]
