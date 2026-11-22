// SPDX-License-Identifier: AGPL-3.0-only
// Copyright Aaron Botsis
// Sampling, pass 3 of 5 (full): the exact threshold and top list of rows samp_select flagged,
// over the whole vocabulary (only when a chunk lane's 8 candidates did not reach the
// threshold or the ntop-th logit). Unflagged rows pass samp_select's results through.
// The definitions are samp_select's, with counts and masses over every token's order key and
// tempered weight from samp_scan (weights relative to their chunk's maximum, rescaled here).
// tensors: 0 wkey (Vp, B) i32  1 wt (Vp, B) f32 (Vp: V rounded up to 64; padding keys KEY_MIN)  2 stats (see samp_scan)  3 sel_i  4 sel_f
//          5 top_idx  6 top_lp (see samp_select)
//        | 7 fin_i (16, B) i32 [theta, sample, greedy token]  8 fin_idx (NMAX, B) i32
//          9 fin_lp (NMAX, B) f32
// index space: [B]
#include "samp.h"

__local__ float64 SC[NC_MAX];  // per-chunk scale of the weights: exp((m_c - m) / T)

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


void main(tensor wkey, tensor wt, tensor stats, tensor sel_i, tensor sel_f, tensor top_idx,
          tensor top_lp, tensor fin_i, tensor fin_idx, tensor fin_lp)
{
    const int5 off = get_index_space_offset();
    const int5 end = off + get_index_space_size();
    const int V = get_dim_size(wkey, 0);  // padded to whole vectors
    const int NC = get_dim_size(stats, 1);
    const int nvec = (V + 63) / 64;
    const int per = (nvec + NC - 1) / NC;
    const float64 lanef = v_convert_i32_to_f32_b((int64)read_lane_id_4b_b(), SW_RHNE, 0, 1, 0);
    DECL_IDX(IDX)
    scalar_cache_reset();
    for (int b = off[0]; b < end[0]; b++) {
        int5 rc = {0, b, 0, 0, 0};
        const int64 si = v_i32_ld_tnsr_b(rc, sel_i);
        int64 oi = v_i32_ld_tnsr_b(rc, top_idx);
        float64 ol = v_f32_ld_tnsr_b(rc, top_lp);
        int5 pc = {1, b, 0, 0, 0};
        const int flags = s_i32_ld_g(gen_addr(pc, sel_i));
        // theta, sample and greedy token: sel_i lanes 0, 3, 2 -> fin_i lanes 0, 1, 2
        int64 theta = v_i32_sel_eq_f32_b(lanef, 0.0f, si, (int64)KEY_MIN);
        NL_REDUCE_I32(theta, IDX, theta, v_i32_max_b)
        int5 xc = {0, b, 0, 0, 0};
        if (flags & 1) {
            pc[0] = 0; const float m = s_f32_ld_g(gen_addr(pc, sel_f));
            pc[0] = 2; const float zt = s_f32_ld_g(gen_addr(pc, sel_f));
            pc[0] = 3; const float invT = s_f32_ld_g(gen_addr(pc, sel_f));
            pc[0] = 5; const float top_k = s_f32_ld_g(gen_addr(pc, sel_f));
            pc[0] = 6; const float top_p = s_f32_ld_g(gen_addr(pc, sel_f));
            for (int c = 0; c < NC; c++) {
                int5 sc = {0, c, b, 0, 0};
                const float mc = s_f32_ld_g(gen_addr(sc, stats));
                SC[c] = weight((float64)((mc - m) * invT));
            }
            theta = KEY_MIN + 1;
#define FULL_COUNT(mid, out)                                                                 \
            { float64 n0_ = 0.0f, n1_ = 0.0f;                                                \
              int v = 0;                                                                     \
              for (; v + 1 < nvec; v += 2) {                                                 \
                  xc[0] = v * 64;                                                            \
                  n0_ += v_f32_sel_geq_i32_b(v_i32_ld_tnsr_b(xc, wkey), mid, 1.0f, 0.0f);    \
                  xc[0] += 64;                                                               \
                  n1_ += v_f32_sel_geq_i32_b(v_i32_ld_tnsr_b(xc, wkey), mid, 1.0f, 0.0f);    \
              }                                                                              \
              n0_ += n1_;                                                                    \
              if (v < nvec) {                                                                \
                  xc[0] = v * 64;                                                            \
                  n0_ += v_f32_sel_geq_i32_b(v_i32_ld_tnsr_b(xc, wkey), mid, 1.0f, 0.0f);    \
              }                                                                              \
              NL_REDUCE_ADD(n0_, IDX, n0_)                                                   \
              out = v_f32_sel_less_f32_b(n0_, top_k - 0.5f, 0.0f, 1.0f); }
#define FULL_MASS(mid, out)                                                                  \
            { float64 ms_ = 0.0f;                                                            \
              for (int c = 0; c < NC; c++) {                                                 \
                  const int v0_ = c * per, v1_ = v0_ + per < nvec ? v0_ + per : nvec;        \
                  float64 t0_ = 0.0f, t1_ = 0.0f;                                            \
                  int v = v0_;                                                               \
                  for (; v + 1 < v1_; v += 2) {                                              \
                      xc[0] = v * 64;                                                        \
                      t0_ += v_f32_sel_geq_i32_b(v_i32_ld_tnsr_b(xc, wkey), mid, v_f32_ld_tnsr_b(xc, wt), 0.0f); \
                      xc[0] += 64;                                                           \
                      t1_ += v_f32_sel_geq_i32_b(v_i32_ld_tnsr_b(xc, wkey), mid, v_f32_ld_tnsr_b(xc, wt), 0.0f); \
                  }                                                                          \
                  if (v < v1_) {                                                             \
                      xc[0] = v * 64;                                                        \
                      t0_ += v_f32_sel_geq_i32_b(v_i32_ld_tnsr_b(xc, wkey), mid, v_f32_ld_tnsr_b(xc, wt), 0.0f); \
                  }                                                                          \
                  ms_ = v_f32_mac_b(t0_ + t1_, SC[c], ms_);                                  \
              }                                                                              \
              NL_REDUCE_ADD(ms_, IDX, ms_)                                                   \
              out = v_f32_sel_less_f32_b(ms_, target, 0.0f, 1.0f); }
            if (top_k > 0.5f)
                KEY_SEARCH(theta, FULL_COUNT)
            if (top_p < 1.0f) {
                const float64 target = (float64)(zt * top_p);
                KEY_SEARCH(theta, FULL_MASS)
            }
        }
        if (flags & 2) {
            pc[0] = 1; const float lse = s_f32_ld_g(gen_addr(pc, sel_f));
            pc[0] = 7; const int ntop = (int)s_f32_ld_g(gen_addr(pc, sel_f));
            int64 pk = KEY_MAX, pi = -1;
            for (int n = 0; n < ntop; n++) {
                int64 nk = KEY_MIN;
                for (int v = 0; v < nvec; v++) {
                    xc[0] = v * 64;
                    const int64 i = (int64)read_lane_id_4b_b() + (int64)(v * 64);
                    const int64 k = v_i32_ld_tnsr_b(xc, wkey);
                    const int64 tie = v_i32_sel_grt_i32_b(i, pi, k, (int64)KEY_MIN);
                    nk = v_i32_max_b(nk, v_i32_sel_less_i32_b(k, pk, k, v_i32_sel_eq_i32_b(k, pk, tie, (int64)KEY_MIN)));
                }
                NL_REDUCE_I32(nk, IDX, nk, v_i32_max_b)
                int64 ni = KEY_MAX;
                for (int v = 0; v < nvec; v++) {
                    xc[0] = v * 64;
                    const int64 i = (int64)read_lane_id_4b_b() + (int64)(v * 64);
                    const int64 k = v_i32_ld_tnsr_b(xc, wkey);
                    const int64 after = v_i32_sel_eq_i32_b(k, pk, v_i32_sel_grt_i32_b(i, pi, i, (int64)KEY_MAX), i);
                    ni = v_i32_min_b(ni, v_i32_sel_eq_i32_b(k, nk, after, (int64)KEY_MAX));
                }
                NL_REDUCE_I32(ni, IDX, ni, v_i32_min_b)
                oi = v_i32_sel_eq_f32_b(lanef, (float)n, ni, oi);
                ol = v_f32_sel_eq_f32_b(lanef, (float)n, key_value(nk) - lse, ol);
                pk = nk;
                pi = ni;
            }
        }
        int64 sample = v_i32_sel_eq_f32_b(lanef, 3.0f, si, (int64)KEY_MIN);
        NL_REDUCE_I32(sample, IDX, sample, v_i32_max_b)
        int64 greedy = v_i32_sel_eq_f32_b(lanef, 2.0f, si, (int64)KEY_MIN);
        NL_REDUCE_I32(greedy, IDX, greedy, v_i32_max_b)
        int64 out = v_i32_sel_eq_f32_b(lanef, 0.0f, theta, (int64)0);
        out = v_i32_sel_eq_f32_b(lanef, 1.0f, sample, out);
        out = v_i32_sel_eq_f32_b(lanef, 2.0f, greedy, out);
        v_i32_st_tnsr(rc, fin_i, out);
        v_i32_st_tnsr(rc, fin_idx, oi);
        v_f32_st_tnsr(rc, fin_lp, ol);
    }
}
