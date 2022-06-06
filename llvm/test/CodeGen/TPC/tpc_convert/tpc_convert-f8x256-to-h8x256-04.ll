; RUN: llc -mcpu gaudi2 %s -o - | FileCheck %s

target triple = "tpc"

define void @main(i32 %dest, i32 %src, i32 %in, i32 %pred) {
entry:
  %sptr = inttoptr i32 %src to <256 x f8_143> addrspace(2)*
  %dptr = inttoptr i32 %dest to <256 x f8_152> addrspace(2)*
  %iptr = inttoptr i32 %in to <256 x f8_152> addrspace(2)*
  %spred = icmp ne i32 %pred, 0 
  %srcval = load <256 x f8_143>, <256 x f8_143> addrspace(2)* %sptr, align 256
  %inval = load <256 x f8_152>, <256 x f8_152> addrspace(2)* %iptr, align 256
  %dval = call <256 x f8_152> @llvm.tpc.convert.v256f8_152.v256f8_143.i1(<256 x f8_143> %srcval, i8 13, i32 3072, <256 x f8_152> %inval, i1 %spred, i1 0)
  store <256 x f8_152> %dval, <256 x f8_152> addrspace(2)* %dptr, align 256
  ret void
}

declare <256 x f8_152> @llvm.tpc.convert.v256f8_152.v256f8_143.i1(<256 x f8_143>, i8, i32, <256 x f8_152>, i1, i1)

; CHECK-DAG: ld_l_v      [[VRF:%V[0-9]+]], %S{{[0-9]+}}
; CHECK-DAG: cmp_neq.i32 [[SPRF:%SP[0-9]+]], %S{{[0-9]+}}, 0x0
; CHECK-DAG: ld_l_v      %V{{[0-9]+}}, %S{{[0-9]+}}
; CHECK:     convert.f8_143 all_lanes target_type=f8_152 rhne %V{{[0-9]+}}, [[VRF]], [[SPRF]]
; CHECK-DAG: st_l_v      %S{{[0-9]+}}, %V{{[0-9]+}}
