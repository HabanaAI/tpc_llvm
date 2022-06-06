; RUN: llc %s -o - | FileCheck %s

target triple = "tpc"

define void @main(i32 %dest, i32 %val) {
entry:
  %0 = inttoptr i32 %dest to <256 x i8> addrspace(2)*
  %1 = inttoptr i32 %val to <256 x i8> addrspace(2)*
  %v = load <256 x i8>, <256 x i8> addrspace(2)* %1, align 256
  %mask = icmp sgt <256 x i8> %v, zeroinitializer
  %maskw = zext <256 x i1> %mask to <256 x i8>
  store <256 x i8> %maskw, <256 x i8> addrspace(2)* %0, align 256
  ret void
}

; CHECK:     ld_l_v      [[VAL:%V[0-9]+]], %S1
; CHECK:     cmp_grt.i8  [[MASK:%VP[0-9]+]], [[VAL]], 0x0
; CHECK-DAG: mov.i8      [[RES:%V[0-9]+]], 0x1, [[MASK]]
; CHECK-DAG: mov.i8      [[RES:%V[0-9]+]], 0x0, ![[MASK]]
; CHECK:     st_l_v      %S0{{.*}}, [[RES]]
