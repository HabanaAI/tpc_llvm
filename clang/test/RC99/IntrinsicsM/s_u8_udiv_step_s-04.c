// RUN: %codegen -S -O1 -triple tpc-none-none -std=rc99 %s -o - | FileCheck %s

void main(int dest, unsigned char dividend, unsigned char divisor) {
  uint8_t_pair_t __local *dptr = (uint8_t_pair_t __local *) dest;
  uint8_t_pair_t quot_rem = { dividend, 0 };
  quot_rem = u8_udiv_step(divisor, 1, 0, quot_rem, 1, 0);
  quot_rem.v2 = 0;
  *dptr = quot_rem;
}

// move 'dividend' to a register pair, clear remainder
// CHECK-DAG: mov.i32      %S[[ZNN:[0-9]+]], 0x0
// CHECK-DAG: add.i32      %S[[DST:[0-9]+]], %S0, 0x4
// CHECK-DAG: st_l %S[[DST]], %S[[ZNN]]
// CHECK-DAG: udiv_step.u8 0x1 %Z[[ZN:[0-9]+]], %S2
// CHECK: st_l %S0,    %S[[ZN]]
