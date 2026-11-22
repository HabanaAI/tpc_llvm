// SPDX-License-Identifier: AGPL-3.0-only
// Copyright Aaron Botsis
// Sampling, pass 2 of 5 (select): one member per row merges the scan's chunk statistics and
// candidates (the per-lane top 8 of every chunk) into
//   - the row maximum m, the log-sum-exp of the raw logits (lse) and the tempered total
//     ZT = sum exp((x - m) / T);
//   - the greedy token (torch.argmax: the largest key, then the lowest id);
//   - the sampling threshold key theta (tokens with key >= theta are kept):
//     theta = max(theta_k, theta_p) with theta_k the largest key with count(key >= theta_k) >=
//     top_k (ties at the cutoff kept) and theta_p the largest key with mass(key >= theta_p) >=
//     top_p * ZT (the nucleus of the full tempered distribution, ties at its crossing kept);
//   - the ntop (<= NMAX) largest logits in descending order (ties by id) with raw logprobs.
// The candidates hold every token above a key unless some lane of some chunk kept DEPTH
// candidates all above it; rows where that happens at theta or at the ntop-th key are flagged
// for samp_full, which recomputes them over the whole vocabulary.
// Rows with NaN or +inf logits or without a finite logit, mode 0 or T <= 0 are greedy.
// params: (8, B) f32 [T, top_p (>= 1: none), top_k (0: none), uniform, mode, ntop, 0, 0]
// tensors: 0 params  1 ckey  2 cidx  3 cw  4 stats (see samp_scan)
//        | 5 sel_i (16, B) i32 [theta, flags, greedy token, sample]
//          6 sel_f (16, B) f32 [m, lse, ZT, 1 / T, uniform, top_k, top_p, ntop]
//          7 top_idx (NMAX, B) i32  8 top_lp (NMAX, B) f32
// flags: 1 theta needs samp_full, 2 the top list needs samp_full. index space: [B]
#include "samp.h"

__local__ float64 SC[NC_MAX];  // per-chunk scale of the candidate weights: exp((m_c - m) / T)

// the largest key K in [lo, KEY_MAX] with PRED(K) true, given PRED(lo) (else lo); PRED's
// argument is `mid`, an int64 lane-broadcast key
#define KEY_SEARCH(lo, PRED)                                                                 \
    {                                                                                       \
        int64 hi_ = KEY_MAX;                                                                 \
        for (int it_ = 0; it_ < 32; it_++) {                                                 \
            const int64 d_ = hi_ - lo;                                                       \
            const int64 mid = lo + ((d_ >> 1) & (int64)KEY_MAX) + 1;                          \
            float64 ok_;                                                                     \
            PRED(mid, ok_)                                                                   \
            ok_ = v_f32_sel_less_i32_b(lo, hi_, ok_, 0.0f);                                  \
            const int64 nlo_ = v_i32_sel_grt_f32_b(ok_, 0.5f, mid, lo);                      \
            hi_ = v_i32_sel_grt_f32_b(ok_, 0.5f, hi_, v_i32_sel_less_i32_b(lo, hi_, mid - 1, hi_)); \
            lo = nlo_;                                                                       \
        }                                                                                   \
    }

