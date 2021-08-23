// RUN: %clang_cc1 -S -emit-llvm -O1 -triple tpc-none-none -std=rc99 -target-cpu gaudi %s -o - | FileCheck --check-prefixes=CHECK-IR %s

void main(int src, int dest) {
  float256 *sptr = (float256 *)src;
  uint256 *dptr = (uint256 *)dest;
  float256 src_val = *sptr;
  *dptr++ = convert_float256_to_uint256(src_val, SW_RZ);
  *dptr = convert_float256_to_uint256(src_val, SW_RD);
}

// CHECK-IR: fptoui <256 x float> {{.*}} to <256 x i32>
// CHECK-IR: call <256 x i32> @llvm.tpc.convert.v256i32.v256f32.i1(<256 x float> {{.*}}, i8 0, i32 197376, <256 x i32> undef, i1 true, i1 false)
