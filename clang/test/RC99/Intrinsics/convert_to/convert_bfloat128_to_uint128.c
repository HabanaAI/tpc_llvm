// RUN: %clang_cc1 -S -emit-llvm -O1 -triple tpc-none-none -std=rc99 -target-cpu gaudi %s -o - | FileCheck --check-prefixes=CHECK-IR %s

void main(int src, int dest) {
  bfloat128 *sptr = (bfloat128 *)src;
  uint128 *dptr = (uint128 *)dest;
  bfloat128 src_val = *sptr;
  *dptr++ = convert_bfloat128_to_uint128(src_val, SW_RZ);
  *dptr = convert_bfloat128_to_uint128(src_val, SW_RD);
}

// CHECK-IR: call <128 x i32> @llvm.tpc.convert.v128i32.v128bf16.i1(<128 x bfloat> {{.*}}, i8 1, i32 66304, <128 x i32> undef, i1 true, i1 false)
// CHECK-IR: call <128 x i32> @llvm.tpc.convert.v128i32.v128bf16.i1(<128 x bfloat> {{.*}}, i8 1, i32 197376, <128 x i32> undef, i1 true, i1 false)
