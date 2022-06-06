; RUN: llc %s -o - | FileCheck %s

target triple = "tpc"

define void @main(i32 %dest, i32 %src1, float %src2, i32 %val1, i32 %val2) {
entry:
  %0 = inttoptr i32 %dest to <64 x float> addrspace(2)*
  %1 = inttoptr i32 %src1 to <64 x float> addrspace(2)*
  %2 = inttoptr i32 %val1 to <64 x float> addrspace(2)*
  %3 = inttoptr i32 %val2 to <64 x float> addrspace(2)*

  %s1 = load <64 x float>, <64 x float> addrspace(2)* %1, align 256
  %v1 = load <64 x float>, <64 x float> addrspace(2)* %2, align 256
  %v2 = load <64 x float>, <64 x float> addrspace(2)* %3, align 256

  %splat.splatinsert = insertelement <64 x float> undef, float %src2, i32 0
  %s2 = shufflevector <64 x float> %splat.splatinsert, <64 x float> undef, <64 x i32> zeroinitializer

  %mask = fcmp ogt <64 x float> %v1, %v2

  %vr = select <64 x i1> %mask, <64 x float> %s1, <64 x float> %s2

  store <64 x float> %vr, <64 x float> addrspace(2)* %0, align 256
  ret void
}

; CHECK-DAG: ld_l_v      [[SRC1:%V[0-9]+]], %S1
; CHECK-DAG: ld_l_v      [[VAL1:%V[0-9]+]], %S3
; CHECK-DAG: ld_l_v      [[VAL2:%V[0-9]+]], %S4
; CHECK:     sel_grt.f32 [[VAL:%V[0-9]+]], [[VAL1]], [[VAL2]], [[SRC1]], %S2
; CHECK:     st_l_v      %S0{{.*}}, [[VAL]]
