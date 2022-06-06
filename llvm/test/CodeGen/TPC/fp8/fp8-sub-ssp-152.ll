; RUN: llc -mcpu gaudi2 %s -o - | FileCheck %s

target triple = "tpc"

define void @main(float %x0, f8_152 %x1, float %res0, i32 %dest, i1 zeroext %pred) {
entry:
  %0 = inttoptr i32 %dest to float addrspace(1)*
  %1 = call float @llvm.tpc.sub.f32.f32.f8_152.i1(float %x0, f8_152 %x1, i8 12, i32 0, float %res0, i1 %pred, i1 false)
  store float %1, float addrspace(1)* %0, align 4
  ret void
}

; Function Attrs: nounwind readnone
declare float @llvm.tpc.sub.f32.f32.f8_152.i1(float, f8_152, i8, i32, float, i1, i1)

; CHECK: mov         [[PRED:%SP[0-9]+]], %S4
; CHECK: sub.f8_152  %S2, %S0, %S1, [[PRED]]
; CHECK: st_l        %S3, %S2
