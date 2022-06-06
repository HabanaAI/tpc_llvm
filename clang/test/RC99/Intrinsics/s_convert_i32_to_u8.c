// RUN: %codegen -S -O1 -triple tpc-none-none -std=rc99 %s -o - | FileCheck %s
// RUN: %codegen -S -O1 -triple tpc-none-none -std=rc99 -target-cpu goya2 -bfloat16 %s -o - | FileCheck --check-prefixes=CHECK,GEN2P %s

void main(unsigned int dest, int x, _Bool pred) {
  volatile unsigned char __local *dest_ptr = (unsigned char __local *)dest;
  unsigned char income = *dest_ptr;

// CHECK: mov{{.*}} [[PRED:%SP[0-9]+]], %S2
// CHECK: ld_l  [[DEST:%S[0-9]+]], %S0

  // s_convert_i32_to_u8
  unsigned char res = income;
  
  res = s_convert_i32_to_u8(x, 0, res, pred, 0);
  *dest_ptr++ = res;
// CHECK: convert.i32 target_type=uint8 rhne [[DEST]], %S1, [[PRED]]
  
  res = s_convert_i32_to_u8(x, 0, res, pred, 0);
  *dest_ptr++ = res;
// CHECK: convert.i32 target_type=uint8 rhne [[DEST]], %S1, [[PRED]]
  
  res = s_convert_i32_to_u8(x, SW_RHNE, res, pred, 0);
  *dest_ptr++ = res;
// CHECK: convert.i32 target_type=uint8 rhne [[DEST]], %S1, [[PRED]]

}

