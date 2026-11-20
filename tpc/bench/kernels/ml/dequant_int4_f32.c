// 4-bit weight dequantization as for GPTQ checkpoints (Qwen publishes
// GPTQ-Int4 models): each 32-bit word of qweight packs 8 consecutive input
// channels of one output channel, and w = (q - 8) * scale with one f32 scale
// per output channel and group of 128 input channels. Output channels are in
// the lanes; 4096 outputs x 256 inputs. Shifts, masks and int-to-float
// conversions, eight stores per word.
//
// BENCH: arch gaudi2
// BENCH: in  i32 4096 32
// BENCH: in  f32 4096 2 range=0.001,0.05
// BENCH: out f32 4096 256
// BENCH: index-space 64

void main(tensor qweight, tensor scales, tensor out) {
  const int5 start = get_index_space_offset();
  const int5 end = get_index_space_size() + start;
  const int words = get_dim_size(qweight, 1);
  int5 c = 0;
  int5 sc = 0;
  int5 o = 0;

  for (int n = start[0] * 64; n < end[0] * 64; n += 64) {
    c[0] = sc[0] = o[0] = n;
    for (int k = 0; k < words; k++) {
      c[1] = k;
      sc[1] = k / 16; // 16 words = one group of 128 inputs
      const int64 word = v_i32_ld_tnsr_b(c, qweight);
      const float64 scale = v_f32_ld_tnsr_b(sc, scales);
      for (int j = 0; j < 8; j++) {
        o[1] = k * 8 + j;
        const int64 nibble = ((word >> (4 * j)) & 15) - 8;
        v_f32_st_tnsr(o, out, convert_int64_to_float64(nibble, SW_RHNE) * scale);
      }
    }
  }
}
