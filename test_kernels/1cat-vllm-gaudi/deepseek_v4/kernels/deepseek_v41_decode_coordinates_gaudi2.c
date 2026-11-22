// SPDX-License-Identifier: Apache-2.0
// One shared coordinate row per token. This experimental producer is not
// selected by serving until its consumers and replay ownership are qualified.
void main(tensor positions, tensor ids, tensor pages, tensor output) {
    const int5 begin = get_index_space_offset();
    const int5 end = begin + get_index_space_size();
    const int64 lane = (int64)read_lane_id_4b_b();
    for (int token = begin[1]; token < end[1]; ++token) {
        const int pos = s_i32_ld_g(gen_addr((int5){token}, positions));
        const int id = s_i32_ld_g(gen_addr((int5){token}, ids));
        const bool position_valid = pos >= 0 && (pos >> 7) < get_dim_size(pages, 0);
        const int page = position_valid ? s_i32_ld_g(gen_addr((int5){pos >> 7}, pages)) : -1;
        const bool active = position_valid && page >= 0;
        const int compressed = pos >> 1;
        int64 meta = {0};
#define FIELD(I, VALUE) meta = v_i32_sel_eq_i32_b(lane, I, VALUE, meta)
        FIELD(0, pos);
        FIELD(1, active ? pos & 255 : -1);
        FIELD(2, active ? pos & 7 : -1);
        FIELD(3, active ? pos : -1);
        FIELD(4, active ? compressed : -1);
        FIELD(5, active ? page * 128 + (pos & 127) : -1);
        FIELD(6, active ? page * 64 + (compressed & 63) : -1);
        FIELD(7, active ? page : -1);
        FIELD(8, active ? pos & 1 : 0);
        FIELD(9, active ? pos & -2 : -1);
        FIELD(10, active ? pos - 127 : -1);
        FIELD(11, active && (id == 129264 || id == 129265));
        FIELD(12, active ? s_i32_min(pos + 1, 128) : 0);
        FIELD(13, active ? pos + 1 : 0);
        FIELD(14, active ? (pos + 1) >> 1 : 0);
        FIELD(15, active ? 640 : 0);
#undef FIELD
        v_i32_st_tnsr((int5){0, token}, output, meta);
        for (int chunk = 0; chunk < 2; ++chunk) {
            const int64 absolute = lane + chunk * 64 + pos - 127;
            const int64 rows = active ? v_i32_sel_geq_i32_b(absolute, 0, absolute & 255, -1) : (int64)-1;
            v_i32_st_tnsr((int5){64 + chunk * 64, token}, output, rows);
        }
    }
}
