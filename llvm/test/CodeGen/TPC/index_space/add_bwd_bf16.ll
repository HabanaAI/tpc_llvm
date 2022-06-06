; RUN: llc -march=tpc -mcpu=gaudi -O2 -emit-index-factors=true %s -o - | FileCheck %s --check-prefix=CHECK-LLVM
; RUN: llc -march=tpc -mcpu=gaudi -O2 -emit-index-factors=true -index-space-mlir %s -o - | FileCheck %s --check-prefix=CHECK-MLIR

; CHECK-LLVM: "SCEVCost From TPC Index":
; CHECK-LLVM: .ascii " SCEVBEGIN IndexSpace:[0].[Input].{ 128, 4, 1, 1, 1 }[1].[Output].{ 128, 4, 1, 1, 1 }[2].[Output].{ 128, 4, 1, 1, 1 } #SCEVEND"

; CHECK-MLIR: "SCEVCost From TPC Index":
; CHECK-MLIR: .ascii " SCEVBEGIN IndexSpace:[0].[Input].{ 128, 4, 1, 1, 1 }[1].[Output].{ 128, 4, 1, 1, 1 }[2].[Output].{ 128, 4, 1, 1, 1 } #SCEVEND"

; ModuleID = './kernels/gaudi/elementwise/binary/add_bwd_bf16.c'
source_filename = "./kernels/gaudi/elementwise/binary/add_bwd_bf16.c"
target datalayout = "e-p0:32:32:32-p1:32:32:32-p2:32:32:32-p3:64:64:64-i32:32:32-n8:16:32-f16:16:16-f32:32:32-v160:32:32-v256:2048:2048-v2048:2048:2048-v4096:2048:2048-v8192:2048:2048"
target triple = "tpc"

; Function Attrs: nounwind writeonly
define dso_local void @main() local_unnamed_addr #0 !unroll_info !3 {
  %1 = tail call <5 x i32> @llvm.tpc.get.index.space.offset()
  %2 = tail call <5 x i32> @llvm.tpc.get.index.space.size()
  %3 = add <5 x i32> %2, %1
  %4 = extractelement <5 x i32> %1, i32 0
  %5 = shl nsw i32 %4, 7
  %6 = extractelement <5 x i32> %3, i32 0
  %7 = shl nsw i32 %6, 7
  %8 = extractelement <5 x i32> %1, i32 1
  %9 = shl nsw i32 %8, 2
  %10 = extractelement <5 x i32> %3, i32 1
  %11 = shl i32 %10, 2
  %12 = extractelement <5 x i32> %1, i32 2
  %13 = extractelement <5 x i32> %3, i32 2
  %14 = extractelement <5 x i32> %1, i32 3
  %15 = extractelement <5 x i32> %3, i32 3
  %16 = extractelement <5 x i32> %1, i32 4
  %17 = extractelement <5 x i32> %3, i32 4
  %18 = insertelement <5 x i32> undef, i32 %5, i32 0
  %19 = insertelement <5 x i32> %18, i32 %9, i32 1
  %20 = insertelement <5 x i32> %19, i32 %12, i32 2
  %21 = insertelement <5 x i32> %20, i32 %14, i32 3
  %22 = insertelement <5 x i32> %21, i32 %16, i32 4
  %23 = icmp slt i32 %5, %7
  br i1 %23, label %24, label %30

24:                                               ; preds = %0
  %25 = icmp slt i32 %16, %17
  %26 = icmp slt i32 %14, %15
  %27 = icmp slt i32 %12, %13
  %28 = icmp slt i32 %9, %11
  br label %31

29:                                               ; preds = %41
  br label %30

30:                                               ; preds = %29, %0
  ret void

31:                                               ; preds = %24, %41
  %32 = phi <5 x i32> [ %22, %24 ], [ %43, %41 ]
  %33 = phi i32 [ %5, %24 ], [ %44, %41 ]
  %34 = phi <5 x i32> [ %22, %24 ], [ %42, %41 ]
  %35 = insertelement <5 x i32> %32, i32 %33, i32 0
  %36 = insertelement <5 x i32> %34, i32 %33, i32 0
  br i1 %25, label %37, label %41

37:                                               ; preds = %31
  br label %46

38:                                               ; preds = %56
  %39 = phi <5 x i32> [ %57, %56 ]
  %40 = phi <5 x i32> [ %58, %56 ]
  br label %41

