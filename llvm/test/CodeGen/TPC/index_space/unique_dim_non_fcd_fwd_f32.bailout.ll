; RUN: llc -march=tpc -mcpu=gaudi -O2 -emit-index-factors=true %s -o - | FileCheck %s --check-prefix=CHECK-LLVM
; RUN: llc -march=tpc -mcpu=gaudi -O2 -emit-index-factors=true -index-space-mlir %s -o - | FileCheck %s --check-prefix=CHECK-MLIR

; CHECK-LLVM: "SCEVCost From TPC Index":
; CHECK-LLVM: .ascii " SCEVBEGIN IndexSpace:[0].[Input].{ GC_CUSTOM, GC_CUSTOM, GC_CUSTOM, GC_CUSTOM, GC_CUSTOM }[1].[Input].{ GC_CUSTOM, GC_CUSTOM, GC_CUSTOM, GC_CUSTOM, GC_CUSTOM }[x0].[Input].{ GC_CUSTOM, GC_CUSTOM, GC_CUSTOM, GC_CUSTOM, GC_CUSTOM }[2].[Output].{ GC_CUSTOM, GC_CUSTOM, GC_CUSTOM, GC_CUSTOM, GC_CUSTOM }[x1].[Output].{ GC_CUSTOM, GC_CUSTOM, GC_CUSTOM, GC_CUSTOM, GC_CUSTOM }[x2].[Output].{ GC_CUSTOM, GC_CUSTOM, GC_CUSTOM, GC_CUSTOM, GC_CUSTOM } #SCEVEND"

; CHECK-MLIR: "SCEVCost From TPC Index":
; CHECK-MLIR: .ascii " SCEVBEGIN IndexSpace:[0].[Input].{ 0, 0, 0, 0, 0 }[1].[Input].{ 0, 0, 0, 0, 0 }[x0].[Input].{ 0, 0, 0, 0, 0 }[2].[Output].{ 0, 0, 0, 0, 0 }[x1].[Output].{ 0, 0, 0, 0, 0 }[x2].[Output].{ 0, 0, 0, 0, 0 } #SCEVEND"

; ModuleID = './kernels/gaudi/misc/unique_dim_non_fcd_fwd_f32.c'
source_filename = "./kernels/gaudi/misc/unique_dim_non_fcd_fwd_f32.c"
target datalayout = "e-p0:32:32:32-p1:32:32:32-p2:32:32:32-p3:64:64:64-i32:32:32-n8:16:32-f16:16:16-f32:32:32-v160:32:32-v256:2048:2048-v2048:2048:2048-v4096:2048:2048-v8192:2048:2048"
target triple = "tpc"

@0 = external dso_local addrspace(1) global i32, section ".sldata"

