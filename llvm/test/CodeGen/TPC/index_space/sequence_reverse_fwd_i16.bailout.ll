; RUN: llc -march=tpc -mcpu=gaudi -O2 -emit-index-factors=true %s -o - | FileCheck %s --check-prefix=CHECK-LLVM
; RUN: llc -march=tpc -mcpu=gaudi -O2 -emit-index-factors=true -index-space-mlir %s -o - | FileCheck %s --check-prefix=CHECK-MLIR

; CHECK-LLVM: "SCEVCost From TPC Index":
; CHECK-LLVM: .ascii " SCEVBEGIN IndexSpace:[0].[Input].{ GC_CUSTOM, GC_CUSTOM, GC_CUSTOM, GC_CUSTOM, GC_CUSTOM }[1].[Output].{ GC_CUSTOM, GC_CUSTOM, GC_CUSTOM, GC_CUSTOM, GC_CUSTOM } #SCEVEND"

; CHECK-MLIR: "SCEVCost From TPC Index":
; CHECK-MLIR: .ascii " SCEVBEGIN IndexSpace:[0].[Input].{ 0, 0, 0, 0, 0 }[1].[Output].{ 0, 0, 0, 0, 0 } #SCEVEND"

; ModuleID = './kernels/gaudi/gnmt/sequence_reverse_fwd_i16.c'
source_filename = "./kernels/gaudi/gnmt/sequence_reverse_fwd_i16.c"
target datalayout = "e-p0:32:32:32-p1:32:32:32-p2:32:32:32-p3:64:64:64-i32:32:32-n8:16:32-f16:16:16-f32:32:32-v160:32:32-v256:2048:2048-v2048:2048:2048-v4096:2048:2048-v8192:2048:2048"
target triple = "tpc"

; Function Attrs: nounwind
define dso_local void @main(i32 %0, i32 %1, i32 %2, i32 %3) local_unnamed_addr #0 !unroll_info !3 {
  %5 = shl i32 %0, 3
  %6 = add i32 %5, 1040
  %7 = tail call i32 @llvm.tpc.ld.l.i32(i32 %6, i32 1, i32 undef, i1 true, i1 false) #4
  %8 = shl i32 %3, 3
  %9 = add i32 %8, 1040
  %10 = tail call i32 @llvm.tpc.ld.l.i32(i32 %9, i32 1, i32 undef, i1 true, i1 false) #4
  %11 = shl i32 %1, 3
  %12 = add i32 %11, 1040
  %13 = tail call i32 @llvm.tpc.ld.l.i32(i32 %12, i32 1, i32 undef, i1 true, i1 false) #4
  %14 = add i32 %5, 1044
  %15 = tail call i32 @llvm.tpc.ld.l.i32(i32 %14, i32 1, i32 undef, i1 true, i1 false) #4
  %16 = add i32 %11, 1044
  %17 = tail call i32 @llvm.tpc.ld.l.i32(i32 %16, i32 1, i32 undef, i1 true, i1 false) #4
  %18 = add i32 %8, 1044
  %19 = tail call i32 @llvm.tpc.ld.l.i32(i32 %18, i32 1, i32 undef, i1 true, i1 false) #4
  tail call void @llvm.tpc.st.l.i32(i32 1048, i32 %7, i32 1, i1 true, i1 false) #4
  tail call void @llvm.tpc.st.l.i32(i32 1052, i32 %15, i32 1, i1 true, i1 false) #4
  tail call void @llvm.tpc.st.l.i32(i32 1056, i32 %13, i32 1, i1 true, i1 false) #4
  tail call void @llvm.tpc.st.l.i32(i32 1060, i32 %17, i32 1, i1 true, i1 false) #4
  tail call void @llvm.tpc.st.l.i32(i32 1064, i32 %10, i32 1, i1 true, i1 false) #4
  tail call void @llvm.tpc.st.l.i32(i32 1068, i32 %19, i32 1, i1 true, i1 false) #4
  %20 = add i32 %5, 1100
  %21 = tail call i32 @llvm.tpc.ld.l.i32(i32 %20, i32 1, i32 undef, i1 true, i1 false) #4
  %22 = add i32 %11, 1100
  %23 = tail call i32 @llvm.tpc.ld.l.i32(i32 %22, i32 1, i32 undef, i1 true, i1 false) #4
  %24 = add i32 %8, 1100
  %25 = tail call i32 @llvm.tpc.ld.l.i32(i32 %24, i32 1, i32 undef, i1 true, i1 false) #4
  tail call void @llvm.tpc.st.l.i32(i32 1104, i32 %7, i32 1, i1 true, i1 false) #4
  tail call void @llvm.tpc.st.l.i32(i32 1108, i32 %21, i32 1, i1 true, i1 false) #4
  tail call void @llvm.tpc.st.l.i32(i32 1112, i32 %13, i32 1, i1 true, i1 false) #4
  tail call void @llvm.tpc.st.l.i32(i32 1116, i32 %23, i32 1, i1 true, i1 false) #4
  tail call void @llvm.tpc.st.l.i32(i32 1120, i32 %10, i32 1, i1 true, i1 false) #4
  tail call void @llvm.tpc.st.l.i32(i32 1124, i32 %25, i32 1, i1 true, i1 false) #4
  %26 = tail call <5 x i32> @llvm.tpc.get.index.space.offset()
  %27 = tail call <5 x i32> @llvm.tpc.get.index.space.size()
  %28 = add <5 x i32> %27, %26
  %29 = extractelement <5 x i32> %26, i32 0
  %30 = shl nsw i32 %29, 7
  %31 = extractelement <5 x i32> %28, i32 0
  %32 = shl nsw i32 %31, 7
  %33 = extractelement <5 x i32> %26, i32 1
  %34 = extractelement <5 x i32> %28, i32 1
  %35 = extractelement <5 x i32> %26, i32 2
  %36 = extractelement <5 x i32> %28, i32 2
  %37 = extractelement <5 x i32> %26, i32 3
  %38 = shl i32 %37, 2
  %39 = extractelement <5 x i32> %28, i32 3
  %40 = shl nsw i32 %39, 2
  %41 = icmp slt i32 %10, %40
  %42 = select i1 %41, i32 %10, i32 %40
  %43 = icmp slt i32 %30, %32
  br i1 %43, label %44, label %54

