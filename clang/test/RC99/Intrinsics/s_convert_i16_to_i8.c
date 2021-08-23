// RUN: %codegen -S -O1 -triple tpc-none-none -std=rc99 %s -o - | FileCheck %s
// RUN: %codegen -S -O1 -triple tpc-none-none -std=rc99 -target-cpu gaudi -bfloat16 %s -o - | FileCheck --check-prefixes=CHECK,GEN2P %s


void main(int dest, int x, int shift, _Bool pred) {
  volatile char __local *dest_ptr = (char __local *)dest;
  char income = *dest_ptr;

// CHECK: mov{{.*}} [[PRED:%SP[0-9]+]], %S3
// CHECK: ld_l  [[DEST:%S[0-9]+]], %S0

  // s_convert_i16_to_i8
  char res = income;
  
  res = s_convert_int16_to_i8(x, shift, 0, res, pred, 0);
  *dest_ptr++ = res;
// CHECK: convert_int16 rhne [[DEST]], %S1, %S2, [[PRED]]
  
  res = s_convert_int16_to_i8(x, 4, 0, res, pred, 0);
  *dest_ptr++ = res;
// CHECK: convert_int16 rhne [[DEST]], %S1, 0x4, [[PRED]]
  
  res = s_convert_int16_to_i8(x, 4, SW_RHNE, res, pred, 0);
  *dest_ptr++ = res;
// CHECK: convert_int16 rhne [[DEST]], %S1, 0x4, [[PRED]]
  
  res = s_convert_int16_to_i8(x, 4, SW_RD, res, pred, 0);
  *dest_ptr++ = res;
// CHECK: convert_int16 rd [[DEST]], %S1, 0x4, [[PRED]]
  
  res = s_convert_int16_to_i8(x, 4, SW_RU, res, pred, 0);
  *dest_ptr++ = res;
// CHECK: convert_int16 ru [[DEST]], %S1, 0x4, [[PRED]]
  
  res = s_convert_int16_to_i8(x, 4, SW_SR, res, pred, 0);
  *dest_ptr++ = res;
// CHECK: convert_int16 sr [[DEST]], %S1, 0x4, [[PRED]]

#if defined(__gaudi__)
  res = s_convert_int16_to_i8(x, 4, SW_RZ, res, pred, 0);
  *dest_ptr++ = res;
// GEN2P: convert_int16 rz [[DEST]], %S1, 0x4, [[PRED]]
#endif
}

