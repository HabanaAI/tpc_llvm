// RUN: %codegen -S -triple tpc-none-none -std=rc99 -O1 %s -o - | FileCheck %s

void main (int dest, float v, _Bool pred_1) {
  typedef struct _float256 {float64 v1; float64 v2; float64 v3; float64 v4;} float256;
  float256 out_value_1 = {0, 0, 0, 0},
           weight_value_1_float = {1, 1, 1, 1},
           in_value_1_float = { v, v, v, v };
  out_value_1.v1 = v_f32_mac_b(weight_value_1_float.v1, in_value_1_float.v1, out_value_1.v1, (0) << 1, pred_1, 0);
  *(float64 __local *)dest = out_value_1.v1;
}

// CHECK-DAG: mov.f32  %V[[ACC:[0-9]+]], 0x0
// CHECK-DAG: mov.f32  %V[[OP1:[0-9]+]], 0x3f800000
// CHECK:     mac.f32  %V[[ACC:[0-9]+]], %V[[OP1]], %S1, %SP{{[0-9]+}}
// CHECK:     st_l_v   %S0, 0x0, %V[[ACC]]
