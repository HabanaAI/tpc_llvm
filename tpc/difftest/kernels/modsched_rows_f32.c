// Loops the MachinePipeliner modulo-schedules (-tpc-modulo-schedule), with
// trip counts that run the pipelined loop and leave iterations to the
// original loop: a running sum over n rows (n = 37 makes 9 unrolled
// iterations and one more), and a second pass over the rows that stores what
// it computes from the sum.
//
// DIFFTEST: arch gaudi2
// DIFFTEST: in  f32 128 37
// DIFFTEST: out f32 128 37
// DIFFTEST: out f32 128 1
// DIFFTEST: index-space 2 1
// DIFFTEST: scalar i32 37

void main(tensor a, tensor out, tensor sums, int n) {
  const int5 start = get_index_space_offset();
  const int5 end = get_index_space_size() + start;
  int5 c = 0;
  int5 o = 0;

  for (int x = start[0] * 64; x < end[0] * 64; x += 64) {
    c[0] = o[0] = x;
    float64 s = 0.0f;
    for (int y = 0; y < n; y++) {
      c[1] = y;
      const float64 v = v_f32_ld_tnsr_b(c, a);
      s += v * v * 0.25f + v * 0.5f;
    }
    v_f32_st_tnsr(o, sums, s);
    const float64 m = s * (1.0f / 37.0f);
    for (int y = 0; y < n; y++) {
      c[1] = y;
      const float64 v = v_f32_ld_tnsr_b(c, a);
      const float64 d = v - m;
      v_f32_st_tnsr(c, out, d * d * 0.5f + d - 1.0f);
    }
  }
}
