// Black-Scholes prices of European call and put options (the PARSEC
// blackscholes workload): d1 = (ln(S/K) + (r + sigma^2/2) T) / (sigma sqrt T),
// d2 = d1 - sigma sqrt T, call = S N(d1) - K e^(-rT) N(d2), put from parity.
// N is the Abramowitz-Stegun polynomial for the normal CDF. A log, a sqrt,
// two reciprocals and three exps per vector of options. 262144 options.
//
// BENCH: arch gaudi2
// BENCH: in  f32 4096 64 range=10,100
// BENCH: in  f32 4096 64 range=10,100
// BENCH: in  f32 4096 64 range=0.1,2
// BENCH: out f32 4096 64
// BENCH: out f32 4096 64
// BENCH: index-space 64 2
// BENCH: scalar f32 0.02
// BENCH: scalar f32 0.3

#define ROWS 32

static float64 cnd(float64 d) {
  const float64 k = v_reciprocal_f32(1.0f + 0.2316419f * v_f32_abs_b(d));
  const float64 poly =
      k * (0.319381530f +
           k * (-0.356563782f + k * (1.781477937f + k * (-1.821255978f + k * 1.330274429f))));
  const float64 n = 1.0f - 0.3989422804f * v_exp_f32(-0.5f * d * d) * poly;
  return v_f32_sel_less_f32_b(d, 0.0f, 1.0f - n, n);
}

void main(tensor spot, tensor strike, tensor years, tensor call, tensor put,
          float rate, float vol) {
  const int5 start = get_index_space_offset();
  const int5 end = get_index_space_size() + start;
  int5 c = 0;

  for (int x = start[0] * 64; x < end[0] * 64; x += 64) {
    c[0] = x;
    for (int y = start[1] * ROWS; y < end[1] * ROWS; y++) {
      c[1] = y;
      const float64 s = v_f32_ld_tnsr_b(c, spot);
      const float64 k = v_f32_ld_tnsr_b(c, strike);
      const float64 t = v_f32_ld_tnsr_b(c, years);
      const float64 vsqrt = vol * v_sqrt_f32(t);
      const float64 d1 = (v_log_f32(s * v_reciprocal_f32(k)) + (rate + 0.5f * vol * vol) * t) *
                         v_reciprocal_f32(vsqrt);
      const float64 d2 = d1 - vsqrt;
      const float64 kdisc = k * v_exp_f32(-rate * t);
      const float64 c1 = cnd(d1);
      const float64 c2 = cnd(d2);
      const float64 cv = s * c1 - kdisc * c2;
      v_f32_st_tnsr(c, call, cv);
      v_f32_st_tnsr(c, put, cv - s + kdisc); // put-call parity
    }
  }
}
