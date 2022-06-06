; RUN: llc -mcpu gaudi2 %s -o - | FileCheck %s

target triple = "tpc"

define void @main(i32 %dest) {
entry:

  %dptr0 = inttoptr i32 %dest to f8_152 addrspace(1)*
  store f8_152 2.0, f8_152 addrspace(1)* %dptr0
  ret void
}

; CHECK: mov.f8_152  [[VAL:%S[0-9]+]], 0x40
; CHECK: st_l        %S0, [[VAL]]