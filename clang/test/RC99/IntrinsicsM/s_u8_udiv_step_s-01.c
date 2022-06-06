// RUN: %codegen -S -O1 -triple tpc-none-none -std=rc99 %s -o - | FileCheck %s

void main(int dest, unsigned char dividend, unsigned char divisor) {
  uint8_t_pair_t __local *dptr = (uint8_t_pair_t __local *) dest;
  uint8_t_pair_t quot_rem = { dividend, 0 };
  quot_rem = u8_udiv_step(divisor, 1, 0, quot_rem, 1, 0);
  *dptr = quot_rem;
}

// move 'dividend' to a register pair, clear remainder
// CHECK-DAG: and.i32      %S[[ZN:[0-9]+]], %S1, 0xff
// CHECK-DAG: mov.i32      %S[[ZNN:[0-9]+]], 0x0

// CHECK:     udiv_step.u8 0x1 %Z[[ZN]], %S2

// CHECK-DAG: st_l %S0,    %S[[ZN]]
// CHECK-DAG: st_l %S{{[0-9]+}}, %S[[ZNN]]
