// Many small members: each scales n rows of one vector column, n a kernel
// scalar (2), so the row loop's trip count is known only at run time. Most of
// a member's time is the code around that loop.
//
// BENCH: arch gaudi2
// BENCH: in  f32 4096 128
// BENCH: out f32 4096 128
// BENCH: index-space 64 64
// BENCH: scalar i32 2

void main(tensor a, tensor out, int n) {
  const int5 start = get_index_space_offset();
  const int5 end = get_index_space_size() + start;
  int5 c = 0;

  for (int x = start[0] * 64; x < end[0] * 64; x += 64) {
    c[0] = x;
    for (int y = start[1] * n; y < end[1] * n; y++) {
      c[1] = y;
      v_f32_st_tnsr(c, out, v_f32_ld_tnsr_b(c, a) * 0.5f + 1.0f);
    }
  }
}
