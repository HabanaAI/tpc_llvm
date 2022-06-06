; RUN: llc -mcpu gaudi2 %s -tpc-asm-format=2 -TPCEvent -o - | FileCheck %s
; ModuleID = './kernels/goya/elementwise/unary/leakyrelu_f32.c'
source_filename = "./kernels/goya/elementwise/unary/leakyrelu_f32.c"
target datalayout = "e-p0:32:32:32-p1:32:32:32-p2:32:32:32-p3:64:64:64-i32:32:32-n8:16:32-f16:16:16-f32:32:32-v160:32:32-v256:2048:2048-v2048:2048:2048-v4096:2048:2048-v8192:2048:2048"
target triple = "tpc"

; Function Attrs: nounwind writeonly
define dso_local void @main(float %0, float %1) local_unnamed_addr #0 {
  %3 = tail call <5 x i32> @llvm.tpc.get.index.space.offset()
  %4 = tail call <5 x i32> @llvm.tpc.get.index.space.size()
  %5 = add <5 x i32> %4, %3
  %6 = extractelement <5 x i32> %3, i32 0
  %7 = shl nsw i32 %6, 6
  %8 = extractelement <5 x i32> %5, i32 0
  %9 = shl nsw i32 %8, 6
  %10 = extractelement <5 x i32> %3, i32 1
  %11 = shl nsw i32 %10, 2
  %12 = extractelement <5 x i32> %5, i32 1
  %13 = shl i32 %12, 2
  %14 = extractelement <5 x i32> %3, i32 2
  %15 = extractelement <5 x i32> %5, i32 2
  %16 = extractelement <5 x i32> %3, i32 3
  %17 = extractelement <5 x i32> %5, i32 3
  %18 = extractelement <5 x i32> %3, i32 4
  %19 = extractelement <5 x i32> %5, i32 4
  %20 = insertelement <64 x float> undef, float %0, i32 0
  %21 = shufflevector <64 x float> %20, <64 x float> undef, <64 x i32> zeroinitializer
  %22 = insertelement <64 x float> undef, float %1, i32 0
  %23 = shufflevector <64 x float> %22, <64 x float> undef, <64 x i32> zeroinitializer
  %24 = icmp slt i32 %7, %9
  br i1 %24, label %25, label %31

25:                                               ; preds = %2
  %26 = icmp slt i32 %18, %19
  %27 = icmp slt i32 %16, %17
  %28 = icmp slt i32 %14, %15
  %29 = icmp slt i32 %11, %13
  br label %32

30:                                               ; preds = %39
  br label %31

31:                                               ; preds = %30, %2
  ret void

32:                                               ; preds = %25, %39
  %33 = phi i32 [ %7, %25 ], [ %41, %39 ]
  %34 = phi <5 x i32> [ zeroinitializer, %25 ], [ %40, %39 ]
  %35 = insertelement <5 x i32> %34, i32 %33, i32 0
  br i1 %26, label %36, label %39

36:                                               ; preds = %32
  br label %43

37:                                               ; preds = %50
  %38 = phi <5 x i32> [ %51, %50 ]
  br label %39

39:                                               ; preds = %37, %32
  %40 = phi <5 x i32> [ %35, %32 ], [ %38, %37 ]
  %41 = add nsw i32 %33, 64
  %42 = icmp slt i32 %41, %9
  br i1 %42, label %32, label %30, !llvm.loop !3

43:                                               ; preds = %36, %50
  %44 = phi i32 [ %52, %50 ], [ %18, %36 ]
  %45 = phi <5 x i32> [ %51, %50 ], [ %35, %36 ]
  %46 = insertelement <5 x i32> %45, i32 %44, i32 4
  br i1 %27, label %47, label %50

47:                                               ; preds = %43
  br label %54

48:                                               ; preds = %61
  %49 = phi <5 x i32> [ %62, %61 ]
  br label %50

