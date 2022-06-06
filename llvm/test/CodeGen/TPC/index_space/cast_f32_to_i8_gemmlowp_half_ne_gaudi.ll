; RUN: llc -march=tpc -mcpu=gaudi -O2 -emit-index-factors=true %s -o - | FileCheck %s --check-prefix=CHECK-LLVM
; RUN: llc -march=tpc -mcpu=gaudi -O2 -emit-index-factors=true -index-space-mlir %s -o - | FileCheck %s --check-prefix=CHECK-MLIR

; CHECK-LLVM: "SCEVCost From TPC Index":
; CHECK-LLVM: .ascii " SCEVBEGIN IndexSpace:[0].[Input].{ 256, 4, 1, 1, 1 }[1].[Output].{ 256, 4, 1, 1, 1 } #SCEVEND"

; CHECK-MLIR: "SCEVCost From TPC Index":
; CHECK-MLIR: .ascii " SCEVBEGIN IndexSpace:[0].[Input].{ 256, 4, 1, 1, 1 }[1].[Output].{ 256, 4, 1, 1, 1 } #SCEVEND"

; ModuleID = './kernels/gaudi/elementwise/unary/cast_f32_to_i8_gemmlowp_half_ne_gaudi.c'
source_filename = "./kernels/gaudi/elementwise/unary/cast_f32_to_i8_gemmlowp_half_ne_gaudi.c"
target datalayout = "e-p0:32:32:32-p1:32:32:32-p2:32:32:32-p3:64:64:64-i32:32:32-n8:16:32-f16:16:16-f32:32:32-v160:32:32-v256:2048:2048-v2048:2048:2048-v4096:2048:2048-v8192:2048:2048"
target triple = "tpc"

; Function Attrs: nounwind writeonly
define dso_local void @main(float %0, float %1) local_unnamed_addr #0 {
  %3 = tail call <5 x i32> @llvm.tpc.get.index.space.offset()
  %4 = tail call <5 x i32> @llvm.tpc.get.index.space.size()
  %5 = add <5 x i32> %4, %3
  %6 = extractelement <5 x i32> %3, i32 0
  %7 = shl nsw i32 %6, 8
  %8 = extractelement <5 x i32> %5, i32 0
  %9 = shl nsw i32 %8, 8
  %10 = extractelement <5 x i32> %3, i32 1
  %11 = shl nsw i32 %10, 2
  %12 = extractelement <5 x i32> %5, i32 1
  %13 = shl nsw i32 %12, 2
  %14 = extractelement <5 x i32> %3, i32 2
  %15 = extractelement <5 x i32> %5, i32 2
  %16 = extractelement <5 x i32> %3, i32 3
  %17 = extractelement <5 x i32> %5, i32 3
  %18 = extractelement <5 x i32> %3, i32 4
  %19 = extractelement <5 x i32> %5, i32 4
  %20 = icmp slt i32 %7, %9
  br i1 %20, label %21, label %32

21:                                               ; preds = %2
  %22 = icmp slt i32 %18, %19
  %23 = icmp slt i32 %16, %17
  %24 = icmp slt i32 %14, %15
  %25 = insertelement <64 x float> undef, float %1, i32 0
  %26 = shufflevector <64 x float> %25, <64 x float> undef, <64 x i32> zeroinitializer
  %27 = insertelement <64 x float> undef, float %0, i32 0
  %28 = shufflevector <64 x float> %27, <64 x float> undef, <64 x i32> zeroinitializer
  %29 = icmp slt i32 %11, %13
  %30 = add nsw i32 %13, -4
  br label %33

31:                                               ; preds = %40
  br label %32

32:                                               ; preds = %31, %2
  ret void

33:                                               ; preds = %21, %40
  %34 = phi <5 x i32> [ zeroinitializer, %21 ], [ %41, %40 ]
  %35 = phi i32 [ %7, %21 ], [ %42, %40 ]
  %36 = insertelement <5 x i32> %34, i32 %35, i32 0
  br i1 %22, label %37, label %40

37:                                               ; preds = %33
  br label %44

38:                                               ; preds = %51
  %39 = phi <5 x i32> [ %52, %51 ]
  br label %40

40:                                               ; preds = %38, %33
  %41 = phi <5 x i32> [ %36, %33 ], [ %39, %38 ]
  %42 = add nsw i32 %35, 256
  %43 = icmp slt i32 %42, %9
  br i1 %43, label %33, label %31

44:                                               ; preds = %37, %51
  %45 = phi <5 x i32> [ %52, %51 ], [ %36, %37 ]
  %46 = phi i32 [ %53, %51 ], [ %18, %37 ]
  %47 = insertelement <5 x i32> %45, i32 %46, i32 4
  br i1 %23, label %48, label %51

48:                                               ; preds = %44
  br label %55

49:                                               ; preds = %62
  %50 = phi <5 x i32> [ %63, %62 ]
  br label %51

51:                                               ; preds = %49, %44
  %52 = phi <5 x i32> [ %47, %44 ], [ %50, %49 ]
  %53 = add i32 %46, 1
  %54 = icmp eq i32 %53, %19
  br i1 %54, label %38, label %44

55:                                               ; preds = %48, %62
  %56 = phi <5 x i32> [ %63, %62 ], [ %47, %48 ]
  %57 = phi i32 [ %64, %62 ], [ %16, %48 ]
  %58 = insertelement <5 x i32> %56, i32 %57, i32 3
  br i1 %24, label %59, label %62

59:                                               ; preds = %55
  br label %66

60:                                               ; preds = %87
  %61 = phi <5 x i32> [ %88, %87 ]
  br label %62

62:                                               ; preds = %60, %55
  %63 = phi <5 x i32> [ %58, %55 ], [ %61, %60 ]
  %64 = add i32 %57, 1
  %65 = icmp eq i32 %64, %17
  br i1 %65, label %49, label %55

