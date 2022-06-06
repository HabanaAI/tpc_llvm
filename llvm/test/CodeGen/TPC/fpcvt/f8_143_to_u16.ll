; RUN: llc -mcpu gaudi2 %s -o - | FileCheck %s --check-prefix=GEN4

target triple = "tpc"

define void @main(i32 %dest, f8_143 %x) {
entry:
  %0 = inttoptr i32 %dest to i16 addrspace(1)*
  %1 = fptoui f8_143 %x to i16
  store i16 %1, i16 addrspace(1)* %0, align 4
  ret void
}

; GEN4: convert.f8_143 target_type=uint16 [[VAL:%S[0-9]+]], %S1
; GEN4: st_l           %S0, [[VAL]]
