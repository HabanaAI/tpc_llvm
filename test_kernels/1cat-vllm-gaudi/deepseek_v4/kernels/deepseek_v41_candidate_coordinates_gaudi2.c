// SPDX-License-Identifier: Apache-2.0
// Expand eight-row candidate blocks and clamp gather addresses in one producer.
void main(tensor blocks, tensor logical, tensor safe, int maximum_row, int block_count) {
    const int5 begin = get_index_space_offset();
    const int5 end = begin + get_index_space_size();
    const int64 lanes = (int64)read_lane_id_4b_b();
    for (int query = begin[1]; query < end[1]; ++query) {
        for (int tile = begin[0]; tile < end[0]; ++tile) {
            int64 ids = -1;
            for (int j = 0; j < 8; ++j) {
                const int slot = tile * 8 + j;
                if (slot >= block_count) break;
                const int block = s_i32_ld_g(gen_addr((int5){slot, query}, blocks));
                const int first = (int)((unsigned)block << 3);
                const int64 current = block >= 0 ? (lanes & 7) + first : (int64)-1;
                ids = v_i32_mov_vb(current, 0, ids, v_i32_cmp_eq_b(lanes >> 3, j), 0);
            }
            const int64 addresses = v_i32_min_b(v_i32_max_b(ids, 0), maximum_row);
            const int5 out = {tile * 64, query};
            const int left = block_count * 8 - tile * 64;
            if (left >= 64) {
                v_i32_st_tnsr(out, logical, ids);
                v_i32_st_tnsr(out, safe, addresses);
            } else {
                v_i32_st_tnsr_partial(out, logical, ids, left - 1, 0);
                v_i32_st_tnsr_partial(out, safe, addresses, left - 1, 0);
            }
        }
    }
}
