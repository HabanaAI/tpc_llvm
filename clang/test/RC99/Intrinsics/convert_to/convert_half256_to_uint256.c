// RUN: %clang_cc1 -S -emit-llvm -O1 -triple tpc-none-none -std=rc99 -target-cpu gaudi2 %s -o - | FileCheck --check-prefixes=CHECK-IR %s
// RUN: %clang_cc1 -S -emit-llvm -O1 -triple tpc-none-none -std=rc99 -target-cpu doron1 %s -o - | FileCheck --check-prefixes=CHECK-IR %s

void main(int src, int dest) {
  half256 *sptr = (half256 *)src;
  uint256 *dptr = (uint256 *)dest;
  half256 src_val = *sptr;
  *dptr++ = convert_half256_to_uint256(src_val, SW_RZ);
  *dptr = convert_half256_to_uint256(src_val, SW_RD);
}

// CHECK-IR: call <256 x i32> @llvm.tpc.convert.v256i32.v256f16.i1(<256 x half> {{.*}}, i8 11, i32 66304, <256 x i32> undef, i1 true, i1 false)
// CHECK-IR: call <256 x i32> @llvm.tpc.convert.v256i32.v256f16.i1(<256 x half> {{.*}}, i8 11, i32 197376, <256 x i32> undef, i1 true, i1 false)
