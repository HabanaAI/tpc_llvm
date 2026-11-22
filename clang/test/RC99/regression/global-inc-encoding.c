// The increment that ld_g and st_g add to their address, as encoded. The
// Gen4Plus forms took Gen3's switch layout, with AUTO_INC in bit 0 and the
// mask-register flag in bit 1, so the objects stepped by another amount than
// the assembly said (inc_8 became 5, inc_2 inc_4, inc_4 inc_1 or inc_0).
//
// RUN: %tpc_clang -march=gaudi2 -O1 -no-instr-compress -c %s -o %t2.o
// RUN: %disasm %t2.o | FileCheck %s
// RUN: %tpc_clang -march=gaudi3 -O1 -no-instr-compress -c %s -o %t3.o
// RUN: %disasm %t3.o | FileCheck %s

void main(tensor in, tensor out, int mask) {
  int5 c = {0, 0, 0, 0, 0};
  int __global *q = (int __global *)gen_addr(c, out, 0, 0, 1, 0);
  s_i32_st_g_inc(&q, 1, SW_INC_8, 1, 0);
  short __global *h = (short __global *)q;
  s_i16_st_g_inc(&h, 2, SW_INC_2, 1, 0);
  *h = 3;

  void __global *a = gen_addr(c, in, 0, 0, 1, 0);
  float64 f = 0;
  f = v_f32_ld_g_inc_partial_b(&a, 15, 0, 0, f, 1, 0);
  bfloat128 b = 0;
  b = v_bf16_ld_g_inc_partial_b(&a, 15, 0, 0, b, 1, 0);
  int5 x = 0;
  x = i_i32_ld_g_inc(&a, 0b00011, 0, x, 1, 0);
  int5 y = 0;
  y = i_i32_ld_g_inc(&a, mask, 0, y, 1, 0);

  v_f32_st_tnsr(c, out, f);
  c[1] = 1;
  v_bf16_st_tnsr(c, out, b);
  c[1] = 2;
  v_i32_st_tnsr(c, out, x[1] + y[0]);
}

// CHECK-DAG: st_g inc_8 AD{{[0-9]+}}, S
// CHECK-DAG: st_g inc_2 AD{{[0-9]+}}, S
// CHECK-DAG: ld_g inc_4 partial V{{[0-9]+}}, AD
// CHECK-DAG: ld_g inc_2 partial V{{[0-9]+}}, AD
// CHECK-DAG: ld_g inc_4 b00011 I{{[0-9]+}}, AD
// CHECK-DAG: ld_g inc_4 M0 I{{[0-9]+}}, AD
