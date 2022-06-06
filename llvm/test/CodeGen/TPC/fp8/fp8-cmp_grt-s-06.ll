; RUN: llc -mcpu gaudi2 %s -o - | FileCheck %s

target triple = "tpc"

define void @main(f8_152 %x0, f8_152 %x1, i32 %dest) {
entry:
  %0 = inttoptr i32 %dest to i32 addrspace(1)*
  %1 = fcmp ogt f8_152 %x0, %x1
  %res = zext i1 %1 to i32
  store i32 %res, i32 addrspace(1)* %0, align 4
  ret void
}

; Function Attrs: nounwind readnone
declare i1 @llvm.tpc.cmp.grt.i1.f8_152.f8_152.i1(f8_152, f8_152, i8, i32, i1, i1, i1)

; CHECK: cmp_grt.f8_152  [[RES:%SP[0-9]+]], %S0, %S1
; CHECK: mov.i32        [[VAL:%S[0-9]+]], 0x1, [[RES]]
; CHECK: mov.i32        [[VAL]], 0x0, ![[RES]]
; CHECK: st_l           %S2, [[VAL]]
