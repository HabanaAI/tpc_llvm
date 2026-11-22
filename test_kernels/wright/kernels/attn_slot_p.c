// SPDX-License-Identifier: AGPL-3.0-only
// Copyright Aaron Botsis
// Slot attention, softmax partials (after attn_dec_p). Slot s holds C keys at positions key0 ..
// key0 + C - 1 and M query columns; column m is token qpos0 + m / hpg. Key i is visible to
// column m when key0 + i <= its position and key0 + i < klen (keys past the chunk's real tokens
// are stale). For each column:
//   mx = max_visible s * scale,  P[i] = exp(s[i] * scale - mx) (bf16, sigma packed per 128 keys
//   (natural key order with P_LINEAR: mla_slot_p),
//   0 when not visible),  l = sum_i P[i]
// A column with no visible keys gets mx = NINF * scale, P = 0 and l = 0.
// slots (8, S) i32 per slot: [group, qcol0, qpos0, key0, klen, 0, kmin, 0] (kmin, P_LINEAR and KMIN
// builds only: keys before position kmin are not visible either, e.g. a window that does not start on
// a page; 0 in every table that has no window)
// tensors: 0 sc (C, M, S) f32  1 slots | 2 P (C, M, S) bf16  3 mx (Mp, S) f32  4 l (Mp, S) f32
// (Mp = M rounded up to 64; lane r of stats column block b is column 64 b + r)
// WINDOW builds (ds_slot_pw.c with P_LINEAR, attn_slot_pd.c): slot field 7 is a window W (0: none); column m
// sees only keys at positions >= its position - W + 1.
// BLOCK_ALL builds (attn_slot_pd.c): every column sees every key below klen (a bidirectional block).
// PER_COLUMN builds (ds_slot_pw1.c, ds_slot_p1.c): one member per column, index space [M, S].
// params: scale, hpg. index space: [ceil(M / 64), S]
#include "common.h"

#define NINF (-3.0e38f)

