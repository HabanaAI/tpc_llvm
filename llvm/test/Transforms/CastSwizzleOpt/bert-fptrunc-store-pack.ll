; NOTE: Assertions have been autogenerated by utils/update_test_checks.py
; RUN: opt < %s -cast-swizzle-opt -aggressive-cast-swizzle -S | FileCheck %s
;XFAIL:*

; ModuleID = 'fused_kernel'
source_filename = "fused_kernel"
target triple = "tpc"

; Function Attrs: nounwind readnone
declare <128 x bfloat> @llvm.tpc.ld.tnsr.v128bf16.i1(<5 x i32>, i8, i32, <128 x bfloat>, i1, i1) #0

; Function Attrs: nounwind writeonly
declare void @llvm.tpc.st.tnsr.v128bf16(<5 x i32>, i8, <128 x bfloat>, i32, i1, i1) #1

; Function Attrs: nounwind
declare <128 x bfloat> @llvm.tpc.tanh.v128bf16(<128 x bfloat>) #2

; Function Attrs: nounwind readnone
declare <64 x float> @llvm.tpc.ld.tnsr.v64f32.i1(<5 x i32>, i8, i32, <64 x float>, i1, i1) #0

; Function Attrs: nounwind readnone
declare <128 x bfloat> @llvm.tpc.fptrunc.swch.v128bf16.v128f32(<128 x float>, i32) #0

; Function Attrs: nounwind readnone
declare <128 x float> @llvm.tpc.fpext.swch.v128f32.v128bf16(<128 x bfloat>, i32) #0

; Function Attrs: nounwind readnone
declare i8 addrspace(3)* @llvm.tpc.gen.addr(<5 x i32>, i8, i32, i8 addrspace(3)*, i1, i1) #0

; Function Attrs: nounwind readnone
declare <5 x i32> @llvm.tpc.get.index.space.offset() #0

; Function Attrs: nounwind readnone
declare <5 x i32> @llvm.tpc.get.index.space.size() #0

