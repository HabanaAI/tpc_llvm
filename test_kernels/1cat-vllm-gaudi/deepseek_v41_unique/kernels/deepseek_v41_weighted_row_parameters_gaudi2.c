// SPDX-License-Identifier: Apache-2.0
// Compute C1's five row parameters once, before feature-parallel emission.
#pragma clang fp contract(off)
#define FLASHINFER_NORM_USE_LOOKUP_RSQRT
#include "../../flashinfer_gaudi/kernels/norm_math_gaudi2.h"
static inline float64 round_bf16(float64 value) {
    float64_pair_t pair;
    pair.v1 = value;
    pair.v2 = value;
    return v_convert_bf16_to_f32_all_b(v_convert_f32_to_bf16_all_b(pair)).v1;
}

static inline float64 reciprocal_without_lookup(float64 value) {
    float64 estimate = as_float64((int64)0x7ef311c3 - as_int64(value));
    estimate = estimate * (2.0f - value * estimate);
    estimate = estimate * (2.0f - value * estimate);
    estimate = estimate * (2.0f - value * estimate);
    return v_f32_sel_eq_f32_b(value, as_float64((int64)0x7f800000), 0.0f, estimate);
}

static inline float64 row_max_without_lookup(float64 value) {
    value = v_f32_max_b(value, v_f32_mov_dual_group_all_b(
        value, 0xffffffff, 1, 0, 3, 2, MkWrA(3, 3, 3, 3), 0));
    value = v_f32_max_b(value, v_f32_mov_dual_group_all_b(
        value, 0xffffffff, 2, 3, 0, 1, MkWrA(3, 3, 3, 3), 0));
    value = v_f32_max_b(value, v_f32_mov_group_b(value, 0xffffffff, 63, 0));
    float64 result = 0;
    #pragma loop_unroll(8)
    for (int lane = 0; lane < 8; ++lane) {
        result = v_f32_max_b(result,
            v_f32_shuffle_b(value, (uchar256)(0x80 | lane), 0, value));
    }
    return result;
}

void main(tensor statistics,tensor parameters,float epsilon,float inverse_width) {
 const int5 begin=get_index_space_offset(),end=begin+get_index_space_size();
 for(int row=begin[0];row<end[0];++row) {
  const float64 squares=v_f32_ld_tnsr_b((int5){0,0,row},statistics);
  const float64 weighted=v_f32_ld_tnsr_b((int5){0,1,row},statistics);
  const float64 rrms=positive_rsqrt(row_sum(squares)*inverse_width+epsilon);
  const float64 maximum=round_bf16(row_max_without_lookup(weighted)*rrms);
  const float64 raw_scale=round_bf16(maximum*(float)(bf16)(1.0f/240.0f));
  const float64 scale=round_bf16(raw_scale+(float)(bf16)(1.0e-8f/240.0f));
  const float64 inverse=round_bf16(reciprocal_without_lookup(scale));
  const uint64 bits=as_uint64(maximum);
  int64 power=convert_uint64_to_int64(bits>>23,0)-134;
  power+=v_i32_sel_grt_u32_b(bits&0x7fffff,0x700000,1,0);
  power=v_i32_sel_eq_f32_b(maximum,0.0f,0,power);
  const float64 dense_scale=as_float64((power+127)<<23);
  const float64 dense_inverse=as_float64((127-power)<<23);
  v_f32_st_tnsr_partial((int5){0,row},parameters,rrms,0,0);
  v_f32_st_tnsr_partial((int5){1,row},parameters,inverse,0,0);
  v_f32_st_tnsr_partial((int5){2,row},parameters,scale,0,0);
  v_f32_st_tnsr_partial((int5){3,row},parameters,dense_inverse,0,0);
  v_f32_st_tnsr_partial((int5){4,row},parameters,dense_scale,0,0);
 }
}
