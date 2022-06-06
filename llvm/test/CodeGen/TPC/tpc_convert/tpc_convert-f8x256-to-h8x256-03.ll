; RUN: llc -mcpu gaudi2 %s -o - | FileCheck %s

target triple = "tpc"

define void @main(i32 %dest, i32 %src, i32 %pred, i32 %in) {
entry:
  %sptr = inttoptr i32 %src to <256 x f8_143> addrspace(2)*
  %dptr = inttoptr i32 %dest to <256 x f8_152> addrspace(2)*
  %pptr = inttoptr i32 %pred to <256 x i1> addrspace(2)*
  %iptr = inttoptr i32 %in to <256 x f8_152> addrspace(2)*
  %srcval = load <256 x f8_143>, <256 x f8_143> addrspace(2)* %sptr, align 256
  %vpred = load <256 x i1>, <256 x i1> addrspace(2)* %pptr, align 256
  %inval = load <256 x f8_152>, <256 x f8_152> addrspace(2)* %iptr, align 256
  %dval = call <256 x f8_152> @llvm.tpc.convert.v256f8_152.v256f8_143.v256i1(<256 x f8_143> %srcval, i8 13, i32 3072, <256 x f8_152> %inval, <256 x i1> %vpred, i1 0)
  store <256 x f8_152> %dval, <256 x f8_152> addrspace(2)* %dptr, align 256
  ret void
}

declare <256 x f8_152> @llvm.tpc.convert.v256f8_152.v256f8_143.v256i1(<256 x f8_143>, i8, i32, <256 x f8_152>, <256 x i1>, i1)

; CHECK-DAG: ld_l_v     [[VPRF:%VP[0-9]+]], %S{{[0-9]+}}
; CHECK-DAG: ld_l_v     [[VRF:%V[0-9]+]], %S{{[0-9]+}}

; CHECK-DAG: ld_l_v     %V{{[0-9]+}}, %S{{[0-9]+}}
; CHECK:     convert.f8_143 all_lanes target_type=f8_152 rhne %V{{[0-9]+}}, [[VRF]], [[VPRF]]
; CHECK-DAG: st_l_v     %S0, %V{{[0-9]+}}
