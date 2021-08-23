// RUN: %clang_cc1 -S -emit-llvm -O1 -triple tpc-none-none -std=rc99 -target-cpu gaudi %s -o - | FileCheck --check-prefixes=CHECK-IR %s

void main(int src, int dest) {
  uint64 *sptr = (uint64 *)src;
  float64 *dptr = (float64 *)dest;
  uint64 src_val = *sptr;
  *dptr++ = convert_uint64_to_float64(src_val, 0);
  *dptr = convert_uint64_to_float64(src_val, SW_RD);
}

// CHECK-IR: uitofp <64 x i32> {{.*}} to <64 x float>
// CHECK-IR: call <64 x float> @llvm.tpc.convert.v64f32.v64i32.i1(<64 x i32> {{.*}}, i8 3, i32 196608, <64 x float> undef, i1 true, i1 false)
