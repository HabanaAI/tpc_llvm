// A select between two 128-lane f32 values (register pairs) under a scalar
// condition: each half must come from the same half of the chosen value. Here
// the select comes from the unbranch pass, which predicates both sides of the
// if and selects between their accumulators.
//
// DIFFTEST: arch gaudi2
// DIFFTEST: in  bf16 128 8
// DIFFTEST: in  bf16 128 8
// DIFFTEST: out f32 128 8
// DIFFTEST: index-space 1 8

void main(tensor x, tensor w, tensor out) {
  const int5 start = get_index_space_offset();
  const int5 end = get_index_space_size() + start;
  int5 i = 0;
  for (int y = start[1]; y < end[1]; y++) {
    i[1] = y;
    const bfloat128 a = v_bf16_ld_tnsr_b(i, x);
    const bfloat128 b = v_bf16_ld_tnsr_b(i, w);
    float128 acc = {0};
    for (int k = 0; k < 4; k++) {
      if (y & 1)
        acc = v_bf16_mac_acc32_b(a, b, acc, 0);
      else
        acc = v_bf16_mac_acc32_b(a, a, acc, 0);
    }
    i[0] = 0;
    v_f32_st_tnsr(i, out, acc.v1);
    i[0] = 64;
    v_f32_st_tnsr(i, out, acc.v2);
    i[0] = 0;
  }
}
