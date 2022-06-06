; RUN: llc -mcpu gaudi2 %s -o - | FileCheck %s

target triple = "tpc"

define void @main(i32 %dest, i16 %x) {
entry:
  %0 = inttoptr i32 %dest to f8_143 addrspace(1)*
  %1 = uitofp i16 %x to f8_143
  store f8_143 %1, f8_143 addrspace(1)* %0, align 4
  ret void
}

; CHECK: convert.u16 target_type=f8_143 [[VAL:%S[0-9]+]], %S1
; CHECK: st_l        %S0, [[VAL]]
