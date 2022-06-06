; RUN: tpc-clang -S -O3 -march=gaudi2 %s
target triple = "tpc"

; ModuleID = 'mlir_kernel_tr.ll'
source_filename = "LLVMDialectModule"

declare i8* @malloc(i64)

declare void @free(i8*)

; Function Attrs: nounwind readnone
declare <5 x i32> @llvm.tpc.get.index.space.offset() #0

; Function Attrs: nounwind readnone
declare <5 x i32> @llvm.tpc.get.index.space.size() #0

define void @main() #3 {
bb:
  %tmp = alloca i32, i32 mul (i32 ptrtoint (i32* getelementptr (i32, i32* null, i32 1) to i32), i32 5), align 4
  %tmp1 = insertvalue { i32*, i32*, i32, [1 x i32], [1 x i32] } undef, i32* %tmp, 0
  %tmp2 = insertvalue { i32*, i32*, i32, [1 x i32], [1 x i32] } %tmp1, i32* %tmp, 1
  %tmp3 = insertvalue { i32*, i32*, i32, [1 x i32], [1 x i32] } %tmp2, i32 0, 2
  %tmp4 = insertvalue { i32*, i32*, i32, [1 x i32], [1 x i32] } %tmp3, i32 5, 3, 0
  %tmp5 = insertvalue { i32*, i32*, i32, [1 x i32], [1 x i32] } %tmp4, i32 1, 4, 0
  %tmp6 = extractvalue { i32*, i32*, i32, [1 x i32], [1 x i32] } %tmp5, 1
  %tmp7 = getelementptr i32, i32* %tmp6, i32 0
  %tmp8 = bitcast i32* %tmp7 to <5 x i32>*
  store <5 x i32> zeroinitializer, <5 x i32>* %tmp8, align 4
  %tmp9 = alloca i32, i32 mul (i32 ptrtoint (i32* getelementptr (i32, i32* null, i32 1) to i32), i32 5), align 4
  %tmp10 = insertvalue { i32*, i32*, i32, [1 x i32], [1 x i32] } undef, i32* %tmp9, 0
  %tmp11 = insertvalue { i32*, i32*, i32, [1 x i32], [1 x i32] } %tmp10, i32* %tmp9, 1
  %tmp12 = insertvalue { i32*, i32*, i32, [1 x i32], [1 x i32] } %tmp11, i32 0, 2
  %tmp13 = insertvalue { i32*, i32*, i32, [1 x i32], [1 x i32] } %tmp12, i32 5, 3, 0
  %tmp14 = insertvalue { i32*, i32*, i32, [1 x i32], [1 x i32] } %tmp13, i32 1, 4, 0
  %tmp15 = extractvalue { i32*, i32*, i32, [1 x i32], [1 x i32] } %tmp14, 1
  %tmp16 = getelementptr i32, i32* %tmp15, i32 0
  %tmp17 = bitcast i32* %tmp16 to <5 x i32>*
  store <5 x i32> zeroinitializer, <5 x i32>* %tmp17, align 4
  %tmp18 = alloca i32, i32 mul (i32 ptrtoint (i32* getelementptr (i32, i32* null, i32 1) to i32), i32 5), align 4
  %tmp19 = insertvalue { i32*, i32*, i32, [1 x i32], [1 x i32] } undef, i32* %tmp18, 0
  %tmp20 = insertvalue { i32*, i32*, i32, [1 x i32], [1 x i32] } %tmp19, i32* %tmp18, 1
  %tmp21 = insertvalue { i32*, i32*, i32, [1 x i32], [1 x i32] } %tmp20, i32 0, 2
  %tmp22 = insertvalue { i32*, i32*, i32, [1 x i32], [1 x i32] } %tmp21, i32 5, 3, 0
  %tmp23 = insertvalue { i32*, i32*, i32, [1 x i32], [1 x i32] } %tmp22, i32 1, 4, 0
  %tmp24 = extractvalue { i32*, i32*, i32, [1 x i32], [1 x i32] } %tmp23, 1
  %tmp25 = getelementptr i32, i32* %tmp24, i32 0
  %tmp26 = bitcast i32* %tmp25 to <5 x i32>*
  store <5 x i32> zeroinitializer, <5 x i32>* %tmp26, align 4
  %tmp27 = call <5 x i32> @llvm.tpc.get.index.space.offset()
  %tmp28 = call <5 x i32> @llvm.tpc.get.index.space.size()
  %tmp29 = add <5 x i32> %tmp27, %tmp28
  %tmp30 = extractelement <5 x i32> %tmp27, i32 0
  %tmp31 = mul i32 %tmp30, 128
  %tmp32 = extractelement <5 x i32> %tmp29, i32 0
  %tmp33 = mul i32 %tmp32, 128
  br label %bb34

bb34:                                             ; preds = %bb37, %bb
  %tmp35 = phi i32 [ %tmp62, %bb37 ], [ %tmp31, %bb ]
  %tmp36 = icmp slt i32 %tmp35, %tmp33
  br i1 %tmp36, label %bb37, label %bb63

