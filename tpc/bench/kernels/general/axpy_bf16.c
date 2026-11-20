// out = alpha * x + y in bf16, alpha a kernel scalar: one MAC per vector.
// Memory-bound on the card, like add_f32, with half the bytes per element.
//
// BENCH: arch gaudi2
// BENCH: in  bf16 8192 128 range=-4,4
// BENCH: in  bf16 8192 128 range=-4,4
// BENCH: out bf16 8192 128
// BENCH: index-space 64 4
// BENCH: scalar f32 0.75

#define ROWS 32

void main(tensor xs, tensor ys, tensor out, float alpha) {
  const int5 start = get_index_space_offset();
  const int5 end = get_index_space_size() + start;
  const bfloat128 a = (bf16)alpha;
  int5 c = 0;

  for (int x = start[0] * 128; x < end[0] * 128; x += 128) {
    c[0] = x;
    for (int y = start[1] * ROWS; y < end[1] * ROWS; y++) {
      c[1] = y;
      bfloat128 vx = v_bf16_ld_tnsr_b(c, xs);
      bfloat128 vy = v_bf16_ld_tnsr_b(c, ys);
      v_bf16_st_tnsr(c, out, v_bf16_mac_b(vx, a, vy));
    }
  }
}
