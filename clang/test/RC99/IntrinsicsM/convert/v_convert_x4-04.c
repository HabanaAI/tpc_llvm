// RUN: %codegen -S -O1 -triple tpc-none-none -std=rc99 -bfloat16 -target-cpu gaudi2 %s -o - | FileCheck %s
// RUN: %codegen -S -O1 -triple tpc-none-none -std=rc99 -bfloat16 -target-cpu doron1 %s -o - | FileCheck %s

void main(int src, int destv, int destd, int desta, _Bool pred, int vpred)
{
  int256 __local *desta_vector_ptr = (int256 __local *)desta;

  uint256 __local *src_uint256_ptr = (uint256 __local *)desta_vector_ptr;

  bool256 __local *vpred256_ptr = (bool256 __local *)vpred;

  uint256 src_uint256 = *src_uint256_ptr;

  bool256 vpred256 = *vpred256_ptr;


  {
    float256 __local *dest_ptr = (float256 __local *)desta_vector_ptr;
    float256 res = {0};
    res = v_convert_u32_to_f32_x4_b(src_uint256, SW_RZ, res, pred, 0);
    *dest_ptr++ = res;
    // CHECK-DAG: convert.u32 target_type=fp32 rz x4 {{%A[0-9]+}}, {{%A[0-9]+}}, {{%SP[0-9]+}}
    desta_vector_ptr = (int256 __local *)dest_ptr;
  }
  {
    float256 __local *dest_ptr = (float256 __local *)desta_vector_ptr;
    float256 res = {0};
    res = v_convert_u32_to_f32_x4_vb(src_uint256, SW_RZ, res, vpred256, 0);
    *dest_ptr++ = res;
    // CHECK-DAG: convert.u32 target_type=fp32 rz x4 {{%A[0-9]+}}, {{%A[0-9]+}}, {{%VP[0-9]+}}
    desta_vector_ptr = (int256 __local *)dest_ptr;
  }
}
