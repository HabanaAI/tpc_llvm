; RUN: llc -mcpu gaudi2 %s -o - | FileCheck %s

target triple = "tpc"

define void @main(i32 %dest, f8_143 %x, float %res, i1 zeroext %pred) {
entry:
  %0 = inttoptr i32 %dest to float addrspace(1)*
  %1 = tail call float @llvm.tpc.convert.f32.f8_143.i1(f8_143 %x, i8 13, i32 0, float %res, i1 %pred, i1 false)
  store float %1, float addrspace(1)* %0, align 4
  ret void
}

; CHECK: mov            [[PRED:%SP[0-9]+]], %S3
; CHECK: convert.f8_143 target_type=fp32 rhne [[VAL:%S[0-9]+]], %S1, [[PRED]]
; CHECK: st_l           %S0, [[VAL]]

; Function Attrs: nounwind readnone
declare float @llvm.tpc.convert.f32.f8_143.i1(f8_143, i8, i32, float, i1, i1)
