// SPDX-License-Identifier: AGPL-3.0-only
// Copyright Aaron Botsis
// QSA indexer queries: q_h = rope_pos(rmsnorm(p[qofs + 128 h ..]) * w_q), h < NH (4), rope on dims 0..63.
// tensors: 0 p (P, T) f32  1 ixn (256) f32  2 cs (128, T) f32 (rope rows of the chunk's positions)
//        | 3 q (128, NH T) f32 (column h + NH t)
// params: eps, qofs. index space: [T]
#include "common.h"

void main(tensor p, tensor ixn, tensor cs, tensor q, float eps, int qofs)
{
    const int5 off = get_index_space_offset();
    const int5 end = off + get_index_space_size();
    const int NH = get_dim_size(q, 1) / get_dim_size(p, 1);
    DECL_IDX(IDX)
    int5 wc = {0, 0, 0, 0, 0};
    const float64 w0 = v_f32_ld_tnsr_b(wc, ixn);
    wc[0] = 64;
    const float64 w1 = v_f32_ld_tnsr_b(wc, ixn);
    for (int t = off[0]; t < end[0]; t++) {
        int5 cc = {0, t, 0, 0, 0};
        const float64 cosv = v_f32_ld_tnsr_b(cc, cs);
        cc[0] = 64;
        const float64 sinv = v_f32_ld_tnsr_b(cc, cs);
        for (int h = 0; h < NH; h++) {
            int5 pc = {qofs + 128 * h, t, 0, 0, 0};
            const float64 a = v_f32_ld_tnsr_b(pc, p);
            pc[0] += 64;
            const float64 c = v_f32_ld_tnsr_b(pc, p);
            float64 ss;
            NL_REDUCE_ADD(v_f32_mac_b(a, a, c * c), IDX, ss);
            const float64 r = nl_rsqrt(ss * (1.0f / 128.0f) + eps);
            const float64 x0 = a * r * w0, x1 = c * r * w1;
            int5 oc = {0, h + NH * t, 0, 0, 0};
            v_f32_st_tnsr(oc, q, v_f32_mac_b(SWAP_HALVES(x0), sinv, x0 * cosv));
            oc[0] = 64;
            v_f32_st_tnsr(oc, q, x1);
        }
    }
}