50:                                               ; preds = %48, %43
  %51 = phi <5 x i32> [ %46, %43 ], [ %49, %48 ]
  %52 = add i32 %44, 1
  %53 = icmp eq i32 %52, %19
  br i1 %53, label %37, label %43, !llvm.loop !5

54:                                               ; preds = %47, %61
  %55 = phi i32 [ %63, %61 ], [ %16, %47 ]
  %56 = phi <5 x i32> [ %62, %61 ], [ %46, %47 ]
  %57 = insertelement <5 x i32> %56, i32 %55, i32 3
  br i1 %28, label %58, label %61

58:                                               ; preds = %54
  br label %65

59:                                               ; preds = %82
  %60 = phi <5 x i32> [ %83, %82 ]
  br label %61

61:                                               ; preds = %59, %54
  %62 = phi <5 x i32> [ %57, %54 ], [ %60, %59 ]
  %63 = add i32 %55, 1
  %64 = icmp eq i32 %63, %17
  br i1 %64, label %48, label %54, !llvm.loop !6

65:                                               ; preds = %58, %82
  %66 = phi i32 [ %99, %82 ], [ %14, %58 ]
  %67 = phi <5 x i32> [ %83, %82 ], [ %57, %58 ]
  %68 = insertelement <5 x i32> %67, i32 %66, i32 2
  %69 = insertelement <5 x i32> %68, i32 %11, i32 1
  %70 = insertelement <5 x i32> %68, i32 %11, i32 1
  %71 = add i32 %11, 4
  %72 = tail call <64 x float> @llvm.tpc.ld.tnsr.v64f32.i1(<5 x i32> %69, i8 0, i32 0, <64 x float> undef, i1 true, i1 false)
  %73 = call <5 x i32> @llvm.tpc.add.v5i32.i32.v5i32.i1(i32 1, <5 x i32> %69, i8 2, i32 2, <5 x i32> %69, i1 true, i1 false)
  %74 = tail call <64 x float> @llvm.tpc.ld.tnsr.v64f32.i1(<5 x i32> %73, i8 0, i32 0, <64 x float> undef, i1 true, i1 false)
  %75 = call <5 x i32> @llvm.tpc.add.v5i32.i32.v5i32.i1(i32 1, <5 x i32> %73, i8 2, i32 2, <5 x i32> %73, i1 true, i1 false)
  %76 = tail call <64 x float> @llvm.tpc.ld.tnsr.v64f32.i1(<5 x i32> %75, i8 0, i32 0, <64 x float> undef, i1 true, i1 false)
  %77 = call <5 x i32> @llvm.tpc.add.v5i32.i32.v5i32.i1(i32 1, <5 x i32> %75, i8 2, i32 2, <5 x i32> %75, i1 true, i1 false)
  %78 = tail call <64 x float> @llvm.tpc.ld.tnsr.v64f32.i1(<5 x i32> %77, i8 0, i32 0, <64 x float> undef, i1 true, i1 false)
  %79 = call <5 x i32> @llvm.tpc.add.v5i32.i32.v5i32.i1(i32 1, <5 x i32> %77, i8 2, i32 2, <5 x i32> %77, i1 true, i1 false)
  br i1 %29, label %80, label %82

80:                                               ; preds = %65
  br label %101

81:                                               ; preds = %101
  br label %82

