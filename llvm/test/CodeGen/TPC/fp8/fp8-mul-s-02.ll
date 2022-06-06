; RUN: llc -mcpu gaudi2 %s -o - | FileCheck %s

target triple = "tpc"

define void @main(f8_143 %x0, float %res0, i32 %dest, i1 zeroext %pred) {
entry:
  %0 = inttoptr i32 %dest to float addrspace(1)*
  %1 = call float @llvm.tpc.mul.f32.f8_143.i1(f8_143 %x0, f8_143 2.0, i8 13, i32 0, float %res0, i1 %pred, i1 false)
  store float %1, float addrspace(1)* %0, align 4
  ret void
}

; Function Attrs: nounwind readnone
declare float @llvm.tpc.mul.f32.f8_143.i1(f8_143, f8_143, i8, i32, float, i1, i1)

; CHECK: mov         [[PRED:%SP[0-9]+]], %S3
; CHECK: mul.f8_143  %S1, %S0, 0x40, [[PRED]]
; CHECK: st_l        %S2, %S1
