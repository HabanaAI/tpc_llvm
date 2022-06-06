; RUN: llc -mcpu greco %s -o -  | FileCheck --check-prefixes=CHECK,GRECO %s
; RUN: llc -mcpu gaudi2 %s -o - | FileCheck --check-prefixes=CHECK,GAUDI2 %s
; RUN: llc -mcpu gaudib %s -o - 

target triple = "tpc"

define void @main(i32 %dest, i32 %src, i32 %pred, i32 %in) {
entry:
  %sptr = inttoptr i32 %src to <128 x half> addrspace(2)*
  %dptr = inttoptr i32 %dest to <128 x float> addrspace(2)*
  %pptr = inttoptr i32 %pred to <256 x i1> addrspace(2)*
  %iptr = inttoptr i32 %in to <128 x float> addrspace(2)*
  %srcval = load <128 x half>, <128 x half> addrspace(2)* %sptr, align 256
  %vpred = load <256 x i1>, <256 x i1> addrspace(2)* %pptr, align 256
  %inval = load <128 x float>, <128 x float> addrspace(2)* %iptr, align 256
  %dval = call <128 x float> @llvm.tpc.convert.v128f32.v128f16.v256i1(<128 x half> %srcval, i8 11, i32 0, <128 x float> %inval, <256 x i1> %vpred, i1 0)
  store <128 x float> %dval, <128 x float> addrspace(2)* %dptr, align 256
  ret void
}

declare <128 x float> @llvm.tpc.convert.v128f32.v128f16.v256i1(<128 x half>, i8, i32, <128 x float>, <256 x i1>, i1)

; CHECK:     ld_l_v     [[VRF:%V[0-9]+]], %S1
; GRECO:     convert.f16 all_lanes target_type=fp32 rhne
; GAUDI2:    convert.f16 all_lanes target_type=fp32 rhne
; CHECK-DAG: st_l_v     %S0, %V{{[0-9]+}}

; move up when it will be possible
; | FileCheck --check-prefixes=CHECK,GRECO %s
; | FileCheck --check-prefixes=CHECK,GAUDI2 %s 
; | FileCheck --check-prefixes=CHECK,GAUDIB %s 