82:                                               ; preds = %81, %65
  %83 = phi <5 x i32> [ %70, %65 ], [ %136, %81 ]
  %84 = phi <64 x float> [ %72, %65 ], [ %125, %81 ]
  %85 = phi <64 x float> [ %74, %65 ], [ %127, %81 ]
  %86 = phi <64 x float> [ %76, %65 ], [ %129, %81 ]
  %87 = phi <64 x float> [ %78, %65 ], [ %131, %81 ]
  %88 = tail call <64 x float> @llvm.tpc.sel.grt.v64f32.v64f32.v64f32.v64f32.v64f32.i1(<64 x float> %84, <64 x float> zeroinitializer, <64 x float> %21, <64 x float> %23, i8 0, i32 0, <64 x float> undef, i1 true, i1 false)
  %89 = tail call <64 x float> @llvm.tpc.sel.grt.v64f32.v64f32.v64f32.v64f32.v64f32.i1(<64 x float> %85, <64 x float> zeroinitializer, <64 x float> %21, <64 x float> %23, i8 0, i32 0, <64 x float> undef, i1 true, i1 false)
  %90 = tail call <64 x float> @llvm.tpc.sel.grt.v64f32.v64f32.v64f32.v64f32.v64f32.i1(<64 x float> %86, <64 x float> zeroinitializer, <64 x float> %21, <64 x float> %23, i8 0, i32 0, <64 x float> undef, i1 true, i1 false)
  %91 = tail call <64 x float> @llvm.tpc.sel.grt.v64f32.v64f32.v64f32.v64f32.v64f32.i1(<64 x float> %87, <64 x float> zeroinitializer, <64 x float> %21, <64 x float> %23, i8 0, i32 0, <64 x float> undef, i1 true, i1 false)
  %92 = fmul <64 x float> %84, %88
  %93 = fmul <64 x float> %85, %89
  %94 = fmul <64 x float> %86, %90
  %95 = fmul <64 x float> %87, %91
  tail call void @llvm.tpc.st.tnsr.v64f32(<5 x i32> %83, i8 1, <64 x float> %92, i32 0, i1 true, i1 false)
  %96 = call <5 x i32> @llvm.tpc.add.v5i32.i32.v5i32.i1(i32 1, <5 x i32> %83, i8 2, i32 2, <5 x i32> %83, i1 true, i1 false)
  tail call void @llvm.tpc.st.tnsr.v64f32(<5 x i32> %96, i8 1, <64 x float> %93, i32 0, i1 true, i1 false)
  %97 = call <5 x i32> @llvm.tpc.add.v5i32.i32.v5i32.i1(i32 1, <5 x i32> %96, i8 2, i32 2, <5 x i32> %96, i1 true, i1 false)
  tail call void @llvm.tpc.st.tnsr.v64f32(<5 x i32> %97, i8 1, <64 x float> %94, i32 0, i1 true, i1 false)
  %98 = call <5 x i32> @llvm.tpc.add.v5i32.i32.v5i32.i1(i32 1, <5 x i32> %97, i8 2, i32 2, <5 x i32> %97, i1 true, i1 false)
  tail call void @llvm.tpc.st.tnsr.v64f32(<5 x i32> %98, i8 1, <64 x float> %95, i32 0, i1 true, i1 false)
  %99 = add i32 %66, 1
  %100 = icmp eq i32 %99, %15
  br i1 %100, label %59, label %65, !llvm.loop !7

