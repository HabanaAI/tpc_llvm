; RUN: llc -march=tpc -mcpu=gaudi %s -o - | FileCheck %s

define void @main(i32 %dest, i32 %src1) {
entry:
  %dptr = inttoptr i32 %dest to <256 x i8> addrspace(2)*
  %s1ptr = inttoptr i32 %src1 to <256 x i8> addrspace(2)*
  %x1 = load <256 x i8>, <256 x i8> addrspace(2)* %s1ptr, align 256
  %splat.0 = insertelement <256 x i8> undef, i8 77, i32 0
  %x2 = shufflevector <256 x i8> %splat.0, <256 x i8> undef, <256 x i32> zeroinitializer
  %add = call <256 x i8> @llvm.sadd.sat.v256i8(<256 x i8> %x1, <256 x i8> %x2)
  store <256 x i8> %add, <256 x i8> addrspace(2)* %dptr
  ret void
}

declare <256 x i8> @llvm.sadd.sat.v256i8(<256 x i8>, <256 x i8>)

; CHECK: add.i8  st [[DEST:%V[0-9]+]], {{%V[0-9]+}}, 0x4d
; CHECK: st_l_v  %S0, [[DEST]]
