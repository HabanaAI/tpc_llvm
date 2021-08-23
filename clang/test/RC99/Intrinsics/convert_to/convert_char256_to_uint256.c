// RUN: %clang_cc1 -S -emit-llvm -O1 -triple tpc-none-none -std=rc99 -target-cpu gaudi %s -o - | FileCheck --check-prefixes=CHECK-IR %s

void main(int src, int dest) {
  char256 *sptr = (char256 *)src;
  uint256 *dptr = (uint256 *)dest;
  char256 src_val = *sptr;
  *dptr = convert_char256_to_uint256(src_val, 0);
}

// CHECK-IR: zext <256 x i8> {{.*}} to <256 x i32>
