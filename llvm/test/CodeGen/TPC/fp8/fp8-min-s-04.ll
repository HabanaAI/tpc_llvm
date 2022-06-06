; RUN: llc -mcpu gaudi2 %s -o - | FileCheck %s

target triple = "tpc"

define void @main(f8_152 %x0, f8_152 %res0, i32 %dest, i1 zeroext %pred) {
entry:
  %0 = inttoptr i32 %dest to f8_152 addrspace(1)*
  %1 = call f8_152 @llvm.tpc.min.f8_152.f8_152.f8_152.i1(f8_152 %x0, f8_152 2.0, i8 12, i32 0, f8_152 %res0, i1 %pred, i1 false)
  store f8_152 %1, f8_152 addrspace(1)* %0, align 4
  ret void
}

; Function Attrs: nounwind readnone
declare f8_152 @llvm.tpc.min.f8_152.f8_152.f8_152.i1(f8_152, f8_152, i8, i32, f8_152, i1, i1)

; CHECK: mov         [[PRED:%SP[0-9]+]], %S3
; CHECK: min.f8_152  %S1, %S0, 0x40, [[PRED]]
; CHECK: st_l        %S2, %S1
