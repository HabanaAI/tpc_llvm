; RUN: llc -mcpu gaudi2 %s -o - | FileCheck %s --check-prefix=GEN4

target triple = "tpc"

define void @main(i32 %dest, f8_143 %x) {
entry:
  %0 = inttoptr i32 %dest to i8 addrspace(1)*
  %1 = fptoui f8_143 %x to i8
  store i8 %1, i8 addrspace(1)* %0, align 4
  ret void
}

; GEN4: convert.f8_143 target_type=uint8 [[VAL:%S[0-9]+]], %S1
; GEN4: st_l           %S0, [[VAL]]
