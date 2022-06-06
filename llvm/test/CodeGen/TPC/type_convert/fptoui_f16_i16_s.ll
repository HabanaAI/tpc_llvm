; RUN: llc -march=tpc -mcpu=goya2 -O0 %s -o - | FileCheck %s
; XFAIL:*
; LLVM-938

target triple = "tpc"

; Function Attrs: noinline nounwind
define dso_local void @func(half %x) #0 {
entry:
  %conv = fptoui half %x to i16
  store i16 %conv, i16 addrspace(1)* inttoptr (i16 256 to i16 addrspace(1)*), align 4
  ret void
}

; CHECK: convert.f16 target_type=int16  [[S1:%S[0-9]+]], [[S0:%S[0-9]+]]
; CHECK: convert.i16 target_type=uint16 rhne [[S0]], [[S1]]
; CHECK: st_l  0x100, [[S0]]
; CHECK: halt
