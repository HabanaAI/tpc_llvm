; RUN: llc %s -o - | FileCheck %s

target datalayout = "e-p0:32:32:32-p1:32:32:32-p2:32:32:32-p3:64:64:64-i32:32:32-i16:16:16-i8:8:8-n8:16:32-f16:16:16-f32:32:32-v256:2048:2048-v2048:2048:2048"
target triple = "tpc-none-none"

define void @func(i32 %x, i32 %y) {
entry:
  %cmp = icmp slt i32 %x, %y
  %conv = sext i1 %cmp to i8
  store i8 %conv, i8 addrspace(1)* inttoptr (i32 256 to i8 addrspace(1)*), align 4
  ret void
}

; CHECK: cmp_less.i32 %SP1, %S0, %S1
; CHECK: mov.i8 %S0, -0x1, %SP1
; CHECK: mov.i8 %S0, 0x0, !%SP1
; CHECK: st_l 0x100, %S0
; CHECK: halt
