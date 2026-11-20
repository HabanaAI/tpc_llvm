// f32 -> bf16 with round to nearest even: two f32 loads, one conversion and
// one bf16 store per 128 elements.
//
// BENCH: arch gaudi2
// BENCH: in  f32  4096 128 range=-1000,1000
// BENCH: out bf16 4096 128
// BENCH: index-space 32 4

#define ROWS 32

void main(tensor a, tensor out) {
  const int5 start = get_index_space_offset();
  const int5 end = get_index_space_size() + start;
  int5 lo = 0;
  int5 hi = 0;

  for (int x = start[0] * 128; x < end[0] * 128; x += 128) {
    lo[0] = x;
    hi[0] = x + 64;
    for (int y = start[1] * ROWS; y < end[1] * ROWS; y++) {
      lo[1] = hi[1] = y;
      float128 pair = {v_f32_ld_tnsr_b(lo, a), v_f32_ld_tnsr_b(hi, a)};
      v_bf16_st_tnsr(lo, out, convert_float128_to_bfloat128(pair, SW_RHNE));
    }
  }
}
