// RUN: %clang_cc1 -S -emit-llvm -O1 -triple tpc-none-none -std=rc99 -target-cpu gaudi %s -o - | FileCheck --check-prefixes=CHECK-IR %s

void main(int src, int dest) {
  uint128 *sptr = (uint128 *)src;
  bfloat128 *dptr = (bfloat128 *)dest;
  uint128 src_val = *sptr;
  *dptr++ = convert_uint128_to_bfloat128(src_val, 0);
  *dptr = convert_uint128_to_bfloat128(src_val, SW_RD);
}

// CHECK-IR: call <128 x bfloat> @llvm.tpc.convert.v128bf16.v128i32.i1(<128 x i32> {{.*}}, i8 3, i32 256, <128 x bfloat> undef, i1 true, i1 false)
// CHECK-IR: call <128 x bfloat> @llvm.tpc.convert.v128bf16.v128i32.i1(<128 x i32> {{.*}}, i8 3, i32 196864, <128 x bfloat> undef, i1 true, i1 false)
