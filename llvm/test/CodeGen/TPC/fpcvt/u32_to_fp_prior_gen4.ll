; RUN: llc -mcpu gaudi %s -o - | FileCheck --check-prefixes=CHECK,GAUDI %s
; RUN: llc -mcpu greco %s -o - | FileCheck --check-prefixes=CHECK,GRECO %s

; Unsigned 32-bit to f32 and bf16 before Gen4, as Habana's compiler does it.
; Before, both converted x & 0x7fffffff as a signed value, so values from 2^31
; up lost their top bit ((float)0x80000001 was 1.0).
; - Gaudi has no UINT32 to f32 conversion:
;   x > INT32_MAX ? f32(x >> 1) * 2 : f32(x).
; - Greco converts UINT32 to f32 natively, and both convert UINT32 to bf16
;   natively.

target triple = "tpc"

define void @u32_f32(i32 %dest, i32 %x) {
entry:
  %0 = inttoptr i32 %dest to float addrspace(1)*
  %1 = uitofp i32 %x to float
  store float %1, float addrspace(1)* %0, align 4
  ret void
}

; CHECK-LABEL: u32_f32:
; GAUDI-DAG:  shr.i32  [[HALF:%S[0-9]+]], %S1, 0x1
; GAUDI-DAG:  cmp_less.i32  [[BIG:%SP[0-9]+]], %S1, 0x0
; GAUDI-DAG:  convert.i32 target_type=fp32 rhne [[SMALL:%S[0-9]+]], %S1
; GAUDI-DAG:  convert.i32 target_type=fp32 rhne [[FHALF:%S[0-9]+]], [[HALF]]
; GAUDI:      add.f32  [[FBIG:%S[0-9]+]], [[FHALF]], [[FHALF]]
; GAUDI:      mov.f32  [[FBIG]], [[SMALL]], ![[BIG]]
; GAUDI:      st_l  %S0, [[FBIG]]
; GRECO:      convert.u32 target_type=fp32 rhne [[VAL:%S[0-9]+]], %S1
; GRECO:      st_l  %S0, [[VAL]]

define void @u32_bf16(i32 %dest, i32 %x) {
entry:
  %0 = inttoptr i32 %dest to bfloat addrspace(1)*
  %1 = uitofp i32 %x to bfloat
  store bfloat %1, bfloat addrspace(1)* %0, align 2
  ret void
}

; CHECK-LABEL: u32_bf16:
; CHECK:       convert.u32 target_type=bf16 rhne [[VAL:%S[0-9]+]], %S1
; CHECK:       st_l  %S0, [[VAL]]
