; RUN: llc %s -o - | FileCheck %s

target triple = "tpc"

define void @main(i32 %dest, i32 %src1, i32 %src2) {
entry:
  %0 = inttoptr i32 %dest to <64 x i32> addrspace(2)*
  %1 = inttoptr i32 %src1 to <64 x i32> addrspace(2)*
  %2 = inttoptr i32 %src2 to <64 x i32> addrspace(2)*
  %3 = load <64 x i32>, <64 x i32> addrspace(2)* %1, align 256
  %4 = load <64 x i32>, <64 x i32> addrspace(2)* %2, align 256
  %mul = mul <64 x i32> %4, %3
  store <64 x i32> %mul, <64 x i32> addrspace(2)* %0, align 256
  ret void
}

; CHECK-DAG: ld_l_v [[V1:%V[0-9]+]], %S1, 0x0
; CHECK-DAG: ld_l_v [[V2:%V[0-9]+]], %S2, 0x0
; mul is commutative so dunno how to check for exact args:
; CHECK: mul.i32 %D[[#R:]], %V{{[0-9]+}}, %V{{[0-9]+}}
; CHECK: st_l_v %S0, 0x0, %V[[#R]]
