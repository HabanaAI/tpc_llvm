; RUN: llc %s -o - | FileCheck %s

target triple = "tpc"

define void @main(i32 %dest, i32 %src1) {
entry:
  %0 = inttoptr i32 %dest to <64 x i1> addrspace(2)*
  %1 = inttoptr i32 %src1 to <64 x float> addrspace(2)*

  %v1 = load <64 x float>, <64 x float> addrspace(2)* %1, align 256

  %res = fcmp uno <64 x float> %v1, %v1

  store <64 x i1> %res, <64 x i1> addrspace(2)* %0, align 256
  ret void
}

; CHECK: ld_l_v      [[VAL1:%V[0-9]+]], %S1
; CHECK: cmp_eq.f32  [[VPRF1:%VP[0-9]+]], [[VAL1]], [[VAL1]]
; CHECK: not.b       [[RES:%VP[0-9]+]], [[VPRF1]]
; CHECK: st_l_v      %S0{{.*}}, [[RES]]
