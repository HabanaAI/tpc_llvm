; RUN: llc -mcpu gaudi2 %s -o - | FileCheck %s

target triple = "tpc"

define void @main(i32 %dest, bfloat16 %x, f8_143 %y) {
entry:
  %0 = inttoptr i32 %dest to f8_143 addrspace(1)*
  %1 = fptrunc bfloat16 %x to f8_143
  store f8_143 %1, f8_143 addrspace(1)* %0, align 4
  ret void
}

; CHECK: convert.bf16 target_type=f8_143 [[VAL:%S[0-9]+]], %S1
; CHECK: st_l         %S0, [[VAL]]
