// SPDX-License-Identifier: AGPL-3.0-only
// Copyright Aaron Botsis
// Prefill pages, after the second batch_gemm: out = sh * shs + sum over the token's routes k with a row
// (bm[k, t] = r >= 0) of y[:, r] * 2^(C-132) [expert ids[k, t]] * hs[r] * route weight.
// tensors: 0 y (H, S) f32  1 c (H, E) f32  2 hs (1, S) f32  3 ids (64, T) i32  4 bm (64, T) i32  5 wts (64, T) f32
//          6 sh (H, T) f32  7 shs (1, T) f32 | 8 out (H, T) f32
// params: R.  index space: [T]
#include "common.h"

void main(tensor y, tensor c, tensor hs, tensor ids, tensor bm, tensor wts, tensor sh, tensor shs, tensor out, int R)
{
    const int5 off = get_index_space_offset();
    const int5 end = off + get_index_space_size();
    const int E = get_dim_size(c, 1), H = get_dim_size(out, 0);
    DECL_IDX(IDX)
    scalar_cache_reset();
    for (int t = off[0]; t < end[0]; t++) {
        const float64 ss = TOKEN_SCALE(shs, t, IDX[0]);
        int rows[8], exps[8];
        float64 fs[8];
        int n = 0;
        for (int k = 0; k < R && k < 8; k++) {
            int5 ic = {k, t, 0, 0, 0};
            const int r = s_i32_ld_g(gen_addr(ic, bm));
            if (r < 0) continue;
            int e = s_i32_ld_g(gen_addr(ic, ids));
            e = e < 0 ? 0 : (e >= E ? E - 1 : e);
            int5 hc = {0, r, 0, 0, 0};
            fs[n] = s_f32_ld_g(gen_addr(ic, wts)) * lane_bcast(v_f32_ld_tnsr_b(hc, hs), 0);
            rows[n] = r;
            exps[n] = e;
            n++;
        }
        if (n == 6) {  // every route has a row: the routes unrolled
            #pragma unroll 2
            for (int i = 0; i < H; i += 64) {
                int5 oc = {i, t, 0, 0, 0};
                float64 a0 = v_f32_ld_tnsr_b(oc, sh) * ss;
                #pragma unroll
                for (int k = 0; k < 6; k++) {
                    int5 yc = {i, rows[k], 0, 0, 0}, cc = {i, exps[k], 0, 0, 0};
                    a0 = v_f32_mac_b(v_f32_ld_tnsr_b(yc, y) * fs[k], v_f32_ld_tnsr_b(cc, c), a0);
                }
                v_f32_st_tnsr(oc, out, a0);
            }
            continue;
        }
        for (int i = 0; i < H; i += 64) {
            int5 oc = {i, t, 0, 0, 0};
            float64 a0 = v_f32_ld_tnsr_b(oc, sh) * ss;
            for (int k = 0; k < n; k++) {
                int5 yc = {i, rows[k], 0, 0, 0}, cc = {i, exps[k], 0, 0, 0};
                a0 = v_f32_mac_b(v_f32_ld_tnsr_b(yc, y) * fs[k], v_f32_ld_tnsr_b(cc, c), a0);
            }
            v_f32_st_tnsr(oc, out, a0);
        }
    }
}
