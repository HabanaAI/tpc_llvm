; RUN: llc -march=tpc -mcpu=gaudi %s -o - | FileCheck %s

define void @main(i32 %dest, i16 %x1, i16 %x2) {
entry:
  %dptr = inttoptr i32 %dest to i16 addrspace(1)*
  %add = call i16 @llvm.sadd.sat.i16(i16 %x1, i16 %x2)
  store i16 %add, i16 addrspace(1)* %dptr
  ret void
}

declare i16 @llvm.sadd.sat.i16(i16, i16)

; CHECK: add.i16 st [[DEST:%S[0-9]+]], %S1, %S2
; CHECK: st_l    %S0, [[DEST]]
