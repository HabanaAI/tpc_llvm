; RUN: llc -march=tpc -mcpu=gaudi -O2 -emit-index-factors=true %s -o - | FileCheck %s --check-prefix=CHECK-LLVM
; RUN: llc -march=tpc -mcpu=gaudi -O2 -emit-index-factors=true -index-space-mlir %s -o - | FileCheck %s --check-prefix=CHECK-MLIR

; CHECK-LLVM: "SCEVCost From TPC Index":
; CHECK-LLVM: .ascii " SCEVBEGIN IndexSpace:[0].[Input].{ 64, 4, 1, 1, 1 }[1].[Input].{ 64, 4, 1, 1, 1 }[2].[Output].{ 64, 4, 1, 1, 1 } #SCEVEND"

; CHECK-MLIR: "SCEVCost From TPC Index":
; CHECK-MLIR: .ascii " SCEVBEGIN IndexSpace:[0].[Input].{ 64, 4, 1, 1, 1 }[1].[Input].{ 64, 4, 1, 1, 1 }[2].[Output].{ 64, 4, 1, 1, 1 } #SCEVEND"

; ModuleID = './kernels/gaudi/elementwise/special/tanh_bwd_f32.c'
source_filename = "./kernels/gaudi/elementwise/special/tanh_bwd_f32.c"
target datalayout = "e-p0:32:32:32-p1:32:32:32-p2:32:32:32-p3:64:64:64-i32:32:32-n8:16:32-f16:16:16-f32:32:32-v160:32:32-v256:2048:2048-v2048:2048:2048-v4096:2048:2048-v8192:2048:2048"
target triple = "tpc"

; Function Attrs: nounwind writeonly
define dso_local void @main() local_unnamed_addr #0 {
  %1 = tail call <5 x i32> @llvm.tpc.get.index.space.offset()
  %2 = tail call <5 x i32> @llvm.tpc.get.index.space.size()
  %3 = add <5 x i32> %2, %1
  %4 = extractelement <5 x i32> %1, i32 0
  %5 = shl nsw i32 %4, 6
  %6 = extractelement <5 x i32> %3, i32 0
  %7 = shl nsw i32 %6, 6
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
  %18 = icmp slt i32 %5, %7
  br i1 %18, label %19, label %25

19:                                               ; preds = %0
  %20 = icmp slt i32 %16, %17
  %21 = icmp slt i32 %14, %15
  %22 = icmp slt i32 %12, %13
  %23 = icmp slt i32 %9, %11
  br label %26

24:                                               ; preds = %33
  br label %25

25:                                               ; preds = %24, %0
  ret void

26:                                               ; preds = %19, %33
  %27 = phi <5 x i32> [ zeroinitializer, %19 ], [ %34, %33 ]
  %28 = phi i32 [ %5, %19 ], [ %35, %33 ]
  %29 = insertelement <5 x i32> %27, i32 %28, i32 0
  br i1 %20, label %30, label %33

30:                                               ; preds = %26
  br label %37

31:                                               ; preds = %44
  %32 = phi <5 x i32> [ %45, %44 ]
  br label %33

33:                                               ; preds = %31, %26
  %34 = phi <5 x i32> [ %29, %26 ], [ %32, %31 ]
  %35 = add nsw i32 %28, 64
  %36 = icmp slt i32 %35, %7
  br i1 %36, label %26, label %24

37:                                               ; preds = %30, %44
  %38 = phi <5 x i32> [ %45, %44 ], [ %29, %30 ]
  %39 = phi i32 [ %46, %44 ], [ %16, %30 ]
  %40 = insertelement <5 x i32> %38, i32 %39, i32 4
  br i1 %21, label %41, label %44

41:                                               ; preds = %37
  br label %48

42:                                               ; preds = %55
  %43 = phi <5 x i32> [ %56, %55 ]
  br label %44

44:                                               ; preds = %42, %37
  %45 = phi <5 x i32> [ %40, %37 ], [ %43, %42 ]
  %46 = add i32 %39, 1
  %47 = icmp eq i32 %46, %17
  br i1 %47, label %31, label %37

48:                                               ; preds = %41, %55
  %49 = phi <5 x i32> [ %56, %55 ], [ %40, %41 ]
  %50 = phi i32 [ %57, %55 ], [ %14, %41 ]
  %51 = insertelement <5 x i32> %49, i32 %50, i32 3
  br i1 %22, label %52, label %55

