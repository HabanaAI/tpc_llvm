; RUN: llc -march=tpc %s -o - | FileCheck %s
; XFAIL: *
; TODO optimal lowering to sel2
target triple = "tpc"

; Function Attrs: nounwind writeonly
define dso_local void @main() local_unnamed_addr #0 {

entry:
  %0 = tail call <64 x float> @llvm.tpc.ld.tnsr.v64f32.i1(<5 x i32> zeroinitializer, i8 0, i32 0, <64 x float> undef, i1 true, i1 false)
  %1 = tail call <64 x float> @llvm.tpc.ld.tnsr.v64f32.i1(<5 x i32> zeroinitializer, i8 1, i32 0, <64 x float> undef, i1 true, i1 false)
  %2 = tail call <64 x float> @llvm.tpc.ld.tnsr.v64f32.i1(<5 x i32> zeroinitializer, i8 2, i32 0, <64 x float> undef, i1 true, i1 false)
  %3 = tail call <64 x float> @llvm.tpc.ld.tnsr.v64f32.i1(<5 x i32> zeroinitializer, i8 3, i32 0, <64 x float> undef, i1 true, i1 false)
  %cmp1 = fcmp ogt <64 x float> %0, %1
  %sel1 = select <64 x i1> %cmp1, <64 x float> %0, <64 x float> %1
  %cmp2 = fcmp ogt <64 x float> %0, %1
  %sel2 = select <64 x i1> %cmp2, <64 x float> %2, <64 x float> %3
; CHECK:    sel2_grt.f32

  ;%4 = tail call <64 x float> @llvm.tpc.sel.eq.v64f32.v64f32.v64f32.v64f32.v64f32.i1(<64 x float> %0, <64 x float> %1, <64 x float> %0, <64 x float> %1, i8 0, i32 0, <64 x float> undef, i1 true, i1 false)
  ;%5 = tail call <64 x float> @llvm.tpc.sel.eq.v64f32.v64f32.v64f32.v64f32.v64f32.i1(<64 x float> %0, <64 x float> %1, <64 x float> %2, <64 x float> %3, i8 0, i32 0, <64 x float> undef, i1 true, i1 false)
  tail call void @llvm.tpc.st.tnsr.v64f32(<5 x i32> zeroinitializer, i8 4, <64 x float> %sel1, i32 0, i1 true, i1 false)
  tail call void @llvm.tpc.st.tnsr.v64f32(<5 x i32> zeroinitializer, i8 5, <64 x float> %sel2, i32 0, i1 true, i1 false)
  ret void
}

; Function Attrs: nounwind readnone
declare <64 x float> @llvm.tpc.ld.tnsr.v64f32.i1(<5 x i32>, i8, i32, <64 x float>, i1, i1)

; Function Attrs: nounwind readnone
declare <64 x float> @llvm.tpc.sel.eq.v64f32.v64f32.v64f32.v64f32.v64f32.i1(<64 x float>, <64 x float>, <64 x float>, <64 x float>, i8, i32, <64 x float>, i1, i1)

; Function Attrs: nounwind writeonly
declare void @llvm.tpc.st.tnsr.v64f32(<5 x i32>, i8, <64 x float>, i32, i1, i1) #2

attributes #0 = { nounwind writeonly "correctly-rounded-divide-sqrt-fp-math"="false" "disable-tail-calls"="false" "frame-pointer"="all" "less-precise-fpmad"="false" "min-legal-vector-width"="0" "no-infs-fp-math"="false" "no-jump-tables"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="false" "stack-protector-buffer-size"="8" "target-cpu"="gaudi" "target-features"="+gaudi" "unsafe-fp-math"="false" "use-soft-float"="false" }
attributes #1 = { nounwind readnone }
attributes #2 = { nounwind writeonly }

!llvm.module.flags = !{!0}

!0 = !{i32 1, !"wchar_size", i32 4}
!2 = !{i32 0}
