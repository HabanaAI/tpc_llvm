// SPDX-License-Identifier: AGPL-3.0-only
// Copyright Aaron Botsis
// Routed experts, between the two batch_gemms: per slot the gate | up rows of the first product (f32, by row
// scale 2^(C-132) and the token's input scale), clamped SwiGLU, then per-slot FP8 quantization (pair order) for the
// second batch_gemm. Width w = w0 + t: the main rows [gate 0..w0 | up 0..w0] come from y, the tail rows
// [gate w0..w | up w0..w] (t = 0 / 32 / 64) from yt. Each member writes one 128-chunk of h (the last: t long) and
// recomputes the whole slot's h for the max.
// tensors: 0 y (2 w0, 1, S) f32  1 yt (2t, 1, S) f32 (any tensor when t = 0)  2 c (2w, E) f32 ([main | tail] rows)
//          3 sx (1, T) f32  4 ids (64, T) i32 | 5 hq (w, 1, S) fp8  6 hs (1, S) f32
// params: R, lim, t, psh.  index space: [S, ceil(w/128)] (slot = t * R + k).  w0 % 128 == 0, w <= 4096.
// Prefill pages (R = 0): S = rows of 2^psh-row pages, ids = pe (P,) (expert of page s >> psh), sx (1, S) per row, 0
// on rows no pair uses, which are skipped. A member then owns a whole slot (index space [S]).
// LIN (ds_xact_lin.c): hq in natural order, and the tail rows come from y: y (2w, 1, S) holds
// [gate 0..w0 | up 0..w0 | gate w0..w | up w0..w] (yt is not read; t <= 64).
#define LIM  // before common.h: it selects the clamped CLAMP_G / CLAMP_U
#include "common.h"

__local__ float64 HB[66];

void main(tensor y, tensor yt, tensor c, tensor sx, tensor ids, tensor hq, tensor hs, int R, float lim, int t, int psh)
{
    const int5 off = get_index_space_offset();
    const int5 end = off + get_index_space_size();
    const int w = get_dim_size(hq, 0), w0 = w - t, E = get_dim_size(c, 1);
    const float64 lanef = v_convert_i32_to_f32_b((int64)read_lane_id_4b_b(), SW_RHNE, 0, 1, 0);
    DECL_IDX(IDX)
    scalar_cache_reset();
    for (int s = off[0]; s < end[0]; s++) {
        const int tk = R ? s / R : s;
        int5 ic = {R ? s % R : s >> psh, R ? tk : 0, 0, 0, 0};
        int e = s_i32_ld_g(gen_addr(ic, ids));
        e = e < 0 ? 0 : (e >= E ? E - 1 : e);
        int5 xc = {0, tk, 0, 0, 0};
        const float xs = s_f32_ld_g(gen_addr(xc, sx));
        if (!R && xs == 0.0f)
            continue;
        float64 amax = 0.0f;
        for (int i = 0; i < w0; i += 64) {
            int5 yc = {i, 0, s, 0, 0}, cc = {i, e, 0, 0, 0};
            const float64 gt = v_f32_ld_tnsr_b(yc, y) * v_f32_ld_tnsr_b(cc, c) * xs;
            yc[0] = i + w0; cc[0] = i + w0;
            const float64 up = v_f32_ld_tnsr_b(yc, y) * v_f32_ld_tnsr_b(cc, c) * xs;
            const float64 h = nl_silus(CLAMP_G(gt)) * CLAMP_U(up);
            HB[i / 64] = h;
            amax = v_f32_max_b(amax, v_f32_abs_b(h));
        }
#ifdef LIN
        if (t) {
            // the tail chunk as one vector: h[w0 .. w0 + t) in lanes < t
            int5 yc = {2 * w0, 0, s, 0, 0}, cc = {2 * w0, e, 0, 0, 0};
            const float64 gt = v_f32_ld_tnsr_b(yc, y) * v_f32_ld_tnsr_b(cc, c) * xs;
            yc[0] = 2 * w0 + t; cc[0] = 2 * w0 + t;
            const float64 up = v_f32_ld_tnsr_b(yc, y) * v_f32_ld_tnsr_b(cc, c) * xs;
            const float64 h = v_f32_sel_less_f32_b(lanef, (float)t, nl_silus(CLAMP_G(gt)) * CLAMP_U(up), 0.0f);
            HB[w0 / 64] = h;
            HB[w0 / 64 + 1] = 0.0f;
            amax = v_f32_max_b(amax, v_f32_abs_b(h));
        }
#else
        if (t) {
            // pack halves of the tail chunk: a = h[w0 .. w0 + t/2), b = h[w0 + t/2 .. w0 + t) (lanes < t/2 valid)
            #pragma unroll 1
            for (int hb = 0; hb < 2; hb++) {
                const int o = hb * (t / 2);
                int5 yc = {o, 0, s, 0, 0}, cc = {2 * w0 + o, e, 0, 0, 0};
                const float64 gt = v_f32_ld_tnsr_b(yc, yt) * v_f32_ld_tnsr_b(cc, c) * xs;
                yc[0] = t + o; cc[0] = 2 * w0 + t + o;
                const float64 up = v_f32_ld_tnsr_b(yc, yt) * v_f32_ld_tnsr_b(cc, c) * xs;
                const float64 h = v_f32_sel_less_f32_b(lanef, (float)(t / 2), nl_silus(CLAMP_G(gt)) * CLAMP_U(up), 0.0f);
                HB[w0 / 64 + hb] = h;
                amax = v_f32_max_b(amax, v_f32_abs_b(h));
            }
        }
#endif
        NL_REDUCE_MAX(amax, IDX, amax);
        const float64 sc = token_scale(amax);
        const float64 inv = token_inv(sc);
        const int q0 = R ? off[1] : 0, q1 = R ? end[1] : (w + 127) >> 7;
        for (int q = q0; q < q1; q++) {
            int5 qc = {128 * q, 0, s, 0, 0};
#ifdef LIN
            if (128 * q < w0)
                v_f8_st_tnsr_partial(qc, hq, pack_lin(HB[2 * q] * inv, HB[2 * q + 1] * inv, 0.0f, 0.0f), 127, 0);
            else
                v_f8_st_tnsr_partial(qc, hq, pack_lin(HB[w0 / 64] * inv, 0.0f, 0.0f, 0.0f), t - 1, 0);
#else
            if (128 * q < w0)
                v_f8_st_tnsr_partial(qc, hq, pack_nat128(HB[2 * q] * inv, HB[2 * q + 1] * inv), 127, 0);
            else
                v_f8_st_tnsr_partial(qc, hq, pack_nat128(HB[w0 / 64] * inv, HB[w0 / 64 + 1] * inv), t - 1, 0);
#endif
            if (q == 0)
                STORE_TOKEN_SCALE(hs, s, sc)
        }
    }
}
