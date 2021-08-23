// RUN: %clang_cc1 -S -emit-llvm -O1 -triple tpc-none-none -std=rc99 -target-cpu gaudi %s -o - | FileCheck --check-prefixes=CHECK-IR %s

void main(int src, int dest) {
  float256 *sptr = (float256 *)src;
  bfloat256 *dptr = (bfloat256 *)dest;
  float256 src_val = *sptr;
  *dptr++ = convert_float256_to_bfloat256(src_val, 0);
  *dptr = convert_float256_to_bfloat256(src_val, SW_RU);
}

// CHECK-IR: fptrunc <256 x float> {{.*}} to <256 x bfloat>
// CHECK-IR: call <256 x bfloat> @llvm.tpc.convert.v256bf16.v256f32.i1(<256 x float> {{.*}}, i8 0, i32 131328, <256 x bfloat> undef, i1 true, i1 false)
