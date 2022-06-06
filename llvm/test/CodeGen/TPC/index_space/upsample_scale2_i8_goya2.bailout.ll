; RUN: llc -march=tpc -mcpu=goya2 -O2 -emit-index-factors=true %s -o - | FileCheck %s --check-prefix=CHECK-LLVM
; RUN: llc -march=tpc -mcpu=goya2 -O2 -emit-index-factors=true -index-space-mlir %s -o - | FileCheck %s --check-prefix=CHECK-MLIR

; CHECK-LLVM: "SCEVCost From TPC Index":
; CHECK-LLVM: .ascii " SCEVBEGIN IndexSpace:[0].[Input].{ GC_CUSTOM, GC_CUSTOM, GC_CUSTOM, GC_CUSTOM, GC_CUSTOM }[1].[Output].{ GC_CUSTOM, GC_CUSTOM, GC_CUSTOM, GC_CUSTOM, GC_CUSTOM } #SCEVEND"

; CHECK-MLIR: "SCEVCost From TPC Index":
; CHECK-MLIR: .ascii " SCEVBEGIN IndexSpace:[0].[Input].{ 0, 0, 0, 0, 0 }[1].[Output].{ 0, 0, 0, 0, 0 } #SCEVEND"

; ModuleID = './goya2/misc/upsample_scale2_i8_goya2.c'
source_filename = "./goya2/misc/upsample_scale2_i8_goya2.c"
target datalayout = "e-p0:32:32:32-p1:32:32:32-p2:32:32:32-p3:64:64:64-i32:32:32-n8:16:32-f16:16:16-f32:32:32-v160:32:32-v256:2048:2048-v2048:2048:2048-v4096:2048:2048-v8192:2048:2048"
target triple = "tpc"

; Function Attrs: nounwind writeonly
define dso_local void @main(i32 %0) local_unnamed_addr #0 {
  %2 = tail call <5 x i32> @llvm.tpc.get.index.space.offset()
  %3 = tail call <5 x i32> @llvm.tpc.get.index.space.size()
  %4 = add <5 x i32> %3, %2
  %5 = extractelement <5 x i32> %2, i32 0
  %6 = shl nsw i32 %5, 6
  %7 = extractelement <5 x i32> %4, i32 0
  %8 = shl nsw i32 %7, 6
  %9 = extractelement <5 x i32> %2, i32 1
  %10 = shl nsw i32 %9, 2
  %11 = extractelement <5 x i32> %4, i32 1
  %12 = shl nsw i32 %11, 2
  %13 = extractelement <5 x i32> %2, i32 2
  %14 = extractelement <5 x i32> %4, i32 2
  %15 = extractelement <5 x i32> %2, i32 3
  %16 = extractelement <5 x i32> %4, i32 3
  %17 = icmp slt i32 %6, %8
  br i1 %17, label %18, label %23

18:                                               ; preds = %1
  %19 = icmp slt i32 %15, %16
  %20 = icmp slt i32 %13, %14
  %21 = icmp slt i32 %10, %12
  br label %24

22:                                               ; preds = %34
  br label %23

23:                                               ; preds = %22, %1
  ret void

24:                                               ; preds = %18, %34
  %25 = phi i32 [ %6, %18 ], [ %37, %34 ]
  %26 = phi <5 x i32> [ zeroinitializer, %18 ], [ %36, %34 ]
  %27 = phi <5 x i32> [ zeroinitializer, %18 ], [ %35, %34 ]
  %28 = insertelement <5 x i32> %26, i32 %25, i32 0
  %29 = insertelement <5 x i32> %27, i32 %25, i32 0
  br i1 %19, label %30, label %34

30:                                               ; preds = %24
  br label %39

31:                                               ; preds = %49
  %32 = phi <5 x i32> [ %50, %49 ]
  %33 = phi <5 x i32> [ %51, %49 ]
  br label %34

34:                                               ; preds = %31, %24
  %35 = phi <5 x i32> [ %29, %24 ], [ %32, %31 ]
  %36 = phi <5 x i32> [ %28, %24 ], [ %33, %31 ]
  %37 = add nsw i32 %25, 64
  %38 = icmp slt i32 %37, %8
  br i1 %38, label %24, label %22