; Function Attrs: nounwind
define dso_local void @main(i32 %0, i32 %1, i32 %2) local_unnamed_addr #0 {
  %4 = tail call i32 @llvm.tpc.ld.l.i32(i32 1048, i32 1, i32 undef, i1 true, i1 false) #4
  %5 = tail call i32 @llvm.tpc.ld.l.i32(i32 1052, i32 1, i32 undef, i1 true, i1 false) #4
  %6 = shl i32 %2, 3
  %7 = add i32 %6, 1040
  %8 = tail call i32 @llvm.tpc.ld.l.i32(i32 %7, i32 1, i32 undef, i1 true, i1 false) #4
  %9 = add i32 %6, 1044
  %10 = tail call i32 @llvm.tpc.ld.l.i32(i32 %9, i32 1, i32 undef, i1 true, i1 false) #4
  tail call void @llvm.tpc.st.l.i32(i32 1048, i32 %8, i32 1, i1 true, i1 false) #4
  tail call void @llvm.tpc.st.l.i32(i32 1052, i32 %10, i32 1, i1 true, i1 false) #4
  tail call void @llvm.tpc.st.l.i32(i32 %7, i32 %4, i32 1, i1 true, i1 false) #4
  tail call void @llvm.tpc.st.l.i32(i32 %9, i32 %5, i32 1, i1 true, i1 false) #4
  tail call void @llvm.tpc.st.l.i32(i32 1104, i32 %8, i32 1, i1 true, i1 false) #4
  tail call void @llvm.tpc.st.l.i32(i32 1108, i32 %10, i32 1, i1 true, i1 false) #4
  %11 = add i32 %6, 1096
  tail call void @llvm.tpc.st.l.i32(i32 %11, i32 %4, i32 1, i1 true, i1 false) #4
  %12 = add i32 %6, 1100
  tail call void @llvm.tpc.st.l.i32(i32 %12, i32 %5, i32 1, i1 true, i1 false) #4
  %13 = tail call i32 @llvm.tpc.ld.l.i32(i32 1040, i32 1, i32 undef, i1 true, i1 false) #4
  %14 = icmp eq i32 %13, 0
  %15 = tail call i32 @llvm.tpc.ld.l.i32(i32 1040, i32 1, i32 undef, i1 true, i1 false) #4
  %16 = select i1 %14, i32 1, i32 %15
  %17 = tail call i32 @llvm.tpc.ld.l.i32(i32 1048, i32 1, i32 undef, i1 true, i1 false) #4
  %18 = icmp eq i32 %17, 0
  %19 = tail call i32 @llvm.tpc.ld.l.i32(i32 1048, i32 1, i32 undef, i1 true, i1 false) #4
  %20 = select i1 %18, i32 1, i32 %19
  %21 = tail call i32 @llvm.tpc.ld.l.i32(i32 1056, i32 1, i32 undef, i1 true, i1 false) #4
  %22 = icmp eq i32 %21, 0
  %23 = tail call i32 @llvm.tpc.ld.l.i32(i32 1056, i32 1, i32 undef, i1 true, i1 false) #4
  %24 = select i1 %22, i32 1, i32 %23
  %25 = tail call i32 @llvm.tpc.ld.l.i32(i32 1064, i32 1, i32 undef, i1 true, i1 false) #4
  %26 = icmp eq i32 %25, 0
  %27 = tail call i32 @llvm.tpc.ld.l.i32(i32 1064, i32 1, i32 undef, i1 true, i1 false) #4
  %28 = select i1 %26, i32 1, i32 %27
  %29 = icmp eq i32 %0, 1
  %30 = icmp eq i32 %1, 1
  %31 = select i1 %29, i8 3, i8 0
  %32 = select i1 %29, i8 4, i8 3
  %33 = select i1 %30, i8 %32, i8 0
  tail call void @llvm.tpc.st.l.i32(i32 2312, i32 0, i32 1, i1 true, i1 false) #4
  %34 = icmp sgt i32 %20, 0
  br i1 %34, label %35, label %50

35:                                               ; preds = %3
  %36 = icmp sgt i32 %28, 0
  %37 = icmp sgt i32 %24, 0
  %38 = icmp sgt i32 %16, 0
  %39 = bitcast i32 addrspace(1)* null to i8 addrspace(1)*
  %40 = tail call <64 x i32> @llvm.read_register.v64i32(metadata !0) #4
  %41 = tail call <64 x float> @llvm.tpc.lookup.1c.v64f32.v64i32(<64 x i32> %40, i32 282, i32 0, <64 x float> undef, i1 true, i1 false) #4
  %42 = tail call <128 x float> @llvm.tpc.lookup.2c.v128f32.v64i32(<64 x i32> %40, i32 282, i32 0, <128 x float> undef, i1 true, i1 false) #4
  %43 = shufflevector <128 x float> %42, <128 x float> undef, <64 x i32> <i32 0, i32 1, i32 2, i32 3, i32 4, i32 5, i32 6, i32 7, i32 8, i32 9, i32 10, i32 11, i32 12, i32 13, i32 14, i32 15, i32 16, i32 17, i32 18, i32 19, i32 20, i32 21, i32 22, i32 23, i32 24, i32 25, i32 26, i32 27, i32 28, i32 29, i32 30, i32 31, i32 32, i32 33, i32 34, i32 35, i32 36, i32 37, i32 38, i32 39, i32 40, i32 41, i32 42, i32 43, i32 44, i32 45, i32 46, i32 47, i32 48, i32 49, i32 50, i32 51, i32 52, i32 53, i32 54, i32 55, i32 56, i32 57, i32 58, i32 59, i32 60, i32 61, i32 62, i32 63>
  %44 = shufflevector <128 x float> %42, <128 x float> undef, <64 x i32> <i32 64, i32 65, i32 66, i32 67, i32 68, i32 69, i32 70, i32 71, i32 72, i32 73, i32 74, i32 75, i32 76, i32 77, i32 78, i32 79, i32 80, i32 81, i32 82, i32 83, i32 84, i32 85, i32 86, i32 87, i32 88, i32 89, i32 90, i32 91, i32 92, i32 93, i32 94, i32 95, i32 96, i32 97, i32 98, i32 99, i32 100, i32 101, i32 102, i32 103, i32 104, i32 105, i32 106, i32 107, i32 108, i32 109, i32 110, i32 111, i32 112, i32 113, i32 114, i32 115, i32 116, i32 117, i32 118, i32 119, i32 120, i32 121, i32 122, i32 123, i32 124, i32 125, i32 126, i32 127>
  %45 = bitcast <64 x float> %41 to <256 x i8>
  %46 = bitcast <64 x float> %43 to <256 x i8>
  %47 = bitcast <64 x float> %44 to <256 x i8>
  br label %54

