// RUN: %codegen -S -O1 -triple tpc-none-none -std=rc99 -target-cpu doron1 %s -o - | FileCheck %s
// XFAIL: *

void main(tensor input, int other_input, int dest, int vpredp) {
  reset_sqz_cntr(0, 0);
  int5 index = {0};
  int64 __local *vector_ptr = (int64 __local *)dest;

  bool256 __local *vpred_ptr = (bool256 __local *)vpredp;
  bool256 vpred = *vpred_ptr;
  
  float64 __local *dest_ptr = (float64 __local *)vector_ptr;
  float64 res = {0};

  v_f32_st_tnsr_sqz(index, input, 0, res, 0, vpred, 0);
  *dest_ptr++ = res;
  vector_ptr = (int64 __local *)dest_ptr;
}

// CHECK: mov.f32  [[ZERO:%S[0-9]+]], 0x0
// CHECK-DAG: st_l mmio 0x0, [[ZERO]]
// CHECK-DAG: st_l mmio 0x0, [[ZERO]]
