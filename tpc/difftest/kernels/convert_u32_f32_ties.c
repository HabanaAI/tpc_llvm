// u32 to f32 for values above INT32_MAX, which Gaudi converts as
// f32((x >> 1) | (x & 1)) * 2. The first 64 values are made so that x >> 1
// lies exactly between two floats: only the low bit kept in the halved value
// rounds them as x itself rounds (the fork lost it).
//
// DIFFTEST: arch gaudi gaudi2
// DIFFTEST: in  u32 128
// DIFFTEST: out f32 128

void main(tensor in, tensor out) {
  int5 c = {0, 0, 0, 0, 0};
  uint64 v = v_u32_ld_tnsr_b(c, in);
  uint64 tie = (v & 0x7fffff00) | 0x80000081;
  v_f32_st_tnsr(c, out, convert_uint64_to_float64(tie, 0));
  c[0] = 64;
  v = v_u32_ld_tnsr_b(c, in);
  v_f32_st_tnsr(c, out, convert_uint64_to_float64(v, 0));
}
