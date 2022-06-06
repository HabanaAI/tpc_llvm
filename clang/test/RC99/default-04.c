// RUN: %clang_cc1 -S -O2 -emit-llvm -triple tpc -std=rc99 -target-cpu goya2 -mllvm -tpc-trans-intr=0 -disable-llvm-passes %s -o - | FileCheck %s

void main(int dest) {
  float64 __local *dptr = (float64 __local *)dest;
  float64 x = *dptr++;
  *dptr++ = v_f32_abs_b(x);
  float64 y = v_f32_abs_b(x, 0);
  *dptr++ = y;
  y = v_f32_abs_b(x, 0, y);
  *dptr++ = y;
  y = v_f32_abs_b(x, 0, y, 0);
  *dptr++ = y;
  *dptr++ = v_f32_abs_b(x, 0, y, 0, 1);
}

// CHECK: call <64 x float> @llvm.tpc.abs.v64f32.v64f32.i1(<64 x float> {{.*}}, i8 0, i32 0, <64 x float> undef, i1 true, i1 false)
// CHECK: call <64 x float> @llvm.tpc.abs.v64f32.v64f32.i1(<64 x float> {{.*}}, i8 0, i32 0, <64 x float> undef, i1 true, i1 false)
// CHECK: call <64 x float> @llvm.tpc.abs.v64f32.v64f32.i1(<64 x float> {{.*}}, i8 0, i32 0, <64 x float> undef, i1 true, i1 false)
// CHECK: call <64 x float> @llvm.tpc.abs.v64f32.v64f32.i1(<64 x float> {{.*}}, i8 0, i32 0, <64 x float> {{.*}}, i1 false, i1 false)
// CHECK: call <64 x float> @llvm.tpc.abs.v64f32.v64f32.i1(<64 x float> {{.*}}, i8 0, i32 0, <64 x float> undef, i1 false, i1 true)

