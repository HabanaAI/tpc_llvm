; RUN: llc -mcpu gaudi2 %s -o - | FileCheck %s

target triple = "tpc"

define void @main(f8_152 %x0, i32 %dest, i1 zeroext %pred) {
entry:
  %0 = inttoptr i32 %dest to i8 addrspace(1)*
  %1 = call i8 @llvm.tpc.extract.exp.i8.f8_152.i1(f8_152 %x0, i8 12, i32 0, i8 undef, i1 %pred, i1 false)
  store i8 %1, i8 addrspace(1)* %0, align 4
  ret void
}

; CHECK: mov                 [[PRED:%SP[0-9]+]], %S2
; CHECK: extract_exp.f8_152  [[VAL:%S[0-9]+]], %S0, [[PRED]]
; CHECK: st_l                %S1, [[VAL]]

; Function Attrs: nounwind readnone
declare i8 @llvm.tpc.extract.exp.i8.f8_152.i1(f8_152, i8, i32, i8, i1, i1)
