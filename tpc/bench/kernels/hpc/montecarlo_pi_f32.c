// Monte Carlo estimate of pi with a xorshift32 generator per lane: draw 256
// points of the unit square per lane and count those inside the quarter
// circle; out = 4 * hits / points. Integer shifts and xors, two int-to-float
// conversions and a compare-select per point.
//
// BENCH: arch gaudi2
// BENCH: in  u32 4096 range=1,4294967295
// BENCH: out f32 4096
// BENCH: index-space 64
// BENCH: scalar i32 256

#define UNIT (1.0f / 16777216.0f) // 2^-24: the top 24 bits as a fraction

void main(tensor seeds, tensor out, int points) {
  const int5 start = get_index_space_offset();
  const int5 end = get_index_space_size() + start;
  int5 c = 0;

  for (int i = start[0] * 64; i < end[0] * 64; i += 64) {
    c[0] = i;
    uint64 s = v_u32_ld_tnsr_b(c, seeds);
    float64 hits = 0.0f;
    for (int p = 0; p < points; p++) {
      s ^= s << 13;
      s ^= s >> 17;
      s ^= s << 5;
      const float64 px = convert_int64_to_float64((int64)(s >> 8), SW_RHNE) * UNIT;
      s ^= s << 13;
      s ^= s >> 17;
      s ^= s << 5;
      const float64 py = convert_int64_to_float64((int64)(s >> 8), SW_RHNE) * UNIT;
      hits += v_f32_sel_less_f32_b(px * px + py * py, 1.0f, 1.0f, 0.0f);
    }
    v_f32_st_tnsr(c, out, hits * (4.0f / (float)points));
  }
}
