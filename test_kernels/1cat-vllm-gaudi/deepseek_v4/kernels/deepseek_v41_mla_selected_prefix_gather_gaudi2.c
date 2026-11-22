// SPDX-License-Identifier: Apache-2.0
// C1 decoded MLA gather with the fixed short-prefix layout computed at the
// consumer.  This removes the per-layer [640] attention-index and length
// tensors while preserving the SWA-ring and logical-main row mapping.
void main(tensor swa,
          tensor main_kv,
          tensor selected,
          tensor position,
          tensor swa_done,
          tensor main_done,
          tensor keys,
          tensor values,
          tensor mask,
          int swa_offset,
          int main_rows,
          int prefix_rows)
{
    const int5 begin = get_index_space_offset();
    const int5 end = begin + get_index_space_size();
    const int logical = s_i32_ld_g(gen_addr((int5){0, 0, 0, 0, 0},
                                             position));
    const bool ready =
        s_i32_ld_g(gen_addr((int5){0, 0, 0, 0, 0}, swa_done)) >= 0 &&
        s_i32_ld_g(gen_addr((int5){0, 0, 0, 0, 0}, main_done)) >= 0;

    for (int row = begin[0]; row < end[0]; ++row) {
        bool valid = ready;
        bool from_swa = row < 128;
        int source_row;
        if (from_swa) {
            const int absolute = logical - 127 + row;
            valid = valid && absolute >= 0;
            source_row = valid ? swa_offset + (absolute & 255) : swa_offset;
        } else {
            const int chosen = s_i32_ld_g(
                gen_addr((int5){row - 128, 0, 0, 0, 0}, selected));
            valid = valid && chosen >= 0 && chosen < main_rows;
            source_row = valid ? chosen : 0;
        }

        for (int chunk = 0; chunk < 4; ++chunk) {
            const int5 output_at = {chunk * 128, row, 0, 0, 0};
            const int5 source_at = {chunk * 128, source_row, 0, 0, 0};
            const bfloat128 value = from_swa
                ? v_bf16_ld_tnsr_b(source_at, swa, 0, (bfloat128)0, valid)
                : v_bf16_ld_tnsr_b(source_at, main_kv, 0, (bfloat128)0,
                                   valid);
            v_bf16_st_tnsr(output_at, keys, value);
            const float128 wide = convert_bfloat128_to_float128(value,
                                                                 SW_LINEAR);
            v_f32_st_tnsr(output_at, values, wide.v1);
            int5 high = output_at;
            high[0] += 64;
            v_f32_st_tnsr(high, values, wide.v2);
        }
        s_f32_st_g(gen_addr((int5){row, 0, 0, 0, 0}, mask),
                   valid ? 1.0f : 0.0f);
    }
}
