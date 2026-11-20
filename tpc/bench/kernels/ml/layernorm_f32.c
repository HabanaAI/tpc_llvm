// LayerNorm over the hidden dimension, as in BERT, ViT and Whisper: the mean
// and variance in two passes, then (x - mean) * rsqrt(var + eps) * gamma +
// beta. Hidden size 1024 in f32; one token per member.
//
// BENCH: arch gaudi2
// BENCH: in  f32 1024 256 range=-4,4
// BENCH: in  f32 1024 range=0.5,1.5
// BENCH: in  f32 1024 range=-0.5,0.5
// BENCH: out f32 1024 256
// BENCH: index-space 1 256
// BENCH: scalar f32 1e-5

void main(tensor x, tensor gamma, tensor beta, tensor out, float eps) {
  const int5 start = get_index_space_offset();
  const int5 end = get_index_space_size() + start;
  const int hidden = get_dim_size(x, 0);
  int5 c = 0;
  int5 g = 0;

  for (int t = start[1]; t < end[1]; t++) {
    c[1] = t;
    float64 sum = 0.0f;
    for (int i = 0; i < hidden; i += 64) {
      c[0] = i;
      sum += v_f32_ld_tnsr_b(c, x);
    }
    const float64 mean = v_f32_reduce_add(sum) / (float)hidden;
    float64 sq = 0.0f;
    for (int i = 0; i < hidden; i += 64) {
      c[0] = i;
      const float64 d = v_f32_ld_tnsr_b(c, x) - mean;
      sq += d * d;
    }
    const float64 r = v_rsqrt_f32(v_f32_reduce_add(sq) / (float)hidden + eps);
    for (int i = 0; i < hidden; i += 64) {
      c[0] = g[0] = i;
      const float64 d = v_f32_ld_tnsr_b(c, x) - mean;
      v_f32_st_tnsr(c, out, d * r * v_f32_ld_tnsr_b(g, gamma) + v_f32_ld_tnsr_b(g, beta));
    }
  }
}
