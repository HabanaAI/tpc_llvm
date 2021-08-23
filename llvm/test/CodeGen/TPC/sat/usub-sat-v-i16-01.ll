; RUN: llc -march=tpc -mcpu=gaudi %s -o - | FileCheck %s

define void @main(i32 %dest, i32 %src1, i32 %src2) {
entry:
  %dptr = inttoptr i32 %dest to <128 x i16> addrspace(2)*
  %s1ptr = inttoptr i32 %src1 to <128 x i16> addrspace(2)*
  %s2ptr = inttoptr i32 %src2 to <128 x i16> addrspace(2)*
  %x1 = load <128 x i16>, <128 x i16> addrspace(2)* %s1ptr, align 256
  %x2 = load <128 x i16>, <128 x i16> addrspace(2)* %s2ptr, align 256
  %sub = call <128 x i16> @llvm.usub.sat.v64i32(<128 x i16> %x1, <128 x i16> %x2)
  store <128 x i16> %sub, <128 x i16> addrspace(2)* %dptr
  ret void
}

declare <128 x i16> @llvm.usub.sat.v64i32(<128 x i16>, <128 x i16>)

; CHECK: sub.u16 st [[DEST:%V[0-9]+]], {{%V[0-9]+}}, {{%V[0-9]+}}
; CHECK: st_l_v  %S0, [[DEST]]
