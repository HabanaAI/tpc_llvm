// SPDX-License-Identifier: Apache-2.0
// Consume the existing Full/Reindex/Reuse publication; only fuse its C1 views/masks.
void main(tensor positions, tensor compressed, tensor output, tensor lengths, int ratio) {
    const int5 begin = get_index_space_offset();
    const int5 end = begin + get_index_space_size();
    const int position = s_i32_ld_g(gen_addr((int5){0}, positions));
    const int64 lanes = as_int64(V_LANE_ID_32);
    for (int block = begin[0]; block < end[0]; ++block) {
        const int offset = block * 64;
        int64 ids;
        if (offset < 128) {
            const int64 window = lanes + offset + position - 127;
            ids = v_i32_sel_geq_i32_b(window, 0, window, -1);
        } else {
            const int64 published = v_i32_ld_tnsr_b((int5){offset - 128, 0, 0, 0, 0}, compressed);
            ids = v_i32_sel_geq_i32_b(published, 0, published + 512, -1);
        }
        v_i32_st_tnsr((int5){offset, 0, 0, 0, 0}, output, ids);
        if (block == 0) {
            const int visible = ratio == 2 ? (position + 1) >> 1 : (ratio ? position + 1 : 0);
            s_i32_st_g(gen_addr((int5){0}, lengths), 128 + visible);
        }
    }
}
