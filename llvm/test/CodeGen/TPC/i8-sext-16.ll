; RUN: llc -march=tpc -mcpu=goya %s -o - | FileCheck %s
; RUN: llc -march=tpc -mcpu=gaudi2 %s -o - | FileCheck %s

define void @func(i8 signext %x) {
entry:
  %conv = sext i8 %x to i16
  store i16 %conv, i16 addrspace(1)* inttoptr (i32 256 to i16 addrspace(1)*), align 4
  ret void
}

; CHECK: convert.i8 target_type=int16 rhne [[VAL:%S[0-9]+]], %S0
; CHECK: st_l       0x100, [[VAL]]

