// GELU, tanh form: 0.5 x (1 + tanh(sqrt(2/pi) (x + 0.044715 x^3))).
// Compute-bound: a cubic and tpclib's v_tanh_f32 per vector.
//
// BENCH: arch gaudi2
// BENCH: in  f32 4096 128 range=-6,6
// BENCH: out f32 4096 128
// BENCH: index-space 64 4

#define ROWS 32

void main(tensor a, tensor out) {
  const int5 start = get_index_space_offset();
  const int5 end = get_index_space_size() + start;
  int5 c = 0;

  for (int x = start[0] * 64; x < end[0] * 64; x += 64) {
    c[0] = x;
    for (int y = start[1] * ROWS; y < end[1] * ROWS; y++) {
      c[1] = y;
      float64 v = v_f32_ld_tnsr_b(c, a);
      float64 u = (v + 0.044715f * v * v * v) * 0.7978845608f;
      v_f32_st_tnsr(c, out, 0.5f * v * (1.0f + v_tanh_f32(u)));
    }
  }
}
