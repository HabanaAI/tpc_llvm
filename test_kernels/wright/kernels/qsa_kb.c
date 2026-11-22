// SPDX-License-Identifier: AGPL-3.0-only
// Copyright Aaron Botsis
// QSA indexer keys for a chunk (Qwen3.8-Flash-Next): raw keys + compressed block keys.
//   raw key of token u: k_u = p[kofs .. kofs+127, u - pos0] (index projection, no norm / rope)
//   block b (tokens 4b .. 4b+3): kb = rope_{4b}(rmsnorm(mean_u k_u) * w_k)   (rope on dims 0..63)
// Block keys are stored duplicated over their 4 key columns (key layout: scores / masks per key).
// Blocks b0 = pos0 / 4 .. b0 + NBc - 1 cover the chunk; keys before pos0 come from the raw-key cache,
// keys past the chunk's real tokens count as zero (a partial block is recomputed when it completes).
// tensors: 0 p (P, T) f32  1 ikc (128, Lc) f32 raw-key cache  2 cs (128, MAXC) f32 rope rows (absolute)
//          3 ixn (256) f32 [q norm | k norm] (1 + gamma)  4 meta i32 [pos0, n]
//        | 5 iks (128, T) f32 raw keys of the chunk  6 kbs (128, 4 NBc) f32 block keys (duplicated)
// params: eps, kofs. index space: [NBc]
#include "common.h"

void main(tensor p, tensor ikc, tensor cs, tensor ixn, tensor meta, tensor iks, tensor kbs, float eps, int kofs)
{
    const int5 off = get_index_space_offset();
    const int5 end = off + get_index_space_size();
    const int T = get_dim_size(p, 1);
    scalar_cache_reset();
    const int pos0 = meta_i32(meta, 0), n = meta_i32(meta, 1);
    const int b0 = pos0 >> 2;
    DECL_IDX(IDX)
    int5 wc = {128, 0, 0, 0, 0};
    const float64 w0 = v_f32_ld_tnsr_b(wc, ixn);
    wc[0] = 192;
    const float64 w1 = v_f32_ld_tnsr_b(wc, ixn);
    for (int q = off[0]; q < end[0]; q++) {
        const int b = b0 + q;
        float64 s0 = 0.0f, s1 = 0.0f;
        for (int j = 0; j < 4; j++) {
            const int u = 4 * b + j, tc = u - pos0;
            float64 a = 0.0f, c = 0.0f;
            if (u < pos0) {
                int5 kc = {0, u, 0, 0, 0};
                a = v_f32_ld_tnsr_b(kc, ikc);
                kc[0] = 64;
                c = v_f32_ld_tnsr_b(kc, ikc);
            } else if (tc < T) {
                int5 pc = {kofs, tc, 0, 0, 0};
                const float64 ra = v_f32_ld_tnsr_b(pc, p);
                pc[0] = kofs + 64;
                const float64 rc = v_f32_ld_tnsr_b(pc, p);
                int5 oc = {0, tc, 0, 0, 0};
                v_f32_st_tnsr(oc, iks, ra);                      // every chunk column is some block's
                oc[0] = 64;
                v_f32_st_tnsr(oc, iks, rc);
                if (tc < n) { a = ra; c = rc; }
            }
            s0 += a; s1 += c;
        }
        s0 = s0 * 0.25f; s1 = s1 * 0.25f;
        float64 ss;
        NL_REDUCE_ADD(v_f32_mac_b(s0, s0, s1 * s1), IDX, ss);
        const float64 r = nl_rsqrt(ss * (1.0f / 128.0f) + eps);
        const float64 x0 = s0 * r * w0, x1 = s1 * r * w1;
        int5 cc = {0, 4 * b, 0, 0, 0};
        const float64 cosv = v_f32_ld_tnsr_b(cc, cs);
        cc[0] = 64;
        const float64 sinv = v_f32_ld_tnsr_b(cc, cs);
        const float64 y0 = v_f32_mac_b(SWAP_HALVES(x0), sinv, x0 * cosv);
        for (int j = 0; j < 4; j++) {
            int5 oc = {0, 4 * q + j, 0, 0, 0};
            v_f32_st_tnsr(oc, kbs, y0);
            oc[0] = 64;
            v_f32_st_tnsr(oc, kbs, x1);
        }
    }
}
