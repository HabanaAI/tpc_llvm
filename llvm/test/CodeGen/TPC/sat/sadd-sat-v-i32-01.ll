; RUN: llc -march=tpc -mcpu=gaudi %s -o - | FileCheck %s

define void @main(i32 %dest, i32 %src1, i32 %src2) {
entry:
  %dptr = inttoptr i32 %dest to <64 x i32> addrspace(2)*
  %s1ptr = inttoptr i32 %src1 to <64 x i32> addrspace(2)*
  %s2ptr = inttoptr i32 %src2 to <64 x i32> addrspace(2)*
  %x1 = load <64 x i32>, <64 x i32> addrspace(2)* %s1ptr, align 256
  %x2 = load <64 x i32>, <64 x i32> addrspace(2)* %s2ptr, align 256
  %add = call <64 x i32> @llvm.sadd.sat.v64i32(<64 x i32> %x1, <64 x i32> %x2)
  store <64 x i32> %add, <64 x i32> addrspace(2)* %dptr
  ret void
}

declare <64 x i32> @llvm.sadd.sat.v64i32(<64 x i32>, <64 x i32>)

; CHECK: add.i32 st [[DEST:%V[0-9]+]], {{%V[0-9]+}}, {{%V[0-9]+}}
; CHECK: st_l_v  %S0, [[DEST]]