66:                                               ; preds = %59, %87
  %67 = phi <5 x i32> [ %88, %87 ], [ %58, %59 ]
  %68 = phi i32 [ %89, %87 ], [ %14, %59 ]
  %69 = insertelement <5 x i32> %67, i32 %68, i32 2
  %70 = insertelement <5 x i32> %69, i32 %11, i32 1
  %71 = tail call <5 x i32> @llvm.tpc.add.mask.v5i32.v5i32(<5 x i32> <i32 64, i32 64, i32 64, i32 64, i32 64>, <5 x i32> %70, i32 1, i8 2, i32 0, <5 x i32> %70, i1 true, i1 false)
  %72 = tail call <5 x i32> @llvm.tpc.add.mask.v5i32.v5i32(<5 x i32> <i32 128, i32 128, i32 128, i32 128, i32 128>, <5 x i32> %70, i32 1, i8 2, i32 0, <5 x i32> %70, i1 true, i1 false)
  %73 = tail call <5 x i32> @llvm.tpc.add.mask.v5i32.v5i32(<5 x i32> <i32 192, i32 192, i32 192, i32 192, i32 192>, <5 x i32> %70, i32 1, i8 2, i32 0, <5 x i32> %70, i1 true, i1 false)
  %74 = tail call <64 x float> @llvm.tpc.ld.tnsr.v64f32.i1(<5 x i32> %70, i8 0, i32 0, <64 x float> undef, i1 true, i1 false)
  %75 = tail call <5 x i32> @llvm.tpc.add.mask.v5i32.i32(<5 x i32> %70, i32 1, i32 2, i8 2, i32 0, <5 x i32> %70, i1 true, i1 false)
  %76 = tail call <64 x float> @llvm.tpc.ld.tnsr.v64f32.i1(<5 x i32> %75, i8 0, i32 0, <64 x float> undef, i1 true, i1 false)
  %77 = tail call <5 x i32> @llvm.tpc.add.mask.v5i32.i32(<5 x i32> %75, i32 1, i32 2, i8 2, i32 0, <5 x i32> %75, i1 true, i1 false)
  %78 = tail call <64 x float> @llvm.tpc.ld.tnsr.v64f32.i1(<5 x i32> %77, i8 0, i32 0, <64 x float> undef, i1 true, i1 false)
  %79 = tail call <5 x i32> @llvm.tpc.add.mask.v5i32.i32(<5 x i32> %77, i32 1, i32 2, i8 2, i32 0, <5 x i32> %77, i1 true, i1 false)
  %80 = tail call <64 x float> @llvm.tpc.ld.tnsr.v64f32.i1(<5 x i32> %79, i8 0, i32 0, <64 x float> undef, i1 true, i1 false)
  %81 = tail call <5 x i32> @llvm.tpc.add.mask.v5i32.i32(<5 x i32> %79, i32 1, i32 2, i8 2, i32 0, <5 x i32> %79, i1 true, i1 false)
  %82 = tail call <64 x float> @llvm.tpc.mac.v64f32.v64f32.i1(<64 x float> %74, <64 x float> %28, i8 0, i32 0, <64 x float> %26, i1 true, i1 false)
  %83 = tail call <64 x float> @llvm.tpc.mac.v64f32.v64f32.i1(<64 x float> %76, <64 x float> %28, i8 0, i32 0, <64 x float> %26, i1 true, i1 false)
  br i1 %29, label %84, label %87

84:                                               ; preds = %66
  br label %91

85:                                               ; preds = %91
  %86 = phi <5 x i32> [ %276, %91 ]
  br label %87

87:                                               ; preds = %85, %66
  %88 = phi <5 x i32> [ %81, %66 ], [ %86, %85 ]
  %89 = add i32 %68, 1
  %90 = icmp eq i32 %89, %15
  br i1 %90, label %60, label %66