44:                                               ; preds = %4
  %45 = icmp slt i32 %33, %34
  %46 = icmp slt i32 %35, %36
  %47 = icmp sgt i32 %42, %38
  %48 = xor i32 %38, -1
  %49 = add i32 %42, %48
  %50 = and i32 %42, 3
  %51 = icmp eq i32 %50, 0
  %52 = icmp ult i32 %49, 3
  br label %55

53:                                               ; preds = %65
  br label %54

54:                                               ; preds = %53, %4
  ret void

55:                                               ; preds = %44, %65
  %56 = phi i32 [ %30, %44 ], [ %68, %65 ]
  %57 = phi <5 x i32> [ zeroinitializer, %44 ], [ %67, %65 ]
  %58 = phi <5 x i32> [ zeroinitializer, %44 ], [ %66, %65 ]
  %59 = insertelement <5 x i32> %57, i32 %56, i32 0
  %60 = insertelement <5 x i32> %58, i32 %56, i32 0
  br i1 %45, label %61, label %65

61:                                               ; preds = %55
  br label %70

62:                                               ; preds = %80
  %63 = phi <5 x i32> [ %81, %80 ]
  %64 = phi <5 x i32> [ %82, %80 ]
  br label %65

65:                                               ; preds = %62, %55
  %66 = phi <5 x i32> [ %60, %55 ], [ %63, %62 ]
  %67 = phi <5 x i32> [ %59, %55 ], [ %64, %62 ]
  %68 = add nsw i32 %56, 128
  %69 = icmp slt i32 %68, %32
  br i1 %69, label %55, label %53, !llvm.loop !4

70:                                               ; preds = %61, %80
  %71 = phi i32 [ %83, %80 ], [ %33, %61 ]
  %72 = phi <5 x i32> [ %82, %80 ], [ %59, %61 ]
  %73 = phi <5 x i32> [ %81, %80 ], [ %60, %61 ]
  %74 = insertelement <5 x i32> %72, i32 %71, i32 1
  %75 = insertelement <5 x i32> %73, i32 %71, i32 1
  br i1 %46, label %76, label %80

76:                                               ; preds = %70
  br label %85

77:                                               ; preds = %120
  %78 = phi <5 x i32> [ %121, %120 ]
  %79 = phi <5 x i32> [ %122, %120 ]
  br label %80

