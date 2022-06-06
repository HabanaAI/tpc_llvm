// RUN: %codegen -S -O1 -triple tpc-none-none -std=rc99 %s -o - | FileCheck %s

void main(int dest, unsigned dividend, unsigned divisor, _Bool pred) {
  uint32_t_pair_t __local *dptr = (uint32_t_pair_t __local *) dest;
  uint32_t_pair_t quot_rem = { dividend, 0 };
  quot_rem = u32_udiv_step(divisor, 1, 0, quot_rem, pred, 0);
  *dptr = quot_rem;
}

// move 'dividend' to a register pair, clear remainder
// CHECK-DAG: mov.f32      %S[[ZN:[0-9]+]], %S1
// CHECK-DAG: mov.i32      %S[[ZNN:[0-9]+]], 0x0

// CHECK:     udiv_step.u32 0x1 %Z[[ZN]], %S2, %SP{{[0-9]+}}

// CHECK-DAG: st_l {{.+}},    %S[[ZN]]
// CHECK-DAG: st_l {{.+}},    %S[[ZNN]]
