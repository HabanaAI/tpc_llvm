// RUN: %clang_cc1 -S -emit-llvm -O1 -triple tpc-none-none -std=rc99 -target-cpu gaudi %s -o - | FileCheck --check-prefixes=CHECK-IR %s

void main(int src, int dest) {
  int256 *sptr = (int256 *)src;
  float256 *dptr = (float256 *)dest;
  int256 src_val = *sptr;
  *dptr++ = convert_int256_to_float256(src_val, 0);
  *dptr = convert_int256_to_float256(src_val, SW_RD);
}

// CHECK-IR: sitofp <256 x i32> {{.*}} to <256 x float>
// CHECK-IR: call <256 x float> @llvm.tpc.convert.v256f32.v256i32.i1(<256 x i32> {{.*}}, i8 2, i32 196608, <256 x float> undef, i1 true, i1 false)
