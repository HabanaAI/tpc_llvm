; RUN: opt -passes=eval-special-function -S %s | FileCheck %s

; The expansions of f32 reciprocal, bf16 sin and bf16 log pass negative
; constants. ConstantInt::get took them as unsigned 64-bit values, which
; APInt refuses for a narrower type; they are now signed.

target triple = "tpc"

; CHECK-LABEL: @recip(
; CHECK: @llvm.tpc.sel.leq{{.*}}, i32 -1065353216,
define <64 x float> @recip(<64 x float> %x) {
  %r = call <64 x float> @llvm.tpc.reciprocal.v64f32(<64 x float> %x)
  ret <64 x float> %r
}

; CHECK-LABEL: @sin(
; CHECK: i16 -32768
define <128 x bfloat> @sin(<128 x bfloat> %x) {
  %r = call <128 x bfloat> @llvm.sin.v128bf16(<128 x bfloat> %x)
  ret <128 x bfloat> %r
}

; CHECK-LABEL: @log(
; CHECK: i16 -4,
define <128 x bfloat> @log(<128 x bfloat> %x) {
  %r = call <128 x bfloat> @llvm.log.v128bf16(<128 x bfloat> %x)
  ret <128 x bfloat> %r
}
