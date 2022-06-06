; RUN: llc %s -o - | FileCheck %s

target triple = "tpc"

define void @main(i32 %dest, i32 %src1, i32 %src2) {
entry:
  %0 = inttoptr i32 %dest to <64 x i1> addrspace(2)*
  %1 = inttoptr i32 %src1 to <64 x float> addrspace(2)*
  %2 = inttoptr i32 %src2 to <64 x float> addrspace(2)*

  %v1 = load <64 x float>, <64 x float> addrspace(2)* %1, align 256
  %v2 = load <64 x float>, <64 x float> addrspace(2)* %2, align 256

  %res = fcmp uno <64 x float> %v1, %v2

  store <64 x i1> %res, <64 x i1> addrspace(2)* %0, align 256
  ret void
}

; CHECK-DAG: ld_l_v      [[VAL1:%V[0-9]+]], %S1
; CHECK-DAG: ld_l_v      [[VAL2:%V[0-9]+]], %S2
; CHECK-DAG: cmp_eq.f32  [[VPRF1:%VP[0-9]+]], [[VAL1]], [[VAL1]]
; CHECK-DAG: cmp_eq.f32  [[VPRF2:%VP[0-9]+]], [[VAL2]], [[VAL2]]
; CHECK-DAG: not.b       [[VPRF3:%VP[0-9]+]], [[VPRF1]]
; CHECK-DAG: not.b       [[VPRF4:%VP[0-9]+]], [[VPRF2]]
; CHECK:     or.b        [[RES:%VP[0-9]+]], [[VPRF3]], [[VPRF4]]
; CHECK:     st_l_v      %S0{{.*}}, [[RES]]
