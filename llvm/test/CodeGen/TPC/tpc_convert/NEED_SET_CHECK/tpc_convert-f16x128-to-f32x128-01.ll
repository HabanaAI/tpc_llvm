; RUN: llc -mcpu greco %s -o -  | FileCheck --check-prefixes=CHECK,GRECO %s
; RUN: llc -mcpu gaudi2 %s -o - | FileCheck --check-prefixes=CHECK,GAUDI2 %s
; RUN: llc -mcpu gaudib %s -o - 


target triple = "tpc"

define void @main(i32 %dest, i32 %src) {
entry:
  %0 = inttoptr i32 %src to <128 x half> addrspace(2)*
  %1 = inttoptr i32 %dest to <128 x float> addrspace(2)*
  %2 = load <128 x half>, <128 x half> addrspace(2)* %0, align 256
  %3 = call <128 x float> @llvm.tpc.convert.v128f32.v128f16.i1(<128 x half> %2, i8 11, i32 0, <128 x float> undef, i1 1, i1 0)
  store <128 x float> %3, <128 x float> addrspace(2)* %1, align 256
  ret void
}

declare <128 x float> @llvm.tpc.convert.v128f32.v128f16.i1(<128 x half>, i8, i32, <128 x float>, i1, i1)

; CHECK:     ld_l_v     [[VRF:%V[0-9]+]], %S1
; GRECO:     convert.f16 all_lanes target_type=fp32 rhne
; GAUDI2:    convert.f16 all_lanes target_type=fp32 rhne
; CHECK-DAG: st_l_v     %S0, %V{{[0-9]+}}

;| FileCheck --check-prefixes=CHECK,GAUDIB %s
;| FileCheck --check-prefixes=CHECK,GAUDI %s 