52:                                               ; preds = %48
  br label %59

53:                                               ; preds = %66
  %54 = phi <5 x i32> [ %67, %66 ]
  br label %55

55:                                               ; preds = %53, %48
  %56 = phi <5 x i32> [ %51, %48 ], [ %54, %53 ]
  %57 = add i32 %50, 1
  %58 = icmp eq i32 %57, %15
  br i1 %58, label %42, label %48

59:                                               ; preds = %52, %66
  %60 = phi <5 x i32> [ %67, %66 ], [ %51, %52 ]
  %61 = phi i32 [ %68, %66 ], [ %12, %52 ]
  %62 = insertelement <5 x i32> %60, i32 %61, i32 2
  br i1 %23, label %63, label %66

63:                                               ; preds = %59
  br label %70

64:                                               ; preds = %70
  %65 = phi <5 x i32> [ %73, %70 ]
  br label %66

66:                                               ; preds = %64, %59
  %67 = phi <5 x i32> [ %62, %59 ], [ %65, %64 ]
  %68 = add i32 %61, 1
  %69 = icmp eq i32 %68, %13
  br i1 %69, label %53, label %59

70:                                               ; preds = %63, %70
  %71 = phi <5 x i32> [ %73, %70 ], [ %62, %63 ]
  %72 = phi i32 [ %78, %70 ], [ %9, %63 ]
  %73 = insertelement <5 x i32> %71, i32 %72, i32 1
  %74 = tail call <64 x float> @llvm.tpc.ld.tnsr.v64f32.i1(<5 x i32> %73, i8 0, i32 0, <64 x float> undef, i1 true, i1 false)
  %75 = tail call <64 x float> @llvm.tpc.ld.tnsr.v64f32.i1(<5 x i32> %73, i8 1, i32 0, <64 x float> undef, i1 true, i1 false)
  %76 = fmul <64 x float> %75, %75
  %77 = tail call <64 x float> @llvm.tpc.mac.v64f32.v64f32.i1(<64 x float> %74, <64 x float> %76, i8 0, i32 2, <64 x float> %74, i1 true, i1 false)
  tail call void @llvm.tpc.st.tnsr.v64f32(<5 x i32> %73, i8 2, <64 x float> %77, i32 0, i1 true, i1 false)
  %78 = add i32 %72, 1
  %79 = icmp eq i32 %78, %11
  br i1 %79, label %64, label %70, !llvm.loop !3
}

; Function Attrs: nounwind readnone
declare <5 x i32> @llvm.tpc.get.index.space.offset() #1

; Function Attrs: nounwind readnone
declare <5 x i32> @llvm.tpc.get.index.space.size() #1

; Function Attrs: nounwind readnone
declare <64 x float> @llvm.tpc.ld.tnsr.v64f32.i1(<5 x i32>, i8, i32, <64 x float>, i1, i1) #1

; Function Attrs: nounwind readnone
declare <64 x float> @llvm.tpc.mac.v64f32.v64f32.i1(<64 x float>, <64 x float>, i8, i32, <64 x float>, i1, i1) #1

; Function Attrs: nounwind writeonly
declare void @llvm.tpc.st.tnsr.v64f32(<5 x i32>, i8, <64 x float>, i32, i1, i1) #2

attributes #0 = { nounwind writeonly "correctly-rounded-divide-sqrt-fp-math"="false" "disable-tail-calls"="false" "frame-pointer"="all" "less-precise-fpmad"="false" "min-legal-vector-width"="0" "no-infs-fp-math"="false" "no-jump-tables"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="false" "stack-protector-buffer-size"="8" "target-cpu"="gaudi" "target-features"="+gaudi" "unsafe-fp-math"="false" "use-soft-float"="false" }
attributes #1 = { nounwind readnone }
attributes #2 = { nounwind writeonly }

!llvm.module.flags = !{!0}
!llvm.ident = !{!1}

!0 = !{i32 1, !"wchar_size", i32 4}
!1 = !{!"clang version 10.0.1 (ssh://gerrit:29418/tpc_llvm10 a787f6fb5fe429a5ac129363b2c223f559a9c49e)"}
!2 = !{i32 0}
!3 = distinct !{!3, !4}
!4 = !{!"llvm.loop.machine.unroll.count", i32 4}
