// RUN: %clang_cc1 -S -emit-llvm -O1 -triple tpc-none-none -std=rc99 -target-cpu gaudi %s -o - | FileCheck --check-prefixes=CHECK-IR %s

void main(int src, int dest) {
  ushort256 *sptr = (ushort256 *)src;
  float256 *dptr = (float256 *)dest;
  ushort256 src_val = *sptr;
  *dptr = convert_ushort256_to_float256(src_val, 0);
}

// CHECK-IR: uitofp <256 x i16> {{.*}} to <256 x float>
