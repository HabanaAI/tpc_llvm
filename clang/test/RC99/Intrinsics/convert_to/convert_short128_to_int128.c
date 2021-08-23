// RUN: %clang_cc1 -S -emit-llvm -O1 -triple tpc-none-none -std=rc99 -target-cpu gaudi %s -o - | FileCheck --check-prefixes=CHECK-IR %s

void main(int src, int dest) {
  short128 *sptr = (short128 *)src;
  int128 *dptr = (int128 *)dest;
  short128 src_val = *sptr;
  *dptr = convert_short128_to_int128(src_val, 0);
}

// CHECK-IR: sext <128 x i16> {{.*}} to <128 x i32>
