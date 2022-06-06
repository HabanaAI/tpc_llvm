// RUN: %clang_cc1 -S -emit-llvm -O1 -triple tpc-none-none -std=rc99 %s -o - | FileCheck --check-prefixes=CHECK-IR %s

void main(int src, int dest) {
  short128 *sptr = (short128 *)src;
  float128 *dptr = (float128 *)dest;
  short128 src_val = *sptr;
  *dptr = convert_short128_to_float128(src_val, 0);
}

// CHECK-IR: call <128 x float> @llvm.tpc.convert.v128f32.v128i16.i1(<128 x i16> {{.*}}, i8 7, i32 0, <128 x float> undef, i1 true, i1 false)
