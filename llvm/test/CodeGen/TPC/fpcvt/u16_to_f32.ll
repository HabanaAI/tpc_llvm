; RUN: llc -mcpu dali %s -o - | FileCheck %s --check-prefix=GEN1
; RUN: llc -mcpu gaudi2 %s -o - | FileCheck %s --check-prefix=GEN4

target triple = "tpc"

define void @main(i32 %dest, i16 %x) {
entry:
  %0 = inttoptr i32 %dest to float addrspace(1)*
  %1 = uitofp i16 %x to float
  store float %1, float addrspace(1)* %0, align 4
  ret void
}

; GEN1: and.i32     [[VAL1:%S[0-9]+]], %S1, 0xffff
; GEN1: convert.i32 target_type=fp32 [[VAL2:%S[0-9]+]], [[VAL1]]
; GEN1: st_l        %S0, [[VAL2]]

; GEN4: convert.u16 target_type=fp32 [[VAL:%S[0-9]+]], %S1
; GEN4: st_l        %S0, [[VAL]]
