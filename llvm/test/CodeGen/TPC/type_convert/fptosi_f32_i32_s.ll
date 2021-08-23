; RUN: llc -march=tpc -mcpu=dali -O0 %s -o - | FileCheck %s

target triple = "tpc"

; Function Attrs: noinline nounwind
define dso_local void @func(float %x) #0 {
entry:
  %conv = fptosi float %x to i32
  store i32 %conv, i32 addrspace(1)* inttoptr (i32 256 to i32 addrspace(1)*), align 4
  ret void
}

; CHECK: convert.f32 target_type=int32  [[S1:%S[0-9]+]], [[S0:%S[0-9]+]], [[SP0:%SP[0-9]+]]
; CHECK: st_l  0x100, [[S1]], [[SP0]]
; CHECK: halt
