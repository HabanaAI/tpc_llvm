; RUN: llc -mcpu gaudi %s -o - | FileCheck %s

target triple = "tpc"

define void @main(i32 %dest, bfloat16 %x) {
entry:
  %0 = inttoptr i32 %dest to i8 addrspace(1)*
  %1 = fptosi bfloat16 %x to i8
  store i8 %1, i8 addrspace(1)* %0, align 4
  ret void
}

; CHECK: convert.bf16 target_type=int8 [[VAL:%S[0-9]+]], %S1
; CHECK: st_l         %S0, [[VAL]]