39:                                               ; preds = %30, %49
  %40 = phi i32 [ %52, %49 ], [ %15, %30 ]
  %41 = phi <5 x i32> [ %51, %49 ], [ %28, %30 ]
  %42 = phi <5 x i32> [ %50, %49 ], [ %29, %30 ]
  %43 = insertelement <5 x i32> %41, i32 %40, i32 3
  %44 = insertelement <5 x i32> %42, i32 %40, i32 3
  br i1 %20, label %45, label %49

45:                                               ; preds = %39
  br label %54

46:                                               ; preds = %65
  %47 = phi <5 x i32> [ %66, %65 ]
  %48 = phi <5 x i32> [ %67, %65 ]
  br label %49

49:                                               ; preds = %46, %39
  %50 = phi <5 x i32> [ %44, %39 ], [ %47, %46 ]
  %51 = phi <5 x i32> [ %43, %39 ], [ %48, %46 ]
  %52 = add i32 %40, 1
  %53 = icmp eq i32 %52, %16
  br i1 %53, label %31, label %39

54:                                               ; preds = %45, %65
  %55 = phi i32 [ %68, %65 ], [ %13, %45 ]
  %56 = phi <5 x i32> [ %67, %65 ], [ %43, %45 ]
  %57 = phi <5 x i32> [ %66, %65 ], [ %44, %45 ]
  %58 = insertelement <5 x i32> %56, i32 %55, i32 2
  %59 = shl nsw i32 %55, 1
  %60 = insertelement <5 x i32> %57, i32 %59, i32 2
  br i1 %21, label %61, label %65

61:                                               ; preds = %54
  br label %70

62:                                               ; preds = %70
  %63 = phi <5 x i32> [ %82, %70 ]
  %64 = phi <5 x i32> [ %102, %70 ]
  br label %65

65:                                               ; preds = %62, %54
  %66 = phi <5 x i32> [ %60, %54 ], [ %64, %62 ]
  %67 = phi <5 x i32> [ %58, %54 ], [ %63, %62 ]
  %68 = add i32 %55, 1
  %69 = icmp eq i32 %68, %14
  br i1 %69, label %46, label %54

