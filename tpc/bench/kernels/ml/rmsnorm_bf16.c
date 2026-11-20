// RMSNorm over the hidden dimension: out = y * rsqrt(mean(y^2) + eps) * gamma.
// y is [hidden, tokens] in bf16, one token per index-space member. Products
// accumulate in f32 (mac_acc32) and are rounded to bf16 once, at the store.
// Pass 1 is a loop-carried f32 accumulator pair; pass 2 multiplies by gamma
// and the scale.
//
// BENCH: arch gaudi2
// BENCH: in  bf16 4096 128 range=-4,4
// BENCH: in  bf16 4096 range=-2,2
// BENCH: out bf16 4096 128
// BENCH: index-space 1 128
// BENCH: scalar f32 1e-6

void main(tensor y, tensor gamma, tensor out, float eps) {
  const int5 start = get_index_space_offset();
  const int5 end = get_index_space_size() + start;
  const int hidden = get_dim_size(y, 0);
  int5 c = 0;
  int5 g = 0;

  for (int t = start[1]; t < end[1]; t++) {
    c[1] = t;

    float128 acc = {0};
    for (int x = 0; x < hidden; x += 128) {
      c[0] = x;
      bfloat128 v = v_bf16_ld_tnsr_b(c, y);
      acc = v_bf16_mac_acc32_b(v, v, acc);
    }
    float64 sum = v_f32_reduce_add(acc.v1 + acc.v2);
    float64 r = v_rsqrt_f32(sum / (float)hidden + eps);

    for (int x = 0; x < hidden; x += 128) {
      c[0] = g[0] = x;
      bfloat128 v = v_bf16_ld_tnsr_b(c, y);
      bfloat128 w = v_bf16_ld_tnsr_b(g, gamma);
      float128 p = v_bf16_mac_acc32_b(v, w, (float128){0});
      p.v1 *= r;
      p.v2 *= r;
      v_bf16_st_tnsr(c, out, convert_float128_to_bfloat128(p, 0));
    }
  }
}
