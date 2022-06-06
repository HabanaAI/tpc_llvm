; RUN: llc -mcpu gaudi2 %s -o - | FileCheck --check-prefixes=CHECK,GAUDI2 %s
; RUN: llc -mcpu gaudib %s -o - 
; RUN: llc -mcpu gaudi %s -o -  
; RUN: llc -mcpu goya %s -o -   

target triple = "tpc"

define void @main(i32 %dest, i32 %src, i32 %pred, i32 %in) {
entry:
  %sptr = inttoptr i32 %src to <128 x i16> addrspace(2)*
  %dptr = inttoptr i32 %dest to <128 x i32> addrspace(2)*
  %pptr = inttoptr i32 %pred to <256 x i1> addrspace(2)*
  %iptr = inttoptr i32 %in to <128 x i32> addrspace(2)*
  %srcval = load <128 x i16>, <128 x i16> addrspace(2)* %sptr, align 256
  %vpred = load <256 x i1>, <256 x i1> addrspace(2)* %pptr, align 256
  %inval = load <128 x i32>, <128 x i32> addrspace(2)* %iptr, align 256
  %dval = call <128 x i32> @llvm.tpc.convert.v128i32.v128i16.v256i1(<128 x i16> %srcval, i8 8, i32 512, <128 x i32> %inval, <256 x i1> %vpred, i1 0)
  store <128 x i32> %dval, <128 x i32> addrspace(2)* %dptr, align 256
  ret void
}

declare <128 x i32> @llvm.tpc.convert.v128i32.v128i16.v256i1(<128 x i16>, i8, i32, <128 x i32>, <256 x i1>, i1)

; CHECK:     ld_l_v     [[VRF:%V[0-9]+]], %S1
; GAUDI2:    convert.u16 all_lanes target_type=int32 rhne
; CHECK-DAG: st_l_v     %S0, %V{{[0-9]+}}


; move up when it will be possible
; | FileCheck --check-prefixes=CHECK,GRECO %s
; | FileCheck --check-prefixes=CHECK,GAUDI2 %s 
; | FileCheck --check-prefixes=CHECK,GAUDIB %s 
; | FileCheck --check-prefixes=CHECK,GAUDI %s  
; | FileCheck --check-prefixes=CHECK,GOYA  %s  

