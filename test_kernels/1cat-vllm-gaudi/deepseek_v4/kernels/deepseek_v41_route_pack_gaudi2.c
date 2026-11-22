// SPDX-License-Identifier: Apache-2.0
// Stable expert/route order and byte-exact activation packing, without a
// generic sort, gather intermediates, atomics, or host occupancy readback.
void main(tensor x, tensor sx, tensor ids, tensor route, tensor inverse,
          tensor packed, tensor scales, tensor experts, tensor weights)
{
    const int5 start = get_index_space_offset();
    const int5 end = start + get_index_space_size();
    const int width = get_dim_size(x, 0);
    for (int original = start[0]; original < end[0]; ++original) {
        const int expert = s_i32_ld_g(gen_addr((int5){original}, ids));
        const int sorted = s_i32_ld_g(gen_addr((int5){original}, inverse));
        const int token = original / 6;
        s_i32_st_g(gen_addr((int5){sorted}, experts), expert);
        s_f32_st_g(gen_addr((int5){0, 0, sorted}, scales),
                   s_f32_ld_g(gen_addr((int5){0, token}, sx)));
        s_f32_st_g(gen_addr((int5){0, sorted}, weights),
                   s_f32_ld_g(gen_addr((int5){original}, route)));
        for (int column = 0; column < width; column += 256)
            v_u8_st_tnsr((int5){column, 0, sorted}, packed,
                        v_u8_ld_tnsr_b((int5){column, token}, x));
    }
}