void main(tensor params, tensor ckey, tensor cidx, tensor cw, tensor stats, tensor sel_i,
          tensor sel_f, tensor top_idx, tensor top_lp)
{
    const int5 off = get_index_space_offset();
    const int5 end = off + get_index_space_size();
    const int NC = get_dim_size(ckey, 2);
    const float64 lanef = v_convert_i32_to_f32_b((int64)read_lane_id_4b_b(), SW_RHNE, 0, 1, 0);
    DECL_IDX(IDX)
    scalar_cache_reset();
    for (int b = off[0]; b < end[0]; b++) {
        int5 pc = {0, b, 0, 0, 0};
        const float T = s_f32_ld_g(gen_addr(pc, params));
        pc[0] = 1; const float top_p = s_f32_ld_g(gen_addr(pc, params));
        pc[0] = 2; const float top_k = s_f32_ld_g(gen_addr(pc, params));
        pc[0] = 3; const float uniform = s_f32_ld_g(gen_addr(pc, params));
        pc[0] = 4; const float mode = s_f32_ld_g(gen_addr(pc, params));
        pc[0] = 5; const int ntop = (int)s_f32_ld_g(gen_addr(pc, params));
        const float invT = T > 0.0f ? 1.0f / T : 1.0f;

        // ---- row statistics
        float64 M = NO_MAX;
        for (int c = 0; c < NC; c++) {
            int5 sc = {0, c, b, 0, 0};
            M = v_f32_max_b(M, v_f32_ld_tnsr_b(sc, stats));
        }
        float64 m;
        SAMP_LANE(M, 0, m)
        float64 z1 = 0.0f, zt = 0.0f, bad = 0.0f;
        for (int c = 0; c < NC; c++) {
            int5 sc = {0, c, b, 0, 0};
            const float64 st = v_f32_ld_tnsr_b(sc, stats);
            float64 mc, z1c, ztc, badc;
            SAMP_LANE(st, 0, mc)
            SAMP_LANE(st, 1, z1c)
            SAMP_LANE(st, 2, ztc)
            SAMP_LANE(st, 3, badc)
            const float64 e = weight((mc - m) * invT);
            SC[c] = e;
            z1 += z1c * weight(mc - m);
            zt += ztc * e;
            bad += badc;
        }
        const float64 lse = m + nl_log(v_f32_max_b(z1, 1.0e-30f));
        const float64 valid = v_f32_sel_grt_f32_b(m, 0.5f * NO_MAX, v_f32_sel_less_f32_b(bad, 0.5f, 1.0f, 0.0f), 0.0f);
        const float64 sample = (mode > 0.5f && T > 0.0f) ? valid : (float64)0.0f;

        // ---- greedy token: the largest key, then the lowest id (chunk tops are each chunk's
        // largest keys with their lowest ids)
        int64 bk = KEY_MIN;
        for (int c = 0; c < NC; c++) {
            int5 cc = {0, 0, c, b, 0};
            bk = v_i32_max_b(bk, v_i32_ld_tnsr_b(cc, ckey));
        }
        NL_REDUCE_I32(bk, IDX, bk, v_i32_max_b)
        int64 bi = KEY_MAX;
        for (int c = 0; c < NC; c++) {
            int5 cc = {0, 0, c, b, 0};
            const int64 k = v_i32_ld_tnsr_b(cc, ckey);
            bi = v_i32_min_b(bi, v_i32_sel_eq_i32_b(k, bk, v_i32_ld_tnsr_b(cc, cidx), (int64)KEY_MAX));
        }
        NL_REDUCE_I32(bi, IDX, bi, v_i32_min_b)

        // ---- sampling threshold
        int64 theta = KEY_MIN + 1;
#define COUNT_GEQ(mid, out)                                                                  \
        { float64 n_ = 0.0f;                                                                 \
          for (int c = 0; c < NC; c++) {                                                     \
              int5 cc = {0, 0, c, b, 0};                                                     \
              for (int j = 0; j < DEPTH; j++) {                                              \
                  cc[1] = j;                                                                 \
                  n_ += v_f32_sel_geq_i32_b(v_i32_ld_tnsr_b(cc, ckey), mid, 1.0f, 0.0f);     \
              }                                                                              \
          }                                                                                  \
          NL_REDUCE_ADD(n_, IDX, n_)                                                         \
          out = v_f32_sel_less_f32_b(n_, top_k - 0.5f, 0.0f, 1.0f); }
#define MASS_GEQ(mid, out)                                                                   \
        { float64 ms_ = 0.0f;                                                                 \
          for (int c = 0; c < NC; c++) {                                                     \
              int5 cc = {0, 0, c, b, 0};                                                     \
              float64 t_ = 0.0f;                                                             \
              for (int j = 0; j < DEPTH; j++) {                                              \
                  cc[1] = j;                                                                 \
                  t_ += v_f32_sel_geq_i32_b(v_i32_ld_tnsr_b(cc, ckey), mid, v_f32_ld_tnsr_b(cc, cw), 0.0f); \
              }                                                                              \
              ms_ = v_f32_mac_b(t_, SC[c], ms_);                                               \
          }                                                                                  \
          NL_REDUCE_ADD(ms_, IDX, ms_)                                                         \
          out = v_f32_sel_less_f32_b(ms_, target, 0.0f, 1.0f); }
        if (top_k > 0.5f)
            KEY_SEARCH(theta, COUNT_GEQ)
        if (top_p < 1.0f) {
            const float64 target = zt * top_p;
            KEY_SEARCH(theta, MASS_GEQ)
        }
        // incomplete candidates: a chunk lane whose last candidate lies above theta
        float64 over = 0.0f;
        for (int c = 0; c < NC; c++) {
            int5 cc = {0, DEPTH - 1, c, b, 0};
            over += v_f32_sel_grt_i32_b(v_i32_ld_tnsr_b(cc, ckey), theta, 1.0f, 0.0f);
        }
        NL_REDUCE_ADD(over, IDX, over)
        const bool filtered = top_k > 0.5f || top_p < 1.0f;
        int64 flags = (int64)0;
        if (filtered)
            flags = v_i32_sel_grt_f32_b(over, 0.5f, v_i32_sel_grt_f32_b(sample, 0.5f, (int64)1, (int64)0), (int64)0);

        // ---- top ntop logits, descending (ties by id), each the largest after the previous
        int64 oi = -1;
        float64 ol = 0.0f;
        int64 pk = KEY_MAX, pi = -1;
        for (int n = 0; n < ntop; n++) {
            int64 nk = KEY_MIN;
            for (int c = 0; c < NC; c++) {
                int5 cc = {0, 0, c, b, 0};
                for (int j = 0; j < DEPTH; j++) {
                    cc[1] = j;
                    const int64 k = v_i32_ld_tnsr_b(cc, ckey), i = v_i32_ld_tnsr_b(cc, cidx);
                    const int64 tie = v_i32_sel_grt_i32_b(i, pi, k, (int64)KEY_MIN);
                    const int64 e = v_i32_sel_less_i32_b(k, pk, k, v_i32_sel_eq_i32_b(k, pk, tie, (int64)KEY_MIN));
                    nk = v_i32_max_b(nk, e);
                }
            }
            NL_REDUCE_I32(nk, IDX, nk, v_i32_max_b)
            int64 ni = KEY_MAX;
            for (int c = 0; c < NC; c++) {
                int5 cc = {0, 0, c, b, 0};
                for (int j = 0; j < DEPTH; j++) {
                    cc[1] = j;
                    const int64 k = v_i32_ld_tnsr_b(cc, ckey), i = v_i32_ld_tnsr_b(cc, cidx);
                    const int64 after = v_i32_sel_eq_i32_b(k, pk, v_i32_sel_grt_i32_b(i, pi, i, (int64)KEY_MAX), i);
                    ni = v_i32_min_b(ni, v_i32_sel_eq_i32_b(k, nk, after, (int64)KEY_MAX));
                }
            }
            NL_REDUCE_I32(ni, IDX, ni, v_i32_min_b)
            oi = v_i32_sel_eq_f32_b(lanef, (float)n, ni, oi);
            ol = v_f32_sel_eq_f32_b(lanef, (float)n, key_value(nk) - lse, ol);
            pk = nk;
            pi = ni;
        }
        if (ntop > 0) {
            float64 miss = 0.0f;
            for (int c = 0; c < NC; c++) {
                int5 cc = {0, DEPTH - 1, c, b, 0};
                miss += v_f32_sel_grt_i32_b(v_i32_ld_tnsr_b(cc, ckey), pk, 1.0f, 0.0f);
            }
            NL_REDUCE_ADD(miss, IDX, miss)
            flags = flags | v_i32_sel_grt_f32_b(miss, 0.5f, (int64)2, (int64)0);
        }

        // ---- outputs
        int64 oi_ = v_i32_sel_eq_f32_b(lanef, 0.0f, theta, (int64)0);
        oi_ = v_i32_sel_eq_f32_b(lanef, 1.0f, flags, oi_);
        oi_ = v_i32_sel_eq_f32_b(lanef, 2.0f, bi, oi_);
        oi_ = v_i32_sel_eq_f32_b(lanef, 3.0f, v_i32_sel_grt_f32_b(sample, 0.5f, (int64)1, (int64)0), oi_);
        float64 of = v_f32_sel_eq_f32_b(lanef, 0.0f, m, 0.0f);
        of = v_f32_sel_eq_f32_b(lanef, 1.0f, lse, of);
        of = v_f32_sel_eq_f32_b(lanef, 2.0f, zt, of);
        of = v_f32_sel_eq_f32_b(lanef, 3.0f, invT, of);
        of = v_f32_sel_eq_f32_b(lanef, 4.0f, uniform, of);
        of = v_f32_sel_eq_f32_b(lanef, 5.0f, top_k, of);
        of = v_f32_sel_eq_f32_b(lanef, 6.0f, top_p, of);
        of = v_f32_sel_eq_f32_b(lanef, 7.0f, (float)ntop, of);
        int5 oc = {0, b, 0, 0, 0};
        v_i32_st_tnsr(oc, sel_i, oi_);
        v_f32_st_tnsr(oc, sel_f, of);
        v_i32_st_tnsr(oc, top_idx, oi);
        v_f32_st_tnsr(oc, top_lp, ol);
    }
}
