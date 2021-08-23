// RUN: %clang_cc1 -S -emit-llvm -O1 -triple tpc-none-none -std=rc99 -target-cpu gaudi %s -o - | FileCheck --check-prefixes=CHECK-IR %s

void main(int src, int dest) {
  float128 *sptr = (float128 *)src;
  uint128 *dptr = (uint128 *)dest;
  float128 src_val = *sptr;
  *dptr++ = convert_float128_to_uint128(src_val, SW_RZ);
  *dptr = convert_float128_to_uint128(src_val, SW_RD);
}

// CHECK-IR: fptoui <128 x float> {{.*}} to <128 x i32>
// CHECK-IR: call <128 x i32> @llvm.tpc.convert.v128i32.v128f32.i1(<128 x float> {{.*}}, i8 0, i32 197376, <128 x i32> undef, i1 true, i1 false)