80:                                               ; preds = %77, %70
  %81 = phi <5 x i32> [ %75, %70 ], [ %78, %77 ]
  %82 = phi <5 x i32> [ %74, %70 ], [ %79, %77 ]
  %83 = add i32 %71, 1
  %84 = icmp eq i32 %83, %34
  br i1 %84, label %62, label %70, !llvm.loop !6

85:                                               ; preds = %76, %120
  %86 = phi i32 [ %123, %120 ], [ %35, %76 ]
  %87 = phi <5 x i32> [ %122, %120 ], [ %74, %76 ]
  %88 = phi <5 x i32> [ %121, %120 ], [ %75, %76 ]
  %89 = insertelement <5 x i32> %87, i32 %86, i32 2
  %90 = xor i32 %86, -1
  %91 = add i32 %13, %90
  %92 = insertelement <5 x i32> %88, i32 %91, i32 2
  br i1 %47, label %93, label %120

93:                                               ; preds = %85
  br i1 %51, label %110, label %94

94:                                               ; preds = %93
  br label %95

95:                                               ; preds = %94, %95
  %96 = phi i32 [ %103, %95 ], [ %38, %94 ]
  %97 = phi <5 x i32> [ %100, %95 ], [ %89, %94 ]
  %98 = phi <5 x i32> [ %101, %95 ], [ %92, %94 ]
  %99 = phi i32 [ %104, %95 ], [ %50, %94 ]
  %100 = insertelement <5 x i32> %97, i32 %96, i32 3
  %101 = insertelement <5 x i32> %98, i32 %96, i32 3
  %102 = tail call <128 x i16> @llvm.tpc.ld.tnsr.v128i16.i1(<5 x i32> %100, i8 0, i32 0, <128 x i16> undef, i1 true, i1 false)
  tail call void @llvm.tpc.st.tnsr.v128i16(<5 x i32> %101, i8 1, <128 x i16> %102, i32 0, i1 true, i1 false)
  %103 = add nsw i32 %96, 1
  %104 = add i32 %99, -1
  %105 = icmp eq i32 %104, 0
  br i1 %105, label %106, label %95, !llvm.loop !7

106:                                              ; preds = %95
  %107 = phi <5 x i32> [ %100, %95 ]
  %108 = phi <5 x i32> [ %101, %95 ]
  %109 = phi i32 [ %103, %95 ]
  br label %110

110:                                              ; preds = %106, %93
  %111 = phi i32 [ %38, %93 ], [ %109, %106 ]
  %112 = phi <5 x i32> [ %89, %93 ], [ %107, %106 ]
  %113 = phi <5 x i32> [ %92, %93 ], [ %108, %106 ]
  %114 = phi <5 x i32> [ undef, %93 ], [ %107, %106 ]
  %115 = phi <5 x i32> [ undef, %93 ], [ %108, %106 ]
  br i1 %52, label %120, label %116

116:                                              ; preds = %110
  br label %125

117:                                              ; preds = %125
  %118 = phi <5 x i32> [ %141, %125 ]
  %119 = phi <5 x i32> [ %142, %125 ]
  br label %120

120:                                              ; preds = %117, %110, %85
  %121 = phi <5 x i32> [ %92, %85 ], [ %115, %110 ], [ %119, %117 ]
  %122 = phi <5 x i32> [ %89, %85 ], [ %114, %110 ], [ %118, %117 ]
  %123 = add i32 %86, 1
  %124 = icmp eq i32 %123, %36
  br i1 %124, label %77, label %85, !llvm.loop !10

