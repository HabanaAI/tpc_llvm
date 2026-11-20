// Column sums: out[x] = a[x, 0] + ... + a[x, 255], one loop-carried vector
// accumulator per member (a chain of dependent adds).
//
// BENCH: arch gaudi2
// BENCH: in  f32 2048 256
// BENCH: out f32 2048 1
// BENCH: index-space 32

void main(tensor a, tensor out) {
  const int5 start = get_index_space_offset();
  const int5 end = get_index_space_size() + start;
  const int rows = get_dim_size(a, 1);
  int5 i = 0;
  int5 o = 0;

  for (int x = start[0] * 64; x < end[0] * 64; x += 64) {
    i[0] = o[0] = x;
    float64 sum = 0.0f;
    for (int y = 0; y < rows; y++) {
      i[1] = y;
      sum += v_f32_ld_tnsr_b(i, a);
    }
    v_f32_st_tnsr(o, out, sum);
  }
}
