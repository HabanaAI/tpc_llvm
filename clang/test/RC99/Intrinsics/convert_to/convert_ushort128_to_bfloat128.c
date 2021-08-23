// RUN: %clang_cc1 -S -emit-llvm -O1 -triple tpc-none-none -std=rc99 -target-cpu gaudi %s -o - | FileCheck --check-prefixes=CHECK-IR %s

void main(int src, int dest) {
  ushort128 *sptr = (ushort128 *)src;
  bfloat128 *dptr = (bfloat128 *)dest;
  ushort128 src_val = *sptr;
  *dptr++ = convert_ushort128_to_bfloat128(src_val, 0);
  *dptr = convert_ushort128_to_bfloat128(src_val, SW_RD);
}

// CHECK-IR: uitofp <128 x i16> {{.*}} to <128 x bfloat>
// CHECK-IR: call <128 x bfloat> @llvm.tpc.convert.v128bf16.v128i16.i1(<128 x i16> {{.*}}, i8 8, i32 196864, <128 x bfloat> undef, i1 true, i1 false)
