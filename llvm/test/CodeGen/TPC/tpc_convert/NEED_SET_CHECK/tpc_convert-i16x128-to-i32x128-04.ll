; RUN: llc -mcpu greco %s -o -  | FileCheck --check-prefixes=CHECK,GRECO %s
; RUN: llc -mcpu gaudi2 %s -o - | FileCheck --check-prefixes=CHECK,GAUDI2 %s
; RUN: llc -mcpu gaudib %s -o - 
; RUN: llc -mcpu gaudi %s -o -  
; RUN: llc -mcpu goya %s -o -   

target triple = "tpc"

define void @main(i32 %dest, i32 %src, i32 %in, i32 %pred) {
entry:
  %sptr = inttoptr i32 %src to <128 x i16> addrspace(2)*
  %dptr = inttoptr i32 %dest to <128 x i32> addrspace(2)*
  %iptr = inttoptr i32 %in to <128 x i32> addrspace(2)*
  %spred = icmp ne i32 %pred, 0 
  %srcval = load <128 x i16>, <128 x i16> addrspace(2)* %sptr, align 256
  %inval = load <128 x i32>, <128 x i32> addrspace(2)* %iptr, align 256
  %dval = call <128 x i32> @llvm.tpc.convert.v128i32.v128i16i1(<128 x i16> %srcval, i8 7, i32 512, <128 x i32> %inval, i1 %spred, i1 0)
  store <128 x i32> %dval, <128 x i32> addrspace(2)* %dptr, align 256
  ret void
}

declare <128 x i32> @llvm.tpc.convert.v128i32.v128i16i1(<128 x i16>, i8, i32, <128 x i32>, i1, i1)

; CHECK:     ld_l_v     [[VRF:%V[0-9]+]], %S1
; GRECO:     convert.i16 all_lanes target_type=int32 rhne
; GAUDI2:    convert.i16 all_lanes target_type=int32 rhne
; CHECK-DAG: st_l_v     %S0, %V{{[0-9]+}}


;| FileCheck --check-prefixes=CHECK,GRECO %s  
;| FileCheck --check-prefixes=CHECK,GAUDI2 %s 
;| FileCheck --check-prefixes=CHECK,GAUDIB %s 
;| FileCheck --check-prefixes=CHECK,GAUDI %s  
;| FileCheck --check-prefixes=CHECK,GOYA %s   