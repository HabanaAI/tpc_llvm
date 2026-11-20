// One radix-4 decimation-in-time butterfly stage of 512 batched 1024-point
// complex FFTs, in split real/imaginary planes: x0..x3 = x[j + q 256],
// y_q = w_q x_q, then X0 = (y0 + y2) + (y1 + y3), X2 = (y0 + y2) - (y1 + y3),
// X1 = (y0 - y2) - i (y1 - y3), X3 = (y0 - y2) + i (y1 - y3). Twiddles are
// loaded once per member; eight loads, eight stores and 34 flops per
// butterfly.
//
// BENCH: arch gaudi2
// BENCH: in  f32 1024 512
// BENCH: in  f32 1024 512
// BENCH: in  f32 256 6 range=-1,1
// BENCH: out f32 1024 512
// BENCH: out f32 1024 512
// BENCH: index-space 4 16

#define ROWS 32
#define QUARTER 256

void main(tensor re, tensor im, tensor twiddles, tensor out_re, tensor out_im) {
  const int5 start = get_index_space_offset();
  const int5 end = get_index_space_size() + start;
  int5 c = 0;
  int5 tw = 0;

  for (int j = start[0] * 64; j < end[0] * 64; j += 64) {
    tw[0] = j;
    tw[1] = 0;
    const float64 w1r = v_f32_ld_tnsr_b(tw, twiddles);
    tw[1] = 1;
    const float64 w1i = v_f32_ld_tnsr_b(tw, twiddles);
    tw[1] = 2;
    const float64 w2r = v_f32_ld_tnsr_b(tw, twiddles);
    tw[1] = 3;
    const float64 w2i = v_f32_ld_tnsr_b(tw, twiddles);
    tw[1] = 4;
    const float64 w3r = v_f32_ld_tnsr_b(tw, twiddles);
    tw[1] = 5;
    const float64 w3i = v_f32_ld_tnsr_b(tw, twiddles);
    for (int y = start[1] * ROWS; y < end[1] * ROWS; y++) {
      c[1] = y;
      c[0] = j;
      const float64 x0r = v_f32_ld_tnsr_b(c, re), x0i = v_f32_ld_tnsr_b(c, im);
      c[0] = j + QUARTER;
      const float64 x1r = v_f32_ld_tnsr_b(c, re), x1i = v_f32_ld_tnsr_b(c, im);
      c[0] = j + 2 * QUARTER;
      const float64 x2r = v_f32_ld_tnsr_b(c, re), x2i = v_f32_ld_tnsr_b(c, im);
      c[0] = j + 3 * QUARTER;
      const float64 x3r = v_f32_ld_tnsr_b(c, re), x3i = v_f32_ld_tnsr_b(c, im);
      const float64 y1r = w1r * x1r - w1i * x1i, y1i = w1r * x1i + w1i * x1r;
      const float64 y2r = w2r * x2r - w2i * x2i, y2i = w2r * x2i + w2i * x2r;
      const float64 y3r = w3r * x3r - w3i * x3i, y3i = w3r * x3i + w3i * x3r;
      const float64 a0r = x0r + y2r, a0i = x0i + y2i;
      const float64 a1r = x0r - y2r, a1i = x0i - y2i;
      const float64 a2r = y1r + y3r, a2i = y1i + y3i;
      const float64 a3r = y1r - y3r, a3i = y1i - y3i;
      c[0] = j;
      v_f32_st_tnsr(c, out_re, a0r + a2r);
      v_f32_st_tnsr(c, out_im, a0i + a2i);
      c[0] = j + QUARTER;
      v_f32_st_tnsr(c, out_re, a1r + a3i);
      v_f32_st_tnsr(c, out_im, a1i - a3r);
      c[0] = j + 2 * QUARTER;
      v_f32_st_tnsr(c, out_re, a0r - a2r);
      v_f32_st_tnsr(c, out_im, a0i - a2i);
      c[0] = j + 3 * QUARTER;
      v_f32_st_tnsr(c, out_re, a1r - a3i);
      v_f32_st_tnsr(c, out_im, a1i + a3r);
    }
  }
}
