// SPDX-License-Identifier: AGPL-3.0-only
// Copyright Aaron Botsis
// mhc_coef for one token (T = 1) from mhc_mixp's chunk partials, the 4 x 4 comb matrix in 16 lanes: row and
// column sums by lane shuffles, every exp, sigmoid and reciprocal once per vector (see mhc_coef.c for the
// coefficients). mix_j = sum over chunks of mixp[j, c]; r = rsqrt(sum ssp / K + rms_eps).
// tensors: 0 mixp (32, C) f32  1 ssp (C) f32  2 base (24) f32  3 scale (3) f32 | 4 coef (24) f32 [pre | post | comb]
// params: iters, eps, rms_eps, K. index space: [1]
#include "common.h"

// lane l <- lane l ^ k of its 16-lane group
static inline uchar256 xor_ctl(int k)
{
    const int64 lane = (int64)read_lane_id_4b_b();
    const int64 e = (lane ^ k) & 15;
    const int64 ctl = (0x80 | ((e >> 3) << 5) | (e & 7)) * 0x01010101;
    return __builtin_bit_cast(uchar256, ctl);
}

void main(tensor mixp, tensor ssp, tensor base, tensor scl, tensor coef, int iters, float eps, float rms_eps, int K)
{
    DECL_IDX(IDX)
    scalar_cache_reset();
    const float64 lanef = v_convert_i32_to_f32_b((int64)read_lane_id_4b_b(), SW_RHNE, 0, 1, 0);
    const uchar256 SH1 = xor_ctl(1), SH2 = xor_ctl(2), SH4 = xor_ctl(4), SH8 = xor_ctl(8);
    float sc[3];
    for (int m = 0; m < 3; m++) { int5 c = {m, 0, 0, 0, 0}; sc[m] = s_f32_ld_g(gen_addr(c, scl)); }
    int5 z = {0, 0, 0, 0, 0}, z8 = {8, 0, 0, 0, 0};
    const int C = get_dim_size(mixp, 1);
    float64 mix = 0.0f, mix8 = 0.0f;
    for (int c = 0; c < C; c++) {
        int5 pc = {0, c, 0, 0, 0};
        mix += v_f32_ld_tnsr_b(pc, mixp);
        pc[0] = 8;
        mix8 += v_f32_ld_tnsr_b(pc, mixp);
    }
    float64 ss;
    NL_REDUCE_ADD(v_f32_sel_less_f32_b(lanef, (float)C, v_f32_ld_tnsr_b(z, ssp), 0.0f), IDX, ss);
    const float64 r = nl_rsqrt(ss * (1.0f / (float)K) + rms_eps);
    // pre (lanes 0..3) and post (lanes 4..7)
    const float64 a = mix * r;
    const float64 pre = nl_sigm(v_f32_mac_b(a, sc[0], v_f32_ld_tnsr_b(z, base))) + eps;
    const float64 post = 2.0f * nl_sigm(v_f32_mac_b(a, sc[1], v_f32_ld_tnsr_b(z, base)));
    v_f32_st_tnsr(z, coef, v_f32_sel_less_f32_b(lanef, 4.0f, pre, post));
    // comb: entry 4 i + j in lane 4 i + j; softmax over each row, then Sinkhorn
    const float64 l = v_f32_mac_b(mix8 * r, sc[2], v_f32_ld_tnsr_b(z8, base));
    float64 mx = v_f32_max_b(l, v_f32_shuffle_b(l, SH1, 0, l));
    mx = v_f32_max_b(mx, v_f32_shuffle_b(mx, SH2, 0, mx));
    const float64 e = nl_exp(l - mx);
    float64 s = e + v_f32_shuffle_b(e, SH1, 0, e);
    s = s + v_f32_shuffle_b(s, SH2, 0, s);
    float64 cm = e * nl_recip(s) + eps;
#define COLS() { float64 c_ = cm + v_f32_shuffle_b(cm, SH4, 0, cm); c_ = c_ + v_f32_shuffle_b(c_, SH8, 0, c_); \
                 cm = cm * nl_recip(c_ + eps); }
#define ROWS() { float64 c_ = cm + v_f32_shuffle_b(cm, SH1, 0, cm); c_ = c_ + v_f32_shuffle_b(c_, SH2, 0, c_); \
                 cm = cm * nl_recip(c_ + eps); }
    COLS()
    for (int it = 1; it < iters; it++) {
        ROWS()
        COLS()
    }
    v_f32_st_tnsr(z8, coef, cm);                             // lanes past 16 fall outside the 24 entries
}
