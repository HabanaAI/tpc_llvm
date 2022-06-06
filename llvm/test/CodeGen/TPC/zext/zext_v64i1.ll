; RUN: llc %s -o - | FileCheck %s

target triple = "tpc"

define void @main(i32 %dest, i32 %val) {
entry:
  %0 = inttoptr i32 %dest to <64 x i32> addrspace(2)*
  %1 = inttoptr i32 %val to <64 x i32> addrspace(2)*
  %v = load <64 x i32>, <64 x i32> addrspace(2)* %1, align 256
  %mask = icmp sgt <64 x i32> %v, zeroinitializer
  %maskw = zext <64 x i1> %mask to <64 x i32>
  store <64 x i32> %maskw, <64 x i32> addrspace(2)* %0, align 256
  ret void
}

; CHECK:     ld_l_v      [[VAL:%V[0-9]+]], %S1
; CHECK:     cmp_grt.i32 [[MASK:%VP[0-9]+]], [[VAL]], 0x0
; CHECK-DAG: mov.i32     [[RES:%V[0-9]+]], 0x1, [[MASK]]
; CHECK-DAG: mov.i32     [[RES:%V[0-9]+]], 0x0, ![[MASK]]
; CHECK:     st_l_v      %S0{{.*}}, [[RES]]
