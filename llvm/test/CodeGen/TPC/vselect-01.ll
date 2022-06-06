; RUN: llc %s -o - | FileCheck %s

target triple = "tpc"

define void @main(i32 %dest, i32 %src1, i32 %src2, i32 %val1) {
entry:
  %0 = inttoptr i32 %dest to <64 x i32> addrspace(2)*
  %1 = inttoptr i32 %src1 to <64 x i32> addrspace(2)*
  %2 = inttoptr i32 %src2 to <64 x i32> addrspace(2)*
  %3 = inttoptr i32 %val1 to <64 x i32> addrspace(2)*

  %v1 = load <64 x i32>, <64 x i32> addrspace(2)* %1, align 256
  %v2 = load <64 x i32>, <64 x i32> addrspace(2)* %2, align 256
  %vp = load <64 x i32>, <64 x i32> addrspace(2)* %3, align 256

  %mask = icmp sgt <64 x i32> %vp, zeroinitializer

  %vr = select <64 x i1> %mask, <64 x i32> %v1, <64 x i32> %v2 

  store <64 x i32> %vr, <64 x i32> addrspace(2)* %0, align 256
  ret void
}

; CHECK-DAG: ld_l_v      [[VAL1:%V[0-9]+]], %S3
; CHECK-DAG: ld_l_v      [[SRC2:%V[0-9]+]], %S2
; CHECK-DAG: ld_l_v      [[SRC1:%V[0-9]+]], %S1
; CHECK:     sel_grt.i32 [[VAL:%V[0-9]+]], [[VAL1]], 0x0, [[SRC1]], [[SRC2]]
; CHECK:     st_l_v      %S0{{.*}}, [[VAL]]