48:                                               ; preds = %185
  %49 = phi i32 [ %210, %185 ]
  br label %50

50:                                               ; preds = %48, %3
  %51 = phi i32 [ 0, %3 ], [ %49, %48 ]
  %52 = insertelement <64 x i32> undef, i32 %51, i32 0
  %53 = shufflevector <64 x i32> %52, <64 x i32> undef, <64 x i32> zeroinitializer
  tail call void @llvm.tpc.st.tnsr.v64i32(<5 x i32> zeroinitializer, i8 2, <64 x i32> %53, i32 0, i1 true, i1 false)
  ret void

54:                                               ; preds = %185, %35
  %55 = phi i32 [ 0, %35 ], [ %210, %185 ]
  %56 = phi i8 [ 0, %35 ], [ %191, %185 ]
  %57 = phi <5 x i32> [ zeroinitializer, %35 ], [ %190, %185 ]
  %58 = phi <5 x i32> [ zeroinitializer, %35 ], [ %189, %185 ]
  %59 = phi <5 x i32> [ zeroinitializer, %35 ], [ %213, %185 ]
  %60 = phi i32 [ 0, %35 ], [ %187, %185 ]
  %61 = phi i32 [ 0, %35 ], [ %214, %185 ]
  %62 = insertelement <5 x i32> %57, i32 %61, i32 1
  %63 = tail call <256 x i1> @llvm.tpc.cmp.eq.v256i1.v64i32.i32.i1(<64 x i32> zeroinitializer, i32 %61, i8 2, i32 0, <256 x i1> undef, i1 true, i1 false)
  %64 = icmp eq i32 %55, 0
  %65 = tail call i32 @llvm.tpc.mov.i32.i32.i1(i32 1, i8 2, i32 0, i32 %55, i1 %64, i1 false)
  %66 = icmp sgt i32 %65, 0
  br i1 %66, label %67, label %185

67:                                               ; preds = %54
  %68 = icmp ne i32 %55, 0
  %69 = add i32 %65, %60
  br label %70

70:                                               ; preds = %175, %67
  %71 = phi <5 x i32> [ %62, %67 ], [ %87, %175 ]
  %72 = phi <5 x i32> [ %58, %67 ], [ %86, %175 ]
  %73 = phi <5 x i32> [ %59, %67 ], [ %85, %175 ]
  %74 = phi i32 [ %60, %67 ], [ %171, %175 ]
  %75 = phi i32 [ 0, %67 ], [ %176, %175 ]
  %76 = insertelement <5 x i32> %72, i32 %75, i32 1
  br i1 %36, label %77, label %83

77:                                               ; preds = %70
  br label %105

78:                                               ; preds = %120
  %79 = phi <256 x i1> [ %121, %120 ]
  %80 = phi <5 x i32> [ %122, %120 ]
  %81 = phi <5 x i32> [ %123, %120 ]
  %82 = phi <5 x i32> [ %124, %120 ]
  br label %83

