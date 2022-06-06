; RUN: llc -mcpu gaudi2 %s -o - | FileCheck %s

target triple = "tpc"

define void @main(i32 %dest, i8 %x) {
entry:
  %0 = inttoptr i32 %dest to f8_152 addrspace(1)*
  %1 = uitofp i8 %x to f8_152
  store f8_152 %1, f8_152 addrspace(1)* %0, align 4
  ret void
}

; CHECK: convert.u8 target_type=f8_152 [[VAL:%S[0-9]+]], %S1
; CHECK: st_l       %S0, [[VAL]]
