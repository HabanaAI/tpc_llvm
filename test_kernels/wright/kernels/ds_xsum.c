// SPDX-License-Identifier: AGPL-3.0-only
// Copyright Aaron Botsis
// Routed experts, after the second batch_gemm: out = sh * shs + sum_k y[:, t*R+k] * 2^(C-132) * hs * route weight
// (shs: the shared expert's FP8 activation scale per token; ones for a bf16 shared expert).
// tensors: 0 y (H, 1, S) f32  1 c (H, E) f32  2 hs (1, S) f32  3 ids (64, T) i32  4 wts (64, T) f32
//          5 sh (H, T) f32  6 shs (1, T) f32 | 7 out (H, T) f32
// params: R.  index space: [H/128, T]
#include "common.h"

void main(tensor y, tensor c, tensor hs, tensor ids, tensor wts, tensor sh, tensor shs, tensor out, int R)
{
    const int5 off = get_index_space_offset();
    const int5 end = off + get_index_space_size();
    const int E = get_dim_size(c, 1);
    DECL_IDX(IDX)
    scalar_cache_reset();
    for (int t = off[1]; t < end[1]; t++)
        for (int b = off[0]; b < end[0]; b++) {
            const float64 ss = TOKEN_SCALE(shs, t, IDX[0]);
            int5 oc = {128 * b, t, 0, 0, 0};
            float64 a0 = v_f32_ld_tnsr_b(oc, sh) * ss;
            oc[0] += 64;
            float64 a1 = v_f32_ld_tnsr_b(oc, sh) * ss;
            for (int k = 0; k < R; k++) {
                const int s = t * R + k;
                int5 ic = {k, t, 0, 0, 0}, hc = {0, s, 0, 0, 0};
                int e = s_i32_ld_g(gen_addr(ic, ids));
                e = e < 0 ? 0 : (e >= E ? E - 1 : e);
                const float f = s_f32_ld_g(gen_addr(ic, wts)) * s_f32_ld_g(gen_addr(hc, hs));
                int5 yc = {128 * b, 0, s, 0, 0}, cc = {128 * b, e, 0, 0, 0};
                a0 = v_f32_mac_b(v_f32_ld_tnsr_b(yc, y) * f, v_f32_ld_tnsr_b(cc, c), a0);
                yc[0] += 64; cc[0] += 64;
                a1 = v_f32_mac_b(v_f32_ld_tnsr_b(yc, y) * f, v_f32_ld_tnsr_b(cc, c), a1);
            }
            oc[0] = 128 * b;
            v_f32_st_tnsr(oc, out, a0);
            oc[0] += 64;
            v_f32_st_tnsr(oc, out, a1);
        }
}