101:                                              ; preds = %80, %101
  %102 = phi i32 [ %137, %101 ], [ %71, %80 ]
  %103 = phi <64 x float> [ %72, %80 ], [ %125, %101 ]
  %104 = phi <64 x float> [ %74, %80 ], [ %127, %101 ]
  %105 = phi <64 x float> [ %76, %80 ], [ %129, %101 ]
  %106 = phi <64 x float> [ %78, %80 ], [ %131, %101 ]
  %107 = phi <5 x i32> [ %79, %80 ], [ %132, %101 ]
  %108 = phi <5 x i32> [ %70, %80 ], [ %136, %101 ]
  %109 = phi <5 x i32> [ %122, %101 ], [ %68, %80 ]
  %110 = insertelement <5 x i32> %109, i32 %102, i32 1
  %111 = tail call <64 x float> @llvm.tpc.sel.grt.v64f32.v64f32.v64f32.v64f32.v64f32.i1(<64 x float> %103, <64 x float> zeroinitializer, <64 x float> %21, <64 x float> %23, i8 0, i32 0, <64 x float> undef, i1 true, i1 false)
  %112 = fmul <64 x float> %103, %111
  %113 = or i32 %102, 1
  %114 = insertelement <5 x i32> %110, i32 %113, i32 1
  %115 = tail call <64 x float> @llvm.tpc.sel.grt.v64f32.v64f32.v64f32.v64f32.v64f32.i1(<64 x float> %104, <64 x float> zeroinitializer, <64 x float> %21, <64 x float> %23, i8 0, i32 0, <64 x float> undef, i1 true, i1 false)
  %116 = fmul <64 x float> %104, %115
  %117 = or i32 %102, 2
  %118 = insertelement <5 x i32> %114, i32 %117, i32 1
  %119 = tail call <64 x float> @llvm.tpc.sel.grt.v64f32.v64f32.v64f32.v64f32.v64f32.i1(<64 x float> %105, <64 x float> zeroinitializer, <64 x float> %21, <64 x float> %23, i8 0, i32 0, <64 x float> undef, i1 true, i1 false)
  %120 = fmul <64 x float> %105, %119
  %121 = or i32 %102, 3
  %122 = insertelement <5 x i32> %118, i32 %121, i32 1
  %123 = tail call <64 x float> @llvm.tpc.sel.grt.v64f32.v64f32.v64f32.v64f32.v64f32.i1(<64 x float> %106, <64 x float> zeroinitializer, <64 x float> %21, <64 x float> %23, i8 0, i32 0, <64 x float> undef, i1 true, i1 false)
  %124 = fmul <64 x float> %106, %123
  %125 = tail call <64 x float> @llvm.tpc.ld.tnsr.v64f32.i1(<5 x i32> %107, i8 0, i32 0, <64 x float> undef, i1 true, i1 false)
  %126 = call <5 x i32> @llvm.tpc.add.v5i32.i32.v5i32.i1(i32 1, <5 x i32> %107, i8 2, i32 2, <5 x i32> %107, i1 true, i1 false)
  %127 = tail call <64 x float> @llvm.tpc.ld.tnsr.v64f32.i1(<5 x i32> %126, i8 0, i32 0, <64 x float> undef, i1 true, i1 false)
  %128 = call <5 x i32> @llvm.tpc.add.v5i32.i32.v5i32.i1(i32 1, <5 x i32> %126, i8 2, i32 2, <5 x i32> %126, i1 true, i1 false)
  %129 = tail call <64 x float> @llvm.tpc.ld.tnsr.v64f32.i1(<5 x i32> %128, i8 0, i32 0, <64 x float> undef, i1 true, i1 false)
  %130 = call <5 x i32> @llvm.tpc.add.v5i32.i32.v5i32.i1(i32 1, <5 x i32> %128, i8 2, i32 2, <5 x i32> %128, i1 true, i1 false)
  %131 = tail call <64 x float> @llvm.tpc.ld.tnsr.v64f32.i1(<5 x i32> %130, i8 0, i32 0, <64 x float> undef, i1 true, i1 false)
  %132 = call <5 x i32> @llvm.tpc.add.v5i32.i32.v5i32.i1(i32 1, <5 x i32> %130, i8 2, i32 2, <5 x i32> %130, i1 true, i1 false)
  tail call void @llvm.tpc.st.tnsr.v64f32(<5 x i32> %108, i8 1, <64 x float> %112, i32 0, i1 true, i1 false)
  %133 = call <5 x i32> @llvm.tpc.add.v5i32.i32.v5i32.i1(i32 1, <5 x i32> %108, i8 2, i32 2, <5 x i32> %108, i1 true, i1 false)
  tail call void @llvm.tpc.st.tnsr.v64f32(<5 x i32> %133, i8 1, <64 x float> %116, i32 0, i1 true, i1 false)
  %134 = call <5 x i32> @llvm.tpc.add.v5i32.i32.v5i32.i1(i32 1, <5 x i32> %133, i8 2, i32 2, <5 x i32> %133, i1 true, i1 false)
  tail call void @llvm.tpc.st.tnsr.v64f32(<5 x i32> %134, i8 1, <64 x float> %120, i32 0, i1 true, i1 false)
  %135 = call <5 x i32> @llvm.tpc.add.v5i32.i32.v5i32.i1(i32 1, <5 x i32> %134, i8 2, i32 2, <5 x i32> %134, i1 true, i1 false)
  tail call void @llvm.tpc.st.tnsr.v64f32(<5 x i32> %135, i8 1, <64 x float> %124, i32 0, i1 true, i1 false)
  %136 = call <5 x i32> @llvm.tpc.add.v5i32.i32.v5i32.i1(i32 1, <5 x i32> %135, i8 2, i32 2, <5 x i32> %135, i1 true, i1 false)
  %137 = add i32 %102, 4
  %138 = icmp eq i32 %137, %13
  br i1 %138, label %81, label %101, !llvm.loop !8
}

