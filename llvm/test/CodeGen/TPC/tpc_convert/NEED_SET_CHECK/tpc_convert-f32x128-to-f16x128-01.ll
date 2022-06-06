; RUN: llc -mcpu greco %s -o -  | FileCheck --check-prefixes=CHECK,GRECO %s
; RUN: llc -mcpu gaudi2 %s -o - | FileCheck --check-prefixes=CHECK,GAUDI2 %s
; RUN: llc -mcpu gaudib %s -o - 


target triple = "tpc"

define void @main(i32 %dest, i32 %src) {
entry:
  %0 = inttoptr i32 %src to <128 x float> addrspace(2)*
  %1 = inttoptr i32 %dest to <128 x half> addrspace(2)*
  %2 = load <128 x float>, <128 x float> addrspace(2)* %0, align 256
  %3 = call <128 x half> @llvm.tpc.convert.v128f16.v128f32.i1(<128 x float> %2, i8 0, i32 2816, <128 x half> undef, i1 1, i1 0)
  store <128 x half> %3, <128 x half> addrspace(2)* %1, align 256
  ret void
}

declare <128 x half> @llvm.tpc.convert.v128f16.v128f32.i1(<128 x float>, i8, i32, <128 x half>, i1, i1)

; CHECK:     ld_l_v     [[VRF:%V[0-9]+]], %S1
; GRECO:     convert.f32 all_lanes target_type=f16 rhne
; GAUDI2:    convert.f32 all_lanes target_type=f16 rhne
; CHECK-DAG: st_l_v     %S0, %V{{[0-9]+}}

;| FileCheck --check-prefixes=CHECK,GAUDIB %s
