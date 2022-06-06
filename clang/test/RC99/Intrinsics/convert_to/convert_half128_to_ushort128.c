// RUN: %clang_cc1 -S -emit-llvm -O1 -triple tpc-none-none -std=rc99 -target-cpu goya2 %s -o - | FileCheck --check-prefixes=CHECK-IR %s

void main(int src, int dest) {
  half128 *sptr = (half128 *)src;
  ushort128 *dptr = (ushort128 *)dest;
  half128 src_val = *sptr;
  *dptr++ = convert_half128_to_ushort128(src_val, SW_RZ);
  *dptr = convert_half128_to_ushort128(src_val, SW_RD);
}

// CHECK-IR: call <128 x i16> @llvm.tpc.convert.v128i16.v128f16.i1(<128 x half> {{.*}}, i8 11, i32 67584, <128 x i16> undef, i1 true, i1 false)
// CHECK-IR: call <128 x i16> @llvm.tpc.convert.v128i16.v128f16.i1(<128 x half> {{.*}}, i8 11, i32 198656, <128 x i16> undef, i1 true, i1 false)
