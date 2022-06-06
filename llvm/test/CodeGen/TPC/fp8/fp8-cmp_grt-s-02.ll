; RUN: llc -mcpu gaudi2 %s -o - | FileCheck %s

target triple = "tpc"

define void @main(f8_143 %x0, i1 zeroext %res0, i32 %dest, i1 zeroext %pred) {
entry:
  %0 = inttoptr i32 %dest to i32 addrspace(1)*
  %1 = call i1 @llvm.tpc.cmp.grt.i1.f8_143.f8_143.i1(f8_143 %x0, f8_143 2.0, i8 13, i32 0, i1 %res0, i1 %pred, i1 false)
  %res = zext i1 %1 to i32
  store i32 %res, i32 addrspace(1)* %0, align 4
  ret void
}

; Function Attrs: nounwind readnone
declare i1 @llvm.tpc.cmp.grt.i1.f8_143.f8_143.i1(f8_143, f8_143, i8, i32, i1, i1, i1)

; CHECK: mov            [[RES:%SP[0-9]+]], %S1
; CHECK: cmp_grt.f8_143  [[RES]], %S0, 0x40
; CHECK: mov.i32        [[VAL:%S[0-9]+]], 0x1, [[RES]]
; CHECK: mov.i32        [[VAL]], 0x0, ![[RES]]
; CHECK: st_l           %S2, [[VAL]]
