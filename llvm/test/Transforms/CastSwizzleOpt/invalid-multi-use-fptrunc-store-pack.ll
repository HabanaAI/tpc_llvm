; NOTE: Assertions have been autogenerated by utils/update_test_checks.py
; RUN: opt < %s -march=gaudi -cast-swizzle-opt -S | FileCheck %s

; ModuleID = 'fused_kernel'
source_filename = "fused_kernel"
target triple = "tpc"

declare <128 x bfloat16> @llvm.tpc.ld.g.v128bf16(i8 addrspace(3)*, i32, <128 x bfloat16>, i1, i1)

declare <128 x bfloat16> @llvm.tpc.ld.tnsr.v128bf16.i1(<5 x i32>, i8, i32, <128 x bfloat16>, i1, i1)

declare void @llvm.tpc.st.tnsr.v128bf16(<5 x i32>, i8, <128 x bfloat16>, i32, i1, i1)

declare <64 x float> @llvm.tpc.ld.tnsr.v64f32.i1(<5 x i32>, i8, i32, <64 x float>, i1, i1)

declare <128 x bfloat16> @llvm.tpc.fptrunc.swch.v128bf16.v128f32(<128 x float>, i32)

declare i8 addrspace(3)* @llvm.tpc.gen.addr(<5 x i32>, i8, i32, i8 addrspace(3)*, i1, i1)

declare <5 x i32> @llvm.tpc.get.index.space.offset()

declare <5 x i32> @llvm.tpc.get.index.space.size()

