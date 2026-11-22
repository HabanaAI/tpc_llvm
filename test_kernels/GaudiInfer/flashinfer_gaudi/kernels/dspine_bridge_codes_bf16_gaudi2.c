// SPDX-License-Identifier: Apache-2.0
// DSpine fused Code Bridge Stage B (100% LUT-free, 4-way VPU-interleaved):
//   if (row & 15) == 0: codes[row] = anchor_codes[row >> 4]
//   else:               codes[row] = bf16((f32(projected[row]) * sqrt_c) / (sqrt(sum(f^2)) + bridge_eps))
// Tensors (dim 0 fastest):
//   0 projected    BF16 [C, BT]
//   1 anchor_codes BF16 [C, B]
//   2 codes        BF16 [C, BT]
// Index space: dim 0 = row [0..BT).

#include "norm_math_gaudi2.h"

static inline float64 positive_reciprocal_exact(float64 value) {
    float64 est = as_float64((int64)0x7ef311c3 - as_int64(value));
    est = est * (2.0f - value * est);
    est = est * (2.0f - value * est);
    est = est * (2.0f - value * est);
    est = est * (2.0f - value * est);
    return v_f32_sel_eq_f32_b(value, as_float64((int64)0x7f800000), 0.0f, est);
}

void main(tensor projected, tensor anchor_codes, tensor codes, float sqrt_c, float bridge_eps) {
    const int5 start = get_index_space_offset();
    const int5 end = start + get_index_space_size();
    const int width = get_dim_size(projected, 0);
    const int tiles = width / 128;  // C=1024 -> 8 (multiple of 4)
    bfloat128 cached[16];

    for (int row = start[0]; row < end[0]; ++row) {
        if ((row & 15) == 0) {
            const int b = row >> 4;
            for (int tile = 0; tile < tiles; tile += 4) {
                const int c0 = tile * 128, c1 = c0 + 128, c2 = c0 + 256, c3 = c0 + 384;
                int5 ac0 = {c0, b, 0, 0, 0}, ac1 = {c1, b, 0, 0, 0};
                int5 ac2 = {c2, b, 0, 0, 0}, ac3 = {c3, b, 0, 0, 0};
                int5 oc0 = {c0, row, 0, 0, 0}, oc1 = {c1, row, 0, 0, 0};
                int5 oc2 = {c2, row, 0, 0, 0}, oc3 = {c3, row, 0, 0, 0};
                v_bf16_st_tnsr(oc0, codes, v_bf16_ld_tnsr_b(ac0, anchor_codes));
                v_bf16_st_tnsr(oc1, codes, v_bf16_ld_tnsr_b(ac1, anchor_codes));
                v_bf16_st_tnsr(oc2, codes, v_bf16_ld_tnsr_b(ac2, anchor_codes));
                v_bf16_st_tnsr(oc3, codes, v_bf16_ld_tnsr_b(ac3, anchor_codes));
            }
            continue;
        }

        float128 sq0 = {0}, sq1 = {0}, sq2 = {0}, sq3 = {0};
        for (int tile = 0; tile < tiles; tile += 4) {
            const int c0 = tile * 128, c1 = c0 + 128, c2 = c0 + 256, c3 = c0 + 384;
            int5 ic0 = {c0, row, 0, 0, 0}, ic1 = {c1, row, 0, 0, 0};
            int5 ic2 = {c2, row, 0, 0, 0}, ic3 = {c3, row, 0, 0, 0};

            const bfloat128 p0 = v_bf16_ld_tnsr_b(ic0, projected);
            const bfloat128 p1 = v_bf16_ld_tnsr_b(ic1, projected);
            const bfloat128 p2 = v_bf16_ld_tnsr_b(ic2, projected);
            const bfloat128 p3 = v_bf16_ld_tnsr_b(ic3, projected);

            cached[tile]     = p0;
            cached[tile + 1] = p1;
            cached[tile + 2] = p2;
            cached[tile + 3] = p3;

            sq0 = v_bf16_mac_acc32_b(p0, p0, sq0, (e_no_negation) << 1);
            sq1 = v_bf16_mac_acc32_b(p1, p1, sq1, (e_no_negation) << 1);
            sq2 = v_bf16_mac_acc32_b(p2, p2, sq2, (e_no_negation) << 1);
            sq3 = v_bf16_mac_acc32_b(p3, p3, sq3, (e_no_negation) << 1);
        }

        const float64 sum_all = ((sq0.v1 + sq1.v1) + (sq2.v1 + sq3.v1)) +
                                ((sq0.v2 + sq1.v2) + (sq2.v2 + sq3.v2));
        const float64 s = row_sum(sum_all);
        const float64 sc = v_f32_max_b(s, 1.1754943508222875e-38f);
        const float64 pn = v_f32_sel_eq_f32_b(s, 0.0f, 0.0f, sc * positive_rsqrt(sc));
        const float64 inv_den = positive_reciprocal_exact(pn + bridge_eps);

        for (int tile = 0; tile < tiles; tile += 4) {
            float64_pair_t v0 = v_convert_bf16_to_f32_all_b(cached[tile]);
            float64_pair_t v1 = v_convert_bf16_to_f32_all_b(cached[tile + 1]);
            float64_pair_t v2 = v_convert_bf16_to_f32_all_b(cached[tile + 2]);
            float64_pair_t v3 = v_convert_bf16_to_f32_all_b(cached[tile + 3]);

            v0.v1 = (v0.v1 * sqrt_c) * inv_den; v0.v2 = (v0.v2 * sqrt_c) * inv_den;
            v1.v1 = (v1.v1 * sqrt_c) * inv_den; v1.v2 = (v1.v2 * sqrt_c) * inv_den;
            v2.v1 = (v2.v1 * sqrt_c) * inv_den; v2.v2 = (v2.v2 * sqrt_c) * inv_den;
            v3.v1 = (v3.v1 * sqrt_c) * inv_den; v3.v2 = (v3.v2 * sqrt_c) * inv_den;

            int5 oc0 = {tile * 128,       row, 0, 0, 0};
            int5 oc1 = {(tile + 1) * 128, row, 0, 0, 0};
            int5 oc2 = {(tile + 2) * 128, row, 0, 0, 0};
            int5 oc3 = {(tile + 3) * 128, row, 0, 0, 0};
            v_bf16_st_tnsr(oc0, codes, v_convert_f32_to_bf16_all_b(v0));
            v_bf16_st_tnsr(oc1, codes, v_convert_f32_to_bf16_all_b(v1));
            v_bf16_st_tnsr(oc2, codes, v_convert_f32_to_bf16_all_b(v2));
            v_bf16_st_tnsr(oc3, codes, v_convert_f32_to_bf16_all_b(v3));
        }
    }
}
