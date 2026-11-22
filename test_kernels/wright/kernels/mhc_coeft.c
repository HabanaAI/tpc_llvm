// SPDX-License-Identifier: AGPL-3.0-only
// Copyright Aaron Botsis
// mhc_coef with one member per token, the 4 x 4 comb matrix in 16 lanes as in mhc_coef1 (see mhc_coef.c for the
// coefficients): a load at token t - j puts token t in lane j, so the token's mix entries gather into lanes from
// the token-major mixT, and lane j of each result goes back to coef[t, j] by a one-lane partial store. For a few
// tokens each member runs the short lane-packed Sinkhorn, not one member 16 vectors for 64 tokens.
// tensors: 0 mixT (T, 32) f32  1 rt (64, T) f32 (r per token, all lanes)  2 base (24) f32  3 scale (3) f32
//          | 4 coef (T, 24) f32 [pre | post | comb]
// params: iters, eps. index space: [T]
// PARTIALS (mhc_coefp.c): mhc_mixc's chunks instead of mixT and rt: tensors 0 mixp (T, 24, C) f32 (each chunk's
// projection)  1 ssp (T, C) f32 (each chunk's sum of squares); r = 1 / sqrt(sum ssp / K + rms_eps).
// params: iters, eps, rms_eps, K.
#include "common.h"

// lane l <- lane l ^ k of its 16-lane group
static inline uchar256 xor_ctl(int k)
{
    const int64 lane = (int64)read_lane_id_4b_b();
    const int64 e = (lane ^ k) & 15;
    const int64 ctl = (0x80 | ((e >> 3) << 5) | (e & 7)) * 0x01010101;
    return __builtin_bit_cast(uchar256, ctl);
}

#ifdef PARTIALS
void main(tensor mixp, tensor ssp, tensor base, tensor scl, tensor coef, int iters, float eps, float rms_eps, int K)
#else
void main(tensor mixT, tensor rt, tensor base, tensor scl, tensor coef, int iters, float eps)
#endif
{
    const int5 off = get_index_space_offset();
    const int5 end = off + get_index_space_size();
    scalar_cache_reset();
    const float64 lanef = v_convert_i32_to_f32_b((int64)read_lane_id_4b_b(), SW_RHNE, 0, 1, 0);
    const uchar256 SH1 = xor_ctl(1), SH2 = xor_ctl(2), SH4 = xor_ctl(4), SH8 = xor_ctl(8);
    float sc[3];
    for (int m = 0; m < 3; m++) { int5 c = {m, 0, 0, 0, 0}; sc[m] = s_f32_ld_g(gen_addr(c, scl)); }
    int5 z = {0, 0, 0, 0, 0}, z8 = {8, 0, 0, 0, 0};
    const float64 base0 = v_f32_ld_tnsr_b(z, base), base8 = v_f32_ld_tnsr_b(z8, base);
    for (int t = off[0]; t < end[0]; t++) {
        // lane j <- mix[t, j] (pre and post, j < 8) and mix[t, 8 + j] (comb, j < 16)
        float64 mix = 0.0f, mix8 = 0.0f;
#ifdef PARTIALS
        const int C = get_dim_size(mixp, 2);
        float ss = 0.0f;
        for (int ch = 0; ch < C; ch++) {
            float64 pm = 0.0f, pm8 = 0.0f;
            #pragma unroll 1
            for (int j = 0; j < 8; j++) {
                int5 c = {t - j, j, ch, 0, 0};
                pm = v_f32_sel_eq_f32_b(lanef, (float)j, v_f32_ld_tnsr_b(c, mixp), pm);
            }
            #pragma unroll 1
            for (int j = 0; j < 16; j++) {
                int5 c = {t - j, 8 + j, ch, 0, 0};
                pm8 = v_f32_sel_eq_f32_b(lanef, (float)j, v_f32_ld_tnsr_b(c, mixp), pm8);
            }
            mix += pm;
            mix8 += pm8;
            int5 sc = {t, ch, 0, 0, 0};
            ss += s_f32_ld_g(gen_addr(sc, ssp));
        }
        const float64 r = nl_rsqrt((float64)(ss * (1.0f / (float)K)) + rms_eps);
#else
        #pragma unroll 1
        for (int j = 0; j < 8; j++) {
            int5 c = {t - j, j, 0, 0, 0};
            mix = v_f32_sel_eq_f32_b(lanef, (float)j, v_f32_ld_tnsr_b(c, mixT), mix);
        }
        #pragma unroll 1
        for (int j = 0; j < 16; j++) {
            int5 c = {t - j, 8 + j, 0, 0, 0};
            mix8 = v_f32_sel_eq_f32_b(lanef, (float)j, v_f32_ld_tnsr_b(c, mixT), mix8);
        }
        int5 rc = {0, t, 0, 0, 0};
        const float64 r = v_f32_ld_tnsr_b(rc, rt);
#endif
        // pre (lanes 0..3) and post (lanes 4..7)
        const float64 a = mix * r;
        const float64 pre = nl_sigm(v_f32_mac_b(a, sc[0], base0)) + eps;
        const float64 post = 2.0f * nl_sigm(v_f32_mac_b(a, sc[1], base0));
        const float64 pp = v_f32_sel_less_f32_b(lanef, 4.0f, pre, post);
        // comb: entry 4 i + j in lane 4 i + j; softmax over each row, then Sinkhorn
        const float64 l = v_f32_mac_b(mix8 * r, sc[2], base8);
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
        #pragma unroll 1
        for (int it = 0; it < iters; it++) {   // columns first, then iters - 1 x (rows, columns)
            if (it > 0)
                ROWS()
            COLS()
        }
        // coef[t, j] <- lane j (a partial store writes its lanes from the coordinate on)
        #pragma unroll
        for (int j = 0; j < 8; j++) {
            int5 c = {t, j, 0, 0, 0};
            v_f32_st_tnsr_partial(c, coef, pp, 0, j);
        }
        #pragma unroll
        for (int j = 0; j < 16; j++) {
            int5 c = {t, 8 + j, 0, 0, 0};
            v_f32_st_tnsr_partial(c, coef, cm, 0, j);
        }
    }
}
