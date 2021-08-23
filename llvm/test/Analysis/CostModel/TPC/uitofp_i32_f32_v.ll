; NOTE: Assertions have been autogenerated by utils/update_analyze_test_checks.py
; RUN: opt < %s -cost-model -cost-kind=latency -analyze -mtriple=tpc -mcpu=dali | FileCheck %s --check-prefix=LATENCY

target datalayout = "e-p0:32:32:32-p1:32:32:32-p2:32:32:32-p3:64:64:64-i32:32:32-i16:32:32-i8:32:32-n8:16:32-f16:16:16-f32:32:32-v16:32:32-v32:32:32-v160:32:32-v256:2048:2048-v2048:2048:2048-v4096:2048:2048-v8192:2048:2048"
target triple = "tpc"


; Function Attrs: noinline nounwind
define dso_local void @main() #0 {
; LATENCY-LABEL: 'main'
; LATENCY-NEXT:  Cost Model: Found an estimated cost of 4 for instruction: %0 = call <5 x i32> @llvm.tpc.get.index.space.offset()
; LATENCY-NEXT:  Cost Model: Found an estimated cost of 4 for instruction: %1 = call <64 x i32> @llvm.tpc.ld.tnsr.v64i32.i1(<5 x i32> %0, i8 0, i32 0, <64 x i32> undef, i1 true, i1 false)
; LATENCY-NEXT:  Cost Model: Found an estimated cost of 4 for instruction: %.cast = uitofp <64 x i32> %1 to <64 x float>
; LATENCY-NEXT:  Cost Model: Found an estimated cost of 2 for instruction: call void @llvm.tpc.st.tnsr.v64f32(<5 x i32> %0, i8 1, <64 x float> %.cast, i32 0, i1 true, i1 false)
; LATENCY-NEXT:  Cost Model: Found an estimated cost of 1 for instruction: ret void
;
entry:
  %0 = call <5 x i32> @llvm.tpc.get.index.space.offset()
  %1 = call <64 x i32> @llvm.tpc.ld.tnsr.v64i32.i1(<5 x i32> %0, i8 0, i32 0, <64 x i32> undef, i1 true, i1 false)
  %.cast = uitofp <64 x i32> %1 to <64 x float>
  call void @llvm.tpc.st.tnsr.v64f32(<5 x i32> %0, i8 1, <64 x float> %.cast, i32 0, i1 true, i1 false)
  ret void
}

; Function Attrs: nounwind readnone
declare <5 x i32> @llvm.tpc.get.index.space.offset() #1

; Function Attrs: nounwind readnone
declare <64 x i32> @llvm.tpc.ld.tnsr.v64i32.i1(<5 x i32>, i8, i32, <64 x i32>, i1, i1) #1

; Function Attrs: nounwind writeonly
declare void @llvm.tpc.st.tnsr.v64f32(<5 x i32>, i8, <64 x float>, i32, i1, i1) #2

; CHECK: and.i32  %V{{[0-9]+}}, %V{{[0-9]+}}, 0x7fffffff, %SP{{[0-9]+}}
; CHECK: convert.i32 lane_sel=0 target_type=fp32  %V{{[0-9]+}}, %V{{[0-9}+}, %SP{{[0-9]+}}
; CHECK: st_tnsr  0x1, %I{{[0-9]+}}, %V{{[0-9]+}}, %SP{{[0-9]+}}
; CHECK: halt
