// RUN: %codegen -S -O1 -triple tpc-none-none -std=rc99 %s -o - | FileCheck %s

void main(int dest, int src, unsigned short divisor) {
  uint16_t_pair_t __local *sptr = (uint16_t_pair_t __local *) src;
  uint16_t_pair_t __local *dptr = (uint16_t_pair_t __local *) dest;
  uint16_t_pair_t quot_rem = *sptr;
  quot_rem = s_u16_udiv_step_s(quot_rem, divisor, 5);
  *dptr = quot_rem;
}

// load dividend and remainder to a register pair
// CHECK-DAG: ld_l  %S[[ZN:[0-9]+]], %S1
// CHECK-DAG: ld_l  %S[[ZNN:[0-9]+]], %S{{[0-9]+}}

// CHECK:     udiv_step.u16 0x5 %Z[[ZN]], %S2, %SP0

// CHECK-DAG: st_l %S0,    %S[[ZN]]
// CHECK-DAG: st_l %S{{[0-9]+}}, %S[[ZNN]]
