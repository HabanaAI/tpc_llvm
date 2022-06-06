// RUN: %clang_cc1 -S -emit-llvm -O1 -triple tpc-none-none -std=rc99 -target-cpu goya2 %s -o - | FileCheck --check-prefixes=CHECK-IR %s

void main(int src, int dest) {
  short128 *sptr = (short128 *)src;
  half128 *dptr = (half128 *)dest;
  short128 src_val = *sptr;
  *dptr++ = convert_short128_to_half128(src_val, 0);
  *dptr = convert_short128_to_half128(src_val, SW_RD);
}

// CHECK-IR: sitofp <128 x i16> {{.*}} to <128 x half>
// CHECK-IR: call <128 x half> @llvm.tpc.convert.v128f16.v128i16.i1(<128 x i16> {{.*}}, i8 7, i32 199424, <128 x half> undef, i1 true, i1 false)
