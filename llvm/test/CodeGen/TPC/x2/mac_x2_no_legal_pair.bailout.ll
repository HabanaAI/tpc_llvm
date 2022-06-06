; RUN: llc -march=tpc -mcpu gaudi -tpc-enable-x2=true -aggressive-x2-combine=false %s -o - | FileCheck %s

; CHECK-NOT: mac.f32 x2

;void main(tensor input_tnr, tensor output_tnr)
;{
;    int5 ifmCoords1 = {0};
;    int5 ifmCoords2 = {1};
;    int5 ifmCoords3 = {2};
;
;    float64 x = v_f32_ld_tnsr_low_b(ifmCoords3, input_tnr);
;
;    float128 val;
;    val.v1 = v_f32_ld_tnsr_low_b(ifmCoords1, input_tnr);
;    val.v2 = v_f32_ld_tnsr_low_b(ifmCoords2, input_tnr);
;
;    // mac instructions are not independent
;    val.v1 = v_f32_mac_b(val.v1, x, 0);
;    val.v2 = v_f32_mac_b(val.v2, x, val.v1);
;
;    v_f32_st_tnsr_low(ifmCoords1, output_tnr, val.v1);
;    v_f32_st_tnsr_low(ifmCoords2, output_tnr, val.v2);
;}

; Function Attrs: norecurse nounwind writeonly mustprogress
define dso_local void @main() local_unnamed_addr #0 {
entry:
  %0 = tail call <64 x float> @llvm.tpc.ld.tnsr.low.v64f32.i1(<5 x i32> <i32 2, i32 0, i32 0, i32 0, i32 0>, i8 0, i32 0, <64 x float> undef, i1 true, i1 false)
  %1 = tail call <64 x float> @llvm.tpc.ld.tnsr.low.v64f32.i1(<5 x i32> zeroinitializer, i8 0, i32 0, <64 x float> undef, i1 true, i1 false)
  %2 = tail call <64 x float> @llvm.tpc.ld.tnsr.low.v64f32.i1(<5 x i32> <i32 1, i32 0, i32 0, i32 0, i32 0>, i8 0, i32 0, <64 x float> undef, i1 true, i1 false)
  %3 = tail call <64 x float> @llvm.tpc.mac.v64f32.v64f32.i1(<64 x float> %1, <64 x float> %0, i8 0, i32 0, <64 x float> zeroinitializer, i1 true, i1 false)
  %4 = tail call <64 x float> @llvm.tpc.mac.v64f32.v64f32.i1(<64 x float> %2, <64 x float> %0, i8 0, i32 0, <64 x float> %3, i1 true, i1 false)
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
!1 = !{!"clang version 12.0.1 (ssh://gerrit:29418/tpc_llvm10 d98a7c31531407611ecc342a2874c76840d4725b)"}
!2 = !{i32 0}