83:                                               ; preds = %78, %70
  %84 = phi <256 x i1> [ zeroinitializer, %70 ], [ %79, %78 ]
  %85 = phi <5 x i32> [ %73, %70 ], [ %80, %78 ]
  %86 = phi <5 x i32> [ %76, %70 ], [ %81, %78 ]
  %87 = phi <5 x i32> [ %71, %70 ], [ %82, %78 ]
  %88 = tail call <64 x i32> @llvm.tpc.mov.v64i32.i32.v256i1(i32 1, i8 2, i32 0, <64 x i32> zeroinitializer, <256 x i1> %84, i1 false)
  %89 = tail call <64 x i32> @llvm.tpc.mov.dual.group.all.v64i32.i1(<64 x i32> %88, i32 -1, i32 0, i32 16756992, <64 x i32> zeroinitializer, i1 true, i1 false) #4
  %90 = add <64 x i32> %89, %88
  %91 = tail call <64 x i32> @llvm.tpc.mov.dual.group.all.v64i32.i1(<64 x i32> %90, i32 -1, i32 0, i32 16731648, <64 x i32> %89, i1 true, i1 false) #4
  %92 = add <64 x i32> %90, %91
  %93 = tail call <64 x i32> @llvm.tpc.mov.group.v64i32.v64i32.i1(<64 x i32> %92, i32 -1, i32 63, <64 x i32> %91, i1 true, i1 false) #4
  %94 = add <64 x i32> %92, %93
  %95 = tail call <64 x i32> @llvm.tpc.shuffle.v64i32.i1(<64 x i32> %94, <256 x i8> %45, i8 2, i32 0, <64 x i32> %93, i1 true, i1 false) #4
  %96 = add <64 x i32> %94, %95
  %97 = tail call <64 x i32> @llvm.tpc.shuffle.v64i32.i1(<64 x i32> %96, <256 x i8> %46, i8 2, i32 0, <64 x i32> %95, i1 true, i1 false) #4
  %98 = add <64 x i32> %96, %97
  %99 = tail call <64 x i32> @llvm.tpc.shuffle.v64i32.i1(<64 x i32> %98, <256 x i8> %47, i8 2, i32 0, <64 x i32> %97, i1 true, i1 false) #4
  %100 = add <64 x i32> %98, %99
  tail call void @llvm.tpc.st.tnsr.v64i32(<5 x i32> zeroinitializer, i8 2, <64 x i32> %100, i32 0, i1 true, i1 false)
  tail call void @llvm.tpc.aso(i32 2, i1 true, i1 false)
  %101 = tail call i32 @llvm.tpc.ld.l.i32(i32 2312, i32 1, i32 undef, i1 true, i1 false) #4
  store volatile i32 %101, i32 addrspace(1)* null, align 4, !tbaa !5
  %102 = load volatile i32, i32 addrspace(1)* null, align 4, !tbaa !5
  %103 = icmp eq i32 %102, %74
  br i1 %103, label %104, label %170

104:                                              ; preds = %83
  br label %165

105:                                              ; preds = %77, %120
  %106 = phi <5 x i32> [ %124, %120 ], [ %71, %77 ]
  %107 = phi <5 x i32> [ %123, %120 ], [ %76, %77 ]
  %108 = phi <5 x i32> [ %122, %120 ], [ %73, %77 ]
  %109 = phi <256 x i1> [ %121, %120 ], [ zeroinitializer, %77 ]
  %110 = phi i32 [ %125, %120 ], [ 0, %77 ]
  %111 = insertelement <5 x i32> %106, i32 %110, i32 3
  %112 = insertelement <5 x i32> %107, i32 %110, i32 3
  %113 = insertelement <5 x i32> %108, i32 %110, i32 3
  br i1 %37, label %114, label %120

114:                                              ; preds = %105
  br label %127

115:                                              ; preds = %142
  %116 = phi <256 x i1> [ %143, %142 ]
  %117 = phi <5 x i32> [ %144, %142 ]
  %118 = phi <5 x i32> [ %145, %142 ]
  %119 = phi <5 x i32> [ %146, %142 ]
  br label %120

120:                                              ; preds = %115, %105
  %121 = phi <256 x i1> [ %109, %105 ], [ %116, %115 ]
  %122 = phi <5 x i32> [ %113, %105 ], [ %117, %115 ]
  %123 = phi <5 x i32> [ %112, %105 ], [ %118, %115 ]
  %124 = phi <5 x i32> [ %111, %105 ], [ %119, %115 ]
  %125 = add nuw nsw i32 %110, 1
  %126 = icmp eq i32 %125, %28
  br i1 %126, label %78, label %105, !llvm.loop !9

