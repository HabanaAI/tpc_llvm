// SPDX-License-Identifier: AGPL-3.0-only
// Copyright Aaron Botsis
// QSA indexer keys of a batched decode step (Qwen3.8-Flash-Next): row b is one token at position pos[b]. Its raw key
// (the indexer K rows of its projection column) is written to iks[:, b]; its block's key is recomputed from the
// block's earlier raw keys (tail[:, j, b], j < pos % 4, read from the raw-key page) and the new one:
//   kb = rope_{4 floor(pos/4)}(rmsnorm(sum_j k_j / 4) * w_k)   (rope on dims 0 .. 63; absent keys count as zero)
// and written to kbs[:, 0..3, b] (duplicated over the block's 4 key rows).
// tensors: 0 p (P, B) f32  1 tail (128, 4, B) f32  2 pos (B,) i32  3 cs (128, MAXC) f32 rope rows (absolute)
//          4 ixn (256) f32 [q norm | k norm] (1 + gamma) | 5 iks (128, B) f32  6 kbs (128, 4, B) f32
// params: eps, kofs (the K rows' offset in p). index space: [B]
#include "common.h"

void main(tensor p, tensor tail, tensor pos, tensor cs, tensor ixn, tensor iks, tensor kbs, float eps, int kofs)
{
    const int5 off = get_index_space_offset();
    const int5 end = off + get_index_space_size();
    scalar_cache_reset();
    DECL_IDX(IDX)
    int5 wc = {128, 0, 0, 0, 0};
    const float64 w0 = v_f32_ld_tnsr_b(wc, ixn);
    wc[0] = 192;
    const float64 w1 = v_f32_ld_tnsr_b(wc, ixn);
    for (int b = off[0]; b < end[0]; b++) {
        int5 pc5 = {b, 0, 0, 0, 0};
        const int position = s_i32_ld_g(gen_addr(pc5, pos));
        const int b0 = position >> 2, have = position & 3;
        int5 pc = {kofs, b, 0, 0, 0};
        const float64 ra = v_f32_ld_tnsr_b(pc, p);
        pc[0] = kofs + 64;
        const float64 rc = v_f32_ld_tnsr_b(pc, p);
        int5 oc = {0, b, 0, 0, 0};
        v_f32_st_tnsr(oc, iks, ra);
        oc[0] = 64;
        v_f32_st_tnsr(oc, iks, rc);
        float64 s0 = ra, s1 = rc;
        for (int j = 0; j < have; j++) {
            int5 tc = {0, j, b, 0, 0};
            s0 += v_f32_ld_tnsr_b(tc, tail);
            tc[0] = 64;
            s1 += v_f32_ld_tnsr_b(tc, tail);
        }
        s0 = s0 * 0.25f; s1 = s1 * 0.25f;
        float64 ss;
        NL_REDUCE_ADD(v_f32_mac_b(s0, s0, s1 * s1), IDX, ss);
        const float64 r = nl_rsqrt(ss * (1.0f / 128.0f) + eps);
        const float64 x0 = s0 * r * w0, x1 = s1 * r * w1;
        int5 cc = {0, 4 * b0, 0, 0, 0};
        const float64 cosv = v_f32_ld_tnsr_b(cc, cs);
        cc[0] = 64;
        const float64 sinv = v_f32_ld_tnsr_b(cc, cs);
        const float64 y0 = v_f32_mac_b(SWAP_HALVES(x0), sinv, x0 * cosv);
        for (int j = 0; j < 4; j++) {
            int5 kc = {0, j, b, 0, 0};
            v_f32_st_tnsr(kc, kbs, y0);
            kc[0] = 64;
            v_f32_st_tnsr(kc, kbs, x1);
        }
    }
}
