// RUN: %codegen -S -O1 -triple tpc-none-none -std=rc99 -bfloat16 -target-cpu gaudi2 %s -o - | FileCheck %s
// RUN: %codegen -S -O1 -triple tpc-none-none -std=rc99 -bfloat16 -target-cpu doron1 %s -o - | FileCheck %s

void main(int src, int dest, _Bool pred, int vpred)
{
  int64 __local *dest_vector_ptr = (int64 __local *)dest;

  float64 __local *src_float64_ptr = (float64 __local *)dest_vector_ptr;
  bfloat128 __local *src_bfloat128_ptr = (bfloat128 __local *)dest_vector_ptr;
  half128 __local *src_half128_ptr = (half128 __local *)dest_vector_ptr;
  minifloat256 __local *src_minifloat256_ptr = (minifloat256 __local *)dest_vector_ptr;
  minihalf256 __local *src_minihalf256_ptr = (minihalf256 __local *)dest_vector_ptr;
  int64 __local *src_int64_ptr = (int64 __local *)dest_vector_ptr;
  short128 __local *src_short128_ptr = (short128 __local *)dest_vector_ptr;
  char256 __local *src_char256_ptr = (char256 __local *)dest_vector_ptr;
  uint64 __local *src_uint64_ptr = (uint64 __local *)dest_vector_ptr;
  ushort128 __local *src_ushort128_ptr = (ushort128 __local *)dest_vector_ptr;
  uchar256 __local *src_uchar256_ptr = (uchar256 __local *)dest_vector_ptr;
  bool256 __local *vpred256_ptr = (bool256 __local *)vpred;
  bool128 __local *vpred128_ptr = (bool128 __local *)vpred;
  bool64 __local *vpred64_ptr = (bool64 __local *)vpred;

  float64 src_float64 = *src_float64_ptr;
  bfloat128 src_bfloat128 = *src_bfloat128_ptr;
  half128 src_half128 = *src_half128_ptr;
  minifloat256 src_minifloat256 = *src_minifloat256_ptr;
  minihalf256 src_minihalf256 = *src_minihalf256_ptr;
  int64 src_int64 = *src_int64_ptr;
  short128 src_short128 = *src_short128_ptr;
  char256 src_char256 = *src_char256_ptr;
  uint64 src_uint64 = *src_uint64_ptr;
  ushort128 src_ushort128 = *src_ushort128_ptr;
  uchar256 src_uchar256 = *src_uchar256_ptr;
  bool256 vpred256 = *vpred256_ptr;
  bool128 vpred128 = *vpred128_ptr;
  bool64 vpred64 = *vpred64_ptr;

  //Same size
  {
    uint64 __local *dest_ptr = (uint64 __local *)dest_vector_ptr;
    uint64 res = {0};
    res = v_convert_f32_to_u32_b(src_float64, SW_RZ, res, pred, 0);
    *dest_ptr++ = res;
    // CHECK-DAG: convert.f32 target_type=uint32 rz {{%V[0-9]+}}, {{%V[0-9]+}}, {{%SP[0-9]+}}
    dest_vector_ptr = (int64 __local *)dest_ptr;
  }
  {
    uint64 __local *dest_ptr = (uint64 __local *)dest_vector_ptr;
    uint64 res = {0};
    res = v_convert_f32_to_u32_vb(src_float64, SW_RZ, res, vpred64, 0);
    *dest_ptr++ = res;
    // CHECK-DAG: convert.f32 target_type=uint32 rz {{%V[0-9]+}}, {{%V[0-9]+}}, {{%VP[0-9]+}}
    dest_vector_ptr = (int64 __local *)dest_ptr;
  }

  {
    ushort128 __local *dest_ptr = (ushort128 __local *)dest_vector_ptr;
    ushort128 res = {0};
    res = v_convert_bf16_to_u16_b(src_bfloat128, SW_RZ, res, pred, 0);
    *dest_ptr++ = res;
    // CHECK-DAG: convert.bf16 target_type=uint16 rz {{%V[0-9]+}}, {{%V[0-9]+}}, {{%SP[0-9]+}}
    dest_vector_ptr = (int64 __local *)dest_ptr;
  }
  {
    ushort128 __local *dest_ptr = (ushort128 __local *)dest_vector_ptr;
    ushort128 res = {0};
    res = v_convert_bf16_to_u16_vb(src_bfloat128, SW_RZ, res, vpred128, 0);
    *dest_ptr++ = res;
    // CHECK-DAG: convert.bf16 target_type=uint16 rz {{%V[0-9]+}}, {{%V[0-9]+}}, {{%VP[0-9]+}}
    dest_vector_ptr = (int64 __local *)dest_ptr;
  }

  {
    ushort128 __local *dest_ptr = (ushort128 __local *)dest_vector_ptr;
    ushort128 res = {0};
    res = v_convert_f16_to_u16_b(src_half128, SW_RZ, res, pred, 0);
    *dest_ptr++ = res;
    // CHECK-DAG: convert.f16 target_type=uint16 rz {{%V[0-9]+}}, {{%V[0-9]+}}, {{%SP[0-9]+}}
    dest_vector_ptr = (int64 __local *)dest_ptr;
  }
  {
    ushort128 __local *dest_ptr = (ushort128 __local *)dest_vector_ptr;
    ushort128 res = {0};
    res = v_convert_f16_to_u16_vb(src_half128, SW_RZ, res, vpred128, 0);
    *dest_ptr++ = res;
    // CHECK-DAG: convert.f16 target_type=uint16 rz {{%V[0-9]+}}, {{%V[0-9]+}}, {{%VP[0-9]+}}
    dest_vector_ptr = (int64 __local *)dest_ptr;
  }

  {
    char256 __local *dest_ptr = (char256 __local *)dest_vector_ptr;
    char256 res = {0};
    res = v_convert_f8_to_i8_b(src_minifloat256, SW_RZ, res, pred, 0);
    *dest_ptr++ = res;
    // CHECK-DAG: convert.f8_143 target_type=int8 rz {{%V[0-9]+}}, {{%V[0-9]+}}, {{%SP[0-9]+}}
    dest_vector_ptr = (int64 __local *)dest_ptr;
  }
  {
    char256 __local *dest_ptr = (char256 __local *)dest_vector_ptr;
    char256 res = {0};
    res = v_convert_f8_to_i8_vb(src_minifloat256, SW_RZ, res, vpred256, 0);
    *dest_ptr++ = res;
    // CHECK-DAG: convert.f8_143 target_type=int8 rz {{%V[0-9]+}}, {{%V[0-9]+}}, {{%VP[0-9]+}}
    dest_vector_ptr = (int64 __local *)dest_ptr;
  }
  {
    uchar256 __local *dest_ptr = (uchar256 __local *)dest_vector_ptr;
    uchar256 res = {0};
    res = v_convert_f8_to_u8_b(src_minifloat256, SW_RZ, res, pred, 0);
    *dest_ptr++ = res;
    // CHECK-DAG: convert.f8_143 target_type=uint8 rz {{%V[0-9]+}}, {{%V[0-9]+}}, {{%SP[0-9]+}}
    dest_vector_ptr = (int64 __local *)dest_ptr;
  }
  {
    uchar256 __local *dest_ptr = (uchar256 __local *)dest_vector_ptr;
    uchar256 res = {0};
    res = v_convert_f8_to_u8_vb(src_minifloat256, SW_RZ, res, vpred256, 0);
    *dest_ptr++ = res;
    // CHECK-DAG: convert.f8_143 target_type=uint8 rz {{%V[0-9]+}}, {{%V[0-9]+}}, {{%VP[0-9]+}}
    dest_vector_ptr = (int64 __local *)dest_ptr;
  }
  {
    minihalf256 __local *dest_ptr = (minihalf256 __local *)dest_vector_ptr;
    minihalf256 res = {0};
    res = v_convert_f8_to_h8_b(src_minifloat256, SW_RZ, res, pred, 0);
    *dest_ptr++ = res;
    // CHECK-DAG: convert.f8_143 target_type=f8_152 rz {{%V[0-9]+}}, {{%V[0-9]+}}, {{%SP[0-9]+}}
    dest_vector_ptr = (int64 __local *)dest_ptr;
  }
  {
    minihalf256 __local *dest_ptr = (minihalf256 __local *)dest_vector_ptr;
    minihalf256 res = {0};
    res = v_convert_f8_to_h8_vb(src_minifloat256, SW_RZ, res, vpred256, 0);
    *dest_ptr++ = res;
    // CHECK-DAG: convert.f8_143 target_type=f8_152 rz {{%V[0-9]+}}, {{%V[0-9]+}}, {{%VP[0-9]+}}
    dest_vector_ptr = (int64 __local *)dest_ptr;
  }

  {
    char256 __local *dest_ptr = (char256 __local *)dest_vector_ptr;
    char256 res = {0};
    res = v_convert_h8_to_i8_b(src_minihalf256, SW_RZ, res, pred, 0);
    *dest_ptr++ = res;
    // CHECK-DAG: convert.f8_152 target_type=int8 rz {{%V[0-9]+}}, {{%V[0-9]+}}, {{%SP[0-9]+}}
    dest_vector_ptr = (int64 __local *)dest_ptr;
  }
  {
    char256 __local *dest_ptr = (char256 __local *)dest_vector_ptr;
    char256 res = {0};
    res = v_convert_h8_to_i8_vb(src_minihalf256, SW_RZ, res, vpred256, 0);
    *dest_ptr++ = res;
    // CHECK-DAG: convert.f8_152 target_type=int8 rz {{%V[0-9]+}}, {{%V[0-9]+}}, {{%VP[0-9]+}}
    dest_vector_ptr = (int64 __local *)dest_ptr;
  }
  {
    uchar256 __local *dest_ptr = (uchar256 __local *)dest_vector_ptr;
    uchar256 res = {0};
    res = v_convert_h8_to_u8_b(src_minihalf256, SW_RZ, res, pred, 0);
    *dest_ptr++ = res;
    // CHECK-DAG: convert.f8_152 target_type=uint8 rz {{%V[0-9]+}}, {{%V[0-9]+}}, {{%SP[0-9]+}}
    dest_vector_ptr = (int64 __local *)dest_ptr;
  }
  {
    uchar256 __local *dest_ptr = (uchar256 __local *)dest_vector_ptr;
    uchar256 res = {0};
    res = v_convert_h8_to_u8_vb(src_minihalf256, SW_RZ, res, vpred256, 0);
    *dest_ptr++ = res;
    // CHECK-DAG: convert.f8_152 target_type=uint8 rz {{%V[0-9]+}}, {{%V[0-9]+}}, {{%VP[0-9]+}}
    dest_vector_ptr = (int64 __local *)dest_ptr;
  }
  {
    minifloat256 __local *dest_ptr = (minifloat256 __local *)dest_vector_ptr;
    minifloat256 res = {0};
    res = v_convert_h8_to_f8_b(src_minihalf256, SW_RZ, res, pred, 0);
    *dest_ptr++ = res;
    // CHECK-DAG: convert.f8_152 target_type=f8_143 rz {{%V[0-9]+}}, {{%V[0-9]+}}, {{%SP[0-9]+}}
    dest_vector_ptr = (int64 __local *)dest_ptr;
  }
  {
    minifloat256 __local *dest_ptr = (minifloat256 __local *)dest_vector_ptr;
    minifloat256 res = {0};
    res = v_convert_h8_to_f8_vb(src_minihalf256, SW_RZ, res, vpred256, 0);
    *dest_ptr++ = res;
    // CHECK-DAG: convert.f8_152 target_type=f8_143 rz {{%V[0-9]+}}, {{%V[0-9]+}}, {{%VP[0-9]+}}
    dest_vector_ptr = (int64 __local *)dest_ptr;
  }

  {
    float64 __local *dest_ptr = (float64 __local *)dest_vector_ptr;
    float64 res = {0};
    res = v_convert_u32_to_f32_b(src_uint64, SW_RZ, res, pred, 0);
    *dest_ptr++ = res;
    // CHECK-DAG: convert.u32 target_type=fp32 rz {{%V[0-9]+}}, {{%V[0-9]+}}, {{%SP[0-9]+}}
    dest_vector_ptr = (int64 __local *)dest_ptr;
  }
  {
    float64 __local *dest_ptr = (float64 __local *)dest_vector_ptr;
    float64 res = {0};
    res = v_convert_u32_to_f32_vb(src_uint64, SW_RZ, res, vpred64, 0);
    *dest_ptr++ = res;
    // CHECK-DAG: convert.u32 target_type=fp32 rz {{%V[0-9]+}}, {{%V[0-9]+}}, {{%VP[0-9]+}}
    dest_vector_ptr = (int64 __local *)dest_ptr;
  }
  {
    int64 __local *dest_ptr = (int64 __local *)dest_vector_ptr;
    int64 res = {0};
    res = v_convert_u32_to_i32_b(src_uint64, SW_RZ, res, pred, 0);
    *dest_ptr++ = res;
    // CHECK-DAG: convert.u32 target_type=int32 rz {{%V[0-9]+}}, {{%V[0-9]+}}, {{%SP[0-9]+}}
    dest_vector_ptr = (int64 __local *)dest_ptr;
  }
  {
    int64 __local *dest_ptr = (int64 __local *)dest_vector_ptr;
    int64 res = {0};
    res = v_convert_u32_to_i32_vb(src_uint64, SW_RZ, res, vpred64, 0);
    *dest_ptr++ = res;
    // CHECK-DAG: convert.u32 target_type=int32 rz {{%V[0-9]+}}, {{%V[0-9]+}}, {{%VP[0-9]+}}
    dest_vector_ptr = (int64 __local *)dest_ptr;
  }

  {
    short128 __local *dest_ptr = (short128 __local *)dest_vector_ptr;
    short128 res = {0};
    res = v_convert_u16_to_i16_b(src_ushort128, SW_RZ, res, pred, 0);
    *dest_ptr++ = res;
    // CHECK-DAG: convert.u16 target_type=int16 rz {{%V[0-9]+}}, {{%V[0-9]+}}, {{%SP[0-9]+}}
    dest_vector_ptr = (int64 __local *)dest_ptr;
  }
  {
    short128 __local *dest_ptr = (short128 __local *)dest_vector_ptr;
    short128 res = {0};
    res = v_convert_u16_to_i16_vb(src_ushort128, SW_RZ, res, vpred128, 0);
    *dest_ptr++ = res;
    // CHECK-DAG: convert.u16 target_type=int16 rz {{%V[0-9]+}}, {{%V[0-9]+}}, {{%VP[0-9]+}}
    dest_vector_ptr = (int64 __local *)dest_ptr;
  }

  {
    minifloat256 __local *dest_ptr = (minifloat256 __local *)dest_vector_ptr;
    minifloat256 res = {0};
    res = v_convert_i8_to_f8_b(src_char256, SW_RZ, res, pred, 0);
    *dest_ptr++ = res;
    // CHECK-DAG: convert.i8 target_type=f8_143 rz {{%V[0-9]+}}, {{%V[0-9]+}}, {{%SP[0-9]+}}
    dest_vector_ptr = (int64 __local *)dest_ptr;
  }
  {
    minifloat256 __local *dest_ptr = (minifloat256 __local *)dest_vector_ptr;
    minifloat256 res = {0};
    res = v_convert_i8_to_f8_vb(src_char256, SW_RZ, res, vpred256, 0);
    *dest_ptr++ = res;
    // CHECK-DAG: convert.i8 target_type=f8_143 rz {{%V[0-9]+}}, {{%V[0-9]+}}, {{%VP[0-9]+}}
    dest_vector_ptr = (int64 __local *)dest_ptr;
  }
  {
    minihalf256 __local *dest_ptr = (minihalf256 __local *)dest_vector_ptr;
    minihalf256 res = {0};
    res = v_convert_i8_to_h8_b(src_char256, SW_RZ, res, pred, 0);
    *dest_ptr++ = res;
    // CHECK-DAG: convert.i8 target_type=f8_152 rz {{%V[0-9]+}}, {{%V[0-9]+}}, {{%SP[0-9]+}}
    dest_vector_ptr = (int64 __local *)dest_ptr;
  }
  {
    minihalf256 __local *dest_ptr = (minihalf256 __local *)dest_vector_ptr;
    minihalf256 res = {0};
    res = v_convert_i8_to_h8_vb(src_char256, SW_RZ, res, vpred256, 0);
    *dest_ptr++ = res;
    // CHECK-DAG: convert.i8 target_type=f8_152 rz {{%V[0-9]+}}, {{%V[0-9]+}}, {{%VP[0-9]+}}
    dest_vector_ptr = (int64 __local *)dest_ptr;
  }

  {
    char256 __local *dest_ptr = (char256 __local *)dest_vector_ptr;
    char256 res = {0};
    res = v_convert_u8_to_i8_b(src_uchar256, SW_RZ, res, pred, 0);
    *dest_ptr++ = res;
    // CHECK-DAG: convert.u8 target_type=int8 rz {{%V[0-9]+}}, {{%V[0-9]+}}, {{%SP[0-9]+}}
    dest_vector_ptr = (int64 __local *)dest_ptr;
  }
  {
    char256 __local *dest_ptr = (char256 __local *)dest_vector_ptr;
    char256 res = {0};
    res = v_convert_u8_to_i8_vb(src_uchar256, SW_RZ, res, vpred256, 0);
    *dest_ptr++ = res;
    // CHECK-DAG: convert.u8 target_type=int8 rz {{%V[0-9]+}}, {{%V[0-9]+}}, {{%VP[0-9]+}}
    dest_vector_ptr = (int64 __local *)dest_ptr;
  }
  {
    minifloat256 __local *dest_ptr = (minifloat256 __local *)dest_vector_ptr;
    minifloat256 res = {0};
    res = v_convert_u8_to_f8_b(src_uchar256, SW_RZ, res, pred, 0);
    *dest_ptr++ = res;
    // CHECK-DAG: convert.u8 target_type=f8_143 rz {{%V[0-9]+}}, {{%V[0-9]+}}, {{%SP[0-9]+}}
    dest_vector_ptr = (int64 __local *)dest_ptr;
  }
  {
    minifloat256 __local *dest_ptr = (minifloat256 __local *)dest_vector_ptr;
    minifloat256 res = {0};
    res = v_convert_u8_to_f8_vb(src_uchar256, SW_RZ, res, vpred256, 0);
    *dest_ptr++ = res;
    // CHECK-DAG: convert.u8 target_type=f8_143 rz {{%V[0-9]+}}, {{%V[0-9]+}}, {{%VP[0-9]+}}
    dest_vector_ptr = (int64 __local *)dest_ptr;
  }
  {
    minihalf256 __local *dest_ptr = (minihalf256 __local *)dest_vector_ptr;
    minihalf256 res = {0};
    res = v_convert_u8_to_h8_b(src_uchar256, SW_RZ, res, pred, 0);
    *dest_ptr++ = res;
    // CHECK-DAG: convert.u8 target_type=f8_152 rz {{%V[0-9]+}}, {{%V[0-9]+}}, {{%SP[0-9]+}}
    dest_vector_ptr = (int64 __local *)dest_ptr;
  }
  {
    minihalf256 __local *dest_ptr = (minihalf256 __local *)dest_vector_ptr;
    minihalf256 res = {0};
    res = v_convert_u8_to_h8_vb(src_uchar256, SW_RZ, res, vpred256, 0);
    *dest_ptr++ = res;
    // CHECK-DAG: convert.u8 target_type=f8_152 rz {{%V[0-9]+}}, {{%V[0-9]+}}, {{%VP[0-9]+}}
    dest_vector_ptr = (int64 __local *)dest_ptr;
  }

  //Up-convert
  {
    int64 __local *dest_ptr = (int64 __local *)dest_vector_ptr;
    int64 res = {0};
    res =  v_convert_bf16_to_i32_b(src_bfloat128, SW_RZ, res, pred, 0);
    *dest_ptr++ = res;
    // CHECK-DAG: convert.bf16 target_type=int32 rz {{%V[0-9]+}}, {{%V[0-9]+}}, {{%SP[0-9]+}}
    dest_vector_ptr = (int64 __local *)dest_ptr;
  }
  {
    int64 __local *dest_ptr = (int64 __local *)dest_vector_ptr;
    int64 res = {0};
    res =  v_convert_bf16_to_i32_vb(src_bfloat128, SW_RZ, res, vpred64, 0);
    *dest_ptr++ = res;
    // CHECK-DAG: convert.bf16 target_type=int32 rz {{%V[0-9]+}}, {{%V[0-9]+}}, {{%VP[0-9]+}}
    dest_vector_ptr = (int64 __local *)dest_ptr;
  }
  {
    uint64 __local *dest_ptr = (uint64 __local *)dest_vector_ptr;
    uint64 res = {0};
    res =  v_convert_bf16_to_u32_b(src_bfloat128, SW_RZ, res, pred, 0);
    *dest_ptr++ = res;
    // CHECK-DAG: convert.bf16 target_type=uint32 rz {{%V[0-9]+}}, {{%V[0-9]+}}, {{%SP[0-9]+}}
    dest_vector_ptr = (int64 __local *)dest_ptr;
  }
  {
    uint64 __local *dest_ptr = (uint64 __local *)dest_vector_ptr;
    uint64 res = {0};
    res =  v_convert_bf16_to_u32_vb(src_bfloat128, SW_RZ, res, vpred64, 0);
    *dest_ptr++ = res;
    // CHECK-DAG: convert.bf16 target_type=uint32 rz {{%V[0-9]+}}, {{%V[0-9]+}}, {{%VP[0-9]+}}
    dest_vector_ptr = (int64 __local *)dest_ptr;
  }

  {
    int64 __local *dest_ptr = (int64 __local *)dest_vector_ptr;
    int64 res = {0};
    res =  v_convert_f16_to_i32_b(src_half128, SW_RZ, res, pred, 0);
    *dest_ptr++ = res;
    // CHECK-DAG: convert.f16 target_type=int32 rz {{%V[0-9]+}}, {{%V[0-9]+}}, {{%SP[0-9]+}}
    dest_vector_ptr = (int64 __local *)dest_ptr;
  }
  {
    int64 __local *dest_ptr = (int64 __local *)dest_vector_ptr;
    int64 res = {0};
    res =  v_convert_f16_to_i32_vb(src_half128, SW_RZ, res, vpred64, 0);
    *dest_ptr++ = res;
    // CHECK-DAG: convert.f16 target_type=int32 rz {{%V[0-9]+}}, {{%V[0-9]+}}, {{%VP[0-9]+}}
    dest_vector_ptr = (int64 __local *)dest_ptr;
  }
  {
    uint64 __local *dest_ptr = (uint64 __local *)dest_vector_ptr;
    uint64 res = {0};
    res =  v_convert_f16_to_u32_b(src_half128, SW_RZ, res, pred, 0);
    *dest_ptr++ = res;
    // CHECK-DAG: convert.f16 target_type=uint32 rz {{%V[0-9]+}}, {{%V[0-9]+}}, {{%SP[0-9]+}}
    dest_vector_ptr = (int64 __local *)dest_ptr;
  }
  {
    uint64 __local *dest_ptr = (uint64 __local *)dest_vector_ptr;
    uint64 res = {0};
    res =  v_convert_f16_to_u32_vb(src_half128, SW_RZ, res, vpred64, 0);
    *dest_ptr++ = res;
    // CHECK-DAG: convert.f16 target_type=uint32 rz {{%V[0-9]+}}, {{%V[0-9]+}}, {{%VP[0-9]+}}
    dest_vector_ptr = (int64 __local *)dest_ptr;
  }

  {
    float64 __local *dest_ptr = (float64 __local *)dest_vector_ptr;
    float64 res = {0};
    res =  v_convert_f8_to_f32_b(src_minifloat256, SW_RZ, res, pred, 0);
    *dest_ptr++ = res;
    // CHECK-DAG: convert.f8_143 target_type=fp32 rz {{%V[0-9]+}}, {{%V[0-9]+}}, {{%SP[0-9]+}}
    dest_vector_ptr = (int64 __local *)dest_ptr;
  }
  {
    float64 __local *dest_ptr = (float64 __local *)dest_vector_ptr;
    float64 res = {0};
    res =  v_convert_f8_to_f32_vb(src_minifloat256, SW_RZ, res, vpred64, 0);
    *dest_ptr++ = res;
    // CHECK-DAG: convert.f8_143 target_type=fp32 rz {{%V[0-9]+}}, {{%V[0-9]+}}, {{%VP[0-9]+}}
    dest_vector_ptr = (int64 __local *)dest_ptr;
  }
  {
    bfloat128 __local *dest_ptr = (bfloat128 __local *)dest_vector_ptr;
    bfloat128 res = {0};
    res =  v_convert_f8_to_bf16_b(src_minifloat256, SW_RZ, res, pred, 0);
    *dest_ptr++ = res;
    // CHECK-DAG: convert.f8_143 target_type=bf16 rz {{%V[0-9]+}}, {{%V[0-9]+}}, {{%SP[0-9]+}}
    dest_vector_ptr = (int64 __local *)dest_ptr;
  }
  {
    bfloat128 __local *dest_ptr = (bfloat128 __local *)dest_vector_ptr;
    bfloat128 res = {0};
    res =  v_convert_f8_to_bf16_vb(src_minifloat256, SW_RZ, res, vpred128, 0);
    *dest_ptr++ = res;
    // CHECK-DAG: convert.f8_143 target_type=bf16 rz {{%V[0-9]+}}, {{%V[0-9]+}}, {{%VP[0-9]+}}
    dest_vector_ptr = (int64 __local *)dest_ptr;
  }
  {
    half128 __local *dest_ptr = (half128 __local *)dest_vector_ptr;
    half128 res = {0};
    res =  v_convert_f8_to_f16_b(src_minifloat256, SW_RZ, res, pred, 0);
    *dest_ptr++ = res;
    // CHECK-DAG: convert.f8_143 target_type=f16 rz {{%V[0-9]+}}, {{%V[0-9]+}}, {{%SP[0-9]+}}
    dest_vector_ptr = (int64 __local *)dest_ptr;
  }
  {
    half128 __local *dest_ptr = (half128 __local *)dest_vector_ptr;
    half128 res = {0};
    res =  v_convert_f8_to_f16_vb(src_minifloat256, SW_RZ, res, vpred128, 0);
    *dest_ptr++ = res;
    // CHECK-DAG: convert.f8_143 target_type=f16 rz {{%V[0-9]+}}, {{%V[0-9]+}}, {{%VP[0-9]+}}
    dest_vector_ptr = (int64 __local *)dest_ptr;
  }
  {
    int64 __local *dest_ptr = (int64 __local *)dest_vector_ptr;
    int64 res = {0};
    res =  v_convert_f8_to_i32_b(src_minifloat256, SW_RZ, res, pred, 0);
    *dest_ptr++ = res;
    // CHECK-DAG: convert.f8_143 target_type=int32 rz {{%V[0-9]+}}, {{%V[0-9]+}}, {{%SP[0-9]+}}
    dest_vector_ptr = (int64 __local *)dest_ptr;
  }
  {
    int64 __local *dest_ptr = (int64 __local *)dest_vector_ptr;
    int64 res = {0};
    res =  v_convert_f8_to_i32_vb(src_minifloat256, SW_RZ, res, vpred64, 0);
    *dest_ptr++ = res;
    // CHECK-DAG: convert.f8_143 target_type=int32 rz {{%V[0-9]+}}, {{%V[0-9]+}}, {{%VP[0-9]+}}
    dest_vector_ptr = (int64 __local *)dest_ptr;
  }
  {
    uint64 __local *dest_ptr = (uint64 __local *)dest_vector_ptr;
    uint64 res = {0};
    res =  v_convert_f8_to_u32_b(src_minifloat256, SW_RZ, res, pred, 0);
    *dest_ptr++ = res;
    // CHECK-DAG: convert.f8_143 target_type=uint32 rz {{%V[0-9]+}}, {{%V[0-9]+}}, {{%SP[0-9]+}}
    dest_vector_ptr = (int64 __local *)dest_ptr;
  }
  {
    uint64 __local *dest_ptr = (uint64 __local *)dest_vector_ptr;
    uint64 res = {0};
    res =  v_convert_f8_to_u32_vb(src_minifloat256, SW_RZ, res, vpred64, 0);
    *dest_ptr++ = res;
    // CHECK-DAG: convert.f8_143 target_type=uint32 rz {{%V[0-9]+}}, {{%V[0-9]+}}, {{%VP[0-9]+}}
    dest_vector_ptr = (int64 __local *)dest_ptr;
  }
  {
    short128 __local *dest_ptr = (short128 __local *)dest_vector_ptr;
    short128 res = {0};
    res =  v_convert_f8_to_i16_b(src_minifloat256, SW_RZ, res, pred, 0);
    *dest_ptr++ = res;
    // CHECK-DAG: convert.f8_143 target_type=int16 rz {{%V[0-9]+}}, {{%V[0-9]+}}, {{%SP[0-9]+}}
    dest_vector_ptr = (int64 __local *)dest_ptr;
  }
  {
    short128 __local *dest_ptr = (short128 __local *)dest_vector_ptr;
    short128 res = {0};
    res =  v_convert_f8_to_i16_vb(src_minifloat256, SW_RZ, res, vpred128, 0);
    *dest_ptr++ = res;
    // CHECK-DAG: convert.f8_143 target_type=int16 rz {{%V[0-9]+}}, {{%V[0-9]+}}, {{%VP[0-9]+}}
    dest_vector_ptr = (int64 __local *)dest_ptr;
  }
  {
    ushort128 __local *dest_ptr = (ushort128 __local *)dest_vector_ptr;
    ushort128 res = {0};
    res =  v_convert_f8_to_u16_b(src_minifloat256, SW_RZ, res, pred, 0);
    *dest_ptr++ = res;
    // CHECK-DAG: convert.f8_143 target_type=uint16 rz {{%V[0-9]+}}, {{%V[0-9]+}}, {{%SP[0-9]+}}
    dest_vector_ptr = (int64 __local *)dest_ptr;
  }
  {
    ushort128 __local *dest_ptr = (ushort128 __local *)dest_vector_ptr;
    ushort128 res = {0};
    res =  v_convert_f8_to_u16_vb(src_minifloat256, SW_RZ, res, vpred128, 0);
    *dest_ptr++ = res;
    // CHECK-DAG: convert.f8_143 target_type=uint16 rz {{%V[0-9]+}}, {{%V[0-9]+}}, {{%VP[0-9]+}}
    dest_vector_ptr = (int64 __local *)dest_ptr;
  }

  {
    float64 __local *dest_ptr = (float64 __local *)dest_vector_ptr;
    float64 res = {0};
    res =  v_convert_h8_to_f32_b(src_minihalf256, SW_RZ, res, pred, 0);
    *dest_ptr++ = res;
    // CHECK-DAG: convert.f8_152 target_type=fp32 rz {{%V[0-9]+}}, {{%V[0-9]+}}, {{%SP[0-9]+}}
    dest_vector_ptr = (int64 __local *)dest_ptr;
  }
  {
    float64 __local *dest_ptr = (float64 __local *)dest_vector_ptr;
    float64 res = {0};
    res =  v_convert_h8_to_f32_vb(src_minihalf256, SW_RZ, res, vpred64, 0);
    *dest_ptr++ = res;
    // CHECK-DAG: convert.f8_152 target_type=fp32 rz {{%V[0-9]+}}, {{%V[0-9]+}}, {{%VP[0-9]+}}
    dest_vector_ptr = (int64 __local *)dest_ptr;
  }
  {
    bfloat128 __local *dest_ptr = (bfloat128 __local *)dest_vector_ptr;
    bfloat128 res = {0};
    res =  v_convert_h8_to_bf16_b(src_minihalf256, SW_RZ, res, pred, 0);
    *dest_ptr++ = res;
    // CHECK-DAG: convert.f8_152 target_type=bf16 rz {{%V[0-9]+}}, {{%V[0-9]+}}, {{%SP[0-9]+}}
    dest_vector_ptr = (int64 __local *)dest_ptr;
  }
  {
    bfloat128 __local *dest_ptr = (bfloat128 __local *)dest_vector_ptr;
    bfloat128 res = {0};
    res =  v_convert_h8_to_bf16_vb(src_minihalf256, SW_RZ, res, vpred128, 0);
    *dest_ptr++ = res;
    // CHECK-DAG: convert.f8_152 target_type=bf16 rz {{%V[0-9]+}}, {{%V[0-9]+}}, {{%VP[0-9]+}}
    dest_vector_ptr = (int64 __local *)dest_ptr;
  }
  {
    half128 __local *dest_ptr = (half128 __local *)dest_vector_ptr;
    half128 res = {0};
    res =  v_convert_h8_to_f16_b(src_minihalf256, SW_RZ, res, pred, 0);
    *dest_ptr++ = res;
    // CHECK-DAG: convert.f8_152 target_type=f16 rz {{%V[0-9]+}}, {{%V[0-9]+}}, {{%SP[0-9]+}}
    dest_vector_ptr = (int64 __local *)dest_ptr;
  }
  {
    half128 __local *dest_ptr = (half128 __local *)dest_vector_ptr;
    half128 res = {0};
    res =  v_convert_h8_to_f16_vb(src_minihalf256, SW_RZ, res, vpred128, 0);
    *dest_ptr++ = res;
    // CHECK-DAG: convert.f8_152 target_type=f16 rz {{%V[0-9]+}}, {{%V[0-9]+}}, {{%VP[0-9]+}}
    dest_vector_ptr = (int64 __local *)dest_ptr;
  }
  {
    int64 __local *dest_ptr = (int64 __local *)dest_vector_ptr;
    int64 res = {0};
    res =  v_convert_h8_to_i32_b(src_minihalf256, SW_RZ, res, pred, 0);
    *dest_ptr++ = res;
    // CHECK-DAG: convert.f8_152 target_type=int32 rz {{%V[0-9]+}}, {{%V[0-9]+}}, {{%SP[0-9]+}}
    dest_vector_ptr = (int64 __local *)dest_ptr;
  }
  {
    int64 __local *dest_ptr = (int64 __local *)dest_vector_ptr;
    int64 res = {0};
    res =  v_convert_h8_to_i32_vb(src_minihalf256, SW_RZ, res, vpred64, 0);
    *dest_ptr++ = res;
    // CHECK-DAG: convert.f8_152 target_type=int32 rz {{%V[0-9]+}}, {{%V[0-9]+}}, {{%VP[0-9]+}}
    dest_vector_ptr = (int64 __local *)dest_ptr;
  }
  {
    uint64 __local *dest_ptr = (uint64 __local *)dest_vector_ptr;
    uint64 res = {0};
    res =  v_convert_h8_to_u32_b(src_minihalf256, SW_RZ, res, pred, 0);
    *dest_ptr++ = res;
    // CHECK-DAG: convert.f8_152 target_type=uint32 rz {{%V[0-9]+}}, {{%V[0-9]+}}, {{%SP[0-9]+}}
    dest_vector_ptr = (int64 __local *)dest_ptr;
  }
  {
    uint64 __local *dest_ptr = (uint64 __local *)dest_vector_ptr;
    uint64 res = {0};
    res =  v_convert_h8_to_u32_vb(src_minihalf256, SW_RZ, res, vpred64, 0);
    *dest_ptr++ = res;
    // CHECK-DAG: convert.f8_152 target_type=uint32 rz {{%V[0-9]+}}, {{%V[0-9]+}}, {{%VP[0-9]+}}
    dest_vector_ptr = (int64 __local *)dest_ptr;
  }
  {
    short128 __local *dest_ptr = (short128 __local *)dest_vector_ptr;
    short128 res = {0};
    res =  v_convert_h8_to_i16_b(src_minihalf256, SW_RZ, res, pred, 0);
    *dest_ptr++ = res;
    // CHECK-DAG: convert.f8_152 target_type=int16 rz {{%V[0-9]+}}, {{%V[0-9]+}}, {{%SP[0-9]+}}
    dest_vector_ptr = (int64 __local *)dest_ptr;
  }
  {
    short128 __local *dest_ptr = (short128 __local *)dest_vector_ptr;
    short128 res = {0};
    res =  v_convert_h8_to_i16_vb(src_minihalf256, SW_RZ, res, vpred128, 0);
    *dest_ptr++ = res;
    // CHECK-DAG: convert.f8_152 target_type=int16 rz {{%V[0-9]+}}, {{%V[0-9]+}}, {{%VP[0-9]+}}
    dest_vector_ptr = (int64 __local *)dest_ptr;
  }
  {
    ushort128 __local *dest_ptr = (ushort128 __local *)dest_vector_ptr;
    ushort128 res = {0};
    res =  v_convert_h8_to_u16_b(src_minihalf256, SW_RZ, res, pred, 0);
    *dest_ptr++ = res;
    // CHECK-DAG: convert.f8_152 target_type=uint16 rz {{%V[0-9]+}}, {{%V[0-9]+}}, {{%SP[0-9]+}}
    dest_vector_ptr = (int64 __local *)dest_ptr;
  }
  {
    ushort128 __local *dest_ptr = (ushort128 __local *)dest_vector_ptr;
    ushort128 res = {0};
    res =  v_convert_h8_to_u16_vb(src_minihalf256, SW_RZ, res, vpred128, 0);
    *dest_ptr++ = res;
    // CHECK-DAG: convert.f8_152 target_type=uint16 rz {{%V[0-9]+}}, {{%V[0-9]+}}, {{%VP[0-9]+}}
    dest_vector_ptr = (int64 __local *)dest_ptr;
  }

  {
    int64 __local *dest_ptr = (int64 __local *)dest_vector_ptr;
    int64 res = {0};
    res = v_convert_u16_to_i32_b(src_ushort128, SW_RZ, res, pred, 0);
    *dest_ptr++ = res;
    // CHECK-DAG: convert.u16 target_type=int32 rz {{%V[0-9]+}}, {{%V[0-9]+}}, {{%SP[0-9]+}}
    dest_vector_ptr = (int64 __local *)dest_ptr;
  }
  {
    int64 __local *dest_ptr = (int64 __local *)dest_vector_ptr;
    int64 res = {0};
    res = v_convert_u16_to_i32_vb(src_ushort128, SW_RZ, res, vpred64, 0);
    *dest_ptr++ = res;
    // CHECK-DAG: convert.u16 target_type=int32 rz {{%V[0-9]+}}, {{%V[0-9]+}}, {{%VP[0-9]+}}
    dest_vector_ptr = (int64 __local *)dest_ptr;
  }
  {
    uint64 __local *dest_ptr = (uint64 __local *)dest_vector_ptr;
    uint64 res = {0};
    res = v_convert_u16_to_u32_b(src_ushort128, SW_RZ, res, pred, 0);
    *dest_ptr++ = res;
    // CHECK-DAG: convert.u16 target_type=uint32 rz {{%V[0-9]+}}, {{%V[0-9]+}}, {{%SP[0-9]+}}
    dest_vector_ptr = (int64 __local *)dest_ptr;
  }
  {
    uint64 __local *dest_ptr = (uint64 __local *)dest_vector_ptr;
    uint64 res = {0};
    res = v_convert_u16_to_u32_vb(src_ushort128, SW_RZ, res, vpred64, 0);
    *dest_ptr++ = res;
    // CHECK-DAG: convert.u16 target_type=uint32 rz {{%V[0-9]+}}, {{%V[0-9]+}}, {{%VP[0-9]+}}
    dest_vector_ptr = (int64 __local *)dest_ptr;
  }
  {
    float64 __local *dest_ptr = (float64 __local *)dest_vector_ptr;
    float64 res = {0};
    res = v_convert_u16_to_f32_b(src_ushort128, SW_RZ, res, pred, 0);
    *dest_ptr++ = res;
    // CHECK-DAG: convert.u16 target_type=fp32 rz {{%V[0-9]+}}, {{%V[0-9]+}}, {{%SP[0-9]+}}
    dest_vector_ptr = (int64 __local *)dest_ptr;
  }
  {
    float64 __local *dest_ptr = (float64 __local *)dest_vector_ptr;
    float64 res = {0};
    res = v_convert_u16_to_f32_vb(src_ushort128, SW_RZ, res, vpred64, 0);
    *dest_ptr++ = res;
    // CHECK-DAG: convert.u16 target_type=fp32 rz {{%V[0-9]+}}, {{%V[0-9]+}}, {{%VP[0-9]+}}
    dest_vector_ptr = (int64 __local *)dest_ptr;
  }

  //Down-convert
  {
    minifloat256 __local *dest_ptr = (minifloat256 __local *)dest_vector_ptr;
    minifloat256 res = {0};
    res = v_convert_f32_to_f8_b(src_float64, 1, SW_RZ, res, pred, 0);
    *dest_ptr++ = res;
    // CHECK-DAG: convert.f32 lane_sel=1 target_type=f8_143 rz {{%V[0-9]+}}, {{%V[0-9]+}}, {{%SP[0-9]+}}
    dest_vector_ptr = (int64 __local *)dest_ptr;
  }
  {
    minifloat256 __local *dest_ptr = (minifloat256 __local *)dest_vector_ptr;
    minifloat256 res = {0};
    res = v_convert_f32_to_f8_vb(src_float64, 1, SW_RZ, res, vpred64, 0);
    *dest_ptr++ = res;
    // CHECK-DAG: convert.f32 lane_sel=1 target_type=f8_143 rz {{%V[0-9]+}}, {{%V[0-9]+}}, {{%VP[0-9]+}}
    dest_vector_ptr = (int64 __local *)dest_ptr;
  }
  {
    minihalf256 __local *dest_ptr = (minihalf256 __local *)dest_vector_ptr;
    minihalf256 res = {0};
    res = v_convert_f32_to_h8_b(src_float64, 1, SW_RZ, res, pred, 0);
    *dest_ptr++ = res;
    // CHECK-DAG: convert.f32 lane_sel=1 target_type=f8_152 rz {{%V[0-9]+}}, {{%V[0-9]+}}, {{%SP[0-9]+}}
    dest_vector_ptr = (int64 __local *)dest_ptr;
  }
  {
    minihalf256 __local *dest_ptr = (minihalf256 __local *)dest_vector_ptr;
    minihalf256 res = {0};
    res = v_convert_f32_to_h8_vb(src_float64, 1, SW_RZ, res, vpred64, 0);
    *dest_ptr++ = res;
    // CHECK-DAG: convert.f32 lane_sel=1 target_type=f8_152 rz {{%V[0-9]+}}, {{%V[0-9]+}}, {{%VP[0-9]+}}
    dest_vector_ptr = (int64 __local *)dest_ptr;
  }
  {
    ushort128 __local *dest_ptr = (ushort128 __local *)dest_vector_ptr;
    ushort128 res = {0};
    res = v_convert_f32_to_u16_b(src_float64, 1, SW_RZ, res, pred, 0);
    *dest_ptr++ = res;
    // CHECK-DAG: convert.f32 lane_sel=1 target_type=uint16 rz {{%V[0-9]+}}, {{%V[0-9]+}}, {{%SP[0-9]+}}
    dest_vector_ptr = (int64 __local *)dest_ptr;
  }
  {
    ushort128 __local *dest_ptr = (ushort128 __local *)dest_vector_ptr;
    ushort128 res = {0};
    res = v_convert_f32_to_u16_vb(src_float64, 1, SW_RZ, res, vpred64, 0);
    *dest_ptr++ = res;
    // CHECK-DAG: convert.f32 lane_sel=1 target_type=uint16 rz {{%V[0-9]+}}, {{%V[0-9]+}}, {{%VP[0-9]+}}
    dest_vector_ptr = (int64 __local *)dest_ptr;
  }
  {
    uchar256 __local *dest_ptr = (uchar256 __local *)dest_vector_ptr;
    uchar256 res = {0};
    res = v_convert_f32_to_u8_b(src_float64, 1, SW_RZ, res, pred, 0);
    *dest_ptr++ = res;
    // CHECK-DAG: convert.f32 lane_sel=1 target_type=uint8 rz {{%V[0-9]+}}, {{%V[0-9]+}}, {{%SP[0-9]+}}
    dest_vector_ptr = (int64 __local *)dest_ptr;
  }
  {
    uchar256 __local *dest_ptr = (uchar256 __local *)dest_vector_ptr;
    uchar256 res = {0};
    res = v_convert_f32_to_u8_vb(src_float64, 1, SW_RZ, res, vpred64, 0);
    *dest_ptr++ = res;
    // CHECK-DAG: convert.f32 lane_sel=1 target_type=uint8 rz {{%V[0-9]+}}, {{%V[0-9]+}}, {{%VP[0-9]+}}
    dest_vector_ptr = (int64 __local *)dest_ptr;
  }

  {
    minifloat256 __local *dest_ptr = (minifloat256 __local *)dest_vector_ptr;
    minifloat256 res = {0};
    res = v_convert_bf16_to_f8_b(src_bfloat128, 1, SW_RZ, res, pred, 0);
    *dest_ptr++ = res;
    // CHECK-DAG: convert.bf16 lane_sel=1 target_type=f8_143 rz {{%V[0-9]+}}, {{%V[0-9]+}}, {{%SP[0-9]+}}
    dest_vector_ptr = (int64 __local *)dest_ptr;
  }
  {
    minifloat256 __local *dest_ptr = (minifloat256 __local *)dest_vector_ptr;
    minifloat256 res = {0};
    res = v_convert_bf16_to_f8_vb(src_bfloat128, 1, SW_RZ, res, vpred128, 0);
    *dest_ptr++ = res;
    // CHECK-DAG: convert.bf16 lane_sel=1 target_type=f8_143 rz {{%V[0-9]+}}, {{%V[0-9]+}}, {{%VP[0-9]+}}
    dest_vector_ptr = (int64 __local *)dest_ptr;
  }
  {
    minihalf256 __local *dest_ptr = (minihalf256 __local *)dest_vector_ptr;
    minihalf256 res = {0};
    res = v_convert_bf16_to_h8_b(src_bfloat128, 1, SW_RZ, res, pred, 0);
    *dest_ptr++ = res;
    // CHECK-DAG: convert.bf16 lane_sel=1 target_type=f8_152 rz {{%V[0-9]+}}, {{%V[0-9]+}}, {{%SP[0-9]+}}
    dest_vector_ptr = (int64 __local *)dest_ptr;
  }
  {
    minihalf256 __local *dest_ptr = (minihalf256 __local *)dest_vector_ptr;
    minihalf256 res = {0};
    res = v_convert_bf16_to_h8_vb(src_bfloat128, 1, SW_RZ, res, vpred128, 0);
    *dest_ptr++ = res;
    // CHECK-DAG: convert.bf16 lane_sel=1 target_type=f8_152 rz {{%V[0-9]+}}, {{%V[0-9]+}}, {{%VP[0-9]+}}
    dest_vector_ptr = (int64 __local *)dest_ptr;
  }
  {
    uchar256 __local *dest_ptr = (uchar256 __local *)dest_vector_ptr;
    uchar256 res = {0};
    res = v_convert_bf16_to_u8_b(src_bfloat128, 1, SW_RZ, res, pred, 0);
    *dest_ptr++ = res;
    // CHECK-DAG: convert.bf16 lane_sel=1 target_type=uint8 rz {{%V[0-9]+}}, {{%V[0-9]+}}, {{%SP[0-9]+}}
    dest_vector_ptr = (int64 __local *)dest_ptr;
  }
  {
    uchar256 __local *dest_ptr = (uchar256 __local *)dest_vector_ptr;
    uchar256 res = {0};
    res = v_convert_bf16_to_u8_vb(src_bfloat128, 1, SW_RZ, res, vpred128, 0);
    *dest_ptr++ = res;
    // CHECK-DAG: convert.bf16 lane_sel=1 target_type=uint8 rz {{%V[0-9]+}}, {{%V[0-9]+}}, {{%VP[0-9]+}}
    dest_vector_ptr = (int64 __local *)dest_ptr;
  }

  {
    minifloat256 __local *dest_ptr = (minifloat256 __local *)dest_vector_ptr;
    minifloat256 res = {0};
    res = v_convert_f16_to_f8_b(src_half128, 1, SW_RZ, res, pred, 0);
    *dest_ptr++ = res;
    // CHECK-DAG: convert.f16 lane_sel=1 target_type=f8_143 rz {{%V[0-9]+}}, {{%V[0-9]+}}, {{%SP[0-9]+}}
    dest_vector_ptr = (int64 __local *)dest_ptr;
  }
  {
    minifloat256 __local *dest_ptr = (minifloat256 __local *)dest_vector_ptr;
    minifloat256 res = {0};
    res = v_convert_f16_to_f8_vb(src_half128, 1, SW_RZ, res, vpred128, 0);
    *dest_ptr++ = res;
    // CHECK-DAG: convert.f16 lane_sel=1 target_type=f8_143 rz {{%V[0-9]+}}, {{%V[0-9]+}}, {{%VP[0-9]+}}
    dest_vector_ptr = (int64 __local *)dest_ptr;
  }
  {
    minihalf256 __local *dest_ptr = (minihalf256 __local *)dest_vector_ptr;
    minihalf256 res = {0};
    res = v_convert_f16_to_h8_b(src_half128, 1, SW_RZ, res, pred, 0);
    *dest_ptr++ = res;
    // CHECK-DAG: convert.f16 lane_sel=1 target_type=f8_152 rz {{%V[0-9]+}}, {{%V[0-9]+}}, {{%SP[0-9]+}}
    dest_vector_ptr = (int64 __local *)dest_ptr;
  }
  {
    minihalf256 __local *dest_ptr = (minihalf256 __local *)dest_vector_ptr;
    minihalf256 res = {0};
    res = v_convert_f16_to_h8_vb(src_half128, 1, SW_RZ, res, vpred128, 0);
    *dest_ptr++ = res;
    // CHECK-DAG: convert.f16 lane_sel=1 target_type=f8_152 rz {{%V[0-9]+}}, {{%V[0-9]+}}, {{%VP[0-9]+}}
    dest_vector_ptr = (int64 __local *)dest_ptr;
  }
  {
    uchar256 __local *dest_ptr = (uchar256 __local *)dest_vector_ptr;
    uchar256 res = {0};
    res = v_convert_f16_to_u8_b(src_half128, 1, SW_RZ, res, pred, 0);
    *dest_ptr++ = res;
    // CHECK-DAG: convert.f16 lane_sel=1 target_type=uint8 rz {{%V[0-9]+}}, {{%V[0-9]+}}, {{%SP[0-9]+}}
    dest_vector_ptr = (int64 __local *)dest_ptr;
  }
  {
    uchar256 __local *dest_ptr = (uchar256 __local *)dest_vector_ptr;
    uchar256 res = {0};
    res = v_convert_f16_to_u8_vb(src_half128, 1, SW_RZ, res, vpred128, 0);
    *dest_ptr++ = res;
    // CHECK-DAG: convert.f16 lane_sel=1 target_type=uint8 rz {{%V[0-9]+}}, {{%V[0-9]+}}, {{%VP[0-9]+}}
    dest_vector_ptr = (int64 __local *)dest_ptr;
  }

  {
    minifloat256 __local *dest_ptr = (minifloat256 __local *)dest_vector_ptr;
    minifloat256 res = {0};
    res = v_convert_i32_to_f8_b(src_int64, 1, SW_RZ, res, pred, 0);
    *dest_ptr++ = res;
    // CHECK-DAG: convert.i32 lane_sel=1 target_type=f8_143 rz {{%V[0-9]+}}, {{%V[0-9]+}}, {{%SP[0-9]+}}
    dest_vector_ptr = (int64 __local *)dest_ptr;
  }
  {
    minifloat256 __local *dest_ptr = (minifloat256 __local *)dest_vector_ptr;
    minifloat256 res = {0};
    res = v_convert_i32_to_f8_vb(src_int64, 1, SW_RZ, res, vpred64, 0);
    *dest_ptr++ = res;
    // CHECK-DAG: convert.i32 lane_sel=1 target_type=f8_143 rz {{%V[0-9]+}}, {{%V[0-9]+}}, {{%VP[0-9]+}}
    dest_vector_ptr = (int64 __local *)dest_ptr;
  }
  {
    minihalf256 __local *dest_ptr = (minihalf256 __local *)dest_vector_ptr;
    minihalf256 res = {0};
    res = v_convert_i32_to_h8_b(src_int64, 1, SW_RZ, res, pred, 0);
    *dest_ptr++ = res;
    // CHECK-DAG: convert.i32 lane_sel=1 target_type=f8_152 rz {{%V[0-9]+}}, {{%V[0-9]+}}, {{%SP[0-9]+}}
    dest_vector_ptr = (int64 __local *)dest_ptr;
  }
  {
    minihalf256 __local *dest_ptr = (minihalf256 __local *)dest_vector_ptr;
    minihalf256 res = {0};
    res = v_convert_i32_to_h8_vb(src_int64, 1, SW_RZ, res, vpred64, 0);
    *dest_ptr++ = res;
    // CHECK-DAG: convert.i32 lane_sel=1 target_type=f8_152 rz {{%V[0-9]+}}, {{%V[0-9]+}}, {{%VP[0-9]+}}
    dest_vector_ptr = (int64 __local *)dest_ptr;
  }
  {
    short128 __local *dest_ptr = (short128 __local *)dest_vector_ptr;
    short128 res = {0};
    res = v_convert_i32_to_i16_b(src_int64, 1, SW_RZ, res, pred, 0);
    *dest_ptr++ = res;
    // CHECK-DAG: convert.i32 lane_sel=1 target_type=int16 rz {{%V[0-9]+}}, {{%V[0-9]+}}, {{%SP[0-9]+}}
    dest_vector_ptr = (int64 __local *)dest_ptr;
  }
  {
    short128 __local *dest_ptr = (short128 __local *)dest_vector_ptr;
    short128 res = {0};
    res = v_convert_i32_to_i16_vb(src_int64, 1, SW_RZ, res, vpred64, 0);
    *dest_ptr++ = res;
    // CHECK-DAG: convert.i32 lane_sel=1 target_type=int16 rz {{%V[0-9]+}}, {{%V[0-9]+}}, {{%VP[0-9]+}}
    dest_vector_ptr = (int64 __local *)dest_ptr;
  }
  {
    ushort128 __local *dest_ptr = (ushort128 __local *)dest_vector_ptr;
    ushort128 res = {0};
    res = v_convert_i32_to_u16_b(src_int64, 1, SW_RZ, res, pred, 0);
    *dest_ptr++ = res;
    // CHECK-DAG: convert.i32 lane_sel=1 target_type=uint16 rz {{%V[0-9]+}}, {{%V[0-9]+}}, {{%SP[0-9]+}}
    dest_vector_ptr = (int64 __local *)dest_ptr;
  }
  {
    ushort128 __local *dest_ptr = (ushort128 __local *)dest_vector_ptr;
    ushort128 res = {0};
    res = v_convert_i32_to_u16_vb(src_int64, 1, SW_RZ, res, vpred64, 0);
    *dest_ptr++ = res;
    // CHECK-DAG: convert.i32 lane_sel=1 target_type=uint16 rz {{%V[0-9]+}}, {{%V[0-9]+}}, {{%VP[0-9]+}}
    dest_vector_ptr = (int64 __local *)dest_ptr;
  }
  {
    char256 __local *dest_ptr = (char256 __local *)dest_vector_ptr;
    char256 res = {0};
    res = v_convert_i32_to_i8_b(src_int64, 1, SW_RZ, res, pred, 0);
    *dest_ptr++ = res;
    // CHECK-DAG: convert.i32 lane_sel=1 target_type=int8 rz {{%V[0-9]+}}, {{%V[0-9]+}}, {{%SP[0-9]+}}
    dest_vector_ptr = (int64 __local *)dest_ptr;
  }
  {
    char256 __local *dest_ptr = (char256 __local *)dest_vector_ptr;
    char256 res = {0};
    res = v_convert_i32_to_i8_vb(src_int64, 1, SW_RZ, res, vpred64, 0);
    *dest_ptr++ = res;
    // CHECK-DAG: convert.i32 lane_sel=1 target_type=int8 rz {{%V[0-9]+}}, {{%V[0-9]+}}, {{%VP[0-9]+}}
    dest_vector_ptr = (int64 __local *)dest_ptr;
  }

  {
    bfloat128 __local *dest_ptr = (bfloat128 __local *)dest_vector_ptr;
    bfloat128 res = {0};
    res = v_convert_u32_to_bf16_b(src_uint64, 1, SW_RZ, res, pred, 0);
    *dest_ptr++ = res;
    // CHECK-DAG: convert.u32 lane_sel=1 target_type=bf16 rz {{%V[0-9]+}}, {{%V[0-9]+}}, {{%SP[0-9]+}}
    dest_vector_ptr = (int64 __local *)dest_ptr;
  }
  {
    bfloat128 __local *dest_ptr = (bfloat128 __local *)dest_vector_ptr;
    bfloat128 res = {0};
    res = v_convert_u32_to_bf16_vb(src_uint64, 1, SW_RZ, res, vpred64, 0);
    *dest_ptr++ = res;
    // CHECK-DAG: convert.u32 lane_sel=1 target_type=bf16 rz {{%V[0-9]+}}, {{%V[0-9]+}}, {{%VP[0-9]+}}
    dest_vector_ptr = (int64 __local *)dest_ptr;
  }
  {
    half128 __local *dest_ptr = (half128 __local *)dest_vector_ptr;
    half128 res = {0};
    res = v_convert_u32_to_f16_b(src_uint64, 1, SW_RZ, res, pred, 0);
    *dest_ptr++ = res;
    // CHECK-DAG: convert.u32 lane_sel=1 target_type=f16 rz {{%V[0-9]+}}, {{%V[0-9]+}}, {{%SP[0-9]+}}
    dest_vector_ptr = (int64 __local *)dest_ptr;
  }
  {
    half128 __local *dest_ptr = (half128 __local *)dest_vector_ptr;
    half128 res = {0};
    res = v_convert_u32_to_f16_vb(src_uint64, 1, SW_RZ, res, vpred64, 0);
    *dest_ptr++ = res;
    // CHECK-DAG: convert.u32 lane_sel=1 target_type=f16 rz {{%V[0-9]+}}, {{%V[0-9]+}}, {{%VP[0-9]+}}
    dest_vector_ptr = (int64 __local *)dest_ptr;
  }
  {
    minifloat256 __local *dest_ptr = (minifloat256 __local *)dest_vector_ptr;
    minifloat256 res = {0};
    res = v_convert_u32_to_f8_b(src_uint64, 1, SW_RZ, res, pred, 0);
    *dest_ptr++ = res;
    // CHECK-DAG: convert.u32 lane_sel=1 target_type=f8_143 rz {{%V[0-9]+}}, {{%V[0-9]+}}, {{%SP[0-9]+}}
    dest_vector_ptr = (int64 __local *)dest_ptr;
  }
  {
    minifloat256 __local *dest_ptr = (minifloat256 __local *)dest_vector_ptr;
    minifloat256 res = {0};
    res = v_convert_u32_to_f8_vb(src_uint64, 1, SW_RZ, res, vpred64, 0);
    *dest_ptr++ = res;
    // CHECK-DAG: convert.u32 lane_sel=1 target_type=f8_143 rz {{%V[0-9]+}}, {{%V[0-9]+}}, {{%VP[0-9]+}}
    dest_vector_ptr = (int64 __local *)dest_ptr;
  }
  {
    minihalf256 __local *dest_ptr = (minihalf256 __local *)dest_vector_ptr;
    minihalf256 res = {0};
    res = v_convert_u32_to_h8_b(src_uint64, 1, SW_RZ, res, pred, 0);
    *dest_ptr++ = res;
    // CHECK-DAG: convert.u32 lane_sel=1 target_type=f8_152 rz {{%V[0-9]+}}, {{%V[0-9]+}}, {{%SP[0-9]+}}
    dest_vector_ptr = (int64 __local *)dest_ptr;
  }
  {
    minihalf256 __local *dest_ptr = (minihalf256 __local *)dest_vector_ptr;
    minihalf256 res = {0};
    res = v_convert_u32_to_h8_vb(src_uint64, 1, SW_RZ, res, vpred64, 0);
    *dest_ptr++ = res;
    // CHECK-DAG: convert.u32 lane_sel=1 target_type=f8_152 rz {{%V[0-9]+}}, {{%V[0-9]+}}, {{%VP[0-9]+}}
    dest_vector_ptr = (int64 __local *)dest_ptr;
  }
  {
    short128 __local *dest_ptr = (short128 __local *)dest_vector_ptr;
    short128 res = {0};
    res = v_convert_u32_to_i16_b(src_uint64, 1, SW_RZ, res, pred, 0);
    *dest_ptr++ = res;
    // CHECK-DAG: convert.u32 lane_sel=1 target_type=int16 rz {{%V[0-9]+}}, {{%V[0-9]+}}, {{%SP[0-9]+}}
    dest_vector_ptr = (int64 __local *)dest_ptr;
  }
  {
    short128 __local *dest_ptr = (short128 __local *)dest_vector_ptr;
    short128 res = {0};
    res = v_convert_u32_to_i16_vb(src_uint64, 1, SW_RZ, res, vpred64, 0);
    *dest_ptr++ = res;
    // CHECK-DAG: convert.u32 lane_sel=1 target_type=int16 rz {{%V[0-9]+}}, {{%V[0-9]+}}, {{%VP[0-9]+}}
    dest_vector_ptr = (int64 __local *)dest_ptr;
  }
  {
    ushort128 __local *dest_ptr = (ushort128 __local *)dest_vector_ptr;
    ushort128 res = {0};
    res = v_convert_u32_to_u16_b(src_uint64, 1, SW_RZ, res, pred, 0);
    *dest_ptr++ = res;
    // CHECK-DAG: convert.u32 lane_sel=1 target_type=uint16 rz {{%V[0-9]+}}, {{%V[0-9]+}}, {{%SP[0-9]+}}
    dest_vector_ptr = (int64 __local *)dest_ptr;
  }
  {
    ushort128 __local *dest_ptr = (ushort128 __local *)dest_vector_ptr;
    ushort128 res = {0};
    res = v_convert_u32_to_u16_vb(src_uint64, 1, SW_RZ, res, vpred64, 0);
    *dest_ptr++ = res;
    // CHECK-DAG: convert.u32 lane_sel=1 target_type=uint16 rz {{%V[0-9]+}}, {{%V[0-9]+}}, {{%VP[0-9]+}}
    dest_vector_ptr = (int64 __local *)dest_ptr;
  }
  {
    char256 __local *dest_ptr = (char256 __local *)dest_vector_ptr;
    char256 res = {0};
    res = v_convert_u32_to_i8_b(src_uint64, 1, SW_RZ, res, pred, 0);
    *dest_ptr++ = res;
    // CHECK-DAG: convert.u32 lane_sel=1 target_type=int8 rz {{%V[0-9]+}}, {{%V[0-9]+}}, {{%SP[0-9]+}}
    dest_vector_ptr = (int64 __local *)dest_ptr;
  }
  {
    char256 __local *dest_ptr = (char256 __local *)dest_vector_ptr;
    char256 res = {0};
    res = v_convert_u32_to_i8_vb(src_uint64, 1, SW_RZ, res, vpred64, 0);
    *dest_ptr++ = res;
    // CHECK-DAG: convert.u32 lane_sel=1 target_type=int8 rz {{%V[0-9]+}}, {{%V[0-9]+}}, {{%VP[0-9]+}}
    dest_vector_ptr = (int64 __local *)dest_ptr;
  }
  {
    uchar256 __local *dest_ptr = (uchar256 __local *)dest_vector_ptr;
    uchar256 res = {0};
    res = v_convert_u32_to_u8_b(src_uint64, 1, SW_RZ, res, pred, 0);
    *dest_ptr++ = res;
    // CHECK-DAG: convert.u32 lane_sel=1 target_type=uint8 rz {{%V[0-9]+}}, {{%V[0-9]+}}, {{%SP[0-9]+}}
    dest_vector_ptr = (int64 __local *)dest_ptr;
  }
  {
    uchar256 __local *dest_ptr = (uchar256 __local *)dest_vector_ptr;
    uchar256 res = {0};
    res = v_convert_u32_to_u8_vb(src_uint64, 1, SW_RZ, res, vpred64, 0);
    *dest_ptr++ = res;
    // CHECK-DAG: convert.u32 lane_sel=1 target_type=uint8 rz {{%V[0-9]+}}, {{%V[0-9]+}}, {{%VP[0-9]+}}
    dest_vector_ptr = (int64 __local *)dest_ptr;
  }

  {
    minifloat256 __local *dest_ptr = (minifloat256 __local *)dest_vector_ptr;
    minifloat256 res = {0};
    res = v_convert_i16_to_f8_b(src_short128, 1, SW_RZ, res, pred, 0);
    *dest_ptr++ = res;
    // CHECK-DAG: convert.i16 lane_sel=1 target_type=f8_143 rz {{%V[0-9]+}}, {{%V[0-9]+}}, {{%SP[0-9]+}}
    dest_vector_ptr = (int64 __local *)dest_ptr;
  }
  {
    minifloat256 __local *dest_ptr = (minifloat256 __local *)dest_vector_ptr;
    minifloat256 res = {0};
    res = v_convert_i16_to_f8_vb(src_short128, 1, SW_RZ, res, vpred128, 0);
    *dest_ptr++ = res;
    // CHECK-DAG: convert.i16 lane_sel=1 target_type=f8_143 rz {{%V[0-9]+}}, {{%V[0-9]+}}, {{%VP[0-9]+}}
    dest_vector_ptr = (int64 __local *)dest_ptr;
  }
  {
    minihalf256 __local *dest_ptr = (minihalf256 __local *)dest_vector_ptr;
    minihalf256 res = {0};
    res = v_convert_i16_to_h8_b(src_short128, 1, SW_RZ, res, pred, 0);
    *dest_ptr++ = res;
    // CHECK-DAG: convert.i16 lane_sel=1 target_type=f8_152 rz {{%V[0-9]+}}, {{%V[0-9]+}}, {{%SP[0-9]+}}
    dest_vector_ptr = (int64 __local *)dest_ptr;
  }
  {
    minihalf256 __local *dest_ptr = (minihalf256 __local *)dest_vector_ptr;
    minihalf256 res = {0};
    res = v_convert_i16_to_h8_vb(src_short128, 1, SW_RZ, res, vpred128, 0);
    *dest_ptr++ = res;
    // CHECK-DAG: convert.i16 lane_sel=1 target_type=f8_152 rz {{%V[0-9]+}}, {{%V[0-9]+}}, {{%VP[0-9]+}}
    dest_vector_ptr = (int64 __local *)dest_ptr;
  }
  {
    char256 __local *dest_ptr = (char256 __local *)dest_vector_ptr;
    char256 res = {0};
    res = v_convert_i16_to_i8_b(src_short128, 1, SW_RZ, res, pred, 0);
    *dest_ptr++ = res;
    // CHECK-DAG: convert.i16 lane_sel=1 target_type=int8 rz {{%V[0-9]+}}, {{%V[0-9]+}}, {{%SP[0-9]+}}
    dest_vector_ptr = (int64 __local *)dest_ptr;
  }
  {
    char256 __local *dest_ptr = (char256 __local *)dest_vector_ptr;
    char256 res = {0};
    res = v_convert_i16_to_i8_vb(src_short128, 1, SW_RZ, res, vpred128, 0);
    *dest_ptr++ = res;
    // CHECK-DAG: convert.i16 lane_sel=1 target_type=int8 rz {{%V[0-9]+}}, {{%V[0-9]+}}, {{%VP[0-9]+}}
    dest_vector_ptr = (int64 __local *)dest_ptr;
  }

  {
    minifloat256 __local *dest_ptr = (minifloat256 __local *)dest_vector_ptr;
    minifloat256 res = {0};
    res = v_convert_u16_to_f8_b(src_ushort128, 1, SW_RZ, res, pred, 0);
    *dest_ptr++ = res;
    // CHECK-DAG: convert.u16 lane_sel=1 target_type=f8_143 rz {{%V[0-9]+}}, {{%V[0-9]+}}, {{%SP[0-9]+}}
    dest_vector_ptr = (int64 __local *)dest_ptr;
  }
  {
    minifloat256 __local *dest_ptr = (minifloat256 __local *)dest_vector_ptr;
    minifloat256 res = {0};
    res = v_convert_u16_to_f8_vb(src_ushort128, 1, SW_RZ, res, vpred128, 0);
    *dest_ptr++ = res;
    // CHECK-DAG: convert.u16 lane_sel=1 target_type=f8_143 rz {{%V[0-9]+}}, {{%V[0-9]+}}, {{%VP[0-9]+}}
    dest_vector_ptr = (int64 __local *)dest_ptr;
  }
  {
    minihalf256 __local *dest_ptr = (minihalf256 __local *)dest_vector_ptr;
    minihalf256 res = {0};
    res = v_convert_u16_to_h8_b(src_ushort128, 1, SW_RZ, res, pred, 0);
    *dest_ptr++ = res;
    // CHECK-DAG: convert.u16 lane_sel=1 target_type=f8_152 rz {{%V[0-9]+}}, {{%V[0-9]+}}, {{%SP[0-9]+}}
    dest_vector_ptr = (int64 __local *)dest_ptr;
  }
  {
    minihalf256 __local *dest_ptr = (minihalf256 __local *)dest_vector_ptr;
    minihalf256 res = {0};
    res = v_convert_u16_to_h8_vb(src_ushort128, 1, SW_RZ, res, vpred128, 0);
    *dest_ptr++ = res;
    // CHECK-DAG: convert.u16 lane_sel=1 target_type=f8_152 rz {{%V[0-9]+}}, {{%V[0-9]+}}, {{%VP[0-9]+}}
    dest_vector_ptr = (int64 __local *)dest_ptr;
  }
  {
    char256 __local *dest_ptr = (char256 __local *)dest_vector_ptr;
    char256 res = {0};
    res = v_convert_u16_to_i8_b(src_ushort128, 1, SW_RZ, res, pred, 0);
    *dest_ptr++ = res;
    // CHECK-DAG: convert.u16 lane_sel=1 target_type=int8 rz {{%V[0-9]+}}, {{%V[0-9]+}}, {{%SP[0-9]+}}
    dest_vector_ptr = (int64 __local *)dest_ptr;
  }
  {
    char256 __local *dest_ptr = (char256 __local *)dest_vector_ptr;
    char256 res = {0};
    res = v_convert_u16_to_i8_vb(src_ushort128, 1, SW_RZ, res, vpred128, 0);
    *dest_ptr++ = res;
    // CHECK-DAG: convert.u16 lane_sel=1 target_type=int8 rz {{%V[0-9]+}}, {{%V[0-9]+}}, {{%VP[0-9]+}}
    dest_vector_ptr = (int64 __local *)dest_ptr;
  }
  {
    uchar256 __local *dest_ptr = (uchar256 __local *)dest_vector_ptr;
    uchar256 res = {0};
    res = v_convert_u16_to_u8_b(src_ushort128, 1, SW_RZ, res, pred, 0);
    *dest_ptr++ = res;
    // CHECK-DAG: convert.u16 lane_sel=1 target_type=uint8 rz {{%V[0-9]+}}, {{%V[0-9]+}}, {{%SP[0-9]+}}
    dest_vector_ptr = (int64 __local *)dest_ptr;
  }
  {
    uchar256 __local *dest_ptr = (uchar256 __local *)dest_vector_ptr;
    uchar256 res = {0};
    res = v_convert_u16_to_u8_vb(src_ushort128, 1, SW_RZ, res, vpred128, 0);
    *dest_ptr++ = res;
    // CHECK-DAG: convert.u16 lane_sel=1 target_type=uint8 rz {{%V[0-9]+}}, {{%V[0-9]+}}, {{%VP[0-9]+}}
    dest_vector_ptr = (int64 __local *)dest_ptr;
  }
}
