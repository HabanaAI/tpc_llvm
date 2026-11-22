// SPDX-License-Identifier: AGPL-3.0-only
// Copyright Aaron Botsis
// DeepSeek-V4.1 candidate blocks, sparse scoring: the compact scores of a column with every entry whose position
// (cpos, ix_cgather) is at or past the column's visible entries vis[c] set to -3e38.
// tensors: 0 score (KC, T) f32  1 cpos (KC, T) i32  2 vis (T) i32 | 3 out (KC, T) f32
// index space: [KC / 512, T]
#include "common.h"

#define NINF (-3.0e38f)

void main(tensor score, tensor cpos, tensor vis, tensor out)
{
    const int5 off = get_index_space_offset();
    const int5 end = off + get_index_space_size();
    scalar_cache_reset();
    for (int c = off[1]; c < end[1]; c++) {
        int5 vc = {c, 0, 0, 0, 0};
        const int lim = s_i32_ld_g(gen_addr(vc, vis));
        for (int g = off[0]; g < end[0]; g++) {
            #pragma unroll
            for (int j = 0; j < 8; j++) {
                int5 sc = {g * 512 + 64 * j, c, 0, 0, 0};
                const int64 p = v_i32_ld_tnsr_b(sc, cpos);
                v_f32_st_tnsr(sc, out, as_float64(v_i32_sel_less_i32_b(p, (int64)lim, as_int64(v_f32_ld_tnsr_b(sc, score)), as_int64((float64)NINF))));
            }
        }
    }
}
