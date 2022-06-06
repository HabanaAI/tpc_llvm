; RUN: llc -mcpu greco %s -o -  | FileCheck --check-prefixes=CHECK,GRECO %s
; RUN: llc -mcpu gaudi2 %s -o - | FileCheck --check-prefixes=CHECK,GRECO %s
; RUN: llc -mcpu gaudib %s -o - 
; RUN: llc -mcpu gaudi %s -o -  
; RUN: llc -mcpu goya %s -o -   

target triple = "tpc"

define void @main(i32 %dest, i32 %src) {
entry:
  %0 = inttoptr i32 %src to <256 x i8> addrspace(2)*
  %1 = inttoptr i32 %dest to <256 x i16> addrspace(2)*
  %2 = load <256 x i8>, <256 x i8> addrspace(2)* %0, align 256
  %3 = call <256 x i16> @llvm.tpc.convert.v256i16.v256i8.i1(<256 x i8> %2, i8 5, i32 2048, <256 x i16> undef, i1 1, i1 0)
  store <256 x i16> %3, <256 x i16> addrspace(2)* %1, align 256
  ret void
}

declare <256 x i16> @llvm.tpc.convert.v256i16.v256i8.i1(<256 x i8>, i8, i32, <256 x i16>, i1, i1)

; GRECO:     ld_l_v     [[VRF:%V[0-9]+]], %S1
; GRECO:     convert.u8 all_lanes target_type=uint16 rhne %D[[DRF:[0-9]+]], [[VRF]]
; CHECK-DAG: st_l_v     %S0, %V[[DRF]]
; CHECK-DAG: st_l_v
