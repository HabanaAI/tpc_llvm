; RUN: llc -march=tpc -mcpu gaudi2 -tpc-enable-x2=true -aggressive-x2-combine=false %s -o - | FileCheck %s

; CHECK: mac.f32 x2 %D[[RES:[0-9]+]], %D[[X0:[0-9]+]], %V[[X1:[0-9]+]], %V[[X2:[0-9]+]]

;void main(tensor input_tnr, tensor output_tnr)
;{
;    int5 ifmCoords1 = {0};
;    int5 ifmCoords2 = {1};
;    int5 ifmCoords3 = {2};
;    float128 val;
;
;    val.v1 = v_f32_ld_tnsr_low_b(ifmCoords1, input_tnr);
;    val.v2 = v_f32_ld_tnsr_low_b(ifmCoords2, input_tnr);
;    // x2 intrinsic used to avoid DRF packing getting optimized out
;    val = v_f32_mul_x2_b(val, val.v1, val.v2);
;
;    float64 x = v_f32_ld_tnsr_low_b(ifmCoords3, input_tnr);
;
;    float128 output;
;    output.v1 = v_f32_mac_b(val.v1, x, output.v1);
;    output.v2 = v_f32_mac_b(val.v2, x, output.v2);
;
;    v_f32_st_tnsr_low(ifmCoords1, output_tnr, output.v1);
;    v_f32_st_tnsr_low(ifmCoords2, output_tnr, output.v2);
;}

