// RUN: %clang_cc1 -S -emit-llvm -O1 -triple tpc-none-none -std=rc99 -target-cpu gaudi %s -o - | FileCheck --check-prefixes=CHECK-IR %s

void main(int src, int dest) {
  float128 *sptr = (float128 *)src;
  bfloat128 *dptr = (bfloat128 *)dest;
  float128 src_val = *sptr;
  *dptr++ = convert_float128_to_bfloat128(src_val, 0);
  *dptr = convert_float128_to_bfloat128(src_val, SW_RU);
}

// CHECK-IR: fptrunc <128 x float> {{.*}} to <128 x bfloat>
// CHECK-IR: call <128 x bfloat> @llvm.tpc.convert.v128bf16.v128f32.i1(<128 x float> {{.*}}, i8 0, i32 131328, <128 x bfloat> undef, i1 true, i1 false)
