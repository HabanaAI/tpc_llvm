// SPDX-License-Identifier: AGPL-3.0-only
// Copyright Aaron Botsis
// Decode MoE: y = sum over local slots k, input blocks b of yk[:, b + NB k] + addsh * sigmoid(lg[E_ALL]) * sh
// (moe_down_w writes only local slots' columns).
// tensors: 0 yk (H, NB*K, T) f32  1 sh (H,T) f32  2 lg (>= E_ALL+1, T) f32  3 ids (64,T) i32 | 4 y (H,T) f32
// params: addsh, E_ALL, lo, E (local experts), K. index space: [H/64, T]
#include "common.h"

void main(tensor yk, tensor sh, tensor lg, tensor ids, tensor y, int addsh, int E_ALL, int lo, int E, int K)
{
    const int5 off = get_index_space_offset();
    const int5 end = off + get_index_space_size();
    const int NB = get_dim_size(yk, 1) / K;
    DECL_IDX(IDX)
    scalar_cache_reset();
    for (int t = off[1]; t < end[1]; t++) {
        int5 gc = {E_ALL, t, 0, 0, 0};
        const float64 gv = BCAST_DG(v_f32_ld_tnsr_b(gc, lg), 0);
        const float64 gate = nl_sigm(v_f32_shuffle_b(gv, IDX[0], 0, gv)) * (float)addsh;
        int loc[64];
        int nl = 0;
        for (int k = 0; k < K; k++) {
            int5 ic = {k, t, 0, 0, 0};
            const int e = s_i32_ld_g(gen_addr(ic, ids)) - lo;
            if (e >= 0 && e < E) loc[nl++] = k;
        }
        for (int blk = off[0]; blk < end[0]; blk++) {
            int5 c = {blk * 64, t, 0, 0, 0};
            float64 a = v_f32_ld_tnsr_b(c, sh) * gate;
            int5 kc = {blk * 64, 0, t, 0, 0};
            for (int j = 0; j < nl; j++)
                for (int b = 0; b < NB; b++) {
                    kc[1] = b + NB * loc[j];
                    a += v_f32_ld_tnsr_b(kc, yk);
                }
            v_f32_st_tnsr(c, y, a);
        }
    }
}
