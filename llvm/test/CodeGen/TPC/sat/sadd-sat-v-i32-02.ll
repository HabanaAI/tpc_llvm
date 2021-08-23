; RUN: llc -march=tpc -mcpu=gaudi %s -o - | FileCheck %s

define void @main(i32 %dest, i32 %src1, i32 %x) {
entry:
  %dptr = inttoptr i32 %dest to <64 x i32> addrspace(2)*
  %s1ptr = inttoptr i32 %src1 to <64 x i32> addrspace(2)*
  %x1 = load <64 x i32>, <64 x i32> addrspace(2)* %s1ptr, align 256
  %splat.0 = insertelement <64 x i32> undef, i32 %x, i32 0
  %x2 = shufflevector <64 x i32> %splat.0, <64 x i32> undef, <64 x i32> zeroinitializer
  %add = call <64 x i32> @llvm.sadd.sat.v64i32(<64 x i32> %x1, <64 x i32> %x2)
  store <64 x i32> %add, <64 x i32> addrspace(2)* %dptr
  ret void
}

declare <64 x i32> @llvm.sadd.sat.v64i32(<64 x i32>, <64 x i32>)

; CHECK: add.i32 st [[DEST:%V[0-9]+]], {{%V[0-9]+}}, %S2
; CHECK: st_l_v  %S0, [[DEST]]
