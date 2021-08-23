// RUN: %clang_cc1 -S -emit-llvm -O1 -triple tpc-none-none -std=rc99 -target-cpu gaudi %s -o - | FileCheck --check-prefixes=CHECK-IR %s

void main(int src, int dest) {
  bfloat128 *sptr = (bfloat128 *)src;
  short128 *dptr = (short128 *)dest;
  bfloat128 src_val = *sptr;
  *dptr++ = convert_bfloat128_to_short128(src_val, SW_RZ);
  *dptr = convert_bfloat128_to_short128(src_val, SW_RD);
}

// CHECK-IR: fptosi <128 x bfloat> {{.*}} to <128 x i16>
// CHECK-IR: call <128 x i16> @llvm.tpc.convert.v128i16.v128bf16.i1(<128 x bfloat> {{.*}}, i8 1, i32 198400, <128 x i16> undef, i1 true, i1 false)
