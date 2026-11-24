// RUN: %tpc_clang -march=gaudi2 -O2 -S %s -o - | FileCheck %s
// RUN: %tpc_clang -march=gaudi2 -O0 -S %s -o - | FileCheck %s

// v_broadcast_element_f32, v_element_shift_up_f32, v_i32_reduce_min and
// SW_FP8_BIAS7, which Habana's headers provide: a shuffle within the dual
// groups and dual-group moves, as Habana's compiler emits them (difftest
// broadcast_shift_reduce_min.c checks the values).

void main(tensor in, tensor out, int n) {
  int5 c = 0;
  float64 x = v_f32_ld_tnsr_b(c, in);
  int64 i = v_i32_ld_tnsr_b(c, in);
  v_f32_st_tnsr(c, out, v_broadcast_element_f32(x, n)); c[1]++;
  v_f32_st_tnsr(c, out, v_element_shift_up_f32(x, n)); c[1]++;
  v_i32_st_tnsr(c, out, v_i32_reduce_min(i)); c[1]++;
  s_i32_st_l(0, SW_FP8_BIAS7);
}

// The broadcast: dual group 3 to all four, under a scalar predicate.
// CHECK-DAG: mov_dg all sdg0=3 sdg1=3 sdg2=3 sdg3=3 weg0=3 weg1=3 weg2=3 weg3=3 {{.*}}, %SP
// The shift: the last of its moves, under a vector predicate.
// CHECK-DAG: mov_dg all sdg0=1 sdg1=2 sdg2=3 sdg3=0 weg0=2 weg1=2 weg2=2 weg3=2 {{.*}}, %VP
// CHECK-DAG: shuffle.i32
// CHECK-DAG: min.i32
