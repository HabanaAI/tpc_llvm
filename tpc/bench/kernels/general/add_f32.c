// Streaming elementwise add, f32: two loads, an add and a store per vector.
// Memory-bound on the card; measures loop overhead and load/store scheduling.
//
// BENCH: arch gaudi2
// BENCH: in  f32 4096 128
// BENCH: in  f32 4096 128
// BENCH: out f32 4096 128
// BENCH: index-space 64 4

#define ROWS 32 // rows of dim 1 per index-space member

void main(tensor a, tensor b, tensor out) {
  const int5 start = get_index_space_offset();
  const int5 end = get_index_space_size() + start;
  int5 c = 0;

  for (int x = start[0] * 64; x < end[0] * 64; x += 64) {
    c[0] = x;
    for (int y = start[1] * ROWS; y < end[1] * ROWS; y++) {
      c[1] = y;
      float64 va = v_f32_ld_tnsr_b(c, a);
      float64 vb = v_f32_ld_tnsr_b(c, b);
      v_f32_st_tnsr(c, out, va + vb);
    }
  }
}