91:                                               ; preds = %84, %91
  %92 = phi <5 x i32> [ %276, %91 ], [ %81, %84 ]
  %93 = phi i32 [ %279, %91 ], [ %11, %84 ]
  %94 = phi <5 x i32> [ %111, %91 ], [ %71, %84 ]
  %95 = phi <5 x i32> [ %123, %91 ], [ %72, %84 ]
  %96 = phi <5 x i32> [ %135, %91 ], [ %73, %84 ]
  %97 = phi <5 x i32> [ %267, %91 ], [ %70, %84 ]
  %98 = phi <64 x float> [ %273, %91 ], [ %78, %84 ]
  %99 = phi <64 x float> [ %275, %91 ], [ %80, %84 ]
  %100 = phi <64 x float> [ %277, %91 ], [ %82, %84 ]
  %101 = phi <64 x float> [ %278, %91 ], [ %83, %84 ]
  %102 = tail call <64 x float> @llvm.tpc.mac.v64f32.v64f32.i1(<64 x float> %98, <64 x float> %28, i8 0, i32 0, <64 x float> %26, i1 true, i1 false)
  %103 = tail call <64 x float> @llvm.tpc.mac.v64f32.v64f32.i1(<64 x float> %99, <64 x float> %28, i8 0, i32 0, <64 x float> %26, i1 true, i1 false)
  %104 = tail call <64 x float> @llvm.tpc.ld.tnsr.v64f32.i1(<5 x i32> %94, i8 0, i32 0, <64 x float> undef, i1 true, i1 false)
  %105 = tail call <5 x i32> @llvm.tpc.add.mask.v5i32.i32(<5 x i32> %94, i32 1, i32 2, i8 2, i32 0, <5 x i32> %94, i1 true, i1 false)
  %106 = tail call <64 x float> @llvm.tpc.ld.tnsr.v64f32.i1(<5 x i32> %105, i8 0, i32 0, <64 x float> undef, i1 true, i1 false)
  %107 = tail call <5 x i32> @llvm.tpc.add.mask.v5i32.i32(<5 x i32> %105, i32 1, i32 2, i8 2, i32 0, <5 x i32> %105, i1 true, i1 false)
  %108 = tail call <64 x float> @llvm.tpc.ld.tnsr.v64f32.i1(<5 x i32> %107, i8 0, i32 0, <64 x float> undef, i1 true, i1 false)
  %109 = tail call <5 x i32> @llvm.tpc.add.mask.v5i32.i32(<5 x i32> %107, i32 1, i32 2, i8 2, i32 0, <5 x i32> %107, i1 true, i1 false)
  %110 = tail call <64 x float> @llvm.tpc.ld.tnsr.v64f32.i1(<5 x i32> %109, i8 0, i32 0, <64 x float> undef, i1 true, i1 false)
  %111 = tail call <5 x i32> @llvm.tpc.add.mask.v5i32.i32(<5 x i32> %109, i32 1, i32 2, i8 2, i32 0, <5 x i32> %109, i1 true, i1 false)
  %112 = tail call <64 x float> @llvm.tpc.mac.v64f32.v64f32.i1(<64 x float> %104, <64 x float> %28, i8 0, i32 0, <64 x float> %26, i1 true, i1 false)
  %113 = tail call <64 x float> @llvm.tpc.mac.v64f32.v64f32.i1(<64 x float> %106, <64 x float> %28, i8 0, i32 0, <64 x float> %26, i1 true, i1 false)
  %114 = tail call <64 x float> @llvm.tpc.mac.v64f32.v64f32.i1(<64 x float> %108, <64 x float> %28, i8 0, i32 0, <64 x float> %26, i1 true, i1 false)
  %115 = tail call <64 x float> @llvm.tpc.mac.v64f32.v64f32.i1(<64 x float> %110, <64 x float> %28, i8 0, i32 0, <64 x float> %26, i1 true, i1 false)
  %116 = tail call <64 x float> @llvm.tpc.ld.tnsr.v64f32.i1(<5 x i32> %95, i8 0, i32 0, <64 x float> undef, i1 true, i1 false)
  %117 = tail call <5 x i32> @llvm.tpc.add.mask.v5i32.i32(<5 x i32> %95, i32 1, i32 2, i8 2, i32 0, <5 x i32> %95, i1 true, i1 false)
  %118 = tail call <64 x float> @llvm.tpc.ld.tnsr.v64f32.i1(<5 x i32> %117, i8 0, i32 0, <64 x float> undef, i1 true, i1 false)
  %119 = tail call <5 x i32> @llvm.tpc.add.mask.v5i32.i32(<5 x i32> %117, i32 1, i32 2, i8 2, i32 0, <5 x i32> %117, i1 true, i1 false)
  %120 = tail call <64 x float> @llvm.tpc.ld.tnsr.v64f32.i1(<5 x i32> %119, i8 0, i32 0, <64 x float> undef, i1 true, i1 false)
  %121 = tail call <5 x i32> @llvm.tpc.add.mask.v5i32.i32(<5 x i32> %119, i32 1, i32 2, i8 2, i32 0, <5 x i32> %119, i1 true, i1 false)
  %122 = tail call <64 x float> @llvm.tpc.ld.tnsr.v64f32.i1(<5 x i32> %121, i8 0, i32 0, <64 x float> undef, i1 true, i1 false)
  %123 = tail call <5 x i32> @llvm.tpc.add.mask.v5i32.i32(<5 x i32> %121, i32 1, i32 2, i8 2, i32 0, <5 x i32> %121, i1 true, i1 false)
  %124 = tail call <64 x float> @llvm.tpc.mac.v64f32.v64f32.i1(<64 x float> %116, <64 x float> %28, i8 0, i32 0, <64 x float> %26, i1 true, i1 false)
  %125 = tail call <64 x float> @llvm.tpc.mac.v64f32.v64f32.i1(<64 x float> %118, <64 x float> %28, i8 0, i32 0, <64 x float> %26, i1 true, i1 false)
  %126 = tail call <64 x float> @llvm.tpc.mac.v64f32.v64f32.i1(<64 x float> %120, <64 x float> %28, i8 0, i32 0, <64 x float> %26, i1 true, i1 false)
  %127 = tail call <64 x float> @llvm.tpc.mac.v64f32.v64f32.i1(<64 x float> %122, <64 x float> %28, i8 0, i32 0, <64 x float> %26, i1 true, i1 false)
  %128 = tail call <64 x float> @llvm.tpc.ld.tnsr.v64f32.i1(<5 x i32> %96, i8 0, i32 0, <64 x float> undef, i1 true, i1 false)
  %129 = tail call <5 x i32> @llvm.tpc.add.mask.v5i32.i32(<5 x i32> %96, i32 1, i32 2, i8 2, i32 0, <5 x i32> %96, i1 true, i1 false)
  %130 = tail call <64 x float> @llvm.tpc.ld.tnsr.v64f32.i1(<5 x i32> %129, i8 0, i32 0, <64 x float> undef, i1 true, i1 false)
  %131 = tail call <5 x i32> @llvm.tpc.add.mask.v5i32.i32(<5 x i32> %129, i32 1, i32 2, i8 2, i32 0, <5 x i32> %129, i1 true, i1 false)
  %132 = tail call <64 x float> @llvm.tpc.ld.tnsr.v64f32.i1(<5 x i32> %131, i8 0, i32 0, <64 x float> undef, i1 true, i1 false)
  %133 = tail call <5 x i32> @llvm.tpc.add.mask.v5i32.i32(<5 x i32> %131, i32 1, i32 2, i8 2, i32 0, <5 x i32> %131, i1 true, i1 false)
  %134 = tail call <64 x float> @llvm.tpc.ld.tnsr.v64f32.i1(<5 x i32> %133, i8 0, i32 0, <64 x float> undef, i1 true, i1 false)
  %135 = tail call <5 x i32> @llvm.tpc.add.mask.v5i32.i32(<5 x i32> %133, i32 1, i32 2, i8 2, i32 0, <5 x i32> %133, i1 true, i1 false)
  %136 = tail call <64 x float> @llvm.tpc.mac.v64f32.v64f32.i1(<64 x float> %128, <64 x float> %28, i8 0, i32 0, <64 x float> %26, i1 true, i1 false)
  %137 = tail call <64 x float> @llvm.tpc.mac.v64f32.v64f32.i1(<64 x float> %130, <64 x float> %28, i8 0, i32 0, <64 x float> %26, i1 true, i1 false)
  %138 = tail call <64 x float> @llvm.tpc.mac.v64f32.v64f32.i1(<64 x float> %132, <64 x float> %28, i8 0, i32 0, <64 x float> %26, i1 true, i1 false)
  %139 = tail call <64 x float> @llvm.tpc.mac.v64f32.v64f32.i1(<64 x float> %134, <64 x float> %28, i8 0, i32 0, <64 x float> %26, i1 true, i1 false)
  %140 = tail call <64 x i32> @llvm.tpc.convert.v64i32.v64f32.i1(<64 x float> %100, i8 0, i32 512, <64 x i32> undef, i1 true, i1 false) #3
  %141 = tail call <256 x i8> @llvm.tpc.convert.int.v256i8.v64i32.v256i8.i1(<64 x i32> %140, <256 x i8> zeroinitializer, i32 0, <256 x i8> zeroinitializer, i1 true, i1 false) #3
  %142 = tail call <64 x i32> @llvm.tpc.convert.v64i32.v64f32.i1(<64 x float> %112, i8 0, i32 512, <64 x i32> undef, i1 true, i1 false) #3
  %143 = tail call <256 x i8> @llvm.tpc.convert.int.v256i8.v64i32.v256i8.i1(<64 x i32> %142, <256 x i8> zeroinitializer, i32 0, <256 x i8> zeroinitializer, i1 true, i1 false) #3
  %144 = tail call <64 x i32> @llvm.tpc.convert.v64i32.v64f32.i1(<64 x float> %124, i8 0, i32 512, <64 x i32> undef, i1 true, i1 false) #3
  %145 = tail call <256 x i8> @llvm.tpc.convert.int.v256i8.v64i32.v256i8.i1(<64 x i32> %144, <256 x i8> zeroinitializer, i32 0, <256 x i8> zeroinitializer, i1 true, i1 false) #3
  %146 = tail call <64 x i32> @llvm.tpc.convert.v64i32.v64f32.i1(<64 x float> %136, i8 0, i32 512, <64 x i32> undef, i1 true, i1 false) #3
  %147 = tail call <256 x i8> @llvm.tpc.convert.int.v256i8.v64i32.v256i8.i1(<64 x i32> %146, <256 x i8> zeroinitializer, i32 0, <256 x i8> zeroinitializer, i1 true, i1 false) #3
  %148 = tail call <256 x i8> @llvm.tpc.pack.v256i8.i1(<256 x i8> %141, i8 4, i32 512, <256 x i8> zeroinitializer, i1 true, i1 false) #3
  %149 = tail call <256 x i8> @llvm.tpc.pack.v256i8.i1(<256 x i8> %141, i8 4, i32 768, <256 x i8> %148, i1 true, i1 false) #3
  %150 = tail call <256 x i8> @llvm.tpc.mov.dual.group.v256i8.i1(<256 x i8> %149, i32 -65536, i32 0, i32 4352, <256 x i8> %149, i1 true, i1 false) #3
  %151 = tail call <256 x i8> @llvm.tpc.mov.dual.group.v256i8.i1(<256 x i8> %150, i32 65535, i32 0, i32 8704, <256 x i8> %150, i1 true, i1 false) #3
  %152 = tail call <256 x i8> @llvm.tpc.mov.dual.group.v256i8.i1(<256 x i8> %151, i32 -65536, i32 0, i32 8960, <256 x i8> %151, i1 true, i1 false) #3
  %153 = tail call <256 x i8> @llvm.tpc.pack.v256i8.i1(<256 x i8> %143, i8 4, i32 512, <256 x i8> zeroinitializer, i1 true, i1 false) #3
  %154 = tail call <256 x i8> @llvm.tpc.pack.v256i8.i1(<256 x i8> %143, i8 4, i32 768, <256 x i8> %153, i1 true, i1 false) #3
  %155 = tail call <256 x i8> @llvm.tpc.mov.dual.group.v256i8.i1(<256 x i8> %154, i32 65535, i32 0, i32 5120, <256 x i8> %152, i1 true, i1 false) #3
  %156 = tail call <256 x i8> @llvm.tpc.mov.dual.group.v256i8.i1(<256 x i8> %154, i32 -65536, i32 0, i32 5376, <256 x i8> %155, i1 true, i1 false) #3
  %157 = tail call <256 x i8> @llvm.tpc.mov.dual.group.v256i8.i1(<256 x i8> %154, i32 65535, i32 0, i32 9728, <256 x i8> %156, i1 true, i1 false) #3
  %158 = tail call <256 x i8> @llvm.tpc.mov.dual.group.v256i8.i1(<256 x i8> %154, i32 -65536, i32 0, i32 9984, <256 x i8> %157, i1 true, i1 false) #3
  %159 = tail call <256 x i8> @llvm.tpc.pack.v256i8.i1(<256 x i8> %145, i8 4, i32 512, <256 x i8> %154, i1 true, i1 false) #3
  %160 = tail call <256 x i8> @llvm.tpc.pack.v256i8.i1(<256 x i8> %145, i8 4, i32 768, <256 x i8> %159, i1 true, i1 false) #3
  %161 = tail call <256 x i8> @llvm.tpc.mov.dual.group.v256i8.i1(<256 x i8> %160, i32 65535, i32 0, i32 6144, <256 x i8> %158, i1 true, i1 false) #3
  %162 = tail call <256 x i8> @llvm.tpc.mov.dual.group.v256i8.i1(<256 x i8> %160, i32 -65536, i32 0, i32 6400, <256 x i8> %161, i1 true, i1 false) #3
  %163 = tail call <256 x i8> @llvm.tpc.mov.dual.group.v256i8.i1(<256 x i8> %160, i32 65535, i32 0, i32 10752, <256 x i8> %162, i1 true, i1 false) #3
  %164 = tail call <256 x i8> @llvm.tpc.mov.dual.group.v256i8.i1(<256 x i8> %160, i32 -65536, i32 0, i32 11008, <256 x i8> %163, i1 true, i1 false) #3
  %165 = tail call <256 x i8> @llvm.tpc.pack.v256i8.i1(<256 x i8> %147, i8 4, i32 512, <256 x i8> %160, i1 true, i1 false) #3
  %166 = tail call <256 x i8> @llvm.tpc.pack.v256i8.i1(<256 x i8> %147, i8 4, i32 768, <256 x i8> %165, i1 true, i1 false) #3
  %167 = tail call <256 x i8> @llvm.tpc.mov.dual.group.v256i8.i1(<256 x i8> %166, i32 65535, i32 0, i32 7168, <256 x i8> %164, i1 true, i1 false) #3
  %168 = tail call <256 x i8> @llvm.tpc.mov.dual.group.v256i8.i1(<256 x i8> %166, i32 -65536, i32 0, i32 7424, <256 x i8> %167, i1 true, i1 false) #3
  %169 = tail call <256 x i8> @llvm.tpc.mov.dual.group.v256i8.i1(<256 x i8> %166, i32 65535, i32 0, i32 11776, <256 x i8> %168, i1 true, i1 false) #3
  %170 = tail call <256 x i8> @llvm.tpc.mov.dual.group.v256i8.i1(<256 x i8> %166, i32 -65536, i32 0, i32 12032, <256 x i8> %169, i1 true, i1 false) #3
  %171 = tail call <64 x i32> @llvm.tpc.convert.v64i32.v64f32.i1(<64 x float> %101, i8 0, i32 512, <64 x i32> undef, i1 true, i1 false) #3
  %172 = tail call <256 x i8> @llvm.tpc.convert.int.v256i8.v64i32.v256i8.i1(<64 x i32> %171, <256 x i8> zeroinitializer, i32 0, <256 x i8> zeroinitializer, i1 true, i1 false) #3
  %173 = tail call <64 x i32> @llvm.tpc.convert.v64i32.v64f32.i1(<64 x float> %113, i8 0, i32 512, <64 x i32> undef, i1 true, i1 false) #3
  %174 = tail call <256 x i8> @llvm.tpc.convert.int.v256i8.v64i32.v256i8.i1(<64 x i32> %173, <256 x i8> zeroinitializer, i32 0, <256 x i8> zeroinitializer, i1 true, i1 false) #3
  %175 = tail call <64 x i32> @llvm.tpc.convert.v64i32.v64f32.i1(<64 x float> %125, i8 0, i32 512, <64 x i32> undef, i1 true, i1 false) #3
  %176 = tail call <256 x i8> @llvm.tpc.convert.int.v256i8.v64i32.v256i8.i1(<64 x i32> %175, <256 x i8> zeroinitializer, i32 0, <256 x i8> zeroinitializer, i1 true, i1 false) #3
  %177 = tail call <64 x i32> @llvm.tpc.convert.v64i32.v64f32.i1(<64 x float> %137, i8 0, i32 512, <64 x i32> undef, i1 true, i1 false) #3
  %178 = tail call <256 x i8> @llvm.tpc.convert.int.v256i8.v64i32.v256i8.i1(<64 x i32> %177, <256 x i8> zeroinitializer, i32 0, <256 x i8> zeroinitializer, i1 true, i1 false) #3
  %179 = tail call <256 x i8> @llvm.tpc.pack.v256i8.i1(<256 x i8> %172, i8 4, i32 512, <256 x i8> zeroinitializer, i1 true, i1 false) #3
  %180 = tail call <256 x i8> @llvm.tpc.pack.v256i8.i1(<256 x i8> %172, i8 4, i32 768, <256 x i8> %179, i1 true, i1 false) #3
  %181 = tail call <256 x i8> @llvm.tpc.mov.dual.group.v256i8.i1(<256 x i8> %180, i32 -65536, i32 0, i32 4352, <256 x i8> %180, i1 true, i1 false) #3
  %182 = tail call <256 x i8> @llvm.tpc.mov.dual.group.v256i8.i1(<256 x i8> %181, i32 65535, i32 0, i32 8704, <256 x i8> %181, i1 true, i1 false) #3
  %183 = tail call <256 x i8> @llvm.tpc.mov.dual.group.v256i8.i1(<256 x i8> %182, i32 -65536, i32 0, i32 8960, <256 x i8> %182, i1 true, i1 false) #3
  %184 = tail call <256 x i8> @llvm.tpc.pack.v256i8.i1(<256 x i8> %174, i8 4, i32 512, <256 x i8> zeroinitializer, i1 true, i1 false) #3
  %185 = tail call <256 x i8> @llvm.tpc.pack.v256i8.i1(<256 x i8> %174, i8 4, i32 768, <256 x i8> %184, i1 true, i1 false) #3
  %186 = tail call <256 x i8> @llvm.tpc.mov.dual.group.v256i8.i1(<256 x i8> %185, i32 65535, i32 0, i32 5120, <256 x i8> %183, i1 true, i1 false) #3
  %187 = tail call <256 x i8> @llvm.tpc.mov.dual.group.v256i8.i1(<256 x i8> %185, i32 -65536, i32 0, i32 5376, <256 x i8> %186, i1 true, i1 false) #3
  %188 = tail call <256 x i8> @llvm.tpc.mov.dual.group.v256i8.i1(<256 x i8> %185, i32 65535, i32 0, i32 9728, <256 x i8> %187, i1 true, i1 false) #3
  %189 = tail call <256 x i8> @llvm.tpc.mov.dual.group.v256i8.i1(<256 x i8> %185, i32 -65536, i32 0, i32 9984, <256 x i8> %188, i1 true, i1 false) #3
  %190 = tail call <256 x i8> @llvm.tpc.pack.v256i8.i1(<256 x i8> %176, i8 4, i32 512, <256 x i8> %185, i1 true, i1 false) #3
  %191 = tail call <256 x i8> @llvm.tpc.pack.v256i8.i1(<256 x i8> %176, i8 4, i32 768, <256 x i8> %190, i1 true, i1 false) #3
  %192 = tail call <256 x i8> @llvm.tpc.mov.dual.group.v256i8.i1(<256 x i8> %191, i32 65535, i32 0, i32 6144, <256 x i8> %189, i1 true, i1 false) #3
  %193 = tail call <256 x i8> @llvm.tpc.mov.dual.group.v256i8.i1(<256 x i8> %191, i32 -65536, i32 0, i32 6400, <256 x i8> %192, i1 true, i1 false) #3
  %194 = tail call <256 x i8> @llvm.tpc.mov.dual.group.v256i8.i1(<256 x i8> %191, i32 65535, i32 0, i32 10752, <256 x i8> %193, i1 true, i1 false) #3
  %195 = tail call <256 x i8> @llvm.tpc.mov.dual.group.v256i8.i1(<256 x i8> %191, i32 -65536, i32 0, i32 11008, <256 x i8> %194, i1 true, i1 false) #3
  %196 = tail call <256 x i8> @llvm.tpc.pack.v256i8.i1(<256 x i8> %178, i8 4, i32 512, <256 x i8> %191, i1 true, i1 false) #3
  %197 = tail call <256 x i8> @llvm.tpc.pack.v256i8.i1(<256 x i8> %178, i8 4, i32 768, <256 x i8> %196, i1 true, i1 false) #3
  %198 = tail call <256 x i8> @llvm.tpc.mov.dual.group.v256i8.i1(<256 x i8> %197, i32 65535, i32 0, i32 7168, <256 x i8> %195, i1 true, i1 false) #3
  %199 = tail call <256 x i8> @llvm.tpc.mov.dual.group.v256i8.i1(<256 x i8> %197, i32 -65536, i32 0, i32 7424, <256 x i8> %198, i1 true, i1 false) #3
  %200 = tail call <256 x i8> @llvm.tpc.mov.dual.group.v256i8.i1(<256 x i8> %197, i32 65535, i32 0, i32 11776, <256 x i8> %199, i1 true, i1 false) #3
  %201 = tail call <256 x i8> @llvm.tpc.mov.dual.group.v256i8.i1(<256 x i8> %197, i32 -65536, i32 0, i32 12032, <256 x i8> %200, i1 true, i1 false) #3
  %202 = tail call <64 x i32> @llvm.tpc.convert.v64i32.v64f32.i1(<64 x float> %102, i8 0, i32 512, <64 x i32> undef, i1 true, i1 false) #3
  %203 = tail call <256 x i8> @llvm.tpc.convert.int.v256i8.v64i32.v256i8.i1(<64 x i32> %202, <256 x i8> zeroinitializer, i32 0, <256 x i8> zeroinitializer, i1 true, i1 false) #3
  %204 = tail call <64 x i32> @llvm.tpc.convert.v64i32.v64f32.i1(<64 x float> %114, i8 0, i32 512, <64 x i32> undef, i1 true, i1 false) #3
  %205 = tail call <256 x i8> @llvm.tpc.convert.int.v256i8.v64i32.v256i8.i1(<64 x i32> %204, <256 x i8> zeroinitializer, i32 0, <256 x i8> zeroinitializer, i1 true, i1 false) #3
  %206 = tail call <64 x i32> @llvm.tpc.convert.v64i32.v64f32.i1(<64 x float> %126, i8 0, i32 512, <64 x i32> undef, i1 true, i1 false) #3
  %207 = tail call <256 x i8> @llvm.tpc.convert.int.v256i8.v64i32.v256i8.i1(<64 x i32> %206, <256 x i8> zeroinitializer, i32 0, <256 x i8> zeroinitializer, i1 true, i1 false) #3
  %208 = tail call <64 x i32> @llvm.tpc.convert.v64i32.v64f32.i1(<64 x float> %138, i8 0, i32 512, <64 x i32> undef, i1 true, i1 false) #3
  %209 = tail call <256 x i8> @llvm.tpc.convert.int.v256i8.v64i32.v256i8.i1(<64 x i32> %208, <256 x i8> zeroinitializer, i32 0, <256 x i8> zeroinitializer, i1 true, i1 false) #3
  %210 = tail call <256 x i8> @llvm.tpc.pack.v256i8.i1(<256 x i8> %203, i8 4, i32 512, <256 x i8> zeroinitializer, i1 true, i1 false) #3
  %211 = tail call <256 x i8> @llvm.tpc.pack.v256i8.i1(<256 x i8> %203, i8 4, i32 768, <256 x i8> %210, i1 true, i1 false) #3
  %212 = tail call <256 x i8> @llvm.tpc.mov.dual.group.v256i8.i1(<256 x i8> %211, i32 -65536, i32 0, i32 4352, <256 x i8> %211, i1 true, i1 false) #3
  %213 = tail call <256 x i8> @llvm.tpc.mov.dual.group.v256i8.i1(<256 x i8> %212, i32 65535, i32 0, i32 8704, <256 x i8> %212, i1 true, i1 false) #3
  %214 = tail call <256 x i8> @llvm.tpc.mov.dual.group.v256i8.i1(<256 x i8> %213, i32 -65536, i32 0, i32 8960, <256 x i8> %213, i1 true, i1 false) #3
  %215 = tail call <256 x i8> @llvm.tpc.pack.v256i8.i1(<256 x i8> %205, i8 4, i32 512, <256 x i8> zeroinitializer, i1 true, i1 false) #3
  %216 = tail call <256 x i8> @llvm.tpc.pack.v256i8.i1(<256 x i8> %205, i8 4, i32 768, <256 x i8> %215, i1 true, i1 false) #3
  %217 = tail call <256 x i8> @llvm.tpc.mov.dual.group.v256i8.i1(<256 x i8> %216, i32 65535, i32 0, i32 5120, <256 x i8> %214, i1 true, i1 false) #3
  %218 = tail call <256 x i8> @llvm.tpc.mov.dual.group.v256i8.i1(<256 x i8> %216, i32 -65536, i32 0, i32 5376, <256 x i8> %217, i1 true, i1 false) #3
  %219 = tail call <256 x i8> @llvm.tpc.mov.dual.group.v256i8.i1(<256 x i8> %216, i32 65535, i32 0, i32 9728, <256 x i8> %218, i1 true, i1 false) #3
  %220 = tail call <256 x i8> @llvm.tpc.mov.dual.group.v256i8.i1(<256 x i8> %216, i32 -65536, i32 0, i32 9984, <256 x i8> %219, i1 true, i1 false) #3
  %221 = tail call <256 x i8> @llvm.tpc.pack.v256i8.i1(<256 x i8> %207, i8 4, i32 512, <256 x i8> %216, i1 true, i1 false) #3
  %222 = tail call <256 x i8> @llvm.tpc.pack.v256i8.i1(<256 x i8> %207, i8 4, i32 768, <256 x i8> %221, i1 true, i1 false) #3
  %223 = tail call <256 x i8> @llvm.tpc.mov.dual.group.v256i8.i1(<256 x i8> %222, i32 65535, i32 0, i32 6144, <256 x i8> %220, i1 true, i1 false) #3
  %224 = tail call <256 x i8> @llvm.tpc.mov.dual.group.v256i8.i1(<256 x i8> %222, i32 -65536, i32 0, i32 6400, <256 x i8> %223, i1 true, i1 false) #3
  %225 = tail call <256 x i8> @llvm.tpc.mov.dual.group.v256i8.i1(<256 x i8> %222, i32 65535, i32 0, i32 10752, <256 x i8> %224, i1 true, i1 false) #3
  %226 = tail call <256 x i8> @llvm.tpc.mov.dual.group.v256i8.i1(<256 x i8> %222, i32 -65536, i32 0, i32 11008, <256 x i8> %225, i1 true, i1 false) #3
  %227 = tail call <256 x i8> @llvm.tpc.pack.v256i8.i1(<256 x i8> %209, i8 4, i32 512, <256 x i8> %222, i1 true, i1 false) #3
  %228 = tail call <256 x i8> @llvm.tpc.pack.v256i8.i1(<256 x i8> %209, i8 4, i32 768, <256 x i8> %227, i1 true, i1 false) #3
  %229 = tail call <256 x i8> @llvm.tpc.mov.dual.group.v256i8.i1(<256 x i8> %228, i32 65535, i32 0, i32 7168, <256 x i8> %226, i1 true, i1 false) #3
  %230 = tail call <256 x i8> @llvm.tpc.mov.dual.group.v256i8.i1(<256 x i8> %228, i32 -65536, i32 0, i32 7424, <256 x i8> %229, i1 true, i1 false) #3
  %231 = tail call <256 x i8> @llvm.tpc.mov.dual.group.v256i8.i1(<256 x i8> %228, i32 65535, i32 0, i32 11776, <256 x i8> %230, i1 true, i1 false) #3
  %232 = tail call <256 x i8> @llvm.tpc.mov.dual.group.v256i8.i1(<256 x i8> %228, i32 -65536, i32 0, i32 12032, <256 x i8> %231, i1 true, i1 false) #3
  %233 = tail call <64 x i32> @llvm.tpc.convert.v64i32.v64f32.i1(<64 x float> %103, i8 0, i32 512, <64 x i32> undef, i1 true, i1 false) #3
  %234 = tail call <256 x i8> @llvm.tpc.convert.int.v256i8.v64i32.v256i8.i1(<64 x i32> %233, <256 x i8> zeroinitializer, i32 0, <256 x i8> zeroinitializer, i1 true, i1 false) #3
  %235 = tail call <64 x i32> @llvm.tpc.convert.v64i32.v64f32.i1(<64 x float> %115, i8 0, i32 512, <64 x i32> undef, i1 true, i1 false) #3
  %236 = tail call <256 x i8> @llvm.tpc.convert.int.v256i8.v64i32.v256i8.i1(<64 x i32> %235, <256 x i8> zeroinitializer, i32 0, <256 x i8> zeroinitializer, i1 true, i1 false) #3
  %237 = tail call <64 x i32> @llvm.tpc.convert.v64i32.v64f32.i1(<64 x float> %127, i8 0, i32 512, <64 x i32> undef, i1 true, i1 false) #3
  %238 = tail call <256 x i8> @llvm.tpc.convert.int.v256i8.v64i32.v256i8.i1(<64 x i32> %237, <256 x i8> zeroinitializer, i32 0, <256 x i8> zeroinitializer, i1 true, i1 false) #3
  %239 = tail call <64 x i32> @llvm.tpc.convert.v64i32.v64f32.i1(<64 x float> %139, i8 0, i32 512, <64 x i32> undef, i1 true, i1 false) #3
  %240 = tail call <256 x i8> @llvm.tpc.convert.int.v256i8.v64i32.v256i8.i1(<64 x i32> %239, <256 x i8> zeroinitializer, i32 0, <256 x i8> zeroinitializer, i1 true, i1 false) #3
  %241 = tail call <256 x i8> @llvm.tpc.pack.v256i8.i1(<256 x i8> %234, i8 4, i32 512, <256 x i8> zeroinitializer, i1 true, i1 false) #3
  %242 = tail call <256 x i8> @llvm.tpc.pack.v256i8.i1(<256 x i8> %234, i8 4, i32 768, <256 x i8> %241, i1 true, i1 false) #3
  %243 = tail call <256 x i8> @llvm.tpc.mov.dual.group.v256i8.i1(<256 x i8> %242, i32 -65536, i32 0, i32 4352, <256 x i8> %242, i1 true, i1 false) #3
  %244 = tail call <256 x i8> @llvm.tpc.mov.dual.group.v256i8.i1(<256 x i8> %243, i32 65535, i32 0, i32 8704, <256 x i8> %243, i1 true, i1 false) #3
  %245 = tail call <256 x i8> @llvm.tpc.mov.dual.group.v256i8.i1(<256 x i8> %244, i32 -65536, i32 0, i32 8960, <256 x i8> %244, i1 true, i1 false) #3
  %246 = tail call <256 x i8> @llvm.tpc.pack.v256i8.i1(<256 x i8> %236, i8 4, i32 512, <256 x i8> zeroinitializer, i1 true, i1 false) #3
  %247 = tail call <256 x i8> @llvm.tpc.pack.v256i8.i1(<256 x i8> %236, i8 4, i32 768, <256 x i8> %246, i1 true, i1 false) #3
  %248 = tail call <256 x i8> @llvm.tpc.mov.dual.group.v256i8.i1(<256 x i8> %247, i32 65535, i32 0, i32 5120, <256 x i8> %245, i1 true, i1 false) #3
  %249 = tail call <256 x i8> @llvm.tpc.mov.dual.group.v256i8.i1(<256 x i8> %247, i32 -65536, i32 0, i32 5376, <256 x i8> %248, i1 true, i1 false) #3
  %250 = tail call <256 x i8> @llvm.tpc.mov.dual.group.v256i8.i1(<256 x i8> %247, i32 65535, i32 0, i32 9728, <256 x i8> %249, i1 true, i1 false) #3
  %251 = tail call <256 x i8> @llvm.tpc.mov.dual.group.v256i8.i1(<256 x i8> %247, i32 -65536, i32 0, i32 9984, <256 x i8> %250, i1 true, i1 false) #3
  %252 = tail call <256 x i8> @llvm.tpc.pack.v256i8.i1(<256 x i8> %238, i8 4, i32 512, <256 x i8> %247, i1 true, i1 false) #3
  %253 = tail call <256 x i8> @llvm.tpc.pack.v256i8.i1(<256 x i8> %238, i8 4, i32 768, <256 x i8> %252, i1 true, i1 false) #3
  %254 = tail call <256 x i8> @llvm.tpc.mov.dual.group.v256i8.i1(<256 x i8> %253, i32 65535, i32 0, i32 6144, <256 x i8> %251, i1 true, i1 false) #3
  %255 = tail call <256 x i8> @llvm.tpc.mov.dual.group.v256i8.i1(<256 x i8> %253, i32 -65536, i32 0, i32 6400, <256 x i8> %254, i1 true, i1 false) #3
  %256 = tail call <256 x i8> @llvm.tpc.mov.dual.group.v256i8.i1(<256 x i8> %253, i32 65535, i32 0, i32 10752, <256 x i8> %255, i1 true, i1 false) #3
  %257 = tail call <256 x i8> @llvm.tpc.mov.dual.group.v256i8.i1(<256 x i8> %253, i32 -65536, i32 0, i32 11008, <256 x i8> %256, i1 true, i1 false) #3
  %258 = tail call <256 x i8> @llvm.tpc.pack.v256i8.i1(<256 x i8> %240, i8 4, i32 512, <256 x i8> %253, i1 true, i1 false) #3
  %259 = tail call <256 x i8> @llvm.tpc.pack.v256i8.i1(<256 x i8> %240, i8 4, i32 768, <256 x i8> %258, i1 true, i1 false) #3
  %260 = tail call <256 x i8> @llvm.tpc.mov.dual.group.v256i8.i1(<256 x i8> %259, i32 65535, i32 0, i32 7168, <256 x i8> %257, i1 true, i1 false) #3
  %261 = tail call <256 x i8> @llvm.tpc.mov.dual.group.v256i8.i1(<256 x i8> %259, i32 -65536, i32 0, i32 7424, <256 x i8> %260, i1 true, i1 false) #3
  %262 = tail call <256 x i8> @llvm.tpc.mov.dual.group.v256i8.i1(<256 x i8> %259, i32 65535, i32 0, i32 11776, <256 x i8> %261, i1 true, i1 false) #3
  %263 = tail call <256 x i8> @llvm.tpc.mov.dual.group.v256i8.i1(<256 x i8> %259, i32 -65536, i32 0, i32 12032, <256 x i8> %262, i1 true, i1 false) #3
  tail call void @llvm.tpc.st.tnsr.v256i8(<5 x i32> %97, i8 1, <256 x i8> %170, i32 0, i1 true, i1 false)
  %264 = tail call <5 x i32> @llvm.tpc.add.mask.v5i32.i32(<5 x i32> %97, i32 1, i32 2, i8 2, i32 0, <5 x i32> %97, i1 true, i1 false)
  tail call void @llvm.tpc.st.tnsr.v256i8(<5 x i32> %264, i8 1, <256 x i8> %201, i32 0, i1 true, i1 false)
  %265 = tail call <5 x i32> @llvm.tpc.add.mask.v5i32.i32(<5 x i32> %264, i32 1, i32 2, i8 2, i32 0, <5 x i32> %264, i1 true, i1 false)
  tail call void @llvm.tpc.st.tnsr.v256i8(<5 x i32> %265, i8 1, <256 x i8> %232, i32 0, i1 true, i1 false)
  %266 = tail call <5 x i32> @llvm.tpc.add.mask.v5i32.i32(<5 x i32> %265, i32 1, i32 2, i8 2, i32 0, <5 x i32> %265, i1 true, i1 false)
  tail call void @llvm.tpc.st.tnsr.v256i8(<5 x i32> %266, i8 1, <256 x i8> %263, i32 0, i1 true, i1 false)
  %267 = tail call <5 x i32> @llvm.tpc.add.mask.v5i32.i32(<5 x i32> %266, i32 1, i32 2, i8 2, i32 0, <5 x i32> %266, i1 true, i1 false)
  %268 = icmp slt i32 %93, %30
  %269 = tail call <64 x float> @llvm.tpc.ld.tnsr.v64f32.i1(<5 x i32> %92, i8 0, i32 0, <64 x float> %128, i1 %268, i1 false)
  %270 = tail call <5 x i32> @llvm.tpc.add.mask.v5i32.i32(<5 x i32> %92, i32 1, i32 2, i8 2, i32 0, <5 x i32> %92, i1 true, i1 false)
  %271 = tail call <64 x float> @llvm.tpc.ld.tnsr.v64f32.i1(<5 x i32> %270, i8 0, i32 0, <64 x float> %130, i1 %268, i1 false)
  %272 = tail call <5 x i32> @llvm.tpc.add.mask.v5i32.i32(<5 x i32> %270, i32 1, i32 2, i8 2, i32 0, <5 x i32> %270, i1 true, i1 false)
  %273 = tail call <64 x float> @llvm.tpc.ld.tnsr.v64f32.i1(<5 x i32> %272, i8 0, i32 0, <64 x float> %132, i1 %268, i1 false)
  %274 = tail call <5 x i32> @llvm.tpc.add.mask.v5i32.i32(<5 x i32> %272, i32 1, i32 2, i8 2, i32 0, <5 x i32> %272, i1 true, i1 false)
  %275 = tail call <64 x float> @llvm.tpc.ld.tnsr.v64f32.i1(<5 x i32> %274, i8 0, i32 0, <64 x float> %134, i1 %268, i1 false)
  %276 = tail call <5 x i32> @llvm.tpc.add.mask.v5i32.i32(<5 x i32> %274, i32 1, i32 2, i8 2, i32 0, <5 x i32> %274, i1 true, i1 false)
  %277 = tail call <64 x float> @llvm.tpc.mac.v64f32.v64f32.i1(<64 x float> %269, <64 x float> %28, i8 0, i32 0, <64 x float> %26, i1 true, i1 false)
  %278 = tail call <64 x float> @llvm.tpc.mac.v64f32.v64f32.i1(<64 x float> %271, <64 x float> %28, i8 0, i32 0, <64 x float> %26, i1 true, i1 false)
  %279 = add nsw i32 %93, 4
  %280 = icmp slt i32 %279, %13
  br i1 %280, label %91, label %85
}

