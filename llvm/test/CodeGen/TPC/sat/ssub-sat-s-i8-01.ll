; RUN: llc -march=tpc -mcpu=gaudi %s -o - | FileCheck %s

define void @main(i32 %dest, i8 %x1, i8 %x2) {
entry:
  %dptr = inttoptr i32 %dest to i8 addrspace(1)*
  %sub = call i8 @llvm.ssub.sat.i8(i8 %x1, i8 %x2)
  store i8 %sub, i8 addrspace(1)* %dptr
  ret void
}

declare i8 @llvm.ssub.sat.i8(i8, i8)

; CHECK: sub.i8  st [[DEST:%S[0-9]+]], %S1, %S2
; CHECK: st_l    %S0, [[DEST]]
