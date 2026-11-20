// Softmax over dim 0, rows of 1024 f32, three passes per row: the maximum,
// the sum of exp(x - max), then exp(x - max) / sum. Uses tpclib's v_exp_f32,
// lane reductions and reciprocal; dim 0 is never split across members.
//
// BENCH: arch gaudi2
// BENCH: in  f32 1024 512 range=-8,8
// BENCH: out f32 1024 512
// BENCH: index-space 1 64

#define ROWS 8

void main(tensor a, tensor out) {
  const int5 start = get_index_space_offset();
  const int5 end = get_index_space_size() + start;
  const int width = get_dim_size(a, 0);
  int5 c = 0;

  for (int y = start[1] * ROWS; y < end[1] * ROWS; y++) {
    c[1] = y;
    float64 m = -3.0e38f;
    for (int x = 0; x < width; x += 64) {
      c[0] = x;
      m = v_f32_max_b(m, v_f32_ld_tnsr_b(c, a));
    }
    m = v_f32_reduce_max(m);
    float64 s = 0.0f;
    for (int x = 0; x < width; x += 64) {
      c[0] = x;
      s += v_exp_f32(v_f32_ld_tnsr_b(c, a) - m);
    }
    const float64 inv = v_reciprocal_f32(v_f32_reduce_add(s));
    for (int x = 0; x < width; x += 64) {
      c[0] = x;
      v_f32_st_tnsr(c, out, v_exp_f32(v_f32_ld_tnsr_b(c, a) - m) * inv);
    }
  }
}
