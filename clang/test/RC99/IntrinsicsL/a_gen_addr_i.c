// RUN: %codegen -S -O1 -triple tpc-none-none -std=rc99 %s -o - | FileCheck %s

void main(tensor out, int value) {
  int5 indx = { 0, 0, 0, 0, 0 };
  int __global__ *ptr = a_gen_addr_i(indx, out);
  *ptr = value;
}

// CHECK:      .globl main
// CHECK:      set_indx %I[[NDX:[0-9]+]], b11111, 0x0, %SP0
// CHECK:      gen_addr %AD[[ADDR:[0-9]+]], 0x0, %I[[NDX]]
// CHECK:      st_g %AD[[ADDR]], %S0, %SP0
// CHECK:      halt
