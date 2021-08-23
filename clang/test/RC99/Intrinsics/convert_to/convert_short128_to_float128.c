// RUN: %clang_cc1 -S -emit-llvm -O1 -triple tpc-none-none -std=rc99 %s -o - | FileCheck --check-prefixes=CHECK-IR %s

void main(int src, int dest) {
  short128 *sptr = (short128 *)src;
  float128 *dptr = (float128 *)dest;
  short128 src_val = *sptr;
  *dptr = convert_short128_to_float128(src_val, 0);
}

// CHECK-IR: sitofp <128 x i16> {{.*}} to <128 x float>
