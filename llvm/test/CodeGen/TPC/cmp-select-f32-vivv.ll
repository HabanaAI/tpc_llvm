; RUN: llc %s -o - | FileCheck %s

target triple = "tpc"

define void @main(i32 %dest, i32 %src1, i32 %src2, i32 %pred) {
entry:
  %0 = inttoptr i32 %dest to <64 x float> addrspace(2)*
  %1 = inttoptr i32 %src1 to <64 x float> addrspace(2)*
  %2 = inttoptr i32 %src2 to <64 x float> addrspace(2)*
  %3 = inttoptr i32 %pred to <64 x float> addrspace(2)*

  %v1 = load <64 x float>, <64 x float> addrspace(2)* %1, align 256
  %v2 = load <64 x float>, <64 x float> addrspace(2)* %2, align 256
  %vp = load <64 x float>, <64 x float> addrspace(2)* %3, align 256

  %mask = fcmp ogt <64 x float> %vp, zeroinitializer

  %vr = select <64 x i1> %mask, <64 x float> %v1, <64 x float> %v2 

  store <64 x float> %vr, <64 x float> addrspace(2)* %0, align 256
  ret void
}

; CHECK: ld_l_v      [[PRED:%V[0-9]+]], %S3
; CHECK: ld_l_v      [[VAL2:%V[0-9]+]], %S2
; CHECK: ld_l_v      [[VAL1:%V[0-9]+]], %S1
; CHECK: sel_grt.f32 [[VAL:%V[0-9]+]], [[PRED]], 0x0, [[VAL1]], [[VAL2]]
; CHECK: st_l_v      %S0{{.*}}, [[VAL]]

