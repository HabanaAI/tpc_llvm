// The mixture-of-experts router of DeepSeek-V3, without its group-limited
// selection: scores = sigmoid(logits) over 256 experts, the 8 experts with the
// largest score + bias are chosen, and their scores, normalized to sum 1 and
// scaled by 2.5, are the routing weights; other experts get 0. Top-8 is eight
// rounds of a lane maximum over the 4 vectors of a token. 512 tokens, 8 per
// member.
//
// BENCH: arch gaudi2
// BENCH: in  f32 256 512 range=-6,6
// BENCH: in  f32 256 range=-0.5,0.5
// BENCH: out f32 256 512
// BENCH: index-space 1 64

#define TOPK 8
#define TOKENS 8 // tokens per member
#define SCALE 2.5f
#define TAKEN -3.0e38f

void main(tensor logits, tensor bias, tensor weights) {
  const int5 start = get_index_space_offset();
  const int5 end = get_index_space_size() + start;
  int5 c = 0;
  int5 b = 0;

  b[0] = 0;
  const float64 b0 = v_f32_ld_tnsr_b(b, bias);
  b[0] = 64;
  const float64 b1 = v_f32_ld_tnsr_b(b, bias);
  b[0] = 128;
  const float64 b2 = v_f32_ld_tnsr_b(b, bias);
  b[0] = 192;
  const float64 b3 = v_f32_ld_tnsr_b(b, bias);

  for (int t = start[1] * TOKENS; t < end[1] * TOKENS; t++) {
    c[1] = t;
    c[0] = 0;
    const float64 s0 = v_sigmoid_f32(v_f32_ld_tnsr_b(c, logits));
    c[0] = 64;
    const float64 s1 = v_sigmoid_f32(v_f32_ld_tnsr_b(c, logits));
    c[0] = 128;
    const float64 s2 = v_sigmoid_f32(v_f32_ld_tnsr_b(c, logits));
    c[0] = 192;
    const float64 s3 = v_sigmoid_f32(v_f32_ld_tnsr_b(c, logits));
    float64 k0 = s0 + b0, k1 = s1 + b1, k2 = s2 + b2, k3 = s3 + b3;
    float64 w0 = 0.0f, w1 = 0.0f, w2 = 0.0f, w3 = 0.0f;
    for (int k = 0; k < TOPK; k++) {
      const float64 m =
          v_f32_reduce_max(v_f32_max_b(v_f32_max_b(k0, k1), v_f32_max_b(k2, k3)));
      w0 = v_f32_sel_eq_f32_b(k0, m, s0, w0);
      w1 = v_f32_sel_eq_f32_b(k1, m, s1, w1);
      w2 = v_f32_sel_eq_f32_b(k2, m, s2, w2);
      w3 = v_f32_sel_eq_f32_b(k3, m, s3, w3);
      k0 = v_f32_sel_eq_f32_b(k0, m, TAKEN, k0);
      k1 = v_f32_sel_eq_f32_b(k1, m, TAKEN, k1);
      k2 = v_f32_sel_eq_f32_b(k2, m, TAKEN, k2);
      k3 = v_f32_sel_eq_f32_b(k3, m, TAKEN, k3);
    }
    const float64 norm = v_reciprocal_f32(v_f32_reduce_add(w0 + w1 + w2 + w3)) * SCALE;
    c[0] = 0;
    v_f32_st_tnsr(c, weights, w0 * norm);
    c[0] = 64;
    v_f32_st_tnsr(c, weights, w1 * norm);
    c[0] = 128;
    v_f32_st_tnsr(c, weights, w2 * norm);
    c[0] = 192;
    v_f32_st_tnsr(c, weights, w3 * norm);
  }
}