bb37:                                             ; preds = %bb34
  %tmp38 = extractvalue { i32*, i32*, i32, [1 x i32], [1 x i32] } %tmp5, 1
  %tmp39 = getelementptr i32, i32* %tmp38, i32 0
  store i32 %tmp35, i32* %tmp39, align 4
  %tmp40 = extractvalue { i32*, i32*, i32, [1 x i32], [1 x i32] } %tmp23, 1
  %tmp41 = getelementptr i32, i32* %tmp40, i32 0
  store i32 %tmp35, i32* %tmp41, align 4
  %tmp42 = extractvalue { i32*, i32*, i32, [1 x i32], [1 x i32] } %tmp23, 1
  %tmp43 = getelementptr i32, i32* %tmp42, i32 0
  %tmp44 = bitcast i32* %tmp43 to <5 x i32>*
  %tmp45 = load <5 x i32>, <5 x i32>* %tmp44, align 4
  %tmp46 = call <64 x float> @llvm.tpc.ld.tnsr.v64f32.i1(<5 x i32> %tmp45, i8 0, i32 0, <64 x float> undef, i1 true, i1 false)
  %tmp47 = add i32 %tmp35, 64
  %tmp48 = extractvalue { i32*, i32*, i32, [1 x i32], [1 x i32] } %tmp14, 1
  %tmp49 = getelementptr i32, i32* %tmp48, i32 0
  store i32 %tmp47, i32* %tmp49, align 4
  %tmp50 = extractvalue { i32*, i32*, i32, [1 x i32], [1 x i32] } %tmp14, 1
  %tmp51 = getelementptr i32, i32* %tmp50, i32 0
  %tmp52 = bitcast i32* %tmp51 to <5 x i32>*
  %tmp53 = load <5 x i32>, <5 x i32>* %tmp52, align 4
  %tmp54 = call <64 x float> @llvm.tpc.ld.tnsr.v64f32.i1(<5 x i32> %tmp53, i8 0, i32 0, <64 x float> undef, i1 true, i1 false)
  %tmp55 = shufflevector <64 x float> %tmp46, <64 x float> %tmp54, <128 x i32> <i32 0, i32 1, i32 2, i32 3, i32 4, i32 5, i32 6, i32 7, i32 8, i32 9, i32 10, i32 11, i32 12, i32 13, i32 14, i32 15, i32 16, i32 17, i32 18, i32 19, i32 20, i32 21, i32 22, i32 23, i32 24, i32 25, i32 26, i32 27, i32 28, i32 29, i32 30, i32 31, i32 32, i32 33, i32 34, i32 35, i32 36, i32 37, i32 38, i32 39, i32 40, i32 41, i32 42, i32 43, i32 44, i32 45, i32 46, i32 47, i32 48, i32 49, i32 50, i32 51, i32 52, i32 53, i32 54, i32 55, i32 56, i32 57, i32 58, i32 59, i32 60, i32 61, i32 62, i32 63, i32 64, i32 65, i32 66, i32 67, i32 68, i32 69, i32 70, i32 71, i32 72, i32 73, i32 74, i32 75, i32 76, i32 77, i32 78, i32 79, i32 80, i32 81, i32 82, i32 83, i32 84, i32 85, i32 86, i32 87, i32 88, i32 89, i32 90, i32 91, i32 92, i32 93, i32 94, i32 95, i32 96, i32 97, i32 98, i32 99, i32 100, i32 101, i32 102, i32 103, i32 104, i32 105, i32 106, i32 107, i32 108, i32 109, i32 110, i32 111, i32 112, i32 113, i32 114, i32 115, i32 116, i32 117, i32 118, i32 119, i32 120, i32 121, i32 122, i32 123, i32 124, i32 125, i32 126, i32 127>
  %tmp56 = fptrunc <128 x float> %tmp55 to <128 x bfloat16>
  %tmp57 = call <128 x bfloat16> @llvm.tpc.fptrunc.swch.v128bf16.v128f32(<128 x float> %tmp55, i32 0)
  %tmp58 = extractvalue { i32*, i32*, i32, [1 x i32], [1 x i32] } %tmp5, 1
  %tmp59 = getelementptr i32, i32* %tmp58, i32 0
  %tmp60 = bitcast i32* %tmp59 to <5 x i32>*
  %tmp61 = load <5 x i32>, <5 x i32>* %tmp60, align 4
  call void @llvm.tpc.st.tnsr.v128bf16(<5 x i32> %tmp61, i8 1, <128 x bfloat16> %tmp57, i32 0, i1 true, i1 false)
  %tmp62 = add i32 %tmp35, 128
  br label %bb34

bb63:                                             ; preds = %bb34
  ret void
}

; Function Attrs: nounwind readnone
declare <64 x float> @llvm.tpc.ld.tnsr.v64f32.i1(<5 x i32>, i8, i32, <64 x float>, i1, i1) #0

; Function Attrs: nounwind readnone
declare <128 x bfloat16> @llvm.tpc.fptrunc.swch.v128bf16.v128f32(<128 x float>, i32) #0

; Function Attrs: nounwind writeonly
declare void @llvm.tpc.st.tnsr.v128bf16(<5 x i32>, i8, <128 x bfloat16>, i32, i1, i1) #1

attributes #0 = { nounwind readnone }
attributes #1 = { nounwind writeonly }
attributes #3 = { "target-cpu"="gaudi2" }

!llvm.module.flags = !{!0}

!0 = !{i32 2, !"Debug Info Version", i32 3}
