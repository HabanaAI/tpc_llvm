// SPDX-License-Identifier: AGPL-3.0-only
// Copyright Aaron Botsis
// ds_xact_lin for prefill pages, one member per page (DeepSeek-V4.1 widths: w = 288 = 256 + a 32-row tail): the
// page's expert scales are read once, then its rows two at a time up to its first unused row (scale 0; an
// expert's rows fill its pages from the start). Same values as ds_xact_lin (R = 0).
// tensors: 0 y (2w, 1, S) f32 [gate 0..256 | up 0..256 | gate 256..288 | up 256..288]  1 c (2w, E) f32
//          2 sx (1, S) f32  3 pe (P,) i32 | 4 hq (w, 1, S) fp8  5 hs (1, S) f32
// params: lim, psh.  index space: [P]
#define LIM  // before common.h: it selects the clamped CLAMP_G / CLAMP_U
#define POW2
#include "common.h"

#define W0 256
#define TL 32

#define ROW(r, V0, V1, V2, V3, V4, AMAX) \
    { \
        int5 yc = {0, 0, (r), 0, 0}; \
        int5 xc = {0, (r), 0, 0, 0}; \
        const float xs = s_f32_ld_g(gen_addr(xc, sx)); \
        float64 g_[5], u_[5]; \
        _Pragma("unroll") for (int k = 0; k < 4; k++) { \
            yc[0] = 64 * k;      g_[k] = v_f32_ld_tnsr_b(yc, y); \
            yc[0] = W0 + 64 * k; u_[k] = v_f32_ld_tnsr_b(yc, y); \
        } \
        yc[0] = 2 * W0;      g_[4] = v_f32_ld_tnsr_b(yc, y); \
        yc[0] = 2 * W0 + TL; u_[4] = v_f32_ld_tnsr_b(yc, y); \
        float64 hh_[5]; \
        _Pragma("unroll") for (int k = 0; k < 5; k++) { \
            const float64 gt = g_[k] * cg[k] * xs, up = u_[k] * cu[k] * xs; \
            hh_[k] = nl_silus(CLAMP_G(gt)) * CLAMP_U(up); \
        } \
        hh_[4] = v_f32_sel_less_f32_b(lanef, (float)TL, hh_[4], 0.0f); \
        V0 = hh_[0]; V1 = hh_[1]; V2 = hh_[2]; V3 = hh_[3]; V4 = hh_[4]; \
        AMAX = v_f32_max_b(v_f32_max_b(v_f32_max_b(v_f32_abs_b(V0), v_f32_abs_b(V1)), \
                                       v_f32_max_b(v_f32_abs_b(V2), v_f32_abs_b(V3))), v_f32_abs_b(V4)); \
    }

#define STORE(r, V0, V1, V2, V3, V4, AMAX) \
    { \
        float64 am_; \
        NL_REDUCE_MAX(AMAX, IDX, am_); \
        const float64 sc_ = token_scale(am_); \
        const float64 inv_ = token_inv(sc_); \
        int5 qc = {0, 0, (r), 0, 0}; \
        v_f8_st_tnsr_partial(qc, hq, pack_lin(V0 * inv_, V1 * inv_, 0.0f, 0.0f), 127, 0); \
        qc[0] = 128; \
        v_f8_st_tnsr_partial(qc, hq, pack_lin(V2 * inv_, V3 * inv_, 0.0f, 0.0f), 127, 0); \
        qc[0] = 256; \
        v_f8_st_tnsr_partial(qc, hq, pack_lin(V4 * inv_, 0.0f, 0.0f, 0.0f), TL - 1, 0); \
        STORE_TOKEN_SCALE(hs, (r), sc_) \
    }

void main(tensor y, tensor c, tensor sx, tensor pe, tensor hq, tensor hs, float lim, int psh)
{
    const int5 off = get_index_space_offset();
    const int5 end = off + get_index_space_size();
    const int E = get_dim_size(c, 1), PS = 1 << psh;
    const float64 lanef = v_convert_i32_to_f32_b((int64)read_lane_id_4b_b(), SW_RHNE, 0, 1, 0);
    DECL_IDX(IDX)
    scalar_cache_reset();
    for (int p = off[0]; p < end[0]; p++) {
        int5 pc = {p, 0, 0, 0, 0};
        int e = s_i32_ld_g(gen_addr(pc, pe));
        if (e < 0)
            continue;
        e = e >= E ? E - 1 : e;
        float64 cg[5], cu[5];
        #pragma unroll
        for (int k = 0; k < 4; k++) {
            int5 cc = {64 * k, e, 0, 0, 0};
            cg[k] = v_f32_ld_tnsr_b(cc, c);
            cc[0] = W0 + 64 * k;
            cu[k] = v_f32_ld_tnsr_b(cc, c);
        }
        int5 cc = {2 * W0, e, 0, 0, 0};
        cg[4] = v_f32_ld_tnsr_b(cc, c);
        cc[0] = 2 * W0 + TL;
        cu[4] = v_f32_ld_tnsr_b(cc, c);
        for (int i = 0; i < PS; i += 2) {
            const int r = PS * p + i;
            int5 xc = {0, r, 0, 0, 0};
            if (s_f32_ld_g(gen_addr(xc, sx)) == 0.0f)
                break;
            xc[1] = r + 1;
            const bool two = s_f32_ld_g(gen_addr(xc, sx)) != 0.0f;
            float64 a0, a1, a2, a3, a4, am, b0, b1, b2, b3, b4, bm;
            ROW(r, a0, a1, a2, a3, a4, am)
            ROW(r + 1, b0, b1, b2, b3, b4, bm)
            STORE(r, a0, a1, a2, a3, a4, am)
            if (!two)
                break;
            STORE(r + 1, b0, b1, b2, b3, b4, bm)
        }
    }
}
