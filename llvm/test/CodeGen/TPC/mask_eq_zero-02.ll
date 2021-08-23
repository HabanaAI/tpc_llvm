; RUN: llc -march=tpc -mcpu=gaudi -O1 %s -o - | FileCheck %s

define void @main(i32 %dest, i16 signext %src1, i16 signext %src2) {
  %dptr = inttoptr i32 %dest to i32 addrspace(1)*
  %and = and i16 %src2, %src1
  %cmp = icmp eq i16 %and, 0
  %cmp2 = zext i1 %cmp to i32
  store i32 %cmp2, i32 addrspace(1)* %dptr, align 4
  ret void
}
; CHECK: cmp_eq.i16 mask_eq_zero %SP{{[0-9]+}}, %S2, %S1