125:                                              ; preds = %116, %125
  %126 = phi i32 [ %144, %125 ], [ %111, %116 ]
  %127 = phi <5 x i32> [ %141, %125 ], [ %112, %116 ]
  %128 = phi <5 x i32> [ %142, %125 ], [ %113, %116 ]
  %129 = insertelement <5 x i32> %127, i32 %126, i32 3
  %130 = insertelement <5 x i32> %128, i32 %126, i32 3
  %131 = tail call <128 x i16> @llvm.tpc.ld.tnsr.v128i16.i1(<5 x i32> %129, i8 0, i32 0, <128 x i16> undef, i1 true, i1 false)
  tail call void @llvm.tpc.st.tnsr.v128i16(<5 x i32> %130, i8 1, <128 x i16> %131, i32 0, i1 true, i1 false)
  %132 = add nsw i32 %126, 1
  %133 = insertelement <5 x i32> %129, i32 %132, i32 3
  %134 = insertelement <5 x i32> %130, i32 %132, i32 3
  %135 = tail call <128 x i16> @llvm.tpc.ld.tnsr.v128i16.i1(<5 x i32> %133, i8 0, i32 0, <128 x i16> undef, i1 true, i1 false)
  tail call void @llvm.tpc.st.tnsr.v128i16(<5 x i32> %134, i8 1, <128 x i16> %135, i32 0, i1 true, i1 false)
  %136 = add nsw i32 %126, 2
  %137 = insertelement <5 x i32> %133, i32 %136, i32 3
  %138 = insertelement <5 x i32> %134, i32 %136, i32 3
  %139 = tail call <128 x i16> @llvm.tpc.ld.tnsr.v128i16.i1(<5 x i32> %137, i8 0, i32 0, <128 x i16> undef, i1 true, i1 false)
  tail call void @llvm.tpc.st.tnsr.v128i16(<5 x i32> %138, i8 1, <128 x i16> %139, i32 0, i1 true, i1 false)
  %140 = add nsw i32 %126, 3
  %141 = insertelement <5 x i32> %137, i32 %140, i32 3
  %142 = insertelement <5 x i32> %138, i32 %140, i32 3
  %143 = tail call <128 x i16> @llvm.tpc.ld.tnsr.v128i16.i1(<5 x i32> %141, i8 0, i32 0, <128 x i16> undef, i1 true, i1 false)
  tail call void @llvm.tpc.st.tnsr.v128i16(<5 x i32> %142, i8 1, <128 x i16> %143, i32 0, i1 true, i1 false)
  %144 = add nsw i32 %126, 4
  %145 = icmp slt i32 %144, %42
  br i1 %145, label %125, label %117, !llvm.loop !11
}

; Function Attrs: nounwind readnone
declare <5 x i32> @llvm.tpc.get.index.space.offset() #1

; Function Attrs: nounwind readnone
declare <5 x i32> @llvm.tpc.get.index.space.size() #1

; Function Attrs: nounwind readnone
declare <128 x i16> @llvm.tpc.ld.tnsr.v128i16.i1(<5 x i32>, i8, i32, <128 x i16>, i1, i1) #1

; Function Attrs: nounwind writeonly
declare void @llvm.tpc.st.tnsr.v128i16(<5 x i32>, i8, <128 x i16>, i32, i1, i1) #2

; Function Attrs: nounwind readonly
declare i32 @llvm.tpc.ld.l.i32(i32, i32, i32, i1, i1) #3

; Function Attrs: nounwind writeonly
declare void @llvm.tpc.st.l.i32(i32, i32, i32, i1, i1) #2

attributes #0 = { nounwind "correctly-rounded-divide-sqrt-fp-math"="false" "disable-tail-calls"="false" "frame-pointer"="all" "less-precise-fpmad"="false" "min-legal-vector-width"="0" "no-infs-fp-math"="false" "no-jump-tables"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="false" "stack-protector-buffer-size"="8" "target-cpu"="gaudi" "target-features"="+gaudi" "unsafe-fp-math"="false" "use-soft-float"="false" }
attributes #1 = { nounwind readnone }
attributes #2 = { nounwind writeonly }
attributes #3 = { nounwind readonly }
attributes #4 = { nounwind }

!llvm.module.flags = !{!0}
!llvm.ident = !{!1}

!0 = !{i32 1, !"wchar_size", i32 4}
!1 = !{!"clang version 10.0.1 (ssh://gerrit:29418/tpc_llvm10 a787f6fb5fe429a5ac129363b2c223f559a9c49e)"}
!2 = !{i32 0}
!3 = !{!"{3, 4}"}
!4 = distinct !{!4, !5}
!5 = !{!"llvm.loop.taken", i1 true}
!6 = distinct !{!6, !5}
!7 = distinct !{!7, !8, !9}
!8 = !{!"llvm.loop.unroll.disable"}
!9 = !{!"llvm.loop.unroll.remainderloop.marker", i32 0}
!10 = distinct !{!10, !5}
!11 = distinct !{!11, !5, !8}
