; RUN: llc -march=tpc -mcpu=gaudi %s -o - | FileCheck %s

define void @main(i32 %dest, i32 %x1) {
entry:
  %dptr = inttoptr i32 %dest to i32 addrspace(1)*
  %sub = call i32 @llvm.usub.sat.i32(i32 %x1, i32 777)
  store i32 %sub, i32 addrspace(1)* %dptr
  ret void
}

declare i32 @llvm.usub.sat.i32(i32, i32)

; CHECK: sub.u32 st [[DEST:%S[0-9]+]], %S1, 0x309
; CHECK: st_l    %S0, [[DEST]]
