; RUN: llc -mcpu greco %s -o - | FileCheck %s

target triple = "tpc"

define void @main(i32 %dest, i32 %src, i32 %in, i32 %pred) {
entry:
  %sptr = inttoptr i32 %src to <256 x i8> addrspace(2)*
  %dptr = inttoptr i32 %dest to <256 x float> addrspace(2)*
  %iptr = inttoptr i32 %in to <256 x float> addrspace(2)*
  %spred = icmp ne i32 %pred, 0 
  %srcval = load <256 x i8>, <256 x i8> addrspace(2)* %sptr, align 256
  %inval = load <256 x float>, <256 x float> addrspace(2)* %iptr, align 256
  %dval = call <256 x float> @llvm.tpc.convert.v256f32.v256i8.i1(<256 x i8> %srcval, i8 5, i32 0, <256 x float> %inval, i1 %spred, i1 0)
  store <256 x float> %dval, <256 x float> addrspace(2)* %dptr, align 256
  ret void
}

declare <256 x float> @llvm.tpc.convert.v256f32.v256i8.i1(<256 x i8>, i8, i32, <256 x float>, i1, i1)

; CHECK-DAG: ld_l_v      [[VRF:%V[0-9]+]], %S1
; CHECK-DAG: cmp_neq.i32 [[SPRF:%SP[0-9]+]], %S3, 0x0
; CHECK-DAG: ld_l_v      %V[[ARF:[0-9]+]], %S2
; CHECK-DAG: ld_l_v      %V{{[0-9]+}}, %S{{[0-9]+}}
; CHECK-DAG: ld_l_v      %V{{[0-9]+}}, %S{{[0-9]+}}
; CHECK-DAG: ld_l_v      %V{{[0-9]+}}, %S{{[0-9]+}}
; CHECK:     convert.u8  all_lanes target_type=fp32 rhne %A[[ARF]], [[VRF]], [[SPRF]]
; CHECK-DAG: st_l_v      %S0, %V[[ARF]]
; CHECK-DAG: st_l_v
; CHECK-DAG: st_l_v
; CHECK-DAG: st_l_v
