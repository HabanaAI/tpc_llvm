// SPDX-License-Identifier: Apache-2.0
// Reduce the six already-scaled BF16 routed-expert rows in their routing
// order. This replaces BF16->FP32, five graph add nodes, FP32->BF16 and the
// final identity while keeping the existing N256 scaling kernel unchanged.
void main(tensor rows, tensor output)
{
    const int5 start = get_index_space_offset();
    const int5 end = start + get_index_space_size();
    for (int token = start[1]; token < end[1]; ++token) {
      for (int block = start[0]; block < end[0]; ++block) {
        const int n = block * 128;
        float128 accumulated = v_convert_bf16_to_f32_all_b(
            v_bf16_ld_tnsr_b((int5){n, 0, token * 6}, rows));
        #pragma loop_unroll(5)
        for (int slot = 1; slot < 6; ++slot) {
            const float128 value = v_convert_bf16_to_f32_all_b(
                v_bf16_ld_tnsr_b((int5){n, 0, token * 6 + slot}, rows));
            accumulated.v1 += value.v1;
            accumulated.v2 += value.v2;
        }
        v_bf16_st_tnsr((int5){n, 0, token}, output,
            v_convert_f32_to_bf16_all_b(accumulated, SW_RHNE));
      }
    }
}
