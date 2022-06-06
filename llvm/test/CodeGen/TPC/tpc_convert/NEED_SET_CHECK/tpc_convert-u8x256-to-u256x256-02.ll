; RUN: llc -mcpu greco %s -o -  | FileCheck --check-prefixes=CHECK,GRECO %s
; RUN: llc -mcpu gaudi2 %s -o - | FileCheck --check-prefixes=CHECK,GRECO %s
; RUN: llc -mcpu gaudib %s -o - 
; RUN: llc -mcpu gaudi %s -o -  
; RUN: llc -mcpu goya %s -o -   

target triple = "tpc"

define void @main(i32 %dest, i32 %src, i32 %pred) {
entry:
  %sptr = inttoptr i32 %src to <256 x i8> addrspace(2)*
  %dptr = inttoptr i32 %dest to <256 x i32> addrspace(2)*
  %pptr = inttoptr i32 %pred to <256 x i1> addrspace(2)*
  %srcval = load <256 x i8>, <256 x i8> addrspace(2)* %sptr, align 256
  %vpred = load <256 x i1>, <256 x i1> addrspace(2)* %pptr, align 256
  %dval = call <256 x i32> @llvm.tpc.convert.v256i16.v256i8.v256i1(<256 x i8> %srcval, i8 5, i32 768, <256 x i32> undef, <256 x i1> %vpred, i1 1)
  store <256 x i32> %dval, <256 x i32> addrspace(2)* %dptr, align 256
  ret void
}

declare <256 x i32> @llvm.tpc.convert.v256i16.v256i8.v256i1(<256 x i8>, i8, i32, <256 x i32>, <256 x i1>, i1)

; CHECK-DAG: ld_l_v     [[VRF:%V[0-9]+]], %S1
; CHECK-DAG: ld_l_v     [[VPRF:%VP[0-9]+]], %S2
; GRECO:     convert.u8 all_lanes target_type=uint32 rhne %A[[ARF:[0-9]+]], [[VRF]], ![[VPRF]]
; CHECK-DAG: st_l_v     %S0, %V[[ARF]]
; CHECK-DAG: st_l_v
