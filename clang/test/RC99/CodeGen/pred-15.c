// RUN: %codegen -triple tpc-none-none -std=rc99 -S -O1 -tpc-special %s -o - | FileCheck %s

void main(int dest, int src1, int src2) {
  char256 a = *(char256 __local *)src1;
  char256 b = *(char256 __local *)src2;
  int256 acc = { 0, 0, 0, 0 };
  acc = av_i8_mac_v_v_b(a, b, acc, e_no_saturation, 1, 1);
  *(int256 __local *)dest = acc;
}

// CHECK-NOT: MAC.B
