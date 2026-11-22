// SPDX-License-Identifier: AGPL-3.0-only
// Copyright Aaron Botsis
// Mistral-Large-4: RoPE (adjacent pairs (2i, 2i+1), as apply_rotary_pos_emb_interleave) on rows r0 .. r0 + 63 of each
// column (r0 < 0: none), then the SSMax query scale s[h] * lp[t] + b[h] on the whole column (s = 0, b = 1: none).
// Columns c = t * hpt + h of x (the GEMV output: token-major heads); y is written head-major, y[:, t, h] (hm = 1), or
// in x's order (hm = 0). cs (128, T): each token's position, [cos_(l>>1) | (l odd ? 1 : -1) sin_(l>>1)].
// tensors: 0 x (D, N) f32  1 cs (128, T) f32  2 s (hpt) f32  3 b (hpt) f32  4 lp (T) f32 (log(pos + 1))
//          | 5 y (D, T, hpt) or (D, N) f32
// params: hpt, r0, hm. index space: [N]. D multiple of 64.
#include "common.h"

void main(tensor x, tensor cs, tensor s, tensor b, tensor lp, tensor y, int hpt, int r0, int hm)
{
    const int5 off = get_index_space_offset();
    const int5 end = off + get_index_space_size();
    const int D = get_dim_size(x, 0);
    const int64 lane = (int64)read_lane_id_4b_b();
    const int64 e = (lane ^ 1) & 15;                                   // pair partner within the dual group
    const int64 ctl = (0x80 | ((e >> 3) << 5) | (e & 7)) * 0x01010101;
    const uchar256 SW = __builtin_bit_cast(uchar256, ctl);
    scalar_cache_reset();
    int t = 0, h = off[0];                                             // token and head of the first column
    while (h >= hpt) { h -= hpt; t++; }
    for (int c = off[0]; c < end[0]; c++) {
        int5 tc = {0, t, 0, 0, 0};
        const float64 co = v_f32_ld_tnsr_b(tc, cs);
        tc[0] = 64;
        const float64 si = v_f32_ld_tnsr_b(tc, cs);
        int5 hc = {h, 0, 0, 0, 0}, pc = {t, 0, 0, 0, 0};
        const float sc = s_f32_ld_g(gen_addr(hc, s)) * s_f32_ld_g(gen_addr(pc, lp)) + s_f32_ld_g(gen_addr(hc, b));
        for (int i = 0; i < D; i += 64) {
            int5 xc = {i, c, 0, 0, 0};
            float64 v = v_f32_ld_tnsr_b(xc, x);
            if (i == r0)
                v = v_f32_mac_b(v_f32_shuffle_b(v, SW, 0, v), si, v * co);
            int5 oc = {i, hm ? t : c, hm ? h : 0, 0, 0};
            v_f32_st_tnsr(oc, y, v * sc);
        }
        if (++h == hpt) { h = 0; t++; }
    }
}
