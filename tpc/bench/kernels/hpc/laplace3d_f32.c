// 7-point Laplacian on a 256 x 64 x 32 grid: out = (sum of the six
// neighbours - 6 a) / h^2, zero outside the grid. Seven loads per vector;
// each member does 8 rows of 8 planes.
//
// BENCH: arch gaudi2
// BENCH: in  f32 256 64 32
// BENCH: out f32 256 64 32
// BENCH: index-space 4 8 4
// BENCH: scalar f32 100.0

#define ROWS 8
#define PLANES 8

void main(tensor a, tensor out, float inv_h2) {
  const int5 start = get_index_space_offset();
  const int5 end = get_index_space_size() + start;
  int5 c = 0;

  for (int z = start[2] * PLANES; z < end[2] * PLANES; z++) {
    for (int y = start[1] * ROWS; y < end[1] * ROWS; y++) {
      for (int x = start[0] * 64; x < end[0] * 64; x += 64) {
        c[0] = x;
        c[1] = y;
        c[2] = z;
        const float64 v = v_f32_ld_tnsr_b(c, a);
        int5 p = c;
        p[0] = x - 1;
        float64 sum = v_f32_ld_tnsr_b(p, a);
        p[0] = x + 1;
        sum += v_f32_ld_tnsr_b(p, a);
        p = c;
        p[1] = y - 1;
        sum += v_f32_ld_tnsr_b(p, a);
        p[1] = y + 1;
        sum += v_f32_ld_tnsr_b(p, a);
        p = c;
        p[2] = z - 1;
        sum += v_f32_ld_tnsr_b(p, a);
        p[2] = z + 1;
        sum += v_f32_ld_tnsr_b(p, a);
        v_f32_st_tnsr(c, out, (sum - 6.0f * v) * inv_h2);
      }
    }
  }
}
