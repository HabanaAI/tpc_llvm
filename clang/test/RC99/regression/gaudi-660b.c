// RUN: %codegen -S -triple tpc-none-none -std=rc99 -O1 %s -o - | FileCheck %s

void main (int dest, float v, _Bool pred_1) {
  float64 out_value[4] = { 0, 0, 0, 0 };
  float64 out_value_1[4];
  float64 weight_value[4] = { 1, 1, 1, 1};
  float64 in_value[4] = { v, v, v, v };
  out_value_1[0] = v_f32_mac_v_v_b(weight_value[0], in_value[0], out_value[0], 0, pred_1, 0);
  *(float64 __local *)dest = out_value_1[0];
}

// CHECK-DAG: mov.f32  %V[[ACC:[0-9]+]], 0x0
// CHECK-DAG: mov.f32  %V[[OP1:[0-9]+]], 0x3f800000
// CHECK:     mac.f32  %V[[ACC:[0-9]+]], %V[[OP1]], %S1, %SP{{[0-9]+}}
// CHECK:     st_l_v   %S0, 0x0, %V[[ACC]], %SP0
