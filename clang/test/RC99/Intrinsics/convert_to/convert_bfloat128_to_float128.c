// RUN: %clang_cc1 -S -emit-llvm -O1 -triple tpc-none-none -std=rc99 -target-cpu gaudi %s -o - | FileCheck --check-prefixes=CHECK-IR %s

void main(int src, int dest) {
  bfloat128 *sptr = (bfloat128 *)src;
  float128 *dptr = (float128 *)dest;
  bfloat128 src_val = *sptr;
  *dptr = convert_bfloat128_to_float128(src_val, 0);
}

// CHECK-IR: fpext <128 x bfloat> {{.*}} to <128 x float>
