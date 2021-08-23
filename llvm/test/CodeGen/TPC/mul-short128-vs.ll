; RUN: llc %s -o - | FileCheck %s

target triple = "tpc"

define void @main(i32 %dest, i32 %src1, i32 %src2, i16 signext %arg) {
entry:
  %0 = inttoptr i32 %dest to <128 x i16> addrspace(2)*
  %1 = inttoptr i32 %src1 to <128 x i16> addrspace(2)*
  %2 = load <128 x i16>, <128 x i16> addrspace(2)* %1, align 256
  %splat.splatinsert = insertelement <128 x i16> undef, i16 %arg, i32 0
  %splat.splat = shufflevector <128 x i16> %splat.splatinsert, <128 x i16> undef, <128 x i32> zeroinitializer
  %mul = mul <128 x i16> %2, %splat.splat
  store <128 x i16> %mul, <128 x i16> addrspace(2)* %0, align 256
  ret void
}
; CHECK: ld_l_v  %V0, %S1, 0x0
; CHECK: mul.i16 %D0, %V0, %S3
; CHECK: st_l_v  %S0, 0x0, %V0
