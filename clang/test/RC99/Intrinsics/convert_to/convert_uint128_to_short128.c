// RUN: %clang_cc1 -S -emit-llvm -O1 -triple tpc-none-none -std=rc99 -target-cpu goya2 %s -o - | FileCheck --check-prefixes=CHECK-IR %s

void main(int src, int dest) {
  uint128 *sptr = (uint128 *)src;
  short128 *dptr = (short128 *)dest;
  uint128 src_val = *sptr;
  *dptr = convert_uint128_to_short128(src_val, 0);
}

// CHECK-IR: call <128 x i16> @llvm.tpc.convert.v128i16.v128i32.i1(<128 x i32> {{.*}}, i8 3, i32 1792, <128 x i16> undef, i1 true, i1 false)
