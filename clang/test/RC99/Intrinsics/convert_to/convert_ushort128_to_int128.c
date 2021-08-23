// RUN: %clang_cc1 -S -emit-llvm -O1 -triple tpc-none-none -std=rc99 -target-cpu gaudi %s -o - | FileCheck --check-prefixes=CHECK-IR %s

void main(int src, int dest) {
  ushort128 *sptr = (ushort128 *)src;
  int128 *dptr = (int128 *)dest;
  ushort128 src_val = *sptr;
  *dptr = convert_ushort128_to_int128(src_val, 0);
}

// CHECK-IR: zext <128 x i16> {{.*}} to <128 x i32>
