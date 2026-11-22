// SPDX-License-Identifier: Apache-2.0
// Request-owned integer addressing; no KV or floating-point arithmetic.
void main(tensor selected, tensor pages, tensor positions, tensor slots,
          tensor swa_done, tensor main_done, tensor rows, tensor indices,
          tensor lengths, int ratio, int swa_rows, int tile)
{
    const int5 start = get_index_space_offset();
    const int5 end = start + get_index_space_size();
    const int width = ratio ? 640 : 128;
    const int page_count = get_dim_size(pages, 0);
    const int page_rows = ratio == 2 ? 64 : 128;
    for (int b = start[1]; b < end[1]; ++b) {
        const int position = s_i32_ld_g(gen_addr((int5){b}, positions));
        const int slot = s_i32_ld_g(gen_addr((int5){b}, slots));
        const bool active = position >= 0 && slot >= 0 && slot < swa_rows / 256;
        const bool ready = active &&
            s_i32_ld_g(gen_addr((int5){b}, swa_done)) >= -1 &&
            s_i32_ld_g(gen_addr((int5){b}, main_done)) >= -1;
        if (start[0] == 0)
            s_i32_st_g(gen_addr((int5){b}, lengths), active ? width : 0);
        for (int block = start[0]; block < end[0]; ++block) {
            for (int j = block * 16; j < (block + 1) * 16; ++j) {
                int physical = -1;
                if (ready) {
                    if (j < 128) {
                        const int logical = position - 127 + j;
                        if (logical >= 0)
                            physical = slot * 256 + (logical & 255);
                    } else {
                        const int logical = s_i32_ld_g(gen_addr((int5){j - 128, b}, selected));
                        if (logical >= 0 && logical < page_count * page_rows &&
                            logical < (position + 1) / ratio) {
                            const int page = s_i32_ld_g(gen_addr((int5){logical / page_rows, b}, pages));
                            if (page > 0)
                                physical = swa_rows + page * page_rows + (logical & (page_rows - 1));
                        }
                    }
                }
                s_i32_st_g(gen_addr((int5){j, b}, rows), physical);
                s_i32_st_g(gen_addr((int5){j, b}, indices),
                           physical >= 0 ? (b % tile) * width + j : -1);
            }
        }
    }
}
