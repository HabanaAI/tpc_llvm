// SPDX-License-Identifier: AGPL-3.0-only
// Copyright Aaron Botsis
// DFlash drafter attention epilogue: the accumulated slot attention of the paired layout (dfl_prep: head hd
// is pseudo-head hd % 8 of pair hd / 8, its values in half (hd % 8) / 4) as the o_proj FP8 GEMM operand:
// head hd's 128 values (sigma order within the head) at rows 128 hd .., per-token FP8 in tau order + scale.
// tensors: 0 acc_o (256, W, 4) f32 (W = 8 T) | 1 o8 (128 NH, T) fp8  2 os (1,T) f32
// index space: [T]. NH = 32.
#include "common.h"
#define NH 32
#define HPG 8
__local__ float64 O[2 * NH];
void main(tensor acc_o, tensor o8, tensor os)
{
    const int5 off = get_index_space_offset();
    const int5 end = off + get_index_space_size();
    DECL_IDX(IDX)
    for (int t = off[0]; t < end[0]; t++) {
        float64 amax = 0.0f;
        for (int hd = 0; hd < NH; hd++) {
            const int pg = hd / HPG, ph = hd % HPG, e = (ph / 4) & 1;
            int5 ac = {128 * e, ph + HPG * t, pg, 0, 0};
            const float64 a = v_f32_ld_tnsr_b(ac, acc_o);
            ac[0] += 64;
            const float64 b = v_f32_ld_tnsr_b(ac, acc_o);
            O[2 * hd] = a;
            O[2 * hd + 1] = b;
            amax = v_f32_max_b(amax, v_f32_max_b(v_f32_abs_b(a), v_f32_abs_b(b)));
        }
        NL_REDUCE_MAX(amax, IDX, amax);
        const float64 sc = token_scale(amax);
        const float64 inv = token_inv(sc);
        for (int v = 0; v < 2 * NH; v += 4) {
            int5 oc = {64 * v, t, 0, 0, 0};
            v_f8_st_tnsr(oc, o8, pack_tau(O[v] * inv, O[v + 1] * inv, O[v + 2] * inv, O[v + 3] * inv));
        }
        STORE_TOKEN_SCALE(os, t, sc)
    }
}
