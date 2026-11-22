// SPDX-License-Identifier: AGPL-3.0-only
// Copyright Aaron Botsis
// Sampling, pass 1 of 5 (scan): one member per (vocabulary chunk, row). The row's logits are cut
// into NC chunks of whole 64-token vectors; a member scans its chunk twice:
//   1. the chunk's finite maximum m_c, the count of NaN / +inf logits and per-lane candidates:
//      the top DEPTH order keys and token ids (descending; equal keys keep the lower id first)
//      when the row filters (top-k / top-p) or lists top logprobs, else only each lane's top;
//   2. when the row samples or reports logprobs: the sum of the tempered weights
//      exp((x - m_c) / T), with logprobs the sum of exp(x - m_c), over finite logits, and
//      (sampling or listing rows) every
//      token's order key and tempered weight (wkey, wt: what the later passes search and draw
//      from, 0 weight for non-finite and absent tokens).
// Greedy rows without logprobs only need pass 1's lane tops.
// tensors: 0 logits (V, B) f32  1 params (8, B) f32 (see samp_select; lane 6: chosen-token
//          logprob wanted)
//        | 2 ckey (64, DEPTH, NC, B) i32  3 cidx (same) i32  4 cw (same) f32: candidate weights
//          exp((x - m_c) / T)  5 stats (64, NC, B) f32: lane 0 m_c (NO_MAX when the chunk has no
//          finite logit), 1 sum exp(x - m_c), 2 sum exp((x - m_c) / T), 3 NaN / +inf count
//          6 wkey (Vp, B) i32  7 wt (Vp, B) f32 (Vp: V rounded up to 64; padding lanes get key
//            KEY_MIN and weight 0)
// index space: [NC, B]
#include "samp.h"

// one lane-top insertion level j of key k (id) into sk/si
#define INSERT(j)                                                                            \
    {                                                                                       \
        const int64 hk_ = v_i32_sel_grt_i32_b(k, sk[j], k, sk[j]);                           \
        const int64 hi_ = v_i32_sel_grt_i32_b(k, sk[j], id, si[j]);                          \
        const int64 lk_ = v_i32_sel_grt_i32_b(k, sk[j], sk[j], k);                           \
        const int64 li_ = v_i32_sel_grt_i32_b(k, sk[j], si[j], id);                          \
        sk[j] = hk_; si[j] = hi_; k = lk_; id = li_;                                         \
    }

// pass 1 over the chunk with LEVELS insertion levels
#define PASS1(LEVELS)                                                                        \
    for (int v = v0; v < v1; v++) {                                                          \
        xc[0] = v * 64;                                                                      \
        const float64 x = v_f32_ld_tnsr_b(xc, logits);                                       \
        const int64 bits = as_int64(x);                                                      \
        const int64 mag = bits & (int64)KEY_MAX;                                             \
        const float64 tokf = lanef + (float)(v * 64);                                        \
        const float64 live = v_f32_sel_less_f32_b(tokf, (float)V, 1.0f, 0.0f);               \
        const float64 nan = v_f32_sel_grt_i32_b(mag, (int64)INF_BITS, live, 0.0f);           \
        bad += v_f32_sel_eq_i32_b(bits, (int64)INF_BITS, live, nan);   /* NaN, +inf */       \
        const float64 fin = v_f32_sel_less_i32_b(mag, (int64)INF_BITS, live, 0.0f);          \
        mx = v_f32_sel_grt_f32_b(fin, 0.5f, v_f32_max_b(mx, x), mx);                         \
        int64 k = v_i32_sel_less_f32_b(tokf, (float)V, order_key(x), (int64)KEY_MIN);        \
        int64 id = lane + (int64)(v * 64);                                                   \
        _Pragma("unroll")                                                                    \
        for (int j = 0; j < LEVELS; j++) INSERT(j)                                           \
    }

