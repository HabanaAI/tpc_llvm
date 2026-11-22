// v_sin_f32 and v_cos_f32 over magnitudes 2^-8 to 2^32: row r scales the
// random input by 2^(r - 8); then inf, NaN and large negative rows. Above
// 8192 the results used to be 0, and above 16777215 NaN; Habana's compiler
// computes them up to 196608 and gives 0 above.
//
// DIFFTEST: arch gaudi gaudi2 gaudi3
// DIFFTEST: in  f32 256 44
// DIFFTEST: out f32 256 44
// DIFFTEST: out f32 256 44
// DIFFTEST: index-space 4 44

void main(tensor in, tensor out_sin, tensor out_cos) {
  const int5 start = get_index_space_offset();
  const int5 end = get_index_space_size() + start;
  int5 c = 0;
  for (int r = start[1]; r < end[1]; r++) {
    c[1] = r;
    for (int x = start[0] * 64; x < end[0] * 64; x += 64) {
      c[0] = x;
      float64 v = v_f32_ld_tnsr_b(c, in);
      // 2^(r - 8) as bits: exponent field r - 8 + 127.
      float s = as_float((r - 8 + 127) << 23);
      v = v * s;
      if (r == 41) v = v * as_float(0x7f800000);   // inf (or NaN for 0)
      if (r == 42) v = as_float(0x7fc00000) + v;   // NaN
      if (r == 43) v = -v * 4096.0f;               // negative, large
      v_f32_st_tnsr(c, out_sin, v_sin_f32(v));
      v_f32_st_tnsr(c, out_cos, v_cos_f32(v));
    }
  }
}
