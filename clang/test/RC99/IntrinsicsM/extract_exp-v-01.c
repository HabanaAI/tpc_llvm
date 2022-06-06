// RUN: %codegen -S -O1 -triple tpc-none-none -std=rc99 %s -o - | FileCheck %s
// RUN: %codegen -S -O1 -triple tpc-none-none -std=rc99 -bfloat16 -target-cpu gaudi %s -o - | FileCheck --check-prefix=CHECK-GAUDI %s
// RUN: %codegen -S -O1 -triple tpc-none-none -std=rc99 -bfloat16 -target-cpu goya2 %s -o - | FileCheck --check-prefix=CHECK-GAUDI %s

void main(int src, int dest, _Bool pred) {
  float64 __local *src_ptr = (float64 __local *)src;
  float64 x0 = *src_ptr++;
  int64 __local *res_ptr = (int64 __local *)dest;

  *res_ptr++ = v_f32_extract_exp_b(x0, 1, 0, 1, 0);
  // CHECK:       extract_exp.f32 subtract_bias %V{{[0-9]+}}, %V{{[0-9]+}}
  // CHECK-GAUDI: extract_exp.f32 biased %V{{[0-9]+}}, %V{{[0-9]+}}
  x0 = *src_ptr++;

  *res_ptr++ = v_f32_extract_exp_b(x0, 0, 0, 1, 0);
  // CHECK:       extract_exp.f32 %V{{[0-9]+}}, %V{{[0-9]+}}
  // CHECK-GAUDI: extract_exp.f32 %V{{[0-9]+}}, %V{{[0-9]+}}
  x0 = *src_ptr++;

  int res = 0;

  *res_ptr++ = v_f32_extract_exp_b(x0, 1, res, pred, 0);
  // CHECK:       extract_exp.f32 subtract_bias %V{{[0-9]+}}, %V{{[0-9]+}}, %SP{{[0-9]+}}
  // CHECK-GAUDI: extract_exp.f32 biased %V{{[0-9]+}}, %V{{[0-9]+}}, %SP{{[0-9]+}}
  x0 = *src_ptr++;

  *res_ptr++ = v_f32_extract_exp_b(x0, 0, res, pred, 0);
  // CHECK:       extract_exp.f32 %V{{[0-9]+}}, %V{{[0-9]+}}, %SP{{[0-9]+}}
  // CHECK-GAUDI: extract_exp.f32 %V{{[0-9]+}}, %V{{[0-9]+}}, %SP{{[0-9]+}}
  x0 = *src_ptr++;
  
  bool256 vpred = v_i1_mov_i1_b(pred, 0, (bool256){0}, 1, 0);

  *res_ptr++ = v_f32_extract_exp_vb(x0, 1, res, to_bool64(vpred), 0);
  // CHECK:       extract_exp.f32 subtract_bias %V{{[0-9]+}}, %V{{[0-9]+}}, %VP{{[0-9]+}}
  // CHECK-GAUDI: extract_exp.f32 biased %V{{[0-9]+}}, %V{{[0-9]+}}, %VP{{[0-9]+}}
  x0 = *src_ptr++;

  *res_ptr++ = v_f32_extract_exp_vb(x0, 0, res, to_bool64(vpred), 0);
  // CHECK:       extract_exp.f32 %V{{[0-9]+}}, %V{{[0-9]+}}, %VP{{[0-9]+}}
  // CHECK-GAUDI: extract_exp.f32 %V{{[0-9]+}}, %V{{[0-9]+}}, %VP{{[0-9]+}}
}