void main(tensor sc, tensor slots, tensor P, tensor mx, tensor l, float scale, int hpg)
{
    const int5 off = get_index_space_offset();
    const int5 end = off + get_index_space_size();
    const int C = get_dim_size(sc, 0), M = get_dim_size(sc, 1);
    const float64 lanef = v_convert_i32_to_f32_b((int64)read_lane_id_4b_b(), SW_RHNE, 0, 1, 0);
    DECL_IDX(IDX)
    scalar_cache_reset();
    for (int s = off[1]; s < end[1]; s++) {
        int5 fc = {2, s, 0, 0, 0};
        const int qpos0 = s_i32_ld_g(gen_addr(fc, slots));
        fc[0] = 3;
        const int key0 = s_i32_ld_g(gen_addr(fc, slots));
        fc[0] = 4;
        const int klen = s_i32_ld_g(gen_addr(fc, slots));
#if defined(P_LINEAR) || defined(KMIN)
        fc[0] = 6;
        const int kmin = s_i32_ld_g(gen_addr(fc, slots));
#ifdef WINDOW
        const float lof0 = (float)(kmin > key0 ? kmin - key0 : 0);  // first visible key
        fc[0] = 7;
        const int win = s_i32_ld_g(gen_addr(fc, slots));
#else
        const float lof = (float)(kmin > key0 ? kmin - key0 : 0);  // first visible key
#endif
#else
        const float lof = 0.0f;
#endif
        for (int cb = off[0]; cb < end[0]; cb++) {
#ifdef PER_COLUMN
            const int c0 = cb, nc = 1;                    // one column per member
#else
            const int c0 = cb * 64, nc = 64;
#endif
            float64 mv = NINF * scale, lv = 0.0f;
            // token / head of the block's first column
            int t = 0, h = c0;
            while (h >= hpg) { h -= hpg; t++; }
            for (int r = 0; r < nc && c0 + r < M; r++) {
                const int m = c0 + r;
#ifdef BLOCK_ALL
                int lim = klen;
#else
                int lim = qpos0 + t + 1 < klen ? qpos0 + t + 1 : klen;
#endif
                lim -= key0;
                lim = lim < 0 ? 0 : (lim > C ? C : lim);
                const float limf = (float)lim;
#ifdef WINDOW
                const int wfirst = qpos0 + t - win + 1 - key0;
                const float lof = (float)(win > 0 && wfirst > (int)lof0 ? wfirst : (int)lof0);
#endif
                int5 c = {0, m, s, 0, 0};
                // pass 1: max over the visible keys
                float64 m0 = NINF, m1 = NINF, m2 = NINF, m3 = NINF;
                for (int i = 0; i < lim; i += 256) {
                    const float64 li = lanef + (float)i;
                    c[0] = i;       m0 = v_f32_max_b(m0, v_f32_sel_less_f32_b(li, lof, NINF, v_f32_sel_less_f32_b(li, limf, v_f32_ld_tnsr_b(c, sc), NINF)));
                    c[0] = i + 64;  m1 = v_f32_max_b(m1, v_f32_sel_less_f32_b(li + 64.0f, lof, NINF, v_f32_sel_less_f32_b(li + 64.0f, limf, v_f32_ld_tnsr_b(c, sc), NINF)));
                    c[0] = i + 128; m2 = v_f32_max_b(m2, v_f32_sel_less_f32_b(li + 128.0f, lof, NINF, v_f32_sel_less_f32_b(li + 128.0f, limf, v_f32_ld_tnsr_b(c, sc), NINF)));
                    c[0] = i + 192; m3 = v_f32_max_b(m3, v_f32_sel_less_f32_b(li + 192.0f, lof, NINF, v_f32_sel_less_f32_b(li + 192.0f, limf, v_f32_ld_tnsr_b(c, sc), NINF)));
                }
                float64 mm = v_f32_max_b(v_f32_max_b(m0, m1), v_f32_max_b(m2, m3));
                NL_REDUCE_MAX(mm, IDX, mm);
                const float64 ms = mm * scale;
                // pass 2: unnormalized probabilities and their sum
                float64 s0 = 0.0f, s1 = 0.0f;
                int5 pc = {0, m, s, 0, 0};
                for (int i = 0; i < C; i += 128) {
                    float64 ea = 0.0f, eb = 0.0f;
                    if (i < lim) {
                        const float64 li = lanef + (float)i;
                        c[0] = i;
                        const float64 va = v_f32_ld_tnsr_b(c, sc);
                        c[0] = i + 64;
                        const float64 vb = v_f32_ld_tnsr_b(c, sc);
                        ea = v_f32_sel_less_f32_b(li, limf, nl_exp(v_f32_mac_b(va, scale, -ms)), 0.0f);
                        eb = v_f32_sel_less_f32_b(li + 64.0f, limf, nl_exp(v_f32_mac_b(vb, scale, -ms)), 0.0f);
                        ea = v_f32_sel_less_f32_b(li, lof, 0.0f, ea);
                        eb = v_f32_sel_less_f32_b(li + 64.0f, lof, 0.0f, eb);
                        s0 = s0 + ea;
                        s1 = s1 + eb;
                    }
                    pc[0] = i;
#ifdef P_LINEAR
                    v_bf16_st_tnsr(pc, P, cvt_bf16_lin(ea, eb));
#else
                    v_bf16_st_tnsr(pc, P, pack_sigma(ea, eb));
#endif
                }
                float64 tot, sum = s0 + s1;
                NL_REDUCE_ADD(sum, IDX, tot);
                mv = v_f32_sel_less_f32_b(lanef, (float)r - 0.5f, mv, ms);   // lanes >= r
                lv = v_f32_sel_less_f32_b(lanef, (float)r - 0.5f, lv, tot);
                if (++h == hpg) { h = 0; t++; }
            }
            int5 stc = {c0, s, 0, 0, 0};
#ifdef PER_COLUMN
            v_f32_st_tnsr_partial(stc, mx, mv, 0, 0);
            v_f32_st_tnsr_partial(stc, l, lv, 0, 0);
#else
            v_f32_st_tnsr(stc, mx, mv);
            v_f32_st_tnsr(stc, l, lv);
#endif
        }
    }
}
