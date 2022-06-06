; RUN: llc %s -o - | FileCheck %s

target triple = "tpc"

define void @main(i32 %dest, i32 %src1, i32 %src2, i32 %val1, i32 %val2) {
entry:
  %0 = inttoptr i32 %dest to <64 x float> addrspace(2)*
  %1 = inttoptr i32 %src1 to <64 x float> addrspace(2)*
  %2 = inttoptr i32 %src2 to <64 x float> addrspace(2)*
  %3 = inttoptr i32 %val1 to <64 x float> addrspace(2)*
  %4 = inttoptr i32 %val2 to <64 x float> addrspace(2)*

  %s1 = load <64 x float>, <64 x float> addrspace(2)* %1, align 256
  %s2 = load <64 x float>, <64 x float> addrspace(2)* %2, align 256
  %v1 = load <64 x float>, <64 x float> addrspace(2)* %3, align 256
  %v2 = load <64 x float>, <64 x float> addrspace(2)* %4, align 256

  %mask = fcmp ogt <64 x float> %v1, %v2

  %vr = select <64 x i1> %mask, <64 x float> %s1, <64 x float> %s2 

  store <64 x float> %vr, <64 x float> addrspace(2)* %0, align 256
  ret void
}

; CHECK-DAG: ld_l_v      [[SRC1:%V[0-9]+]], %S1
; CHECK-DAG: ld_l_v      [[SRC2:%V[0-9]+]], %S2
; CHECK-DAG: ld_l_v      [[VAL1:%V[0-9]+]], %S3
; CHECK-DAG: ld_l_v      [[VAL2:%V[0-9]+]], %S4
; CHECK:     sel_grt.f32 [[VAL:%V[0-9]+]], [[VAL1]], [[VAL2]], [[SRC1]], [[SRC2]]
; CHECK:     st_l_v      %S0{{.*}}, [[VAL]]
