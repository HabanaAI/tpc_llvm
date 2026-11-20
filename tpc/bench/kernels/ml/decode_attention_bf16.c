// Attention for one decode step in the flash-decoding style: one query per head
// against a KV cache of 256 positions, with an online softmax that keeps a
// running maximum, a running sum and a running weighted sum of V. head_dim 128
// in the lanes, bf16 cache, f32 math; 32 heads, one per member. A lane
// reduction and two exps per position.
//
// BENCH: arch gaudi2
// BENCH: in  bf16 128 32 range=-2,2
// BENCH: in  bf16 128 256 32 range=-2,2
// BENCH: in  bf16 128 256 32 range=-2,2
// BENCH: out f32  128 32
// BENCH: index-space 1 32

#define SCALE 0.08838834764f // 1 / sqrt(head_dim)

void main(tensor q, tensor k, tensor v, tensor out) {
  const int5 start = get_index_space_offset();
  const int5 end = get_index_space_size() + start;
  const int len = get_dim_size(k, 1);
  int5 qc = 0;
  int5 kc = 0;

  for (int h = start[1]; h < end[1]; h++) {
    qc[1] = kc[2] = h;
    qc[0] = 0;
    const bfloat128 qv = v_bf16_ld_tnsr_b(qc, q);
    float64 m = -3.0e38f;
    float64 l = 0.0f;
    float128 acc = {0};
    for (int p = 0; p < len; p++) {
      kc[1] = p;
      float128 prod = v_bf16_mac_acc32_b(v_bf16_ld_tnsr_b(kc, k), qv, (float128){0});
      const float64 s = v_f32_reduce_add(prod.v1 + prod.v2) * SCALE;
      const float64 m_new = v_f32_max_b(m, s);
      const float64 alpha = v_exp_f32(m - m_new);
      const float64 e = v_exp_f32(s - m_new);
      l = l * alpha + e;
      float128 vv = convert_bfloat128_to_float128(v_bf16_ld_tnsr_b(kc, v), 0);
      acc.v1 = acc.v1 * alpha + vv.v1 * e;
      acc.v2 = acc.v2 * alpha + vv.v2 * e;
      m = m_new;
    }
    const float64 inv = v_reciprocal_f32(l);
    qc[0] = 0;
    v_f32_st_tnsr(qc, out, acc.v1 * inv);
    qc[0] = 64;
    v_f32_st_tnsr(qc, out, acc.v2 * inv);
  }
}
