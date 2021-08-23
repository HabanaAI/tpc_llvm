; RUN: llc -march=tpc -mcpu=gaudi -O1 %s -o - | FileCheck %s

define void @main(i32 %dest, i32 %src1, i32 %src2) {
  %sptr1 = inttoptr i32 %src1 to <256 x i8> addrspace(2)*
  %sptr2 = inttoptr i32 %src2 to <256 x i8> addrspace(2)*
  %s1 = load <256 x i8>, <256 x i8> addrspace(2)* %sptr1, align 256
  %s2 = load <256 x i8>, <256 x i8> addrspace(2)* %sptr2, align 256
  %and = and <256 x i8> %s1, %s2
  %cmp = icmp eq <256 x i8> %and, zeroinitializer

  %dptr = inttoptr i32 %dest to <256 x i1> addrspace(2)*
  store <256 x i1> %cmp, <256 x i1> addrspace(2)* %dptr, align 256
  ret void
}
; CHECK-DAG: ld_l_v    [[VAL1:%V[0-9]+]], %S1
; CHECK-DAG: ld_l_v    [[VAL2:%V[0-9]+]], %S2
; CHECK:     cmp_eq.i8 mask_eq_zero %VP{{[0-9]+}}, [[VAL1]], [[VAL2]]

