// RUN: %clang_cc1 -S -O2  -emit-llvm -triple tpc -std=rc99 -target-cpu goya2 -mllvm -tpc-trans-intr=0 %s -o - | FileCheck %s

void main(int dest, float x, float y) {
  float __local *dptr = (float __local *)dest;
  *dptr++ = s_f32_abs(x);
  *dptr++ = s_f32_abs(x, SW_NO_SAT);
  *dptr++ = s_f32_abs(y, 0, 1);
  *dptr++ = s_f32_abs(x, 0, 2, 0);
  *dptr++ = s_f32_abs(x, 0, 2, 0, 1);
}

// CHECK: call float @llvm.tpc.abs.f32.f32.i1(float %x, i8 0, i32 0, float undef, i1 true, i1 false)
// CHECK: call float @llvm.tpc.abs.f32.f32.i1(float %x, i8 0, i32 1, float undef, i1 true, i1 false)
// CHECK: call float @llvm.tpc.abs.f32.f32.i1(float %y, i8 0, i32 0, float undef, i1 true, i1 false)
// CHECK: call float @llvm.tpc.abs.f32.f32.i1(float %x, i8 0, i32 0, float 2.000000e+00, i1 false, i1 false)
// CHECK: call float @llvm.tpc.abs.f32.f32.i1(float %x, i8 0, i32 0, float undef, i1 false, i1 true)

