// RUN: %codegen -S -O1 -triple tpc-none-none -std=rc99 %s -o - | FileCheck %s

void main(int dest, int src, unsigned char divisor) {
  uint8_t_pair_t __local *sptr = (uint8_t_pair_t __local *) src;
  uint8_t_pair_t __local *dptr = (uint8_t_pair_t __local *) dest;
  uint8_t_pair_t quot_rem = *sptr;
  quot_rem = u8_udiv_step(divisor, 5, 0, quot_rem, 1, 0);
  *dptr = quot_rem;
}

// load dividend and remainder to a register pair
// CHECK-DAG: ld_l  %S[[ZN:[0-9]+]], %S1
// CHECK-DAG: ld_l  %S[[ZNN:[0-9]+]], %S{{[0-9]+}}

// CHECK:     udiv_step.u8 0x5 %Z[[ZN]], %S2

// CHECK-DAG: st_l %S0,    %S[[ZN]]
// CHECK-DAG: st_l %S{{[0-9]+}}, %S[[ZNN]]
