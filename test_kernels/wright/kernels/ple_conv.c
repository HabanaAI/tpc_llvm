// SPDX-License-Identifier: AGPL-3.0-only
// Copyright Aaron Botsis
// PLE depthwise causal conv (kernel 4, dilation 3) + SiLU and the residual update, per 64 channels:
//   conv[c,t] = sum_k w[k][c] * in[c, t - 3 (3 - k)],  in = [state (9 previous positions) | gvn]
//   xo = x + gv + silu(conv)   (padded tokens t >= n_real: xo = x + gv)
//   state_out = the last 9 inputs up to token n_real - 1.
// tensors: 0 x (C,T) f32  1 gv (C,T)  2 gvn (C,T)  3 w (C,4) f32 [k][c]  4 st_in (C,9) f32  5 meta i32
//        | 6 xo (C,T) f32  7 st_out (C,9) f32
// -DMID (speculative decoding, T = 1 + drafts): one more output st_mid (C,9,T-1), the state after token t
// at index t (t < T-1).
// -DARR (pipelined speculative step): st_in (C,9,S) read at slot meta[2], st_out (C,9,T): the state after
// token t at slot t (every t; all T tokens real).
// -DBATCH (batched decode, Qwen3.8-Flash-Next runner): T rows of one token each, row t's state in slot t of
// st_in / st_out (C, 9, T); padding rows (t >= n_real) add no conv term. index space: [C/64, T]
// index space: [C/64]
#include "common.h"

