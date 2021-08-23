; RUN: llc -mcpu dali -tpc-hwwa-conv-maxint=0 %s -o - | FileCheck %s

target triple = "tpc"

define void @main(i32 %dest, float %x) {
entry:
  %0 = inttoptr i32 %dest to i16 addrspace(1)*
  %1 = fptosi float %x to i16
  store i16 %1, i16 addrspace(1)* %0, align 4
  ret void
}

; CHECK: convert.f32 target_type=int16 [[VAL:%S[0-9]+]], %S1
; CHECK: st_l        %S0, [[VAL]]
