; RUN: llc -march=tpc -mcpu=gaudi -O2 -emit-index-factors=true %s -o - | FileCheck %s --check-prefix=CHECK-LLVM
; RUN: llc -march=tpc -mcpu=gaudi -O2 -emit-index-factors=true -index-space-mlir %s -o - | FileCheck %s --check-prefix=CHECK-MLIR

; CHECK-LLVM: "SCEVCost From TPC Index":
; CHECK-LLVM: .ascii " SCEVBEGIN IndexSpace:[1].[Output].{ 128, 4, 1, 1, 1 } #SCEVEND"

; CHECK-MLIR: "SCEVCost From TPC Index":
; CHECK-MLIR: .ascii " SCEVBEGIN IndexSpace:[1].[Output].{ 128, 4, 1, 1, 1 } #SCEVEND"

; ModuleID = './kernels/gaudi/elementwise/unary/fill_with_const_bwd_trn_16bits.c'
source_filename = "./kernels/gaudi/elementwise/unary/fill_with_const_bwd_trn_16bits.c"
target datalayout = "e-p0:32:32:32-p1:32:32:32-p2:32:32:32-p3:64:64:64-i32:32:32-n8:16:32-f16:16:16-f32:32:32-v160:32:32-v256:2048:2048-v2048:2048:2048-v4096:2048:2048-v8192:2048:2048"
target triple = "tpc"

; Function Attrs: nounwind writeonly
define dso_local void @main(bfloat %0) local_unnamed_addr #0 {
  %2 = tail call <5 x i32> @llvm.tpc.get.index.space.offset()
  %3 = tail call <5 x i32> @llvm.tpc.get.index.space.size()
  %4 = add <5 x i32> %3, %2
  %5 = extractelement <5 x i32> %2, i32 0
  %6 = shl nsw i32 %5, 7
  %7 = extractelement <5 x i32> %4, i32 0
  %8 = shl nsw i32 %7, 7
  %9 = extractelement <5 x i32> %2, i32 1
  %10 = shl nsw i32 %9, 2
  %11 = extractelement <5 x i32> %4, i32 1
  %12 = shl nsw i32 %11, 2
  %13 = extractelement <5 x i32> %2, i32 2
  %14 = extractelement <5 x i32> %4, i32 2
  %15 = extractelement <5 x i32> %2, i32 3
  %16 = extractelement <5 x i32> %4, i32 3
  %17 = extractelement <5 x i32> %2, i32 4
  %18 = extractelement <5 x i32> %4, i32 4
  %19 = insertelement <5 x i32> <i32 0, i32 undef, i32 0, i32 0, i32 0>, i32 %10, i32 1
  %20 = insertelement <128 x bfloat> undef, bfloat %0, i32 0
  %21 = shufflevector <128 x bfloat> %20, <128 x bfloat> undef, <128 x i32> zeroinitializer
  %22 = icmp slt i32 %6, %8
  br i1 %22, label %23, label %29

23:                                               ; preds = %1
  %24 = icmp slt i32 %17, %18
  %25 = icmp slt i32 %15, %16
  %26 = icmp slt i32 %13, %14
  %27 = icmp slt i32 %10, %12
  br label %30

28:                                               ; preds = %37
  br label %29

29:                                               ; preds = %28, %1
  ret void

30:                                               ; preds = %23, %37
  %31 = phi i32 [ %6, %23 ], [ %39, %37 ]
  %32 = phi <5 x i32> [ %19, %23 ], [ %38, %37 ]
  %33 = insertelement <5 x i32> %32, i32 %31, i32 0
  br i1 %24, label %34, label %37

34:                                               ; preds = %30
  br label %41

35:                                               ; preds = %48
  %36 = phi <5 x i32> [ %49, %48 ]
  br label %37

37:                                               ; preds = %35, %30
  %38 = phi <5 x i32> [ %33, %30 ], [ %36, %35 ]
  %39 = add nsw i32 %31, 128
  %40 = icmp slt i32 %39, %8
  br i1 %40, label %30, label %28

41:                                               ; preds = %34, %48
  %42 = phi i32 [ %50, %48 ], [ %17, %34 ]
  %43 = phi <5 x i32> [ %49, %48 ], [ %33, %34 ]
  %44 = insertelement <5 x i32> %43, i32 %42, i32 4
  br i1 %25, label %45, label %48

45:                                               ; preds = %41
  br label %52

46:                                               ; preds = %59
  %47 = phi <5 x i32> [ %60, %59 ]
  br label %48

48:                                               ; preds = %46, %41
  %49 = phi <5 x i32> [ %44, %41 ], [ %47, %46 ]
  %50 = add i32 %42, 1
  %51 = icmp eq i32 %50, %18
  br i1 %51, label %35, label %41

52:                                               ; preds = %45, %59
  %53 = phi i32 [ %61, %59 ], [ %15, %45 ]
  %54 = phi <5 x i32> [ %60, %59 ], [ %44, %45 ]
  %55 = insertelement <5 x i32> %54, i32 %53, i32 3
  br i1 %26, label %56, label %59

