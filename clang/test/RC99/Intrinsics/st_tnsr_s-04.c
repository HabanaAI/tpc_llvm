// RUN: %codegen -S -O1 -triple tpc-none-none -std=rc99 -target-cpu gaudi2 -bfloat16 %s -o - | FileCheck %s
// RUN: %codegen -S -O1 -triple tpc-none-none -std=rc99 -target-cpu doron1 -bfloat16 %s -o - | FileCheck %s

void main(tensor input, int other_input, int value, _Bool pred) {
  int5 index = {0};

  float __local *float_value_ptr = (float __local *)value;
  int __local *int_value_ptr = (int __local *)value;

  float float_value = *float_value_ptr;
  int int_value = *int_value_ptr;

  {
    s_f32_st_tnsr_s(index, input, float_value, 0, pred, 0);
    // CHECK-DAG: st_tnsr_s 0x0, %I{{[0-9]+}}, %S{{[0-9]+}}, %SP{{[0-9]+}}
    s_f32_st_tnsr_s(index, other_input, float_value, 0, pred, 0);
    // CHECK-DAG: st_tnsr_s %ST_TNSR_ID_REG, %I{{[0-9]+}}, %S{{[0-9]+}}, %SP{{[0-9]+}}
    s_f32_st_tnsr_s_rmw(index, input, float_value, 1, 0, pred, 0);
    // CHECK-DAG: st_tnsr_s rmw_sel 0x0, %I{{[0-9]+}}, %S{{[0-9]+}}, %ST_RMW_REG, %SP{{[0-9]+}}
    s_f32_st_tnsr_s_rmw(index, other_input, float_value, 1, 0, pred, 0);
    // CHECK-DAG: st_tnsr_s rmw_sel %ST_TNSR_ID_REG, %I{{[0-9]+}}, %S{{[0-9]+}}, %ST_RMW_REG, %SP{{[0-9]+}}
    s_f32_st_tnsr_s(index, input, 2.0, 0, pred, 0);
    // CHECK-DAG: st_tnsr_s 0x0, %I{{[0-9]+}}, 0x40000000, %SP{{[0-9]+}}
    s_f32_st_tnsr_s(index, other_input, 2.0, 0, pred, 0);
    // CHECK-DAG: st_tnsr_s %ST_TNSR_ID_REG, %I{{[0-9]+}}, 0x40000000, %SP{{[0-9]+}}
    s_f32_st_tnsr_s_rmw(index, input, 2.0, 1, 0, pred, 0);
    // CHECK-DAG: st_tnsr_s rmw_sel 0x0, %I{{[0-9]+}}, 0x40000000, %ST_RMW_REG, %SP{{[0-9]+}}
    s_f32_st_tnsr_s_rmw(index, other_input, 2.0, 1, 0, pred, 0);
    // CHECK-DAG: st_tnsr_s rmw_sel %ST_TNSR_ID_REG, %I{{[0-9]+}}, 0x40000000, %ST_RMW_REG, %SP{{[0-9]+}}
    
  }
  {
    s_bf16_st_tnsr_s(index, input, float_value, 0, pred, 0);
    // CHECK-DAG: st_tnsr_s 0x0, %I{{[0-9]+}}, %S{{[0-9]+}}, %SP{{[0-9]+}}
    s_bf16_st_tnsr_s(index, other_input, float_value, 0, pred, 0);
    // CHECK-DAG: st_tnsr_s %ST_TNSR_ID_REG, %I{{[0-9]+}}, %S{{[0-9]+}}, %SP{{[0-9]+}}
    s_bf16_st_tnsr_s_rmw(index, input, float_value, 1, 0, pred, 0);
    // CHECK-DAG: st_tnsr_s rmw_sel 0x0, %I{{[0-9]+}}, %S{{[0-9]+}}, %ST_RMW_REG, %SP{{[0-9]+}}
    s_bf16_st_tnsr_s_rmw(index, other_input, float_value, 1, 0, pred, 0);
    // CHECK-DAG: st_tnsr_s rmw_sel %ST_TNSR_ID_REG, %I{{[0-9]+}}, %S{{[0-9]+}}, %ST_RMW_REG, %SP{{[0-9]+}}
    s_bf16_st_tnsr_s(index, input, 2.0, 0, pred, 0);
    // CHECK-DAG: st_tnsr_s 0x0, %I{{[0-9]+}}, 0x4000, %SP{{[0-9]+}}
    s_bf16_st_tnsr_s(index, other_input, 2.0, 0, pred, 0);
    // CHECK-DAG: st_tnsr_s %ST_TNSR_ID_REG, %I{{[0-9]+}}, 0x4000, %SP{{[0-9]+}}
    s_bf16_st_tnsr_s_rmw(index, input, 2.0, 1, 0, pred, 0);
    // CHECK-DAG: st_tnsr_s rmw_sel 0x0, %I{{[0-9]+}}, 0x4000, %ST_RMW_REG, %SP{{[0-9]+}}
    s_bf16_st_tnsr_s_rmw(index, other_input, 2.0, 1, 0, pred, 0);
    // CHECK-DAG: st_tnsr_s rmw_sel %ST_TNSR_ID_REG, %I{{[0-9]+}}, 0x4000, %ST_RMW_REG, %SP{{[0-9]+}}
  }
  {
    s_f16_st_tnsr_s(index, input, float_value, 0, pred, 0);
    // CHECK-DAG: st_tnsr_s 0x0, %I{{[0-9]+}}, %S{{[0-9]+}}, %SP{{[0-9]+}}
    s_f16_st_tnsr_s(index, other_input, float_value, 0, pred, 0);
    // CHECK-DAG: st_tnsr_s %ST_TNSR_ID_REG, %I{{[0-9]+}}, %S{{[0-9]+}}, %SP{{[0-9]+}}
    s_f16_st_tnsr_s_rmw(index, input, float_value, 1, 0, pred, 0);
    // CHECK-DAG: st_tnsr_s rmw_sel 0x0, %I{{[0-9]+}}, %S{{[0-9]+}}, %ST_RMW_REG, %SP{{[0-9]+}}
    s_f16_st_tnsr_s_rmw(index, other_input, float_value, 1, 0, pred, 0);
    // CHECK-DAG: st_tnsr_s rmw_sel %ST_TNSR_ID_REG, %I{{[0-9]+}}, %S{{[0-9]+}}, %ST_RMW_REG, %SP{{[0-9]+}}
    s_f16_st_tnsr_s(index, input, 2.0, 0, pred, 0);
    // CHECK-DAG: st_tnsr_s 0x0, %I{{[0-9]+}}, 0x4000, %SP{{[0-9]+}}
    s_f16_st_tnsr_s(index, other_input, 2.0, 0, pred, 0);
    // CHECK-DAG: st_tnsr_s %ST_TNSR_ID_REG, %I{{[0-9]+}}, 0x4000, %SP{{[0-9]+}}
    s_f16_st_tnsr_s_rmw(index, input, 2.0, 1, 0, pred, 0);
    // CHECK-DAG: st_tnsr_s rmw_sel 0x0, %I{{[0-9]+}}, 0x4000, %ST_RMW_REG, %SP{{[0-9]+}}
    s_f16_st_tnsr_s_rmw(index, other_input, 2.0, 1, 0, pred, 0);
    // CHECK-DAG: st_tnsr_s rmw_sel %ST_TNSR_ID_REG, %I{{[0-9]+}}, 0x4000, %ST_RMW_REG, %SP{{[0-9]+}}
  }
  {
    s_f8_st_tnsr_s(index, input, float_value, 0, pred, 0);
    // CHECK-DAG: st_tnsr_s 0x0, %I{{[0-9]+}}, %S{{[0-9]+}}, %SP{{[0-9]+}}
    s_f8_st_tnsr_s(index, other_input, float_value, 0, pred, 0);
    // CHECK-DAG: st_tnsr_s %ST_TNSR_ID_REG, %I{{[0-9]+}}, %S{{[0-9]+}}, %SP{{[0-9]+}}
    s_f8_st_tnsr_s(index, input, 2.0, 0, pred, 0);
    // CHECK-DAG: st_tnsr_s 0x0, %I{{[0-9]+}}, 0x40, %SP{{[0-9]+}}
    s_f8_st_tnsr_s(index, other_input, 2.0, 0, pred, 0);
    // CHECK-DAG: st_tnsr_s %ST_TNSR_ID_REG, %I{{[0-9]+}}, 0x40, %SP{{[0-9]+}}
  }
  {
    s_h8_st_tnsr_s(index, input, float_value, 0, pred, 0);
    // CHECK-DAG: st_tnsr_s 0x0, %I{{[0-9]+}}, %S{{[0-9]+}}, %SP{{[0-9]+}}
    s_h8_st_tnsr_s(index, other_input, float_value, 0, pred, 0);
    // CHECK-DAG: st_tnsr_s %ST_TNSR_ID_REG, %I{{[0-9]+}}, %S{{[0-9]+}}, %SP{{[0-9]+}}
    s_h8_st_tnsr_s_rmw(index, input, float_value, 1, 0, pred, 0);
    // CHECK-DAG: st_tnsr_s rmw_sel 0x0, %I{{[0-9]+}}, %S{{[0-9]+}}, %ST_RMW_REG, %SP{{[0-9]+}}
    s_h8_st_tnsr_s_rmw(index, other_input, float_value, 1, 0, pred, 0);
    // CHECK-DAG: st_tnsr_s rmw_sel %ST_TNSR_ID_REG, %I{{[0-9]+}}, %S{{[0-9]+}}, %ST_RMW_REG, %SP{{[0-9]+}}
    s_h8_st_tnsr_s(index, input, 2.0, 0, pred, 0);
    // CHECK-DAG: st_tnsr_s 0x0, %I{{[0-9]+}}, 0x40, %SP{{[0-9]+}}
    s_h8_st_tnsr_s(index, other_input, 2.0, 0, pred, 0);
    // CHECK-DAG: st_tnsr_s %ST_TNSR_ID_REG, %I{{[0-9]+}}, 0x40, %SP{{[0-9]+}}
    s_h8_st_tnsr_s_rmw(index, input, 2.0, 1, 0, pred, 0);
    // CHECK-DAG: st_tnsr_s rmw_sel 0x0, %I{{[0-9]+}}, 0x40, %ST_RMW_REG, %SP{{[0-9]+}}
    s_h8_st_tnsr_s_rmw(index, other_input, 2.0, 1, 0, pred, 0);
    // CHECK-DAG: st_tnsr_s rmw_sel %ST_TNSR_ID_REG, %I{{[0-9]+}}, 0x40, %ST_RMW_REG, %SP{{[0-9]+}}
  }
  {
    s_i32_st_tnsr_s(index, input, int_value, 0, pred, 0);
    // CHECK-DAG: st_tnsr_s 0x0, %I{{[0-9]+}}, %S{{[0-9]+}}, %SP{{[0-9]+}}
    s_i32_st_tnsr_s(index, other_input, int_value, 0, pred, 0);
    // CHECK-DAG: st_tnsr_s %ST_TNSR_ID_REG, %I{{[0-9]+}}, %S{{[0-9]+}}, %SP{{[0-9]+}}
    s_i32_st_tnsr_s_rmw(index, input, int_value, 1, 0, pred, 0);
    // CHECK-DAG: st_tnsr_s rmw_sel 0x0, %I{{[0-9]+}}, %S{{[0-9]+}}, %ST_RMW_REG, %SP{{[0-9]+}}
    s_i32_st_tnsr_s_rmw(index, other_input, int_value, 1, 0, pred, 0);
    // CHECK-DAG: st_tnsr_s rmw_sel %ST_TNSR_ID_REG, %I{{[0-9]+}}, %S{{[0-9]+}}, %ST_RMW_REG, %SP{{[0-9]+}}
    s_i32_st_tnsr_s(index, input, 2, 0, pred, 0);
    // CHECK-DAG: st_tnsr_s 0x0, %I{{[0-9]+}}, 0x2, %SP{{[0-9]+}}
    s_i32_st_tnsr_s(index, other_input, 2, 0, pred, 0);
    // CHECK-DAG: st_tnsr_s %ST_TNSR_ID_REG, %I{{[0-9]+}}, 0x2, %SP{{[0-9]+}}
    s_i32_st_tnsr_s_rmw(index, input, 2.0, 1, 0, pred, 0);
    // CHECK-DAG: st_tnsr_s rmw_sel 0x0, %I{{[0-9]+}}, 0x2, %ST_RMW_REG, %SP{{[0-9]+}}
    s_i32_st_tnsr_s_rmw(index, other_input, 2.0, 1, 0, pred, 0);
    // CHECK-DAG: st_tnsr_s rmw_sel %ST_TNSR_ID_REG, %I{{[0-9]+}}, 0x2, %ST_RMW_REG, %SP{{[0-9]+}}
  }
  {
    s_u32_st_tnsr_s(index, input, int_value, 0, pred, 0);
    // CHECK-DAG: st_tnsr_s 0x0, %I{{[0-9]+}}, %S{{[0-9]+}}, %SP{{[0-9]+}}
    s_u32_st_tnsr_s(index, other_input, int_value, 0, pred, 0);
    // CHECK-DAG: st_tnsr_s %ST_TNSR_ID_REG, %I{{[0-9]+}}, %S{{[0-9]+}}, %SP{{[0-9]+}}
    s_u32_st_tnsr_s_rmw(index, input, int_value, 1, 0, pred, 0);
    // CHECK-DAG: st_tnsr_s rmw_sel 0x0, %I{{[0-9]+}}, %S{{[0-9]+}}, %ST_RMW_REG, %SP{{[0-9]+}}
    s_u32_st_tnsr_s_rmw(index, other_input, int_value, 1, 0, pred, 0);
    // CHECK-DAG: st_tnsr_s rmw_sel %ST_TNSR_ID_REG, %I{{[0-9]+}}, %S{{[0-9]+}}, %ST_RMW_REG, %SP{{[0-9]+}}
    s_u32_st_tnsr_s(index, input, 2, 0, pred, 0);
    // CHECK-DAG: st_tnsr_s 0x0, %I{{[0-9]+}}, 0x2, %SP{{[0-9]+}}
    s_u32_st_tnsr_s(index, other_input, 2, 0, pred, 0);
    // CHECK-DAG: st_tnsr_s %ST_TNSR_ID_REG, %I{{[0-9]+}}, 0x2, %SP{{[0-9]+}}
    s_u32_st_tnsr_s_rmw(index, input, 2.0, 1, 0, pred, 0);
    // CHECK-DAG: st_tnsr_s rmw_sel 0x0, %I{{[0-9]+}}, 0x2, %ST_RMW_REG, %SP{{[0-9]+}}
    s_u32_st_tnsr_s_rmw(index, other_input, 2.0, 1, 0, pred, 0);
    // CHECK-DAG: st_tnsr_s rmw_sel %ST_TNSR_ID_REG, %I{{[0-9]+}}, 0x2, %ST_RMW_REG, %SP{{[0-9]+}}
  }
  {
    s_i16_st_tnsr_s(index, input, int_value, 0, pred, 0);
    // CHECK-DAG: st_tnsr_s 0x0, %I{{[0-9]+}}, %S{{[0-9]+}}, %SP{{[0-9]+}}
    s_i16_st_tnsr_s(index, other_input, int_value, 0, pred, 0);
    // CHECK-DAG: st_tnsr_s %ST_TNSR_ID_REG, %I{{[0-9]+}}, %S{{[0-9]+}}, %SP{{[0-9]+}}
    s_i16_st_tnsr_s_rmw(index, input, int_value, 1, 0, pred, 0);
    // CHECK-DAG: st_tnsr_s rmw_sel 0x0, %I{{[0-9]+}}, %S{{[0-9]+}}, %ST_RMW_REG, %SP{{[0-9]+}}
    s_i16_st_tnsr_s_rmw(index, other_input, int_value, 1, 0, pred, 0);
    // CHECK-DAG: st_tnsr_s rmw_sel %ST_TNSR_ID_REG, %I{{[0-9]+}}, %S{{[0-9]+}}, %ST_RMW_REG, %SP{{[0-9]+}}
    s_i16_st_tnsr_s(index, input, 2, 0, pred, 0);
    // CHECK-DAG: st_tnsr_s 0x0, %I{{[0-9]+}}, 0x2, %SP{{[0-9]+}}
    s_i16_st_tnsr_s(index, other_input, 2, 0, pred, 0);
    // CHECK-DAG: st_tnsr_s %ST_TNSR_ID_REG, %I{{[0-9]+}}, 0x2, %SP{{[0-9]+}}
    s_i16_st_tnsr_s_rmw(index, input, 2.0, 1, 0, pred, 0);
    // CHECK-DAG: st_tnsr_s rmw_sel 0x0, %I{{[0-9]+}}, 0x2, %ST_RMW_REG, %SP{{[0-9]+}}
    s_i16_st_tnsr_s_rmw(index, other_input, 2.0, 1, 0, pred, 0);
    // CHECK-DAG: st_tnsr_s rmw_sel %ST_TNSR_ID_REG, %I{{[0-9]+}}, 0x2, %ST_RMW_REG, %SP{{[0-9]+}}
  }
  {
    s_u16_st_tnsr_s(index, input, int_value, 0, pred, 0);
    // CHECK-DAG: st_tnsr_s 0x0, %I{{[0-9]+}}, %S{{[0-9]+}}, %SP{{[0-9]+}}
    s_u16_st_tnsr_s(index, other_input, int_value, 0, pred, 0);
    // CHECK-DAG: st_tnsr_s %ST_TNSR_ID_REG, %I{{[0-9]+}}, %S{{[0-9]+}}, %SP{{[0-9]+}}
    s_u16_st_tnsr_s_rmw(index, input, int_value, 1, 0, pred, 0);
    // CHECK-DAG: st_tnsr_s rmw_sel 0x0, %I{{[0-9]+}}, %S{{[0-9]+}}, %ST_RMW_REG, %SP{{[0-9]+}}
    s_u16_st_tnsr_s_rmw(index, other_input, int_value, 1, 0, pred, 0);
    // CHECK-DAG: st_tnsr_s rmw_sel %ST_TNSR_ID_REG, %I{{[0-9]+}}, %S{{[0-9]+}}, %ST_RMW_REG, %SP{{[0-9]+}}
    s_u16_st_tnsr_s(index, input, 2, 0, pred, 0);
    // CHECK-DAG: st_tnsr_s 0x0, %I{{[0-9]+}}, 0x2, %SP{{[0-9]+}}
    s_u16_st_tnsr_s(index, other_input, 2, 0, pred, 0);
    // CHECK-DAG: st_tnsr_s %ST_TNSR_ID_REG, %I{{[0-9]+}}, 0x2, %SP{{[0-9]+}}
    s_u16_st_tnsr_s_rmw(index, input, 2.0, 1, 0, pred, 0);
    // CHECK-DAG: st_tnsr_s rmw_sel 0x0, %I{{[0-9]+}}, 0x2, %ST_RMW_REG, %SP{{[0-9]+}}
    s_u16_st_tnsr_s_rmw(index, other_input, 2.0, 1, 0, pred, 0);
    // CHECK-DAG: st_tnsr_s rmw_sel %ST_TNSR_ID_REG, %I{{[0-9]+}}, 0x2, %ST_RMW_REG, %SP{{[0-9]+}}
  }
  {
    s_i8_st_tnsr_s(index, input, int_value, 0, pred, 0);
    // CHECK-DAG: st_tnsr_s 0x0, %I{{[0-9]+}}, %S{{[0-9]+}}, %SP{{[0-9]+}}
    s_i8_st_tnsr_s(index, other_input, int_value, 0, pred, 0);
    // CHECK-DAG: st_tnsr_s %ST_TNSR_ID_REG, %I{{[0-9]+}}, %S{{[0-9]+}}, %SP{{[0-9]+}}
    s_i8_st_tnsr_s_rmw(index, input, int_value, 1, 0, pred, 0);
    // CHECK-DAG: st_tnsr_s rmw_sel 0x0, %I{{[0-9]+}}, %S{{[0-9]+}}, %ST_RMW_REG, %SP{{[0-9]+}}
    s_i8_st_tnsr_s_rmw(index, other_input, int_value, 1, 0, pred, 0);
    // CHECK-DAG: st_tnsr_s rmw_sel %ST_TNSR_ID_REG, %I{{[0-9]+}}, %S{{[0-9]+}}, %ST_RMW_REG, %SP{{[0-9]+}}
    s_i8_st_tnsr_s(index, input, 2, 0, pred, 0);
    // CHECK-DAG: st_tnsr_s 0x0, %I{{[0-9]+}}, 0x2, %SP{{[0-9]+}}
    s_i8_st_tnsr_s(index, other_input, 2, 0, pred, 0);
    // CHECK-DAG: st_tnsr_s %ST_TNSR_ID_REG, %I{{[0-9]+}}, 0x2, %SP{{[0-9]+}}
    s_i8_st_tnsr_s_rmw(index, input, 2.0, 1, 0, pred, 0);
    // CHECK-DAG: st_tnsr_s rmw_sel 0x0, %I{{[0-9]+}}, 0x2, %ST_RMW_REG, %SP{{[0-9]+}}
    s_i8_st_tnsr_s_rmw(index, other_input, 2.0, 1, 0, pred, 0);
    // CHECK-DAG: st_tnsr_s rmw_sel %ST_TNSR_ID_REG, %I{{[0-9]+}}, 0x2, %ST_RMW_REG, %SP{{[0-9]+}}
  }
  {
    s_u8_st_tnsr_s(index, input, int_value, 0, pred, 0);
    // CHECK-DAG: st_tnsr_s 0x0, %I{{[0-9]+}}, %S{{[0-9]+}}, %SP{{[0-9]+}}
    s_u8_st_tnsr_s(index, other_input, int_value, 0, pred, 0);
    // CHECK-DAG: st_tnsr_s %ST_TNSR_ID_REG, %I{{[0-9]+}}, %S{{[0-9]+}}, %SP{{[0-9]+}}
    s_u8_st_tnsr_s_rmw(index, input, int_value, 1, 0, pred, 0);
    // CHECK-DAG: st_tnsr_s rmw_sel 0x0, %I{{[0-9]+}}, %S{{[0-9]+}}, %ST_RMW_REG, %SP{{[0-9]+}}
    s_u8_st_tnsr_s_rmw(index, other_input, int_value, 1, 0, pred, 0);
    // CHECK-DAG: st_tnsr_s rmw_sel %ST_TNSR_ID_REG, %I{{[0-9]+}}, %S{{[0-9]+}}, %ST_RMW_REG, %SP{{[0-9]+}}
    s_u8_st_tnsr_s(index, input, 2, 0, pred, 0);
    // CHECK-DAG: st_tnsr_s 0x0, %I{{[0-9]+}}, 0x2, %SP{{[0-9]+}}
    s_u8_st_tnsr_s(index, other_input, 2, 0, pred, 0);
    // CHECK-DAG: st_tnsr_s %ST_TNSR_ID_REG, %I{{[0-9]+}}, 0x2, %SP{{[0-9]+}}
    s_u8_st_tnsr_s_rmw(index, input, 2.0, 1, 0, pred, 0);
    // CHECK-DAG: st_tnsr_s rmw_sel 0x0, %I{{[0-9]+}}, 0x2, %ST_RMW_REG, %SP{{[0-9]+}}
    s_u8_st_tnsr_s_rmw(index, other_input, 2.0, 1, 0, pred, 0);
    // CHECK-DAG: st_tnsr_s rmw_sel %ST_TNSR_ID_REG, %I{{[0-9]+}}, 0x2, %ST_RMW_REG, %SP{{[0-9]+}}
  }
}
