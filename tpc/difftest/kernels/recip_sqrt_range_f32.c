// The header's v_reciprocal_f32, v_sqrt_f32, v_rsqrt_f32 and their fast
// versions over every exponent. Row r < 256 gives the input the exponent
// field r with its random sign and mantissa (r = 0: denormals, r = 255: inf
// and NaN); then rows with a zero mantissa, an all-ones mantissa, +-0 and tiny
// denormals. gaudi2 computes them with form_fp_num's special_func modes, the
// others without; the results must be the same as Habana's compiler's,
// including the fast versions' results for negative inputs.
//
// DIFFTEST: arch gaudi gaudi2 gaudi3
// DIFFTEST: in  f32 1024 260
// DIFFTEST: out f32 1024 260
// DIFFTEST: out f32 1024 260
// DIFFTEST: out f32 1024 260
// DIFFTEST: out f32 1024 260
// DIFFTEST: out f32 1024 260
// DIFFTEST: out f32 1024 260
// DIFFTEST: index-space 16 260

void main(tensor in, tensor rcp_fast, tensor sqrt_fast,
          tensor rsqrt_fast, tensor rcp, tensor sqrt, tensor rsqrt) {
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
      v_f32_st_tnsr(c, rcp_fast, v_reciprocal_fast_f32(v));
      v_f32_st_tnsr(c, sqrt_fast, v_sqrt_fast_f32(v));
      v_f32_st_tnsr(c, rsqrt_fast, v_rsqrt_fast_f32(v));
      v_f32_st_tnsr(c, rcp, v_reciprocal_f32(v));
      v_f32_st_tnsr(c, sqrt, v_sqrt_f32(v));
      v_f32_st_tnsr(c, rsqrt, v_rsqrt_f32(v));
    }
  }
}
