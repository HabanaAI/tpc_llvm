; RUN: llc -mcpu gaudi2 %s -o - | FileCheck %s

target triple = "tpc"

define void @main(f8_152 %x0, f8_152 %x1, f8_152 %res0, i32 %dest, i1 zeroext %pred) {
entry:
  %0 = inttoptr i32 %dest to f8_152 addrspace(1)*
  %1 = call f8_152 @llvm.tpc.max.f8_152.f8_152.f8_152.i1(f8_152 %x0, f8_152 %x1, i8 12, i32 0, f8_152 %res0, i1 %pred, i1 false)
  store f8_152 %1, f8_152 addrspace(1)* %0, align 4
  ret void
}

; Function Attrs: nounwind readnone
declare f8_152 @llvm.tpc.max.f8_152.f8_152.f8_152.i1(f8_152, f8_152, i8, i32, f8_152, i1, i1)

; CHECK: mov         [[PRED:%SP[0-9]+]], %S4
; CHECK: max.f8_152  %S2, %S0, %S1, [[PRED]]
; CHECK: st_l        %S3, %S2
