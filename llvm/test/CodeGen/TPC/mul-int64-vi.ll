; RUN: llc %s -o - | FileCheck %s

target triple = "tpc"

define void @main(i32 %dest, i32 %src1, i32 %src2) {
entry:
  %0 = inttoptr i32 %dest to <64 x i32> addrspace(2)*
  %1 = inttoptr i32 %src1 to <64 x i32> addrspace(2)*
  %2 = load <64 x i32>, <64 x i32> addrspace(2)* %1, align 256
  %mul = mul <64 x i32> %2, <i32 123, i32 123, i32 123, i32 123, i32 123, i32 123, i32 123, i32 123, i32 123, i32 123, i32 123, i32 123, i32 123, i32 123, i32 123, i32 123, i32 123, i32 123, i32 123, i32 123, i32 123, i32 123, i32 123, i32 123, i32 123, i32 123, i32 123, i32 123, i32 123, i32 123, i32 123, i32 123, i32 123, i32 123, i32 123, i32 123, i32 123, i32 123, i32 123, i32 123, i32 123, i32 123, i32 123, i32 123, i32 123, i32 123, i32 123, i32 123, i32 123, i32 123, i32 123, i32 123, i32 123, i32 123, i32 123, i32 123, i32 123, i32 123, i32 123, i32 123, i32 123, i32 123, i32 123, i32 123>
  store <64 x i32> %mul, <64 x i32> addrspace(2)* %0, align 256
  ret void
}

; CHECK: ld_l_v  %V0, %S1, 0x0
; CHECK: mul.i32 %D0, %V0, 0x7b
; CHECK: st_l_v  %S0, 0x0, %V0
