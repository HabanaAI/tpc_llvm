// SPDX-License-Identifier: AGPL-3.0-only
// Copyright Aaron Botsis
// DeepSeek-V4.1 candidate blocks, level one: the largest score of each block of 8 compressed entries of a
// column over its visible entries (vis[c]). Blocks with no visible entry hold -3e38, and the block holding the
// newest visible entry holds +3e38, so ix_thr's KS-th largest block keeps it. Block b of a group of 64 blocks
// (512 entries) is stored at lane 8 (b % 8) + b / 8 of that group's vector (ix_cmask reads this order).
// tensors: 0 score (K, T) f32  1 vis (T) i32 | 2 bmax (K / 8, T) f32
// index space: [K / 512, T]
#include "common.h"

#define NINF (-3.0e38f)
#define PIN 3.0e38f

// lane l <- element e (0..15) of its dual group
static inline uchar256 ctl_of(int64 e)
{
    const int64 c = (0x80 | ((e >> 3) << 5) | (e & 7)) * 0x01010101;
    return __builtin_bit_cast(uchar256, c);
}

void main(tensor score, tensor vis, tensor bmax)
{
    const int5 off = get_index_space_offset();
    const int5 end = off + get_index_space_size();
    const int64 lane = (int64)read_lane_id_4b_b();
    const float64 lanef = v_convert_i32_to_f32_b(lane, SW_RHNE, 0, 1, 0);
    const float64 l8f = v_convert_i32_to_f32_b(lane & 7, SW_RHNE, 0, 1, 0);
    const int64 e = lane & 15;
    const uchar256 SH1 = ctl_of(e ^ 1), SH2 = ctl_of(e ^ 2), SH4 = ctl_of(e ^ 4);
    scalar_cache_reset();
    for (int c = off[1]; c < end[1]; c++) {
        int5 vc = {c, 0, 0, 0, 0};
        const int lim = s_i32_ld_g(gen_addr(vc, vis));
        const float limf = (float)lim;
        const int last = lim > 0 ? (lim - 1) >> 3 : -1;
        for (int g = off[0]; g < end[0]; g++) {
            float64 out = NINF;
            if (g * 512 < lim) {
                #pragma unroll
                for (int j = 0; j < 8; j++) {
                    const int e0 = g * 512 + 64 * j;
                    int5 sc = {e0, c, 0, 0, 0};
                    float64 v = v_f32_sel_less_f32_b(lanef + (float)e0, limf, v_f32_ld_tnsr_b(sc, score), NINF);
                    v = v_f32_max_b(v, v_f32_shuffle_b(v, SH1, 0, v));
                    v = v_f32_max_b(v, v_f32_shuffle_b(v, SH2, 0, v));
                    v = v_f32_max_b(v, v_f32_shuffle_b(v, SH4, 0, v));
                    // lanes 8k .. 8k + 7 all hold block 8 j + k: lane 8 k + j takes it
                    out = v_f32_sel_eq_f32_b(l8f, (float)j, v, out);
                }
                const int b = last - 64 * g;
                if (b >= 0 && b < 64)
                    out = v_f32_sel_eq_f32_b(lanef, (float)(8 * (b & 7) + (b >> 3)), PIN, out);
            }
            int5 oc = {64 * g, c, 0, 0, 0};
            v_f32_st_tnsr(oc, bmax, out);
        }
    }
}