define void @main() {
; CHECK-LABEL: @main(
; CHECK-NEXT:  bb:
; CHECK-NEXT:    [[I:%.*]] = call <5 x i32> @llvm.tpc.get.index.space.offset()
; CHECK-NEXT:    [[I41:%.*]] = call <5 x i32> @llvm.tpc.get.index.space.size()
; CHECK-NEXT:    [[I42:%.*]] = add <5 x i32> [[I]], [[I41]]
; CHECK-NEXT:    [[I43:%.*]] = extractelement <5 x i32> [[I]], i32 0
; CHECK-NEXT:    [[I44:%.*]] = shl i32 [[I43]], 7
; CHECK-NEXT:    [[I45:%.*]] = extractelement <5 x i32> [[I42]], i32 0
; CHECK-NEXT:    [[I46:%.*]] = shl i32 [[I45]], 7
; CHECK-NEXT:    [[I47:%.*]] = extractelement <5 x i32> [[I]], i32 1
; CHECK-NEXT:    [[I48:%.*]] = extractelement <5 x i32> [[I42]], i32 1
; CHECK-NEXT:    [[DOTSROA_0_4_VEC_INSERT:%.*]] = insertelement <5 x i32> zeroinitializer, i32 0, i32 1
; CHECK-NEXT:    [[DOTSROA_0_0_VEC_INSERT:%.*]] = insertelement <5 x i32> [[DOTSROA_0_4_VEC_INSERT]], i32 0, i32 0
; CHECK-NEXT:    [[I49:%.*]] = call i8 addrspace(3)* @llvm.tpc.gen.addr(<5 x i32> [[DOTSROA_0_0_VEC_INSERT]], i8 1, i32 0, i8 addrspace(3)* undef, i1 true, i1 false)
; CHECK-NEXT:    [[I50:%.*]] = call <64 x float> @llvm.tpc.ld.g.v64f32.i1(i8 addrspace(3)* [[I49]], i32 0, <64 x float> undef, i1 true, i1 false)
; CHECK-NEXT:    [[I51:%.*]] = call <64 x float> @llvm.tpc.ld.g.v64f32.i1(i8 addrspace(3)* [[I49]], i32 0, <64 x float> undef, i1 true, i1 false)
; CHECK-NEXT:    [[DOTSROA_015_4_VEC_INSERT:%.*]] = insertelement <5 x i32> zeroinitializer, i32 0, i32 1
; CHECK-NEXT:    [[DOTSROA_015_0_VEC_INSERT:%.*]] = insertelement <5 x i32> [[DOTSROA_015_4_VEC_INSERT]], i32 0, i32 0
; CHECK-NEXT:    [[I52:%.*]] = call i8 addrspace(3)* @llvm.tpc.gen.addr(<5 x i32> [[DOTSROA_015_0_VEC_INSERT]], i8 3, i32 0, i8 addrspace(3)* undef, i1 true, i1 false)
; CHECK-NEXT:    [[I53:%.*]] = call <64 x float> @llvm.tpc.ld.g.v64f32.i1(i8 addrspace(3)* [[I52]], i32 0, <64 x float> undef, i1 true, i1 false)
; CHECK-NEXT:    [[I54:%.*]] = call <64 x float> @llvm.tpc.ld.g.v64f32.i1(i8 addrspace(3)* [[I52]], i32 0, <64 x float> undef, i1 true, i1 false)
; CHECK-NEXT:    [[DOTSROA_017_4_VEC_INSERT:%.*]] = insertelement <5 x i32> zeroinitializer, i32 0, i32 1
; CHECK-NEXT:    [[DOTSROA_017_0_VEC_INSERT:%.*]] = insertelement <5 x i32> [[DOTSROA_017_4_VEC_INSERT]], i32 0, i32 0
; CHECK-NEXT:    [[I55:%.*]] = call i8 addrspace(3)* @llvm.tpc.gen.addr(<5 x i32> [[DOTSROA_017_0_VEC_INSERT]], i8 4, i32 0, i8 addrspace(3)* undef, i1 true, i1 false)
; CHECK-NEXT:    [[I56:%.*]] = call <128 x bfloat> @llvm.tpc.ld.g.v128bf16.i1(i8 addrspace(3)* [[I55]], i32 0, <128 x bfloat> undef, i1 true, i1 false)
; CHECK-NEXT:    [[DOTSROA_019_4_VEC_INSERT:%.*]] = insertelement <5 x i32> zeroinitializer, i32 0, i32 1
; CHECK-NEXT:    [[DOTSROA_019_0_VEC_INSERT:%.*]] = insertelement <5 x i32> [[DOTSROA_019_4_VEC_INSERT]], i32 0, i32 0
; CHECK-NEXT:    [[I57:%.*]] = call i8 addrspace(3)* @llvm.tpc.gen.addr(<5 x i32> [[DOTSROA_019_0_VEC_INSERT]], i8 5, i32 0, i8 addrspace(3)* undef, i1 true, i1 false)
; CHECK-NEXT:    [[I58:%.*]] = call <64 x float> @llvm.tpc.ld.g.v64f32.i1(i8 addrspace(3)* [[I57]], i32 0, <64 x float> undef, i1 true, i1 false)
; CHECK-NEXT:    [[I59:%.*]] = call <64 x float> @llvm.tpc.ld.g.v64f32.i1(i8 addrspace(3)* [[I57]], i32 0, <64 x float> undef, i1 true, i1 false)
; CHECK-NEXT:    br label [[BB60:%.*]]
; CHECK:       bb60:
; CHECK-NEXT:    [[DOTSROA_031_0:%.*]] = phi <5 x i32> [ zeroinitializer, [[BB:%.*]] ], [ [[DOTSROA_031_1:%.*]], [[BB106:%.*]] ]
; CHECK-NEXT:    [[DOTSROA_029_0:%.*]] = phi <5 x i32> [ zeroinitializer, [[BB]] ], [ [[DOTSROA_029_1:%.*]], [[BB106]] ]
; CHECK-NEXT:    [[DOTSROA_027_0:%.*]] = phi <5 x i32> [ zeroinitializer, [[BB]] ], [ [[DOTSROA_027_1:%.*]], [[BB106]] ]
; CHECK-NEXT:    [[DOTSROA_025_0:%.*]] = phi <5 x i32> [ zeroinitializer, [[BB]] ], [ [[DOTSROA_025_1:%.*]], [[BB106]] ]
; CHECK-NEXT:    [[DOTSROA_023_0:%.*]] = phi <5 x i32> [ zeroinitializer, [[BB]] ], [ [[DOTSROA_023_1:%.*]], [[BB106]] ]
; CHECK-NEXT:    [[DOTSROA_021_0:%.*]] = phi <5 x i32> [ zeroinitializer, [[BB]] ], [ [[DOTSROA_021_1:%.*]], [[BB106]] ]
; CHECK-NEXT:    [[DOTSROA_033_0:%.*]] = phi <5 x i32> [ zeroinitializer, [[BB]] ], [ [[DOTSROA_033_1:%.*]], [[BB106]] ]
; CHECK-NEXT:    [[DOTSROA_035_0:%.*]] = phi <5 x i32> [ zeroinitializer, [[BB]] ], [ [[DOTSROA_035_1:%.*]], [[BB106]] ]
; CHECK-NEXT:    [[DOTSROA_037_0:%.*]] = phi <5 x i32> [ zeroinitializer, [[BB]] ], [ [[DOTSROA_037_1:%.*]], [[BB106]] ]
; CHECK-NEXT:    [[DOTSROA_039_0:%.*]] = phi <5 x i32> [ zeroinitializer, [[BB]] ], [ [[DOTSROA_039_1:%.*]], [[BB106]] ]
; CHECK-NEXT:    [[I61:%.*]] = phi i32 [ [[I107:%.*]], [[BB106]] ], [ [[I47]], [[BB]] ]
; CHECK-NEXT:    [[I62:%.*]] = icmp slt i32 [[I61]], [[I48]]
; CHECK-NEXT:    br i1 [[I62]], label [[BB63:%.*]], label [[BB108:%.*]]
; CHECK:       bb63:
; CHECK-NEXT:    [[DOTSROA_039_4_VEC_INSERT:%.*]] = insertelement <5 x i32> [[DOTSROA_039_0]], i32 [[I61]], i32 1
; CHECK-NEXT:    [[DOTSROA_037_4_VEC_INSERT:%.*]] = insertelement <5 x i32> [[DOTSROA_037_0]], i32 [[I61]], i32 1
; CHECK-NEXT:    [[DOTSROA_035_4_VEC_INSERT:%.*]] = insertelement <5 x i32> [[DOTSROA_035_0]], i32 [[I61]], i32 1
; CHECK-NEXT:    [[DOTSROA_033_4_VEC_INSERT:%.*]] = insertelement <5 x i32> [[DOTSROA_033_0]], i32 [[I61]], i32 1
; CHECK-NEXT:    [[DOTSROA_031_4_VEC_INSERT:%.*]] = insertelement <5 x i32> [[DOTSROA_031_0]], i32 [[I61]], i32 1
; CHECK-NEXT:    [[DOTSROA_029_4_VEC_INSERT:%.*]] = insertelement <5 x i32> [[DOTSROA_029_0]], i32 [[I61]], i32 1
; CHECK-NEXT:    [[DOTSROA_027_4_VEC_INSERT:%.*]] = insertelement <5 x i32> [[DOTSROA_027_0]], i32 [[I61]], i32 1
; CHECK-NEXT:    [[DOTSROA_025_4_VEC_INSERT:%.*]] = insertelement <5 x i32> [[DOTSROA_025_0]], i32 [[I61]], i32 1
; CHECK-NEXT:    [[DOTSROA_023_4_VEC_INSERT:%.*]] = insertelement <5 x i32> [[DOTSROA_023_0]], i32 [[I61]], i32 1
; CHECK-NEXT:    [[DOTSROA_021_4_VEC_INSERT:%.*]] = insertelement <5 x i32> [[DOTSROA_021_0]], i32 [[I61]], i32 1
; CHECK-NEXT:    br label [[BB64:%.*]]
; CHECK:       bb64:
; CHECK-NEXT:    [[DOTSROA_031_1]] = phi <5 x i32> [ [[DOTSROA_031_4_VEC_INSERT]], [[BB63]] ], [ [[DOTSROA_031_0_VEC_INSERT:%.*]], [[BB67:%.*]] ]
; CHECK-NEXT:    [[DOTSROA_029_1]] = phi <5 x i32> [ [[DOTSROA_029_4_VEC_INSERT]], [[BB63]] ], [ [[DOTSROA_029_0_VEC_INSERT:%.*]], [[BB67]] ]
; CHECK-NEXT:    [[DOTSROA_027_1]] = phi <5 x i32> [ [[DOTSROA_027_4_VEC_INSERT]], [[BB63]] ], [ [[DOTSROA_027_0_VEC_INSERT:%.*]], [[BB67]] ]
; CHECK-NEXT:    [[DOTSROA_025_1]] = phi <5 x i32> [ [[DOTSROA_025_4_VEC_INSERT]], [[BB63]] ], [ [[DOTSROA_025_0_VEC_INSERT:%.*]], [[BB67]] ]
; CHECK-NEXT:    [[DOTSROA_023_1]] = phi <5 x i32> [ [[DOTSROA_023_4_VEC_INSERT]], [[BB63]] ], [ [[DOTSROA_023_0_VEC_INSERT:%.*]], [[BB67]] ]
; CHECK-NEXT:    [[DOTSROA_021_1]] = phi <5 x i32> [ [[DOTSROA_021_4_VEC_INSERT]], [[BB63]] ], [ [[DOTSROA_021_0_VEC_INSERT:%.*]], [[BB67]] ]
; CHECK-NEXT:    [[DOTSROA_033_1]] = phi <5 x i32> [ [[DOTSROA_033_4_VEC_INSERT]], [[BB63]] ], [ [[DOTSROA_033_0_VEC_INSERT:%.*]], [[BB67]] ]
; CHECK-NEXT:    [[DOTSROA_035_1]] = phi <5 x i32> [ [[DOTSROA_035_4_VEC_INSERT]], [[BB63]] ], [ [[DOTSROA_035_0_VEC_INSERT:%.*]], [[BB67]] ]
; CHECK-NEXT:    [[DOTSROA_037_1]] = phi <5 x i32> [ [[DOTSROA_037_4_VEC_INSERT]], [[BB63]] ], [ [[DOTSROA_037_0_VEC_INSERT:%.*]], [[BB67]] ]
; CHECK-NEXT:    [[DOTSROA_039_1]] = phi <5 x i32> [ [[DOTSROA_039_4_VEC_INSERT]], [[BB63]] ], [ [[DOTSROA_039_0_VEC_INSERT:%.*]], [[BB67]] ]
; CHECK-NEXT:    [[I65:%.*]] = phi i32 [ [[I105:%.*]], [[BB67]] ], [ [[I44]], [[BB63]] ]
; CHECK-NEXT:    [[I66:%.*]] = icmp slt i32 [[I65]], [[I46]]
; CHECK-NEXT:    br i1 [[I66]], label [[BB67]], label [[BB106]]
; CHECK:       bb67:
; CHECK-NEXT:    [[DOTSROA_039_0_VEC_INSERT]] = insertelement <5 x i32> [[DOTSROA_039_1]], i32 [[I65]], i32 0
; CHECK-NEXT:    [[DOTSROA_033_0_VEC_INSERT]] = insertelement <5 x i32> [[DOTSROA_033_1]], i32 [[I65]], i32 0
; CHECK-NEXT:    [[DOTSROA_021_0_VEC_INSERT]] = insertelement <5 x i32> [[DOTSROA_021_1]], i32 [[I65]], i32 0
; CHECK-NEXT:    [[I68:%.*]] = call <64 x float> @llvm.tpc.ld.tnsr.v64f32.i1(<5 x i32> [[DOTSROA_021_0_VEC_INSERT]], i8 0, i32 0, <64 x float> undef, i1 true, i1 false)
; CHECK-NEXT:    [[I69:%.*]] = or i32 [[I65]], 64
; CHECK-NEXT:    [[DOTSROA_023_0_VEC_INSERT]] = insertelement <5 x i32> [[DOTSROA_023_1]], i32 [[I69]], i32 0
; CHECK-NEXT:    [[I70:%.*]] = call <64 x float> @llvm.tpc.ld.tnsr.v64f32.i1(<5 x i32> [[DOTSROA_023_0_VEC_INSERT]], i8 0, i32 0, <64 x float> undef, i1 true, i1 false)
; CHECK-NEXT:    [[DOTSROA_025_0_VEC_INSERT]] = insertelement <5 x i32> [[DOTSROA_025_1]], i32 [[I65]], i32 0
; CHECK-NEXT:    [[I71:%.*]] = call <64 x float> @llvm.tpc.ld.tnsr.v64f32.i1(<5 x i32> [[DOTSROA_025_0_VEC_INSERT]], i8 0, i32 0, <64 x float> undef, i1 true, i1 false)
; CHECK-NEXT:    [[DOTSROA_027_0_VEC_INSERT]] = insertelement <5 x i32> [[DOTSROA_027_1]], i32 [[I69]], i32 0
; CHECK-NEXT:    [[I72:%.*]] = call <64 x float> @llvm.tpc.ld.tnsr.v64f32.i1(<5 x i32> [[DOTSROA_027_0_VEC_INSERT]], i8 0, i32 0, <64 x float> undef, i1 true, i1 false)
; CHECK-NEXT:    [[I73:%.*]] = fmul <64 x float> [[I71]], [[I68]]
; CHECK-NEXT:    [[I74:%.*]] = fmul <64 x float> [[I72]], [[I70]]
; CHECK-NEXT:    [[DOTSROA_029_0_VEC_INSERT]] = insertelement <5 x i32> [[DOTSROA_029_1]], i32 [[I65]], i32 0
; CHECK-NEXT:    [[I75:%.*]] = call <64 x float> @llvm.tpc.ld.tnsr.v64f32.i1(<5 x i32> [[DOTSROA_029_0_VEC_INSERT]], i8 0, i32 0, <64 x float> undef, i1 true, i1 false)
; CHECK-NEXT:    [[DOTSROA_031_0_VEC_INSERT]] = insertelement <5 x i32> [[DOTSROA_031_1]], i32 [[I69]], i32 0
; CHECK-NEXT:    [[I76:%.*]] = call <64 x float> @llvm.tpc.ld.tnsr.v64f32.i1(<5 x i32> [[DOTSROA_031_0_VEC_INSERT]], i8 0, i32 0, <64 x float> undef, i1 true, i1 false)
; CHECK-NEXT:    [[I77:%.*]] = fmul <64 x float> [[I73]], [[I75]]
; CHECK-NEXT:    [[I78:%.*]] = fmul <64 x float> [[I74]], [[I76]]
; CHECK-NEXT:    [[I79:%.*]] = fmul <64 x float> [[I50]], [[I77]]
; CHECK-NEXT:    [[I80:%.*]] = fmul <64 x float> [[I51]], [[I78]]
; CHECK-NEXT:    [[I81:%.*]] = shufflevector <64 x float> [[I79]], <64 x float> [[I80]], <128 x i32> <i32 0, i32 1, i32 2, i32 3, i32 4, i32 5, i32 6, i32 7, i32 8, i32 9, i32 10, i32 11, i32 12, i32 13, i32 14, i32 15, i32 16, i32 17, i32 18, i32 19, i32 20, i32 21, i32 22, i32 23, i32 24, i32 25, i32 26, i32 27, i32 28, i32 29, i32 30, i32 31, i32 32, i32 33, i32 34, i32 35, i32 36, i32 37, i32 38, i32 39, i32 40, i32 41, i32 42, i32 43, i32 44, i32 45, i32 46, i32 47, i32 48, i32 49, i32 50, i32 51, i32 52, i32 53, i32 54, i32 55, i32 56, i32 57, i32 58, i32 59, i32 60, i32 61, i32 62, i32 63, i32 64, i32 65, i32 66, i32 67, i32 68, i32 69, i32 70, i32 71, i32 72, i32 73, i32 74, i32 75, i32 76, i32 77, i32 78, i32 79, i32 80, i32 81, i32 82, i32 83, i32 84, i32 85, i32 86, i32 87, i32 88, i32 89, i32 90, i32 91, i32 92, i32 93, i32 94, i32 95, i32 96, i32 97, i32 98, i32 99, i32 100, i32 101, i32 102, i32 103, i32 104, i32 105, i32 106, i32 107, i32 108, i32 109, i32 110, i32 111, i32 112, i32 113, i32 114, i32 115, i32 116, i32 117, i32 118, i32 119, i32 120, i32 121, i32 122, i32 123, i32 124, i32 125, i32 126, i32 127>
; CHECK-NEXT:    [[I82:%.*]] = call <128 x bfloat> @llvm.tpc.fptrunc.swch.v128bf16.v128f32(<128 x float> [[I81]], i32 0)
; CHECK-NEXT:    [[I83:%.*]] = call <128 x bfloat> @llvm.tpc.ld.tnsr.v128bf16.i1(<5 x i32> [[DOTSROA_033_0_VEC_INSERT]], i8 2, i32 0, <128 x bfloat> undef, i1 true, i1 false)
; CHECK-NEXT:    [[I84:%.*]] = fadd <128 x bfloat> [[I83]], [[I82]]
; CHECK-NEXT:    [[TMP0:%.*]] = call <128 x float> @llvm.tpc.convert.v128f32.v128bf16.i1(<128 x bfloat> [[I84]], i8 1, i32 327680, <128 x float> undef, i1 true, i1 false)
; CHECK-NEXT:    [[I86:%.*]] = shufflevector <128 x float> [[TMP0]], <128 x float> undef, <64 x i32> <i32 0, i32 1, i32 2, i32 3, i32 4, i32 5, i32 6, i32 7, i32 8, i32 9, i32 10, i32 11, i32 12, i32 13, i32 14, i32 15, i32 16, i32 17, i32 18, i32 19, i32 20, i32 21, i32 22, i32 23, i32 24, i32 25, i32 26, i32 27, i32 28, i32 29, i32 30, i32 31, i32 32, i32 33, i32 34, i32 35, i32 36, i32 37, i32 38, i32 39, i32 40, i32 41, i32 42, i32 43, i32 44, i32 45, i32 46, i32 47, i32 48, i32 49, i32 50, i32 51, i32 52, i32 53, i32 54, i32 55, i32 56, i32 57, i32 58, i32 59, i32 60, i32 61, i32 62, i32 63>
; CHECK-NEXT:    [[I87:%.*]] = shufflevector <128 x float> [[TMP0]], <128 x float> undef, <64 x i32> <i32 64, i32 65, i32 66, i32 67, i32 68, i32 69, i32 70, i32 71, i32 72, i32 73, i32 74, i32 75, i32 76, i32 77, i32 78, i32 79, i32 80, i32 81, i32 82, i32 83, i32 84, i32 85, i32 86, i32 87, i32 88, i32 89, i32 90, i32 91, i32 92, i32 93, i32 94, i32 95, i32 96, i32 97, i32 98, i32 99, i32 100, i32 101, i32 102, i32 103, i32 104, i32 105, i32 106, i32 107, i32 108, i32 109, i32 110, i32 111, i32 112, i32 113, i32 114, i32 115, i32 116, i32 117, i32 118, i32 119, i32 120, i32 121, i32 122, i32 123, i32 124, i32 125, i32 126, i32 127>
; CHECK-NEXT:    [[I88:%.*]] = fmul <64 x float> [[I53]], [[I86]]
; CHECK-NEXT:    [[I89:%.*]] = fmul <64 x float> [[I54]], [[I87]]
; CHECK-NEXT:    [[I90:%.*]] = shufflevector <64 x float> [[I88]], <64 x float> [[I89]], <128 x i32> <i32 0, i32 1, i32 2, i32 3, i32 4, i32 5, i32 6, i32 7, i32 8, i32 9, i32 10, i32 11, i32 12, i32 13, i32 14, i32 15, i32 16, i32 17, i32 18, i32 19, i32 20, i32 21, i32 22, i32 23, i32 24, i32 25, i32 26, i32 27, i32 28, i32 29, i32 30, i32 31, i32 32, i32 33, i32 34, i32 35, i32 36, i32 37, i32 38, i32 39, i32 40, i32 41, i32 42, i32 43, i32 44, i32 45, i32 46, i32 47, i32 48, i32 49, i32 50, i32 51, i32 52, i32 53, i32 54, i32 55, i32 56, i32 57, i32 58, i32 59, i32 60, i32 61, i32 62, i32 63, i32 64, i32 65, i32 66, i32 67, i32 68, i32 69, i32 70, i32 71, i32 72, i32 73, i32 74, i32 75, i32 76, i32 77, i32 78, i32 79, i32 80, i32 81, i32 82, i32 83, i32 84, i32 85, i32 86, i32 87, i32 88, i32 89, i32 90, i32 91, i32 92, i32 93, i32 94, i32 95, i32 96, i32 97, i32 98, i32 99, i32 100, i32 101, i32 102, i32 103, i32 104, i32 105, i32 106, i32 107, i32 108, i32 109, i32 110, i32 111, i32 112, i32 113, i32 114, i32 115, i32 116, i32 117, i32 118, i32 119, i32 120, i32 121, i32 122, i32 123, i32 124, i32 125, i32 126, i32 127>
; CHECK-NEXT:    [[TMP1:%.*]] = call <128 x bfloat> @llvm.tpc.convert.v128bf16.v128f32.i1(<128 x float> [[I90]], i8 0, i32 327940, <128 x bfloat> undef, i1 true, i1 false)
; CHECK-NEXT:    [[I92:%.*]] = call <128 x bfloat> @llvm.tpc.tanh.v128bf16(<128 x bfloat> [[TMP1]])
; CHECK-NEXT:    [[I93:%.*]] = fadd <128 x bfloat> [[I56]], [[I92]]
; CHECK-NEXT:    [[I94:%.*]] = call <128 x float> @llvm.tpc.fpext.swch.v128f32.v128bf16(<128 x bfloat> [[I93]], i32 0)
; CHECK-NEXT:    [[I95:%.*]] = shufflevector <128 x float> [[I94]], <128 x float> undef, <64 x i32> <i32 0, i32 1, i32 2, i32 3, i32 4, i32 5, i32 6, i32 7, i32 8, i32 9, i32 10, i32 11, i32 12, i32 13, i32 14, i32 15, i32 16, i32 17, i32 18, i32 19, i32 20, i32 21, i32 22, i32 23, i32 24, i32 25, i32 26, i32 27, i32 28, i32 29, i32 30, i32 31, i32 32, i32 33, i32 34, i32 35, i32 36, i32 37, i32 38, i32 39, i32 40, i32 41, i32 42, i32 43, i32 44, i32 45, i32 46, i32 47, i32 48, i32 49, i32 50, i32 51, i32 52, i32 53, i32 54, i32 55, i32 56, i32 57, i32 58, i32 59, i32 60, i32 61, i32 62, i32 63>
; CHECK-NEXT:    [[I96:%.*]] = shufflevector <128 x float> [[I94]], <128 x float> undef, <64 x i32> <i32 64, i32 65, i32 66, i32 67, i32 68, i32 69, i32 70, i32 71, i32 72, i32 73, i32 74, i32 75, i32 76, i32 77, i32 78, i32 79, i32 80, i32 81, i32 82, i32 83, i32 84, i32 85, i32 86, i32 87, i32 88, i32 89, i32 90, i32 91, i32 92, i32 93, i32 94, i32 95, i32 96, i32 97, i32 98, i32 99, i32 100, i32 101, i32 102, i32 103, i32 104, i32 105, i32 106, i32 107, i32 108, i32 109, i32 110, i32 111, i32 112, i32 113, i32 114, i32 115, i32 116, i32 117, i32 118, i32 119, i32 120, i32 121, i32 122, i32 123, i32 124, i32 125, i32 126, i32 127>
; CHECK-NEXT:    [[DOTSROA_035_0_VEC_INSERT]] = insertelement <5 x i32> [[DOTSROA_035_1]], i32 [[I65]], i32 0
; CHECK-NEXT:    [[I97:%.*]] = call <64 x float> @llvm.tpc.ld.tnsr.v64f32.i1(<5 x i32> [[DOTSROA_035_0_VEC_INSERT]], i8 0, i32 0, <64 x float> undef, i1 true, i1 false)
; CHECK-NEXT:    [[DOTSROA_037_0_VEC_INSERT]] = insertelement <5 x i32> [[DOTSROA_037_1]], i32 [[I69]], i32 0
; CHECK-NEXT:    [[I98:%.*]] = call <64 x float> @llvm.tpc.ld.tnsr.v64f32.i1(<5 x i32> [[DOTSROA_037_0_VEC_INSERT]], i8 0, i32 0, <64 x float> undef, i1 true, i1 false)
; CHECK-NEXT:    [[I99:%.*]] = fmul <64 x float> [[I58]], [[I97]]
; CHECK-NEXT:    [[I100:%.*]] = fmul <64 x float> [[I59]], [[I98]]
; CHECK-NEXT:    [[I101:%.*]] = fmul <64 x float> [[I95]], [[I99]]
; CHECK-NEXT:    [[I102:%.*]] = fmul <64 x float> [[I96]], [[I100]]
; CHECK-NEXT:    [[I103:%.*]] = shufflevector <64 x float> [[I101]], <64 x float> [[I102]], <128 x i32> <i32 0, i32 1, i32 2, i32 3, i32 4, i32 5, i32 6, i32 7, i32 8, i32 9, i32 10, i32 11, i32 12, i32 13, i32 14, i32 15, i32 16, i32 17, i32 18, i32 19, i32 20, i32 21, i32 22, i32 23, i32 24, i32 25, i32 26, i32 27, i32 28, i32 29, i32 30, i32 31, i32 32, i32 33, i32 34, i32 35, i32 36, i32 37, i32 38, i32 39, i32 40, i32 41, i32 42, i32 43, i32 44, i32 45, i32 46, i32 47, i32 48, i32 49, i32 50, i32 51, i32 52, i32 53, i32 54, i32 55, i32 56, i32 57, i32 58, i32 59, i32 60, i32 61, i32 62, i32 63, i32 64, i32 65, i32 66, i32 67, i32 68, i32 69, i32 70, i32 71, i32 72, i32 73, i32 74, i32 75, i32 76, i32 77, i32 78, i32 79, i32 80, i32 81, i32 82, i32 83, i32 84, i32 85, i32 86, i32 87, i32 88, i32 89, i32 90, i32 91, i32 92, i32 93, i32 94, i32 95, i32 96, i32 97, i32 98, i32 99, i32 100, i32 101, i32 102, i32 103, i32 104, i32 105, i32 106, i32 107, i32 108, i32 109, i32 110, i32 111, i32 112, i32 113, i32 114, i32 115, i32 116, i32 117, i32 118, i32 119, i32 120, i32 121, i32 122, i32 123, i32 124, i32 125, i32 126, i32 127>
; CHECK-NEXT:    [[TMP2:%.*]] = call <128 x bfloat> @llvm.tpc.convert.v128bf16.v128f32.i1(<128 x float> [[I103]], i8 0, i32 327940, <128 x bfloat> undef, i1 true, i1 false)
; CHECK-NEXT:    call void @llvm.tpc.st.tnsr.v128bf16(<5 x i32> [[DOTSROA_039_0_VEC_INSERT]], i8 6, <128 x bfloat> [[TMP2]], i32 4, i1 true, i1 false)
; CHECK-NEXT:    [[I105]] = add i32 [[I65]], 128
; CHECK-NEXT:    br label [[BB64]]
; CHECK:       bb106:
; CHECK-NEXT:    [[I107]] = add i32 [[I61]], 1
; CHECK-NEXT:    br label [[BB60]]
; CHECK:       bb108:
; CHECK-NEXT:    ret void
;
bb:
  %i = call <5 x i32> @llvm.tpc.get.index.space.offset()
  %i41 = call <5 x i32> @llvm.tpc.get.index.space.size()
  %i42 = add <5 x i32> %i, %i41
  %i43 = extractelement <5 x i32> %i, i32 0
  %i44 = shl i32 %i43, 7
  %i45 = extractelement <5 x i32> %i42, i32 0
  %i46 = shl i32 %i45, 7
  %i47 = extractelement <5 x i32> %i, i32 1
  %i48 = extractelement <5 x i32> %i42, i32 1
  %.sroa.0.4.vec.insert = insertelement <5 x i32> zeroinitializer, i32 0, i32 1
  %.sroa.0.0.vec.insert = insertelement <5 x i32> %.sroa.0.4.vec.insert, i32 0, i32 0
  %i49 = call i8 addrspace(3)* @llvm.tpc.gen.addr(<5 x i32> %.sroa.0.0.vec.insert, i8 1, i32 0, i8 addrspace(3)* undef, i1 true, i1 false)
  %i50 = call <64 x float> @llvm.tpc.ld.g.v64f32.i1(i8 addrspace(3)* %i49, i32 0, <64 x float> undef, i1 true, i1 false)
  %i51 = call <64 x float> @llvm.tpc.ld.g.v64f32.i1(i8 addrspace(3)* %i49, i32 0, <64 x float> undef, i1 true, i1 false)
  %.sroa.015.4.vec.insert = insertelement <5 x i32> zeroinitializer, i32 0, i32 1
  %.sroa.015.0.vec.insert = insertelement <5 x i32> %.sroa.015.4.vec.insert, i32 0, i32 0
  %i52 = call i8 addrspace(3)* @llvm.tpc.gen.addr(<5 x i32> %.sroa.015.0.vec.insert, i8 3, i32 0, i8 addrspace(3)* undef, i1 true, i1 false)
  %i53 = call <64 x float> @llvm.tpc.ld.g.v64f32.i1(i8 addrspace(3)* %i52, i32 0, <64 x float> undef, i1 true, i1 false)
  %i54 = call <64 x float> @llvm.tpc.ld.g.v64f32.i1(i8 addrspace(3)* %i52, i32 0, <64 x float> undef, i1 true, i1 false)
  %.sroa.017.4.vec.insert = insertelement <5 x i32> zeroinitializer, i32 0, i32 1
  %.sroa.017.0.vec.insert = insertelement <5 x i32> %.sroa.017.4.vec.insert, i32 0, i32 0
  %i55 = call i8 addrspace(3)* @llvm.tpc.gen.addr(<5 x i32> %.sroa.017.0.vec.insert, i8 4, i32 0, i8 addrspace(3)* undef, i1 true, i1 false)
  %i56 = call <128 x bfloat> @llvm.tpc.ld.g.v128bf16.i1(i8 addrspace(3)* %i55, i32 0, <128 x bfloat> undef, i1 true, i1 false)
  %.sroa.019.4.vec.insert = insertelement <5 x i32> zeroinitializer, i32 0, i32 1
  %.sroa.019.0.vec.insert = insertelement <5 x i32> %.sroa.019.4.vec.insert, i32 0, i32 0
  %i57 = call i8 addrspace(3)* @llvm.tpc.gen.addr(<5 x i32> %.sroa.019.0.vec.insert, i8 5, i32 0, i8 addrspace(3)* undef, i1 true, i1 false)
  %i58 = call <64 x float> @llvm.tpc.ld.g.v64f32.i1(i8 addrspace(3)* %i57, i32 0, <64 x float> undef, i1 true, i1 false)
  %i59 = call <64 x float> @llvm.tpc.ld.g.v64f32.i1(i8 addrspace(3)* %i57, i32 0, <64 x float> undef, i1 true, i1 false)
  br label %bb60

  bb60:                                             ; preds = %bb106, %bb
  %.sroa.031.0 = phi <5 x i32> [ zeroinitializer, %bb ], [ %.sroa.031.1, %bb106 ]
  %.sroa.029.0 = phi <5 x i32> [ zeroinitializer, %bb ], [ %.sroa.029.1, %bb106 ]
  %.sroa.027.0 = phi <5 x i32> [ zeroinitializer, %bb ], [ %.sroa.027.1, %bb106 ]
  %.sroa.025.0 = phi <5 x i32> [ zeroinitializer, %bb ], [ %.sroa.025.1, %bb106 ]
  %.sroa.023.0 = phi <5 x i32> [ zeroinitializer, %bb ], [ %.sroa.023.1, %bb106 ]
  %.sroa.021.0 = phi <5 x i32> [ zeroinitializer, %bb ], [ %.sroa.021.1, %bb106 ]
  %.sroa.033.0 = phi <5 x i32> [ zeroinitializer, %bb ], [ %.sroa.033.1, %bb106 ]
  %.sroa.035.0 = phi <5 x i32> [ zeroinitializer, %bb ], [ %.sroa.035.1, %bb106 ]
  %.sroa.037.0 = phi <5 x i32> [ zeroinitializer, %bb ], [ %.sroa.037.1, %bb106 ]
  %.sroa.039.0 = phi <5 x i32> [ zeroinitializer, %bb ], [ %.sroa.039.1, %bb106 ]
  %i61 = phi i32 [ %i107, %bb106 ], [ %i47, %bb ]
  %i62 = icmp slt i32 %i61, %i48
  br i1 %i62, label %bb63, label %bb108

  bb63:                                             ; preds = %bb60
  %.sroa.039.4.vec.insert = insertelement <5 x i32> %.sroa.039.0, i32 %i61, i32 1
  %.sroa.037.4.vec.insert = insertelement <5 x i32> %.sroa.037.0, i32 %i61, i32 1
  %.sroa.035.4.vec.insert = insertelement <5 x i32> %.sroa.035.0, i32 %i61, i32 1
  %.sroa.033.4.vec.insert = insertelement <5 x i32> %.sroa.033.0, i32 %i61, i32 1
  %.sroa.031.4.vec.insert = insertelement <5 x i32> %.sroa.031.0, i32 %i61, i32 1
  %.sroa.029.4.vec.insert = insertelement <5 x i32> %.sroa.029.0, i32 %i61, i32 1
  %.sroa.027.4.vec.insert = insertelement <5 x i32> %.sroa.027.0, i32 %i61, i32 1
  %.sroa.025.4.vec.insert = insertelement <5 x i32> %.sroa.025.0, i32 %i61, i32 1
  %.sroa.023.4.vec.insert = insertelement <5 x i32> %.sroa.023.0, i32 %i61, i32 1
  %.sroa.021.4.vec.insert = insertelement <5 x i32> %.sroa.021.0, i32 %i61, i32 1
  br label %bb64

  bb64:                                             ; preds = %bb67, %bb63
  %.sroa.031.1 = phi <5 x i32> [ %.sroa.031.4.vec.insert, %bb63 ], [ %.sroa.031.0.vec.insert, %bb67 ]
  %.sroa.029.1 = phi <5 x i32> [ %.sroa.029.4.vec.insert, %bb63 ], [ %.sroa.029.0.vec.insert, %bb67 ]
  %.sroa.027.1 = phi <5 x i32> [ %.sroa.027.4.vec.insert, %bb63 ], [ %.sroa.027.0.vec.insert, %bb67 ]
  %.sroa.025.1 = phi <5 x i32> [ %.sroa.025.4.vec.insert, %bb63 ], [ %.sroa.025.0.vec.insert, %bb67 ]
  %.sroa.023.1 = phi <5 x i32> [ %.sroa.023.4.vec.insert, %bb63 ], [ %.sroa.023.0.vec.insert, %bb67 ]
  %.sroa.021.1 = phi <5 x i32> [ %.sroa.021.4.vec.insert, %bb63 ], [ %.sroa.021.0.vec.insert, %bb67 ]
  %.sroa.033.1 = phi <5 x i32> [ %.sroa.033.4.vec.insert, %bb63 ], [ %.sroa.033.0.vec.insert, %bb67 ]
  %.sroa.035.1 = phi <5 x i32> [ %.sroa.035.4.vec.insert, %bb63 ], [ %.sroa.035.0.vec.insert, %bb67 ]
  %.sroa.037.1 = phi <5 x i32> [ %.sroa.037.4.vec.insert, %bb63 ], [ %.sroa.037.0.vec.insert, %bb67 ]
  %.sroa.039.1 = phi <5 x i32> [ %.sroa.039.4.vec.insert, %bb63 ], [ %.sroa.039.0.vec.insert, %bb67 ]
  %i65 = phi i32 [ %i105, %bb67 ], [ %i44, %bb63 ]
  %i66 = icmp slt i32 %i65, %i46
  br i1 %i66, label %bb67, label %bb106

  bb67:                                             ; preds = %bb64
  %.sroa.039.0.vec.insert = insertelement <5 x i32> %.sroa.039.1, i32 %i65, i32 0
  %.sroa.033.0.vec.insert = insertelement <5 x i32> %.sroa.033.1, i32 %i65, i32 0
  %.sroa.021.0.vec.insert = insertelement <5 x i32> %.sroa.021.1, i32 %i65, i32 0
  %i68 = call <64 x float> @llvm.tpc.ld.tnsr.v64f32.i1(<5 x i32> %.sroa.021.0.vec.insert, i8 0, i32 0, <64 x float> undef, i1 true, i1 false)
  %i69 = or i32 %i65, 64
  %.sroa.023.0.vec.insert = insertelement <5 x i32> %.sroa.023.1, i32 %i69, i32 0
  %i70 = call <64 x float> @llvm.tpc.ld.tnsr.v64f32.i1(<5 x i32> %.sroa.023.0.vec.insert, i8 0, i32 0, <64 x float> undef, i1 true, i1 false)
  %.sroa.025.0.vec.insert = insertelement <5 x i32> %.sroa.025.1, i32 %i65, i32 0
  %i71 = call <64 x float> @llvm.tpc.ld.tnsr.v64f32.i1(<5 x i32> %.sroa.025.0.vec.insert, i8 0, i32 0, <64 x float> undef, i1 true, i1 false)
  %.sroa.027.0.vec.insert = insertelement <5 x i32> %.sroa.027.1, i32 %i69, i32 0
  %i72 = call <64 x float> @llvm.tpc.ld.tnsr.v64f32.i1(<5 x i32> %.sroa.027.0.vec.insert, i8 0, i32 0, <64 x float> undef, i1 true, i1 false)
  %i73 = fmul <64 x float> %i71, %i68
  %i74 = fmul <64 x float> %i72, %i70
  %.sroa.029.0.vec.insert = insertelement <5 x i32> %.sroa.029.1, i32 %i65, i32 0
  %i75 = call <64 x float> @llvm.tpc.ld.tnsr.v64f32.i1(<5 x i32> %.sroa.029.0.vec.insert, i8 0, i32 0, <64 x float> undef, i1 true, i1 false)
  %.sroa.031.0.vec.insert = insertelement <5 x i32> %.sroa.031.1, i32 %i69, i32 0
  %i76 = call <64 x float> @llvm.tpc.ld.tnsr.v64f32.i1(<5 x i32> %.sroa.031.0.vec.insert, i8 0, i32 0, <64 x float> undef, i1 true, i1 false)
  %i77 = fmul <64 x float> %i73, %i75
  %i78 = fmul <64 x float> %i74, %i76
  %i79 = fmul <64 x float> %i50, %i77
  %i80 = fmul <64 x float> %i51, %i78
  %i81 = shufflevector <64 x float> %i79, <64 x float> %i80, <128 x i32> <i32 0, i32 1, i32 2, i32 3, i32 4, i32 5, i32 6, i32 7, i32 8, i32 9, i32 10, i32 11, i32 12, i32 13, i32 14, i32 15, i32 16, i32 17, i32 18, i32 19, i32 20, i32 21, i32 22, i32 23, i32 24, i32 25, i32 26, i32 27, i32 28, i32 29, i32 30, i32 31, i32 32, i32 33, i32 34, i32 35, i32 36, i32 37, i32 38, i32 39, i32 40, i32 41, i32 42, i32 43, i32 44, i32 45, i32 46, i32 47, i32 48, i32 49, i32 50, i32 51, i32 52, i32 53, i32 54, i32 55, i32 56, i32 57, i32 58, i32 59, i32 60, i32 61, i32 62, i32 63, i32 64, i32 65, i32 66, i32 67, i32 68, i32 69, i32 70, i32 71, i32 72, i32 73, i32 74, i32 75, i32 76, i32 77, i32 78, i32 79, i32 80, i32 81, i32 82, i32 83, i32 84, i32 85, i32 86, i32 87, i32 88, i32 89, i32 90, i32 91, i32 92, i32 93, i32 94, i32 95, i32 96, i32 97, i32 98, i32 99, i32 100, i32 101, i32 102, i32 103, i32 104, i32 105, i32 106, i32 107, i32 108, i32 109, i32 110, i32 111, i32 112, i32 113, i32 114, i32 115, i32 116, i32 117, i32 118, i32 119, i32 120, i32 121, i32 122, i32 123, i32 124, i32 125, i32 126, i32 127>
  %i82 = call <128 x bfloat> @llvm.tpc.fptrunc.swch.v128bf16.v128f32(<128 x float> %i81, i32 0)
  %i83 = call <128 x bfloat> @llvm.tpc.ld.tnsr.v128bf16.i1(<5 x i32> %.sroa.033.0.vec.insert, i8 2, i32 0, <128 x bfloat> undef, i1 true, i1 false)
  %i84 = fadd <128 x bfloat> %i83, %i82
  %i85 = call <128 x float> @llvm.tpc.fpext.swch.v128f32.v128bf16(<128 x bfloat> %i84, i32 0)
  %i86 = shufflevector <128 x float> %i85, <128 x float> undef, <64 x i32> <i32 0, i32 1, i32 2, i32 3, i32 4, i32 5, i32 6, i32 7, i32 8, i32 9, i32 10, i32 11, i32 12, i32 13, i32 14, i32 15, i32 16, i32 17, i32 18, i32 19, i32 20, i32 21, i32 22, i32 23, i32 24, i32 25, i32 26, i32 27, i32 28, i32 29, i32 30, i32 31, i32 32, i32 33, i32 34, i32 35, i32 36, i32 37, i32 38, i32 39, i32 40, i32 41, i32 42, i32 43, i32 44, i32 45, i32 46, i32 47, i32 48, i32 49, i32 50, i32 51, i32 52, i32 53, i32 54, i32 55, i32 56, i32 57, i32 58, i32 59, i32 60, i32 61, i32 62, i32 63>
  %i87 = shufflevector <128 x float> %i85, <128 x float> undef, <64 x i32> <i32 64, i32 65, i32 66, i32 67, i32 68, i32 69, i32 70, i32 71, i32 72, i32 73, i32 74, i32 75, i32 76, i32 77, i32 78, i32 79, i32 80, i32 81, i32 82, i32 83, i32 84, i32 85, i32 86, i32 87, i32 88, i32 89, i32 90, i32 91, i32 92, i32 93, i32 94, i32 95, i32 96, i32 97, i32 98, i32 99, i32 100, i32 101, i32 102, i32 103, i32 104, i32 105, i32 106, i32 107, i32 108, i32 109, i32 110, i32 111, i32 112, i32 113, i32 114, i32 115, i32 116, i32 117, i32 118, i32 119, i32 120, i32 121, i32 122, i32 123, i32 124, i32 125, i32 126, i32 127>
  %i88 = fmul <64 x float> %i53, %i86
  %i89 = fmul <64 x float> %i54, %i87
  %i90 = shufflevector <64 x float> %i88, <64 x float> %i89, <128 x i32> <i32 0, i32 1, i32 2, i32 3, i32 4, i32 5, i32 6, i32 7, i32 8, i32 9, i32 10, i32 11, i32 12, i32 13, i32 14, i32 15, i32 16, i32 17, i32 18, i32 19, i32 20, i32 21, i32 22, i32 23, i32 24, i32 25, i32 26, i32 27, i32 28, i32 29, i32 30, i32 31, i32 32, i32 33, i32 34, i32 35, i32 36, i32 37, i32 38, i32 39, i32 40, i32 41, i32 42, i32 43, i32 44, i32 45, i32 46, i32 47, i32 48, i32 49, i32 50, i32 51, i32 52, i32 53, i32 54, i32 55, i32 56, i32 57, i32 58, i32 59, i32 60, i32 61, i32 62, i32 63, i32 64, i32 65, i32 66, i32 67, i32 68, i32 69, i32 70, i32 71, i32 72, i32 73, i32 74, i32 75, i32 76, i32 77, i32 78, i32 79, i32 80, i32 81, i32 82, i32 83, i32 84, i32 85, i32 86, i32 87, i32 88, i32 89, i32 90, i32 91, i32 92, i32 93, i32 94, i32 95, i32 96, i32 97, i32 98, i32 99, i32 100, i32 101, i32 102, i32 103, i32 104, i32 105, i32 106, i32 107, i32 108, i32 109, i32 110, i32 111, i32 112, i32 113, i32 114, i32 115, i32 116, i32 117, i32 118, i32 119, i32 120, i32 121, i32 122, i32 123, i32 124, i32 125, i32 126, i32 127>
  %i91 = call <128 x bfloat> @llvm.tpc.fptrunc.swch.v128bf16.v128f32(<128 x float> %i90, i32 0)
  %i92 = call <128 x bfloat> @llvm.tpc.tanh.v128bf16(<128 x bfloat> %i91)
  %i93 = fadd <128 x bfloat> %i56, %i92
  %i94 = call <128 x float> @llvm.tpc.fpext.swch.v128f32.v128bf16(<128 x bfloat> %i93, i32 0)
  %i95 = shufflevector <128 x float> %i94, <128 x float> undef, <64 x i32> <i32 0, i32 1, i32 2, i32 3, i32 4, i32 5, i32 6, i32 7, i32 8, i32 9, i32 10, i32 11, i32 12, i32 13, i32 14, i32 15, i32 16, i32 17, i32 18, i32 19, i32 20, i32 21, i32 22, i32 23, i32 24, i32 25, i32 26, i32 27, i32 28, i32 29, i32 30, i32 31, i32 32, i32 33, i32 34, i32 35, i32 36, i32 37, i32 38, i32 39, i32 40, i32 41, i32 42, i32 43, i32 44, i32 45, i32 46, i32 47, i32 48, i32 49, i32 50, i32 51, i32 52, i32 53, i32 54, i32 55, i32 56, i32 57, i32 58, i32 59, i32 60, i32 61, i32 62, i32 63>
  %i96 = shufflevector <128 x float> %i94, <128 x float> undef, <64 x i32> <i32 64, i32 65, i32 66, i32 67, i32 68, i32 69, i32 70, i32 71, i32 72, i32 73, i32 74, i32 75, i32 76, i32 77, i32 78, i32 79, i32 80, i32 81, i32 82, i32 83, i32 84, i32 85, i32 86, i32 87, i32 88, i32 89, i32 90, i32 91, i32 92, i32 93, i32 94, i32 95, i32 96, i32 97, i32 98, i32 99, i32 100, i32 101, i32 102, i32 103, i32 104, i32 105, i32 106, i32 107, i32 108, i32 109, i32 110, i32 111, i32 112, i32 113, i32 114, i32 115, i32 116, i32 117, i32 118, i32 119, i32 120, i32 121, i32 122, i32 123, i32 124, i32 125, i32 126, i32 127>
  %.sroa.035.0.vec.insert = insertelement <5 x i32> %.sroa.035.1, i32 %i65, i32 0
  %i97 = call <64 x float> @llvm.tpc.ld.tnsr.v64f32.i1(<5 x i32> %.sroa.035.0.vec.insert, i8 0, i32 0, <64 x float> undef, i1 true, i1 false)
  %.sroa.037.0.vec.insert = insertelement <5 x i32> %.sroa.037.1, i32 %i69, i32 0
  %i98 = call <64 x float> @llvm.tpc.ld.tnsr.v64f32.i1(<5 x i32> %.sroa.037.0.vec.insert, i8 0, i32 0, <64 x float> undef, i1 true, i1 false)
  %i99 = fmul <64 x float> %i58, %i97
  %i100 = fmul <64 x float> %i59, %i98
  %i101 = fmul <64 x float> %i95, %i99
  %i102 = fmul <64 x float> %i96, %i100
  %i103 = shufflevector <64 x float> %i101, <64 x float> %i102, <128 x i32> <i32 0, i32 1, i32 2, i32 3, i32 4, i32 5, i32 6, i32 7, i32 8, i32 9, i32 10, i32 11, i32 12, i32 13, i32 14, i32 15, i32 16, i32 17, i32 18, i32 19, i32 20, i32 21, i32 22, i32 23, i32 24, i32 25, i32 26, i32 27, i32 28, i32 29, i32 30, i32 31, i32 32, i32 33, i32 34, i32 35, i32 36, i32 37, i32 38, i32 39, i32 40, i32 41, i32 42, i32 43, i32 44, i32 45, i32 46, i32 47, i32 48, i32 49, i32 50, i32 51, i32 52, i32 53, i32 54, i32 55, i32 56, i32 57, i32 58, i32 59, i32 60, i32 61, i32 62, i32 63, i32 64, i32 65, i32 66, i32 67, i32 68, i32 69, i32 70, i32 71, i32 72, i32 73, i32 74, i32 75, i32 76, i32 77, i32 78, i32 79, i32 80, i32 81, i32 82, i32 83, i32 84, i32 85, i32 86, i32 87, i32 88, i32 89, i32 90, i32 91, i32 92, i32 93, i32 94, i32 95, i32 96, i32 97, i32 98, i32 99, i32 100, i32 101, i32 102, i32 103, i32 104, i32 105, i32 106, i32 107, i32 108, i32 109, i32 110, i32 111, i32 112, i32 113, i32 114, i32 115, i32 116, i32 117, i32 118, i32 119, i32 120, i32 121, i32 122, i32 123, i32 124, i32 125, i32 126, i32 127>
  %i104 = call <128 x bfloat> @llvm.tpc.fptrunc.swch.v128bf16.v128f32(<128 x float> %i103, i32 0)
  call void @llvm.tpc.st.tnsr.v128bf16(<5 x i32> %.sroa.039.0.vec.insert, i8 6, <128 x bfloat> %i104, i32 0, i1 true, i1 false)
  %i105 = add i32 %i65, 128
  br label %bb64

  bb106:                                            ; preds = %bb64
  %i107 = add i32 %i61, 1
  br label %bb60

  bb108:                                            ; preds = %bb60
  ret void
}

; Function Attrs: nounwind readonly
declare <128 x bfloat> @llvm.tpc.ld.g.v128bf16.i1(i8 addrspace(3)*, i32, <128 x bfloat>, i1, i1) #3

; Function Attrs: nounwind readonly
declare <64 x float> @llvm.tpc.ld.g.v64f32.i1(i8 addrspace(3)*, i32, <64 x float>, i1, i1) #3

attributes #0 = { nounwind readnone }
attributes #1 = { nounwind writeonly }
attributes #2 = { nounwind }
attributes #3 = { nounwind readonly }

