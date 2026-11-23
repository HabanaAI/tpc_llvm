; The gaudi2 bodies that LinkTPCHeaders links for llvm.sqrt, llvm.tpc.rsqrt
; and llvm.tpc.reciprocal set the exponent with form_fp_num's special_func
; modes (SW_PRE_SQRT_RSQRT = 1, SW_POST_SQRT = 2, SW_POST_RSQRT = 3,
; SW_POST_RECIP = 4), as Habana's compiler does; gaudi has no such modes.
; LinkTPCHeaders takes the arch from main's target-cpu.
; RUN: sed 's/TARGET_CPU/gaudi2/' %s | %tpc_clang -march=gaudi2 -O2 -S -x ir - -o - | FileCheck %s --check-prefix=GAUDI2
; RUN: sed 's/TARGET_CPU/gaudi/' %s | %tpc_clang -march=gaudi -O2 -S -x ir - -o - | FileCheck %s --check-prefix=GAUDI

; sqrt and rsqrt share the first form_fp_num.
; GAUDI2-DAG: form_fp_num.f32 force_sign0 special_func=1
; GAUDI2-DAG: form_fp_num.f32 force_sign0 special_func=2
; GAUDI2-DAG: form_fp_num.f32 force_sign0 special_func=3
; GAUDI2-DAG: form_fp_num.f32 exp_add_bias special_func=4
; GAUDI2-DAG: calc_fp_special.f32 func=sqrt
; GAUDI2-DAG: calc_fp_special.f32 func=rsqrt
; GAUDI2-DAG: calc_fp_special.f32 func=recip
; GAUDI2-NOT: extract_exp
; GAUDI2-NOT: sel_

; GAUDI-NOT: special_func
; GAUDI-DAG: extract_exp.f32
; GAUDI-DAG: sel_leq.i32
; GAUDI-DAG: calc_fp_special.f32 func=sqrt
; GAUDI-DAG: calc_fp_special.f32 func=rsqrt
; GAUDI-DAG: calc_fp_special.f32 func=recip

target triple = "tpc"

define void @main() #0 {
entry:
  %v = call <64 x float> @llvm.tpc.ld.tnsr.v64f32.i1(<5 x i32> zeroinitializer, i8 0, i32 0, <64 x float> undef, i1 true, i1 false)
  %sqrt = call <64 x float> @llvm.sqrt.v64f32(<64 x float> %v)
  %rsqrt = call <64 x float> @llvm.tpc.rsqrt.v64f32(<64 x float> %v)
  %recip = call <64 x float> @llvm.tpc.reciprocal.v64f32(<64 x float> %v)
  call void @llvm.tpc.st.tnsr.v64f32(<5 x i32> zeroinitializer, i8 1, <64 x float> %sqrt, i32 0, i1 true, i1 false)
  call void @llvm.tpc.st.tnsr.v64f32(<5 x i32> zeroinitializer, i8 2, <64 x float> %rsqrt, i32 0, i1 true, i1 false)
  call void @llvm.tpc.st.tnsr.v64f32(<5 x i32> zeroinitializer, i8 3, <64 x float> %recip, i32 0, i1 true, i1 false)
  ret void
}

declare <64 x float> @llvm.tpc.ld.tnsr.v64f32.i1(<5 x i32>, i8, i32, <64 x float>, i1, i1)
declare void @llvm.tpc.st.tnsr.v64f32(<5 x i32>, i8, <64 x float>, i32, i1, i1)
declare <64 x float> @llvm.sqrt.v64f32(<64 x float>)
declare <64 x float> @llvm.tpc.rsqrt.v64f32(<64 x float>)
declare <64 x float> @llvm.tpc.reciprocal.v64f32(<64 x float>)

attributes #0 = { nounwind "target-cpu"="TARGET_CPU" "tpc-kernel"="true" }