define void @main() {
; CHECK-LABEL: @main(
; CHECK-NEXT:  bb:
; CHECK-NEXT:    [[I:%.*]] = call <5 x i32> @llvm.tpc.get.index.space.offset()
; CHECK-NEXT:    [[I20:%.*]] = call <5 x i32> @llvm.tpc.get.index.space.size()
; CHECK-NEXT:    [[I21:%.*]] = add <5 x i32> [[I]], [[I20]]
; CHECK-NEXT:    [[I22:%.*]] = extractelement <5 x i32> [[I]], i32 0
; CHECK-NEXT:    [[I23:%.*]] = shl i32 [[I22]], 7
; CHECK-NEXT:    [[I24:%.*]] = extractelement <5 x i32> [[I21]], i32 0
; CHECK-NEXT:    [[I25:%.*]] = shl i32 [[I24]], 7
; CHECK-NEXT:    [[I26:%.*]] = extractelement <5 x i32> [[I]], i32 1
; CHECK-NEXT:    [[I27:%.*]] = extractelement <5 x i32> [[I21]], i32 1
; CHECK-NEXT:    [[DOTSROA_018_8_VEC_INSERT:%.*]] = insertelement <5 x i32> zeroinitializer, i32 0, i32 2
; CHECK-NEXT:    [[DOTSROA_018_12_VEC_INSERT:%.*]] = insertelement <5 x i32> [[DOTSROA_018_8_VEC_INSERT]], i32 0, i32 3
; CHECK-NEXT:    [[DOTSROA_016_8_VEC_INSERT:%.*]] = insertelement <5 x i32> zeroinitializer, i32 0, i32 2
; CHECK-NEXT:    [[DOTSROA_016_12_VEC_INSERT:%.*]] = insertelement <5 x i32> [[DOTSROA_016_8_VEC_INSERT]], i32 0, i32 3
; CHECK-NEXT:    [[DOTSROA_014_8_VEC_INSERT:%.*]] = insertelement <5 x i32> zeroinitializer, i32 0, i32 2
; CHECK-NEXT:    [[DOTSROA_014_12_VEC_INSERT:%.*]] = insertelement <5 x i32> [[DOTSROA_014_8_VEC_INSERT]], i32 0, i32 3
; CHECK-NEXT:    [[DOTSROA_012_8_VEC_INSERT:%.*]] = insertelement <5 x i32> zeroinitializer, i32 0, i32 2
; CHECK-NEXT:    [[DOTSROA_012_12_VEC_INSERT:%.*]] = insertelement <5 x i32> [[DOTSROA_012_8_VEC_INSERT]], i32 0, i32 3
; CHECK-NEXT:    [[DOTSROA_010_8_VEC_INSERT:%.*]] = insertelement <5 x i32> zeroinitializer, i32 0, i32 2
; CHECK-NEXT:    [[DOTSROA_010_12_VEC_INSERT:%.*]] = insertelement <5 x i32> [[DOTSROA_010_8_VEC_INSERT]], i32 0, i32 3
; CHECK-NEXT:    [[DOTSROA_08_8_VEC_INSERT:%.*]] = insertelement <5 x i32> zeroinitializer, i32 0, i32 2
; CHECK-NEXT:    [[DOTSROA_08_12_VEC_INSERT:%.*]] = insertelement <5 x i32> [[DOTSROA_08_8_VEC_INSERT]], i32 0, i32 3
; CHECK-NEXT:    [[DOTSROA_0_12_VEC_INSERT:%.*]] = insertelement <5 x i32> zeroinitializer, i32 0, i32 3
; CHECK-NEXT:    [[DOTSROA_0_8_VEC_INSERT:%.*]] = insertelement <5 x i32> [[DOTSROA_0_12_VEC_INSERT]], i32 0, i32 2
; CHECK-NEXT:    [[DOTSROA_0_4_VEC_INSERT:%.*]] = insertelement <5 x i32> [[DOTSROA_0_8_VEC_INSERT]], i32 0, i32 1
; CHECK-NEXT:    [[DOTSROA_0_0_VEC_INSERT:%.*]] = insertelement <5 x i32> [[DOTSROA_0_4_VEC_INSERT]], i32 0, i32 0
; CHECK-NEXT:    [[I28:%.*]] = call i8 addrspace(3)* @llvm.tpc.gen.addr(<5 x i32> [[DOTSROA_0_0_VEC_INSERT]], i8 1, i32 0, i8 addrspace(3)* undef, i1 true, i1 false)
; CHECK-NEXT:    [[I29:%.*]] = call <128 x bfloat> @llvm.tpc.ld.g.v128bf16.i1(i8 addrspace(3)* [[I28]], i32 0, <128 x bfloat> undef, i1 true, i1 false)
; CHECK-NEXT:    br label [[BB30:%.*]]
; CHECK:       bb30:
; CHECK-NEXT:    [[DOTSROA_016_0:%.*]] = phi <5 x i32> [ [[DOTSROA_016_12_VEC_INSERT]], [[BB:%.*]] ], [ [[DOTSROA_016_1:%.*]], [[BB49:%.*]] ]
; CHECK-NEXT:    [[DOTSROA_014_0:%.*]] = phi <5 x i32> [ [[DOTSROA_014_12_VEC_INSERT]], [[BB]] ], [ [[DOTSROA_014_1:%.*]], [[BB49]] ]
; CHECK-NEXT:    [[DOTSROA_012_0:%.*]] = phi <5 x i32> [ [[DOTSROA_012_12_VEC_INSERT]], [[BB]] ], [ [[DOTSROA_012_1:%.*]], [[BB49]] ]
; CHECK-NEXT:    [[DOTSROA_010_0:%.*]] = phi <5 x i32> [ [[DOTSROA_010_12_VEC_INSERT]], [[BB]] ], [ [[DOTSROA_010_1:%.*]], [[BB49]] ]
; CHECK-NEXT:    [[DOTSROA_08_0:%.*]] = phi <5 x i32> [ [[DOTSROA_08_12_VEC_INSERT]], [[BB]] ], [ [[DOTSROA_08_1:%.*]], [[BB49]] ]
; CHECK-NEXT:    [[DOTSROA_018_0:%.*]] = phi <5 x i32> [ [[DOTSROA_018_12_VEC_INSERT]], [[BB]] ], [ [[DOTSROA_018_1:%.*]], [[BB49]] ]
; CHECK-NEXT:    [[I31:%.*]] = phi i32 [ [[I50:%.*]], [[BB49]] ], [ [[I26]], [[BB]] ]
; CHECK-NEXT:    [[I32:%.*]] = icmp slt i32 [[I31]], [[I27]]
; CHECK-NEXT:    br i1 [[I32]], label [[BB33:%.*]], label [[BB51:%.*]]
; CHECK:       bb33:
; CHECK-NEXT:    [[DOTSROA_018_4_VEC_INSERT:%.*]] = insertelement <5 x i32> [[DOTSROA_018_0]], i32 [[I31]], i32 1
; CHECK-NEXT:    [[DOTSROA_016_4_VEC_INSERT:%.*]] = insertelement <5 x i32> [[DOTSROA_016_0]], i32 [[I31]], i32 1
; CHECK-NEXT:    [[DOTSROA_014_4_VEC_INSERT:%.*]] = insertelement <5 x i32> [[DOTSROA_014_0]], i32 [[I31]], i32 1
; CHECK-NEXT:    [[DOTSROA_012_4_VEC_INSERT:%.*]] = insertelement <5 x i32> [[DOTSROA_012_0]], i32 [[I31]], i32 1
; CHECK-NEXT:    [[DOTSROA_010_4_VEC_INSERT:%.*]] = insertelement <5 x i32> [[DOTSROA_010_0]], i32 [[I31]], i32 1
; CHECK-NEXT:    [[DOTSROA_08_4_VEC_INSERT:%.*]] = insertelement <5 x i32> [[DOTSROA_08_0]], i32 [[I31]], i32 1
; CHECK-NEXT:    br label [[BB34:%.*]]
; CHECK:       bb34:
; CHECK-NEXT:    [[DOTSROA_016_1]] = phi <5 x i32> [ [[DOTSROA_016_4_VEC_INSERT]], [[BB33]] ], [ [[DOTSROA_016_0_VEC_INSERT:%.*]], [[BB37:%.*]] ]
; CHECK-NEXT:    [[DOTSROA_014_1]] = phi <5 x i32> [ [[DOTSROA_014_4_VEC_INSERT]], [[BB33]] ], [ [[DOTSROA_014_0_VEC_INSERT:%.*]], [[BB37]] ]
; CHECK-NEXT:    [[DOTSROA_012_1]] = phi <5 x i32> [ [[DOTSROA_012_4_VEC_INSERT]], [[BB33]] ], [ [[DOTSROA_012_0_VEC_INSERT:%.*]], [[BB37]] ]
; CHECK-NEXT:    [[DOTSROA_010_1]] = phi <5 x i32> [ [[DOTSROA_010_4_VEC_INSERT]], [[BB33]] ], [ [[DOTSROA_010_0_VEC_INSERT:%.*]], [[BB37]] ]
; CHECK-NEXT:    [[DOTSROA_08_1]] = phi <5 x i32> [ [[DOTSROA_08_4_VEC_INSERT]], [[BB33]] ], [ [[DOTSROA_08_0_VEC_INSERT:%.*]], [[BB37]] ]
; CHECK-NEXT:    [[DOTSROA_018_1]] = phi <5 x i32> [ [[DOTSROA_018_4_VEC_INSERT]], [[BB33]] ], [ [[DOTSROA_018_0_VEC_INSERT:%.*]], [[BB37]] ]
; CHECK-NEXT:    [[I35:%.*]] = phi i32 [ [[I48:%.*]], [[BB37]] ], [ [[I23]], [[BB33]] ]
; CHECK-NEXT:    [[I36:%.*]] = icmp slt i32 [[I35]], [[I25]]
; CHECK-NEXT:    br i1 [[I36]], label [[BB37]], label [[BB49]]
; CHECK:       bb37:
; CHECK-NEXT:    [[DOTSROA_018_0_VEC_INSERT]] = insertelement <5 x i32> [[DOTSROA_018_1]], i32 [[I35]], i32 0
; CHECK-NEXT:    [[DOTSROA_016_0_VEC_INSERT]] = insertelement <5 x i32> [[DOTSROA_016_1]], i32 [[I35]], i32 0
; CHECK-NEXT:    [[DOTSROA_014_0_VEC_INSERT]] = insertelement <5 x i32> [[DOTSROA_014_1]], i32 [[I35]], i32 0
; CHECK-NEXT:    [[DOTSROA_012_0_VEC_INSERT]] = insertelement <5 x i32> [[DOTSROA_012_1]], i32 [[I35]], i32 0
; CHECK-NEXT:    [[DOTSROA_08_0_VEC_INSERT]] = insertelement <5 x i32> [[DOTSROA_08_1]], i32 [[I35]], i32 0
; CHECK-NEXT:    [[I38:%.*]] = call <64 x float> @llvm.tpc.ld.tnsr.v64f32.i1(<5 x i32> [[DOTSROA_08_0_VEC_INSERT]], i8 0, i32 0, <64 x float> undef, i1 true, i1 false)
; CHECK-NEXT:    [[I39:%.*]] = or i32 [[I35]], 64
; CHECK-NEXT:    [[DOTSROA_010_0_VEC_INSERT]] = insertelement <5 x i32> [[DOTSROA_010_1]], i32 [[I39]], i32 0
; CHECK-NEXT:    [[I40:%.*]] = call <64 x float> @llvm.tpc.ld.tnsr.v64f32.i1(<5 x i32> [[DOTSROA_010_0_VEC_INSERT]], i8 0, i32 0, <64 x float> undef, i1 true, i1 false)
; CHECK-NEXT:    [[I41:%.*]] = shufflevector <64 x float> [[I38]], <64 x float> [[I40]], <128 x i32> <i32 0, i32 1, i32 2, i32 3, i32 4, i32 5, i32 6, i32 7, i32 8, i32 9, i32 10, i32 11, i32 12, i32 13, i32 14, i32 15, i32 16, i32 17, i32 18, i32 19, i32 20, i32 21, i32 22, i32 23, i32 24, i32 25, i32 26, i32 27, i32 28, i32 29, i32 30, i32 31, i32 32, i32 33, i32 34, i32 35, i32 36, i32 37, i32 38, i32 39, i32 40, i32 41, i32 42, i32 43, i32 44, i32 45, i32 46, i32 47, i32 48, i32 49, i32 50, i32 51, i32 52, i32 53, i32 54, i32 55, i32 56, i32 57, i32 58, i32 59, i32 60, i32 61, i32 62, i32 63, i32 64, i32 65, i32 66, i32 67, i32 68, i32 69, i32 70, i32 71, i32 72, i32 73, i32 74, i32 75, i32 76, i32 77, i32 78, i32 79, i32 80, i32 81, i32 82, i32 83, i32 84, i32 85, i32 86, i32 87, i32 88, i32 89, i32 90, i32 91, i32 92, i32 93, i32 94, i32 95, i32 96, i32 97, i32 98, i32 99, i32 100, i32 101, i32 102, i32 103, i32 104, i32 105, i32 106, i32 107, i32 108, i32 109, i32 110, i32 111, i32 112, i32 113, i32 114, i32 115, i32 116, i32 117, i32 118, i32 119, i32 120, i32 121, i32 122, i32 123, i32 124, i32 125, i32 126, i32 127>
; CHECK-NEXT:    [[I42:%.*]] = call <128 x bfloat> @llvm.tpc.fptrunc.swch.v128bf16.v128f32(<128 x float> [[I41]], i32 0)
; CHECK-NEXT:    call void @llvm.tpc.st.tnsr.v128bf16(<5 x i32> [[DOTSROA_012_0_VEC_INSERT]], i8 5, <128 x bfloat> [[I42]], i32 0, i1 true, i1 false)
; CHECK-NEXT:    [[I43:%.*]] = fmul <128 x bfloat> [[I29]], [[I42]]
; CHECK-NEXT:    [[I44:%.*]] = call <128 x bfloat> @llvm.tpc.ld.tnsr.v128bf16.i1(<5 x i32> [[DOTSROA_014_0_VEC_INSERT]], i8 2, i32 0, <128 x bfloat> undef, i1 true, i1 false)
; CHECK-NEXT:    [[I45:%.*]] = fmul <128 x bfloat> [[I43]], [[I44]]
; CHECK-NEXT:    [[I46:%.*]] = call <128 x bfloat> @llvm.tpc.ld.tnsr.v128bf16.i1(<5 x i32> [[DOTSROA_016_0_VEC_INSERT]], i8 3, i32 0, <128 x bfloat> undef, i1 true, i1 false)
; CHECK-NEXT:    [[I47:%.*]] = fadd <128 x bfloat> [[I45]], [[I46]]
; CHECK-NEXT:    call void @llvm.tpc.st.tnsr.v128bf16(<5 x i32> [[DOTSROA_018_0_VEC_INSERT]], i8 4, <128 x bfloat> [[I47]], i32 0, i1 true, i1 false)
; CHECK-NEXT:    [[I48]] = add i32 [[I35]], 128
; CHECK-NEXT:    br label [[BB34]]
; CHECK:       bb49:
; CHECK-NEXT:    [[I50]] = add i32 [[I31]], 1
; CHECK-NEXT:    br label [[BB30]]
; CHECK:       bb51:
; CHECK-NEXT:    ret void
;
bb:
  %i = call <5 x i32> @llvm.tpc.get.index.space.offset()
  %i20 = call <5 x i32> @llvm.tpc.get.index.space.size()
  %i21 = add <5 x i32> %i, %i20
  %i22 = extractelement <5 x i32> %i, i32 0
  %i23 = shl i32 %i22, 7
  %i24 = extractelement <5 x i32> %i21, i32 0
  %i25 = shl i32 %i24, 7
  %i26 = extractelement <5 x i32> %i, i32 1
  %i27 = extractelement <5 x i32> %i21, i32 1
  %.sroa.018.8.vec.insert = insertelement <5 x i32> zeroinitializer, i32 0, i32 2
  %.sroa.018.12.vec.insert = insertelement <5 x i32> %.sroa.018.8.vec.insert, i32 0, i32 3
  %.sroa.016.8.vec.insert = insertelement <5 x i32> zeroinitializer, i32 0, i32 2
  %.sroa.016.12.vec.insert = insertelement <5 x i32> %.sroa.016.8.vec.insert, i32 0, i32 3
  %.sroa.014.8.vec.insert = insertelement <5 x i32> zeroinitializer, i32 0, i32 2
  %.sroa.014.12.vec.insert = insertelement <5 x i32> %.sroa.014.8.vec.insert, i32 0, i32 3
  %.sroa.012.8.vec.insert = insertelement <5 x i32> zeroinitializer, i32 0, i32 2
  %.sroa.012.12.vec.insert = insertelement <5 x i32> %.sroa.012.8.vec.insert, i32 0, i32 3
  %.sroa.010.8.vec.insert = insertelement <5 x i32> zeroinitializer, i32 0, i32 2
  %.sroa.010.12.vec.insert = insertelement <5 x i32> %.sroa.010.8.vec.insert, i32 0, i32 3
  %.sroa.08.8.vec.insert = insertelement <5 x i32> zeroinitializer, i32 0, i32 2
  %.sroa.08.12.vec.insert = insertelement <5 x i32> %.sroa.08.8.vec.insert, i32 0, i32 3
  %.sroa.0.12.vec.insert = insertelement <5 x i32> zeroinitializer, i32 0, i32 3
  %.sroa.0.8.vec.insert = insertelement <5 x i32> %.sroa.0.12.vec.insert, i32 0, i32 2
  %.sroa.0.4.vec.insert = insertelement <5 x i32> %.sroa.0.8.vec.insert, i32 0, i32 1
  %.sroa.0.0.vec.insert = insertelement <5 x i32> %.sroa.0.4.vec.insert, i32 0, i32 0
  %i28 = call i8 addrspace(3)* @llvm.tpc.gen.addr(<5 x i32> %.sroa.0.0.vec.insert, i8 1, i32 0, i8 addrspace(3)* undef, i1 true, i1 false)
  %i29 = call <128 x bfloat16> @llvm.tpc.ld.g.v128bf16(i8 addrspace(3)* %i28, i32 0, <128 x bfloat16> undef, i1 true, i1 false)
  br label %bb30

bb30:                                             ; preds = %bb49, %bb
  %.sroa.016.0 = phi <5 x i32> [ %.sroa.016.12.vec.insert, %bb ], [ %.sroa.016.1, %bb49 ]
  %.sroa.014.0 = phi <5 x i32> [ %.sroa.014.12.vec.insert, %bb ], [ %.sroa.014.1, %bb49 ]
  %.sroa.012.0 = phi <5 x i32> [ %.sroa.012.12.vec.insert, %bb ], [ %.sroa.012.1, %bb49 ]
  %.sroa.010.0 = phi <5 x i32> [ %.sroa.010.12.vec.insert, %bb ], [ %.sroa.010.1, %bb49 ]
  %.sroa.08.0 = phi <5 x i32> [ %.sroa.08.12.vec.insert, %bb ], [ %.sroa.08.1, %bb49 ]
  %.sroa.018.0 = phi <5 x i32> [ %.sroa.018.12.vec.insert, %bb ], [ %.sroa.018.1, %bb49 ]
  %i31 = phi i32 [ %i50, %bb49 ], [ %i26, %bb ]
  %i32 = icmp slt i32 %i31, %i27
  br i1 %i32, label %bb33, label %bb51

bb33:                                             ; preds = %bb30
  %.sroa.018.4.vec.insert = insertelement <5 x i32> %.sroa.018.0, i32 %i31, i32 1
  %.sroa.016.4.vec.insert = insertelement <5 x i32> %.sroa.016.0, i32 %i31, i32 1
  %.sroa.014.4.vec.insert = insertelement <5 x i32> %.sroa.014.0, i32 %i31, i32 1
  %.sroa.012.4.vec.insert = insertelement <5 x i32> %.sroa.012.0, i32 %i31, i32 1
  %.sroa.010.4.vec.insert = insertelement <5 x i32> %.sroa.010.0, i32 %i31, i32 1
  %.sroa.08.4.vec.insert = insertelement <5 x i32> %.sroa.08.0, i32 %i31, i32 1
  br label %bb34

bb34:                                             ; preds = %bb37, %bb33
  %.sroa.016.1 = phi <5 x i32> [ %.sroa.016.4.vec.insert, %bb33 ], [ %.sroa.016.0.vec.insert, %bb37 ]
  %.sroa.014.1 = phi <5 x i32> [ %.sroa.014.4.vec.insert, %bb33 ], [ %.sroa.014.0.vec.insert, %bb37 ]
  %.sroa.012.1 = phi <5 x i32> [ %.sroa.012.4.vec.insert, %bb33 ], [ %.sroa.012.0.vec.insert, %bb37 ]
  %.sroa.010.1 = phi <5 x i32> [ %.sroa.010.4.vec.insert, %bb33 ], [ %.sroa.010.0.vec.insert, %bb37 ]
  %.sroa.08.1 = phi <5 x i32> [ %.sroa.08.4.vec.insert, %bb33 ], [ %.sroa.08.0.vec.insert, %bb37 ]
  %.sroa.018.1 = phi <5 x i32> [ %.sroa.018.4.vec.insert, %bb33 ], [ %.sroa.018.0.vec.insert, %bb37 ]
  %i35 = phi i32 [ %i48, %bb37 ], [ %i23, %bb33 ]
  %i36 = icmp slt i32 %i35, %i25
  br i1 %i36, label %bb37, label %bb49

bb37:                                             ; preds = %bb34
  %.sroa.018.0.vec.insert = insertelement <5 x i32> %.sroa.018.1, i32 %i35, i32 0
  %.sroa.016.0.vec.insert = insertelement <5 x i32> %.sroa.016.1, i32 %i35, i32 0
  %.sroa.014.0.vec.insert = insertelement <5 x i32> %.sroa.014.1, i32 %i35, i32 0
  %.sroa.012.0.vec.insert = insertelement <5 x i32> %.sroa.012.1, i32 %i35, i32 0
  %.sroa.08.0.vec.insert = insertelement <5 x i32> %.sroa.08.1, i32 %i35, i32 0
  %i38 = call <64 x float> @llvm.tpc.ld.tnsr.v64f32.i1(<5 x i32> %.sroa.08.0.vec.insert, i8 0, i32 0, <64 x float> undef, i1 true, i1 false)
  %i39 = or i32 %i35, 64
  %.sroa.010.0.vec.insert = insertelement <5 x i32> %.sroa.010.1, i32 %i39, i32 0
  %i40 = call <64 x float> @llvm.tpc.ld.tnsr.v64f32.i1(<5 x i32> %.sroa.010.0.vec.insert, i8 0, i32 0, <64 x float> undef, i1 true, i1 false)
  %i41 = shufflevector <64 x float> %i38, <64 x float> %i40, <128 x i32> <i32 0, i32 1, i32 2, i32 3, i32 4, i32 5, i32 6, i32 7, i32 8, i32 9, i32 10, i32 11, i32 12, i32 13, i32 14, i32 15, i32 16, i32 17, i32 18, i32 19, i32 20, i32 21, i32 22, i32 23, i32 24, i32 25, i32 26, i32 27, i32 28, i32 29, i32 30, i32 31, i32 32, i32 33, i32 34, i32 35, i32 36, i32 37, i32 38, i32 39, i32 40, i32 41, i32 42, i32 43, i32 44, i32 45, i32 46, i32 47, i32 48, i32 49, i32 50, i32 51, i32 52, i32 53, i32 54, i32 55, i32 56, i32 57, i32 58, i32 59, i32 60, i32 61, i32 62, i32 63, i32 64, i32 65, i32 66, i32 67, i32 68, i32 69, i32 70, i32 71, i32 72, i32 73, i32 74, i32 75, i32 76, i32 77, i32 78, i32 79, i32 80, i32 81, i32 82, i32 83, i32 84, i32 85, i32 86, i32 87, i32 88, i32 89, i32 90, i32 91, i32 92, i32 93, i32 94, i32 95, i32 96, i32 97, i32 98, i32 99, i32 100, i32 101, i32 102, i32 103, i32 104, i32 105, i32 106, i32 107, i32 108, i32 109, i32 110, i32 111, i32 112, i32 113, i32 114, i32 115, i32 116, i32 117, i32 118, i32 119, i32 120, i32 121, i32 122, i32 123, i32 124, i32 125, i32 126, i32 127>
  %i42 = call <128 x bfloat16> @llvm.tpc.fptrunc.swch.v128bf16.v128f32(<128 x float> %i41, i32 0)
  call void @llvm.tpc.st.tnsr.v128bf16(<5 x i32> %.sroa.012.0.vec.insert, i8 5, <128 x bfloat16> %i42, i32 0, i1 true, i1 false)
  %i43 = fmul <128 x bfloat16> %i29, %i42
  %i44 = call <128 x bfloat16> @llvm.tpc.ld.tnsr.v128bf16.i1(<5 x i32> %.sroa.014.0.vec.insert, i8 2, i32 0, <128 x bfloat16> undef, i1 true, i1 false)
  %i45 = fmul <128 x bfloat16> %i43, %i44
  %i46 = call <128 x bfloat16> @llvm.tpc.ld.tnsr.v128bf16.i1(<5 x i32> %.sroa.016.0.vec.insert, i8 3, i32 0, <128 x bfloat16> undef, i1 true, i1 false)
  %i47 = fadd <128 x bfloat16> %i45, %i46
  call void @llvm.tpc.st.tnsr.v128bf16(<5 x i32> %.sroa.018.0.vec.insert, i8 4, <128 x bfloat16> %i47, i32 0, i1 true, i1 false)
  %i48 = add i32 %i35, 128
  br label %bb34

bb49:                                             ; preds = %bb34
  %i50 = add i32 %i31, 1
  br label %bb30

bb51:                                             ; preds = %bb30
  ret void
}