70:                                               ; preds = %61, %70
  %71 = phi i32 [ %103, %70 ], [ %10, %61 ]
  %72 = phi <5 x i32> [ %82, %70 ], [ %58, %61 ]
  %73 = phi <5 x i32> [ %102, %70 ], [ %60, %61 ]
  %74 = insertelement <5 x i32> %72, i32 %71, i32 1
  %75 = shl nsw i32 %71, 1
  %76 = insertelement <5 x i32> %73, i32 %75, i32 1
  %77 = tail call <256 x i8> @llvm.tpc.ld.tnsr.v256i8.i1(<5 x i32> %74, i8 0, i32 0, <256 x i8> undef, i1 true, i1 false)
  %78 = tail call <5 x i32> @llvm.tpc.add.mask.v5i32.i32(<5 x i32> %74, i32 1, i32 2, i8 2, i32 0, <5 x i32> %74, i1 true, i1 false)
  %79 = tail call <256 x i8> @llvm.tpc.ld.tnsr.v256i8.i1(<5 x i32> %78, i8 0, i32 0, <256 x i8> undef, i1 true, i1 false)
  %80 = tail call <5 x i32> @llvm.tpc.add.mask.v5i32.i32(<5 x i32> %78, i32 1, i32 2, i8 2, i32 0, <5 x i32> %78, i1 true, i1 false)
  %81 = tail call <256 x i8> @llvm.tpc.ld.tnsr.v256i8.i1(<5 x i32> %80, i8 0, i32 0, <256 x i8> undef, i1 true, i1 false)
  %82 = tail call <5 x i32> @llvm.tpc.add.mask.v5i32.i32(<5 x i32> %80, i32 1, i32 2, i8 2, i32 0, <5 x i32> %80, i1 true, i1 false)
  %83 = tail call <256 x i8> @llvm.tpc.ld.tnsr.v256i8.i1(<5 x i32> %82, i8 0, i32 0, <256 x i8> undef, i1 true, i1 false)
  tail call void @llvm.tpc.st.tnsr.v256i8(<5 x i32> %76, i8 1, <256 x i8> %77, i32 0, i1 true, i1 false)
  %84 = tail call <5 x i32> @llvm.tpc.add.mask.v5i32.i32(<5 x i32> %76, i32 1, i32 2, i8 2, i32 0, <5 x i32> %76, i1 true, i1 false)
  tail call void @llvm.tpc.st.tnsr.v256i8(<5 x i32> %84, i8 1, <256 x i8> %77, i32 0, i1 true, i1 false)
  %85 = tail call <5 x i32> @llvm.tpc.add.mask.v5i32.i32(<5 x i32> %84, i32 1, i32 4, i8 2, i32 0, <5 x i32> %84, i1 true, i1 false)
  tail call void @llvm.tpc.st.tnsr.v256i8(<5 x i32> %85, i8 1, <256 x i8> %77, i32 0, i1 true, i1 false)
  %86 = tail call <5 x i32> @llvm.tpc.add.mask.v5i32.i32(<5 x i32> %85, i32 -1, i32 2, i8 2, i32 0, <5 x i32> %85, i1 true, i1 false)
  tail call void @llvm.tpc.st.tnsr.v256i8(<5 x i32> %86, i8 1, <256 x i8> %77, i32 0, i1 true, i1 false)
  %87 = tail call <5 x i32> @llvm.tpc.add.mask.v5i32.i32(<5 x i32> %86, i32 -1, i32 4, i8 2, i32 0, <5 x i32> %86, i1 true, i1 false)
  %88 = tail call <5 x i32> @llvm.tpc.add.mask.v5i32.i32(<5 x i32> %87, i32 2, i32 2, i8 2, i32 0, <5 x i32> %87, i1 true, i1 false)
  tail call void @llvm.tpc.st.tnsr.v256i8(<5 x i32> %88, i8 1, <256 x i8> %79, i32 0, i1 true, i1 false)
  %89 = tail call <5 x i32> @llvm.tpc.add.mask.v5i32.i32(<5 x i32> %88, i32 1, i32 2, i8 2, i32 0, <5 x i32> %88, i1 true, i1 false)
  tail call void @llvm.tpc.st.tnsr.v256i8(<5 x i32> %89, i8 1, <256 x i8> %79, i32 0, i1 true, i1 false)
  %90 = tail call <5 x i32> @llvm.tpc.add.mask.v5i32.i32(<5 x i32> %89, i32 1, i32 4, i8 2, i32 0, <5 x i32> %89, i1 true, i1 false)
  tail call void @llvm.tpc.st.tnsr.v256i8(<5 x i32> %90, i8 1, <256 x i8> %79, i32 0, i1 true, i1 false)
  %91 = tail call <5 x i32> @llvm.tpc.add.mask.v5i32.i32(<5 x i32> %90, i32 -1, i32 2, i8 2, i32 0, <5 x i32> %90, i1 true, i1 false)
  tail call void @llvm.tpc.st.tnsr.v256i8(<5 x i32> %91, i8 1, <256 x i8> %79, i32 0, i1 true, i1 false)
  %92 = tail call <5 x i32> @llvm.tpc.add.mask.v5i32.i32(<5 x i32> %91, i32 -1, i32 4, i8 2, i32 0, <5 x i32> %91, i1 true, i1 false)
  %93 = tail call <5 x i32> @llvm.tpc.add.mask.v5i32.i32(<5 x i32> %92, i32 2, i32 2, i8 2, i32 0, <5 x i32> %92, i1 true, i1 false)
  tail call void @llvm.tpc.st.tnsr.v256i8(<5 x i32> %93, i8 1, <256 x i8> %81, i32 0, i1 true, i1 false)
  %94 = tail call <5 x i32> @llvm.tpc.add.mask.v5i32.i32(<5 x i32> %93, i32 1, i32 2, i8 2, i32 0, <5 x i32> %93, i1 true, i1 false)
  tail call void @llvm.tpc.st.tnsr.v256i8(<5 x i32> %94, i8 1, <256 x i8> %81, i32 0, i1 true, i1 false)
  %95 = tail call <5 x i32> @llvm.tpc.add.mask.v5i32.i32(<5 x i32> %94, i32 1, i32 4, i8 2, i32 0, <5 x i32> %94, i1 true, i1 false)
  tail call void @llvm.tpc.st.tnsr.v256i8(<5 x i32> %95, i8 1, <256 x i8> %81, i32 0, i1 true, i1 false)
  %96 = tail call <5 x i32> @llvm.tpc.add.mask.v5i32.i32(<5 x i32> %95, i32 -1, i32 2, i8 2, i32 0, <5 x i32> %95, i1 true, i1 false)
  tail call void @llvm.tpc.st.tnsr.v256i8(<5 x i32> %96, i8 1, <256 x i8> %81, i32 0, i1 true, i1 false)
  %97 = tail call <5 x i32> @llvm.tpc.add.mask.v5i32.i32(<5 x i32> %96, i32 -1, i32 4, i8 2, i32 0, <5 x i32> %96, i1 true, i1 false)
  %98 = tail call <5 x i32> @llvm.tpc.add.mask.v5i32.i32(<5 x i32> %97, i32 2, i32 2, i8 2, i32 0, <5 x i32> %97, i1 true, i1 false)
  tail call void @llvm.tpc.st.tnsr.v256i8(<5 x i32> %98, i8 1, <256 x i8> %83, i32 0, i1 true, i1 false)
  %99 = tail call <5 x i32> @llvm.tpc.add.mask.v5i32.i32(<5 x i32> %98, i32 1, i32 2, i8 2, i32 0, <5 x i32> %98, i1 true, i1 false)
  tail call void @llvm.tpc.st.tnsr.v256i8(<5 x i32> %99, i8 1, <256 x i8> %83, i32 0, i1 true, i1 false)
  %100 = tail call <5 x i32> @llvm.tpc.add.mask.v5i32.i32(<5 x i32> %99, i32 1, i32 4, i8 2, i32 0, <5 x i32> %99, i1 true, i1 false)
  tail call void @llvm.tpc.st.tnsr.v256i8(<5 x i32> %100, i8 1, <256 x i8> %83, i32 0, i1 true, i1 false)
  %101 = tail call <5 x i32> @llvm.tpc.add.mask.v5i32.i32(<5 x i32> %100, i32 -1, i32 2, i8 2, i32 0, <5 x i32> %100, i1 true, i1 false)
  tail call void @llvm.tpc.st.tnsr.v256i8(<5 x i32> %101, i8 1, <256 x i8> %83, i32 0, i1 true, i1 false)
  %102 = tail call <5 x i32> @llvm.tpc.add.mask.v5i32.i32(<5 x i32> %101, i32 -1, i32 4, i8 2, i32 0, <5 x i32> %101, i1 true, i1 false)
  %103 = add nsw i32 %71, 4
  %104 = icmp slt i32 %103, %12
  br i1 %104, label %70, label %62
}