56:                                               ; preds = %52
  br label %63

57:                                               ; preds = %70
  %58 = phi <5 x i32> [ %72, %70 ]
  br label %59

59:                                               ; preds = %57, %52
  %60 = phi <5 x i32> [ %55, %52 ], [ %58, %57 ]
  %61 = add i32 %53, 1
  %62 = icmp eq i32 %61, %16
  br i1 %62, label %46, label %52

63:                                               ; preds = %56, %70
  %64 = phi i32 [ %73, %70 ], [ %13, %56 ]
  %65 = phi <5 x i32> [ %72, %70 ], [ %55, %56 ]
  %66 = insertelement <5 x i32> %65, i32 %64, i32 2
  br i1 %27, label %67, label %70

67:                                               ; preds = %63
  br label %75

68:                                               ; preds = %75
  %69 = phi <5 x i32> [ %81, %75 ]
  br label %70

70:                                               ; preds = %68, %63
  %71 = phi <5 x i32> [ %66, %63 ], [ %69, %68 ]
  %72 = insertelement <5 x i32> %71, i32 %10, i32 1
  %73 = add i32 %64, 1
  %74 = icmp eq i32 %73, %14
  br i1 %74, label %57, label %63

75:                                               ; preds = %67, %75
  %76 = phi i32 [ %82, %75 ], [ %10, %67 ]
  %77 = phi <5 x i32> [ %81, %75 ], [ %66, %67 ]
  tail call void @llvm.tpc.st.tnsr.v128bf16(<5 x i32> %77, i8 1, <128 x bfloat> %21, i32 0, i1 true, i1 false)
  %78 = tail call <5 x i32> @llvm.tpc.add.mask.v5i32.v5i32(<5 x i32> <i32 1, i32 1, i32 1, i32 1, i32 1>, <5 x i32> %77, i32 2, i8 2, i32 0, <5 x i32> %77, i1 true, i1 false)
  tail call void @llvm.tpc.st.tnsr.v128bf16(<5 x i32> %78, i8 1, <128 x bfloat> %21, i32 0, i1 true, i1 false)
  %79 = tail call <5 x i32> @llvm.tpc.add.mask.v5i32.v5i32(<5 x i32> <i32 1, i32 1, i32 1, i32 1, i32 1>, <5 x i32> %78, i32 2, i8 2, i32 0, <5 x i32> %78, i1 true, i1 false)
  tail call void @llvm.tpc.st.tnsr.v128bf16(<5 x i32> %79, i8 1, <128 x bfloat> %21, i32 0, i1 true, i1 false)
  %80 = tail call <5 x i32> @llvm.tpc.add.mask.v5i32.v5i32(<5 x i32> <i32 1, i32 1, i32 1, i32 1, i32 1>, <5 x i32> %79, i32 2, i8 2, i32 0, <5 x i32> %79, i1 true, i1 false)
  tail call void @llvm.tpc.st.tnsr.v128bf16(<5 x i32> %80, i8 1, <128 x bfloat> %21, i32 0, i1 true, i1 false)
  %81 = tail call <5 x i32> @llvm.tpc.add.mask.v5i32.v5i32(<5 x i32> <i32 1, i32 1, i32 1, i32 1, i32 1>, <5 x i32> %80, i32 2, i8 2, i32 0, <5 x i32> %80, i1 true, i1 false)
  %82 = add nsw i32 %76, 4
  %83 = icmp slt i32 %82, %12
  br i1 %83, label %75, label %68
}

; Function Attrs: nounwind readnone
declare <5 x i32> @llvm.tpc.get.index.space.offset() #1

; Function Attrs: nounwind readnone
declare <5 x i32> @llvm.tpc.get.index.space.size() #1

; Function Attrs: nounwind writeonly
declare void @llvm.tpc.st.tnsr.v128bf16(<5 x i32>, i8, <128 x bfloat>, i32, i1, i1) #2

; Function Attrs: nounwind readnone
declare <5 x i32> @llvm.tpc.add.mask.v5i32.v5i32(<5 x i32>, <5 x i32>, i32, i8, i32, <5 x i32>, i1, i1) #1

attributes #0 = { nounwind writeonly "correctly-rounded-divide-sqrt-fp-math"="false" "disable-tail-calls"="false" "frame-pointer"="all" "less-precise-fpmad"="false" "min-legal-vector-width"="0" "no-infs-fp-math"="false" "no-jump-tables"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="false" "stack-protector-buffer-size"="8" "target-cpu"="gaudi" "target-features"="+gaudi" "unsafe-fp-math"="false" "use-soft-float"="false" }
attributes #1 = { nounwind readnone }
attributes #2 = { nounwind writeonly }

!llvm.module.flags = !{!0}
!llvm.ident = !{!1}

!0 = !{i32 1, !"wchar_size", i32 4}
!1 = !{!"clang version 10.0.1 (ssh://gerrit:29418/tpc_llvm10 a787f6fb5fe429a5ac129363b2c223f559a9c49e)"}
!2 = !{i32 0}
