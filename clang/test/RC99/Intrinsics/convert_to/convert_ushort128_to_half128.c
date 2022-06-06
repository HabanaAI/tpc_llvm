// RUN: %clang_cc1 -S -emit-llvm -O1 -triple tpc-none-none -std=rc99 -target-cpu goya2 %s -o - | FileCheck --check-prefixes=CHECK-IR %s

void main(int src, int dest) {
  ushort128 *sptr = (ushort128 *)src;
  half128 *dptr = (half128 *)dest;
  ushort128 src_val = *sptr;
  *dptr++ = convert_ushort128_to_half128(src_val, 0);
  *dptr = convert_ushort128_to_half128(src_val, SW_RD);
}

// CHECK-IR: uitofp <128 x i16> {{.*}} to <128 x half>
// CHECK-IR: call <128 x half> @llvm.tpc.convert.v128f16.v128i16.i1(<128 x i16> {{.*}}, i8 8, i32 199424, <128 x half> undef, i1 true, i1 false)
