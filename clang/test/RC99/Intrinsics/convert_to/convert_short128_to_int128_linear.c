// RUN: %clang_cc1 -S -emit-llvm -O1 -triple tpc-none-none -std=rc99 -target-cpu gaudi %s -o - | FileCheck --check-prefixes=CHECK-IR %s

void main(int src, int dest) {
  short128 *sptr = (short128 *)src;
  int128 *dptr = (int128 *)dest;
  short128 src_val = *sptr;
  *dptr++ = convert_short128_to_int128(src_val, 0);
  *dptr = convert_short128_to_int128(src_val, SW_LINEAR);
}

// CHECK-IR: call <128 x i32> @llvm.tpc.convert.v128i32.v128i16.i1(<128 x i16> {{.*}}, i8 7, i32 512, <128 x i32> undef, i1 true, i1 false)
// CHECK-IR: call <128 x i32> @llvm.tpc.convert.linear.v128i32.v128i16.i1(<128 x i16> {{.*}}, i8 7, i32 512, <128 x i32> undef, i1 true, i1 false)
