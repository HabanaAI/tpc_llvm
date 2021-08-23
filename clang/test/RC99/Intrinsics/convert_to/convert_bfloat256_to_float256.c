// RUN: %clang_cc1 -S -emit-llvm -O1 -triple tpc-none-none -std=rc99 -target-cpu gaudi %s -o - | FileCheck --check-prefixes=CHECK-IR %s

void main(int src, int dest) {
  bfloat256 *sptr = (bfloat256 *)src;
  float256 *dptr = (float256 *)dest;
  bfloat256 src_val = *sptr;
  *dptr = convert_bfloat256_to_float256(src_val, 0);
}

// CHECK-IR: fpext <256 x bfloat> {{.*}} to <256 x float>
