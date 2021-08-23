; RUN: llc %s -o - | FileCheck %s

target triple = "tpc"

define void @main(i32 %dest, i32 %src1, i8 signext %src2) {
entry:
  %0 = inttoptr i32 %dest to <256 x i8> addrspace(2)*
  %1 = inttoptr i32 %src1 to <256 x i8> addrspace(2)*
  %2 = load <256 x i8>, <256 x i8> addrspace(2)* %1, align 256
  %splat.splatinsert = insertelement <256 x i8> undef, i8 %src2, i32 0
  %splat.splat = shufflevector <256 x i8> %splat.splatinsert, <256 x i8> undef, <256 x i32> zeroinitializer
  %mul = mul <256 x i8> %2, %splat.splat
  store <256 x i8> %mul, <256 x i8> addrspace(2)* %0, align 256
  ret void
}
; CHECK: ld_l_v  %V0, %S1, 0x0
; CHECK: mul.i8  %A0, %V0, %S2
; CHECK: st_l_v  %S0, 0x0, %V0
