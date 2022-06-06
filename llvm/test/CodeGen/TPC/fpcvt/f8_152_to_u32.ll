; RUN: llc -mcpu gaudi2 %s -o - | FileCheck %s --check-prefix=GEN4

target triple = "tpc"

define void @main(i32 %dest, f8_152 %x) {
entry:
  %0 = inttoptr i32 %dest to i32 addrspace(1)*
  %1 = fptoui f8_152 %x to i32
  store i32 %1, i32 addrspace(1)* %0, align 4
  ret void
}

; GEN4: convert.f8_152 target_type=uint32 [[VAL:%S[0-9]+]], %S1
; GEN4: st_l           %S0, [[VAL]]
