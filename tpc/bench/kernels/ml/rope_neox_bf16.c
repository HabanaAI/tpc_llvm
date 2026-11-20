// Rotary position embedding in the rotate-half (GPT-NeoX) form that Qwen2 and
// Qwen3 use (vLLM's rotary_embedding): the halves x1, x2 of each 128-wide head
// become x1 cos - x2 sin and x2 cos + x1 sin, with cos and sin per position
// from a precomputed table. 32 query heads, 256 positions; each member
// rotates 8 heads of one position with the same cos and sin vectors.
//
// BENCH: arch gaudi2
// BENCH: in  bf16 128 32 256 range=-4,4
// BENCH: in  f32  64 256 range=-1,1
// BENCH: in  f32  64 256 range=-1,1
// BENCH: out bf16 128 32 256
// BENCH: index-space 1 4 256

#define HEADS 8 // heads per member

void main(tensor q, tensor cos_table, tensor sin_table, tensor out) {
  const int5 start = get_index_space_offset();
  const int5 end = get_index_space_size() + start;
  int5 c = 0;
  int5 p = 0;

  for (int t = start[2]; t < end[2]; t++) {
    c[2] = p[1] = t;
    const float64 cs = v_f32_ld_tnsr_b(p, cos_table);
    const float64 sn = v_f32_ld_tnsr_b(p, sin_table);
    for (int h = start[1] * HEADS; h < end[1] * HEADS; h++) {
      c[1] = h;
      float128 x = convert_bfloat128_to_float128(v_bf16_ld_tnsr_b(c, q), 0);
      float128 r;
      r.v1 = x.v1 * cs - x.v2 * sn;
      r.v2 = x.v2 * cs + x.v1 * sn;
      v_bf16_st_tnsr(c, out, convert_float128_to_bfloat128(r, SW_RHNE));
    }
  }
}
