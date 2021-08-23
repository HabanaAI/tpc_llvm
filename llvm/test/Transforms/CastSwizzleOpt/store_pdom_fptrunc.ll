; NOTE: Assertions have been autogenerated by utils/update_test_checks.py
; RUN: opt < %s -cast-swizzle-opt -S | FileCheck %s

declare i8* @malloc(i64)

declare void @free(i8*)

declare i16 @llvm.tpc.ld.g.i16(i8 addrspace(3)*, i32, i16, i1, i1)

declare i32 @llvm.tpc.ld.g.i32(i8 addrspace(3)*, i32, i32, i1, i1)

declare bfloat16 @llvm.tpc.ld.g.bf16(i8 addrspace(3)*, i32, bfloat16, i1, i1)

declare float @llvm.tpc.ld.g.f32(i8 addrspace(3)*, i32, float, i1, i1)

declare <128 x bfloat16> @llvm.tpc.ld.g.v128bf16(i8 addrspace(3)*, i32, <128 x bfloat16>, i1, i1)

declare <128 x bfloat16> @llvm.tpc.ld.tnsr.v128bf16.i1(<5 x i32>, i8, i32, <128 x bfloat16>, i1, i1)

declare void @llvm.tpc.st.tnsr.v128bf16(<5 x i32>, i8, <128 x bfloat16>, i32, i1, i1)

declare <128 x bfloat16> @llvm.tpc.tanh.v128bf16(<128 x bfloat16>)

declare <64 x float> @llvm.tpc.ld.g.v64f32(i8 addrspace(3)*, i32, <64 x float>, i1, i1)

declare <64 x float> @llvm.tpc.ld.tnsr.v64f32.i1(<5 x i32>, i8, i32, <64 x float>, i1, i1)

declare void @llvm.tpc.st.tnsr.v64f32(<5 x i32>, i8, <64 x float>, i32, i1, i1)

declare <64 x float> @llvm.tpc.tanh.v64f32(<64 x float>)

declare <128 x float> @llvm.tpc.ld.g.v128f32(i8 addrspace(3)*, i32, <128 x float>, i1, i1)

declare <128 x float> @llvm.tpc.ld.tnsr.v128f32.i1(<5 x i32>, i8, i32, <128 x float>, i1, i1)

declare void @llvm.tpc.st.tnsr.v128f32(<5 x i32>, i8, <128 x float>, i32, i1, i1)

declare <128 x float> @llvm.tpc.tanh.v128f32(<128 x float>)

declare <256 x i8> @llvm.tpc.ld.g.v256i8(i8 addrspace(3)*, i32, <256 x i8>, i1, i1)

declare <256 x i8> @llvm.tpc.ld.tnsr.v256i8.i1(<5 x i32>, i8, i32, <256 x i8>, i1, i1)

declare void @llvm.tpc.st.tnsr.v256i8(<5 x i32>, i8, <256 x i8>, i32, i1, i1)

declare <256 x i8> @llvm.tpc.tanh.v256i8(<256 x i8>)

declare <128 x i16> @llvm.tpc.ld.g.v128i16(i8 addrspace(3)*, i32, <128 x i16>, i1, i1)

declare <128 x i16> @llvm.tpc.ld.tnsr.v128i16.i1(<5 x i32>, i8, i32, <128 x i16>, i1, i1)

declare void @llvm.tpc.st.tnsr.v128i16(<5 x i32>, i8, <128 x i16>, i32, i1, i1)

declare <128 x i16> @llvm.tpc.tanh.v128i16(<128 x i16>)

declare <64 x i32> @llvm.tpc.ld.g.v64i32(i8 addrspace(3)*, i32, <64 x i32>, i1, i1)

declare <64 x i32> @llvm.tpc.ld.tnsr.v64i32.i1(<5 x i32>, i8, i32, <64 x i32>, i1, i1)

declare void @llvm.tpc.st.tnsr.v64i32(<5 x i32>, i8, <64 x i32>, i32, i1, i1)

declare <64 x i32> @llvm.tpc.tanh.v64i32(<64 x i32>)

declare <256 x i8> @llvm.tpc.read.lfsr()

declare void @llvm.tpc.write.lfsr(<256 x i8>)

declare i8 addrspace(3)* @llvm.tpc.gen.addr(<5 x i32>, i8, i32, i8 addrspace(3)*, i1, i1)

