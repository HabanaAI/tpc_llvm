; RUN: llc -march=tpc -mcpu=gaudi %s -o - | FileCheck %s

define void @main(i32 %dest, i32 %src1, i16 %x) {
entry:
  %dptr = inttoptr i32 %dest to <128 x i16> addrspace(2)*
  %s1ptr = inttoptr i32 %src1 to <128 x i16> addrspace(2)*
  %x1 = load <128 x i16>, <128 x i16> addrspace(2)* %s1ptr, align 256
  %splat.0 = insertelement <128 x i16> undef, i16 %x, i32 0
  %x2 = shufflevector <128 x i16> %splat.0, <128 x i16> undef, <128 x i32> zeroinitializer
  %add = call <128 x i16> @llvm.sadd.sat.v64i32(<128 x i16> %x1, <128 x i16> %x2)
  store <128 x i16> %add, <128 x i16> addrspace(2)* %dptr
  ret void
}

declare <128 x i16> @llvm.sadd.sat.v64i32(<128 x i16>, <128 x i16>)

; CHECK: add.i16 st [[DEST:%V[0-9]+]], {{%V[0-9]+}}, %S2
; CHECK: st_l_v  %S0, [[DEST]]
