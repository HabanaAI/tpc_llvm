// RUN: %codegen -S -O1 -triple tpc-none-none -std=rc99 %s -o - | FileCheck %s
// RUN: %codegen -S -O1 -triple tpc-none-none -std=rc99 -bfloat16 -target-cpu gaudi %s -o - | FileCheck --check-prefix=CHECK-GAUDI %s


void main(int dest, _Bool pred) {
  int64 __local *res_ptr = (int64 __local *)dest;

  *res_ptr++ = v_f32_extract_exp_s(0.8, 1);
  // CHECK-DAG:       extract_exp.f32 subtract_bias %V{{[0-9]+}}, 0x3f4ccccd, %SP0
  // CHECK-GAUDI-DAG: extract_exp.f32 biased %V{{[0-9]+}}, 0x3f4ccccd, %SP0

  *res_ptr++ = v_f32_extract_exp_s(0.8, 0);
  // CHECK-DAG:       extract_exp.f32 %V{{[0-9]+}}, 0x3f4ccccd, %SP0
  // CHECK-GAUDI-DAG: extract_exp.f32 %V{{[0-9]+}}, 0x3f4ccccd, %SP0

  int res = 0;

  *res_ptr++ = v_f32_extract_exp_s_b(0.8, res, 1, pred, 0);
  // CHECK-DAG:       extract_exp.f32 subtract_bias %V{{[0-9]+}}, 0x3f4ccccd, %SP{{[0-9]+}}
  // CHECK-GAUDI-DAG: extract_exp.f32 biased %V{{[0-9]+}}, 0x3f4ccccd, %SP{{[0-9]+}}

  *res_ptr++ = v_f32_extract_exp_s_b(0.8, res, 0, pred, 0);
  // CHECK-DAG:       extract_exp.f32 %V{{[0-9]+}}, 0x3f4ccccd, %SP{{[0-9]+}}
  // CHECK-GAUDI-DAG: extract_exp.f32 %V{{[0-9]+}}, 0x3f4ccccd, %SP{{[0-9]+}}
}