void main(tensor logits, tensor params, tensor ckey, tensor cidx, tensor cw, tensor stats,
          tensor wkey, tensor wt)
{
    const int5 off = get_index_space_offset();
    const int5 end = off + get_index_space_size();
    const int V = get_dim_size(logits, 0);
    const int NC = get_dim_size(ckey, 2);
    const int nvec = (V + 63) / 64;
    const int per = (nvec + NC - 1) / NC;
    const int64 lane = (int64)read_lane_id_4b_b();
    const float64 lanef = v_convert_i32_to_f32_b(lane, SW_RHNE, 0, 1, 0);
    const float64 nomax = NO_MAX;
    DECL_IDX(IDX)
    scalar_cache_reset();
    for (int b = off[1]; b < end[1]; b++) {
        int5 pc = {0, b, 0, 0, 0};
        const float T = s_f32_ld_g(gen_addr(pc, params));
        pc[0] = 1; const float top_p = s_f32_ld_g(gen_addr(pc, params));
        pc[0] = 2; const float top_k = s_f32_ld_g(gen_addr(pc, params));
        pc[0] = 4; const float mode = s_f32_ld_g(gen_addr(pc, params));
        pc[0] = 5; const float ntop = s_f32_ld_g(gen_addr(pc, params));
        pc[0] = 6; const float want_lp = s_f32_ld_g(gen_addr(pc, params));
        const bool sample = mode > 0.5f && T > 0.0f;
        const bool listing = ntop > 0.5f;
        const bool cands = (sample && (top_k > 0.5f || top_p < 1.0f)) || listing;
        const bool sums = sample || listing || want_lp > 0.5f;
        const bool logprobs = listing || want_lp > 0.5f;
        const bool weights = sample || listing;
        const float invT = T > 0.0f ? 1.0f / T : 1.0f;
        for (int c = off[0]; c < end[0]; c++) {
            const int v0 = c * per, v1 = v0 + per < nvec ? v0 + per : nvec;
            int64 sk[DEPTH], si[DEPTH];
            #pragma unroll
            for (int j = 0; j < DEPTH; j++) { sk[j] = KEY_MIN; si[j] = -1; }
            float64 mx = nomax, bad = 0.0f;
            int5 xc = {0, b, 0, 0, 0};
            if (cands) {
                PASS1(DEPTH)
            } else {
                PASS1(1)
            }
            float64 m;
            NL_REDUCE_MAX(mx, IDX, m);
            float64 nbad;
            NL_REDUCE_ADD(bad, IDX, nbad);
            float64 z1 = 0.0f, zt = 0.0f;
#define PASS2(Z1)                                                                                        \
    for (int v = v0; v < v1; v++) {                                                                      \
        xc[0] = v * 64;                                                                                  \
        const float64 x = v_f32_ld_tnsr_b(xc, logits);                                                   \
        const int64 mag = as_int64(x) & (int64)KEY_MAX;                                                  \
        const float64 tokf = lanef + (float)(v * 64);                                                    \
        float64 d = v_f32_sel_less_i32_b(mag, (int64)INF_BITS, x - m, -1000.0f);                         \
        d = v_f32_sel_less_f32_b(tokf, (float)V, d, -1000.0f);                                           \
        if (Z1) z1 += weight(d);                                                                         \
        const float64 w = weight(d * invT);                                                              \
        zt += w;                                                                                         \
        if (weights) {                                                                                   \
            v_i32_st_tnsr(xc, wkey, v_i32_sel_less_f32_b(tokf, (float)V, order_key(x), (int64)KEY_MIN)); \
            v_f32_st_tnsr(xc, wt, w);                                                                    \
        }                                                                                                \
    }
            if (sums && logprobs) {
                PASS2(1)
            } else if (sums) {
                PASS2(0)
            }
            float64 s1, st;
            NL_REDUCE_ADD(z1, IDX, s1);
            NL_REDUCE_ADD(zt, IDX, st);
            float64 out = v_f32_sel_eq_f32_b(lanef, 0.0f, m, 0.0f);
            out = v_f32_sel_eq_f32_b(lanef, 1.0f, s1, out);
            out = v_f32_sel_eq_f32_b(lanef, 2.0f, st, out);
            out = v_f32_sel_eq_f32_b(lanef, 3.0f, nbad, out);
            int5 sc = {0, c, b, 0, 0};
            v_f32_st_tnsr(sc, stats, out);
            int5 cc = {0, 0, c, b, 0};
            #pragma unroll
            for (int j = 0; j < DEPTH; j++) {
                cc[1] = j;
                v_i32_st_tnsr(cc, ckey, sk[j]);
                v_i32_st_tnsr(cc, cidx, si[j]);
                const float64 val = key_value(sk[j]);
                const int64 mag = as_int64(val) & (int64)KEY_MAX;
                float64 d = v_f32_sel_less_i32_b(mag, (int64)INF_BITS, val - m, -1000.0f);
                d = v_f32_sel_grt_i32_b(sk[j], (int64)KEY_MIN, d, -1000.0f);      // absent
                v_f32_st_tnsr(cc, cw, weight(d * invT));
            }
        }
    }
}
