// Vector local memory: stage a column of 8 vectors in VLM, then combine each
// row with its mirror, out[y] = a[y] - a[7 - y], for 8 planes per member.
//
// BENCH: arch gaudi2
// BENCH: in  f32 4096 8 16
// BENCH: out f32 4096 8 16
// BENCH: index-space 64 1 2

#define PLANES 8
#define VEC_BYTES 256

void main(tensor a, tensor out) {
  const int5 start = get_index_space_offset();
  const int5 end = get_index_space_size() + start;
  int5 c = 0;

  for (int z = start[2] * PLANES; z < end[2] * PLANES; z++) {
    c[2] = z;
    for (int x = start[0] * 64; x < end[0] * 64; x += 64) {
      c[0] = x;
      for (int y = 0; y < 8; y++) {
        c[1] = y;
        v_f32_st_l_v(y * VEC_BYTES, v_f32_ld_tnsr_b(c, a));
      }
      for (int y = 0; y < 8; y++) {
        c[1] = y;
        float64 mine = v_f32_ld_l_v_b(y * VEC_BYTES);
        float64 mirror = v_f32_ld_l_v_b((7 - y) * VEC_BYTES);
        v_f32_st_tnsr(c, out, mine - mirror);
      }
    }
  }
}
