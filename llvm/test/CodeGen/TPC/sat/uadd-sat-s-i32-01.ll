; RUN: llc -march=tpc -mcpu=gaudi %s -o - | FileCheck %s

define void @main(i32 %dest, i32 %x1, i32 %x2) {
entry:
  %dptr = inttoptr i32 %dest to i32 addrspace(1)*
  %add = call i32 @llvm.uadd.sat.i32(i32 %x1, i32 %x2)
  store i32 %add, i32 addrspace(1)* %dptr
  ret void
}

declare i32 @llvm.uadd.sat.i32(i32, i32)

; CHECK: add.u32 st [[DEST:%S[0-9]+]], %S1, %S2
; CHECK: st_l    %S0, [[DEST]]
