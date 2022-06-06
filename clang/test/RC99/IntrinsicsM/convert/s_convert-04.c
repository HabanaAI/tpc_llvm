// RUN: %codegen -S -O1 -triple tpc-none-none -std=rc99 -bfloat16 -target-cpu gaudi2 %s -o - | FileCheck %s
// RUN: %codegen -S -O1 -triple tpc-none-none -std=rc99 -bfloat16 -target-cpu doron1 %s -o - | FileCheck %s

void main(int src, int dest, _Bool pred)
{
  void __local *dest_void_ptr = (void __local *)dest;
  void __local *src_void_ptr = (void __local *)src;

  float __local *src_float_ptr = (float __local *)src_void_ptr;
  bf16 __local *src_bf16_ptr = (bf16 __local *)src_void_ptr;
  half __local *src_half_ptr = (half __local *)src_void_ptr;
  int __local *src_int32_ptr = (int __local *)src_void_ptr;
  short __local *src_int16_ptr = (short __local *)src_void_ptr;
  unsigned __local *src_uint32_ptr = (unsigned __local *)src_void_ptr;
  unsigned short __local *src_uint16_ptr = (unsigned short __local *)src_void_ptr;
  signed char __local *src_int8_ptr = (signed char __local *)src_void_ptr;
  unsigned char __local *src_uint8_ptr = (unsigned char __local *)src_void_ptr;
  minifloat __local *src_minifloat_ptr = (minifloat __local *)src_void_ptr;
  minihalf __local *src_minihalf_ptr = (minihalf __local *)src_void_ptr;

  float src_float = *src_float_ptr;
  bf16 src_bf16 = *src_bf16_ptr;
  half src_half = *src_half_ptr;
  int src_int32 = *src_int32_ptr;
  short src_int16 = *src_int16_ptr;
  unsigned src_uint32 = *src_uint32_ptr;
  unsigned short src_uint16 = *src_uint16_ptr;
  signed char src_int8 = *src_int8_ptr;
  unsigned char src_uint8 = *src_uint8_ptr;
  minifloat src_minifloat = *src_minifloat_ptr;
  minihalf src_minihalf = *src_minihalf_ptr;

  {
    minifloat __local *dest_ptr = (minifloat __local *)dest_void_ptr;
    minifloat res = 0;
    res = s_convert_f32_to_f8(src_float, SW_RZ, res, pred, 0);
    *dest_ptr++ = res;
    // CHECK-DAG: convert.f32 target_type=f8_143 rz {{%S[0-9]+}}, {{%S[0-9]+}}, {{%SP[0-9]+}}
    dest_void_ptr = dest_ptr;
  }
  {
    minihalf __local *dest_ptr = (minihalf __local *)dest_void_ptr;
    minihalf res = 0;
    res = s_convert_f32_to_h8(src_float, SW_RZ, res, pred, 0);
    *dest_ptr++ = res;
    // CHECK-DAG: convert.f32 target_type=f8_152 rz {{%S[0-9]+}}, {{%S[0-9]+}}, {{%SP[0-9]+}}
    dest_void_ptr = dest_ptr;
  }
  {
    unsigned __local *dest_ptr = (unsigned __local *)dest_void_ptr;
    unsigned res = 0;
    res = s_convert_f32_to_u32(src_float, SW_RZ, res, pred, 0);
    *dest_ptr++ = res;
    // CHECK-DAG: convert.f32 target_type=uint32 rz {{%S[0-9]+}}, {{%S[0-9]+}}, {{%SP[0-9]+}}
    dest_void_ptr = dest_ptr;
  }
  {
    unsigned short __local *dest_ptr = (unsigned short __local *)dest_void_ptr;
    unsigned short res = 0;
    res = s_convert_f32_to_u16(src_float, SW_RZ, res, pred, 0);
    *dest_ptr++ = res;
    // CHECK-DAG: convert.f32 target_type=uint16 rz {{%S[0-9]+}}, {{%S[0-9]+}}, {{%SP[0-9]+}}
    dest_void_ptr = dest_ptr;
  }
  {
    unsigned char __local *dest_ptr = (unsigned char __local *)dest_void_ptr;
    unsigned char res = 0;
    res = s_convert_f32_to_u8(src_float, SW_RZ, res, pred, 0);
    *dest_ptr++ = res;
    // CHECK-DAG: convert.f32 target_type=uint8 rz {{%S[0-9]+}}, {{%S[0-9]+}}, {{%SP[0-9]+}}
    dest_void_ptr = dest_ptr;
  }

  {
    minifloat __local *dest_ptr = (minifloat __local *)dest_void_ptr;
    minifloat res = 0;
    res = s_convert_bf16_to_f8(src_bf16, SW_RZ, res, pred, 0);
    *dest_ptr++ = res;
    // CHECK-DAG: convert.bf16 target_type=f8_143 rz {{%S[0-9]+}}, {{%S[0-9]+}}, {{%SP[0-9]+}}
    dest_void_ptr = dest_ptr;
  }
  {
    minihalf __local *dest_ptr = (minihalf __local *)dest_void_ptr;
    minihalf res = 0;
    res = s_convert_bf16_to_h8(src_bf16, SW_RZ, res, pred, 0);
    *dest_ptr++ = res;
    // CHECK-DAG: convert.bf16 target_type=f8_152 rz {{%S[0-9]+}}, {{%S[0-9]+}}, {{%SP[0-9]+}}
    dest_void_ptr = dest_ptr;
  }
  {
    int __local *dest_ptr = (int __local *)dest_void_ptr;
    int res = 0;
    res = s_convert_bf16_to_i32(src_bf16, SW_RZ, res, pred, 0);
    *dest_ptr++ = res;
    // CHECK-DAG: convert.bf16 target_type=int32 rz {{%S[0-9]+}}, {{%S[0-9]+}}, {{%SP[0-9]+}}
    dest_void_ptr = dest_ptr;
  }
  {
    unsigned __local *dest_ptr = (unsigned __local *)dest_void_ptr;
    unsigned res = 0;
    res = s_convert_bf16_to_u32(src_bf16, SW_RZ, res, pred, 0);
    *dest_ptr++ = res;
    // CHECK-DAG: convert.bf16 target_type=uint32 rz {{%S[0-9]+}}, {{%S[0-9]+}}, {{%SP[0-9]+}}
    dest_void_ptr = dest_ptr;
  }
  {
    unsigned short __local *dest_ptr = (unsigned short __local *)dest_void_ptr;
    unsigned short res = 0;
    res = s_convert_bf16_to_u16(src_bf16, SW_RZ, res, pred, 0);
    *dest_ptr++ = res;
    // CHECK-DAG: convert.bf16 target_type=uint16 rz {{%S[0-9]+}}, {{%S[0-9]+}}, {{%SP[0-9]+}}
    dest_void_ptr = dest_ptr;
  }
  {
    unsigned char __local *dest_ptr = (unsigned char __local *)dest_void_ptr;
    unsigned char res = 0;
    res = s_convert_bf16_to_u8(src_bf16, SW_RZ, res, pred, 0);
    *dest_ptr++ = res;
    // CHECK-DAG: convert.bf16 target_type=uint8 rz {{%S[0-9]+}}, {{%S[0-9]+}}, {{%SP[0-9]+}}
    dest_void_ptr = dest_ptr;
  }

  {
    minifloat __local *dest_ptr = (minifloat __local *)dest_void_ptr;
    minifloat res = 0;
    res = s_convert_f16_to_f8(src_half, SW_RZ, res, pred, 0);
    *dest_ptr++ = res;
    // CHECK-DAG: convert.f16 target_type=f8_143 rz {{%S[0-9]+}}, {{%S[0-9]+}}, {{%SP[0-9]+}}
    dest_void_ptr = dest_ptr;
  }
  {
    minihalf __local *dest_ptr = (minihalf __local *)dest_void_ptr;
    minihalf res = 0;
    res = s_convert_f16_to_h8(src_half, SW_RZ, res, pred, 0);
    *dest_ptr++ = res;
    // CHECK-DAG: convert.f16 target_type=f8_152 rz {{%S[0-9]+}}, {{%S[0-9]+}}, {{%SP[0-9]+}}
    dest_void_ptr = dest_ptr;
  }
  {
    int __local *dest_ptr = (int __local *)dest_void_ptr;
    int res = 0;
    res = s_convert_f16_to_i32(src_half, SW_RZ, res, pred, 0);
    *dest_ptr++ = res;
    // CHECK-DAG: convert.f16 target_type=int32 rz {{%S[0-9]+}}, {{%S[0-9]+}}, {{%SP[0-9]+}}
    dest_void_ptr = dest_ptr;
  }
  {
    unsigned __local *dest_ptr = (unsigned __local *)dest_void_ptr;
    unsigned res = 0;
    res = s_convert_f16_to_u32(src_half, SW_RZ, res, pred, 0);
    *dest_ptr++ = res;
    // CHECK-DAG: convert.f16 target_type=uint32 rz {{%S[0-9]+}}, {{%S[0-9]+}}, {{%SP[0-9]+}}
    dest_void_ptr = dest_ptr;
  }
  {
    unsigned short __local *dest_ptr = (unsigned short __local *)dest_void_ptr;
    unsigned short res = 0;
    res = s_convert_f16_to_u16(src_half, SW_RZ, res, pred, 0);
    *dest_ptr++ = res;
    // CHECK-DAG: convert.f16 target_type=uint16 rz {{%S[0-9]+}}, {{%S[0-9]+}}, {{%SP[0-9]+}}
    dest_void_ptr = dest_ptr;
  }
  {
    unsigned char __local *dest_ptr = (unsigned char __local *)dest_void_ptr;
    unsigned char res = 0;
    res = s_convert_f16_to_u8(src_half, SW_RZ, res, pred, 0);
    *dest_ptr++ = res;
    // CHECK-DAG: convert.f16 target_type=uint8 rz {{%S[0-9]+}}, {{%S[0-9]+}}, {{%SP[0-9]+}}
    dest_void_ptr = dest_ptr;
  }

  {
    float __local *dest_ptr = (float __local *)dest_void_ptr;
    float res = 0;
    res = s_convert_f8_to_f32(src_minifloat, SW_RZ, res, pred, 0);
    *dest_ptr++ = res;
    // CHECK-DAG: convert.f8_143 target_type=fp32 rz {{%S[0-9]+}}, {{%S[0-9]+}}, {{%SP[0-9]+}}
    dest_void_ptr = dest_ptr;
  }
  {
    bf16 __local *dest_ptr = (bf16 __local *)dest_void_ptr;
    bf16 res = 0;
    res = s_convert_f8_to_bf16(src_minifloat, SW_RZ, res, pred, 0);
    *dest_ptr++ = res;
    // CHECK-DAG: convert.f8_143 target_type=bf16 rz {{%S[0-9]+}}, {{%S[0-9]+}}, {{%SP[0-9]+}}
    dest_void_ptr = dest_ptr;
  }
  {
    half __local *dest_ptr = (half __local *)dest_void_ptr;
    half res = 0;
    res = s_convert_f8_to_f16(src_minifloat, SW_RZ, res, pred, 0);
    *dest_ptr++ = res;
    // CHECK-DAG: convert.f8_143 target_type=f16 rz {{%S[0-9]+}}, {{%S[0-9]+}}, {{%SP[0-9]+}}
    dest_void_ptr = dest_ptr;
  }
  {
    minihalf __local *dest_ptr = (minihalf __local *)dest_void_ptr;
    minihalf res = 0;
    res = s_convert_f8_to_h8(src_minifloat, SW_RZ, res, pred, 0);
    *dest_ptr++ = res;
    // CHECK-DAG: convert.f8_143 target_type=f8_152 rz {{%S[0-9]+}}, {{%S[0-9]+}}, {{%SP[0-9]+}}
    dest_void_ptr = dest_ptr;
  }
  {
    int __local *dest_ptr = (int __local *)dest_void_ptr;
    int res = 0;
    res = s_convert_f8_to_i32(src_minifloat, SW_RZ, res, pred, 0);
    *dest_ptr++ = res;
    // CHECK-DAG: convert.f8_143 target_type=int32 rz {{%S[0-9]+}}, {{%S[0-9]+}}, {{%SP[0-9]+}}
    dest_void_ptr = dest_ptr;
  }
  {
    unsigned __local *dest_ptr = (unsigned __local *)dest_void_ptr;
    unsigned res = 0;
    res = s_convert_f8_to_u32(src_minifloat, SW_RZ, res, pred, 0);
    *dest_ptr++ = res;
    // CHECK-DAG: convert.f8_143 target_type=uint32 rz {{%S[0-9]+}}, {{%S[0-9]+}}, {{%SP[0-9]+}}
    dest_void_ptr = dest_ptr;
  }
  {
    short __local *dest_ptr = (short __local *)dest_void_ptr;
    short res = 0;
    res = s_convert_f8_to_i16(src_minifloat, SW_RZ, res, pred, 0);
    *dest_ptr++ = res;
    // CHECK-DAG: convert.f8_143 target_type=int16 rz {{%S[0-9]+}}, {{%S[0-9]+}}, {{%SP[0-9]+}}
    dest_void_ptr = dest_ptr;
  }
  {
    unsigned short __local *dest_ptr = (unsigned short __local *)dest_void_ptr;
    unsigned short res = 0;
    res = s_convert_f8_to_u16(src_minifloat, SW_RZ, res, pred, 0);
    *dest_ptr++ = res;
    // CHECK-DAG: convert.f8_143 target_type=uint16 rz {{%S[0-9]+}}, {{%S[0-9]+}}, {{%SP[0-9]+}}
    dest_void_ptr = dest_ptr;
  }
  {
    signed char __local *dest_ptr = (signed char __local *)dest_void_ptr;
    signed char res = 0;
    res = s_convert_f8_to_i8(src_minifloat, SW_RZ, res, pred, 0);
    *dest_ptr++ = res;
    // CHECK-DAG: convert.f8_143 target_type=int8 rz {{%S[0-9]+}}, {{%S[0-9]+}}, {{%SP[0-9]+}}
    dest_void_ptr = dest_ptr;
  }
  {
    unsigned char __local *dest_ptr = (unsigned char __local *)dest_void_ptr;
    unsigned char res = 0;
    res = s_convert_f8_to_u8(src_minifloat, SW_RZ, res, pred, 0);
    *dest_ptr++ = res;
    // CHECK-DAG: convert.f8_143 target_type=uint8 rz {{%S[0-9]+}}, {{%S[0-9]+}}, {{%SP[0-9]+}}
    dest_void_ptr = dest_ptr;
  }

  {
    float __local *dest_ptr = (float __local *)dest_void_ptr;
    float res = 0;
    res = s_convert_h8_to_f32(src_minihalf, SW_RZ, res, pred, 0);
    *dest_ptr++ = res;
    // CHECK-DAG: convert.f8_152 target_type=fp32 rz {{%S[0-9]+}}, {{%S[0-9]+}}, {{%SP[0-9]+}}
    dest_void_ptr = dest_ptr;
  }
  {
    bf16 __local *dest_ptr = (bf16 __local *)dest_void_ptr;
    bf16 res = 0;
    res = s_convert_h8_to_bf16(src_minihalf, SW_RZ, res, pred, 0);
    *dest_ptr++ = res;
    // CHECK-DAG: convert.f8_152 target_type=bf16 rz {{%S[0-9]+}}, {{%S[0-9]+}}, {{%SP[0-9]+}}
    dest_void_ptr = dest_ptr;
  }
  {
    half __local *dest_ptr = (half __local *)dest_void_ptr;
    half res = 0;
    res = s_convert_h8_to_f16(src_minihalf, SW_RZ, res, pred, 0);
    *dest_ptr++ = res;
    // CHECK-DAG: convert.f8_152 target_type=f16 rz {{%S[0-9]+}}, {{%S[0-9]+}}, {{%SP[0-9]+}}
    dest_void_ptr = dest_ptr;
  }
  {
    minifloat __local *dest_ptr = (minifloat __local *)dest_void_ptr;
    minifloat res = 0;
    res = s_convert_h8_to_f8(src_minihalf, SW_RZ, res, pred, 0);
    *dest_ptr++ = res;
    // CHECK-DAG: convert.f8_152 target_type=f8_143 rz {{%S[0-9]+}}, {{%S[0-9]+}}, {{%SP[0-9]+}}
    dest_void_ptr = dest_ptr;
  }
  {
    int __local *dest_ptr = (int __local *)dest_void_ptr;
    int res = 0;
    res = s_convert_h8_to_i32(src_minihalf, SW_RZ, res, pred, 0);
    *dest_ptr++ = res;
    // CHECK-DAG: convert.f8_152 target_type=int32 rz {{%S[0-9]+}}, {{%S[0-9]+}}, {{%SP[0-9]+}}
    dest_void_ptr = dest_ptr;
  }
  {
    unsigned __local *dest_ptr = (unsigned __local *)dest_void_ptr;
    unsigned res = 0;
    res = s_convert_h8_to_u32(src_minihalf, SW_RZ, res, pred, 0);
    *dest_ptr++ = res;
    // CHECK-DAG: convert.f8_152 target_type=uint32 rz {{%S[0-9]+}}, {{%S[0-9]+}}, {{%SP[0-9]+}}
    dest_void_ptr = dest_ptr;
  }
  {
    short __local *dest_ptr = (short __local *)dest_void_ptr;
    short res = 0;
    res = s_convert_h8_to_i16(src_minihalf, SW_RZ, res, pred, 0);
    *dest_ptr++ = res;
    // CHECK-DAG: convert.f8_152 target_type=int16 rz {{%S[0-9]+}}, {{%S[0-9]+}}, {{%SP[0-9]+}}
    dest_void_ptr = dest_ptr;
  }
  {
    unsigned short __local *dest_ptr = (unsigned short __local *)dest_void_ptr;
    unsigned short res = 0;
    res = s_convert_h8_to_u16(src_minihalf, SW_RZ, res, pred, 0);
    *dest_ptr++ = res;
    // CHECK-DAG: convert.f8_152 target_type=uint16 rz {{%S[0-9]+}}, {{%S[0-9]+}}, {{%SP[0-9]+}}
    dest_void_ptr = dest_ptr;
  }
  {
    signed char __local *dest_ptr = (signed char __local *)dest_void_ptr;
    signed char res = 0;
    res = s_convert_h8_to_i8(src_minihalf, SW_RZ, res, pred, 0);
    *dest_ptr++ = res;
    // CHECK-DAG: convert.f8_152 target_type=int8 rz {{%S[0-9]+}}, {{%S[0-9]+}}, {{%SP[0-9]+}}
    dest_void_ptr = dest_ptr;
  }
  {
    unsigned char __local *dest_ptr = (unsigned char __local *)dest_void_ptr;
    unsigned char res = 0;
    res = s_convert_h8_to_u8(src_minihalf, SW_RZ, res, pred, 0);
    *dest_ptr++ = res;
    // CHECK-DAG: convert.f8_152 target_type=uint8 rz {{%S[0-9]+}}, {{%S[0-9]+}}, {{%SP[0-9]+}}
    dest_void_ptr = dest_ptr;
  }

  {
    minifloat __local *dest_ptr = (minifloat __local *)dest_void_ptr;
    minifloat res = 0;
    res = s_convert_i32_to_f8(src_int32, SW_RZ, res, pred, 0);
    *dest_ptr++ = res;
    // CHECK-DAG: convert.i32 target_type=f8_143 rz {{%S[0-9]+}}, {{%S[0-9]+}}, {{%SP[0-9]+}}
    dest_void_ptr = dest_ptr;
  }
  {
    minihalf __local *dest_ptr = (minihalf __local *)dest_void_ptr;
    minihalf res = 0;
    res = s_convert_i32_to_h8(src_int32, SW_RZ, res, pred, 0);
    *dest_ptr++ = res;
    // CHECK-DAG: convert.i32 target_type=f8_152 rz {{%S[0-9]+}}, {{%S[0-9]+}}, {{%SP[0-9]+}}
    dest_void_ptr = dest_ptr;
  }
  {
    unsigned __local *dest_ptr = (unsigned __local *)dest_void_ptr;
    unsigned res = 0;
    res = s_convert_i32_to_u32(src_int32, SW_RZ, res, pred, 0);
    *dest_ptr++ = res;
    // CHECK-DAG: convert.i32 target_type=uint32 rz {{%S[0-9]+}}, {{%S[0-9]+}}, {{%SP[0-9]+}}
    dest_void_ptr = dest_ptr;
  }
  {
    short __local *dest_ptr = (short __local *)dest_void_ptr;
    short res = 0;
    res = s_convert_i32_to_i16(src_int32, SW_RZ, res, pred, 0);
    *dest_ptr++ = res;
    // CHECK-DAG: convert.i32 target_type=int16 rz {{%S[0-9]+}}, {{%S[0-9]+}}, {{%SP[0-9]+}}
    dest_void_ptr = dest_ptr;
  }
  {
    unsigned short __local *dest_ptr = (unsigned short __local *)dest_void_ptr;
    unsigned short res = 0;
    res = s_convert_i32_to_u16(src_int32, SW_RZ, res, pred, 0);
    *dest_ptr++ = res;
    // CHECK-DAG: convert.i32 target_type=uint16 rz {{%S[0-9]+}}, {{%S[0-9]+}}, {{%SP[0-9]+}}
    dest_void_ptr = dest_ptr;
  }
  {
    char __local *dest_ptr = (char __local *)dest_void_ptr;
    char res = 0;
    res = s_convert_i32_to_i8(src_int32, SW_RZ, res, pred, 0);
    *dest_ptr++ = res;
    // CHECK-DAG: convert.i32 target_type=int8 rz {{%S[0-9]+}}, {{%S[0-9]+}}, {{%SP[0-9]+}}
    dest_void_ptr = dest_ptr;
  }
  {
    unsigned char __local *dest_ptr = (unsigned char __local *)dest_void_ptr;
    unsigned char res = 0;
    res = s_convert_i32_to_u8(src_int32, SW_RZ, res, pred, 0);
    *dest_ptr++ = res;
    // CHECK-DAG: convert.i32 target_type=uint8 rz {{%S[0-9]+}}, {{%S[0-9]+}}, {{%SP[0-9]+}}
    dest_void_ptr = dest_ptr;
  }

  {
    float __local *dest_ptr = (float __local *)dest_void_ptr;
    float res = 0;
    res = s_convert_u32_to_f32(src_uint32, SW_RZ, res, pred, 0);
    *dest_ptr++ = res;
    // CHECK-DAG: convert.u32 target_type=fp32 rz {{%S[0-9]+}}, {{%S[0-9]+}}, {{%SP[0-9]+}}
    dest_void_ptr = dest_ptr;
  }
  {
    bf16 __local *dest_ptr = (bf16 __local *)dest_void_ptr;
    bf16 res = 0;
    res = s_convert_u32_to_bf16(src_uint32, SW_RZ, res, pred, 0);
    *dest_ptr++ = res;
    // CHECK-DAG: convert.u32 target_type=bf16 rz {{%S[0-9]+}}, {{%S[0-9]+}}, {{%SP[0-9]+}}
    dest_void_ptr = dest_ptr;
  }
  {
    half __local *dest_ptr = (half __local *)dest_void_ptr;
    half res = 0;
    res = s_convert_u32_to_f16(src_uint32, SW_RZ, res, pred, 0);
    *dest_ptr++ = res;
    // CHECK-DAG: convert.u32 target_type=f16 rz {{%S[0-9]+}}, {{%S[0-9]+}}, {{%SP[0-9]+}}
    dest_void_ptr = dest_ptr;
  }
  {
    minifloat __local *dest_ptr = (minifloat __local *)dest_void_ptr;
    minifloat res = 0;
    res = s_convert_u32_to_f8(src_uint32, SW_RZ, res, pred, 0);
    *dest_ptr++ = res;
    // CHECK-DAG: convert.u32 target_type=f8_143 rz {{%S[0-9]+}}, {{%S[0-9]+}}, {{%SP[0-9]+}}
    dest_void_ptr = dest_ptr;
  }
  {
    minihalf __local *dest_ptr = (minihalf __local *)dest_void_ptr;
    minihalf res = 0;
    res = s_convert_u32_to_h8(src_uint32, SW_RZ, res, pred, 0);
    *dest_ptr++ = res;
    // CHECK-DAG: convert.u32 target_type=f8_152 rz {{%S[0-9]+}}, {{%S[0-9]+}}, {{%SP[0-9]+}}
    dest_void_ptr = dest_ptr;
  }
  {
    int __local *dest_ptr = (int __local *)dest_void_ptr;
    int res = 0;
    res = s_convert_u32_to_i32(src_uint32, SW_RZ, res, pred, 0);
    *dest_ptr++ = res;
    // CHECK-DAG: convert.u32 target_type=int32 rz {{%S[0-9]+}}, {{%S[0-9]+}}, {{%SP[0-9]+}}
    dest_void_ptr = dest_ptr;
  }
  {
    short __local *dest_ptr = (short __local *)dest_void_ptr;
    short res = 0;
    res = s_convert_u32_to_i16(src_uint32, SW_RZ, res, pred, 0);
    *dest_ptr++ = res;
    // CHECK-DAG: convert.u32 target_type=int16 rz {{%S[0-9]+}}, {{%S[0-9]+}}, {{%SP[0-9]+}}
    dest_void_ptr = dest_ptr;
  }
  {
    unsigned short __local *dest_ptr = (unsigned short __local *)dest_void_ptr;
    unsigned short res = 0;
    res = s_convert_u32_to_u16(src_uint32, SW_RZ, res, pred, 0);
    *dest_ptr++ = res;
    // CHECK-DAG: convert.u32 target_type=uint16 rz {{%S[0-9]+}}, {{%S[0-9]+}}, {{%SP[0-9]+}}
    dest_void_ptr = dest_ptr;
  }
  {
    int __local *dest_ptr = (int __local *)dest_void_ptr;
    int res = 0;
    res = s_convert_u32_to_i8(src_uint32, SW_RZ, res, pred, 0);
    *dest_ptr++ = res;
    // CHECK-DAG: convert.u32 target_type=int8 rz {{%S[0-9]+}}, {{%S[0-9]+}}, {{%SP[0-9]+}}
    dest_void_ptr = dest_ptr;
  }
  {
    unsigned char __local *dest_ptr = (unsigned char __local *)dest_void_ptr;
    unsigned char res = 0;
    res = s_convert_u32_to_u8(src_uint32, SW_RZ, res, pred, 0);
    *dest_ptr++ = res;
    // CHECK-DAG: convert.u32 target_type=uint8 rz {{%S[0-9]+}}, {{%S[0-9]+}}, {{%SP[0-9]+}}
    dest_void_ptr = dest_ptr;
  }

  {
    minifloat __local *dest_ptr = (minifloat __local *)dest_void_ptr;
    minifloat res = 0;
    res = s_convert_i16_to_f8(src_int16, SW_RZ, res, pred, 0);
    *dest_ptr++ = res;
    // CHECK-DAG: convert.i16 target_type=f8_143 rz {{%S[0-9]+}}, {{%S[0-9]+}}, {{%SP[0-9]+}}
    dest_void_ptr = dest_ptr;
  }
  {
    minihalf __local *dest_ptr = (minihalf __local *)dest_void_ptr;
    minihalf res = 0;
    res = s_convert_i16_to_h8(src_int16, SW_RZ, res, pred, 0);
    *dest_ptr++ = res;
    // CHECK-DAG: convert.i16 target_type=f8_152 rz {{%S[0-9]+}}, {{%S[0-9]+}}, {{%SP[0-9]+}}
    dest_void_ptr = dest_ptr;
  }
  {
    char __local *dest_ptr = (char __local *)dest_void_ptr;
    char res = 0;
    res = s_convert_i16_to_i8(src_int16, SW_RZ, res, pred, 0);
    *dest_ptr++ = res;
    // CHECK-DAG: convert.i16 target_type=int8 rz {{%S[0-9]+}}, {{%S[0-9]+}}, {{%SP[0-9]+}}
    dest_void_ptr = dest_ptr;
  }

  {
    minifloat __local *dest_ptr = (minifloat __local *)dest_void_ptr;
    minifloat res = 0;
    res = s_convert_u16_to_f8(src_uint16, SW_RZ, res, pred, 0);
    *dest_ptr++ = res;
    // CHECK-DAG: convert.u16 target_type=f8_143 rz {{%S[0-9]+}}, {{%S[0-9]+}}, {{%SP[0-9]+}}
    dest_void_ptr = dest_ptr;
  }
  {
    minihalf __local *dest_ptr = (minihalf __local *)dest_void_ptr;
    minihalf res = 0;
    res = s_convert_u16_to_h8(src_uint16, SW_RZ, res, pred, 0);
    *dest_ptr++ = res;
    // CHECK-DAG: convert.u16 target_type=f8_152 rz {{%S[0-9]+}}, {{%S[0-9]+}}, {{%SP[0-9]+}}
    dest_void_ptr = dest_ptr;
  }
  {
    int __local *dest_ptr = (int __local *)dest_void_ptr;
    int res = 0;
    res = s_convert_u16_to_i32(src_uint16, SW_RZ, res, pred, 0);
    *dest_ptr++ = res;
    // CHECK-DAG: convert.u16 target_type=int32 rz {{%S[0-9]+}}, {{%S[0-9]+}}, {{%SP[0-9]+}}
    dest_void_ptr = dest_ptr;
  }
  {
    unsigned __local *dest_ptr = (unsigned __local *)dest_void_ptr;
    unsigned res = 0;
    res = s_convert_u16_to_u32(src_uint16, SW_RZ, res, pred, 0);
    *dest_ptr++ = res;
    // CHECK-DAG: convert.u16 target_type=uint32 rz {{%S[0-9]+}}, {{%S[0-9]+}}, {{%SP[0-9]+}}
    dest_void_ptr = dest_ptr;
  }
  {
    short __local *dest_ptr = (short __local *)dest_void_ptr;
    short res = 0;
    res = s_convert_u16_to_i16(src_uint16, SW_RZ, res, pred, 0);
    *dest_ptr++ = res;
    // CHECK-DAG: convert.u16 target_type=int16 rz {{%S[0-9]+}}, {{%S[0-9]+}}, {{%SP[0-9]+}}
    dest_void_ptr = dest_ptr;
  }
  {
    char __local *dest_ptr = (char __local *)dest_void_ptr;
    char res = 0;
    res = s_convert_u16_to_i8(src_uint16, SW_RZ, res, pred, 0);
    *dest_ptr++ = res;
    // CHECK-DAG: convert.u16 target_type=int8 rz {{%S[0-9]+}}, {{%S[0-9]+}}, {{%SP[0-9]+}}
    dest_void_ptr = dest_ptr;
  }
  {
    unsigned char __local *dest_ptr = (unsigned char __local *)dest_void_ptr;
    unsigned char res = 0;
    res = s_convert_u16_to_u8(src_uint16, SW_RZ, res, pred, 0);
    *dest_ptr++ = res;
    // CHECK-DAG: convert.u16 target_type=uint8 rz {{%S[0-9]+}}, {{%S[0-9]+}}, {{%SP[0-9]+}}
    dest_void_ptr = dest_ptr;
  }

  {
    float __local *dest_ptr = (float __local *)dest_void_ptr;
    float res = 0;
    res = s_convert_u16_to_f32(src_uint16, SW_RZ, res, pred, 0);
    *dest_ptr++ = res;
    // CHECK-DAG: convert.u16 target_type=fp32 rz {{%S[0-9]+}}, {{%S[0-9]+}}, {{%SP[0-9]+}}
    dest_void_ptr = dest_ptr;
  }

  {
    minifloat __local *dest_ptr = (minifloat __local *)dest_void_ptr;
    minifloat res = 0;
    res = s_convert_i8_to_f8(src_int16, SW_RZ, res, pred, 0);
    *dest_ptr++ = res;
    // CHECK-DAG: convert.i8 target_type=f8_143 rz {{%S[0-9]+}}, {{%S[0-9]+}}, {{%SP[0-9]+}}
    dest_void_ptr = dest_ptr;
  }
  {
    minihalf __local *dest_ptr = (minihalf __local *)dest_void_ptr;
    minihalf res = 0;
    res = s_convert_i8_to_h8(src_int16, SW_RZ, res, pred, 0);
    *dest_ptr++ = res;
    // CHECK-DAG: convert.i8 target_type=f8_152 rz {{%S[0-9]+}}, {{%S[0-9]+}}, {{%SP[0-9]+}}
    dest_void_ptr = dest_ptr;
  }

  {
    minifloat __local *dest_ptr = (minifloat __local *)dest_void_ptr;
    minifloat res = 0;
    res = s_convert_u8_to_f8(src_int16, SW_RZ, res, pred, 0);
    *dest_ptr++ = res;
    // CHECK-DAG: convert.u8 target_type=f8_143 rz {{%S[0-9]+}}, {{%S[0-9]+}}, {{%SP[0-9]+}}
    dest_void_ptr = dest_ptr;
  }
  {
    minihalf __local *dest_ptr = (minihalf __local *)dest_void_ptr;
    minihalf res = 0;
    res = s_convert_u8_to_h8(src_int16, SW_RZ, res, pred, 0);
    *dest_ptr++ = res;
    // CHECK-DAG: convert.u8 target_type=f8_152 rz {{%S[0-9]+}}, {{%S[0-9]+}}, {{%SP[0-9]+}}
    dest_void_ptr = dest_ptr;
  }
  {
    char __local *dest_ptr = (char __local *)dest_void_ptr;
    char res = 0;
    res = s_convert_u8_to_i8(src_uint8, SW_RZ, res, pred, 0);
    *dest_ptr++ = res;
    // CHECK-DAG: convert.u8 target_type=int8 rz {{%S[0-9]+}}, {{%S[0-9]+}}, {{%SP[0-9]+}}
    dest_void_ptr = dest_ptr;
  }
}