; Function Attrs: norecurse nounwind writeonly mustprogress
define dso_local void @main() local_unnamed_addr #0 {
entry:
  %0 = tail call <64 x float> @llvm.tpc.ld.tnsr.low.v64f32.i1(<5 x i32> zeroinitializer, i8 0, i32 0, <64 x float> undef, i1 true, i1 false)
  %val.sroa.0.0.vecblend = shufflevector <64 x float> %0, <64 x float> poison, <128 x i32> <i32 0, i32 1, i32 2, i32 3, i32 4, i32 5, i32 6, i32 7, i32 8, i32 9, i32 10, i32 11, i32 12, i32 13, i32 14, i32 15, i32 16, i32 17, i32 18, i32 19, i32 20, i32 21, i32 22, i32 23, i32 24, i32 25, i32 26, i32 27, i32 28, i32 29, i32 30, i32 31, i32 32, i32 33, i32 34, i32 35, i32 36, i32 37, i32 38, i32 39, i32 40, i32 41, i32 42, i32 43, i32 44, i32 45, i32 46, i32 47, i32 48, i32 49, i32 50, i32 51, i32 52, i32 53, i32 54, i32 55, i32 56, i32 57, i32 58, i32 59, i32 60, i32 61, i32 62, i32 63, i32 undef, i32 undef, i32 undef, i32 undef, i32 undef, i32 undef, i32 undef, i32 undef, i32 undef, i32 undef, i32 undef, i32 undef, i32 undef, i32 undef, i32 undef, i32 undef, i32 undef, i32 undef, i32 undef, i32 undef, i32 undef, i32 undef, i32 undef, i32 undef, i32 undef, i32 undef, i32 undef, i32 undef, i32 undef, i32 undef, i32 undef, i32 undef, i32 undef, i32 undef, i32 undef, i32 undef, i32 undef, i32 undef, i32 undef, i32 undef, i32 undef, i32 undef, i32 undef, i32 undef, i32 undef, i32 undef, i32 undef, i32 undef, i32 undef, i32 undef, i32 undef, i32 undef, i32 undef, i32 undef, i32 undef, i32 undef, i32 undef, i32 undef, i32 undef, i32 undef, i32 undef, i32 undef, i32 undef, i32 undef>
  %1 = tail call <64 x float> @llvm.tpc.ld.tnsr.low.v64f32.i1(<5 x i32> <i32 1, i32 0, i32 0, i32 0, i32 0>, i8 0, i32 0, <64 x float> undef, i1 true, i1 false)
  %val.sroa.0.256.vec.expand = shufflevector <64 x float> %1, <64 x float> poison, <128 x i32> <i32 undef, i32 undef, i32 undef, i32 undef, i32 undef, i32 undef, i32 undef, i32 undef, i32 undef, i32 undef, i32 undef, i32 undef, i32 undef, i32 undef, i32 undef, i32 undef, i32 undef, i32 undef, i32 undef, i32 undef, i32 undef, i32 undef, i32 undef, i32 undef, i32 undef, i32 undef, i32 undef, i32 undef, i32 undef, i32 undef, i32 undef, i32 undef, i32 undef, i32 undef, i32 undef, i32 undef, i32 undef, i32 undef, i32 undef, i32 undef, i32 undef, i32 undef, i32 undef, i32 undef, i32 undef, i32 undef, i32 undef, i32 undef, i32 undef, i32 undef, i32 undef, i32 undef, i32 undef, i32 undef, i32 undef, i32 undef, i32 undef, i32 undef, i32 undef, i32 undef, i32 undef, i32 undef, i32 undef, i32 undef, i32 0, i32 1, i32 2, i32 3, i32 4, i32 5, i32 6, i32 7, i32 8, i32 9, i32 10, i32 11, i32 12, i32 13, i32 14, i32 15, i32 16, i32 17, i32 18, i32 19, i32 20, i32 21, i32 22, i32 23, i32 24, i32 25, i32 26, i32 27, i32 28, i32 29, i32 30, i32 31, i32 32, i32 33, i32 34, i32 35, i32 36, i32 37, i32 38, i32 39, i32 40, i32 41, i32 42, i32 43, i32 44, i32 45, i32 46, i32 47, i32 48, i32 49, i32 50, i32 51, i32 52, i32 53, i32 54, i32 55, i32 56, i32 57, i32 58, i32 59, i32 60, i32 61, i32 62, i32 63>
  %val.sroa.0.256.vecblend = shufflevector <128 x float> %val.sroa.0.0.vecblend, <128 x float> %val.sroa.0.256.vec.expand, <128 x i32> <i32 0, i32 1, i32 2, i32 3, i32 4, i32 5, i32 6, i32 7, i32 8, i32 9, i32 10, i32 11, i32 12, i32 13, i32 14, i32 15, i32 16, i32 17, i32 18, i32 19, i32 20, i32 21, i32 22, i32 23, i32 24, i32 25, i32 26, i32 27, i32 28, i32 29, i32 30, i32 31, i32 32, i32 33, i32 34, i32 35, i32 36, i32 37, i32 38, i32 39, i32 40, i32 41, i32 42, i32 43, i32 44, i32 45, i32 46, i32 47, i32 48, i32 49, i32 50, i32 51, i32 52, i32 53, i32 54, i32 55, i32 56, i32 57, i32 58, i32 59, i32 60, i32 61, i32 62, i32 63, i32 192, i32 193, i32 194, i32 195, i32 196, i32 197, i32 198, i32 199, i32 200, i32 201, i32 202, i32 203, i32 204, i32 205, i32 206, i32 207, i32 208, i32 209, i32 210, i32 211, i32 212, i32 213, i32 214, i32 215, i32 216, i32 217, i32 218, i32 219, i32 220, i32 221, i32 222, i32 223, i32 224, i32 225, i32 226, i32 227, i32 228, i32 229, i32 230, i32 231, i32 232, i32 233, i32 234, i32 235, i32 236, i32 237, i32 238, i32 239, i32 240, i32 241, i32 242, i32 243, i32 244, i32 245, i32 246, i32 247, i32 248, i32 249, i32 250, i32 251, i32 252, i32 253, i32 254, i32 255>
  %2 = tail call <128 x float> @llvm.tpc.mul.x2.f32.v128f32.v128f32.v64f32.v64f32.i1(<128 x float> %val.sroa.0.256.vecblend, <64 x float> %0, <64 x float> %1, i8 0, i32 16, <128 x float> undef, i1 true, i1 false)
  %3 = tail call <64 x float> @llvm.tpc.ld.tnsr.low.v64f32.i1(<5 x i32> <i32 2, i32 0, i32 0, i32 0, i32 0>, i8 0, i32 0, <64 x float> undef, i1 true, i1 false)
  %val.sroa.0.0.vec.extract20 = shufflevector <128 x float> %2, <128 x float> poison, <64 x i32> <i32 0, i32 1, i32 2, i32 3, i32 4, i32 5, i32 6, i32 7, i32 8, i32 9, i32 10, i32 11, i32 12, i32 13, i32 14, i32 15, i32 16, i32 17, i32 18, i32 19, i32 20, i32 21, i32 22, i32 23, i32 24, i32 25, i32 26, i32 27, i32 28, i32 29, i32 30, i32 31, i32 32, i32 33, i32 34, i32 35, i32 36, i32 37, i32 38, i32 39, i32 40, i32 41, i32 42, i32 43, i32 44, i32 45, i32 46, i32 47, i32 48, i32 49, i32 50, i32 51, i32 52, i32 53, i32 54, i32 55, i32 56, i32 57, i32 58, i32 59, i32 60, i32 61, i32 62, i32 63>
  %4 = tail call <64 x float> @llvm.tpc.mac.v64f32.v64f32.i1(<64 x float> %val.sroa.0.0.vec.extract20, <64 x float> %3, i8 0, i32 0, <64 x float> undef, i1 true, i1 false)
  %val.sroa.0.256.vec.extract27 = shufflevector <128 x float> %2, <128 x float> poison, <64 x i32> <i32 64, i32 65, i32 66, i32 67, i32 68, i32 69, i32 70, i32 71, i32 72, i32 73, i32 74, i32 75, i32 76, i32 77, i32 78, i32 79, i32 80, i32 81, i32 82, i32 83, i32 84, i32 85, i32 86, i32 87, i32 88, i32 89, i32 90, i32 91, i32 92, i32 93, i32 94, i32 95, i32 96, i32 97, i32 98, i32 99, i32 100, i32 101, i32 102, i32 103, i32 104, i32 105, i32 106, i32 107, i32 108, i32 109, i32 110, i32 111, i32 112, i32 113, i32 114, i32 115, i32 116, i32 117, i32 118, i32 119, i32 120, i32 121, i32 122, i32 123, i32 124, i32 125, i32 126, i32 127>
  %5 = tail call <64 x float> @llvm.tpc.mac.v64f32.v64f32.i1(<64 x float> %val.sroa.0.256.vec.extract27, <64 x float> %3, i8 0, i32 0, <64 x float> undef, i1 true, i1 false)
  tail call void @llvm.tpc.st.tnsr.low.v64f32(<5 x i32> zeroinitializer, i8 1, <64 x float> %4, i32 0, i1 true, i1 false)
  tail call void @llvm.tpc.st.tnsr.low.v64f32(<5 x i32> <i32 1, i32 0, i32 0, i32 0, i32 0>, i8 1, <64 x float> %5, i32 0, i1 true, i1 false)
  ret void
}

; Function Attrs: nounwind readnone
declare <64 x float> @llvm.tpc.ld.tnsr.low.v64f32.i1(<5 x i32>, i8, i32, <64 x float>, i1, i1) #1

; Function Attrs: nounwind readnone
declare <128 x float> @llvm.tpc.mul.x2.f32.v128f32.v128f32.v64f32.v64f32.i1(<128 x float>, <64 x float>, <64 x float>, i8, i32, <128 x float>, i1, i1) #1

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