127:                                              ; preds = %114, %142
  %128 = phi <5 x i32> [ %146, %142 ], [ %111, %114 ]
  %129 = phi <5 x i32> [ %145, %142 ], [ %112, %114 ]
  %130 = phi <5 x i32> [ %144, %142 ], [ %113, %114 ]
  %131 = phi <256 x i1> [ %143, %142 ], [ %109, %114 ]
  %132 = phi i32 [ %147, %142 ], [ 0, %114 ]
  %133 = insertelement <5 x i32> %128, i32 %132, i32 2
  %134 = insertelement <5 x i32> %129, i32 %132, i32 2
  %135 = insertelement <5 x i32> %130, i32 %132, i32 2
  br i1 %38, label %136, label %142

136:                                              ; preds = %127
  br label %149

137:                                              ; preds = %149
  %138 = phi <5 x i32> [ %155, %149 ]
  %139 = phi <5 x i32> [ %156, %149 ]
  %140 = phi <5 x i32> [ %157, %149 ]
  %141 = phi <256 x i1> [ %162, %149 ]
  br label %142

142:                                              ; preds = %137, %127
  %143 = phi <256 x i1> [ %131, %127 ], [ %141, %137 ]
  %144 = phi <5 x i32> [ %135, %127 ], [ %140, %137 ]
  %145 = phi <5 x i32> [ %134, %127 ], [ %139, %137 ]
  %146 = phi <5 x i32> [ %133, %127 ], [ %138, %137 ]
  %147 = add nuw nsw i32 %132, 1
  %148 = icmp eq i32 %147, %24
  br i1 %148, label %115, label %127, !llvm.loop !11

149:                                              ; preds = %136, %149
  %150 = phi <5 x i32> [ %155, %149 ], [ %133, %136 ]
  %151 = phi <5 x i32> [ %156, %149 ], [ %134, %136 ]
  %152 = phi <5 x i32> [ %157, %149 ], [ %135, %136 ]
  %153 = phi <256 x i1> [ %162, %149 ], [ %131, %136 ]
  %154 = phi i32 [ %163, %149 ], [ 0, %136 ]
  %155 = insertelement <5 x i32> %150, i32 %154, i32 0
  %156 = insertelement <5 x i32> %151, i32 %154, i32 0
  %157 = insertelement <5 x i32> %152, i32 %154, i32 0
  %158 = tail call <64 x float> @llvm.tpc.ld.tnsr.v64f32.i1(<5 x i32> %155, i8 0, i32 0, <64 x float> undef, i1 true, i1 false)
  %159 = tail call <64 x float> @llvm.tpc.ld.tnsr.v64f32.i1(<5 x i32> %156, i8 1, i32 0, <64 x float> zeroinitializer, i1 %68, i1 false)
  %160 = tail call <256 x i1> @llvm.tpc.cmp.neq.v256i1.v64f32.v64f32.i1(<64 x float> %158, <64 x float> %159, i8 0, i32 0, <256 x i1> undef, i1 true, i1 false)
  %161 = or <256 x i1> %153, %63
  %162 = or <256 x i1> %161, %160
  tail call void @llvm.tpc.st.tnsr.v64f32(<5 x i32> %157, i8 1, <64 x float> %158, i32 0, i1 true, i1 false)
  %163 = add nuw nsw i32 %154, 64
  %164 = icmp slt i32 %163, %16
  br i1 %164, label %149, label %137, !llvm.loop !12

165:                                              ; preds = %104, %165
  %166 = tail call i32 @llvm.tpc.ld.l.i32(i32 2312, i32 1, i32 undef, i1 true, i1 false) #4
  store volatile i32 %166, i32 addrspace(1)* null, align 4, !tbaa !5
  %167 = load volatile i32, i32 addrspace(1)* null, align 4, !tbaa !5
  %168 = icmp eq i32 %167, %74
  br i1 %168, label %165, label %169

169:                                              ; preds = %165
  br label %170

170:                                              ; preds = %169, %83
  %171 = add nsw i32 %74, 1
  %172 = tail call i8 addrspace(3)* @llvm.tpc.gen.addr(<5 x i32> zeroinitializer, i8 2, i32 0, i8 addrspace(3)* undef, i1 true, i1 false)
  %173 = tail call i32 @llvm.tpc.ld.g.i32.i1(i8 addrspace(3)* %172, i32 0, i32 undef, i1 true, i1 false)
  %174 = icmp eq i32 %173, 0
  br i1 %174, label %178, label %175

