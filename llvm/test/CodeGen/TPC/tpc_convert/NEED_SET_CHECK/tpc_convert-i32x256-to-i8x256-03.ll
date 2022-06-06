; RUN: llc -mcpu greco %s -o -  | FileCheck --check-prefixes=CHECK,GRECO %s
; RUN: llc -mcpu gaudi2 %s -o - | FileCheck --check-prefixes=CHECK,GAUDI2 %s
; RUN: llc -mcpu gaudib %s -o - 
; RUN: llc -mcpu gaudi %s -o -  
; RUN: llc -mcpu goya %s -o -   

target triple = "tpc"

define void @main(i32 %dest, i32 %src, i32 %pred, i32 %in) {
entry:
  %sptr = inttoptr i32 %src to <256 x i32> addrspace(2)*
  %dptr = inttoptr i32 %dest to <256 x i8> addrspace(2)*
  %pptr = inttoptr i32 %pred to <256 x i1> addrspace(2)*
  %iptr = inttoptr i32 %in to <256 x i8> addrspace(2)*
  %srcval = load <256 x i32>, <256 x i32> addrspace(2)* %sptr, align 256
  %vpred = load <256 x i1>, <256 x i1> addrspace(2)* %pptr, align 256
  %inval = load <256 x i8>, <256 x i8> addrspace(2)* %iptr, align 256
  %dval = call <256 x i8> @llvm.tpc.convert.v256i8.v256i32.v256i1(<256 x i32> %srcval, i8 2, i32 1024, <256 x i8> %inval, <256 x i1> %vpred, i1 0)
  store <256 x i8> %dval, <256 x i8> addrspace(2)* %dptr, align 256
  ret void
}

declare <256 x i8> @llvm.tpc.convert.v256i8.v256i32.v256i1(<256 x i32>, i8, i32, <256 x i8>, <256 x i1>, i1)


; CHECK:     ld_l_v     [[VRF:%V[0-9]+]], %S1
; GRECO:     convert_int32 all_lanes rhne to_8 %V{{[0-9]+}}, %A0
; GAUDI2:    convert.i32 all_lanes target_type=int8 rhne
; CHECK-DAG: st_l_v     %S0, %V{{[0-9]+}}


; move up when it will be possible
; | FileCheck --check-prefixes=CHECK,GRECO %s
; | FileCheck --check-prefixes=CHECK,GAUDI2 %s 
; | FileCheck --check-prefixes=CHECK,GAUDIB %s 
; | FileCheck --check-prefixes=CHECK,GAUDI %s  
; | FileCheck --check-prefixes=CHECK,GOYA  %s  

