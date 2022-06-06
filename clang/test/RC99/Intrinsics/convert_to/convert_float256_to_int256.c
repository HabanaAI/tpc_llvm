// RUN: %clang_cc1 -S -emit-llvm -O1 -triple tpc-none-none -std=rc99 -target-cpu gaudi %s -o - | FileCheck --check-prefixes=CHECK-IR %s

void main(int src, int dest) {
  float256 *sptr = (float256 *)src;
  int256 *dptr = (int256 *)dest;
  float256 src_val = *sptr;
  *dptr++ = convert_float256_to_int256(src_val, SW_RZ);
  *dptr = convert_float256_to_int256(src_val, SW_RD);
}

// CHECK-IR: call <256 x i32> @llvm.tpc.convert.v256i32.v256f32.i1(<256 x float> {{.*}}, i8 0, i32 66048, <256 x i32> undef, i1 true, i1 false)
// CHECK-IR: call <256 x i32> @llvm.tpc.convert.v256i32.v256f32.i1(<256 x float> {{.*}}, i8 0, i32 197120, <256 x i32> undef, i1 true, i1 false)