; Function Attrs: nounwind readnone
declare <5 x i32> @llvm.tpc.get.index.space.offset() #1

; Function Attrs: nounwind readnone
declare <5 x i32> @llvm.tpc.get.index.space.size() #1

; Function Attrs: nounwind readnone
declare <256 x i8> @llvm.tpc.ld.tnsr.v256i8.i1(<5 x i32>, i8, i32, <256 x i8>, i1, i1) #1

; Function Attrs: nounwind writeonly
declare void @llvm.tpc.st.tnsr.v256i8(<5 x i32>, i8, <256 x i8>, i32, i1, i1) #2

; Function Attrs: nounwind readnone
declare <5 x i32> @llvm.tpc.add.mask.v5i32.i32(<5 x i32>, i32, i32, i8, i32, <5 x i32>, i1, i1) #1

attributes #0 = { nounwind writeonly "correctly-rounded-divide-sqrt-fp-math"="false" "disable-tail-calls"="false" "frame-pointer"="all" "less-precise-fpmad"="false" "min-legal-vector-width"="0" "no-infs-fp-math"="false" "no-jump-tables"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="false" "stack-protector-buffer-size"="8" "target-cpu"="greco" "target-features"="+greco" "unsafe-fp-math"="false" "use-soft-float"="false" }
attributes #1 = { nounwind readnone }
attributes #2 = { nounwind writeonly }

!llvm.module.flags = !{!0}
!llvm.ident = !{!1}

!0 = !{i32 1, !"wchar_size", i32 4}
!1 = !{!"clang version 10.0.1 (ssh://gerrit:29418/tpc_llvm10 116063130bf9c6bf24496cb3fcd97cd02e50d4fb)"}
!2 = !{i32 0}
