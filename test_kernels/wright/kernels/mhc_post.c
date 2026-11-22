// SPDX-License-Identifier: AGPL-3.0-only
// Copyright Aaron Botsis
// GLM mHC residual update after a sub-layer, one member per token:
//   Xo_j = post_j y + sum_i comb[i][j] X_i        (y summed over NR rank slices when NR > 1)
// mode 1 (embedding init): Xo_j = y for every stream (X, coef unused).
// Also rt[:, t] = 1 / rms(Xo_t flattened over the 4 streams) (all lanes), the next mHC's r.
// tensors: 0 X (4H, T) f32  1 y (H, T, NR) f32  2 coef (T, 24) f32 | 3 Xo (4H, T) f32  4 rt (64, T) f32
// params: eps (rms of the flattened streams), mode. index space: [1, T]
#include "common.h"

#define NS 4

void main(tensor X, tensor y, tensor coef, tensor Xo, tensor rt, float eps, int mode)
{
    const int5 off = get_index_space_offset();
    const int5 end = off + get_index_space_size();
    const int H = get_dim_size(y, 0), NR = get_dim_size(y, 2);
    const float inv_n = 1.0f / (float)(NS * H);
    scalar_cache_reset();
    DECL_IDX(IDX)
    for (int t = off[1]; t < end[1]; t++) {
        float64 post[NS], cm[16];
        #pragma unroll
        for (int s = 0; s < NS; s++)
            post[s] = mode == 1 ? (float64)1.0f : TOKEN_COEF(coef, t, 4 + s);
        #pragma unroll
        for (int m = 0; m < 16; m++)
            cm[m] = mode == 1 ? (float64)0.0f : TOKEN_COEF(coef, t, 8 + m);
        const float fx = mode == 1 ? 0.0f : 1.0f;
        float64 acc = 0.0f, acc2 = 0.0f;
        for (int i = 0; i < H; i += 64) {
            int5 yc = {i, t, 0, 0, 0};
            float64 yv = v_f32_ld_tnsr_b(yc, y);
            for (yc[2] = 1; yc[2] < NR; yc[2]++) yv += v_f32_ld_tnsr_b(yc, y);
            float64 xs[NS];
            #pragma unroll
            for (int s = 0; s < NS; s++) {
                int5 c = {s * H + i, t, 0, 0, 0};
                xs[s] = fx == 0.0f ? (float64)0.0f : v_f32_ld_tnsr_b(c, X);
            }
            #pragma unroll
            for (int j = 0; j < NS; j++) {
                float64 o = yv * post[j];
                #pragma unroll
                for (int s = 0; s < NS; s++) o = v_f32_mac_b(xs[s], cm[4 * s + j], o);
                int5 c = {j * H + i, t, 0, 0, 0};
                v_f32_st_tnsr(c, Xo, o);
                if (j & 1) acc2 = v_f32_mac_b(o, o, acc2); else acc = v_f32_mac_b(o, o, acc);
            }
        }
        float64 ss;
        NL_REDUCE_ADD(acc + acc2, IDX, ss);
        int5 rc = {0, t, 0, 0, 0};
        v_f32_st_tnsr(rc, rt, nl_rsqrt(ss * inv_n + eps));
    }
}
