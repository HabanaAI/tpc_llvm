// SwiGLU, the MLP activation of DeepSeek-V3 and Qwen3 (vLLM's silu_and_mul):
// out = silu(gate) * up, with gate and up the two halves of dim 0 of x. bf16
// in and out, f32 math. Width 2048, DeepSeek-V3's routed-expert size; one
// token per member.
//
// BENCH: arch gaudi2
// BENCH: in  bf16 4096 128 range=-6,6
// BENCH: out bf16 2048 128
// BENCH: index-space 1 128

void main(tensor x, tensor out) {
  const int5 start = get_index_space_offset();
  const int5 end = get_index_space_size() + start;
  const int width = get_dim_size(out, 0);
  int5 g = 0;
  int5 u = 0;

  for (int t = start[1]; t < end[1]; t++) {
    g[1] = u[1] = t;
    for (int i = 0; i < width; i += 128) {
      g[0] = i;
      u[0] = i + width;
      float128 a = convert_bfloat128_to_float128(v_bf16_ld_tnsr_b(g, x), 0);
      float128 b = convert_bfloat128_to_float128(v_bf16_ld_tnsr_b(u, x), 0);
      float128 r;
      r.v1 = a.v1 * v_sigmoid_f32(a.v1) * b.v1;
      r.v2 = a.v2 * v_sigmoid_f32(a.v2) * b.v2;
      v_bf16_st_tnsr(g, out, convert_float128_to_bfloat128(r, SW_RHNE));
    }
  }
}
