// RUN: %clang_cc1 -S -emit-llvm -O1 -triple tpc-none-none -std=rc99 -target-cpu gaudi %s -o - | FileCheck --check-prefixes=CHECK-IR %s

void main(int src, int dest) {
  int64 *sptr = (int64 *)src;
  float64 *dptr = (float64 *)dest;
  int64 src_val = *sptr;
  *dptr++ = convert_int64_to_float64(src_val, 0);
  *dptr = convert_int64_to_float64(src_val, SW_RD);
}

// CHECK-IR: sitofp <64 x i32> {{.*}} to <64 x float>
// CHECK-IR: call <64 x float> @llvm.tpc.convert.v64f32.v64i32.i1(<64 x i32> {{.*}}, i8 2, i32 196608, <64 x float> undef, i1 true, i1 false)
