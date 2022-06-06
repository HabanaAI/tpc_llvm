; RUN: llc -mcpu gaudi2 %s -o - | FileCheck %s

target triple = "tpc"

define void @main(i32 %dest, f8_152 %fp) {
entry:
  %dptr = inttoptr i32 %dest to <256 x f8_152> addrspace(2)*
  %splat.0 = insertelement <256 x f8_152> undef, f8_152 %fp, i32 0
  %splat = shufflevector <256 x f8_152> %splat.0, <256 x f8_152> undef, <256 x i32> zeroinitializer
  store <256 x f8_152> %splat, <256 x f8_152> addrspace(2)* %dptr, align 256
  ret void
}

; CHECK: mov.f8_152 [[VAL:%V[0-9]+]], %S1
; CHECK: st_l_v     %S0, [[VAL]]
