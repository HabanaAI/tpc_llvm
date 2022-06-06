// RUN: %clang_cc1 -S -emit-llvm -O1 -triple tpc-none-none -std=rc99 -target-cpu gaudi %s -o - | FileCheck --check-prefixes=CHECK-IR %s

void main(int src, int dest) {
  short256 *sptr = (short256 *)src;
  uint256 *dptr = (uint256 *)dest;
  short256 src_val = *sptr;
  *dptr = convert_short256_to_uint256(src_val, 0);
}

// CHECK-IR: call <256 x i32> @llvm.tpc.convert.v256i32.v256i16.i1(<256 x i16> {{.*}}, i8 7, i32 768, <256 x i32> undef, i1 true, i1 false)
