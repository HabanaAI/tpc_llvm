; RUN: llc -mcpu gaudi2 %s -o - | FileCheck %s

target triple = "tpc"

define void @main(i32 %dest) {
entry:
  %dptr = inttoptr i32 %dest to <256 x f8_152> addrspace(2)*
  store <256 x f8_152> zeroinitializer, <256 x f8_152> addrspace(2)* %dptr, align 512
  ret void
}

; CHECK: mov.f8_152 [[VAL:%V[0-9]+]], 0x0
; CHECK: st_l_v     %S0, [[VAL]]
