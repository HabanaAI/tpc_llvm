; RUN: tpc-clang -S -O3 -march=gaudi %s -o - | FileCheck %s
; ModuleID = 'tpc_kernel'
source_filename = "fused_kernel"
target datalayout = "e-p0:32:32:32-p1:32:32:32-p2:32:32:32-p3:64:64:64-i32:32:32-n8:16:32-f16:16:16-f32:32:32-v160:32:32-v256:2048:2048-v2048:2048:2048-v4096:2048:2048-v8192:2048:2048"
target triple = "tpc"


; Function Attrs: nounwind readnone
declare <64 x float> @llvm.tpc.ld.tnsr.v64f32.i1(<5 x i32>, i8, i32, <64 x float>, i1, i1) #0

; Function Attrs: nounwind writeonly
declare void @llvm.tpc.st.tnsr.v64f32(<5 x i32>, i8, <64 x float>, i32, i1, i1) #1

; Function Attrs: nounwind readnone
declare <64 x i32> @llvm.tpc.ld.tnsr.v64i32.i1(<5 x i32>, i8, i32, <64 x i32>, i1, i1) #0

; Function Attrs: nounwind readnone
declare <256 x float> @llvm.tpc.sitofp.swch.v256f32.v256i8(<256 x i8>, i32) #0

; Function Attrs: nounwind readnone
declare <64 x float> @llvm.tpc.sitofp.swch.v64f32.v64i32(<64 x i32>, i32) #0

; Function Attrs: nounwind readnone
declare <256 x i8> @llvm.tpc.fptosi.swch.v256i8.v256f32(<256 x float>, i32) #0

; Function Attrs: nounwind readnone
declare <5 x i32> @llvm.tpc.get.index.space.offset() #0

; Function Attrs: nounwind readnone
declare <5 x i32> @llvm.tpc.get.index.space.size() #0

; CHECK-NOT: Cannot select
; CHECK: convert.i32 target_type=fp32  %V{{[0-9]+}}, %V{{[0-9]+}}, %SP{{[0-9]+}}
; CHECK: convert.i32 target_type=fp32  %V{{[0-9]+}}, %V{{[0-9]+}}, %SP{{[0-9]+}}
; CHECK: convert.i32 target_type=fp32  %V{{[0-9]+}}, %V{{[0-9]+}}, %SP{{[0-9]+}}
; CHECK: convert.i32 target_type=fp32  %V{{[0-9]+}}, %V{{[0-9]+}}, %SP{{[0-9]+}}

