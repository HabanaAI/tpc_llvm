// SPDX-License-Identifier: AGPL-3.0-only
// Copyright Aaron Botsis
// DeepSeek-V4.1 compressed attention of a prefill slot over every visible entry, softmax partials with each
// column's top-512 list as a mask (mla_slot_pk with DeepSeek's lists): entry key0 + i is kept for column col when
// it is visible (key0 + i < (its position + 1) / ratio and < klen) and its index score key k = mkey(score[key0 + i,
// col]) is above the column's threshold th (ix_thr), or equal to it at a position <= the column's tie cut
// (ix_list's slot lane 7): exactly the entries ix_list lists.
//   mx = max_kept s * scale,  P[i] = exp(s[i] * scale - mx) (bf16, natural key order, 0 when not kept),
//   l = sum_i P[i];  a column with no kept entry gets mx = NINF * scale, P = 0 and l = 0.
// slots (8, S) i32 per slot: [group, qcol0, qpos0, key0, klen, qtok0, ratio, 0]
// tensors: 0 sc (C, M, S) f32  1 slots  2 score (K, T) f32  3 thr (64, T) i32  4 lslots (8, T) i32
//        | 5 P (C, M, S) bf16  6 mx (Mp, S) f32  7 l (Mp, S) f32
// params: scale, hpg. index space: [ceil(M / 64), S]
#include "common.h"

#define NINF (-3.0e38f)

static inline uint64 mkey(float64 v)
{
    const uint64 b = as_uint64(v);
    return as_uint64(v_f32_sel_less_f32_b(v, 0.0f, as_float64(~b), as_float64(b | (uint64)0x80000000u)));
}

// 1.0f where entry i of the column is visible (i < lim) and listed, else 0
#define KEPT(li, i)                                                                                  \
    ({ int5 kc_ = {key0 + (i), col, 0, 0, 0};                                                        \
       const uint64 k_ = mkey(v_f32_ld_tnsr_b(kc_, score));                                         \
       const float64 tie_ = v_f32_sel_grt_f32_b(li + (float)key0, cutf, 0.0f, 1.0f);               \
       const float64 eq_ = as_float64(v_u32_sel_eq_u32_b(k_, th, as_uint64(tie_), (uint64)0));     \
       const float64 gt_ = as_float64(v_u32_sel_less_u32_b(th, k_, one, (uint64)0));               \
       v_f32_sel_less_f32_b(li, limf, v_f32_max_b(gt_, eq_), 0.0f); })

void main(tensor sc, tensor slots, tensor score, tensor thr, tensor lslots, tensor P, tensor mx, tensor l,
          float scale, int hpg)
{
    const int5 off = get_index_space_offset();
    const int5 end = off + get_index_space_size();
    const int C = get_dim_size(sc, 0), M = get_dim_size(sc, 1);
    const float64 lanef = v_convert_i32_to_f32_b((int64)read_lane_id_4b_b(), SW_RHNE, 0, 1, 0);
    const uint64 one = as_uint64((float64)1.0f);
    DECL_IDX(IDX)
    scalar_cache_reset();
    for (int s = off[1]; s < end[1]; s++) {
        int5 fc = {2, s, 0, 0, 0};
        const int qpos0 = s_i32_ld_g(gen_addr(fc, slots));
        fc[0] = 3;
        const int key0 = s_i32_ld_g(gen_addr(fc, slots));
        fc[0] = 4;
        const int klen = s_i32_ld_g(gen_addr(fc, slots));
        fc[0] = 5;
        const int tok0 = s_i32_ld_g(gen_addr(fc, slots));
        fc[0] = 6;
        int ratio = s_i32_ld_g(gen_addr(fc, slots));
        ratio = ratio > 0 ? ratio : 1;
        for (int cb = off[0]; cb < end[0]; cb++) {
            float64 mv = NINF * scale, lv = 0.0f;
            // token / head of the block's first column
            int t = 0, h = cb * 64;
            while (h >= hpg) { h -= hpg; t++; }
            for (int r = 0; r < 64 && cb * 64 + r < M; r++) {
                const int m = cb * 64 + r;
                const int seen = (qpos0 + t + 1) / ratio;
                int lim = seen < klen ? seen : klen;
                lim -= key0;
                lim = lim < 0 ? 0 : (lim > C ? C : lim);
                const float limf = (float)lim;
                const int col = tok0 + t;
                int5 tc = {0, col, 0, 0, 0};
                const uint64 th = __builtin_bit_cast(uint64, v_i32_ld_tnsr_b(tc, thr));
                int5 cc = {7, col, 0, 0, 0};
                const int cut = s_i32_ld_g(gen_addr(cc, lslots));
                const float cutf = cut > 16777216 ? 3.0e38f : (float)cut;
                int5 c = {0, m, s, 0, 0};
                // pass 1: max over the kept entries
                float64 m0 = NINF, m1 = NINF, m2 = NINF, m3 = NINF;
                for (int i = 0; i < lim; i += 256) {
                    const float64 li = lanef + (float)i;
                    c[0] = i;       m0 = v_f32_max_b(m0, v_f32_sel_eq_f32_b(KEPT(li, i), 1.0f, v_f32_ld_tnsr_b(c, sc), NINF));
                    c[0] = i + 64;  m1 = v_f32_max_b(m1, v_f32_sel_eq_f32_b(KEPT(li + 64.0f, i + 64), 1.0f, v_f32_ld_tnsr_b(c, sc), NINF));
                    c[0] = i + 128; m2 = v_f32_max_b(m2, v_f32_sel_eq_f32_b(KEPT(li + 128.0f, i + 128), 1.0f, v_f32_ld_tnsr_b(c, sc), NINF));
                    c[0] = i + 192; m3 = v_f32_max_b(m3, v_f32_sel_eq_f32_b(KEPT(li + 192.0f, i + 192), 1.0f, v_f32_ld_tnsr_b(c, sc), NINF));
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
                        ea = v_f32_sel_eq_f32_b(KEPT(li, i), 1.0f, nl_exp(v_f32_mac_b(va, scale, -ms)), 0.0f);
                        eb = v_f32_sel_eq_f32_b(KEPT(li + 64.0f, i + 64), 1.0f, nl_exp(v_f32_mac_b(vb, scale, -ms)), 0.0f);
                        s0 = s0 + ea;
                        s1 = s1 + eb;
                    }
                    pc[0] = i;
                    v_bf16_st_tnsr(pc, P, cvt_bf16_lin(ea, eb));
                }
                float64 tot, sum = s0 + s1;
                NL_REDUCE_ADD(sum, IDX, tot);
                mv = v_f32_sel_less_f32_b(lanef, (float)r - 0.5f, mv, ms);   // lanes >= r
                lv = v_f32_sel_less_f32_b(lanef, (float)r - 0.5f, lv, tot);
                if (++h == hpg) { h = 0; t++; }
            }
            int5 stc = {cb * 64, s, 0, 0, 0};
            v_f32_st_tnsr(stc, mx, mv);
            v_f32_st_tnsr(stc, l, lv);
        }
    }
}
