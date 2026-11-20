// Elementwise select: out = a > b ? a * 0.5 : b - 1, one compare-select per
// vector on top of the two loads and the store.
//
// BENCH: arch gaudi2
// BENCH: in  f32 4096 128
// BENCH: in  f32 4096 128
// BENCH: out f32 4096 128
// BENCH: index-space 64 4

#define ROWS 32

void main(tensor a, tensor b, tensor out) {
  const int5 start = get_index_space_offset();
  const int5 end = get_index_space_size() + start;
  int5 c = 0;

  for (int x = start[0] * 64; x < end[0] * 64; x += 64) {
    c[0] = x;
    for (int y = start[1] * ROWS; y < end[1] * ROWS; y++) {
      c[1] = y;
      const float64 va = v_f32_ld_tnsr_b(c, a);
      const float64 vb = v_f32_ld_tnsr_b(c, b);
      v_f32_st_tnsr(c, out, v_f32_sel_grt_f32_b(va, vb, va * 0.5f, vb - 1.0f));
    }
  }
}