175:                                              ; preds = %170
  %176 = add nuw nsw i32 %75, 1
  %177 = icmp eq i32 %176, %65
  br i1 %177, label %178, label %70, !llvm.loop !13

178:                                              ; preds = %175, %170
  %179 = phi <5 x i32> [ %85, %175 ], [ %85, %170 ]
  %180 = phi <5 x i32> [ %86, %175 ], [ %86, %170 ]
  %181 = phi <5 x i32> [ %87, %175 ], [ %87, %170 ]
  %182 = phi i32 [ %75, %170 ], [ %65, %175 ]
  %183 = phi i32 [ %171, %170 ], [ %69, %175 ]
  %184 = phi i8 [ 0, %170 ], [ 1, %175 ]
  br label %185

185:                                              ; preds = %178, %54
  %186 = phi i32 [ 0, %54 ], [ %182, %178 ]
  %187 = phi i32 [ %60, %54 ], [ %183, %178 ]
  %188 = phi <5 x i32> [ %59, %54 ], [ %179, %178 ]
  %189 = phi <5 x i32> [ %58, %54 ], [ %180, %178 ]
  %190 = phi <5 x i32> [ %62, %54 ], [ %181, %178 ]
  %191 = phi i8 [ %56, %54 ], [ %184, %178 ]
  %192 = and i8 %191, 1
  %193 = icmp ne i8 %192, 0
  %194 = tail call i32 @llvm.tpc.mov.i32.i32.i1(i32 %55, i8 2, i32 0, i32 %186, i1 %193, i1 false)
  %195 = insertelement <5 x i32> <i32 undef, i32 0, i32 0, i32 0, i32 0>, i32 %194, i32 0
  %196 = tail call <64 x i32> @llvm.tpc.mov.v64i32.i32.i1(i32 0, i8 2, i32 0, <64 x i32> undef, i1 true, i1 false)
  %197 = tail call <64 x i32> @llvm.tpc.ld.tnsr.v64i32.i1(<5 x i32> %195, i8 %33, i32 0, <64 x i32> %196, i1 %30, i1 false)
  %198 = tail call <64 x i32> @llvm.read_register.v64i32(metadata !0)
  %199 = tail call <256 x i1> @llvm.tpc.cmp.eq.v256i1.v64i32.i32.i1(<64 x i32> %198, i32 0, i8 2, i32 0, <256 x i1> undef, i1 true, i1 false)
  %200 = tail call <64 x i32> @llvm.tpc.add.v64i32.v64i32.v64i32.v256i1(<64 x i32> %197, <64 x i32> <i32 1, i32 1, i32 1, i32 1, i32 1, i32 1, i32 1, i32 1, i32 1, i32 1, i32 1, i32 1, i32 1, i32 1, i32 1, i32 1, i32 1, i32 1, i32 1, i32 1, i32 1, i32 1, i32 1, i32 1, i32 1, i32 1, i32 1, i32 1, i32 1, i32 1, i32 1, i32 1, i32 1, i32 1, i32 1, i32 1, i32 1, i32 1, i32 1, i32 1, i32 1, i32 1, i32 1, i32 1, i32 1, i32 1, i32 1, i32 1, i32 1, i32 1, i32 1, i32 1, i32 1, i32 1, i32 1, i32 1, i32 1, i32 1, i32 1, i32 1, i32 1, i32 1, i32 1, i32 1>, i8 2, i32 0, <64 x i32> %197, <256 x i1> %199, i1 false)
  %201 = tail call <64 x i32> @llvm.tpc.mov.v64i32.i32.v256i1(i32 1, i8 2, i32 0, <64 x i32> zeroinitializer, <256 x i1> %199, i1 false)
  %202 = tail call <64 x i32> @llvm.tpc.mov.v64i32.i32.i1(i32 1, i8 2, i32 0, <64 x i32> zeroinitializer, i1 %193, i1 false)
  %203 = tail call <64 x i32> @llvm.tpc.sel.eq.v64i32.v64i32.i32.v64i32.v64i32.i1(<64 x i32> %202, i32 1, <64 x i32> %201, <64 x i32> %200, i8 2, i32 0, <64 x i32> undef, i1 true, i1 false)
  tail call void @llvm.tpc.st.tnsr.v64i32(<5 x i32> %195, i8 %33, <64 x i32> %203, i32 0, i1 %30, i1 false)
  %204 = insertelement <5 x i32> <i32 undef, i32 0, i32 0, i32 0, i32 0>, i32 %61, i32 0
  %205 = insertelement <64 x i32> undef, i32 %55, i32 0
  %206 = shufflevector <64 x i32> %205, <64 x i32> undef, <64 x i32> zeroinitializer
  %207 = insertelement <64 x i32> undef, i32 %186, i32 0
  %208 = shufflevector <64 x i32> %207, <64 x i32> undef, <64 x i32> zeroinitializer
  %209 = tail call <64 x i32> @llvm.tpc.sel.eq.v64i32.v64i32.i32.v64i32.v64i32.v256i1(<64 x i32> %202, i32 1, <64 x i32> %206, <64 x i32> %208, i8 2, i32 0, <64 x i32> zeroinitializer, <256 x i1> %199, i1 false)
  tail call void @llvm.tpc.st.tnsr.v64i32(<5 x i32> %204, i8 %31, <64 x i32> %209, i32 0, i1 %29, i1 false)
  %210 = tail call i32 @llvm.tpc.add.i32.i32.i32.i1(i32 1, i32 %55, i8 2, i32 0, i32 %55, i1 %193, i1 false)
  %211 = extractelement <5 x i32> %188, i32 1
  %212 = add nsw i32 %211, 1
  %213 = tail call <5 x i32> @llvm.tpc.set.indx(<5 x i32> %188, i32 2, i32 %212, i32 0, i1 %193, i1 false)
  %214 = add nuw nsw i32 %61, 1
  %215 = icmp eq i32 %214, %20
  br i1 %215, label %48, label %54, !llvm.loop !14
}

