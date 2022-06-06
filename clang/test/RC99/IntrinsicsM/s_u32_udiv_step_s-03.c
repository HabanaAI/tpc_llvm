// RUN: %codegen -S -O1 -triple tpc-none-none -std=rc99 %s -o - | FileCheck %s

void main(int dest, int src, unsigned divisor) {
  uint32_t_pair_t __local *sptr = (uint32_t_pair_t __local *) src;
  unsigned __local *dptr = (unsigned __local *) dest;
  uint32_t_pair_t quot_rem = *sptr;
  quot_rem = u32_udiv_step(divisor, 5, 0, quot_rem, 1, 0);
  dptr[0] = quot_rem.v1;
  dptr[1] = quot_rem.v2;
}

// load dividend and remainder to a register pair
// CHECK-DAG: ld_l  %S[[ZN:[0-9]+]], %S1
// CHECK-DAG: ld_l  %S[[ZNN:[0-9]+]], %S{{[0-9]+}}

// CHECK:     udiv_step.u32 0x5 %Z[[ZN]], %S2

// CHECK-DAG: st_l %S0,    %S[[ZN]]
// CHECK-DAG: st_l %S{{[0-9]+}}, %S[[ZNN]]
