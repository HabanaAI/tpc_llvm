// SPDX-License-Identifier: AGPL-3.0-only
// Copyright Aaron Botsis
// DeepSeek-V4.1 compressor, ratio 2: each completed group of two consecutive tokens pools their KV with a softmax of
// their gate scores per channel:
//   latent[:, g] = kv_a w_a + kv_b w_b,  (w_a, w_b) = softmax(score_a, score_b)
// Token b is column groups[1, g] of this step; token a is column groups[0, g], or, when that is -1, the pending
// token in row groups[2, g] of the state, [kv (512) | score (512)].
// tensors: 0 kv (512, T) f32  1 score (512, T) f32  2 state (1024, R) f32  3 groups (4, G) i32
//          | 4 latent (512, G) f32
// index space: [G]
#include "common.h"

void main(tensor kv, tensor score, tensor state, tensor groups, tensor latent)
{
    const int5 off = get_index_space_offset();
    const int5 end = off + get_index_space_size();
    scalar_cache_reset();
    for (int g = off[0]; g < end[0]; g++) {
        int5 gc = {0, g, 0, 0, 0};
        const int ca = s_i32_ld_g(gen_addr(gc, groups));
        gc[0] = 1;
        const int cb = s_i32_ld_g(gen_addr(gc, groups));
        gc[0] = 2;
        const int ra = s_i32_ld_g(gen_addr(gc, groups));
        for (int i = 0; i < 512; i += 64) {
            int5 bc = {i, cb, 0, 0, 0};
            const float64 kb = v_f32_ld_tnsr_b(bc, kv), sb = v_f32_ld_tnsr_b(bc, score);
            float64 ka, sa;
            if (ca >= 0) {
                int5 ac = {i, ca, 0, 0, 0};
                ka = v_f32_ld_tnsr_b(ac, kv);
                sa = v_f32_ld_tnsr_b(ac, score);
            } else {
                int5 ac = {i, ra, 0, 0, 0};
                ka = v_f32_ld_tnsr_b(ac, state);
                ac[0] = 512 + i;
                sa = v_f32_ld_tnsr_b(ac, state);
            }
            const float64 m = v_f32_max_b(sa, sb);
            const float64 ea = nl_exp(sa - m), eb = nl_exp(sb - m);
            int5 oc = {i, g, 0, 0, 0};
            v_f32_st_tnsr(oc, latent, (ka * ea + kb * eb) * nl_recip(ea + eb));
        }
    }
}