; Function Attrs: argmemonly nounwind willreturn
declare void @llvm.lifetime.start.p0i8(i64 immarg, i8* nocapture) #1

; Function Attrs: nounwind readnone
declare <256 x i1> @llvm.tpc.cmp.eq.v256i1.v64i32.i32.i1(<64 x i32>, i32, i8, i32, <256 x i1>, i1, i1) #2

; Function Attrs: nounwind readnone
declare i32 @llvm.tpc.mov.i32.i32.i1(i32, i8, i32, i32, i1, i1) #2

; Function Attrs: nounwind readnone
declare <64 x float> @llvm.tpc.ld.tnsr.v64f32.i1(<5 x i32>, i8, i32, <64 x float>, i1, i1) #2

; Function Attrs: nounwind readnone
declare <256 x i1> @llvm.tpc.cmp.neq.v256i1.v64f32.v64f32.i1(<64 x float>, <64 x float>, i8, i32, <256 x i1>, i1, i1) #2

; Function Attrs: nounwind writeonly
declare void @llvm.tpc.st.tnsr.v64f32(<5 x i32>, i8, <64 x float>, i32, i1, i1) #3

; Function Attrs: argmemonly nounwind willreturn
declare void @llvm.lifetime.end.p0i8(i64 immarg, i8* nocapture) #1

; Function Attrs: nounwind readnone
declare <64 x i32> @llvm.tpc.mov.v64i32.i32.v256i1(i32, i8, i32, <64 x i32>, <256 x i1>, i1) #2

; Function Attrs: nounwind writeonly
declare void @llvm.tpc.st.tnsr.v64i32(<5 x i32>, i8, <64 x i32>, i32, i1, i1) #3

; Function Attrs: nounwind
declare void @llvm.tpc.aso(i32, i1, i1) #4

; Function Attrs: nounwind readnone
declare i8 addrspace(3)* @llvm.tpc.gen.addr(<5 x i32>, i8, i32, i8 addrspace(3)*, i1, i1) #2

; Function Attrs: nounwind readonly
declare i32 @llvm.tpc.ld.g.i32.i1(i8 addrspace(3)*, i32, i32, i1, i1) #5

; Function Attrs: nounwind readnone
declare <64 x i32> @llvm.tpc.mov.v64i32.i32.i1(i32, i8, i32, <64 x i32>, i1, i1) #2

; Function Attrs: nounwind readnone
declare <64 x i32> @llvm.tpc.ld.tnsr.v64i32.i1(<5 x i32>, i8, i32, <64 x i32>, i1, i1) #2

