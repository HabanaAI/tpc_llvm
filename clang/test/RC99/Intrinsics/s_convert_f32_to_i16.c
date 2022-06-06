// RUN_UNCOMMIT_AFTER_GAUDI_1911: %codegen -S -O1 -triple tpc-none-none -std=rc99 %s -o - | FileCheck %s
// RUN_UNCOMMIT_AFTER_GAUDI_1911: %codegen -S -O1 -triple tpc-none-none -std=rc99 -target-cpu gaudi -bfloat16 %s -o - | FileCheck %s
// RUN_UNCOMMIT_AFTER_GAUDI_1911: %codegen -S -O1 -triple tpc-none-none -std=rc99 -target-cpu gaudib -bfloat16 %s -o - | FileCheck %s
// RUN: %codegen -S -O1 -triple tpc-none-none -std=rc99 -target-cpu goya2 -bfloat16 %s -o - | FileCheck %s
// RUN: %codegen -S -O1 -triple tpc-none-none -std=rc99 -target-cpu gaudi2 -bfloat16 %s -o - | FileCheck %s
// RUN: %codegen -S -O1 -triple tpc-none-none -std=rc99 -target-cpu doron1 -bfloat16 %s -o - | FileCheck %s

// XFAIL: *
// GAUDI-2392
            

void main(int dest, float x, _Bool pred) {
  volatile short __local *dest_ptr = (short __local *)dest;
  short income = *dest_ptr;

  // CHECK: mov{{.*}} [[PRED:%SP[0-9]+]], %S2
  // CHECK: ld_l  [[DEST:%S[0-9]+]], %S0

  // s_convert_i32_to_i16
  short res = income;
  
  res = s_convert_f32_to_i16(x, SW_RHNE, res, pred, 0);
  *dest_ptr++ = res;
  // CHECK: convert.f32 target_type=int16 rhne [[DEST]], %S1, [[PRED]]
  
  res = s_convert_f32_to_i16(x, SW_RZ, res, pred, 0);
  *dest_ptr++ = res;
  // CHECK: convert.f32 target_type=int16 rz [[DEST]], %S1, [[PRED]]
}

