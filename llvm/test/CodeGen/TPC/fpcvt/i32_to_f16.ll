; RUN: llc -mcpu goya2 %s -o - | FileCheck %s

target triple = "tpc"

define void @main(i32 %dest, i32 %x) {
entry:
  %0 = inttoptr i32 %dest to half addrspace(1)*
  %1 = sitofp i32 %x to half
  store half %1, half addrspace(1)* %0, align 4
  ret void
}

; CHECK: convert.i32 target_type=f16 [[VAL:%S[0-9]+]], %S1
; CHECK: st_l        %S0, [[VAL]]
