// RUN: %tpc_clang -S -O2 -march=gaudi2 %s -o - | FileCheck %s
// RUN: %tpc_clang -S -O2 -march=gaudi3 %s -o - | FileCheck %s

// Two identical stores are two stores, as with Habana's compiler: an MMIO
// write happens twice, an RMW add adds twice, and a squeeze store writes after
// the data of the one before. EarlyCSE used to keep one of each.

void main(tensor t, int a, int v) {
  int5 c = {0, 0, 0, 0, 0};
  int64 x = v_i32_ld_tnsr_b(c, t);
  s_i32_st_l(a, v, SW_MMIO);
  s_i32_st_l(a, v, SW_MMIO);
  v_i32_st_tnsr(c, t, x);
  v_i32_st_tnsr(c, t, x);
  v_i32_st_tnsr_rmw(c, t, x, RMW_DT_INT32 | RMW_OP_ADD | RMW_SET, 0, 1, 0);
  v_i32_st_tnsr_rmw(c, t, x, RMW_DT_INT32 | RMW_OP_ADD | RMW_SET, 0, 1, 0);
  bool256 p = {0};
  reset_sqz_cntr(2, 0);
  v_i32_st_tnsr_sqz(c, t, 2, x, 0, p, 1);
  v_i32_st_tnsr_sqz(c, t, 2, x, 0, p, 1);
}

// CHECK-LABEL: main:
// CHECK-COUNT-2: st_l mmio %S{{[0-9]+}}, %S{{[0-9]+}}
// CHECK-NOT:     st_l mmio %S{{[0-9]+}}, %S{{[0-9]+}}
// CHECK-COUNT-2: st_tnsr 0x0, %I{{[0-9]+}}, %V{{[0-9]+}}
// CHECK-NOT:     st_tnsr 0x0, %I{{[0-9]+}}, %V{{[0-9]+}}
// CHECK-COUNT-2: st_tnsr rmw_sel 0x0,
// CHECK-NOT:     st_tnsr rmw_sel
// CHECK-COUNT-2: st_tnsr_sqz 0x0, 0x2,
// CHECK-NOT:     st_tnsr_sqz
// CHECK:         halt
