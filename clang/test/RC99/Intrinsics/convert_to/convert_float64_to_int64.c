// RUN: %clang_cc1 -S -emit-llvm -O1 -triple tpc-none-none -std=rc99 -target-cpu gaudi %s -o - | FileCheck --check-prefixes=CHECK-IR %s

void main(int src, int dest) {
  float64 *sptr = (float64 *)src;
  int64 *dptr = (int64 *)dest;
  float64 src_val = *sptr;
  *dptr++ = convert_float64_to_int64(src_val, SW_RZ);
  *dptr = convert_float64_to_int64(src_val, SW_RD);
}

// CHECK-IR: fptosi <64 x float> {{.*}} to <64 x i32>
// CHECK-IR: call <64 x i32> @llvm.tpc.convert.v64i32.v64f32.i1(<64 x float> {{.*}}, i8 0, i32 197120, <64 x i32> undef, i1 true, i1 false)
