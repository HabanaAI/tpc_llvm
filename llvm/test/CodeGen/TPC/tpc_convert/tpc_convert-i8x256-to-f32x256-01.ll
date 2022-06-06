; RUN: llc -mcpu greco %s -o - | FileCheck %s

target triple = "tpc"

define void @main(i32 %dest, i32 %src) {
entry:
  %0 = inttoptr i32 %src to <256 x i8> addrspace(2)*
  %1 = inttoptr i32 %dest to <256 x float> addrspace(2)*
  %2 = load <256 x i8>, <256 x i8> addrspace(2)* %0, align 256
  %3 = call <256 x float> @llvm.tpc.convert.v256f32.v256i8.i1(<256 x i8> %2, i8 4, i32 0, <256 x float> undef, i1 1, i1 0)
  store <256 x float> %3, <256 x float> addrspace(2)* %1, align 256
  ret void
}

declare <256 x float> @llvm.tpc.convert.v256f32.v256i8.i1(<256 x i8>, i8, i32, <256 x float>, i1, i1)

; CHECK:     ld_l_v     [[VRF:%V[0-9]+]], %S1
; CHECK:     convert.i8 all_lanes target_type=fp32 rhne %A[[ARF:[0-9]+]], [[VRF]]
; CHECK-DAG: st_l_v     %S0, %V[[ARF]]
; CHECK-DAG: st_l_v
; CHECK-DAG: st_l_v
; CHECK-DAG: st_l_v
