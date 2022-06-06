// RUN: %codegen -S -O1 -triple tpc-none-none -std=rc99 -bfloat16 -target-cpu doron1 %s -o - | FileCheck %s

void main(int src, int dest, _Bool pred) {
  float64 __local *src_ptr = (float64 __local *)src;
  float64 x0 = *src_ptr++;
  int64 __local *res_ptr = (int64 __local *)dest;

  *res_ptr++ = v_f32_extract_exp_b(x0, SW_PRE_LOG, 0, 1, 0);
  // CHECK: extract_exp.f32 pre_log %V{{[0-9]+}}, %V{{[0-9]+}}
  x0 = *src_ptr++;

  int res = 0;

  *res_ptr++ = v_f32_extract_exp_b(x0, SW_PRE_LOG, res, pred, 0);
  // CHECK: extract_exp.f32 pre_log %V{{[0-9]+}}, %V{{[0-9]+}}, %SP{{[0-9]+}}
  x0 = *src_ptr++;

  bool256 vpred = v_i1_mov_i1_b(pred, 0, (bool256){0}, 1, 0);
  *res_ptr++ = v_f32_extract_exp_vb(x0, SW_PRE_LOG, res, to_bool64(vpred), 0);
  // CHECK: extract_exp.f32 pre_log %V{{[0-9]+}}, %V{{[0-9]+}}, %VP{{[0-9]+}}
}
