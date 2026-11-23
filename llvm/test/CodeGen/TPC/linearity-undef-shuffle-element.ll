; RUN: opt -mtriple=tpc -mcpu=gaudi2 -passes=tpc-eliminate-redundant-linearity -S %s -o - | FileCheck %s

; Comparing two shuffles element by element, an element that is undefined in
; one and defined in the other is a difference. The second shuffle's undef
; check read the first one's mask, and a defined first element against an
; undefined second one asserted.


define <128 x bfloat> @main(<64 x float> %0) {
entry:
  %scaled.sroa.0.0.vec.expand = shufflevector <64 x float> %0, <64 x float> zeroinitializer, <128 x i32> <i32 0, i32 1, i32 2, i32 3, i32 4, i32 5, i32 6, i32 7, i32 8, i32 9, i32 10, i32 11, i32 12, i32 13, i32 14, i32 15, i32 16, i32 17, i32 18, i32 19, i32 20, i32 21, i32 22, i32 23, i32 24, i32 25, i32 26, i32 27, i32 28, i32 29, i32 30, i32 31, i32 32, i32 33, i32 34, i32 35, i32 36, i32 37, i32 38, i32 39, i32 40, i32 41, i32 42, i32 43, i32 44, i32 45, i32 46, i32 47, i32 48, i32 49, i32 50, i32 51, i32 52, i32 53, i32 54, i32 55, i32 56, i32 57, i32 58, i32 59, i32 60, i32 61, i32 62, i32 63, i32 poison, i32 poison, i32 poison, i32 poison, i32 poison, i32 poison, i32 poison, i32 poison, i32 poison, i32 poison, i32 poison, i32 poison, i32 poison, i32 poison, i32 poison, i32 poison, i32 poison, i32 poison, i32 poison, i32 poison, i32 poison, i32 poison, i32 poison, i32 poison, i32 poison, i32 poison, i32 poison, i32 poison, i32 poison, i32 poison, i32 poison, i32 poison, i32 poison, i32 poison, i32 poison, i32 poison, i32 poison, i32 poison, i32 poison, i32 poison, i32 poison, i32 poison, i32 poison, i32 poison, i32 poison, i32 poison, i32 poison, i32 poison, i32 poison, i32 poison, i32 poison, i32 poison, i32 poison, i32 poison, i32 poison, i32 poison, i32 poison, i32 poison, i32 poison, i32 poison, i32 poison, i32 poison, i32 poison, i32 poison>
  %1 = call <128 x bfloat> @llvm.tpc.convert.linear.v128bf16.v128f32.i1(<128 x float> %scaled.sroa.0.0.vec.expand, i8 0, i32 0, <128 x bfloat> zeroinitializer, i1 false, i1 false)
  ret <128 x bfloat> %1
}

; Function Attrs: nounwind memory(none)
declare <128 x bfloat> @llvm.tpc.convert.linear.v128bf16.v128f32.i1(<128 x float>, i8, i32, <128 x bfloat>, i1, i1) #0

attributes #0 = { nounwind memory(none) }

; CHECK: call <128 x bfloat> @llvm.tpc.convert.linear.v128bf16.v128f32.i1
