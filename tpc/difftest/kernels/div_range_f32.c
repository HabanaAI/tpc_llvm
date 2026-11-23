// Vector f32 division, 1.0f / v and w / v, over every exponent. Row r < 256
// gives v the exponent field r with the input's random sign and mantissa
// (r = 0: denormals, r = 255: inf and NaN); then rows with a zero mantissa,
// an all-ones mantissa, +-0 and tiny denormals. For v <= -2^126, 1/v
// underflows to +0 as with Habana's compiler, not to -0. Not on gaudi3: the
// fork rejects vector division there at -O0 and gives -0 above.
//
// DIFFTEST: arch gaudi gaudi2
// DIFFTEST: in  f32 1024 260
// DIFFTEST: in  f32 1024 260
// DIFFTEST: out f32 1024 260
// DIFFTEST: out f32 1024 260
// DIFFTEST: index-space 16 260

void main(tensor in, tensor num, tensor out_recip, tensor out_div) {
  const int5 start = get_index_space_offset();
  const int5 end = get_index_space_size() + start;
  int5 c = 0;
  for (int r = start[1]; r < end[1]; r++) {
    c[1] = r;
    unsigned keep = 0x807fffff, set = (unsigned)r << 23;
    if (r == 256) { keep = 0xff800000; set = 0; }
    if (r == 257) { keep = 0xffffffff; set = 0x007fffff; }
    if (r == 258) { keep = 0x80000000; set = 0; }
    if (r == 259) { keep = 0x800003ff; set = 0; }
    for (int x = start[0] * 64; x < end[0] * 64; x += 64) {
      c[0] = x;
      uint64 b = as_uint64(v_f32_ld_tnsr_b(c, in));
      float64 v = as_float64((b & keep) | set);
      float64 w = v_f32_ld_tnsr_b(c, num);
      v_f32_st_tnsr(c, out_recip, 1.0f / v);
      v_f32_st_tnsr(c, out_div, w / v);
    }
  }
}
