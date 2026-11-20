// The Mandelbrot set by escape time: for each pixel of a 512 x 256 view of
// [-2, 1] x [-1.2, 1.2], iterate z = z^2 + c 64 times and count the
// iterations with |z| <= 2. Every lane runs all 64 iterations; once a lane has
// escaped it stops counting. Pixel coordinates come from the lane id.
//
// BENCH: arch gaudi2
// BENCH: out f32 512 256
// BENCH: index-space 8 8

#define ROWS 32
#define ITERS 64

void main(tensor out) {
  const int5 start = get_index_space_offset();
  const int5 end = get_index_space_size() + start;
  const float64 lane = convert_int64_to_float64((int64)read_lane_id_4b_b(), SW_RHNE);
  int5 c = 0;

  for (int x = start[0] * 64; x < end[0] * 64; x += 64) {
    c[0] = x;
    const float64 cre = (lane + (float)x) * (3.0f / 512.0f) - 2.0f;
    for (int y = start[1] * ROWS; y < end[1] * ROWS; y++) {
      c[1] = y;
      const float cim = (float)y * (2.4f / 256.0f) - 1.2f;
      float64 zr = 0.0f, zi = 0.0f, count = 0.0f;
      for (int i = 0; i < ITERS; i++) {
        const float64 zr2 = zr * zr, zi2 = zi * zi;
        count += v_f32_sel_leq_f32_b(zr2 + zi2, 4.0f, 1.0f, 0.0f);
        zi = 2.0f * zr * zi + cim;
        zr = zr2 - zi2 + cre;
      }
      v_f32_st_tnsr(c, out, count);
    }
  }
}
