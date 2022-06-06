; RUN: llc %s -o - | FileCheck %s

target triple = "tpc"

define void @main(i32 %dest, i32 %src1, i32 %src2, i32 %pred) {
entry:
  %0 = inttoptr i32 %dest to <64 x i32> addrspace(2)*
  %1 = inttoptr i32 %src1 to <64 x i32> addrspace(2)*
  %2 = inttoptr i32 %pred to <64 x i1> addrspace(2)*

  %v1 = load <64 x i32>, <64 x i32> addrspace(2)* %1, align 256
  %vp = load <64 x i1>, <64 x i1> addrspace(2)* %2, align 256

  %splat.splatinsert = insertelement <64 x i32> undef, i32 %src2, i32 0
  %v2 = shufflevector <64 x i32> %splat.splatinsert, <64 x i32> undef, <64 x i32> zeroinitializer

  %vr = select <64 x i1> %vp, <64 x i32> %v2, <64 x i32> %v1 

  store <64 x i32> %vr, <64 x i32> addrspace(2)* %0, align 256
  ret void
}

; CHECK-DAG: ld_l_v  [[PRED:%VP[0-9]+]], %S3
; CHECK-DAG: ld_l_v  [[VAL1:%V[0-9]+]], %S1
; CHECK:     mov.i32 [[VAL1]], %S2, [[PRED]]
; CHECK:     st_l_v  %S0,{{.*}} [[VAL1]]
