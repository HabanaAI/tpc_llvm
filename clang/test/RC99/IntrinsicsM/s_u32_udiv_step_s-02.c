// RUN: %codegen -S -O1 -triple tpc-none-none -std=rc99 %s -o - | FileCheck %s

void main(int dest, int src, unsigned divisor) {
  uint32_t_pair_t __local *sptr = (uint32_t_pair_t __local *) src;
  uint32_t_pair_t __local *dptr = (uint32_t_pair_t __local *) dest;
  uint32_t_pair_t quot_rem = *sptr;
  quot_rem = s_u32_udiv_step_s(quot_rem, divisor, 5);
  *dptr = quot_rem;
}

// load dividend and remainder to a register pair
// CHECK-DAG: ld_l  %S[[ZN:[0-9]+]], %S1
// CHECK-DAG: ld_l  %S[[ZNN:[0-9]+]], %S{{[0-9]+}}

// CHECK:     udiv_step.u32 0x5 %Z[[ZN]], %S2, %SP0

// CHECK-DAG: st_l %S0,    %S[[ZN]]
// CHECK-DAG: st_l %S{{[0-9]+}}, %S[[ZNN]]
