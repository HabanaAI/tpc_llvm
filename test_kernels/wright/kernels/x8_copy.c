// SPDX-License-Identifier: AGPL-3.0-only
// Copyright Aaron Botsis
// Routed experts: one (M, K) FP8 matrix per route slot, copied from the expert the router chose.
// slot = t * R + k -> ids[k, t]; R = 0 (prefill pages): slot s -> ids[s, 0]. An id out of range (padding,
// unused page) gives zeros.
// tensors: 0 W (M, K, E) fp8  1 ids (64, T) i32 or (P,) i32 | 2 Wd (M, K, S) fp8
// params: R. index space: [M/256, S, ceil(K/128)]
#include "common.h"

void main(tensor W, tensor ids, tensor Wd, int R)
{
    const int5 off = get_index_space_offset();
    const int5 end = off + get_index_space_size();
    const int E = get_dim_size(W, 2), K = get_dim_size(Wd, 1);
    scalar_cache_reset();
    for (int s = off[1]; s < end[1]; s++) {
        int5 ic = {R ? s % R : s, R ? s / R : 0, 0, 0, 0};
        const int e = s_i32_ld_g(gen_addr(ic, ids));
        for (int nt = off[0]; nt < end[0]; nt++)
            for (int kb = off[2]; kb < end[2]; kb++) {
                int5 dst = {256 * nt, 128 * kb, s, 0, 0};
                int5 src = {256 * nt, 128 * kb, e, 0, 0};
                const int L = K - 128 * kb < 128 ? K - 128 * kb : 128;
                if (e < 0 || e >= E) {
                    for (int r = 0; r < L; r++) {
                        v_f8_st_tnsr(dst, Wd, (minifloat256)0);
                        dst[1]++;
                    }
                    continue;
                }
                #pragma unroll 4
                for (int r = 0; r < L; r++) {
                    v_f8_st_tnsr(dst, Wd, v_f8_ld_tnsr_b(src, W));
                    dst[1]++;
                    src[1]++;
                }
            }
    }
}
