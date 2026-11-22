// SPDX-License-Identifier: Apache-2.0
// Candidate blocks contain eight consecutive rows. Preserve their slot order,
// duplicates and holes while avoiding elementwise general-purpose gathers.
void main(tensor common, tensor blocks, tensor positions,
          tensor scores, tensor rows, int ratio, int source_rows, int block_count) {
    const int5 begin = get_index_space_offset();
    const int5 end = begin + get_index_space_size();
    const int64 lanes = (int64)V_LANE_ID_32;
    const float64 negative_infinity = as_float64((int64)0xff800000);
    for (int query = begin[1]; query < end[1]; ++query) {
        const int visible = (s_i32_ld_g(gen_addr((int5){query}, positions)) + 1) / ratio;
        for (int tile = begin[0]; tile < end[0]; ++tile) {
            float64 values = negative_infinity;
            int64 ids = -1;
            // Accumulate the eight 8-row blocks in one register and issue one
            // full-vector store per output. The partial-load offset selects
            // destination lanes; source coordinates name the first loaded row.
            for (int j = 0; j < 8; ++j) {
                const int slot = tile * 8 + j;
                if (slot >= block_count) break;
                const int block = s_i32_ld_g(gen_addr((int5){slot, query}, blocks));
                const int first = block * 8;
                const int lane_offset = j * 8;
                const bool64 group = v_i32_cmp_eq_b(lanes / 8, j);
                const int64 current_ids = block >= 0 ? (lanes & 7) + first : (int64)-1;
                ids = v_i32_mov_vb(current_ids, 0, ids, group, 0);
                if (block >= 0 && block < source_rows / 8 && first < visible) {
                    values = v_f32_ld_tnsr_partial_b((int5){first, query}, common,
                                                     7, lane_offset, 0, values);
                }
            }
            values = v_f32_mov_vb(values, 0, negative_infinity,
                                  v_i32_cmp_geq_b(ids, 0) & v_i32_cmp_less_b(ids, visible) &
                                  v_i32_cmp_less_b(ids, source_rows), 0);
            const int5 output = {tile * 64, query};
            const int left = block_count * 8 - tile * 64;
            if (left >= 64) {
                v_f32_st_tnsr(output, scores, values);
                v_i32_st_tnsr(output, rows, ids);
            } else {
                v_f32_st_tnsr_partial(output, scores, values, left - 1, 0);
                v_i32_st_tnsr_partial(output, rows, ids, left - 1, 0);
            }
        }
    }
}
