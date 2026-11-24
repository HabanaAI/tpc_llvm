; RUN: llc -mtriple=tpc -mcpu=gaudi2 %s -o - | FileCheck %s

; A shuffle of a register pair whose result registers are whole registers of
; the operands, such as both halves from the low half, which SROA makes of a
; float128 whose halves are the same value: a pair of those registers. Only a
; half from each operand was recognized; the rest asserted.

define void @main(ptr addrspace(2) %p) #0 {
  %a = load <128 x float>, ptr addrspace(2) %p
  %lo = shufflevector <128 x float> %a, <128 x float> poison, <128 x i32> <i32 0, i32 1, i32 2, i32 3, i32 4, i32 5, i32 6, i32 7, i32 8, i32 9, i32 10, i32 11, i32 12, i32 13, i32 14, i32 15, i32 16, i32 17, i32 18, i32 19, i32 20, i32 21, i32 22, i32 23, i32 24, i32 25, i32 26, i32 27, i32 28, i32 29, i32 30, i32 31, i32 32, i32 33, i32 34, i32 35, i32 36, i32 37, i32 38, i32 39, i32 40, i32 41, i32 42, i32 43, i32 44, i32 45, i32 46, i32 47, i32 48, i32 49, i32 50, i32 51, i32 52, i32 53, i32 54, i32 55, i32 56, i32 57, i32 58, i32 59, i32 60, i32 61, i32 62, i32 63, i32 0, i32 1, i32 2, i32 3, i32 4, i32 5, i32 6, i32 7, i32 8, i32 9, i32 10, i32 11, i32 12, i32 13, i32 14, i32 15, i32 16, i32 17, i32 18, i32 19, i32 20, i32 21, i32 22, i32 23, i32 24, i32 25, i32 26, i32 27, i32 28, i32 29, i32 30, i32 31, i32 32, i32 33, i32 34, i32 35, i32 36, i32 37, i32 38, i32 39, i32 40, i32 41, i32 42, i32 43, i32 44, i32 45, i32 46, i32 47, i32 48, i32 49, i32 50, i32 51, i32 52, i32 53, i32 54, i32 55, i32 56, i32 57, i32 58, i32 59, i32 60, i32 61, i32 62, i32 63>
  %q = getelementptr <128 x float>, ptr addrspace(2) %p, i32 1
  store <128 x float> %lo, ptr addrspace(2) %q
  ret void
}

attributes #0 = { "target-cpu"="gaudi2" "tpc-kernel"="true" }

; The low register is stored as both halves.
; CHECK: ld_l_v [[LO:%V[0-9]+]], %S{{[0-9]+}}
; CHECK-NOT: ld_l_v
; CHECK-DAG: st_l_v %S{{[0-9]+}}, [[LO]]
; CHECK-DAG: st_l_v %S{{[0-9]+}}, [[LO]]