41:                                               ; preds = %38, %31
  %42 = phi <5 x i32> [ %36, %31 ], [ %39, %38 ]
  %43 = phi <5 x i32> [ %35, %31 ], [ %40, %38 ]
  %44 = add nsw i32 %33, 128
  %45 = icmp slt i32 %44, %7
  br i1 %45, label %31, label %29, !llvm.loop !4

46:                                               ; preds = %37, %56
  %47 = phi <5 x i32> [ %58, %56 ], [ %35, %37 ]
  %48 = phi i32 [ %59, %56 ], [ %16, %37 ]
  %49 = phi <5 x i32> [ %57, %56 ], [ %36, %37 ]
  %50 = insertelement <5 x i32> %47, i32 %48, i32 4
  %51 = insertelement <5 x i32> %49, i32 %48, i32 4
  br i1 %26, label %52, label %56

52:                                               ; preds = %46
  br label %61

53:                                               ; preds = %71
  %54 = phi <5 x i32> [ %72, %71 ]
  %55 = phi <5 x i32> [ %73, %71 ]
  br label %56

56:                                               ; preds = %53, %46
  %57 = phi <5 x i32> [ %51, %46 ], [ %54, %53 ]
  %58 = phi <5 x i32> [ %50, %46 ], [ %55, %53 ]
  %59 = add i32 %48, 1
  %60 = icmp eq i32 %59, %17
  br i1 %60, label %38, label %46, !llvm.loop !6

61:                                               ; preds = %52, %71
  %62 = phi <5 x i32> [ %73, %71 ], [ %50, %52 ]
  %63 = phi i32 [ %74, %71 ], [ %14, %52 ]
  %64 = phi <5 x i32> [ %72, %71 ], [ %51, %52 ]
  %65 = insertelement <5 x i32> %62, i32 %63, i32 3
  %66 = insertelement <5 x i32> %64, i32 %63, i32 3
  br i1 %27, label %67, label %71

67:                                               ; preds = %61
  br label %76

68:                                               ; preds = %86
  %69 = phi <5 x i32> [ %87, %86 ]
  %70 = phi <5 x i32> [ %88, %86 ]
  br label %71

71:                                               ; preds = %68, %61
  %72 = phi <5 x i32> [ %66, %61 ], [ %69, %68 ]
  %73 = phi <5 x i32> [ %65, %61 ], [ %70, %68 ]
  %74 = add i32 %63, 1
  %75 = icmp eq i32 %74, %15
  br i1 %75, label %53, label %61, !llvm.loop !7

76:                                               ; preds = %67, %86
  %77 = phi <5 x i32> [ %88, %86 ], [ %65, %67 ]
  %78 = phi i32 [ %89, %86 ], [ %12, %67 ]
  %79 = phi <5 x i32> [ %87, %86 ], [ %66, %67 ]
  %80 = insertelement <5 x i32> %79, i32 %78, i32 2
  %81 = insertelement <5 x i32> %77, i32 %78, i32 2
  br i1 %28, label %82, label %86

82:                                               ; preds = %76
  br label %91

83:                                               ; preds = %91
  %84 = phi <5 x i32> [ %107, %91 ]
  %85 = phi <5 x i32> [ %108, %91 ]
  br label %86

86:                                               ; preds = %83, %76
  %87 = phi <5 x i32> [ %80, %76 ], [ %85, %83 ]
  %88 = phi <5 x i32> [ %81, %76 ], [ %84, %83 ]
  %89 = add i32 %78, 1
  %90 = icmp eq i32 %89, %13
  br i1 %90, label %68, label %76, !llvm.loop !8

