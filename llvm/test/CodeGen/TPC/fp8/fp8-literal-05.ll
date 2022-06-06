; RUN: llc -mcpu gaudi2 %s -o - | FileCheck %s

target triple = "tpc"

define void @main(i32 %dest) {
entry:

  %dptr0 = inttoptr i32 %dest to f8_143 addrspace(1)*
  store f8_143 1.0, f8_143 addrspace(1)* %dptr0
  ret void
}

; CHECK: mov.f8_143  [[VAL:%S[0-9]+]], 0x38
; CHECK: st_l        %S0, [[VAL]]
