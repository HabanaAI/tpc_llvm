// SPDX-License-Identifier: Apache-2.0
void main(tensor indices, tensor lengths, tensor sink, tensor output, int rows) {
    const int5 first = get_index_space_offset();
    const int5 end = first + get_index_space_size();
    const int columns = get_dim_size(indices, 0);
    const int64 lane = (int64)read_lane_id_4b_b();
    for (int token = first[2]; token < end[2]; ++token) {
        const int5 length_at = {token, 0, 0, 0, 0};
        const int length = s_i32_ld_g(gen_addr(length_at, lengths));
        for (int head = first[1]; head < end[1]; ++head) {
            const int5 sink_at = {head, 0, 0, 0, 0};
            const float bias = s_f32_ld_g(gen_addr(sink_at, sink));
            for (int block = first[0]; block < end[0]; ++block) {
                const int offset = block * 64;
                const int5 index_at = {offset, token, 0, 0, 0};
                const int count = s_i32_min(s_i32_max(columns - offset, 0), 64);
                const int64 ids = v_i32_ld_tnsr_partial_b(index_at, indices, count - 1, 0, 0, -1, count > 0);
                const int64 column = lane + offset;
                const bool64 valid = v_i32_cmp_less_b(column, columns) & v_i32_cmp_less_b(column, length) &
                                     v_i32_cmp_geq_b(ids, 0) & v_i32_cmp_less_b(ids, rows);
                float64 mask = v_f32_mov_vb(0.0f, 0, as_float64((uint64)0xff800000), valid);
                mask = v_f32_sel_eq_i32_b(column, columns, bias, mask);
                const int5 destination = {offset, head, token, 0, 0};
                v_f32_st_tnsr_partial(destination, output, mask, s_i32_min(columns + 1 - offset, 64) - 1, 0);
            }
        }
    }
}
