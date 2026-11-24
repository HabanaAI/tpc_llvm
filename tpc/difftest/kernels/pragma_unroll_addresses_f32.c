// Unrolled in order by its pragma, each copy computing 2 global addresses;
// the count is lowered so the addresses fit the 8 address registers, which
// cannot be spilled. 4 of 1cat's DeepSeek-V4 kernels failed on the simulator
// with the spilled addresses.
//
// DIFFTEST: arch gaudi2
// DIFFTEST: in  i32 64 32 range=-1,3
// DIFFTEST: in  f32 64 32
// DIFFTEST: in  f32 128 4
// DIFFTEST: out f32 128 4
// DIFFTEST: index-space 2 4

void main(tensor ids, tensor scale, tensor channel, tensor out) {
  const int5 start = get_index_space_offset(), end = start + get_index_space_size();
  for (int token = start[1]; token < end[1]; ++token) {
    for (int block = start[0]; block < end[0]; ++block) {
      float64 acc = 0;
#pragma loop_unroll(6)
      for (int slot = 0; slot < 6; ++slot) {
        const int row = token * 6 + slot;
        const int e = s_i32_ld_g(gen_addr((int5){row}, ids));
        const float sx = s_f32_ld_g(gen_addr((int5){0, row}, scale));
        acc += v_f32_ld_tnsr_b((int5){block * 64, e}, channel) * sx;
      }
      v_f32_st_tnsr((int5){block * 64, token}, out, acc);
    }
  }
}

