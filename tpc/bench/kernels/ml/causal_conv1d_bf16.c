// Causal depthwise convolution of width 4 followed by SiLU, the short
// convolution of Mamba layers and of Qwen3-Next's Gated DeltaNet layers
// (causal_conv1d_fn): out[c, t] = silu(sum_k w[c, k] x[c, t - 3 + k]), with x
// zero before t = 0. Channels are in the lanes; each member walks 256 time
// steps of 128 channels and keeps the last three inputs in registers.
//
// BENCH: arch gaudi2
// BENCH: in  bf16 4096 256 range=-4,4
// BENCH: in  bf16 4096 4 range=-1,1
// BENCH: out bf16 4096 256
// BENCH: index-space 32

void main(tensor x, tensor w, tensor out) {
  const int5 start = get_index_space_offset();
  const int5 end = get_index_space_size() + start;
  const int steps = get_dim_size(x, 1);
  int5 c = 0;
  int5 wc = 0;

  for (int ch = start[0] * 128; ch < end[0] * 128; ch += 128) {
    c[0] = wc[0] = ch;
    wc[1] = 0;
    const float128 w0 = convert_bfloat128_to_float128(v_bf16_ld_tnsr_b(wc, w), 0);
    wc[1] = 1;
    const float128 w1 = convert_bfloat128_to_float128(v_bf16_ld_tnsr_b(wc, w), 0);
    wc[1] = 2;
    const float128 w2 = convert_bfloat128_to_float128(v_bf16_ld_tnsr_b(wc, w), 0);
    wc[1] = 3;
    const float128 w3 = convert_bfloat128_to_float128(v_bf16_ld_tnsr_b(wc, w), 0);
    float128 x0 = {0}, x1 = {0}, x2 = {0}; // x[t - 3], x[t - 2], x[t - 1]
    for (int t = 0; t < steps; t++) {
      c[1] = t;
      float128 x3 = convert_bfloat128_to_float128(v_bf16_ld_tnsr_b(c, x), 0);
      float128 y;
      y.v1 = w0.v1 * x0.v1 + w1.v1 * x1.v1 + w2.v1 * x2.v1 + w3.v1 * x3.v1;
      y.v2 = w0.v2 * x0.v2 + w1.v2 * x1.v2 + w2.v2 * x2.v2 + w3.v2 * x3.v2;
      y.v1 = y.v1 * v_sigmoid_f32(y.v1);
      y.v2 = y.v2 * v_sigmoid_f32(y.v2);
      v_bf16_st_tnsr(c, out, convert_float128_to_bfloat128(y, SW_RHNE));
      x0 = x1;
      x1 = x2;
      x2 = x3;
    }
  }
}
