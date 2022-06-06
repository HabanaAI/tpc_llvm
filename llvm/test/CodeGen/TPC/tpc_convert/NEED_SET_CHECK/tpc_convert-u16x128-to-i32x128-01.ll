; RUN: llc -mcpu gaudi2 %s -o - | FileCheck --check-prefixes=CHECK,GAUDI2 %s

target triple = "tpc"

define void @main(i32 %dest, i32 %src) {
entry:
  %0 = inttoptr i32 %src to <128 x i16> addrspace(2)*
  %1 = inttoptr i32 %dest to <128 x i32> addrspace(2)*
  %2 = load <128 x i16>, <128 x i16> addrspace(2)* %0, align 256
  %3 = call <128 x i32> @llvm.tpc.convert.v128i32.v128i16i1(<128 x i16> %2, i8 8, i32 512, <128 x i32> undef, i1 1, i1 0)
  store <128 x i32> %3, <128 x i32> addrspace(2)* %1, align 256
  ret void
}

declare <128 x i32> @llvm.tpc.convert.v128i32.v128i16i1(<128 x i16>, i8, i32, <128 x i32>, i1, i1)

; GAUDI2:    convert.u16 all_lanes target_type=int32 rhne
; CHECK-DAG: st_l_v     %S0, %V{{[0-9]+}}



; move up when it will be possible
; | FileCheck --check-prefixes=CHECK,GAUDI2 %s 
; | FileCheck --check-prefixes=CHECK,GAUDIB %s 
; | FileCheck --check-prefixes=CHECK,GAUDI %s  
; | FileCheck --check-prefixes=CHECK,GOYA  %s  