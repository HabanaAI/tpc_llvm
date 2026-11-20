// One Jacobi sweep of the 2-D heat equation with the 5-point stencil:
// out = a + c (a[x-1] + a[x+1] + a[y-1] + a[y+1] - 4 a), zero outside the
// grid (the tensor's padding value). Five loads per vector, two of them not
// vector-aligned. 1024 x 512 grid.
//
// BENCH: arch gaudi2
// BENCH: in  f32 1024 512
// BENCH: out f32 1024 512
// BENCH: index-space 16 16
// BENCH: scalar f32 0.2

#define ROWS 32

void main(tensor a, tensor out, float coef) {
  const int5 start = get_index_space_offset();
  const int5 end = get_index_space_size() + start;
  int5 c = 0, w = 0, e = 0, n = 0, s = 0;

  for (int x = start[0] * 64; x < end[0] * 64; x += 64) {
    c[0] = n[0] = s[0] = x;
    w[0] = x - 1;
    e[0] = x + 1;
    for (int y = start[1] * ROWS; y < end[1] * ROWS; y++) {
      c[1] = w[1] = e[1] = y;
      n[1] = y - 1;
      s[1] = y + 1;
      const float64 v = v_f32_ld_tnsr_b(c, a);
      const float64 sum = v_f32_ld_tnsr_b(w, a) + v_f32_ld_tnsr_b(e, a) +
                          v_f32_ld_tnsr_b(n, a) + v_f32_ld_tnsr_b(s, a);
      v_f32_st_tnsr(c, out, v + coef * (sum - 4.0f * v));
    }
  }
}
