; RUN: llc %s -o - | FileCheck %s

target triple = "tpc"

define void @main(i32 %dest, i32 %val) {
entry:
  %0 = inttoptr i32 %dest to <128 x i16> addrspace(2)*
  %1 = inttoptr i32 %val to <128 x i16> addrspace(2)*
  %v = load <128 x i16>, <128 x i16> addrspace(2)* %1, align 256
  %mask = icmp sgt <128 x i16> %v, zeroinitializer
  %maskw = zext <128 x i1> %mask to <128 x i16>
  store <128 x i16> %maskw, <128 x i16> addrspace(2)* %0, align 256
  ret void
}

; CHECK:     ld_l_v      [[VAL:%V[0-9]+]], %S1
; CHECK:     cmp_grt.i16 [[MASK:%VP[0-9]+]], [[VAL]], 0x0
; CHECK-DAG: mov.i16     [[RES:%V[0-9]+]], 0x1, [[MASK]]
; CHECK-DAG: mov.i16     [[RES:%V[0-9]+]], 0x0, ![[MASK]]
; CHECK:     st_l_v      %S0{{.*}}, [[RES]]
