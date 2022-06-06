; RUN: llc -mcpu greco %s -o -  | FileCheck --check-prefixes=CHECK,GRECO %s
; RUN: llc -mcpu gaudi2 %s -o - | FileCheck --check-prefixes=CHECK,GAUDI2 %s
; RUN: llc -mcpu gaudib %s -o - 
; RUN: llc -mcpu gaudi %s -o -  
; RUN: llc -mcpu goya %s -o -   

target triple = "tpc"

define void @main(i32 %dest, i32 %src) {
entry:
  %0 = inttoptr i32 %src to <256 x i32> addrspace(2)*
  %1 = inttoptr i32 %dest to <256 x i8> addrspace(2)*
  %2 = load <256 x i32>, <256 x i32> addrspace(2)* %0, align 256
  %3 = call <256 x i8> @llvm.tpc.convert.v256i8.v256i32.i1(<256 x i32> %2, i8 2, i32 1024, <256 x i8> undef, i1 1, i1 0)
  store <256 x i8> %3, <256 x i8> addrspace(2)* %1, align 256
  ret void
}

declare <256 x i8> @llvm.tpc.convert.v256i8.v256i32.i1(<256 x i32>, i8, i32, <256 x i8>, i1, i1)

; CHECK:     ld_l_v     [[VRF:%V[0-9]+]], %S1
; GRECO:     convert_int32 all_lanes rhne to_8 %V{{[0-9]+}}, %A0
; GAUDI2:    convert.i32 all_lanes target_type=int8 rhne
; CHECK-DAG: st_l_v     %S0, %V{{[0-9]+}}



; move up when it will be possible
; | FileCheck --check-prefixes=CHECK,GAUDI2 %s 
; | FileCheck --check-prefixes=CHECK,GAUDIB %s 
; | FileCheck --check-prefixes=CHECK,GAUDI %s  
; | FileCheck --check-prefixes=CHECK,GOYA  %s  