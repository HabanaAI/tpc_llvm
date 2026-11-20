; RUN: llc -march=tpc -mcpu=gaudi2 %s -o - | FileCheck %s
; RUN: llc -march=tpc -mcpu=gaudi2 -O0 %s -o - | FileCheck %s

; A float rounded to a narrower float type and stored through a global
; pointer is converted before the store. The truncating store used to be selected as a
; plain 16-bit global store of the f32 register, which stored the low half of
; the float's bits ((half)1.5f stored 0x0000).

target triple = "tpc"

; CHECK-LABEL: to_half:
; CHECK:       convert.f32 target_type=f16 [[H:%S[0-9]+]], %S0
; CHECK:       st_g %AD{{[0-9]+}}, [[H]]
define void @to_half(float %f) {
  %p = call ptr addrspace(3) @llvm.tpc.gen.addr(<5 x i32> zeroinitializer, i8 0, i32 0, ptr addrspace(3) undef, i1 true, i1 false)
  %h = fptrunc float %f to half
  store half %h, ptr addrspace(3) %p
  ret void
}

; CHECK-LABEL: to_bf16:
; CHECK:       convert.f32 target_type=bf16 [[B:%S[0-9]+]], %S0
; CHECK:       st_g %AD{{[0-9]+}}, [[B]]
define void @to_bf16(float %f) {
  %p = call ptr addrspace(3) @llvm.tpc.gen.addr(<5 x i32> zeroinitializer, i8 0, i32 0, ptr addrspace(3) undef, i1 true, i1 false)
  %b = fptrunc float %f to bfloat
  store bfloat %b, ptr addrspace(3) %p
  ret void
}

; CHECK-LABEL: to_f8:
; CHECK-DAG:   convert.f32 target_type=f8_143 [[F:%S[0-9]+]], %S0
; CHECK-DAG:   convert.bf16 target_type=f8_152 [[G:%S[0-9]+]], %S1
; CHECK-DAG:   convert.f16 target_type=f8_143 [[K:%S[0-9]+]], %S2
; CHECK-DAG:   st_g %AD{{[0-9]+}}, [[F]]
; CHECK-DAG:   st_g %AD{{[0-9]+}}, [[G]]
; CHECK-DAG:   st_g %AD{{[0-9]+}}, [[K]]
define void @to_f8(float %f, bfloat %b, half %h) {
  %p = call ptr addrspace(3) @llvm.tpc.gen.addr(<5 x i32> zeroinitializer, i8 0, i32 0, ptr addrspace(3) undef, i1 true, i1 false)
  %q = call ptr addrspace(3) @llvm.tpc.gen.addr(<5 x i32> <i32 1, i32 0, i32 0, i32 0, i32 0>, i8 0, i32 0, ptr addrspace(3) undef, i1 true, i1 false)
  %r = call ptr addrspace(3) @llvm.tpc.gen.addr(<5 x i32> <i32 2, i32 0, i32 0, i32 0, i32 0>, i8 0, i32 0, ptr addrspace(3) undef, i1 true, i1 false)
  %x = fptrunc float %f to f8_143
  store f8_143 %x, ptr addrspace(3) %p
  %y = fptrunc bfloat %b to f8_152
  store f8_152 %y, ptr addrspace(3) %q
  %z = fptrunc half %h to f8_143
  store f8_143 %z, ptr addrspace(3) %r
  ret void
}

declare ptr addrspace(3) @llvm.tpc.gen.addr(<5 x i32>, i8, i32, ptr addrspace(3), i1, i1)
