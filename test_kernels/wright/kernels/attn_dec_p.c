// SPDX-License-Identifier: AGPL-3.0-only
// Copyright Aaron Botsis
// Split-K flash decode, pass 1 (one node for all KV groups). For key chunk c (CH keys) and
// score row r (= group j = r / hpg, head h = r % hpg):
//   m[c,r] = max_k s*scale,  P[:, h, c, j] = exp(s*scale - m[c,r]) (bf16, sigma packed per 128
//   keys; 0 for keys >= pos0),  l[c,r] = sum_k P
// attn_dec_comb merges the chunk results.
// tensors: 0 sp (Lp, R) f32  1 meta [pos0, ..] | 2 P (CH, hpg, S, G) bf16  3 stm (64, S)  4 stl (64, S)
// (member c writes column c of the stats: one full 64-lane vector, lane r = row r; R <= 64)
// params: scale, hpg. index space: [S]
#include "common.h"

#define NINF (-3.0e38f)
#define CH 512

// -DQSA: an extra input km (Lp,) f32 key bias (0 / -1e30, the QSA selection) is added to every row.
#ifdef QSA
void main(tensor sp, tensor meta, tensor km, tensor P, tensor stm, tensor stl, float scale, int hpg)
#define SPLD(c) (v_f32_ld_tnsr_b(c, sp) + v_f32_ld_tnsr_b((int5){c[0], 0, 0, 0, 0}, km))
#else
void main(tensor sp, tensor meta, tensor P, tensor stm, tensor stl, float scale, int hpg)
#define SPLD(c) v_f32_ld_tnsr_b(c, sp)
#endif
{
    const int5 off = get_index_space_offset();
    const int5 end = off + get_index_space_size();
    scalar_cache_reset();
    const int R = get_dim_size(sp, 1);
    const int lim = meta_i32(meta, 0);
    const float limf = (float)lim;
    const float64 lanef = v_convert_i32_to_f32_b((int64)read_lane_id_4b_b(), SW_RHNE, 0, 1, 0);
    DECL_IDX(IDX)
    for (int ci = off[0]; ci < end[0]; ci++) {
        const int c0 = ci * CH;
        int h = 0, j = 0;
        float64 mv = NINF, lv = 0.0f;
        for (int r = 0; r < R; r++) {
            int5 c = {0, r, 0, 0, 0};
            // pass 1: chunk max
            float64 m0 = NINF, m1 = NINF, m2 = NINF, m3 = NINF;
            for (int i = c0; i < c0 + CH && i < lim; i += 256) {
                const float64 li = lanef + (float)i;
                c[0] = i;       m0 = v_f32_max_b(m0, v_f32_sel_less_f32_b(li, limf, SPLD(c), NINF));
                c[0] = i + 64;  m1 = v_f32_max_b(m1, v_f32_sel_less_f32_b(li + 64.0f, limf, SPLD(c), NINF));
                c[0] = i + 128; m2 = v_f32_max_b(m2, v_f32_sel_less_f32_b(li + 128.0f, limf, SPLD(c), NINF));
                c[0] = i + 192; m3 = v_f32_max_b(m3, v_f32_sel_less_f32_b(li + 192.0f, limf, SPLD(c), NINF));
            }
            float64 m = v_f32_max_b(v_f32_max_b(m0, m1), v_f32_max_b(m2, m3));
            NL_REDUCE_MAX(m, IDX, m);
            const float64 ms = m * scale;
            // pass 2: probabilities (unnormalized) + their sum
            float64 s0 = 0.0f, s1 = 0.0f;
            int5 pc = {0, h, ci, j, 0};
            for (int i = c0; i < c0 + CH; i += 128) {
                float64 ea = 0.0f, eb = 0.0f;
                if (i < lim) {
                    const float64 li = lanef + (float)i;
                    c[0] = i;
                    float64 va = SPLD(c);
                    c[0] = i + 64;
                    float64 vb = SPLD(c);
                    ea = v_f32_sel_less_f32_b(li, limf, nl_exp(v_f32_mac_b(va, scale, -ms)), 0.0f);
                    eb = v_f32_sel_less_f32_b(li + 64.0f, limf, nl_exp(v_f32_mac_b(vb, scale, -ms)), 0.0f);
                    s0 = s0 + ea;
                    s1 = s1 + eb;
                }
                pc[0] = i - c0;
                v_bf16_st_tnsr(pc, P, pack_sigma(ea, eb));
            }
            float64 tot, sum = s0 + s1;
            NL_REDUCE_ADD(sum, IDX, tot);
            mv = v_f32_sel_less_f32_b(lanef, (float)r - 0.5f, mv, ms);   // lanes >= r
            lv = v_f32_sel_less_f32_b(lanef, (float)r - 0.5f, lv, tot);
            if (++h == hpg) { h = 0; j++; }
        }
        int5 sc = {0, ci, 0, 0, 0};
        v_f32_st_tnsr(sc, stm, mv);
        v_f32_st_tnsr(sc, stl, lv);
    }
}