#ifdef MID
void main(tensor x, tensor gv, tensor gvn, tensor w, tensor st_in, tensor meta, tensor xo, tensor st_out, tensor st_mid)
#else
void main(tensor x, tensor gv, tensor gvn, tensor w, tensor st_in, tensor meta, tensor xo, tensor st_out)
#endif
{
    const int5 off = get_index_space_offset();
    const int5 end = off + get_index_space_size();
    const int T = get_dim_size(x, 1);
    scalar_cache_reset();
    const int n_real = meta_i32(meta, 1);
#ifdef ARR
    const int sl = meta_i32(meta, 2);
#else
    const int sl = 0;
#endif
#ifdef BATCH
    // -DBATCH (batched decode): T rows of one token each; row t's state is slot t of st_in / st_out (C, 9, T);
    // index space: [C/64, T]
    for (int cb = off[0]; cb < end[0]; cb++)
    for (int tb = off[1]; tb < end[1]; tb++) {
        const int c0 = cb * 64;
        int5 wc = {c0, 0, 0, 0, 0};
        const float64 w0 = v_f32_ld_tnsr_b(wc, w);
        wc[1] = 1; const float64 w1 = v_f32_ld_tnsr_b(wc, w);
        wc[1] = 2; const float64 w2 = v_f32_ld_tnsr_b(wc, w);
        wc[1] = 3; const float64 w3 = v_f32_ld_tnsr_b(wc, w);
        int5 sc = {c0, 0, tb, 0, 0};
        float64 h[9];
        #pragma unroll
        for (int j = 0; j < 9; j++) { sc[1] = j; h[j] = v_f32_ld_tnsr_b(sc, st_in); }
        int5 c = {c0, tb, 0, 0, 0};
        const float64 v = v_f32_ld_tnsr_b(c, gvn);
        const float64 cv = w0 * h[0] + w1 * h[3] + w2 * h[6] + w3 * v;
        const float64 real = tb < n_real ? 1.0f : 0.0f;
        v_f32_st_tnsr(c, xo, v_f32_ld_tnsr_b(c, x) + v_f32_ld_tnsr_b(c, gv) + nl_silus(cv) * real);
        #pragma unroll
        for (int j = 0; j < 8; j++) { sc[1] = j; v_f32_st_tnsr(sc, st_out, h[j + 1]); }
        sc[1] = 8; v_f32_st_tnsr(sc, st_out, v);
    }
    return;
#endif
    for (int cb = off[0]; cb < end[0]; cb++) {
        const int c0 = cb * 64;
        int5 wc = {c0, 0, 0, 0, 0};
        const float64 w0 = v_f32_ld_tnsr_b(wc, w);
        wc[1] = 1; const float64 w1 = v_f32_ld_tnsr_b(wc, w);
        wc[1] = 2; const float64 w2 = v_f32_ld_tnsr_b(wc, w);
        wc[1] = 3; const float64 w3 = v_f32_ld_tnsr_b(wc, w);
        int5 sc = {c0, 0, sl, 0, 0};
        float64 h0, h1, h2, h3, h4, h5, h6, h7, h8;          // in[t-9] .. in[t-1] (named: no array rotation)
        sc[1] = 0; h0 = v_f32_ld_tnsr_b(sc, st_in);
        sc[1] = 1; h1 = v_f32_ld_tnsr_b(sc, st_in);
        sc[1] = 2; h2 = v_f32_ld_tnsr_b(sc, st_in);
        sc[1] = 3; h3 = v_f32_ld_tnsr_b(sc, st_in);
        sc[1] = 4; h4 = v_f32_ld_tnsr_b(sc, st_in);
        sc[1] = 5; h5 = v_f32_ld_tnsr_b(sc, st_in);
        sc[1] = 6; h6 = v_f32_ld_tnsr_b(sc, st_in);
        sc[1] = 7; h7 = v_f32_ld_tnsr_b(sc, st_in);
        sc[1] = 8; h8 = v_f32_ld_tnsr_b(sc, st_in);
        for (int t = 0; t < n_real; t++) {
            int5 c = {c0, t, 0, 0, 0};
            const float64 v = v_f32_ld_tnsr_b(c, gvn);
            const float64 cv = w0 * h0 + w1 * h3 + w2 * h6 + w3 * v;
            v_f32_st_tnsr(c, xo, v_f32_ld_tnsr_b(c, x) + v_f32_ld_tnsr_b(c, gv) + nl_silus(cv));
            h0 = h1; h1 = h2; h2 = h3; h3 = h4; h4 = h5; h5 = h6; h6 = h7; h7 = h8; h8 = v;
#if defined(MID) || defined(ARR)
#ifdef ARR
#define st_mid st_out
            if (1) {                       // the state after token t at slot t
#else
            if (t < T - 1) {               // the state after token t at mid index t
#endif
                int5 mc = {c0, 0, t, 0, 0};
                mc[1] = 0; v_f32_st_tnsr(mc, st_mid, h0); mc[1] = 1; v_f32_st_tnsr(mc, st_mid, h1);
                mc[1] = 2; v_f32_st_tnsr(mc, st_mid, h2); mc[1] = 3; v_f32_st_tnsr(mc, st_mid, h3);
                mc[1] = 4; v_f32_st_tnsr(mc, st_mid, h4); mc[1] = 5; v_f32_st_tnsr(mc, st_mid, h5);
                mc[1] = 6; v_f32_st_tnsr(mc, st_mid, h6); mc[1] = 7; v_f32_st_tnsr(mc, st_mid, h7);
                mc[1] = 8; v_f32_st_tnsr(mc, st_mid, h8);
            }
#endif
        }
        for (int t = n_real; t < T; t++) {
            int5 c = {c0, t, 0, 0, 0};
            v_f32_st_tnsr(c, xo, v_f32_ld_tnsr_b(c, x) + v_f32_ld_tnsr_b(c, gv));
        }
#ifndef ARR
        sc[1] = 0; v_f32_st_tnsr(sc, st_out, h0);
        sc[1] = 1; v_f32_st_tnsr(sc, st_out, h1);
        sc[1] = 2; v_f32_st_tnsr(sc, st_out, h2);
        sc[1] = 3; v_f32_st_tnsr(sc, st_out, h3);
        sc[1] = 4; v_f32_st_tnsr(sc, st_out, h4);
        sc[1] = 5; v_f32_st_tnsr(sc, st_out, h5);
        sc[1] = 6; v_f32_st_tnsr(sc, st_out, h6);
        sc[1] = 7; v_f32_st_tnsr(sc, st_out, h7);
        sc[1] = 8; v_f32_st_tnsr(sc, st_out, h8);
#endif
    }
}
