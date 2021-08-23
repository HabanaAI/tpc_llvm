; RUN: llc -march=tpc -mcpu=gaudi %s -o - | FileCheck %s

define void @main(i32 %dest, i8 %x1) {
entry:
  %dptr = inttoptr i32 %dest to i8 addrspace(1)*
  %sub = call i8 @llvm.usub.sat.i8(i8 %x1, i8 77)
  store i8 %sub, i8 addrspace(1)* %dptr
  ret void
}

declare i8 @llvm.usub.sat.i8(i8, i8)

; CHECK: sub.u8  st [[DEST:%S[0-9]+]], %S1, 0x4d
; CHECK: st_l    %S0, [[DEST]]
