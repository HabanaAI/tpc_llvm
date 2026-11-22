// SPDX-License-Identifier: Apache-2.0
// DSpine fused Code Bridge Stage A (100% LUT-free, 4-way VPU-interleaved):
//   out        = bf16(hsum + mlp)                              [H, BT] BF16
//   ms         = sum(f32(out)^2) * inv_h                       [1, BT] F32
//   rms        = sqrt(max(ms, kF32Tiny))                       [1, BT] F32
//   normalized = bf16(f32(out) / (rms + bridge_eps))           [H, BT] BF16
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

void main(tensor hsum, tensor mlp, tensor out, tensor normalized, tensor rms_out, tensor ms_out,
          float inv_h, float bridge_eps) {
    const int5 start = get_index_space_offset();
    const int5 end = start + get_index_space_size();
    const int width = get_dim_size(hsum, 0);
    const int tiles = width / 128;  // H=5120 -> 40 (multiple of 4)
    bfloat128 cached[64];

    for (int row = start[0]; row < end[0]; ++row) {
        float128 sq0 = {0}, sq1 = {0}, sq2 = {0}, sq3 = {0};

        for (int tile = 0; tile < tiles; tile += 4) {
            const int c0 = tile * 128, c1 = c0 + 128, c2 = c0 + 256, c3 = c0 + 384;
            int5 ic0 = {c0, row, 0, 0, 0}, ic1 = {c1, row, 0, 0, 0};
            int5 ic2 = {c2, row, 0, 0, 0}, ic3 = {c3, row, 0, 0, 0};

            const bfloat128 s0 = v_bf16_ld_tnsr_b(ic0, hsum) + v_bf16_ld_tnsr_b(ic0, mlp);
            const bfloat128 s1 = v_bf16_ld_tnsr_b(ic1, hsum) + v_bf16_ld_tnsr_b(ic1, mlp);
            const bfloat128 s2 = v_bf16_ld_tnsr_b(ic2, hsum) + v_bf16_ld_tnsr_b(ic2, mlp);
            const bfloat128 s3 = v_bf16_ld_tnsr_b(ic3, hsum) + v_bf16_ld_tnsr_b(ic3, mlp);

            v_bf16_st_tnsr(ic0, out, s0);
            v_bf16_st_tnsr(ic1, out, s1);
            v_bf16_st_tnsr(ic2, out, s2);
            v_bf16_st_tnsr(ic3, out, s3);

            cached[tile]     = s0;
            cached[tile + 1] = s1;
            cached[tile + 2] = s2;
            cached[tile + 3] = s3;

            sq0 = v_bf16_mac_acc32_b(s0, s0, sq0, (e_no_negation) << 1);
            sq1 = v_bf16_mac_acc32_b(s1, s1, sq1, (e_no_negation) << 1);
            sq2 = v_bf16_mac_acc32_b(s2, s2, sq2, (e_no_negation) << 1);
            sq3 = v_bf16_mac_acc32_b(s3, s3, sq3, (e_no_negation) << 1);
        }

        const float64 sum_all = ((sq0.v1 + sq1.v1) + (sq2.v1 + sq3.v1)) +
                                ((sq0.v2 + sq1.v2) + (sq2.v2 + sq3.v2));
        const float64 ms = row_sum(sum_all) * inv_h;
        const float64 ms_c = v_f32_max_b(ms, 1.1754943508222875e-38f);
        const float64 rms = ms_c * positive_rsqrt(ms_c);
        const float64 inv_den = positive_reciprocal_exact(rms + bridge_eps);

        int5 sc = {0, row, 0, 0, 0};
        v_f32_st_tnsr(sc, rms_out, rms);
        v_f32_st_tnsr(sc, ms_out, ms);

        for (int tile = 0; tile < tiles; tile += 4) {
            float64_pair_t v0 = v_convert_bf16_to_f32_all_b(cached[tile]);
            float64_pair_t v1 = v_convert_bf16_to_f32_all_b(cached[tile + 1]);
            float64_pair_t v2 = v_convert_bf16_to_f32_all_b(cached[tile + 2]);
            float64_pair_t v3 = v_convert_bf16_to_f32_all_b(cached[tile + 3]);

            v0.v1 *= inv_den; v0.v2 *= inv_den;
            v1.v1 *= inv_den; v1.v2 *= inv_den;
            v2.v1 *= inv_den; v2.v2 *= inv_den;
            v3.v1 *= inv_den; v3.v2 *= inv_den;

            const bfloat128 n0 = v_convert_f32_to_bf16_all_b(v0);
            const bfloat128 n1 = v_convert_f32_to_bf16_all_b(v1);
            const bfloat128 n2 = v_convert_f32_to_bf16_all_b(v2);
            const bfloat128 n3 = v_convert_f32_to_bf16_all_b(v3);

            int5 oc0 = {tile * 128,       row, 0, 0, 0};
            int5 oc1 = {(tile + 1) * 128, row, 0, 0, 0};
            int5 oc2 = {(tile + 2) * 128, row, 0, 0, 0};
            int5 oc3 = {(tile + 3) * 128, row, 0, 0, 0};
            v_bf16_st_tnsr(oc0, normalized, n0);
            v_bf16_st_tnsr(oc1, normalized, n1);
            v_bf16_st_tnsr(oc2, normalized, n2);
            v_bf16_st_tnsr(oc3, normalized, n3);
        }
    }
}