; Function Attrs: nounwind readonly
declare <64 x i32> @llvm.read_register.v64i32(metadata) #5

; Function Attrs: nounwind readnone
declare <64 x i32> @llvm.tpc.add.v64i32.v64i32.v64i32.v256i1(<64 x i32>, <64 x i32>, i8, i32, <64 x i32>, <256 x i1>, i1) #2

; Function Attrs: nounwind readnone
declare <64 x i32> @llvm.tpc.sel.eq.v64i32.v64i32.i32.v64i32.v64i32.i1(<64 x i32>, i32, <64 x i32>, <64 x i32>, i8, i32, <64 x i32>, i1, i1) #2

; Function Attrs: nounwind readnone
declare <64 x i32> @llvm.tpc.sel.eq.v64i32.v64i32.i32.v64i32.v64i32.v256i1(<64 x i32>, i32, <64 x i32>, <64 x i32>, i8, i32, <64 x i32>, <256 x i1>, i1) #2

; Function Attrs: nounwind readnone
declare i32 @llvm.tpc.add.i32.i32.i32.i1(i32, i32, i8, i32, i32, i1, i1) #2

; Function Attrs: nounwind readnone
declare <5 x i32> @llvm.tpc.set.indx(<5 x i32>, i32, i32, i32, i1, i1) #2

; Function Attrs: nounwind speculatable
declare i32 @llvm.tpc.ld.l.i32(i32, i32, i32, i1, i1) #6

; Function Attrs: nounwind writeonly
declare void @llvm.tpc.st.l.i32(i32, i32, i32, i1, i1) #3

; Function Attrs: nounwind readnone
declare <64 x float> @llvm.tpc.lookup.1c.v64f32.v64i32(<64 x i32>, i32, i32, <64 x float>, i1, i1) #2

; Function Attrs: nounwind readnone
declare <128 x float> @llvm.tpc.lookup.2c.v128f32.v64i32(<64 x i32>, i32, i32, <128 x float>, i1, i1) #2

; Function Attrs: nounwind readnone
declare <64 x i32> @llvm.tpc.mov.dual.group.all.v64i32.i1(<64 x i32>, i32, i32, i32, <64 x i32>, i1, i1) #2

; Function Attrs: nounwind readnone
declare <64 x i32> @llvm.tpc.mov.group.v64i32.v64i32.i1(<64 x i32>, i32, i32, <64 x i32>, i1, i1) #2

; Function Attrs: nounwind readnone
declare <64 x i32> @llvm.tpc.shuffle.v64i32.i1(<64 x i32>, <256 x i8>, i8, i32, <64 x i32>, i1, i1) #2

attributes #0 = { nounwind "correctly-rounded-divide-sqrt-fp-math"="false" "disable-tail-calls"="false" "frame-pointer"="all" "less-precise-fpmad"="false" "min-legal-vector-width"="2048" "no-infs-fp-math"="false" "no-jump-tables"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="false" "stack-protector-buffer-size"="8" "target-cpu"="gaudi" "target-features"="+gaudi" "unsafe-fp-math"="false" "use-soft-float"="false" }
attributes #1 = { argmemonly nounwind willreturn }
attributes #2 = { nounwind readnone }
attributes #3 = { nounwind writeonly }
attributes #4 = { nounwind }
attributes #5 = { nounwind readonly }
attributes #6 = { nounwind speculatable }

!llvm.named.register.v_lane_id_32 = !{!0}
!llvm.module.flags = !{!1}
!llvm.ident = !{!2}

!0 = !{!"v_lane_id_32"}
!1 = !{i32 1, !"wchar_size", i32 4}
!2 = !{!"clang version 10.0.1 (ssh://gerrit:29418/tpc_llvm10 a787f6fb5fe429a5ac129363b2c223f559a9c49e)"}
!3 = !{i32 4}
!4 = !{i32 0}
!5 = !{!6, !6, i64 0}
!6 = !{!"int", !7, i64 0}
!7 = !{!"omnipotent char", !8, i64 0}
!8 = !{!"Simple C/C++ TBAA"}
!9 = distinct !{!9, !10}
!10 = !{!"llvm.loop.taken", i1 true}
!11 = distinct !{!11, !10}
!12 = distinct !{!12, !10}
!13 = distinct !{!13, !10}
!14 = distinct !{!14, !10}
