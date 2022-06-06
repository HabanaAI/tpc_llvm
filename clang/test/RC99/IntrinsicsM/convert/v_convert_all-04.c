// RUN: %codegen -S -O1 -triple tpc-none-none -std=rc99 -bfloat16 -target-cpu gaudi2 %s -o - | FileCheck --check-prefixes=CHECK,GAUDI2 %s
// UN: %codegen -S -O1 -triple tpc-none-none -std=rc99 -bfloat16 -target-cpu doron1 %s -o - | FileCheck --check-prefixes=DORON1 %s
// doron1 disabled until code for conversion will be stable
// GAUDI-1543
void main(int src, int destv, int destd, int desta, _Bool pred, int vpred)
{
  int64 __local *destv_vector_ptr = (int64 __local *)destv;
  int128 __local *destd_vector_ptr = (int128 __local *)destd;
  int256 __local *desta_vector_ptr = (int256 __local *)desta;

  //VRF
  float64 __local *src_float64_ptr = (float64 __local *)destv_vector_ptr;
  bfloat128 __local *src_bfloat128_ptr = (bfloat128 __local *)destv_vector_ptr;
  half128 __local *src_half128_ptr = (half128 __local *)destv_vector_ptr;
  minifloat256 __local *src_minifloat256_ptr = (minifloat256 __local *)destv_vector_ptr;
  minihalf256 __local *src_minihalf256_ptr = (minihalf256 __local *)destv_vector_ptr;
  int64 __local *src_int64_ptr = (int64 __local *)destv_vector_ptr;
  short128 __local *src_short128_ptr = (short128 __local *)destv_vector_ptr;
  char256 __local *src_char256_ptr = (char256 __local *)destv_vector_ptr;
  uint64 __local *src_uint64_ptr = (uint64 __local *)destv_vector_ptr;
  ushort128 __local *src_ushort128_ptr = (ushort128 __local *)destv_vector_ptr;
  uchar256 __local *src_uchar256_ptr = (uchar256 __local *)destv_vector_ptr;
  //DRF
  float128 __local *src_float128_ptr = (float128 __local *)destd_vector_ptr;
  bfloat256 __local *src_bfloat256_ptr = (bfloat256 __local *)destd_vector_ptr;
  half256 __local *src_half256_ptr = (half256 __local *)destd_vector_ptr;
  int128 __local *src_int128_ptr = (int128 __local *)destd_vector_ptr;
  short256 __local *src_short256_ptr = (short256 __local *)destd_vector_ptr;
  char512 __local *src_char512_ptr = (char512 __local *)destd_vector_ptr;
  uint128 __local *src_uint128_ptr = (uint128 __local *)destd_vector_ptr;
  ushort256 __local *src_ushort256_ptr = (ushort256 __local *)destd_vector_ptr;
  uchar512 __local *src_uchar512_ptr = (uchar512 __local *)destd_vector_ptr;
  //ARF
  float256 __local *src_float256_ptr = (float256 __local *)desta_vector_ptr;
  int256 __local *src_int256_ptr = (int256 __local *)desta_vector_ptr;
  uint256 __local *src_uint256_ptr = (uint256 __local *)desta_vector_ptr;

  bool256 __local *vpred256_ptr = (bool256 __local *)vpred;
  bool128 __local *vpred128_ptr = (bool128 __local *)vpred;
  bool64 __local *vpred64_ptr = (bool64 __local *)vpred;

  //VRF
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
  //DRF
  float128 src_float128 = *src_float128_ptr;
  bfloat256 src_bfloat256 = *src_bfloat256_ptr;
  half256 src_half256 = *src_half256_ptr;
  int128 src_int128 = *src_int128_ptr;
  short256 src_short256 = *src_short256_ptr;
  char512 src_char512 = *src_char512_ptr;
  uint128 src_uint128 = *src_uint128_ptr;
  ushort256 src_ushort256 = *src_ushort256_ptr;
  uchar512 src_uchar512 = *src_uchar512_ptr;
  //ARF
  float256 src_float256 = *src_float256_ptr;
  int256 src_int256 = *src_int256_ptr;
  uint256 src_uint256 = *src_uint256_ptr;

  bool256 vpred256 = *vpred256_ptr;
  bool128 vpred128 = *vpred128_ptr;
  bool64 vpred64 = *vpred64_ptr;


  {
    minifloat256 __local *dest_ptr = (minifloat256 __local *)destv_vector_ptr;
    minifloat256 res = {0};
    res = v_convert_f32_to_f8_all_b(src_float256, SW_RZ, res, pred, 0);
    *dest_ptr++ = res;
    // CHECK-DAG: convert.f32 all_lanes target_type=f8_143 rz {{%V[0-9]+}}, {{%A[0-9]+}}, {{%SP[0-9]+}}
    destv_vector_ptr = (int64 __local *)dest_ptr;
  }
  {
    minifloat256 __local *dest_ptr = (minifloat256 __local *)destv_vector_ptr;
    minifloat256 res = {0};
    res = v_convert_f32_to_f8_all_vb(src_float256, SW_RZ, res, vpred256, 0);
    *dest_ptr++ = res;
    // CHECK-DAG: convert.f32 all_lanes target_type=f8_143 rz {{%V[0-9]+}}, {{%A[0-9]+}}, {{%VP[0-9]+}}
    destv_vector_ptr = (int64 __local *)dest_ptr;
  }
  {
    minihalf256 __local *dest_ptr = (minihalf256 __local *)destv_vector_ptr;
    minihalf256 res = {0};
    res = v_convert_f32_to_h8_all_b(src_float256, SW_RZ, res, pred, 0);
    *dest_ptr++ = res;
    // CHECK-DAG: convert.f32 all_lanes target_type=f8_152 rz {{%V[0-9]+}}, {{%A[0-9]+}}, {{%SP[0-9]+}}
    destv_vector_ptr = (int64 __local *)dest_ptr;
  }
  {
    minihalf256 __local *dest_ptr = (minihalf256 __local *)destv_vector_ptr;
    minihalf256 res = {0};
    res = v_convert_f32_to_h8_all_vb(src_float256, SW_RZ, res, vpred256, 0);
    *dest_ptr++ = res;
    // CHECK-DAG: convert.f32 all_lanes target_type=f8_152 rz {{%V[0-9]+}}, {{%A[0-9]+}}, {{%VP[0-9]+}}
    destv_vector_ptr = (int64 __local *)dest_ptr;
  }
  {
    ushort128 __local *dest_ptr = (ushort128 __local *)destv_vector_ptr;
    ushort128 res = {0};
    res = v_convert_f32_to_u16_all_b(src_float128, SW_RZ, res, pred, 0);
    *dest_ptr++ = res;
    // CHECK-DAG: convert.f32 all_lanes target_type=uint16 rz {{%V[0-9]+}}, {{%D[0-9]+}}, {{%SP[0-9]+}}
    destv_vector_ptr = (int64 __local *)dest_ptr;
  }
  {
    ushort128 __local *dest_ptr = (ushort128 __local *)destv_vector_ptr;
    ushort128 res = {0};
    res = v_convert_f32_to_u16_all_vb(src_float128, SW_RZ, res, vpred128, 0);
    *dest_ptr++ = res;
    // CHECK-DAG: convert.f32 all_lanes target_type=uint16 rz {{%V[0-9]+}}, {{%D[0-9]+}}, {{%VP[0-9]+}}
    destv_vector_ptr = (int64 __local *)dest_ptr;
  }
  {
    uchar256 __local *dest_ptr = (uchar256 __local *)destv_vector_ptr;
    uchar256 res = {0};
    res = v_convert_f32_to_u8_all_b(src_float256, SW_RZ, res, pred, 0);
    *dest_ptr++ = res;
    // CHECK-DAG: convert.f32 all_lanes target_type=uint8 rz {{%V[0-9]+}}, {{%A[0-9]+}}, {{%SP[0-9]+}}
    destv_vector_ptr = (int64 __local *)dest_ptr;
  }
  {
    uchar256 __local *dest_ptr = (uchar256 __local *)destv_vector_ptr;
    uchar256 res = {0};
    res = v_convert_f32_to_u8_all_vb(src_float256, SW_RZ, res, vpred256, 0);
    *dest_ptr++ = res;
    // CHECK-DAG: convert.f32 all_lanes target_type=uint8 rz {{%V[0-9]+}}, {{%A[0-9]+}}, {{%VP[0-9]+}}
    destv_vector_ptr = (int64 __local *)dest_ptr;
  }

  {
    minifloat256 __local *dest_ptr = (minifloat256 __local *)destv_vector_ptr;
    minifloat256 res = {0};
    res =  v_convert_bf16_to_f8_all_b(src_bfloat256, SW_RZ, res, pred, 0);
    *dest_ptr++ = res;
    // CHECK-DAG: convert.bf16 all_lanes target_type=f8_143 rz {{%V[0-9]+}}, {{%D[0-9]+}}, {{%SP[0-9]+}}
    destv_vector_ptr = (int64 __local *)dest_ptr;
  }
  {
    minifloat256 __local *dest_ptr = (minifloat256 __local *)destv_vector_ptr;
    minifloat256 res = {0};
    res =  v_convert_bf16_to_f8_all_vb(src_bfloat256, SW_RZ, res, vpred256, 0);
    *dest_ptr++ = res;
    // CHECK-DAG: convert.bf16 all_lanes target_type=f8_143 rz {{%V[0-9]+}}, {{%D[0-9]+}}, {{%VP[0-9]+}}
    destv_vector_ptr = (int64 __local *)dest_ptr;
  }
  {
    minihalf256 __local *dest_ptr = (minihalf256 __local *)destv_vector_ptr;
    minihalf256 res = {0};
    res =  v_convert_bf16_to_h8_all_b(src_bfloat256, SW_RZ, res, pred, 0);
    *dest_ptr++ = res;
    // CHECK-DAG: convert.bf16 all_lanes target_type=f8_152 rz {{%V[0-9]+}}, {{%D[0-9]+}}, {{%SP[0-9]+}}
    destv_vector_ptr = (int64 __local *)dest_ptr;
  }
  {
    minihalf256 __local *dest_ptr = (minihalf256 __local *)destv_vector_ptr;
    minihalf256 res = {0};
    res =  v_convert_bf16_to_h8_all_vb(src_bfloat256, SW_RZ, res, vpred256, 0);
    *dest_ptr++ = res;
    // CHECK-DAG: convert.bf16 all_lanes target_type=f8_152 rz {{%V[0-9]+}}, {{%D[0-9]+}}, {{%VP[0-9]+}}
    destv_vector_ptr = (int64 __local *)dest_ptr;
  }
  {
    int128 __local *dest_ptr = (int128 __local *)destd_vector_ptr;
    int128 res = {0};
    res =  v_convert_bf16_to_i32_all_b(src_bfloat128, SW_RZ, res, pred, 0);
    *dest_ptr++ = res;
    // CHECK-DAG: convert.bf16 all_lanes target_type=int32 rz {{%D[0-9]+}}, {{%V[0-9]+}}, {{%SP[0-9]+}}
    destd_vector_ptr = (int128 __local *)dest_ptr;
  }
  {
    int128 __local *dest_ptr = (int128 __local *)destd_vector_ptr;
    int128 res = {0};
    res =  v_convert_bf16_to_i32_all_vb(src_bfloat128, SW_RZ, res, vpred128, 0);
    *dest_ptr++ = res;
    // CHECK-DAG: convert.bf16 all_lanes target_type=int32 rz {{%D[0-9]+}}, {{%V[0-9]+}}, {{%VP[0-9]+}}
    destd_vector_ptr = (int128 __local *)dest_ptr;
  }
  {
    uint128 __local *dest_ptr = (uint128 __local *)destd_vector_ptr;
    uint128 res = {0};
    res =  v_convert_bf16_to_u32_all_b(src_bfloat128, SW_RZ, res, pred, 0);
    *dest_ptr++ = res;
    // CHECK-DAG: convert.bf16 all_lanes target_type=uint32 rz {{%D[0-9]+}}, {{%V[0-9]+}}, {{%SP[0-9]+}}
    destd_vector_ptr = (int128 __local *)dest_ptr;
  }
  {
    uint128 __local *dest_ptr = (uint128 __local *)destd_vector_ptr;
    uint128 res = {0};
    res =  v_convert_bf16_to_u32_all_vb(src_bfloat128, SW_RZ, res, vpred128, 0);
    *dest_ptr++ = res;
    // CHECK-DAG: convert.bf16 all_lanes target_type=uint32 rz {{%D[0-9]+}}, {{%V[0-9]+}}, {{%VP[0-9]+}}
    destd_vector_ptr = (int128 __local *)dest_ptr;
  }
  {
    uchar256 __local *dest_ptr = (uchar256 __local *)destv_vector_ptr;
    uchar256 res = {0};
    res =  v_convert_bf16_to_u8_all_b(src_bfloat256, SW_RZ, res, pred, 0);
    *dest_ptr++ = res;
    // CHECK-DAG: convert.bf16 all_lanes target_type=uint8 rz {{%V[0-9]+}}, {{%D[0-9]+}}, {{%SP[0-9]+}}
    destv_vector_ptr = (int64 __local *)dest_ptr;
  }
  {
    uchar256 __local *dest_ptr = (uchar256 __local *)destv_vector_ptr;
    uchar256 res = {0};
    res =  v_convert_bf16_to_u8_all_vb(src_bfloat256, SW_RZ, res, vpred256, 0);
    *dest_ptr++ = res;
    // CHECK-DAG: convert.bf16 all_lanes target_type=uint8 rz {{%V[0-9]+}}, {{%D[0-9]+}}, {{%VP[0-9]+}}
    destv_vector_ptr = (int64 __local *)dest_ptr;
  }

  {
    minifloat256 __local *dest_ptr = (minifloat256 __local *)destv_vector_ptr;
    minifloat256 res = {0};
    res =  v_convert_f16_to_f8_all_b(src_half256, SW_RZ, res, pred, 0);
    *dest_ptr++ = res;
    // CHECK-DAG: convert.f16 all_lanes target_type=f8_143 rz {{%V[0-9]+}}, {{%D[0-9]+}}, {{%SP[0-9]+}}
    destv_vector_ptr = (int64 __local *)dest_ptr;
  }
  {
    minifloat256 __local *dest_ptr = (minifloat256 __local *)destv_vector_ptr;
    minifloat256 res = {0};
    res =  v_convert_f16_to_f8_all_vb(src_half256, SW_RZ, res, vpred256, 0);
    *dest_ptr++ = res;
    // CHECK-DAG: convert.f16 all_lanes target_type=f8_143 rz {{%V[0-9]+}}, {{%D[0-9]+}}, {{%VP[0-9]+}}
    destv_vector_ptr = (int64 __local *)dest_ptr;
  }
  {
    minihalf256 __local *dest_ptr = (minihalf256 __local *)destv_vector_ptr;
    minihalf256 res = {0};
    res =  v_convert_f16_to_h8_all_b(src_half256, SW_RZ, res, pred, 0);
    *dest_ptr++ = res;
    // CHECK-DAG: convert.f16 all_lanes target_type=f8_152 rz {{%V[0-9]+}}, {{%D[0-9]+}}, {{%SP[0-9]+}}
    destv_vector_ptr = (int64 __local *)dest_ptr;
  }
  {
    minihalf256 __local *dest_ptr = (minihalf256 __local *)destv_vector_ptr;
    minihalf256 res = {0};
    res =  v_convert_f16_to_h8_all_vb(src_half256, SW_RZ, res, vpred256, 0);
    *dest_ptr++ = res;
    // CHECK-DAG: convert.f16 all_lanes target_type=f8_152 rz {{%V[0-9]+}}, {{%D[0-9]+}}, {{%VP[0-9]+}}
    destv_vector_ptr = (int64 __local *)dest_ptr;
  }
  {
    int128 __local *dest_ptr = (int128 __local *)destd_vector_ptr;
    int128 res = {0};
    res =  v_convert_f16_to_i32_all_b(src_half128, SW_RZ, res, pred, 0);
    *dest_ptr++ = res;
    // CHECK-DAG: convert.f16 all_lanes target_type=int32 rz {{%D[0-9]+}}, {{%V[0-9]+}}, {{%SP[0-9]+}}
    destd_vector_ptr = (int128 __local *)dest_ptr;
  }
  {
    int128 __local *dest_ptr = (int128 __local *)destd_vector_ptr;
    int128 res = {0};
    res =  v_convert_f16_to_i32_all_vb(src_half128, SW_RZ, res, vpred128, 0);
    *dest_ptr++ = res;
    // CHECK-DAG: convert.f16 all_lanes target_type=int32 rz {{%D[0-9]+}}, {{%V[0-9]+}}, {{%VP[0-9]+}}
    destd_vector_ptr = (int128 __local *)dest_ptr;
  }
  {
    uint128 __local *dest_ptr = (uint128 __local *)destd_vector_ptr;
    uint128 res = {0};
    res =  v_convert_f16_to_u32_all_b(src_half128, SW_RZ, res, pred, 0);
    *dest_ptr++ = res;
    // CHECK-DAG: convert.f16 all_lanes target_type=uint32 rz {{%D[0-9]+}}, {{%V[0-9]+}}, {{%SP[0-9]+}}
    destd_vector_ptr = (int128 __local *)dest_ptr;
  }
  {
    uint128 __local *dest_ptr = (uint128 __local *)destd_vector_ptr;
    uint128 res = {0};
    res =  v_convert_f16_to_u32_all_vb(src_half128, SW_RZ, res, vpred128, 0);
    *dest_ptr++ = res;
    // CHECK-DAG: convert.f16 all_lanes target_type=uint32 rz {{%D[0-9]+}}, {{%V[0-9]+}}, {{%VP[0-9]+}}
    destd_vector_ptr = (int128 __local *)dest_ptr;
  }
  {
    uchar256 __local *dest_ptr = (uchar256 __local *)destv_vector_ptr;
    uchar256 res = {0};
    res =  v_convert_f16_to_u8_all_b(src_half256, SW_RZ, res, pred, 0);
    *dest_ptr++ = res;
    // CHECK-DAG: convert.f16 all_lanes target_type=uint8 rz {{%V[0-9]+}}, {{%D[0-9]+}}, {{%SP[0-9]+}}
    destv_vector_ptr = (int64 __local *)dest_ptr;
  }
  {
    uchar256 __local *dest_ptr = (uchar256 __local *)destv_vector_ptr;
    uchar256 res = {0};
    res =  v_convert_f16_to_u8_all_vb(src_half256, SW_RZ, res, vpred256, 0);
    *dest_ptr++ = res;
    // CHECK-DAG: convert.f16 all_lanes target_type=uint8 rz {{%V[0-9]+}}, {{%D[0-9]+}}, {{%VP[0-9]+}}
    destv_vector_ptr = (int64 __local *)dest_ptr;
  }

  {
    float256 __local *dest_ptr = (float256 __local *)desta_vector_ptr;
    float256 res = {0};
    res =  v_convert_f8_to_f32_all_b(src_minifloat256, SW_RZ, res, pred, 0);
    *dest_ptr++ = res;
    // CHECK-DAG: convert.f8_143 all_lanes target_type=fp32 rz {{%A[0-9]+}}, {{%V[0-9]+}}, {{%SP[0-9]+}}
    desta_vector_ptr = (int256 __local *)dest_ptr;
  }
  {
    float256 __local *dest_ptr = (float256 __local *)desta_vector_ptr;
    float256 res = {0};
    res =  v_convert_f8_to_f32_all_vb(src_minifloat256, SW_RZ, res, vpred256, 0);
    *dest_ptr++ = res;
    // CHECK-DAG: convert.f8_143 all_lanes target_type=fp32 rz {{%A[0-9]+}}, {{%V[0-9]+}}, {{%VP[0-9]+}}
    desta_vector_ptr = (int256 __local *)dest_ptr;
  }
  {
    bfloat256 __local *dest_ptr = (bfloat256 __local *)destd_vector_ptr;
    bfloat256 res = {0};
    res =  v_convert_f8_to_bf16_all_b(src_minifloat256, SW_RZ, res, pred, 0);
    *dest_ptr++ = res;
    // CHECK-DAG: convert.f8_143 all_lanes target_type=bf16 rz {{%D[0-9]+}}, {{%V[0-9]+}}, {{%SP[0-9]+}}
    destd_vector_ptr = (int128 __local *)dest_ptr;
  }
  {
    bfloat256 __local *dest_ptr = (bfloat256 __local *)destd_vector_ptr;
    bfloat256 res = {0};
    res =  v_convert_f8_to_bf16_all_vb(src_minifloat256, SW_RZ, res, vpred256, 0);
    *dest_ptr++ = res;
    // CHECK-DAG: convert.f8_143 all_lanes target_type=bf16 rz {{%D[0-9]+}}, {{%V[0-9]+}}, {{%VP[0-9]+}}
    destd_vector_ptr = (int128 __local *)dest_ptr;
  }
  {
    half256 __local *dest_ptr = (half256 __local *)destd_vector_ptr;
    half256 res = {0};
    res =  v_convert_f8_to_f16_all_b(src_minifloat256, SW_RZ, res, pred, 0);
    *dest_ptr++ = res;
    // CHECK-DAG: convert.f8_143 all_lanes target_type=f16 rz {{%D[0-9]+}}, {{%V[0-9]+}}, {{%SP[0-9]+}}
    destd_vector_ptr = (int128 __local *)dest_ptr;
  }
  {
    half256 __local *dest_ptr = (half256 __local *)destd_vector_ptr;
    half256 res = {0};
    res =  v_convert_f8_to_f16_all_vb(src_minifloat256, SW_RZ, res, vpred256, 0);
    *dest_ptr++ = res;
    // CHECK-DAG: convert.f8_143 all_lanes target_type=f16 rz {{%D[0-9]+}}, {{%V[0-9]+}}, {{%VP[0-9]+}}
    destd_vector_ptr = (int128 __local *)dest_ptr;
  }
  {
    int256 __local *dest_ptr = (int256 __local *)desta_vector_ptr;
    int256 res = {0};
    res =  v_convert_f8_to_i32_all_b(src_minifloat256, SW_RZ, res, pred, 0);
    *dest_ptr++ = res;
    // CHECK-DAG : convert.f8_143 all_lanes target_type=int32 rz {{%A[0-9]+}}, {{%V[0-9]+}}, {{%SP[0-9]+}}
    desta_vector_ptr = (int256 __local *)dest_ptr;
  }
  {
    int256 __local *dest_ptr = (int256 __local *)desta_vector_ptr;
    int256 res = {0};
    res =  v_convert_f8_to_i32_all_vb(src_minifloat256, SW_RZ, res, vpred256, 0);
    *dest_ptr++ = res;
    // CHECK-DAG: convert.f8_143 all_lanes target_type=int32 rz {{%A[0-9]+}}, {{%V[0-9]+}}, {{%VP[0-9]+}}
    desta_vector_ptr = (int256 __local *)dest_ptr;
  }
  {
    uint256 __local *dest_ptr = (uint256 __local *)desta_vector_ptr;
    uint256 res = {0};
    res =  v_convert_f8_to_u32_all_b(src_minifloat256, SW_RZ, res, pred, 0);
    *dest_ptr++ = res;
    // CHECK-DAG: convert.f8_143 all_lanes target_type=uint32 rz {{%A[0-9]+}}, {{%V[0-9]+}}, {{%SP[0-9]+}}
    desta_vector_ptr = (int256 __local *)dest_ptr;
  }
  {
    uint256 __local *dest_ptr = (uint256 __local *)desta_vector_ptr;
    uint256 res = {0};
    res =  v_convert_f8_to_u32_all_vb(src_minifloat256, SW_RZ, res, vpred256, 0);
    *dest_ptr++ = res;
    // CHECK-DAG: convert.f8_143 all_lanes target_type=uint32 rz {{%A[0-9]+}}, {{%V[0-9]+}}, {{%VP[0-9]+}}
    desta_vector_ptr = (int256 __local *)dest_ptr;
  }
  {
    short256 __local *dest_ptr = (short256 __local *)destd_vector_ptr;
    short256 res = {0};
    res =  v_convert_f8_to_i16_all_b(src_minifloat256, SW_RZ, res, pred, 0);
    *dest_ptr++ = res;
    // CHECK-DAG: convert.f8_143 all_lanes target_type=int16 rz {{%D[0-9]+}}, {{%V[0-9]+}}, {{%SP[0-9]+}}
    destd_vector_ptr = (int128 __local *)dest_ptr;
  }
  {
    short256 __local *dest_ptr = (short256 __local *)destd_vector_ptr;
    short256 res = {0};
    res =  v_convert_f8_to_i16_all_vb(src_minifloat256, SW_RZ, res, vpred256, 0);
    *dest_ptr++ = res;
    // CHECK-DAG: convert.f8_143 all_lanes target_type=int16 rz {{%D[0-9]+}}, {{%V[0-9]+}}, {{%VP[0-9]+}}
    destd_vector_ptr = (int128 __local *)dest_ptr;
  }
  {
    ushort256 __local *dest_ptr = (ushort256 __local *)destd_vector_ptr;
    ushort256 res = {0};
    res =  v_convert_f8_to_u16_all_b(src_minifloat256, SW_RZ, res, pred, 0);
    *dest_ptr++ = res;
    // CHECK-DAG: convert.f8_143 all_lanes target_type=uint16 rz {{%D[0-9]+}}, {{%V[0-9]+}}, {{%SP[0-9]+}}
    destd_vector_ptr = (int128 __local *)dest_ptr;
  }
  {
    ushort256 __local *dest_ptr = (ushort256 __local *)destd_vector_ptr;
    ushort256 res = {0};
    res =  v_convert_f8_to_u16_all_vb(src_minifloat256, SW_RZ, res, vpred256, 0);
    *dest_ptr++ = res;
    // CHECK-DAG: convert.f8_143 all_lanes target_type=uint16 rz {{%D[0-9]+}}, {{%V[0-9]+}}, {{%VP[0-9]+}}
    destd_vector_ptr = (int128 __local *)dest_ptr;
  }

  {
    float256 __local *dest_ptr = (float256 __local *)desta_vector_ptr;
    float256 res = {0};
    res =  v_convert_h8_to_f32_all_b(src_minihalf256, SW_RZ, res, pred, 0);
    *dest_ptr++ = res;
    // CHECK-DAG: convert.f8_152 all_lanes target_type=fp32 rz {{%A[0-9]+}}, {{%V[0-9]+}}, {{%SP[0-9]+}}
    desta_vector_ptr = (int256 __local *)dest_ptr;
  }
  {
    float256 __local *dest_ptr = (float256 __local *)desta_vector_ptr;
    float256 res = {0};
    res =  v_convert_h8_to_f32_all_vb(src_minihalf256, SW_RZ, res, vpred256, 0);
    *dest_ptr++ = res;
    // CHECK-DAG: convert.f8_152 all_lanes target_type=fp32 rz {{%A[0-9]+}}, {{%V[0-9]+}}, {{%VP[0-9]+}}
    desta_vector_ptr = (int256 __local *)dest_ptr;
  }
  {
    bfloat256 __local *dest_ptr = (bfloat256 __local *)destd_vector_ptr;
    bfloat256 res = {0};
    res =  v_convert_h8_to_bf16_all_b(src_minihalf256, SW_RZ, res, pred, 0);
    *dest_ptr++ = res;
    // CHECK-DAG: convert.f8_152 all_lanes target_type=bf16 rz {{%D[0-9]+}}, {{%V[0-9]+}}, {{%SP[0-9]+}}
    destd_vector_ptr = (int128 __local *)dest_ptr;
  }
  {
    bfloat256 __local *dest_ptr = (bfloat256 __local *)destd_vector_ptr;
    bfloat256 res = {0};
    res =  v_convert_h8_to_bf16_all_vb(src_minihalf256, SW_RZ, res, vpred256, 0);
    *dest_ptr++ = res;
    // CHECK-DAG: convert.f8_152 all_lanes target_type=bf16 rz {{%D[0-9]+}}, {{%V[0-9]+}}, {{%VP[0-9]+}}
    destd_vector_ptr = (int128 __local *)dest_ptr;
  }
  {
    half256 __local *dest_ptr = (half256 __local *)destd_vector_ptr;
    half256 res = {0};
    res =  v_convert_h8_to_f16_all_b(src_minihalf256, SW_RZ, res, pred, 0);
    *dest_ptr++ = res;
    // CHECK-DAG: convert.f8_152 all_lanes target_type=f16 rz {{%D[0-9]+}}, {{%V[0-9]+}}, {{%SP[0-9]+}}
    destd_vector_ptr = (int128 __local *)dest_ptr;
  }
  {
    half256 __local *dest_ptr = (half256 __local *)destd_vector_ptr;
    half256 res = {0};
    res =  v_convert_h8_to_f16_all_vb(src_minihalf256, SW_RZ, res, vpred256, 0);
    *dest_ptr++ = res;
    // CHECK-DAG: convert.f8_152 all_lanes target_type=f16 rz {{%D[0-9]+}}, {{%V[0-9]+}}, {{%VP[0-9]+}}
    destd_vector_ptr = (int128 __local *)dest_ptr;
  }
  {
    int256 __local *dest_ptr = (int256 __local *)desta_vector_ptr;
    int256 res = {0};
    res =  v_convert_h8_to_i32_all_b(src_minihalf256, SW_RZ, res, pred, 0);
    *dest_ptr++ = res;
    // CHECK-DAG: convert.f8_152 all_lanes target_type=int32 rz {{%A[0-9]+}}, {{%V[0-9]+}}, {{%SP[0-9]+}}
    desta_vector_ptr = (int256 __local *)dest_ptr;
  }
  {
    int256 __local *dest_ptr = (int256 __local *)desta_vector_ptr;
    int256 res = {0};
    res =  v_convert_h8_to_i32_all_vb(src_minihalf256, SW_RZ, res, vpred256, 0);
    *dest_ptr++ = res;
    // CHECK-DAG: convert.f8_152 all_lanes target_type=int32 rz {{%A[0-9]+}}, {{%V[0-9]+}}, {{%VP[0-9]+}}
    desta_vector_ptr = (int256 __local *)dest_ptr;
  }
  {
    uint256 __local *dest_ptr = (uint256 __local *)desta_vector_ptr;
    uint256 res = {0};
    res =  v_convert_h8_to_u32_all_b(src_minihalf256, SW_RZ, res, pred, 0);
    *dest_ptr++ = res;
    // CHECK-DAG: convert.f8_152 all_lanes target_type=uint32 rz {{%A[0-9]+}}, {{%V[0-9]+}}, {{%SP[0-9]+}}
    desta_vector_ptr = (int256 __local *)dest_ptr;
  }
  {
    uint256 __local *dest_ptr = (uint256 __local *)desta_vector_ptr;
    uint256 res = {0};
    res =  v_convert_h8_to_u32_all_vb(src_minihalf256, SW_RZ, res, vpred256, 0);
    *dest_ptr++ = res;
    // CHECK-DAG: convert.f8_152 all_lanes target_type=uint32 rz {{%A[0-9]+}}, {{%V[0-9]+}}, {{%VP[0-9]+}}
    desta_vector_ptr = (int256 __local *)dest_ptr;
  }
  {
    short256 __local *dest_ptr = (short256 __local *)destd_vector_ptr;
    short256 res = {0};
    res =  v_convert_h8_to_i16_all_b(src_minihalf256, SW_RZ, res, pred, 0);
    *dest_ptr++ = res;
    // CHECK-DAG: convert.f8_152 all_lanes target_type=int16 rz {{%D[0-9]+}}, {{%V[0-9]+}}, {{%SP[0-9]+}}
    destd_vector_ptr = (int128 __local *)dest_ptr;
  }
  {
    short256 __local *dest_ptr = (short256 __local *)destd_vector_ptr;
    short256 res = {0};
    res =  v_convert_h8_to_i16_all_vb(src_minihalf256, SW_RZ, res, vpred256, 0);
    *dest_ptr++ = res;
    // CHECK-DAG: convert.f8_152 all_lanes target_type=int16 rz {{%D[0-9]+}}, {{%V[0-9]+}}, {{%VP[0-9]+}}
    destd_vector_ptr = (int128 __local *)dest_ptr;
  }
  {
    ushort256 __local *dest_ptr = (ushort256 __local *)destd_vector_ptr;
    ushort256 res = {0};
    res =  v_convert_h8_to_u16_all_b(src_minihalf256, SW_RZ, res, pred, 0);
    *dest_ptr++ = res;
    // CHECK-DAG: convert.f8_152 all_lanes target_type=uint16 rz {{%D[0-9]+}}, {{%V[0-9]+}}, {{%SP[0-9]+}}
    destd_vector_ptr = (int128 __local *)dest_ptr;
  }
  {
    ushort256 __local *dest_ptr = (ushort256 __local *)destd_vector_ptr;
    ushort256 res = {0};
    res =  v_convert_h8_to_u16_all_vb(src_minihalf256, SW_RZ, res, vpred256, 0);
    *dest_ptr++ = res;
    // CHECK-DAG: convert.f8_152 all_lanes target_type=uint16 rz {{%D[0-9]+}}, {{%V[0-9]+}}, {{%VP[0-9]+}}
    destd_vector_ptr = (int128 __local *)dest_ptr;
  }

  {
    minifloat256 __local *dest_ptr = (minifloat256 __local *)destv_vector_ptr;
    minifloat256 res = {0};
    res =  v_convert_i32_to_f8_all_b(src_int256, SW_RZ, res, pred, 0);
    *dest_ptr++ = res;
    // CHECK-DAG: convert.i32 all_lanes target_type=f8_143 rz {{%V[0-9]+}}, {{%A[0-9]+}}, {{%SP[0-9]+}}
    destv_vector_ptr = (int64 __local *)dest_ptr;
  }
  {
    minifloat256 __local *dest_ptr = (minifloat256 __local *)destv_vector_ptr;
    minifloat256 res = {0};
    res =  v_convert_i32_to_f8_all_vb(src_int256, SW_RZ, res, vpred256, 0);
    *dest_ptr++ = res;
    // CHECK-DAG: convert.i32 all_lanes target_type=f8_143 rz {{%V[0-9]+}}, {{%A[0-9]+}}, {{%VP[0-9]+}}
    destv_vector_ptr = (int64 __local *)dest_ptr;
  }
  {
    minihalf256 __local *dest_ptr = (minihalf256 __local *)destv_vector_ptr;
    minihalf256 res = {0};
    res =  v_convert_i32_to_h8_all_b(src_int256, SW_RZ, res, pred, 0);
    *dest_ptr++ = res;
    // CHECK-DAG: convert.i32 all_lanes target_type=f8_152 rz {{%V[0-9]+}}, {{%A[0-9]+}}, {{%SP[0-9]+}}
    destv_vector_ptr = (int64 __local *)dest_ptr;
  }
  {
    minihalf256 __local *dest_ptr = (minihalf256 __local *)destv_vector_ptr;
    minihalf256 res = {0};
    res =  v_convert_i32_to_h8_all_vb(src_int256, SW_RZ, res, vpred256, 0);
    *dest_ptr++ = res;
    // CHECK-DAG: convert.i32 all_lanes target_type=f8_152 rz {{%V[0-9]+}}, {{%A[0-9]+}}, {{%VP[0-9]+}}
    destv_vector_ptr = (int64 __local *)dest_ptr;
  }
  {
    short128 __local *dest_ptr = (short128 __local *)destv_vector_ptr;
    short128 res = {0};
    res =  v_convert_i32_to_i16_all_b(src_int128, SW_RZ, res, pred, 0);
    *dest_ptr++ = res;
    // CHECK-DAG: convert.i32 all_lanes target_type=int16 rz {{%V[0-9]+}}, {{%D[0-9]+}}, {{%SP[0-9]+}}
    destv_vector_ptr = (int64 __local *)dest_ptr;
  }
  {
    short128 __local *dest_ptr = (short128 __local *)destv_vector_ptr;
    short128 res = {0};
    res =  v_convert_i32_to_i16_all_vb(src_int128, SW_RZ, res, vpred128, 0);
    *dest_ptr++ = res;
    // CHECK-DAG: convert.i32 all_lanes target_type=int16 rz {{%V[0-9]+}}, {{%D[0-9]+}}, {{%VP[0-9]+}}
    destv_vector_ptr = (int64 __local *)dest_ptr;
  }
  {
    ushort128 __local *dest_ptr = (ushort128 __local *)destv_vector_ptr;
    ushort128 res = {0};
    res =  v_convert_i32_to_u16_all_b(src_int128, SW_RZ, res, pred, 0);
    *dest_ptr++ = res;
    // CHECK-DAG: convert.i32 all_lanes target_type=uint16 rz {{%V[0-9]+}}, {{%D[0-9]+}}, {{%SP[0-9]+}}
    destv_vector_ptr = (int64 __local *)dest_ptr;
  }
  {
    ushort128 __local *dest_ptr = (ushort128 __local *)destv_vector_ptr;
    ushort128 res = {0};
    res =  v_convert_i32_to_u16_all_vb(src_int128, SW_RZ, res, vpred128, 0);
    *dest_ptr++ = res;
    // CHECK-DAG: convert.i32 all_lanes target_type=uint16 rz {{%V[0-9]+}}, {{%D[0-9]+}}, {{%VP[0-9]+}}
    destv_vector_ptr = (int64 __local *)dest_ptr;
  }
  {
    char256 __local *dest_ptr = (char256 __local *)destv_vector_ptr;
    char256 res = {0};
    res =  v_convert_i32_to_i8_all_b(src_int256, SW_RZ, res, pred, 0);
    *dest_ptr++ = res;
    // CHECK-DAG: convert.i32 all_lanes target_type=int8 rz {{%V[0-9]+}}, {{%A[0-9]+}}, {{%SP[0-9]+}}
    destv_vector_ptr = (int64 __local *)dest_ptr;
  }
  {
    char256 __local *dest_ptr = (char256 __local *)destv_vector_ptr;
    char256 res = {0};
    res =  v_convert_i32_to_i8_all_vb(src_int256, SW_RZ, res, vpred256, 0);
    *dest_ptr++ = res;
    // CHECK-DAG: convert.i32 all_lanes target_type=int8 rz {{%V[0-9]+}}, {{%A[0-9]+}}, {{%VP[0-9]+}}
    destv_vector_ptr = (int64 __local *)dest_ptr;
  }
  {
    uchar256 __local *dest_ptr = (uchar256 __local *)destv_vector_ptr;
    uchar256 res = {0};
    res =  v_convert_i32_to_u8_all_b(src_int256, SW_RZ, res, pred, 0);
    *dest_ptr++ = res;
    // CHECK-DAG: convert.i32 all_lanes target_type=uint8 rz {{%V[0-9]+}}, {{%A[0-9]+}}, {{%SP[0-9]+}}
    destv_vector_ptr = (int64 __local *)dest_ptr;
  }
  {
    uchar256 __local *dest_ptr = (uchar256 __local *)destv_vector_ptr;
    uchar256 res = {0};
    res =  v_convert_i32_to_u8_all_vb(src_int256, SW_RZ, res, vpred256, 0);
    *dest_ptr++ = res;
    // CHECK-DAG: convert.i32 all_lanes target_type=uint8 rz {{%V[0-9]+}}, {{%A[0-9]+}}, {{%VP[0-9]+}}
    destv_vector_ptr = (int64 __local *)dest_ptr;
  }

  {
    minifloat256 __local *dest_ptr = (minifloat256 __local *)destv_vector_ptr;
    minifloat256 res = {0};
    res =  v_convert_u32_to_f8_all_b(src_uint256, SW_RZ, res, pred, 0);
    *dest_ptr++ = res;
    // CHECK-DAG: convert.u32 all_lanes target_type=f8_143 rz {{%V[0-9]+}}, {{%A[0-9]+}}, {{%SP[0-9]+}}
    destv_vector_ptr = (int64 __local *)dest_ptr;
  }
  {
    minifloat256 __local *dest_ptr = (minifloat256 __local *)destv_vector_ptr;
    minifloat256 res = {0};
    res =  v_convert_u32_to_f8_all_vb(src_uint256, SW_RZ, res, vpred256, 0);
    *dest_ptr++ = res;
    // CHECK-DAG: convert.u32 all_lanes target_type=f8_143 rz {{%V[0-9]+}}, {{%A[0-9]+}}, {{%VP[0-9]+}}
    destv_vector_ptr = (int64 __local *)dest_ptr;
  }
  {
    minihalf256 __local *dest_ptr = (minihalf256 __local *)destv_vector_ptr;
    minihalf256 res = {0};
    res =  v_convert_u32_to_h8_all_b(src_uint256, SW_RZ, res, pred, 0);
    *dest_ptr++ = res;
    // CHECK-DAG: convert.u32 all_lanes target_type=f8_152 rz {{%V[0-9]+}}, {{%A[0-9]+}}, {{%SP[0-9]+}}
    destv_vector_ptr = (int64 __local *)dest_ptr;
  }
  {
    minihalf256 __local *dest_ptr = (minihalf256 __local *)destv_vector_ptr;
    minihalf256 res = {0};
    res =  v_convert_u32_to_h8_all_vb(src_uint256, SW_RZ, res, vpred256, 0);
    *dest_ptr++ = res;
    // CHECK-DAG: convert.u32 all_lanes target_type=f8_152 rz {{%V[0-9]+}}, {{%A[0-9]+}}, {{%VP[0-9]+}}
    destv_vector_ptr = (int64 __local *)dest_ptr;
  }
  {
    bfloat128 __local *dest_ptr = (bfloat128 __local *)destv_vector_ptr;
    bfloat128 res = {0};
    res =  v_convert_u32_to_bf16_all_b(src_uint128, SW_RZ, res, pred, 0);
    *dest_ptr++ = res;
    // CHECK-DAG: convert.u32 all_lanes target_type=bf16 rz {{%V[0-9]+}}, {{%D[0-9]+}}, {{%SP[0-9]+}}
    destv_vector_ptr = (int64 __local *)dest_ptr;
  }
  {
    bfloat128 __local *dest_ptr = (bfloat128 __local *)destv_vector_ptr;
    bfloat128 res = {0};
    res =  v_convert_u32_to_bf16_all_vb(src_uint128, SW_RZ, res, vpred128, 0);
    *dest_ptr++ = res;
    // CHECK-DAG: convert.u32 all_lanes target_type=bf16 rz {{%V[0-9]+}}, {{%D[0-9]+}}, {{%VP[0-9]+}}
    destv_vector_ptr = (int64 __local *)dest_ptr;
  }
  {
    half128 __local *dest_ptr = (half128 __local *)destv_vector_ptr;
    half128 res = {0};
    res =  v_convert_u32_to_f16_all_b(src_uint128, SW_RZ, res, pred, 0);
    *dest_ptr++ = res;
    // CHECK-DAG: convert.u32 all_lanes target_type=f16 rz {{%V[0-9]+}}, {{%D[0-9]+}}, {{%SP[0-9]+}}
    destv_vector_ptr = (int64 __local *)dest_ptr;
  }
  {
    half128 __local *dest_ptr = (half128 __local *)destv_vector_ptr;
    half128 res = {0};
    res =  v_convert_u32_to_f16_all_vb(src_uint128, SW_RZ, res, vpred128, 0);
    *dest_ptr++ = res;
    // CHECK-DAG: convert.u32 all_lanes target_type=f16 rz {{%V[0-9]+}}, {{%D[0-9]+}}, {{%VP[0-9]+}}
    destv_vector_ptr = (int64 __local *)dest_ptr;
  }
  {
    short128 __local *dest_ptr = (short128 __local *)destv_vector_ptr;
    short128 res = {0};
    res =  v_convert_u32_to_i16_all_b(src_uint128, SW_RZ, res, pred, 0);
    *dest_ptr++ = res;
    // CHECK-DAG: convert.u32 all_lanes target_type=int16 rz {{%V[0-9]+}}, {{%D[0-9]+}}, {{%SP[0-9]+}}
    destv_vector_ptr = (int64 __local *)dest_ptr;
  }
  {
    short128 __local *dest_ptr = (short128 __local *)destv_vector_ptr;
    short128 res = {0};
    res =  v_convert_u32_to_i16_all_vb(src_uint128, SW_RZ, res, vpred128, 0);
    *dest_ptr++ = res;
    // CHECK-DAG: convert.u32 all_lanes target_type=int16 rz {{%V[0-9]+}}, {{%D[0-9]+}}, {{%VP[0-9]+}}
    destv_vector_ptr = (int64 __local *)dest_ptr;
  }
  {
    ushort128 __local *dest_ptr = (ushort128 __local *)destv_vector_ptr;
    ushort128 res = {0};
    res =  v_convert_u32_to_u16_all_b(src_uint128, SW_RZ, res, pred, 0);
    *dest_ptr++ = res;
    // CHECK-DAG: convert.u32 all_lanes target_type=uint16 rz {{%V[0-9]+}}, {{%D[0-9]+}}, {{%SP[0-9]+}}
    destv_vector_ptr = (int64 __local *)dest_ptr;
  }
  {
    ushort128 __local *dest_ptr = (ushort128 __local *)destv_vector_ptr;
    ushort128 res = {0};
    res =  v_convert_u32_to_u16_all_vb(src_uint128, SW_RZ, res, vpred128, 0);
    *dest_ptr++ = res;
    // CHECK-DAG: convert.u32 all_lanes target_type=uint16 rz {{%V[0-9]+}}, {{%D[0-9]+}}, {{%VP[0-9]+}}
    destv_vector_ptr = (int64 __local *)dest_ptr;
  }
  {
    char256 __local *dest_ptr = (char256 __local *)destv_vector_ptr;
    char256 res = {0};
    res =  v_convert_u32_to_i8_all_b(src_uint256, SW_RZ, res, pred, 0);
    *dest_ptr++ = res;
    // CHECK-DAG: convert.u32 all_lanes target_type=int8 rz {{%V[0-9]+}}, {{%A[0-9]+}}, {{%SP[0-9]+}}
    destv_vector_ptr = (int64 __local *)dest_ptr;
  }
  {
    char256 __local *dest_ptr = (char256 __local *)destv_vector_ptr;
    char256 res = {0};
    res =  v_convert_u32_to_i8_all_vb(src_uint256, SW_RZ, res, vpred256, 0);
    *dest_ptr++ = res;
    // CHECK-DAG: convert.u32 all_lanes target_type=int8 rz {{%V[0-9]+}}, {{%A[0-9]+}}, {{%VP[0-9]+}}
    destv_vector_ptr = (int64 __local *)dest_ptr;
  }
  {
    uchar256 __local *dest_ptr = (uchar256 __local *)destv_vector_ptr;
    uchar256 res = {0};
    res =  v_convert_u32_to_u8_all_b(src_uint256, SW_RZ, res, pred, 0);
    *dest_ptr++ = res;
    // CHECK-DAG: convert.u32 all_lanes target_type=uint8 rz {{%V[0-9]+}}, {{%A[0-9]+}}, {{%SP[0-9]+}}
    destv_vector_ptr = (int64 __local *)dest_ptr;
  }
  {
    uchar256 __local *dest_ptr = (uchar256 __local *)destv_vector_ptr;
    uchar256 res = {0};
    res =  v_convert_u32_to_u8_all_vb(src_uint256, SW_RZ, res, vpred256, 0);
    *dest_ptr++ = res;
    // CHECK-DAG: convert.u32 all_lanes target_type=uint8 rz {{%V[0-9]+}}, {{%A[0-9]+}}, {{%VP[0-9]+}}
    destv_vector_ptr = (int64 __local *)dest_ptr;
  }

  {
    minifloat256 __local *dest_ptr = (minifloat256 __local *)destv_vector_ptr;
    minifloat256 res = {0};
    res =  v_convert_i16_to_f8_all_b(src_short256, SW_RZ, res, pred, 0);
    *dest_ptr++ = res;
    // CHECK-DAG: convert.i16 all_lanes target_type=f8_143 rz {{%V[0-9]+}}, {{%D[0-9]+}}, {{%SP[0-9]+}}
    destv_vector_ptr = (int64 __local *)dest_ptr;
  }
  {
    minifloat256 __local *dest_ptr = (minifloat256 __local *)destv_vector_ptr;
    minifloat256 res = {0};
    res =  v_convert_i16_to_f8_all_vb(src_short256, SW_RZ, res, vpred256, 0);
    *dest_ptr++ = res;
    // CHECK-DAG: convert.i16 all_lanes target_type=f8_143 rz {{%V[0-9]+}}, {{%D[0-9]+}}, {{%VP[0-9]+}}
    destv_vector_ptr = (int64 __local *)dest_ptr;
  }
  {
    minihalf256 __local *dest_ptr = (minihalf256 __local *)destv_vector_ptr;
    minihalf256 res = {0};
    res =  v_convert_i16_to_h8_all_b(src_short256, SW_RZ, res, pred, 0);
    *dest_ptr++ = res;
    // CHECK-DAG: convert.i16 all_lanes target_type=f8_152 rz {{%V[0-9]+}}, {{%D[0-9]+}}, {{%SP[0-9]+}}
    destv_vector_ptr = (int64 __local *)dest_ptr;
  }
  {
    minihalf256 __local *dest_ptr = (minihalf256 __local *)destv_vector_ptr;
    minihalf256 res = {0};
    res =  v_convert_i16_to_h8_all_vb(src_short256, SW_RZ, res, vpred256, 0);
    *dest_ptr++ = res;
    // CHECK-DAG: convert.i16 all_lanes target_type=f8_152 rz {{%V[0-9]+}}, {{%D[0-9]+}}, {{%VP[0-9]+}}
    destv_vector_ptr = (int64 __local *)dest_ptr;
  }
  {
    char256 __local *dest_ptr = (char256 __local *)destv_vector_ptr;
    char256 res = {0};
    res =  v_convert_i16_to_i8_all_b(src_short256, SW_RZ, res, pred, 0);
    *dest_ptr++ = res;
    // CHECK-DAG: convert.i16 all_lanes target_type=int8 rz {{%V[0-9]+}}, {{%D[0-9]+}}, {{%SP[0-9]+}}
    destv_vector_ptr = (int64 __local *)dest_ptr;
  }
  {
    char256 __local *dest_ptr = (char256 __local *)destv_vector_ptr;
    char256 res = {0};
    res =  v_convert_i16_to_i8_all_vb(src_short256, SW_RZ, res, vpred256, 0);
    *dest_ptr++ = res;
    // CHECK-DAG: convert.i16 all_lanes target_type=int8 rz {{%V[0-9]+}}, {{%D[0-9]+}}, {{%VP[0-9]+}}
    destv_vector_ptr = (int64 __local *)dest_ptr;
  }

  {
    float128 __local *dest_ptr = (float128 __local *)destd_vector_ptr;
    float128 res = {0};
    res =  v_convert_u16_to_f32_all_b(src_ushort128, SW_RZ, res, pred, 0);
    *dest_ptr++ = res;
    // CHECK-DAG: convert.u16 all_lanes target_type=fp32 rz {{%D[0-9]+}}, {{%V[0-9]+}}, {{%SP[0-9]+}}
    destd_vector_ptr = (int128 __local *)dest_ptr;
  }
  {
    float128 __local *dest_ptr = (float128 __local *)destd_vector_ptr;
    float128 res = {0};
    res =  v_convert_u16_to_f32_all_vb(src_ushort128, SW_RZ, res, vpred128, 0);
    *dest_ptr++ = res;
    // CHECK-DAG: convert.u16 all_lanes target_type=fp32 rz {{%D[0-9]+}}, {{%V[0-9]+}}, {{%VP[0-9]+}}
    destd_vector_ptr = (int128 __local *)dest_ptr;
  }
  {
    minifloat256 __local *dest_ptr = (minifloat256 __local *)destv_vector_ptr;
    minifloat256 res = {0};
    res =  v_convert_u16_to_f8_all_b(src_ushort256, SW_RZ, res, pred, 0);
    *dest_ptr++ = res;
    // CHECK-DAG: convert.u16 all_lanes target_type=f8_143 rz {{%V[0-9]+}}, {{%D[0-9]+}}, {{%SP[0-9]+}}
    destv_vector_ptr = (int64 __local *)dest_ptr;
  }
  {
    minifloat256 __local *dest_ptr = (minifloat256 __local *)destv_vector_ptr;
    minifloat256 res = {0};
    res =  v_convert_u16_to_f8_all_vb(src_ushort256, SW_RZ, res, vpred256, 0);
    *dest_ptr++ = res;
    // CHECK-DAG: convert.u16 all_lanes target_type=f8_143 rz {{%V[0-9]+}}, {{%D[0-9]+}}, {{%VP[0-9]+}}
    destv_vector_ptr = (int64 __local *)dest_ptr;
  }
  {
    minihalf256 __local *dest_ptr = (minihalf256 __local *)destv_vector_ptr;
    minihalf256 res = {0};
    res =  v_convert_u16_to_h8_all_b(src_ushort256, SW_RZ, res, pred, 0);
    *dest_ptr++ = res;
    // CHECK-DAG: convert.u16 all_lanes target_type=f8_152 rz {{%V[0-9]+}}, {{%D[0-9]+}}, {{%SP[0-9]+}}
    destv_vector_ptr = (int64 __local *)dest_ptr;
  }
  {
    minihalf256 __local *dest_ptr = (minihalf256 __local *)destv_vector_ptr;
    minihalf256 res = {0};
    res =  v_convert_u16_to_h8_all_vb(src_ushort256, SW_RZ, res, vpred256, 0);
    *dest_ptr++ = res;
    // CHECK-DAG: convert.u16 all_lanes target_type=f8_152 rz {{%V[0-9]+}}, {{%D[0-9]+}}, {{%VP[0-9]+}}
    destv_vector_ptr = (int64 __local *)dest_ptr;
  }
  {
    int128 __local *dest_ptr = (int128 __local *)destd_vector_ptr;
    int128 res = {0};
    res =  v_convert_u16_to_i32_all_b(src_ushort128, SW_RZ, res, pred, 0);
    *dest_ptr++ = res;
    // CHECK-DAG: convert.u16 all_lanes target_type=int32 rz {{%D[0-9]+}}, {{%V[0-9]+}}, {{%SP[0-9]+}}
    destd_vector_ptr = (int128 __local *)dest_ptr;
  }
  {
    int128 __local *dest_ptr = (int128 __local *)destd_vector_ptr;
    int128 res = {0};
    res =  v_convert_u16_to_i32_all_vb(src_ushort128, SW_RZ, res, vpred128, 0);
    *dest_ptr++ = res;
    // CHECK-DAG: convert.u16 all_lanes target_type=int32 rz {{%D[0-9]+}}, {{%V[0-9]+}}, {{%VP[0-9]+}}
    destd_vector_ptr = (int128 __local *)dest_ptr;
  }
  {
    uint128 __local *dest_ptr = (uint128 __local *)destd_vector_ptr;
    uint128 res = {0};
    res =  v_convert_u16_to_u32_all_b(src_ushort128, SW_RZ, res, pred, 0);
    *dest_ptr++ = res;
    // CHECK-DAG: convert.u16 all_lanes target_type=uint32 rz {{%D[0-9]+}}, {{%V[0-9]+}}, {{%SP[0-9]+}}
    destd_vector_ptr = (int128 __local *)dest_ptr;
  }
  {
    uint128 __local *dest_ptr = (uint128 __local *)destd_vector_ptr;
    uint128 res = {0};
    res =  v_convert_u16_to_u32_all_vb(src_ushort128, SW_RZ, res, vpred128, 0);
    *dest_ptr++ = res;
    // CHECK-DAG: convert.u16 all_lanes target_type=uint32 rz {{%D[0-9]+}}, {{%V[0-9]+}}, {{%VP[0-9]+}}
    destd_vector_ptr = (int128 __local *)dest_ptr;
  }
  {
    char256 __local *dest_ptr = (char256 __local *)destv_vector_ptr;
    char256 res = {0};
    res =  v_convert_u16_to_i8_all_b(src_ushort256, SW_RZ, res, pred, 0);
    *dest_ptr++ = res;
    // CHECK-DAG: convert.u16 all_lanes target_type=int8 rz {{%V[0-9]+}}, {{%D[0-9]+}}, {{%SP[0-9]+}}
    destv_vector_ptr = (int64 __local *)dest_ptr;
  }
  {
    char256 __local *dest_ptr = (char256 __local *)destv_vector_ptr;
    char256 res = {0};
    res =  v_convert_u16_to_i8_all_vb(src_ushort256, SW_RZ, res, vpred256, 0);
    *dest_ptr++ = res;
    // CHECK-DAG: convert.u16 all_lanes target_type=int8 rz {{%V[0-9]+}}, {{%D[0-9]+}}, {{%VP[0-9]+}}
    destv_vector_ptr = (int64 __local *)dest_ptr;
  }
  {
    uchar256 __local *dest_ptr = (uchar256 __local *)destv_vector_ptr;
    uchar256 res = {0};
    res =  v_convert_u16_to_u8_all_b(src_ushort256, SW_RZ, res, pred, 0);
    *dest_ptr++ = res;
    // CHECK-DAG: convert.u16 all_lanes target_type=uint8 rz {{%V[0-9]+}}, {{%D[0-9]+}}, {{%SP[0-9]+}}
    destv_vector_ptr = (int64 __local *)dest_ptr;
  }
  {
    uchar256 __local *dest_ptr = (uchar256 __local *)destv_vector_ptr;
    uchar256 res = {0};
    res =  v_convert_u16_to_u8_all_vb(src_ushort256, SW_RZ, res, vpred256, 0);
    *dest_ptr++ = res;
    // CHECK-DAG: convert.u16 all_lanes target_type=uint8 rz {{%V[0-9]+}}, {{%D[0-9]+}}, {{%VP[0-9]+}}
    destv_vector_ptr = (int64 __local *)dest_ptr;
  }

  {
    half256 __local *dest_ptr = (half256 __local *)destd_vector_ptr;
    half256 res = {0};
    res =  v_convert_i8_to_f16_all_b(src_char256, SW_RZ, res, pred, 0);
    *dest_ptr++ = res;
    // CHECK-DAG: convert.i8 all_lanes target_type=f16 rz {{%D[0-9]+}}, {{%V[0-9]+}}, {{%SP[0-9]+}}
    destd_vector_ptr = (int128 __local *)dest_ptr;
  }
  {
    half256 __local *dest_ptr = (half256 __local *)destd_vector_ptr;
    half256 res = {0};
    res =  v_convert_i8_to_f16_all_vb(src_char256, SW_RZ, res, vpred256, 0);
    *dest_ptr++ = res;
    // CHECK-DAG: convert.i8 all_lanes target_type=f16 rz {{%D[0-9]+}}, {{%V[0-9]+}}, {{%VP[0-9]+}}
    destd_vector_ptr = (int128 __local *)dest_ptr;
  }

  {
    half256 __local *dest_ptr = (half256 __local *)destd_vector_ptr;
    half256 res = {0};
    res =  v_convert_u8_to_f16_all_b(src_uchar256, SW_RZ, res, pred, 0);
    *dest_ptr++ = res;
    // CHECK-DAG: convert.u8 all_lanes target_type=f16 rz {{%D[0-9]+}}, {{%V[0-9]+}}, {{%SP[0-9]+}}
    destd_vector_ptr = (int128 __local *)dest_ptr;
  }
  {
    half256 __local *dest_ptr = (half256 __local *)destd_vector_ptr;
    half256 res = {0};
    res =  v_convert_u8_to_f16_all_vb(src_uchar256, SW_RZ, res, vpred256, 0);
    *dest_ptr++ = res;
    // CHECK-DAG: convert.u8 all_lanes target_type=f16 rz {{%D[0-9]+}}, {{%V[0-9]+}}, {{%VP[0-9]+}}
    destd_vector_ptr = (int128 __local *)dest_ptr;
  }
}
