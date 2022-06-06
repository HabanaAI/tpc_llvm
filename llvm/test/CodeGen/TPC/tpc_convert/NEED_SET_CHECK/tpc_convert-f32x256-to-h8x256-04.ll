; RUN: llc -mcpu gaudi2 %s -o - | FileCheck --check-prefixes=CHECK,GAUDI2 %s

target triple = "tpc"

define void @main(i32 %dest, i32 %src, i32 %in, i32 %pred) {
entry:
  %sptr = inttoptr i32 %src to <256 x float> addrspace(2)*
  %dptr = inttoptr i32 %dest to <256 x f8_152> addrspace(2)*
  %iptr = inttoptr i32 %in to <256 x f8_152> addrspace(2)*
  %spred = icmp ne i32 %pred, 0 
  %srcval = load <256 x float>, <256 x float> addrspace(2)* %sptr, align 256
  %inval = load <256 x f8_152>, <256 x f8_152> addrspace(2)* %iptr, align 256
  %dval = call <256 x f8_152> @llvm.tpc.convert.v256f8_152.v256f32.i1(<256 x float> %srcval, i8 0, i32 3072, <256 x f8_152> %inval, i1 %spred, i1 0)
  store <256 x f8_152> %dval, <256 x f8_152> addrspace(2)* %dptr, align 256
  ret void
}

; CHECK:     ld_l_v     [[VRF:%V[0-9]+]], %S1
; GAUDI2:   convert.f32 all_lanes target_type=f8_152 rhne
; CHECK-DAG: st_l_v     %S0, %V{{[0-9]+}}



declare <256 x f8_152> @llvm.tpc.convert.v256f8_152.v256f32.i1(<256 x float>, i8, i32, <256 x f8_152>, i1, i1)

;| FileCheck --check-prefixes=CHECK,GAUDI2 %s 