define void @main() !dbg !3 {
; CHECK-LABEL: @main(
; CHECK-NEXT:  bb:
; CHECK-NEXT:    [[I:%.*]] = alloca i32, i32 mul (i32 ptrtoint (i32* getelementptr (i32, i32* null, i32 1) to i32), i32 5), align 4
; CHECK-NEXT:    [[I1:%.*]] = insertvalue { i32*, i32*, i32, [1 x i32], [1 x i32] } undef, i32* [[I]], 0
; CHECK-NEXT:    [[I2:%.*]] = insertvalue { i32*, i32*, i32, [1 x i32], [1 x i32] } [[I1]], i32* [[I]], 1
; CHECK-NEXT:    [[I3:%.*]] = insertvalue { i32*, i32*, i32, [1 x i32], [1 x i32] } [[I2]], i32 0, 2
; CHECK-NEXT:    [[I4:%.*]] = insertvalue { i32*, i32*, i32, [1 x i32], [1 x i32] } [[I3]], i32 5, 3, 0
; CHECK-NEXT:    [[I5:%.*]] = insertvalue { i32*, i32*, i32, [1 x i32], [1 x i32] } [[I4]], i32 1, 4, 0
; CHECK-NEXT:    [[I6:%.*]] = extractvalue { i32*, i32*, i32, [1 x i32], [1 x i32] } [[I5]], 1
; CHECK-NEXT:    [[I7:%.*]] = getelementptr i32, i32* [[I6]], i32 0
; CHECK-NEXT:    [[I8:%.*]] = bitcast i32* [[I7]] to <5 x i32>*
; CHECK-NEXT:    store <5 x i32> zeroinitializer, <5 x i32>* [[I8]], align 4
; CHECK-NEXT:    [[I9:%.*]] = alloca i32, i32 mul (i32 ptrtoint (i32* getelementptr (i32, i32* null, i32 1) to i32), i32 5), align 4
; CHECK-NEXT:    [[I10:%.*]] = insertvalue { i32*, i32*, i32, [1 x i32], [1 x i32] } undef, i32* [[I9]], 0
; CHECK-NEXT:    [[I11:%.*]] = insertvalue { i32*, i32*, i32, [1 x i32], [1 x i32] } [[I10]], i32* [[I9]], 1
; CHECK-NEXT:    [[I12:%.*]] = insertvalue { i32*, i32*, i32, [1 x i32], [1 x i32] } [[I11]], i32 0, 2
; CHECK-NEXT:    [[I13:%.*]] = insertvalue { i32*, i32*, i32, [1 x i32], [1 x i32] } [[I12]], i32 5, 3, 0
; CHECK-NEXT:    [[I14:%.*]] = insertvalue { i32*, i32*, i32, [1 x i32], [1 x i32] } [[I13]], i32 1, 4, 0
; CHECK-NEXT:    [[I15:%.*]] = extractvalue { i32*, i32*, i32, [1 x i32], [1 x i32] } [[I14]], 1
; CHECK-NEXT:    [[I16:%.*]] = getelementptr i32, i32* [[I15]], i32 0
; CHECK-NEXT:    [[I17:%.*]] = bitcast i32* [[I16]] to <5 x i32>*
; CHECK-NEXT:    store <5 x i32> zeroinitializer, <5 x i32>* [[I17]], align 4
; CHECK-NEXT:    [[I18:%.*]] = alloca i32, i32 mul (i32 ptrtoint (i32* getelementptr (i32, i32* null, i32 1) to i32), i32 5), align 4
; CHECK-NEXT:    [[I19:%.*]] = insertvalue { i32*, i32*, i32, [1 x i32], [1 x i32] } undef, i32* [[I18]], 0
; CHECK-NEXT:    [[I20:%.*]] = insertvalue { i32*, i32*, i32, [1 x i32], [1 x i32] } [[I19]], i32* [[I18]], 1
; CHECK-NEXT:    [[I21:%.*]] = insertvalue { i32*, i32*, i32, [1 x i32], [1 x i32] } [[I20]], i32 0, 2
; CHECK-NEXT:    [[I22:%.*]] = insertvalue { i32*, i32*, i32, [1 x i32], [1 x i32] } [[I21]], i32 5, 3, 0
; CHECK-NEXT:    [[I23:%.*]] = insertvalue { i32*, i32*, i32, [1 x i32], [1 x i32] } [[I22]], i32 1, 4, 0
; CHECK-NEXT:    [[I24:%.*]] = extractvalue { i32*, i32*, i32, [1 x i32], [1 x i32] } [[I23]], 1
; CHECK-NEXT:    [[I25:%.*]] = getelementptr i32, i32* [[I24]], i32 0
; CHECK-NEXT:    [[I26:%.*]] = bitcast i32* [[I25]] to <5 x i32>*
; CHECK-NEXT:    store <5 x i32> zeroinitializer, <5 x i32>* [[I26]], align 4
; CHECK-NEXT:    [[I27:%.*]] = alloca i32, i32 mul (i32 ptrtoint (i32* getelementptr (i32, i32* null, i32 1) to i32), i32 5), align 4
; CHECK-NEXT:    [[I28:%.*]] = insertvalue { i32*, i32*, i32, [1 x i32], [1 x i32] } undef, i32* [[I27]], 0
; CHECK-NEXT:    [[I29:%.*]] = insertvalue { i32*, i32*, i32, [1 x i32], [1 x i32] } [[I28]], i32* [[I27]], 1
; CHECK-NEXT:    [[I30:%.*]] = insertvalue { i32*, i32*, i32, [1 x i32], [1 x i32] } [[I29]], i32 0, 2
; CHECK-NEXT:    [[I31:%.*]] = insertvalue { i32*, i32*, i32, [1 x i32], [1 x i32] } [[I30]], i32 5, 3, 0
; CHECK-NEXT:    [[I32:%.*]] = insertvalue { i32*, i32*, i32, [1 x i32], [1 x i32] } [[I31]], i32 1, 4, 0
; CHECK-NEXT:    [[I33:%.*]] = extractvalue { i32*, i32*, i32, [1 x i32], [1 x i32] } [[I32]], 1
; CHECK-NEXT:    [[I34:%.*]] = getelementptr i32, i32* [[I33]], i32 0
; CHECK-NEXT:    [[I35:%.*]] = bitcast i32* [[I34]] to <5 x i32>*
; CHECK-NEXT:    store <5 x i32> zeroinitializer, <5 x i32>* [[I35]], align 4
; CHECK-NEXT:    br label [[BB36:%.*]]
; CHECK:       bb36:
; CHECK-NEXT:    [[I37:%.*]] = phi i32 [ [[I99:%.*]], [[BB98:%.*]] ], [ 0, [[BB:%.*]] ]
; CHECK-NEXT:    [[I38:%.*]] = icmp slt i32 [[I37]], 4
; CHECK-NEXT:    br i1 [[I38]], label [[BB39:%.*]], label [[BB100:%.*]]
; CHECK:       bb39:
; CHECK-NEXT:    [[I40:%.*]] = extractvalue { i32*, i32*, i32, [1 x i32], [1 x i32] } [[I5]], 1
; CHECK-NEXT:    [[I41:%.*]] = getelementptr i32, i32* [[I40]], i32 2
; CHECK-NEXT:    store i32 [[I37]], i32* [[I41]], align 4
; CHECK-NEXT:    [[I42:%.*]] = extractvalue { i32*, i32*, i32, [1 x i32], [1 x i32] } [[I14]], 1
; CHECK-NEXT:    [[I43:%.*]] = getelementptr i32, i32* [[I42]], i32 2
; CHECK-NEXT:    store i32 [[I37]], i32* [[I43]], align 4
; CHECK-NEXT:    [[I44:%.*]] = extractvalue { i32*, i32*, i32, [1 x i32], [1 x i32] } [[I23]], 1
; CHECK-NEXT:    [[I45:%.*]] = getelementptr i32, i32* [[I44]], i32 2
; CHECK-NEXT:    store i32 [[I37]], i32* [[I45]], align 4
; CHECK-NEXT:    [[I46:%.*]] = extractvalue { i32*, i32*, i32, [1 x i32], [1 x i32] } [[I32]], 1
; CHECK-NEXT:    [[I47:%.*]] = getelementptr i32, i32* [[I46]], i32 2
; CHECK-NEXT:    store i32 [[I37]], i32* [[I47]], align 4
; CHECK-NEXT:    br label [[BB48:%.*]]
; CHECK:       bb48:
; CHECK-NEXT:    [[I49:%.*]] = phi i32 [ [[I97:%.*]], [[BB96:%.*]] ], [ 0, [[BB39]] ]
; CHECK-NEXT:    [[I50:%.*]] = icmp slt i32 [[I49]], 2
; CHECK-NEXT:    br i1 [[I50]], label [[BB51:%.*]], label [[BB98]]
; CHECK:       bb51:
; CHECK-NEXT:    [[I52:%.*]] = extractvalue { i32*, i32*, i32, [1 x i32], [1 x i32] } [[I5]], 1
; CHECK-NEXT:    [[I53:%.*]] = getelementptr i32, i32* [[I52]], i32 1
; CHECK-NEXT:    store i32 [[I49]], i32* [[I53]], align 4
; CHECK-NEXT:    [[I54:%.*]] = extractvalue { i32*, i32*, i32, [1 x i32], [1 x i32] } [[I14]], 1
; CHECK-NEXT:    [[I55:%.*]] = getelementptr i32, i32* [[I54]], i32 1
; CHECK-NEXT:    store i32 [[I49]], i32* [[I55]], align 4
; CHECK-NEXT:    [[I56:%.*]] = extractvalue { i32*, i32*, i32, [1 x i32], [1 x i32] } [[I23]], 1
; CHECK-NEXT:    [[I57:%.*]] = getelementptr i32, i32* [[I56]], i32 1
; CHECK-NEXT:    store i32 [[I49]], i32* [[I57]], align 4
; CHECK-NEXT:    [[I58:%.*]] = extractvalue { i32*, i32*, i32, [1 x i32], [1 x i32] } [[I32]], 1
; CHECK-NEXT:    [[I59:%.*]] = getelementptr i32, i32* [[I58]], i32 1
; CHECK-NEXT:    store i32 [[I49]], i32* [[I59]], align 4
; CHECK-NEXT:    br label [[BB60:%.*]]
; CHECK:       bb60:
; CHECK-NEXT:    [[I61:%.*]] = phi i32 [ [[I95:%.*]], [[BB63:%.*]] ], [ 0, [[BB51]] ]
; CHECK-NEXT:    [[I62:%.*]] = icmp slt i32 [[I61]], 128
; CHECK-NEXT:    br i1 [[I62]], label [[BB63]], label [[BB96]]
; CHECK:       bb63:
; CHECK-NEXT:    [[I64:%.*]] = extractvalue { i32*, i32*, i32, [1 x i32], [1 x i32] } [[I5]], 1
; CHECK-NEXT:    [[I65:%.*]] = getelementptr i32, i32* [[I64]], i32 0
; CHECK-NEXT:    store i32 [[I61]], i32* [[I65]], align 4
; CHECK-NEXT:    [[I66:%.*]] = extractvalue { i32*, i32*, i32, [1 x i32], [1 x i32] } [[I14]], 1
; CHECK-NEXT:    [[I67:%.*]] = getelementptr i32, i32* [[I66]], i32 0
; CHECK-NEXT:    store i32 [[I61]], i32* [[I67]], align 4
; CHECK-NEXT:    [[I68:%.*]] = extractvalue { i32*, i32*, i32, [1 x i32], [1 x i32] } [[I32]], 1
; CHECK-NEXT:    [[I69:%.*]] = getelementptr i32, i32* [[I68]], i32 0
; CHECK-NEXT:    store i32 [[I61]], i32* [[I69]], align 4
; CHECK-NEXT:    [[I70:%.*]] = extractvalue { i32*, i32*, i32, [1 x i32], [1 x i32] } [[I32]], 1
; CHECK-NEXT:    [[I71:%.*]] = getelementptr i32, i32* [[I70]], i32 0
; CHECK-NEXT:    [[I72:%.*]] = bitcast i32* [[I71]] to <5 x i32>*
; CHECK-NEXT:    [[I73:%.*]] = load <5 x i32>, <5 x i32>* [[I72]], align 4
; CHECK-NEXT:    [[I74:%.*]] = call <64 x float> @llvm.tpc.ld.tnsr.v64f32.i1(<5 x i32> [[I73]], i8 0, i32 0, <64 x float> undef, i1 true, i1 false)
; CHECK-NEXT:    [[I75:%.*]] = add i32 [[I61]], 64
; CHECK-NEXT:    [[I76:%.*]] = extractvalue { i32*, i32*, i32, [1 x i32], [1 x i32] } [[I23]], 1
; CHECK-NEXT:    [[I77:%.*]] = getelementptr i32, i32* [[I76]], i32 0
; CHECK-NEXT:    store i32 [[I75]], i32* [[I77]], align 4
; CHECK-NEXT:    [[I78:%.*]] = extractvalue { i32*, i32*, i32, [1 x i32], [1 x i32] } [[I23]], 1
; CHECK-NEXT:    [[I79:%.*]] = getelementptr i32, i32* [[I78]], i32 0
; CHECK-NEXT:    [[I80:%.*]] = bitcast i32* [[I79]] to <5 x i32>*
; CHECK-NEXT:    [[I81:%.*]] = load <5 x i32>, <5 x i32>* [[I80]], align 4
; CHECK-NEXT:    [[I82:%.*]] = call <64 x float> @llvm.tpc.ld.tnsr.v64f32.i1(<5 x i32> [[I81]], i8 0, i32 0, <64 x float> undef, i1 true, i1 false)
; CHECK-NEXT:    [[I83:%.*]] = shufflevector <64 x float> [[I74]], <64 x float> [[I82]], <128 x i32> <i32 0, i32 1, i32 2, i32 3, i32 4, i32 5, i32 6, i32 7, i32 8, i32 9, i32 10, i32 11, i32 12, i32 13, i32 14, i32 15, i32 16, i32 17, i32 18, i32 19, i32 20, i32 21, i32 22, i32 23, i32 24, i32 25, i32 26, i32 27, i32 28, i32 29, i32 30, i32 31, i32 32, i32 33, i32 34, i32 35, i32 36, i32 37, i32 38, i32 39, i32 40, i32 41, i32 42, i32 43, i32 44, i32 45, i32 46, i32 47, i32 48, i32 49, i32 50, i32 51, i32 52, i32 53, i32 54, i32 55, i32 56, i32 57, i32 58, i32 59, i32 60, i32 61, i32 62, i32 63, i32 64, i32 65, i32 66, i32 67, i32 68, i32 69, i32 70, i32 71, i32 72, i32 73, i32 74, i32 75, i32 76, i32 77, i32 78, i32 79, i32 80, i32 81, i32 82, i32 83, i32 84, i32 85, i32 86, i32 87, i32 88, i32 89, i32 90, i32 91, i32 92, i32 93, i32 94, i32 95, i32 96, i32 97, i32 98, i32 99, i32 100, i32 101, i32 102, i32 103, i32 104, i32 105, i32 106, i32 107, i32 108, i32 109, i32 110, i32 111, i32 112, i32 113, i32 114, i32 115, i32 116, i32 117, i32 118, i32 119, i32 120, i32 121, i32 122, i32 123, i32 124, i32 125, i32 126, i32 127>
; CHECK-NEXT:    [[I84:%.*]] = fptrunc <128 x float> [[I83]] to <128 x bfloat>
; CHECK-NEXT:    [[I85:%.*]] = extractvalue { i32*, i32*, i32, [1 x i32], [1 x i32] } [[I14]], 1
; CHECK-NEXT:    [[I86:%.*]] = getelementptr i32, i32* [[I85]], i32 0
; CHECK-NEXT:    [[I87:%.*]] = bitcast i32* [[I86]] to <5 x i32>*
; CHECK-NEXT:    [[I88:%.*]] = load <5 x i32>, <5 x i32>* [[I87]], align 4
; CHECK-NEXT:    [[I89:%.*]] = call <128 x bfloat> @llvm.tpc.ld.tnsr.v128bf16.i1(<5 x i32> [[I88]], i8 1, i32 0, <128 x bfloat> undef, i1 true, i1 false)
; CHECK-NEXT:    [[I90:%.*]] = fadd <128 x bfloat> [[I84]], [[I89]]
; CHECK-NEXT:    [[I91:%.*]] = extractvalue { i32*, i32*, i32, [1 x i32], [1 x i32] } [[I5]], 1
; CHECK-NEXT:    [[I92:%.*]] = getelementptr i32, i32* [[I91]], i32 0
; CHECK-NEXT:    [[I93:%.*]] = bitcast i32* [[I92]] to <5 x i32>*
; CHECK-NEXT:    [[I94:%.*]] = load <5 x i32>, <5 x i32>* [[I93]], align 4
; CHECK-NEXT:    call void @llvm.tpc.st.tnsr.v128bf16(<5 x i32> [[I94]], i8 2, <128 x bfloat> [[I90]], i32 0, i1 true, i1 false)
; CHECK-NEXT:    [[I95]] = add i32 [[I61]], 128
; CHECK-NEXT:    br label [[BB60]]
; CHECK:       bb96:
; CHECK-NEXT:    [[I97]] = add i32 [[I49]], 1
; CHECK-NEXT:    br label [[BB48]]
; CHECK:       bb98:
; CHECK-NEXT:    [[I99]] = add i32 [[I37]], 1
; CHECK-NEXT:    br label [[BB36]]
; CHECK:       bb100:
; CHECK-NEXT:    ret void
;
bb:
  %i = alloca i32, i32 mul (i32 ptrtoint (i32* getelementptr (i32, i32* null, i32 1) to i32), i32 5), align 4, !dbg !7
  %i1 = insertvalue { i32*, i32*, i32, [1 x i32], [1 x i32] } undef, i32* %i, 0, !dbg !9
  %i2 = insertvalue { i32*, i32*, i32, [1 x i32], [1 x i32] } %i1, i32* %i, 1, !dbg !10
  %i3 = insertvalue { i32*, i32*, i32, [1 x i32], [1 x i32] } %i2, i32 0, 2, !dbg !11
  %i4 = insertvalue { i32*, i32*, i32, [1 x i32], [1 x i32] } %i3, i32 5, 3, 0, !dbg !12
  %i5 = insertvalue { i32*, i32*, i32, [1 x i32], [1 x i32] } %i4, i32 1, 4, 0, !dbg !13
  %i6 = extractvalue { i32*, i32*, i32, [1 x i32], [1 x i32] } %i5, 1, !dbg !14
  %i7 = getelementptr i32, i32* %i6, i32 0, !dbg !15
  %i8 = bitcast i32* %i7 to <5 x i32>*, !dbg !16
  store <5 x i32> zeroinitializer, <5 x i32>* %i8, align 4, !dbg !17
  %i9 = alloca i32, i32 mul (i32 ptrtoint (i32* getelementptr (i32, i32* null, i32 1) to i32), i32 5), align 4, !dbg !18
  %i10 = insertvalue { i32*, i32*, i32, [1 x i32], [1 x i32] } undef, i32* %i9, 0, !dbg !19
  %i11 = insertvalue { i32*, i32*, i32, [1 x i32], [1 x i32] } %i10, i32* %i9, 1, !dbg !20
  %i12 = insertvalue { i32*, i32*, i32, [1 x i32], [1 x i32] } %i11, i32 0, 2, !dbg !21
  %i13 = insertvalue { i32*, i32*, i32, [1 x i32], [1 x i32] } %i12, i32 5, 3, 0, !dbg !22
  %i14 = insertvalue { i32*, i32*, i32, [1 x i32], [1 x i32] } %i13, i32 1, 4, 0, !dbg !23
  %i15 = extractvalue { i32*, i32*, i32, [1 x i32], [1 x i32] } %i14, 1, !dbg !24
  %i16 = getelementptr i32, i32* %i15, i32 0, !dbg !25
  %i17 = bitcast i32* %i16 to <5 x i32>*, !dbg !26
  store <5 x i32> zeroinitializer, <5 x i32>* %i17, align 4, !dbg !27
  %i18 = alloca i32, i32 mul (i32 ptrtoint (i32* getelementptr (i32, i32* null, i32 1) to i32), i32 5), align 4, !dbg !28
  %i19 = insertvalue { i32*, i32*, i32, [1 x i32], [1 x i32] } undef, i32* %i18, 0, !dbg !29
  %i20 = insertvalue { i32*, i32*, i32, [1 x i32], [1 x i32] } %i19, i32* %i18, 1, !dbg !30
  %i21 = insertvalue { i32*, i32*, i32, [1 x i32], [1 x i32] } %i20, i32 0, 2, !dbg !31
  %i22 = insertvalue { i32*, i32*, i32, [1 x i32], [1 x i32] } %i21, i32 5, 3, 0, !dbg !32
  %i23 = insertvalue { i32*, i32*, i32, [1 x i32], [1 x i32] } %i22, i32 1, 4, 0, !dbg !33
  %i24 = extractvalue { i32*, i32*, i32, [1 x i32], [1 x i32] } %i23, 1, !dbg !34
  %i25 = getelementptr i32, i32* %i24, i32 0, !dbg !35
  %i26 = bitcast i32* %i25 to <5 x i32>*, !dbg !36
  store <5 x i32> zeroinitializer, <5 x i32>* %i26, align 4, !dbg !37
  %i27 = alloca i32, i32 mul (i32 ptrtoint (i32* getelementptr (i32, i32* null, i32 1) to i32), i32 5), align 4, !dbg !38
  %i28 = insertvalue { i32*, i32*, i32, [1 x i32], [1 x i32] } undef, i32* %i27, 0, !dbg !39
  %i29 = insertvalue { i32*, i32*, i32, [1 x i32], [1 x i32] } %i28, i32* %i27, 1, !dbg !40
  %i30 = insertvalue { i32*, i32*, i32, [1 x i32], [1 x i32] } %i29, i32 0, 2, !dbg !41
  %i31 = insertvalue { i32*, i32*, i32, [1 x i32], [1 x i32] } %i30, i32 5, 3, 0, !dbg !42
  %i32 = insertvalue { i32*, i32*, i32, [1 x i32], [1 x i32] } %i31, i32 1, 4, 0, !dbg !43
  %i33 = extractvalue { i32*, i32*, i32, [1 x i32], [1 x i32] } %i32, 1, !dbg !44
  %i34 = getelementptr i32, i32* %i33, i32 0, !dbg !45
  %i35 = bitcast i32* %i34 to <5 x i32>*, !dbg !46
  store <5 x i32> zeroinitializer, <5 x i32>* %i35, align 4, !dbg !47
  br label %bb36, !dbg !48

bb36:                                             ; preds = %bb98, %bb
  %i37 = phi i32 [ %i99, %bb98 ], [ 0, %bb ]
  %i38 = icmp slt i32 %i37, 4, !dbg !49
  br i1 %i38, label %bb39, label %bb100, !dbg !50

bb39:                                             ; preds = %bb36
  %i40 = extractvalue { i32*, i32*, i32, [1 x i32], [1 x i32] } %i5, 1, !dbg !51
  %i41 = getelementptr i32, i32* %i40, i32 2, !dbg !52
  store i32 %i37, i32* %i41, align 4, !dbg !53
  %i42 = extractvalue { i32*, i32*, i32, [1 x i32], [1 x i32] } %i14, 1, !dbg !54
  %i43 = getelementptr i32, i32* %i42, i32 2, !dbg !55
  store i32 %i37, i32* %i43, align 4, !dbg !56
  %i44 = extractvalue { i32*, i32*, i32, [1 x i32], [1 x i32] } %i23, 1, !dbg !57
  %i45 = getelementptr i32, i32* %i44, i32 2, !dbg !58
  store i32 %i37, i32* %i45, align 4, !dbg !59
  %i46 = extractvalue { i32*, i32*, i32, [1 x i32], [1 x i32] } %i32, 1, !dbg !60
  %i47 = getelementptr i32, i32* %i46, i32 2, !dbg !61
  store i32 %i37, i32* %i47, align 4, !dbg !62
  br label %bb48, !dbg !63

bb48:                                             ; preds = %bb96, %bb39
  %i49 = phi i32 [ %i97, %bb96 ], [ 0, %bb39 ]
  %i50 = icmp slt i32 %i49, 2, !dbg !64
  br i1 %i50, label %bb51, label %bb98, !dbg !65

bb51:                                             ; preds = %bb48
  %i52 = extractvalue { i32*, i32*, i32, [1 x i32], [1 x i32] } %i5, 1, !dbg !66
  %i53 = getelementptr i32, i32* %i52, i32 1, !dbg !67
  store i32 %i49, i32* %i53, align 4, !dbg !68
  %i54 = extractvalue { i32*, i32*, i32, [1 x i32], [1 x i32] } %i14, 1, !dbg !69
  %i55 = getelementptr i32, i32* %i54, i32 1, !dbg !70
  store i32 %i49, i32* %i55, align 4, !dbg !71
  %i56 = extractvalue { i32*, i32*, i32, [1 x i32], [1 x i32] } %i23, 1, !dbg !72
  %i57 = getelementptr i32, i32* %i56, i32 1, !dbg !73
  store i32 %i49, i32* %i57, align 4, !dbg !74
  %i58 = extractvalue { i32*, i32*, i32, [1 x i32], [1 x i32] } %i32, 1, !dbg !75
  %i59 = getelementptr i32, i32* %i58, i32 1, !dbg !76
  store i32 %i49, i32* %i59, align 4, !dbg !77
  br label %bb60, !dbg !78

bb60:                                             ; preds = %bb63, %bb51
  %i61 = phi i32 [ %i95, %bb63 ], [ 0, %bb51 ]
  %i62 = icmp slt i32 %i61, 128, !dbg !79
  br i1 %i62, label %bb63, label %bb96, !dbg !80

bb63:                                             ; preds = %bb60
  %i64 = extractvalue { i32*, i32*, i32, [1 x i32], [1 x i32] } %i5, 1, !dbg !81
  %i65 = getelementptr i32, i32* %i64, i32 0, !dbg !82
  store i32 %i61, i32* %i65, align 4, !dbg !83
  %i66 = extractvalue { i32*, i32*, i32, [1 x i32], [1 x i32] } %i14, 1, !dbg !84
  %i67 = getelementptr i32, i32* %i66, i32 0, !dbg !85
  store i32 %i61, i32* %i67, align 4, !dbg !86
  %i68 = extractvalue { i32*, i32*, i32, [1 x i32], [1 x i32] } %i32, 1, !dbg !87
  %i69 = getelementptr i32, i32* %i68, i32 0, !dbg !88
  store i32 %i61, i32* %i69, align 4, !dbg !89
  %i70 = extractvalue { i32*, i32*, i32, [1 x i32], [1 x i32] } %i32, 1, !dbg !90
  %i71 = getelementptr i32, i32* %i70, i32 0, !dbg !91
  %i72 = bitcast i32* %i71 to <5 x i32>*, !dbg !92
  %i73 = load <5 x i32>, <5 x i32>* %i72, align 4, !dbg !93
  %i74 = call <64 x float> @llvm.tpc.ld.tnsr.v64f32.i1(<5 x i32> %i73, i8 0, i32 0, <64 x float> undef, i1 true, i1 false), !dbg !94
  %i75 = add i32 %i61, 64, !dbg !95
  %i76 = extractvalue { i32*, i32*, i32, [1 x i32], [1 x i32] } %i23, 1, !dbg !96
  %i77 = getelementptr i32, i32* %i76, i32 0, !dbg !97
  store i32 %i75, i32* %i77, align 4, !dbg !98
  %i78 = extractvalue { i32*, i32*, i32, [1 x i32], [1 x i32] } %i23, 1, !dbg !99
  %i79 = getelementptr i32, i32* %i78, i32 0, !dbg !100
  %i80 = bitcast i32* %i79 to <5 x i32>*, !dbg !101
  %i81 = load <5 x i32>, <5 x i32>* %i80, align 4, !dbg !102
  %i82 = call <64 x float> @llvm.tpc.ld.tnsr.v64f32.i1(<5 x i32> %i81, i8 0, i32 0, <64 x float> undef, i1 true, i1 false), !dbg !103
  %i83 = shufflevector <64 x float> %i74, <64 x float> %i82, <128 x i32> <i32 0, i32 1, i32 2, i32 3, i32 4, i32 5, i32 6, i32 7, i32 8, i32 9, i32 10, i32 11, i32 12, i32 13, i32 14, i32 15, i32 16, i32 17, i32 18, i32 19, i32 20, i32 21, i32 22, i32 23, i32 24, i32 25, i32 26, i32 27, i32 28, i32 29, i32 30, i32 31, i32 32, i32 33, i32 34, i32 35, i32 36, i32 37, i32 38, i32 39, i32 40, i32 41, i32 42, i32 43, i32 44, i32 45, i32 46, i32 47, i32 48, i32 49, i32 50, i32 51, i32 52, i32 53, i32 54, i32 55, i32 56, i32 57, i32 58, i32 59, i32 60, i32 61, i32 62, i32 63, i32 64, i32 65, i32 66, i32 67, i32 68, i32 69, i32 70, i32 71, i32 72, i32 73, i32 74, i32 75, i32 76, i32 77, i32 78, i32 79, i32 80, i32 81, i32 82, i32 83, i32 84, i32 85, i32 86, i32 87, i32 88, i32 89, i32 90, i32 91, i32 92, i32 93, i32 94, i32 95, i32 96, i32 97, i32 98, i32 99, i32 100, i32 101, i32 102, i32 103, i32 104, i32 105, i32 106, i32 107, i32 108, i32 109, i32 110, i32 111, i32 112, i32 113, i32 114, i32 115, i32 116, i32 117, i32 118, i32 119, i32 120, i32 121, i32 122, i32 123, i32 124, i32 125, i32 126, i32 127>, !dbg !104
  %i84 = fptrunc <128 x float> %i83 to <128 x bfloat16>, !dbg !105
  %i85 = extractvalue { i32*, i32*, i32, [1 x i32], [1 x i32] } %i14, 1, !dbg !106
  %i86 = getelementptr i32, i32* %i85, i32 0, !dbg !107
  %i87 = bitcast i32* %i86 to <5 x i32>*, !dbg !108
  %i88 = load <5 x i32>, <5 x i32>* %i87, align 4, !dbg !109
  %i89 = call <128 x bfloat16> @llvm.tpc.ld.tnsr.v128bf16.i1(<5 x i32> %i88, i8 1, i32 0, <128 x bfloat16> undef, i1 true, i1 false), !dbg !110
  %i90 = fadd <128 x bfloat16> %i84, %i89, !dbg !111
  %i91 = extractvalue { i32*, i32*, i32, [1 x i32], [1 x i32] } %i5, 1, !dbg !112
  %i92 = getelementptr i32, i32* %i91, i32 0, !dbg !113
  %i93 = bitcast i32* %i92 to <5 x i32>*, !dbg !114
  %i94 = load <5 x i32>, <5 x i32>* %i93, align 4, !dbg !115
  call void @llvm.tpc.st.tnsr.v128bf16(<5 x i32> %i94, i8 2, <128 x bfloat16> %i90, i32 0, i1 true, i1 false), !dbg !116
  %i95 = add i32 %i61, 128, !dbg !117
  br label %bb60, !dbg !118

bb96:                                             ; preds = %bb60
  %i97 = add i32 %i49, 1, !dbg !119
  br label %bb48, !dbg !120

bb98:                                             ; preds = %bb48
  %i99 = add i32 %i37, 1, !dbg !121
  br label %bb36, !dbg !122

bb100:                                            ; preds = %bb36
  ret void, !dbg !123
}

!llvm.dbg.cu = !{!0}
!llvm.module.flags = !{!2}

!0 = distinct !DICompileUnit(language: DW_LANG_C, file: !1, producer: "mlir", isOptimized: true, runtimeVersion: 0, emissionKind: FullDebug)
!1 = !DIFile(filename: "LLVMDialectModule", directory: "/")
!2 = !{i32 2, !"Debug Info Version", i32 3}
!3 = distinct !DISubprogram(name: "main", linkageName: "main", scope: null, file: !4, line: 35, type: !5, scopeLine: 35,  retainedNodes: !6)
!4 = !DIFile(filename: "b.mlir", directory: "/home/aashahid/mytasks/cast-swizzle")
!5 = !DISubroutineType(types: !6)
!6 = !{}
!7 = !DILocation(line: 54, column: 11, scope: !8)
!8 = !DILexicalBlockFile(scope: !3, file: !4, discriminator: 0)
!9 = !DILocation(line: 56, column: 11, scope: !8)
!10 = !DILocation(line: 57, column: 11, scope: !8)
!11 = !DILocation(line: 59, column: 11, scope: !8)
!12 = !DILocation(line: 61, column: 11, scope: !8)
!13 = !DILocation(line: 62, column: 11, scope: !8)
!14 = !DILocation(line: 63, column: 11, scope: !8)
!15 = !DILocation(line: 68, column: 11, scope: !8)
!16 = !DILocation(line: 69, column: 11, scope: !8)
!17 = !DILocation(line: 70, column: 5, scope: !8)
!18 = !DILocation(line: 78, column: 11, scope: !8)
!19 = !DILocation(line: 80, column: 11, scope: !8)
!20 = !DILocation(line: 81, column: 11, scope: !8)
!21 = !DILocation(line: 83, column: 11, scope: !8)
!22 = !DILocation(line: 85, column: 11, scope: !8)
!23 = !DILocation(line: 86, column: 11, scope: !8)
!24 = !DILocation(line: 87, column: 11, scope: !8)
!25 = !DILocation(line: 92, column: 11, scope: !8)
!26 = !DILocation(line: 93, column: 11, scope: !8)
!27 = !DILocation(line: 94, column: 5, scope: !8)
!28 = !DILocation(line: 102, column: 11, scope: !8)
!29 = !DILocation(line: 104, column: 11, scope: !8)
!30 = !DILocation(line: 105, column: 11, scope: !8)
!31 = !DILocation(line: 107, column: 11, scope: !8)
!32 = !DILocation(line: 109, column: 11, scope: !8)
!33 = !DILocation(line: 110, column: 11, scope: !8)
!34 = !DILocation(line: 111, column: 11, scope: !8)
!35 = !DILocation(line: 116, column: 11, scope: !8)
!36 = !DILocation(line: 117, column: 11, scope: !8)
!37 = !DILocation(line: 118, column: 5, scope: !8)
!38 = !DILocation(line: 126, column: 11, scope: !8)
!39 = !DILocation(line: 128, column: 11, scope: !8)
!40 = !DILocation(line: 129, column: 11, scope: !8)
!41 = !DILocation(line: 131, column: 11, scope: !8)
!42 = !DILocation(line: 133, column: 11, scope: !8)
!43 = !DILocation(line: 134, column: 11, scope: !8)
!44 = !DILocation(line: 135, column: 11, scope: !8)
!45 = !DILocation(line: 140, column: 12, scope: !8)
!46 = !DILocation(line: 141, column: 12, scope: !8)
!47 = !DILocation(line: 142, column: 5, scope: !8)
!48 = !DILocation(line: 143, column: 5, scope: !8)
!49 = !DILocation(line: 145, column: 12, scope: !8)
!50 = !DILocation(line: 146, column: 5, scope: !8)
!51 = !DILocation(line: 148, column: 12, scope: !8)
!52 = !DILocation(line: 153, column: 12, scope: !8)
!53 = !DILocation(line: 154, column: 5, scope: !8)
!54 = !DILocation(line: 155, column: 12, scope: !8)
!55 = !DILocation(line: 160, column: 12, scope: !8)
!56 = !DILocation(line: 161, column: 5, scope: !8)
!57 = !DILocation(line: 162, column: 12, scope: !8)
!58 = !DILocation(line: 167, column: 12, scope: !8)
!59 = !DILocation(line: 168, column: 5, scope: !8)
!60 = !DILocation(line: 169, column: 12, scope: !8)
!61 = !DILocation(line: 174, column: 12, scope: !8)
!62 = !DILocation(line: 175, column: 5, scope: !8)
!63 = !DILocation(line: 176, column: 5, scope: !8)
!64 = !DILocation(line: 178, column: 12, scope: !8)
!65 = !DILocation(line: 179, column: 5, scope: !8)
!66 = !DILocation(line: 181, column: 12, scope: !8)
!67 = !DILocation(line: 186, column: 12, scope: !8)
!68 = !DILocation(line: 187, column: 5, scope: !8)
!69 = !DILocation(line: 188, column: 12, scope: !8)
!70 = !DILocation(line: 193, column: 12, scope: !8)
!71 = !DILocation(line: 194, column: 5, scope: !8)
!72 = !DILocation(line: 195, column: 12, scope: !8)
!73 = !DILocation(line: 200, column: 12, scope: !8)
!74 = !DILocation(line: 201, column: 5, scope: !8)
!75 = !DILocation(line: 202, column: 12, scope: !8)
!76 = !DILocation(line: 207, column: 12, scope: !8)
!77 = !DILocation(line: 208, column: 5, scope: !8)
!78 = !DILocation(line: 209, column: 5, scope: !8)
!79 = !DILocation(line: 211, column: 12, scope: !8)
!80 = !DILocation(line: 212, column: 5, scope: !8)
!81 = !DILocation(line: 214, column: 12, scope: !8)
!82 = !DILocation(line: 219, column: 12, scope: !8)
!83 = !DILocation(line: 220, column: 5, scope: !8)
!84 = !DILocation(line: 221, column: 12, scope: !8)
!85 = !DILocation(line: 226, column: 12, scope: !8)
!86 = !DILocation(line: 227, column: 5, scope: !8)
!87 = !DILocation(line: 228, column: 12, scope: !8)
!88 = !DILocation(line: 233, column: 12, scope: !8)
!89 = !DILocation(line: 234, column: 5, scope: !8)
!90 = !DILocation(line: 235, column: 12, scope: !8)
!91 = !DILocation(line: 240, column: 12, scope: !8)
!92 = !DILocation(line: 241, column: 12, scope: !8)
!93 = !DILocation(line: 242, column: 12, scope: !8)
!94 = !DILocation(line: 247, column: 12, scope: !8)
!95 = !DILocation(line: 248, column: 12, scope: !8)
!96 = !DILocation(line: 249, column: 12, scope: !8)
!97 = !DILocation(line: 254, column: 12, scope: !8)
!98 = !DILocation(line: 255, column: 5, scope: !8)
!99 = !DILocation(line: 256, column: 12, scope: !8)
!100 = !DILocation(line: 261, column: 12, scope: !8)
!101 = !DILocation(line: 262, column: 12, scope: !8)
!102 = !DILocation(line: 263, column: 12, scope: !8)
!103 = !DILocation(line: 268, column: 12, scope: !8)
!104 = !DILocation(line: 269, column: 12, scope: !8)
!105 = !DILocation(line: 270, column: 12, scope: !8)
!106 = !DILocation(line: 271, column: 12, scope: !8)
!107 = !DILocation(line: 276, column: 12, scope: !8)
!108 = !DILocation(line: 277, column: 12, scope: !8)
!109 = !DILocation(line: 278, column: 12, scope: !8)
!110 = !DILocation(line: 283, column: 12, scope: !8)
!111 = !DILocation(line: 284, column: 12, scope: !8)
!112 = !DILocation(line: 285, column: 12, scope: !8)
!113 = !DILocation(line: 290, column: 12, scope: !8)
!114 = !DILocation(line: 291, column: 12, scope: !8)
!115 = !DILocation(line: 292, column: 12, scope: !8)
!116 = !DILocation(line: 296, column: 12, scope: !8)
!117 = !DILocation(line: 297, column: 12, scope: !8)
!118 = !DILocation(line: 298, column: 5, scope: !8)
!119 = !DILocation(line: 300, column: 12, scope: !8)
!120 = !DILocation(line: 301, column: 5, scope: !8)
!121 = !DILocation(line: 303, column: 12, scope: !8)
!122 = !DILocation(line: 304, column: 5, scope: !8)
!123 = !DILocation(line: 306, column: 5, scope: !8)
