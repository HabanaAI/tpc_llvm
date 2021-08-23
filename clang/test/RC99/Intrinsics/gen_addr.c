// RUN: %codegen -S -O1 -triple tpc-none-none -std=rc99 %s -o - | FileCheck %s
// RUN: %codegen -S -O1 -triple tpc-none-none -std=rc99 -target-cpu gaudi -bfloat16 %s -o - | FileCheck %s

void main(tensor out, int value, _Bool p) {
// CHECK:      .globl main
  int5 indx = { 0, 0, 0, 0, 0 };
  int __global *ptr1 = gen_addr(indx, out, 0, 0, 1, 0);
  *ptr1 = value;

// CHECK:      set_indx [[NDX:%I[0-9]+]], b11111, 0x0
// CHECK:      gen_addr [[ADDR:%AD[0-9]+]], 0x0, [[NDX]]
// CHECK:      st_g     [[ADDR]], %S0

  indx[0] = 1;
  int __global *ptr2 = gen_addr(indx, out, 0, ptr1, p, 0);
  *ptr2 = value;

// CHECK:      gen_addr  [[ADDR]], 0x0, %I{{[0-9]+}}, %SP{{[0-9]+}}
// CHECK:      st_g %AD{{[0-9]+}}, %S0

  indx[0] = 2;
  int __global *ptr3 = gen_addr(indx, out, 0, ptr2, p, 1);
  *ptr3 = value;

// CHECK:      gen_addr  [[ADDR]], 0x0, %I{{[0-9]+}}, !%SP{{[0-9]+}}
// CHECK:      st_g %AD{{[0-9]+}}, %S0

// CHECK:      halt
}
