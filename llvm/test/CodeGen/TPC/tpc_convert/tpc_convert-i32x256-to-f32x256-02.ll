; RUN: llc -mcpu greco %s -o - | FileCheck %s
; XFAIL:*
; GAUDI-2280

target triple = "tpc"

define void @main(i32 %dest, i32 %src, i32 %pred) {
entry:
  %sptr = inttoptr i32 %src to <256 x i32> addrspace(2)*
  %dptr = inttoptr i32 %dest to <256 x float> addrspace(2)*
  %pptr = inttoptr i32 %pred to <256 x i1> addrspace(2)*
  %srcval = load <256 x i32>, <256 x i32> addrspace(2)* %sptr, align 256
  %vpred = load <256 x i1>, <256 x i1> addrspace(2)* %pptr, align 256
  %dval = call <256 x float> @llvm.tpc.convert.v256f32.v256i32.v256i1(<256 x i32> %srcval, i8 2, i32 0, <256 x float> undef, <256 x i1> %vpred, i1 1)
  store <256 x float> %dval, <256 x float> addrspace(2)* %dptr, align 256
  ret void
}

declare <256 x float> @llvm.tpc.convert.v256f32.v256i32.v256i1(<256 x i32>, i8, i32, <256 x float>, <256 x i1>, i1)

; CHECK-DAG: ld_l_v     [[VRF:%V[0-9]+]], %S1
; CHECK-DAG: ld_l_v     [[VPRF:%VP[0-9]+]], %S2
; CHECK:     convert.i32 all_lanes target_type=fp32 rhne %A[[ARF:[0-9]+]],  %A[[ARF]], ![[VPRF]]
; CHECK-DAG: st_l_v     %S0, %V[[ARF]]
; CHECK-DAG: st_l_v
; CHECK-DAG: st_l_v
; CHECK-DAG: st_l_v
