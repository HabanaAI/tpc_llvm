; RUN: llc -mtriple=tpc -mcpu=gaudi2 %s -o - | FileCheck %s

; Operations on vectors wider than a register, which live in register pairs and
; quads and which InstCombine forms (the fabs of both halves of a float128
; becomes one <128 x float> fabs), are done one register at a time. They could
; not be selected.

define void @main(ptr addrspace(2) %p) #0 {
  %a = load <128 x float>, ptr addrspace(2) %p
  %fabs = call <128 x float> @llvm.fabs.v128f32(<128 x float> %a)
  store <128 x float> %fabs, ptr addrspace(2) %p
  %q = getelementptr <256 x float>, ptr addrspace(2) %p, i32 1
  %b = load <256 x float>, ptr addrspace(2) %q
  %fneg = fneg <256 x float> %b
  %fmul = fmul <256 x float> %fneg, %b
  %max = call <256 x float> @llvm.maxnum.v256f32(<256 x float> %fmul, <256 x float> %b)
  store <256 x float> %max, ptr addrspace(2) %q
  %r = getelementptr <256 x bfloat>, ptr addrspace(2) %p, i32 4
  %c = load <256 x bfloat>, ptr addrspace(2) %r
  %bmul = fmul <256 x bfloat> %c, %c
  store <256 x bfloat> %bmul, ptr addrspace(2) %r
  %s = getelementptr <256 x i16>, ptr addrspace(2) %p, i32 5
  %d = load <256 x i16>, ptr addrspace(2) %s
  %xor = xor <256 x i16> %d, splat (i16 7)
  store <256 x i16> %xor, ptr addrspace(2) %s
  %t = getelementptr <128 x i32>, ptr addrspace(2) %p, i32 6
  %e = load <128 x i32>, ptr addrspace(2) %t
  %smax = call <128 x i32> @llvm.smax.v128i32(<128 x i32> %e, <128 x i32> splat (i32 3))
  store <128 x i32> %smax, ptr addrspace(2) %t
  ret void
}

declare <128 x float> @llvm.fabs.v128f32(<128 x float>)
declare <256 x float> @llvm.maxnum.v256f32(<256 x float>, <256 x float>)
declare <128 x i32> @llvm.smax.v128i32(<128 x i32>, <128 x i32>)

attributes #0 = { "target-cpu"="gaudi2" "tpc-kernel"="true" }

; CHECK-DAG: form_fp_num.f32 force_sign0
; CHECK-DAG: form_fp_num.f32 force_sign0
; CHECK-DAG: xor.f32
; CHECK-DAG: mul.f32
; CHECK-DAG: max.f32
; CHECK-DAG: mul.bf16
; CHECK-DAG: xor.i16
; CHECK-DAG: sel_grt.i32
