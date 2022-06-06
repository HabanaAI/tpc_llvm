// RUN: %codegen -S -O1 -triple tpc-none-none -std=rc99 -bfloat16 -target-cpu gaudi2 %s -o - | FileCheck --check-prefixes=GAUDI2 %s
// RUN: %codegen -S -O1 -triple tpc-none-none -std=rc99 -bfloat16 -target-cpu doron1 %s -o - | FileCheck --check-prefixes=DORON1 %s

// GAUDI-2396

void main(int src, int destv, int destd, int desta, _Bool pred, int vpred)
{
  int128 __local *destd_vector_ptr = (int128 __local *)destd;

  uint128 __local *src_uint128_ptr = (uint128 __local *)destd_vector_ptr;

  bool128 __local *vpred128_ptr = (bool128 __local *)vpred;

  uint128 src_uint128 = *src_uint128_ptr;

  bool128 vpred128 = *vpred128_ptr;


  {
    float128 __local *dest_ptr = (float128 __local *)destd_vector_ptr;
    float128 res = {0};
    res = v_convert_u32_to_f32_x2_b(src_uint128, SW_RZ, res, pred, 0);
    *dest_ptr++ = res;
    // GAUDI2: convert.u32 x2 target_type=fp32 rz {{%D[0-9]+}}, {{%D[0-9]+}}, {{%SP[0-9]+}}
    // DORON1: convert.u32 x2  all_lanes target_type=fp32 rz {{%D[0-9]+}}, {{%D[0-9]+}}, {{%SP[0-9]+}}
    destd_vector_ptr = (int128 __local *)dest_ptr;
  }
  {
    float128 __local *dest_ptr = (float128 __local *)destd_vector_ptr;
    float128 res = {0};
    res = v_convert_u32_to_f32_x2_vb(src_uint128, SW_RZ, res, vpred128, 0);
    *dest_ptr++ = res;
    // GAUDI2: convert.u32 x2 target_type=fp32 rz {{%D[0-9]+}}, {{%D[0-9]+}}, {{%VP[0-9]+}}
    // DORON1: convert.u32 x2 all_lanes target_type=fp32 rz {{%D[0-9]+}}, {{%D[0-9]+}}, {{%VP[0-9]+}}
    destd_vector_ptr = (int128 __local *)dest_ptr;
  }
}
