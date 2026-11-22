// SPDX-License-Identifier: Apache-2.0
// One owner per request preserves source-slot order, including duplicate IDs.
// All inactive output bytes are initialized on every replay.
void main(tensor candidates, tensor positions, tensor compact, tensor source_slots,
          tensor counts, int ratio) {
    const int5 begin = get_index_space_offset(), end = begin + get_index_space_size();
    for (int request = begin[0]; request < end[0]; ++request) {
        const int position = s_i32_ld_g(gen_addr((int5){request}, positions));
        const int visible = position < 0 ? 0 : (position + 1) / ratio;
        const int limit = (visible + 7) / 8;
        int count = 0;
        if (visible > 512) {
            for (int slot = 0; slot < 2048; ++slot) {
                const int id = s_i32_ld_g(gen_addr((int5){slot, request}, candidates));
                if (id >= 0 && id < limit) {
                    s_i32_st_g(gen_addr((int5){count, request}, compact), id);
                    s_i32_st_g(gen_addr((int5){count, request}, source_slots), slot);
                    ++count;
                }
            }
        }
        int tail = count;
        for (; tail < 2048 && (tail % 64); ++tail) {
            s_i32_st_g(gen_addr((int5){tail, request}, compact), -1);
            s_i32_st_g(gen_addr((int5){tail, request}, source_slots), -1);
        }
        for (; tail < 2048; tail += 64) {
            v_i32_st_tnsr((int5){tail, request}, compact, (int64)-1);
            v_i32_st_tnsr((int5){tail, request}, source_slots, (int64)-1);
        }
        s_i32_st_g(gen_addr((int5){request}, counts), count);
    }
}
