// RUN: %clang_cc1 -S -emit-llvm -O1 -triple tpc-none-none -std=rc99 -target-cpu gaudi %s -o - | FileCheck --check-prefixes=CHECK-IR %s

void main(int src, int dest) {
  short256 *sptr = (short256 *)src;
  int256 *dptr = (int256 *)dest;
  short256 src_val = *sptr;
  *dptr = convert_short256_to_int256(src_val, 0);
}

// CHECK-IR: sext <256 x i16> {{.*}} to <256 x i32>
