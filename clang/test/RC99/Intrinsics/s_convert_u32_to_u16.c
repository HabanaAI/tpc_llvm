// RUN: %codegen -S -O1 -triple tpc-none-none -std=rc99 %s -o - | FileCheck %s
// RUN: %codegen -S -O1 -triple tpc-none-none -std=rc99 -target-cpu gaudi -bfloat16 %s -o - | FileCheck --check-prefixes=CHECK,GEN2P %s


void main(int dest, int x, int shift, _Bool pred) {
  volatile unsigned short __local *dest_ptr = (unsigned short __local *)dest;
  unsigned short income = *dest_ptr;

// CHECK: mov{{.*}} [[PRED:%SP[0-9]+]], %S3
// CHECK: ld_l  [[DEST:%S[0-9]+]], %S0

  // s_convert_u32_to_u16
  unsigned short res = income;
  
  res = s_convert_uint32_to_u16(x, shift, 0, res, pred, 0);
  *dest_ptr++ = res;
// CHECK: convert_uint32 rhne to_16 [[DEST]], %S1, %S2, [[PRED]]
  
  res = s_convert_uint32_to_u16(x, 4, 0, res, pred, 0);
  *dest_ptr++ = res;
// CHECK: convert_uint32 rhne to_16 [[DEST]], %S1, 0x4, [[PRED]]
  
  res = s_convert_uint32_to_u16(x, 4, SW_RHNE, res, pred, 0);
  *dest_ptr++ = res;
// CHECK: convert_uint32 rhne to_16 [[DEST]], %S1, 0x4, [[PRED]]
  
  res = s_convert_uint32_to_u16(x, 4, SW_RD, res, pred, 0);
  *dest_ptr++ = res;
// CHECK: convert_uint32 rd to_16 [[DEST]], %S1, 0x4, [[PRED]]
  
  res = s_convert_uint32_to_u16(x, 4, SW_RU, res, pred, 0);
  *dest_ptr++ = res;
// CHECK: convert_uint32 ru to_16 [[DEST]], %S1, 0x4, [[PRED]]
  
  res = s_convert_uint32_to_u16(x, 4, SW_SR, res, pred, 0);
  *dest_ptr++ = res;
// CHECK: convert_uint32 sr to_16 [[DEST]], %S1, 0x4, [[PRED]]

#if defined(__gaudi__)
  res = s_convert_uint32_to_u16(x, 4, SW_RZ, res, pred, 0);
  *dest_ptr++ = res;
// GEN2P: convert_uint32 rz to_16 [[DEST]], %S1, 0x4, [[PRED]]
#endif
}

