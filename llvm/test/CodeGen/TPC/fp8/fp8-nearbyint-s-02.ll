; RUN: llc -mcpu gaudi2 %s -o - | FileCheck %s

target triple = "tpc"

define void @main(i32 %dest, f8_152 %x, f8_152 %res, i1 zeroext %pred) {
entry:
  %0 = inttoptr i32 %dest to f8_152 addrspace(1)*
  %1 = call f8_152 @llvm.tpc.nearbyint.f8_152.f8_152.i1(f8_152 %x, i8 12, i32 512, f8_152 %res, i1 %pred, i1 false)
  store f8_152 %1, f8_152 addrspace(1)* %0, align 2
  ret void
}

; CHECK: mov              [[PRED:%SP[0-9]+]], %S3
; CHECK: nearbyint.f8_152 rhne [[VAL:%S[0-9]+]], %S1, [[PRED]]
; CHECK: st_l             %S0, [[VAL]]

; Function Attrs: nounwind readnone
declare f8_152 @llvm.tpc.nearbyint.f8_152.f8_152.i1(f8_152, i8, i32, f8_152, i1, i1)