; Function Attrs: nounwind readnone
declare <5 x i32> @llvm.tpc.get.index.space.offset() #1

; Function Attrs: nounwind readnone
declare <5 x i32> @llvm.tpc.get.index.space.size() #1

; Function Attrs: nounwind readnone
declare <5 x i32> @llvm.tpc.add.mask.v5i32.v5i32(<5 x i32>, <5 x i32>, i32, i8, i32, <5 x i32>, i1, i1) #1

; Function Attrs: nounwind readnone
declare <64 x float> @llvm.tpc.ld.tnsr.v64f32.i1(<5 x i32>, i8, i32, <64 x float>, i1, i1) #1

; Function Attrs: nounwind readnone
declare <64 x float> @llvm.tpc.mac.v64f32.v64f32.i1(<64 x float>, <64 x float>, i8, i32, <64 x float>, i1, i1) #1

; Function Attrs: nounwind writeonly
declare void @llvm.tpc.st.tnsr.v256i8(<5 x i32>, i8, <256 x i8>, i32, i1, i1) #2

; Function Attrs: nounwind readnone
declare <64 x i32> @llvm.tpc.convert.v64i32.v64f32.i1(<64 x float>, i8, i32, <64 x i32>, i1, i1) #1

; Function Attrs: nounwind readnone
declare <256 x i8> @llvm.tpc.convert.int.v256i8.v64i32.v256i8.i1(<64 x i32>, <256 x i8>, i32, <256 x i8>, i1, i1) #1

