; RUN: llc -mcpu gaudib %s -o - | FileCheck %s

; Gaudi B converts 32-bit integers to f16 through f32, as Habana's compiler
; does (unsigned ones as on Gaudi: x > INT32_MAX ? f32(x >> 1) * 2 : f32(x)).

target triple = "tpc"

define void @i32_f16(i32 %dest, i32 %x) {
entry:
  %0 = inttoptr i32 %dest to half addrspace(1)*
  %1 = sitofp i32 %x to half
  store half %1, half addrspace(1)* %0, align 2
  ret void
}

; CHECK-LABEL: i32_f16:
; CHECK:       convert.i32 target_type=fp32 rhne [[F32:%S[0-9]+]], %S1
; CHECK:       convert.f32 target_type=f16 rhne [[F16:%S[0-9]+]], [[F32]]
; CHECK:       st_l  %S0, [[F16]]

define void @u32_f16(i32 %dest, i32 %x) {
entry:
  %0 = inttoptr i32 %dest to half addrspace(1)*
  %1 = uitofp i32 %x to half
  store half %1, half addrspace(1)* %0, align 2
  ret void
}

; CHECK-LABEL: u32_f16:
; CHECK:       mov.f32  [[F32:%S[0-9]+]], %S{{[0-9]+}}, !%SP{{[0-9]+}}
; CHECK:       convert.f32 target_type=f16 rhne [[F16:%S[0-9]+]], [[F32]]
; CHECK:       st_l  %S0, [[F16]]
