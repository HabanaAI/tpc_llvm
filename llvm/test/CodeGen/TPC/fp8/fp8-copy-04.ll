; RUN: llc -mcpu gaudi2 %s -o - | FileCheck %s

target triple = "tpc"

define void @main(i32 %dest) {
entry:

  %dptr = inttoptr i32 %dest to f8_152 addrspace(1)*
  store f8_152 0xI1e, f8_152 addrspace(1)* %dptr

  ret void
}

; CHECK: mov.f8_152  %S1, 0x1e
; CHECK: st_l        %S0, %S1
