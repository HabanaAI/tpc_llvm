// sigmoid_bf16 (tanh-based) on random bit patterns (every exponent, NaN, inf)
// and on -30..30; no_saturation_sigmoid_bf16 and no_saturation_sigmoid_f32 on
// gaudi2, where Habana's compiler has them.
//
// DIFFTEST: arch gaudi gaudi2
// DIFFTEST: in  u16 128 64
// DIFFTEST: in  bf16 128 64 range=-30,30
// DIFFTEST: in  u32 64 64
// DIFFTEST: in  f32 64 64 range=-30,30
// DIFFTEST: out bf16 128 64
// DIFFTEST: out bf16 128 64
// DIFFTEST: out bf16 128 64
// DIFFTEST: out bf16 128 64
// DIFFTEST: out f32 64 64
// DIFFTEST: out f32 64 64
// DIFFTEST: index-space 1 64

void main(tensor bits16, tensor in16, tensor bits32, tensor in32,
          tensor o1, tensor o2, tensor o3, tensor o4, tensor o5, tensor o6) {
  int5 c = 0;
  c[1] = get_index_space_offset()[1];
  const short128 b = v_i16_ld_tnsr_b(c, bits16);
  const bfloat128 x = *(const bfloat128 *)&b;
  const bfloat128 y = v_bf16_ld_tnsr_b(c, in16);
  v_bf16_st_tnsr(c, o1, sigmoid_bf16(x));
  v_bf16_st_tnsr(c, o2, sigmoid_bf16(y));
#if defined(__gaudi2__)
  v_bf16_st_tnsr(c, o3, no_saturation_sigmoid_bf16(x));
  v_bf16_st_tnsr(c, o4, no_saturation_sigmoid_bf16(y));
  const int64 w = v_i32_ld_tnsr_b(c, bits32);
  v_f32_st_tnsr(c, o5, no_saturation_sigmoid_f32(*(const float64 *)&w));
  v_f32_st_tnsr(c, o6, no_saturation_sigmoid_f32(v_f32_ld_tnsr_b(c, in32)));
#endif
}
