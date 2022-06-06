; RUN: llc -mcpu gaudi2 %s -o - | FileCheck %s

target triple = "tpc"

define void @main(i32 %dest) {
entry:

  %dptr = inttoptr i32 %dest to f8_143 addrspace(1)*
  store f8_143 0xG02, f8_143 addrspace(1)* %dptr

  ret void
}

; CHECK: mov.f8_143  %S1, 0x2
; CHECK: st_l        %S0, %S1
