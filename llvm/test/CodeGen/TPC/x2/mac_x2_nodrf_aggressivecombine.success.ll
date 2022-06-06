; RUN: llc -march=tpc -mcpu gaudi2 -tpc-enable-x2=true -aggressive-x2-combine=true %s -o - | FileCheck %s

; CHECK: mac.f32 x2 %D[[RES:[0-9]+]], %D[[X0:[0-9]+]], %V[[X1:[0-9]+]], %V[[X2:[0-9]+]]

; Function Attrs: norecurse nounwind writeonly mustprogress
define dso_local void @main() local_unnamed_addr #0 {
entry:
  %0 = tail call <64 x float> @llvm.tpc.ld.tnsr.low.v64f32.i1(<5 x i32> zeroinitializer, i8 0, i32 0, <64 x float> undef, i1 true, i1 false)
  %1 = tail call <64 x float> @llvm.tpc.ld.tnsr.low.v64f32.i1(<5 x i32> <i32 1, i32 0, i32 0, i32 0, i32 0>, i8 0, i32 0, <64 x float> undef, i1 true, i1 false)
  %2 = tail call <64 x float> @llvm.tpc.ld.tnsr.low.v64f32.i1(<5 x i32> <i32 2, i32 0, i32 0, i32 0, i32 0>, i8 0, i32 0, <64 x float> undef, i1 true, i1 false)
  %3 = tail call <64 x float> @llvm.tpc.mac.v64f32.v64f32.i1(<64 x float> undef, <64 x float> %2, i8 0, i32 0, <64 x float> %0, i1 true, i1 false)
  %4 = tail call <64 x float> @llvm.tpc.mac.v64f32.v64f32.i1(<64 x float> undef, <64 x float> %2, i8 0, i32 0, <64 x float> %1, i1 true, i1 false)
  tail call void @llvm.tpc.st.tnsr.low.v64f32(<5 x i32> zeroinitializer, i8 1, <64 x float> %3, i32 0, i1 true, i1 false)
  tail call void @llvm.tpc.st.tnsr.low.v64f32(<5 x i32> <i32 1, i32 0, i32 0, i32 0, i32 0>, i8 1, <64 x float> %4, i32 0, i1 true, i1 false)
  ret void
}

; Function Attrs: nounwind readnone
declare <64 x float> @llvm.tpc.ld.tnsr.low.v64f32.i1(<5 x i32>, i8, i32, <64 x float>, i1, i1) #1

; Function Attrs: nounwind readnone
declare <64 x float> @llvm.tpc.mac.v64f32.v64f32.i1(<64 x float>, <64 x float>, i8, i32, <64 x float>, i1, i1) #1

; Function Attrs: nounwind writeonly
declare void @llvm.tpc.st.tnsr.low.v64f32(<5 x i32>, i8, <64 x float>, i32, i1, i1) #2

attributes #0 = { norecurse nounwind writeonly mustprogress "disable-tail-calls"="false" "frame-pointer"="all" "less-precise-fpmad"="false" "min-legal-vector-width"="0" "no-infs-fp-math"="false" "no-jump-tables"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="true" "stack-protector-buffer-size"="8" "target-cpu"="gaudi2" "target-features"="+gaudi2" "tpc-kernel"="true" "uniform-work-group-size"="true" "unsafe-fp-math"="false" "use-soft-float"="false" }
attributes #1 = { nounwind readnone }
attributes #2 = { nounwind writeonly }

!llvm.linker.options = !{}
!llvm.module.flags = !{!0}
!llvm.ident = !{!1}

!0 = !{i32 1, !"wchar_size", i32 4}
!1 = !{!"clang version 12.0.1 (ssh://gerrit:29418/tpc_llvm10 050d7c84e922f4cc4766a8d69a5882d58bcfcb38)"}
!2 = !{i32 0}
