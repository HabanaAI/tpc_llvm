; RUN: llc -march=tpc -mcpu=gaudi -O1 %s -o - | FileCheck %s

define void @main(i32 %dest, i32 %src1, i32 %src2) {
  %dptr = inttoptr i32 %dest to i32 addrspace(1)*
  %and = and i32 %src2, %src1
  %cmp = icmp eq i32 %and, 0
  %cmp2 = zext i1 %cmp to i32
  store i32 %cmp2, i32 addrspace(1)* %dptr, align 4
  ret void
}
; CHECK: cmp_eq.i32 mask_eq_zero %SP{{[0-9]+}}, %S2, %S1

