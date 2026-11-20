// DeepSeek-V3's FP8 activation quantization (act_quant in its inference
// code): each group of 128 channels of a token gets the scale
// s = max|x| / FP8_MAX and is stored as x / s in FP8 E4M3 (f8_143), whose
// largest value on Gaudi2 is 240. Two groups make one 256-lane FP8 vector.
// The 56 scales of a token are gathered into one vector by lane and stored
// together. Hidden size 7168; one token per member.
//
// BENCH: arch gaudi2
// BENCH: in  bf16   7168 64 range=-8,8
// BENCH: out f8_143 7168 64
// BENCH: out f32    64 64
// BENCH: index-space 1 64

#define FP8_MAX 240.0f

void main(tensor x, tensor q, tensor scales) {
  const int5 start = get_index_space_offset();
  const int5 end = get_index_space_size() + start;
  const int hidden = get_dim_size(x, 0);
  const uint64 lane = read_lane_id_4b_b();
  int5 c = 0;
  int5 s = 0;

  for (int t = start[1]; t < end[1]; t++) {
    c[1] = s[1] = t;
    float64 group_scales = 0.0f;
    for (int i = 0; i < hidden; i += 256) {
      c[0] = i;
      float128 a = convert_bfloat128_to_float128(v_bf16_ld_tnsr_b(c, x), 0);
      c[0] = i + 128;
      float128 b = convert_bfloat128_to_float128(v_bf16_ld_tnsr_b(c, x), 0);
      float64 amax_a = v_f32_reduce_max(v_f32_max_b(v_f32_abs_b(a.v1), v_f32_abs_b(a.v2)));
      float64 amax_b = v_f32_reduce_max(v_f32_max_b(v_f32_abs_b(b.v1), v_f32_abs_b(b.v2)));
      amax_a = v_f32_max_b(amax_a, 1e-4f);
      amax_b = v_f32_max_b(amax_b, 1e-4f);
      const float64 inv_a = v_reciprocal_f32(amax_a) * FP8_MAX;
      const float64 inv_b = v_reciprocal_f32(amax_b) * FP8_MAX;
      float256 y;
      y.v1 = a.v1 * inv_a;
      y.v2 = a.v2 * inv_a;
      y.v3 = b.v1 * inv_b;
      y.v4 = b.v2 * inv_b;
      c[0] = i;
      v_f8_st_tnsr(c, q, convert_float256_to_minifloat256(y, SW_RHNE));
      const unsigned group = i / 128;
      group_scales = v_f32_sel_eq_u32_b(lane, group, amax_a * (1.0f / FP8_MAX), group_scales);
      group_scales = v_f32_sel_eq_u32_b(lane, group + 1, amax_b * (1.0f / FP8_MAX), group_scales);
    }
    v_f32_st_tnsr(s, scales, group_scales);
  }
}
