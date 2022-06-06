; RUN: llc -march=tpc -mcpu=goya %s -o - | FileCheck %s
; RUN: llc -march=tpc -mcpu=gaudi2 %s -o - | FileCheck %s

define void @func(i16 signext %x) {
entry:
  %conv = sext i16 %x to i32
  store i32 %conv, i32 addrspace(1)* inttoptr (i32 256 to i32 addrspace(1)*), align 4
  ret void
}

; CHECK: convert.i16 target_type=int32 rhne [[VAL:%S[0-9]+]], %S0
; CHECK: st_l        0x100, [[VAL]]

