// RUN: %clang_cc1 -S -emit-llvm -O1 -triple tpc-none-none -std=rc99 -target-cpu gaudi %s -o - | FileCheck --check-prefixes=CHECK-IR %s

void main(int src, int dest) {
  bfloat256 *sptr = (bfloat256 *)src;
  uint256 *dptr = (uint256 *)dest;
  bfloat256 src_val = *sptr;
  *dptr++ = convert_bfloat256_to_uint256(src_val, SW_RZ);
  *dptr = convert_bfloat256_to_uint256(src_val, SW_RD);
}

// CHECK-IR: call <256 x i32> @llvm.tpc.convert.v256i32.v256bf16.i1(<256 x bfloat> {{.*}}, i8 1, i32 66304, <256 x i32> undef, i1 true, i1 false)
// CHECK-IR: call <256 x i32> @llvm.tpc.convert.v256i32.v256bf16.i1(<256 x bfloat> {{.*}}, i8 1, i32 197376, <256 x i32> undef, i1 true, i1 false)
