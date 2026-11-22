// SPDX-License-Identifier: AGPL-3.0-only
// Copyright Aaron Botsis
// mHC residual update after a sub-layer (mhc_post without the rms, spread over the hidden dim):
//   Xo_j = post_j y + sum_i comb[i][j] X_i        (y summed over NR rank slices when NR > 1)
// The next site's 1 / rms(Xo) comes from mhc_mix.
// tensors: 0 X (4H, T) f32  1 y (H, T, NR) f32  2 coef (T, 24) f32 | 3 Xo (4H, T) f32.
// index space: [T, nb]: nb = clamp(48 / T, 1, H / 256) blocks of the hidden dim (a multiple of 64 each)
#include "common.h"

#define NS 4

void main(tensor X, tensor y, tensor coef, tensor Xo)
{
    const int5 off = get_index_space_offset();
    const int5 end = off + get_index_space_size();
    const int H = get_dim_size(y, 0), NR = get_dim_size(y, 2), T = get_dim_size(y, 1);
    int nb = 48 / T;
    nb = nb < 1 ? 1 : (nb > H / 256 ? H / 256 : nb);
    const int blk = (H / nb + 63) / 64 * 64;
    scalar_cache_reset();
    for (int t = off[0]; t < end[0]; t++) {
        float post[NS], cm[16];
        for (int s = 0; s < NS; s++) {
            int5 c = {t, 4 + s, 0, 0, 0};
            post[s] = s_f32_ld_g(gen_addr(c, coef));
        }
        for (int m = 0; m < 16; m++) {
            int5 c = {t, 8 + m, 0, 0, 0};
            cm[m] = s_f32_ld_g(gen_addr(c, coef));
        }
        for (int b = off[1]; b < end[1]; b++)
            #pragma unroll 4
            for (int i = blk * b; i < blk * b + blk && i < H; i += 64) {
                int5 yc = {i, t, 0, 0, 0};
                float64 yv = v_f32_ld_tnsr_b(yc, y), yw = 0.0f;
                for (yc[2] = 1; yc[2] + 1 < NR; yc[2] += 2) {
                    yv += v_f32_ld_tnsr_b(yc, y);
                    yc[2]++;
                    yw += v_f32_ld_tnsr_b(yc, y);
                    yc[2]--;
                }
                if (yc[2] < NR) yv += v_f32_ld_tnsr_b(yc, y);
                yv += yw;
                float64 xs[NS];
                #pragma unroll
                for (int s = 0; s < NS; s++) {
                    int5 c = {s * H + i, t, 0, 0, 0};
                    xs[s] = v_f32_ld_tnsr_b(c, X);
                }
                #pragma unroll
                for (int j = 0; j < NS; j++) {
                    float64 o = yv * post[j];
                    #pragma unroll
                    for (int s = 0; s < NS; s++) o = v_f32_mac_b(xs[s], cm[4 * s + j], o);
                    int5 c = {j * H + i, t, 0, 0, 0};
                    v_f32_st_tnsr(c, Xo, o);
                }
            }
    }
}