; Function Attrs: nounwind writeonly
define void @main() {
bb:
	%i = call <5 x i32> @llvm.tpc.get.index.space.offset()
	%i35 = call <5 x i32> @llvm.tpc.get.index.space.size()
	%i36 = add <5 x i32> %i, %i35
	%i37 = extractelement <5 x i32> %i, i32 0
	%i38 = shl i32 %i37, 8
	%i39 = extractelement <5 x i32> %i36, i32 0
	%i40 = shl i32 %i39, 8
	br label %bb41

	bb41:                                             ; preds = %bb44, %bb
	%.sroa.033.0 = phi <5 x i32> [ zeroinitializer, %bb ], [ %.sroa.033.0.vec.insert, %bb44 ]
	%.sroa.031.0 = phi <5 x i32> [ zeroinitializer, %bb ], [ %.sroa.031.0.vec.insert, %bb44 ]
	%.sroa.029.0 = phi <5 x i32> [ zeroinitializer, %bb ], [ %.sroa.029.0.vec.insert, %bb44 ]
	%.sroa.027.0 = phi <5 x i32> [ zeroinitializer, %bb ], [ %.sroa.027.0.vec.insert, %bb44 ]
	%.sroa.025.0 = phi <5 x i32> [ zeroinitializer, %bb ], [ %.sroa.025.0.vec.insert, %bb44 ]
	%.sroa.023.0 = phi <5 x i32> [ zeroinitializer, %bb ], [ %.sroa.023.0.vec.insert, %bb44 ]
	%.sroa.021.0 = phi <5 x i32> [ zeroinitializer, %bb ], [ %.sroa.021.0.vec.insert, %bb44 ]
	%.sroa.019.0 = phi <5 x i32> [ zeroinitializer, %bb ], [ %.sroa.019.0.vec.insert, %bb44 ]
	%.sroa.017.0 = phi <5 x i32> [ zeroinitializer, %bb ], [ %.sroa.017.0.vec.insert, %bb44 ]
	%.sroa.015.0 = phi <5 x i32> [ zeroinitializer, %bb ], [ %.sroa.015.0.vec.insert, %bb44 ]
	%.sroa.013.0 = phi <5 x i32> [ zeroinitializer, %bb ], [ %.sroa.013.0.vec.insert, %bb44 ]
	%.sroa.0.0 = phi <5 x i32> [ zeroinitializer, %bb ], [ %.sroa.0.0.vec.insert, %bb44 ]
	%i42 = phi i32 [ %i77, %bb44 ], [ %i38, %bb ]
	%i43 = icmp slt i32 %i42, %i40
	br i1 %i43, label %bb44, label %bb78

	bb44:                                             ; preds = %bb41
	%.sroa.0.0.vec.insert = insertelement <5 x i32> %.sroa.0.0, i32 %i42, i32 0
	%i45 = call <64 x i32> @llvm.tpc.ld.tnsr.v64i32.i1(<5 x i32> %.sroa.0.0.vec.insert, i8 0, i32 0, <64 x i32> undef, i1 true, i1 false)
	%i46 = or i32 %i42, 64
	%.sroa.013.0.vec.insert = insertelement <5 x i32> %.sroa.013.0, i32 %i46, i32 0
	%i47 = call <64 x i32> @llvm.tpc.ld.tnsr.v64i32.i1(<5 x i32> %.sroa.013.0.vec.insert, i8 0, i32 0, <64 x i32> undef, i1 true, i1 false)
	%i48 = or i32 %i42, 128
	%.sroa.015.0.vec.insert = insertelement <5 x i32> %.sroa.015.0, i32 %i48, i32 0
	%i49 = call <64 x i32> @llvm.tpc.ld.tnsr.v64i32.i1(<5 x i32> %.sroa.015.0.vec.insert, i8 0, i32 0, <64 x i32> undef, i1 true, i1 false)
	%i50 = or i32 %i42, 192
	%.sroa.017.0.vec.insert = insertelement <5 x i32> %.sroa.017.0, i32 %i50, i32 0
	%i51 = call <64 x i32> @llvm.tpc.ld.tnsr.v64i32.i1(<5 x i32> %.sroa.017.0.vec.insert, i8 0, i32 0, <64 x i32> undef, i1 true, i1 false)
	%i52 = call <64 x float> @llvm.tpc.sitofp.swch.v64f32.v64i32(<64 x i32> %i45, i32 327680)
	%i53 = call <64 x float> @llvm.tpc.sitofp.swch.v64f32.v64i32(<64 x i32> %i47, i32 327680)
	%i54 = call <64 x float> @llvm.tpc.sitofp.swch.v64f32.v64i32(<64 x i32> %i49, i32 327680)
	%i55 = call <64 x float> @llvm.tpc.sitofp.swch.v64f32.v64i32(<64 x i32> %i51, i32 327680)
	%.sroa.019.0.vec.insert = insertelement <5 x i32> %.sroa.019.0, i32 %i42, i32 0
	%i56 = call <64 x float> @llvm.tpc.ld.tnsr.v64f32.i1(<5 x i32> %.sroa.019.0.vec.insert, i8 1, i32 0, <64 x float> undef, i1 true, i1 false)
	%.sroa.021.0.vec.insert = insertelement <5 x i32> %.sroa.021.0, i32 %i46, i32 0
	%i57 = call <64 x float> @llvm.tpc.ld.tnsr.v64f32.i1(<5 x i32> %.sroa.021.0.vec.insert, i8 1, i32 0, <64 x float> undef, i1 true, i1 false)
	%.sroa.023.0.vec.insert = insertelement <5 x i32> %.sroa.023.0, i32 %i48, i32 0
	%i58 = call <64 x float> @llvm.tpc.ld.tnsr.v64f32.i1(<5 x i32> %.sroa.023.0.vec.insert, i8 1, i32 0, <64 x float> undef, i1 true, i1 false)
	%.sroa.025.0.vec.insert = insertelement <5 x i32> %.sroa.025.0, i32 %i50, i32 0
	%i59 = call <64 x float> @llvm.tpc.ld.tnsr.v64f32.i1(<5 x i32> %.sroa.025.0.vec.insert, i8 1, i32 0, <64 x float> undef, i1 true, i1 false)
	%i60 = fcmp oeq <64 x float> %i52, %i56
	%i61 = fcmp oeq <64 x float> %i53, %i57
	%i62 = fcmp oeq <64 x float> %i54, %i58
	%i63 = fcmp oeq <64 x float> %i55, %i59
	%i64 = select <64 x i1> %i60, <64 x float> <float 1.000000e+00, float 1.000000e+00, float 1.000000e+00, float 1.000000e+00, float 1.000000e+00, float 1.000000e+00, float 1.000000e+00, float 1.000000e+00, float 1.000000e+00, float 1.000000e+00, float 1.000000e+00, float 1.000000e+00, float 1.000000e+00, float 1.000000e+00, float 1.000000e+00, float 1.000000e+00, float 1.000000e+00, float 1.000000e+00, float 1.000000e+00, float 1.000000e+00, float 1.000000e+00, float 1.000000e+00, float 1.000000e+00, float 1.000000e+00, float 1.000000e+00, float 1.000000e+00, float 1.000000e+00, float 1.000000e+00, float 1.000000e+00, float 1.000000e+00, float 1.000000e+00, float 1.000000e+00, float 1.000000e+00, float 1.000000e+00, float 1.000000e+00, float 1.000000e+00, float 1.000000e+00, float 1.000000e+00, float 1.000000e+00, float 1.000000e+00, float 1.000000e+00, float 1.000000e+00, float 1.000000e+00, float 1.000000e+00, float 1.000000e+00, float 1.000000e+00, float 1.000000e+00, float 1.000000e+00, float 1.000000e+00, float 1.000000e+00, float 1.000000e+00, float 1.000000e+00, float 1.000000e+00, float 1.000000e+00, float 1.000000e+00, float 1.000000e+00, float 1.000000e+00, float 1.000000e+00, float 1.000000e+00, float 1.000000e+00, float 1.000000e+00, float 1.000000e+00, float 1.000000e+00, float 1.000000e+00>, <64 x float> zeroinitializer
	%i65 = select <64 x i1> %i61, <64 x float> <float 1.000000e+00, float 1.000000e+00, float 1.000000e+00, float 1.000000e+00, float 1.000000e+00, float 1.000000e+00, float 1.000000e+00, float 1.000000e+00, float 1.000000e+00, float 1.000000e+00, float 1.000000e+00, float 1.000000e+00, float 1.000000e+00, float 1.000000e+00, float 1.000000e+00, float 1.000000e+00, float 1.000000e+00, float 1.000000e+00, float 1.000000e+00, float 1.000000e+00, float 1.000000e+00, float 1.000000e+00, float 1.000000e+00, float 1.000000e+00, float 1.000000e+00, float 1.000000e+00, float 1.000000e+00, float 1.000000e+00, float 1.000000e+00, float 1.000000e+00, float 1.000000e+00, float 1.000000e+00, float 1.000000e+00, float 1.000000e+00, float 1.000000e+00, float 1.000000e+00, float 1.000000e+00, float 1.000000e+00, float 1.000000e+00, float 1.000000e+00, float 1.000000e+00, float 1.000000e+00, float 1.000000e+00, float 1.000000e+00, float 1.000000e+00, float 1.000000e+00, float 1.000000e+00, float 1.000000e+00, float 1.000000e+00, float 1.000000e+00, float 1.000000e+00, float 1.000000e+00, float 1.000000e+00, float 1.000000e+00, float 1.000000e+00, float 1.000000e+00, float 1.000000e+00, float 1.000000e+00, float 1.000000e+00, float 1.000000e+00, float 1.000000e+00, float 1.000000e+00, float 1.000000e+00, float 1.000000e+00>, <64 x float> zeroinitializer
	%i66 = select <64 x i1> %i62, <64 x float> <float 1.000000e+00, float 1.000000e+00, float 1.000000e+00, float 1.000000e+00, float 1.000000e+00, float 1.000000e+00, float 1.000000e+00, float 1.000000e+00, float 1.000000e+00, float 1.000000e+00, float 1.000000e+00, float 1.000000e+00, float 1.000000e+00, float 1.000000e+00, float 1.000000e+00, float 1.000000e+00, float 1.000000e+00, float 1.000000e+00, float 1.000000e+00, float 1.000000e+00, float 1.000000e+00, float 1.000000e+00, float 1.000000e+00, float 1.000000e+00, float 1.000000e+00, float 1.000000e+00, float 1.000000e+00, float 1.000000e+00, float 1.000000e+00, float 1.000000e+00, float 1.000000e+00, float 1.000000e+00, float 1.000000e+00, float 1.000000e+00, float 1.000000e+00, float 1.000000e+00, float 1.000000e+00, float 1.000000e+00, float 1.000000e+00, float 1.000000e+00, float 1.000000e+00, float 1.000000e+00, float 1.000000e+00, float 1.000000e+00, float 1.000000e+00, float 1.000000e+00, float 1.000000e+00, float 1.000000e+00, float 1.000000e+00, float 1.000000e+00, float 1.000000e+00, float 1.000000e+00, float 1.000000e+00, float 1.000000e+00, float 1.000000e+00, float 1.000000e+00, float 1.000000e+00, float 1.000000e+00, float 1.000000e+00, float 1.000000e+00, float 1.000000e+00, float 1.000000e+00, float 1.000000e+00, float 1.000000e+00>, <64 x float> zeroinitializer
	%i67 = select <64 x i1> %i63, <64 x float> <float 1.000000e+00, float 1.000000e+00, float 1.000000e+00, float 1.000000e+00, float 1.000000e+00, float 1.000000e+00, float 1.000000e+00, float 1.000000e+00, float 1.000000e+00, float 1.000000e+00, float 1.000000e+00, float 1.000000e+00, float 1.000000e+00, float 1.000000e+00, float 1.000000e+00, float 1.000000e+00, float 1.000000e+00, float 1.000000e+00, float 1.000000e+00, float 1.000000e+00, float 1.000000e+00, float 1.000000e+00, float 1.000000e+00, float 1.000000e+00, float 1.000000e+00, float 1.000000e+00, float 1.000000e+00, float 1.000000e+00, float 1.000000e+00, float 1.000000e+00, float 1.000000e+00, float 1.000000e+00, float 1.000000e+00, float 1.000000e+00, float 1.000000e+00, float 1.000000e+00, float 1.000000e+00, float 1.000000e+00, float 1.000000e+00, float 1.000000e+00, float 1.000000e+00, float 1.000000e+00, float 1.000000e+00, float 1.000000e+00, float 1.000000e+00, float 1.000000e+00, float 1.000000e+00, float 1.000000e+00, float 1.000000e+00, float 1.000000e+00, float 1.000000e+00, float 1.000000e+00, float 1.000000e+00, float 1.000000e+00, float 1.000000e+00, float 1.000000e+00, float 1.000000e+00, float 1.000000e+00, float 1.000000e+00, float 1.000000e+00, float 1.000000e+00, float 1.000000e+00, float 1.000000e+00, float 1.000000e+00>, <64 x float> zeroinitializer
	%i68 = shufflevector <64 x float> %i64, <64 x float> %i65, <128 x i32> <i32 0, i32 1, i32 2, i32 3, i32 4, i32 5, i32 6, i32 7, i32 8, i32 9, i32 10, i32 11, i32 12, i32 13, i32 14, i32 15, i32 16, i32 17, i32 18, i32 19, i32 20, i32 21, i32 22, i32 23, i32 24, i32 25, i32 26, i32 27, i32 28, i32 29, i32 30, i32 31, i32 32, i32 33, i32 34, i32 35, i32 36, i32 37, i32 38, i32 39, i32 40, i32 41, i32 42, i32 43, i32 44, i32 45, i32 46, i32 47, i32 48, i32 49, i32 50, i32 51, i32 52, i32 53, i32 54, i32 55, i32 56, i32 57, i32 58, i32 59, i32 60, i32 61, i32 62, i32 63, i32 64, i32 65, i32 66, i32 67, i32 68, i32 69, i32 70, i32 71, i32 72, i32 73, i32 74, i32 75, i32 76, i32 77, i32 78, i32 79, i32 80, i32 81, i32 82, i32 83, i32 84, i32 85, i32 86, i32 87, i32 88, i32 89, i32 90, i32 91, i32 92, i32 93, i32 94, i32 95, i32 96, i32 97, i32 98, i32 99, i32 100, i32 101, i32 102, i32 103, i32 104, i32 105, i32 106, i32 107, i32 108, i32 109, i32 110, i32 111, i32 112, i32 113, i32 114, i32 115, i32 116, i32 117, i32 118, i32 119, i32 120, i32 121, i32 122, i32 123, i32 124, i32 125, i32 126, i32 127>
	%i69 = shufflevector <64 x float> %i66, <64 x float> %i67, <128 x i32> <i32 0, i32 1, i32 2, i32 3, i32 4, i32 5, i32 6, i32 7, i32 8, i32 9, i32 10, i32 11, i32 12, i32 13, i32 14, i32 15, i32 16, i32 17, i32 18, i32 19, i32 20, i32 21, i32 22, i32 23, i32 24, i32 25, i32 26, i32 27, i32 28, i32 29, i32 30, i32 31, i32 32, i32 33, i32 34, i32 35, i32 36, i32 37, i32 38, i32 39, i32 40, i32 41, i32 42, i32 43, i32 44, i32 45, i32 46, i32 47, i32 48, i32 49, i32 50, i32 51, i32 52, i32 53, i32 54, i32 55, i32 56, i32 57, i32 58, i32 59, i32 60, i32 61, i32 62, i32 63, i32 64, i32 65, i32 66, i32 67, i32 68, i32 69, i32 70, i32 71, i32 72, i32 73, i32 74, i32 75, i32 76, i32 77, i32 78, i32 79, i32 80, i32 81, i32 82, i32 83, i32 84, i32 85, i32 86, i32 87, i32 88, i32 89, i32 90, i32 91, i32 92, i32 93, i32 94, i32 95, i32 96, i32 97, i32 98, i32 99, i32 100, i32 101, i32 102, i32 103, i32 104, i32 105, i32 106, i32 107, i32 108, i32 109, i32 110, i32 111, i32 112, i32 113, i32 114, i32 115, i32 116, i32 117, i32 118, i32 119, i32 120, i32 121, i32 122, i32 123, i32 124, i32 125, i32 126, i32 127>
	%i70 = shufflevector <128 x float> %i68, <128 x float> %i69, <256 x i32> <i32 0, i32 1, i32 2, i32 3, i32 4, i32 5, i32 6, i32 7, i32 8, i32 9, i32 10, i32 11, i32 12, i32 13, i32 14, i32 15, i32 16, i32 17, i32 18, i32 19, i32 20, i32 21, i32 22, i32 23, i32 24, i32 25, i32 26, i32 27, i32 28, i32 29, i32 30, i32 31, i32 32, i32 33, i32 34, i32 35, i32 36, i32 37, i32 38, i32 39, i32 40, i32 41, i32 42, i32 43, i32 44, i32 45, i32 46, i32 47, i32 48, i32 49, i32 50, i32 51, i32 52, i32 53, i32 54, i32 55, i32 56, i32 57, i32 58, i32 59, i32 60, i32 61, i32 62, i32 63, i32 64, i32 65, i32 66, i32 67, i32 68, i32 69, i32 70, i32 71, i32 72, i32 73, i32 74, i32 75, i32 76, i32 77, i32 78, i32 79, i32 80, i32 81, i32 82, i32 83, i32 84, i32 85, i32 86, i32 87, i32 88, i32 89, i32 90, i32 91, i32 92, i32 93, i32 94, i32 95, i32 96, i32 97, i32 98, i32 99, i32 100, i32 101, i32 102, i32 103, i32 104, i32 105, i32 106, i32 107, i32 108, i32 109, i32 110, i32 111, i32 112, i32 113, i32 114, i32 115, i32 116, i32 117, i32 118, i32 119, i32 120, i32 121, i32 122, i32 123, i32 124, i32 125, i32 126, i32 127, i32 128, i32 129, i32 130, i32 131, i32 132, i32 133, i32 134, i32 135, i32 136, i32 137, i32 138, i32 139, i32 140, i32 141, i32 142, i32 143, i32 144, i32 145, i32 146, i32 147, i32 148, i32 149, i32 150, i32 151, i32 152, i32 153, i32 154, i32 155, i32 156, i32 157, i32 158, i32 159, i32 160, i32 161, i32 162, i32 163, i32 164, i32 165, i32 166, i32 167, i32 168, i32 169, i32 170, i32 171, i32 172, i32 173, i32 174, i32 175, i32 176, i32 177, i32 178, i32 179, i32 180, i32 181, i32 182, i32 183, i32 184, i32 185, i32 186, i32 187, i32 188, i32 189, i32 190, i32 191, i32 192, i32 193, i32 194, i32 195, i32 196, i32 197, i32 198, i32 199, i32 200, i32 201, i32 202, i32 203, i32 204, i32 205, i32 206, i32 207, i32 208, i32 209, i32 210, i32 211, i32 212, i32 213, i32 214, i32 215, i32 216, i32 217, i32 218, i32 219, i32 220, i32 221, i32 222, i32 223, i32 224, i32 225, i32 226, i32 227, i32 228, i32 229, i32 230, i32 231, i32 232, i32 233, i32 234, i32 235, i32 236, i32 237, i32 238, i32 239, i32 240, i32 241, i32 242, i32 243, i32 244, i32 245, i32 246, i32 247, i32 248, i32 249, i32 250, i32 251, i32 252, i32 253, i32 254, i32 255>
	%i71 = call <256 x i8> @llvm.tpc.fptosi.swch.v256i8.v256f32(<256 x float> %i70, i32 327680)
	%i72 = call <256 x float> @llvm.tpc.sitofp.swch.v256f32.v256i8(<256 x i8> %i71, i32 327680)
	%i73 = shufflevector <256 x float> %i72, <256 x float> undef, <64 x i32> <i32 0, i32 1, i32 2, i32 3, i32 4, i32 5, i32 6, i32 7, i32 8, i32 9, i32 10, i32 11, i32 12, i32 13, i32 14, i32 15, i32 16, i32 17, i32 18, i32 19, i32 20, i32 21, i32 22, i32 23, i32 24, i32 25, i32 26, i32 27, i32 28, i32 29, i32 30, i32 31, i32 32, i32 33, i32 34, i32 35, i32 36, i32 37, i32 38, i32 39, i32 40, i32 41, i32 42, i32 43, i32 44, i32 45, i32 46, i32 47, i32 48, i32 49, i32 50, i32 51, i32 52, i32 53, i32 54, i32 55, i32 56, i32 57, i32 58, i32 59, i32 60, i32 61, i32 62, i32 63>
	%i74 = shufflevector <256 x float> %i72, <256 x float> undef, <64 x i32> <i32 64, i32 65, i32 66, i32 67, i32 68, i32 69, i32 70, i32 71, i32 72, i32 73, i32 74, i32 75, i32 76, i32 77, i32 78, i32 79, i32 80, i32 81, i32 82, i32 83, i32 84, i32 85, i32 86, i32 87, i32 88, i32 89, i32 90, i32 91, i32 92, i32 93, i32 94, i32 95, i32 96, i32 97, i32 98, i32 99, i32 100, i32 101, i32 102, i32 103, i32 104, i32 105, i32 106, i32 107, i32 108, i32 109, i32 110, i32 111, i32 112, i32 113, i32 114, i32 115, i32 116, i32 117, i32 118, i32 119, i32 120, i32 121, i32 122, i32 123, i32 124, i32 125, i32 126, i32 127>
	%i75 = shufflevector <256 x float> %i72, <256 x float> undef, <64 x i32> <i32 128, i32 129, i32 130, i32 131, i32 132, i32 133, i32 134, i32 135, i32 136, i32 137, i32 138, i32 139, i32 140, i32 141, i32 142, i32 143, i32 144, i32 145, i32 146, i32 147, i32 148, i32 149, i32 150, i32 151, i32 152, i32 153, i32 154, i32 155, i32 156, i32 157, i32 158, i32 159, i32 160, i32 161, i32 162, i32 163, i32 164, i32 165, i32 166, i32 167, i32 168, i32 169, i32 170, i32 171, i32 172, i32 173, i32 174, i32 175, i32 176, i32 177, i32 178, i32 179, i32 180, i32 181, i32 182, i32 183, i32 184, i32 185, i32 186, i32 187, i32 188, i32 189, i32 190, i32 191>
	%i76 = shufflevector <256 x float> %i72, <256 x float> undef, <64 x i32> <i32 192, i32 193, i32 194, i32 195, i32 196, i32 197, i32 198, i32 199, i32 200, i32 201, i32 202, i32 203, i32 204, i32 205, i32 206, i32 207, i32 208, i32 209, i32 210, i32 211, i32 212, i32 213, i32 214, i32 215, i32 216, i32 217, i32 218, i32 219, i32 220, i32 221, i32 222, i32 223, i32 224, i32 225, i32 226, i32 227, i32 228, i32 229, i32 230, i32 231, i32 232, i32 233, i32 234, i32 235, i32 236, i32 237, i32 238, i32 239, i32 240, i32 241, i32 242, i32 243, i32 244, i32 245, i32 246, i32 247, i32 248, i32 249, i32 250, i32 251, i32 252, i32 253, i32 254, i32 255>
	%.sroa.027.0.vec.insert = insertelement <5 x i32> %.sroa.027.0, i32 %i42, i32 0
	call void @llvm.tpc.st.tnsr.v64f32(<5 x i32> %.sroa.027.0.vec.insert, i8 2, <64 x float> %i73, i32 0, i1 true, i1 false)
	%.sroa.029.0.vec.insert = insertelement <5 x i32> %.sroa.029.0, i32 %i46, i32 0
	call void @llvm.tpc.st.tnsr.v64f32(<5 x i32> %.sroa.029.0.vec.insert, i8 2, <64 x float> %i74, i32 0, i1 true, i1 false)
	%.sroa.031.0.vec.insert = insertelement <5 x i32> %.sroa.031.0, i32 %i48, i32 0
	call void @llvm.tpc.st.tnsr.v64f32(<5 x i32> %.sroa.031.0.vec.insert, i8 2, <64 x float> %i75, i32 0, i1 true, i1 false)
	%.sroa.033.0.vec.insert = insertelement <5 x i32> %.sroa.033.0, i32 %i50, i32 0
	call void @llvm.tpc.st.tnsr.v64f32(<5 x i32> %.sroa.033.0.vec.insert, i8 2, <64 x float> %i76, i32 0, i1 true, i1 false)
	%i77 = add i32 %i42, 256
	br label %bb41

	bb78:                                             ; preds = %bb41
	ret void
}