; Function Attrs: nounwind readnone
declare <256 x i8> @llvm.tpc.pack.v256i8.i1(<256 x i8>, i8, i32, <256 x i8>, i1, i1) #1

; Function Attrs: nounwind readnone
declare <256 x i8> @llvm.tpc.mov.dual.group.v256i8.i1(<256 x i8>, i32, i32, i32, <256 x i8>, i1, i1) #1

; Function Attrs: nounwind readnone
declare <5 x i32> @llvm.tpc.add.mask.v5i32.i32(<5 x i32>, i32, i32, i8, i32, <5 x i32>, i1, i1) #1

attributes #0 = { nounwind writeonly "correctly-rounded-divide-sqrt-fp-math"="false" "disable-tail-calls"="false" "frame-pointer"="all" "less-precise-fpmad"="false" "min-legal-vector-width"="2048" "no-infs-fp-math"="false" "no-jump-tables"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="false" "stack-protector-buffer-size"="8" "target-cpu"="gaudi" "target-features"="+gaudi" "unsafe-fp-math"="false" "use-soft-float"="false" }
attributes #1 = { nounwind readnone }
attributes #2 = { nounwind writeonly }
attributes #3 = { nounwind }

!llvm.module.flags = !{!0}
!llvm.ident = !{!1}

!0 = !{i32 1, !"wchar_size", i32 4}
!1 = !{!"clang version 10.0.1 (ssh://gerrit:29418/tpc_llvm10 a787f6fb5fe429a5ac129363b2c223f559a9c49e)"}
!2 = !{i32 0}
