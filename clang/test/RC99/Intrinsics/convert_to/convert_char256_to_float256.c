// RUN: %clang_cc1 -S -emit-llvm -O1 -triple tpc-none-none -std=rc99 -target-cpu gaudi %s -o - | FileCheck --check-prefixes=CHECK-IR %s

void main(int src, int dest) {
  char256 *sptr = (char256 *)src;
  float256 *dptr = (float256 *)dest;
  char256 src_val = *sptr;
  *dptr = convert_char256_to_float256(src_val, 0);
}

// CHECK-IR: sitofp <256 x i8> {{.*}} to <256 x float>
