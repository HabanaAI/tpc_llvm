; RUN: llc -mcpu gaudi2 %s -o - | FileCheck %s

target triple = "tpc"

define void @main(i32 %dest, f8_143 %x, f8_143 %res, i1 zeroext %pred) {
entry:
  %0 = inttoptr i32 %dest to f8_143 addrspace(1)*
  %1 = call f8_143 @llvm.tpc.nearbyint.f8_143.f8_143.i1(f8_143 %x, i8 13, i32 512, f8_143 %res, i1 %pred, i1 false)
  store f8_143 %1, f8_143 addrspace(1)* %0, align 2
  ret void
}

; CHECK: mov              [[PRED:%SP[0-9]+]], %S3
; CHECK: nearbyint.f8_143 rhne [[VAL:%S[0-9]+]], %S1, [[PRED]]
; CHECK: st_l             %S0, [[VAL]]


; Function Attrs: nounwind readnone
declare f8_143 @llvm.tpc.nearbyint.f8_143.f8_143.i1(f8_143, i8, i32, f8_143, i1, i1)