91:                                               ; preds = %82, %91
  %92 = phi <5 x i32> [ %107, %91 ], [ %81, %82 ]
  %93 = phi i32 [ %110, %91 ], [ %9, %82 ]
  %94 = phi <5 x i32> [ %108, %91 ], [ %80, %82 ]
  %95 = insertelement <5 x i32> %92, i32 %93, i32 1
  %96 = insertelement <5 x i32> %94, i32 %93, i32 1
  %97 = tail call <128 x bfloat> @llvm.tpc.ld.tnsr.v128bf16.i1(<5 x i32> %95, i8 0, i32 0, <128 x bfloat> undef, i1 true, i1 false)
  tail call void @llvm.tpc.st.tnsr.v128bf16(<5 x i32> %96, i8 1, <128 x bfloat> %97, i32 0, i1 true, i1 false)
  tail call void @llvm.tpc.st.tnsr.v128bf16(<5 x i32> %96, i8 2, <128 x bfloat> %97, i32 0, i1 true, i1 false)
  %98 = or i32 %93, 1
  %99 = insertelement <5 x i32> %95, i32 %98, i32 1
  %100 = insertelement <5 x i32> %96, i32 %98, i32 1
  %101 = tail call <128 x bfloat> @llvm.tpc.ld.tnsr.v128bf16.i1(<5 x i32> %99, i8 0, i32 0, <128 x bfloat> undef, i1 true, i1 false)
  tail call void @llvm.tpc.st.tnsr.v128bf16(<5 x i32> %100, i8 1, <128 x bfloat> %101, i32 0, i1 true, i1 false)
  tail call void @llvm.tpc.st.tnsr.v128bf16(<5 x i32> %100, i8 2, <128 x bfloat> %101, i32 0, i1 true, i1 false)
  %102 = or i32 %93, 2
  %103 = insertelement <5 x i32> %99, i32 %102, i32 1
  %104 = insertelement <5 x i32> %100, i32 %102, i32 1
  %105 = tail call <128 x bfloat> @llvm.tpc.ld.tnsr.v128bf16.i1(<5 x i32> %103, i8 0, i32 0, <128 x bfloat> undef, i1 true, i1 false)
  tail call void @llvm.tpc.st.tnsr.v128bf16(<5 x i32> %104, i8 1, <128 x bfloat> %105, i32 0, i1 true, i1 false)
  tail call void @llvm.tpc.st.tnsr.v128bf16(<5 x i32> %104, i8 2, <128 x bfloat> %105, i32 0, i1 true, i1 false)
  %106 = or i32 %93, 3
  %107 = insertelement <5 x i32> %103, i32 %106, i32 1
  %108 = insertelement <5 x i32> %104, i32 %106, i32 1
  %109 = tail call <128 x bfloat> @llvm.tpc.ld.tnsr.v128bf16.i1(<5 x i32> %107, i8 0, i32 0, <128 x bfloat> undef, i1 true, i1 false)
  tail call void @llvm.tpc.st.tnsr.v128bf16(<5 x i32> %108, i8 1, <128 x bfloat> %109, i32 0, i1 true, i1 false)
  tail call void @llvm.tpc.st.tnsr.v128bf16(<5 x i32> %108, i8 2, <128 x bfloat> %109, i32 0, i1 true, i1 false)
  %110 = add i32 %93, 4
  %111 = icmp eq i32 %110, %11
  br i1 %111, label %83, label %91, !llvm.loop !9
}

; Function Attrs: nounwind readnone
declare <5 x i32> @llvm.tpc.get.index.space.offset() #1

; Function Attrs: nounwind readnone
declare <5 x i32> @llvm.tpc.get.index.space.size() #1

; Function Attrs: nounwind readnone
declare <128 x bfloat> @llvm.tpc.ld.tnsr.v128bf16.i1(<5 x i32>, i8, i32, <128 x bfloat>, i1, i1) #1

; Function Attrs: nounwind writeonly
declare void @llvm.tpc.st.tnsr.v128bf16(<5 x i32>, i8, <128 x bfloat>, i32, i1, i1) #2

attributes #0 = { nounwind writeonly "correctly-rounded-divide-sqrt-fp-math"="false" "disable-tail-calls"="false" "frame-pointer"="all" "less-precise-fpmad"="false" "min-legal-vector-width"="0" "no-infs-fp-math"="false" "no-jump-tables"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="false" "stack-protector-buffer-size"="8" "target-cpu"="gaudi" "target-features"="+gaudi" "unsafe-fp-math"="false" "use-soft-float"="false" }
attributes #1 = { nounwind readnone }
attributes #2 = { nounwind writeonly }

!llvm.module.flags = !{!0}
!llvm.ident = !{!1}

!0 = !{i32 1, !"wchar_size", i32 4}
!1 = !{!"clang version 10.0.1 (ssh://gerrit:29418/tpc_llvm10 a787f6fb5fe429a5ac129363b2c223f559a9c49e)"}
!2 = !{i32 0}
!3 = !{!"{4, 4}"}
!4 = distinct !{!4, !5}
!5 = !{!"llvm.loop.taken", i1 true}
!6 = distinct !{!6, !5}
!7 = distinct !{!7, !5}
!8 = distinct !{!8, !5}
!9 = distinct !{!9, !5, !10}
!10 = !{!"llvm.loop.unroll.disable"}
