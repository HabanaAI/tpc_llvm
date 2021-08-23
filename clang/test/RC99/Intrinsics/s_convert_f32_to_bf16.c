// RUN: %codegen -S -O1 -triple tpc-none-none -std=rc99 -target-cpu gaudi -bfloat16 %s -o - | FileCheck %s




void main(int dest, float x, _Bool pred) {
  volatile short __local *dest_ptr = (short __local *)dest;
  bf16 income = *dest_ptr;

  // CHECK: mov{{.*}} [[PRED:%SP[0-9]+]], %S2
  // CHECK: ld_l  [[DEST:%S[0-9]+]], %S0

  // s_convert_i32_to_i16
  bf16 res = income;
  
  res = s_convert_f32_to_bf16(x, SW_RHNE, res, pred, 0);
  *dest_ptr++ = res;
  // CHECK: convert.f32 target_type=bf16 rhne [[DEST]], %S1, [[PRED]]
  
  res = s_convert_f32_to_bf16(x, SW_SR, res, pred, 0);
  *dest_ptr++ = res;
  // CHECK: convert.f32 target_type=bf16 sr [[DEST]], %S1, [[PRED]]
  
  res = s_convert_f32_to_bf16(x, SW_RHAZ, res, pred, 0);
  *dest_ptr++ = res;
  // CHECK: convert.f32 target_type=bf16 rhaz [[DEST]], %S1, [[PRED]]
}

