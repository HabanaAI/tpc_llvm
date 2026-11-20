// Running sum down each column: out[x, y] = a[x, 0] + ... + a[x, y]. A
// loop-carried value that is also stored every iteration.
//
// BENCH: arch gaudi2
// BENCH: in  f32 2048 256
// BENCH: out f32 2048 256
// BENCH: index-space 32

void main(tensor a, tensor out) {
  const int5 start = get_index_space_offset();
  const int5 end = get_index_space_size() + start;
  const int rows = get_dim_size(a, 1);
  int5 c = 0;

  for (int x = start[0] * 64; x < end[0] * 64; x += 64) {
    c[0] = x;
    float64 run = 0.0f;
    for (int y = 0; y < rows; y++) {
      c[1] = y;
      run = run + v_f32_ld_tnsr_b(c, a);
      v_f32_st_tnsr(c, out, run);
    }
  }
}
