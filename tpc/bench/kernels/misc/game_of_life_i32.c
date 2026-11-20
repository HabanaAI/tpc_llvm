// One generation of Conway's Game of Life on a 1024 x 512 grid of 0/1 cells,
// dead outside the grid (the tensor's padding). Nine loads per vector, six
// of them not vector-aligned, and two integer compare-selects.
//
// BENCH: arch gaudi2
// BENCH: in  i32 1024 512 range=0,1
// BENCH: out i32 1024 512
// BENCH: index-space 16 16

#define ROWS 32

void main(tensor cells, tensor out) {
  const int5 start = get_index_space_offset();
  const int5 end = get_index_space_size() + start;
  int5 c = 0;

  for (int x = start[0] * 64; x < end[0] * 64; x += 64) {
    for (int y = start[1] * ROWS; y < end[1] * ROWS; y++) {
      int64 n = 0;
      int5 p = 0;
      for (int dy = -1; dy <= 1; dy++) {
        for (int dx = -1; dx <= 1; dx++) {
          if (dx == 0 && dy == 0)
            continue;
          p[0] = x + dx;
          p[1] = y + dy;
          n += v_i32_ld_tnsr_b(p, cells);
        }
      }
      c[0] = x;
      c[1] = y;
      const int64 alive = v_i32_ld_tnsr_b(c, cells);
      const int64 next = v_i32_sel_eq_i32_b(n, 3, 1, v_i32_sel_eq_i32_b(n, 2, alive, 0));
      v_i32_st_tnsr(c, out, next);
    }
  }
}