; Function Attrs: nounwind readnone
declare <5 x i32> @llvm.tpc.get.index.space.offset() #1

; Function Attrs: nounwind readnone
declare <5 x i32> @llvm.tpc.get.index.space.size() #1

; Function Attrs: nounwind readnone
declare <64 x float> @llvm.tpc.ld.tnsr.v64f32.i1(<5 x i32>, i8, i32, <64 x float>, i1, i1) #1

; Function Attrs: nounwind readnone
declare <64 x float> @llvm.tpc.sel.grt.v64f32.v64f32.v64f32.v64f32.v64f32.i1(<64 x float>, <64 x float>, <64 x float>, <64 x float>, i8, i32, <64 x float>, i1, i1) #1

; Function Attrs: nounwind writeonly
declare void @llvm.tpc.st.tnsr.v64f32(<5 x i32>, i8, <64 x float>, i32, i1, i1) #2

; Function Attrs: nounwind readnone
declare <5 x i32> @llvm.tpc.add.v5i32.i32.v5i32.i1(i32, <5 x i32>, i8, i32, <5 x i32>, i1, i1) #1

attributes #0 = { nounwind writeonly "correctly-rounded-divide-sqrt-fp-math"="false" "disable-tail-calls"="false" "frame-pointer"="all" "less-precise-fpmad"="false" "min-legal-vector-width"="0" "no-infs-fp-math"="false" "no-jump-tables"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="false" "stack-protector-buffer-size"="8" "target-cpu"="dali" "target-features"="+dali" "unsafe-fp-math"="false" "use-soft-float"="false" }
attributes #1 = { nounwind readnone }
attributes #2 = { nounwind writeonly }

!llvm.module.flags = !{!0}
!llvm.ident = !{!1}
!llvm.tpc.scalar_data = !{!2}
!llvm.tpc.vector_data = !{!2}

!0 = !{i32 1, !"wchar_size", i32 4}
!1 = !{!"clang version 10.0.1 (ssh://gerrit:29418/tpc_llvm10 2dac91cf1f89d5a8e0add680c5be4030acfbbbc7)"}
!2 = !{i32 0}
!3 = distinct !{!3, !4}
!4 = !{!"llvm.loop.taken", i1 true}
!5 = distinct !{!5, !4}
!6 = distinct !{!6, !4}
!7 = distinct !{!7, !4}
!8 = distinct !{!8, !4, !9}
!9 = !{!"llvm.loop.unroll.disable"}
; CHECK: event spu 0x0
; CHECK: event spu 0x1
; CHECK: event spu 0x2
; CHECK: event spu 0x3
; CHECK: event spu 0x4
; CHECK: event spu 0x5
; CHECK: event spu 0x6
; CHECK: event spu 0x7
; CHECK: event spu 0x8
; CHECK: event spu 0x9
; CHECK: event spu 0xa
; CHECK: event spu 0xb