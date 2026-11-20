// The fused residual add and RMSNorm that runs between the layers of
// DeepSeek-V3 and Qwen3 (after vLLM's fused_add_rms_norm): z = x + residual
// in f32, residual_out = z in bf16, and out = residual_out * rsqrt(mean(z^2) +
// eps) * w, rounded to bf16 once. DeepSeek-V3's hidden size, 7168; one token
// per member. The second pass recomputes z instead of reading residual_out
// back.
//
// BENCH: arch gaudi2
// BENCH: in  bf16 7168 64 range=-4,4
// BENCH: in  bf16 7168 64 range=-4,4
// BENCH: in  bf16 7168 range=-2,2
// BENCH: out bf16 7168 64
// BENCH: out bf16 7168 64
// BENCH: index-space 1 64
// BENCH: scalar f32 1e-6

void main(tensor x, tensor residual, tensor w, tensor out, tensor residual_out,
          float eps) {
  const int5 start = get_index_space_offset();
  const int5 end = get_index_space_size() + start;
  const int hidden = get_dim_size(x, 0);
  int5 c = 0;
  int5 g = 0;

  for (int t = start[1]; t < end[1]; t++) {
    c[1] = t;
    float64 acc = 0.0f;
    for (int i = 0; i < hidden; i += 128) {
      c[0] = i;
      float128 a = convert_bfloat128_to_float128(v_bf16_ld_tnsr_b(c, x), 0);
      float128 b = convert_bfloat128_to_float128(v_bf16_ld_tnsr_b(c, residual), 0);
      float128 z;
      z.v1 = a.v1 + b.v1;
      z.v2 = a.v2 + b.v2;
      acc += z.v1 * z.v1 + z.v2 * z.v2;
      v_bf16_st_tnsr(c, residual_out, convert_float128_to_bfloat128(z, SW_RHNE));
    }
    const float64 r = v_rsqrt_f32(v_f32_reduce_add(acc) / (float)hidden + eps);

    for (int i = 0; i < hidden; i += 128) {
      c[0] = g[0] = i;
      float128 a = convert_bfloat128_to_float128(v_bf16_ld_tnsr_b(c, x), 0);
      float128 b = convert_bfloat128_to_float128(v_bf16_ld_tnsr_b(c, residual), 0);
      float128 z;
      z.v1 = a.v1 + b.v1;
      z.v2 = a.v2 + b.v2;
      z = convert_bfloat128_to_float128(convert_float128_to_bfloat128(z, SW_RHNE), 0);
      float128 wv = convert_bfloat128_to_float128(v_bf16_ld_tnsr_b(g, w), 0);
      float128 o;
      o.v1 = z.v1 * r * wv.v1;
      o.v2 = z.v2 * r * wv.v2;
      v_bf16_st_tnsr(c, out, convert_float128_to_bfloat128(o, SW_RHNE));
    }
  }
}
