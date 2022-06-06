; RUN: llc -mcpu gaudi2 %s -o - | FileCheck --check-prefixes=CHECK,GAUDI2 %s

target triple = "tpc"

define void @main(i32 %dest, i32 %src, i32 %pred, i32 %in) {
entry:
  %sptr = inttoptr i32 %src to <256 x float> addrspace(2)*
  %dptr = inttoptr i32 %dest to <256 x f8_143> addrspace(2)*
  %pptr = inttoptr i32 %pred to <256 x i1> addrspace(2)*
  %iptr = inttoptr i32 %in to <256 x f8_143> addrspace(2)*
  %srcval = load <256 x float>, <256 x float> addrspace(2)* %sptr, align 256
  %vpred = load <256 x i1>, <256 x i1> addrspace(2)* %pptr, align 256
  %inval = load <256 x f8_143>, <256 x f8_143> addrspace(2)* %iptr, align 256
  %dval = call <256 x f8_143> @llvm.tpc.convert.v256f8_143.v256f32.v256i1(<256 x float> %srcval, i8 0, i32 3328, <256 x f8_143> %inval, <256 x i1> %vpred, i1 0)
  store <256 x f8_143> %dval, <256 x f8_143> addrspace(2)* %dptr, align 256
  ret void
}

declare <256 x f8_143> @llvm.tpc.convert.v256f8_143.v256f32.v256i1(<256 x float>, i8, i32, <256 x f8_143>, <256 x i1>, i1)


; CHECK:     ld_l_v     [[VRF:%V[0-9]+]], %S1
; GAUDI2:   convert.f32 all_lanes target_type=f8_143 rhne
; CHECK-DAG: st_l_v     %S0, %V{{[0-9]+}}


; move up when it will be possible
; | FileCheck --check-prefixes=CHECK,GAUDI2 %s 

