// SPDX-License-Identifier: Apache-2.0
// g2e direct MME+SRAM kernel: 4-way VPU-interleaved dequant(down/out) + residual add + RMSNorm + per-row FP8 quant.
// LUT-free (uses norm_math_gaudi2.h positive_rsqrt + row_sum; caches H=5120 -> 40 tiles in VLM), full 256B FP8 stores!
// Tensors (dim 0 fastest):
//   0 raw          BF16 [H, R]  unscaled MME output in SRAM
//   1 sa           F32  [1, R]  row activation scale in SRAM
//   2 swb          F32  [H]     weight column scales, each 128-col chunk pre-interleaved as [even 64, odd 64]
//   3 residual     BF16 [H, R]  incoming residual in SRAM
//   4 norm_weight  BF16 [H]     RMSNorm weight (1 + w already folded: post_norm1p / in_norm1p)
//   5 quantized    F8   [H, R]  FP8 E4M3 codes for next GEMM in SRAM
//   6 scales       F32  [1, R]  row FP8 scales for next GEMM in SRAM
//   7 residual_out BF16 [H, R]  updated residual (residual + dequant(raw)) in SRAM
// Scalar params (SRF 0..2): float epsilon, float inverse_width (1/H), float inverse_range (1/240).
// Index space: dim 1 = row [0..R).

#include "norm_math_gaudi2.h"

static inline float64 bf16_round(float64 value) {
    float64_pair_t pair;
    pair.v1 = value;
    pair.v2 = value;
    return v_convert_bf16_to_f32_all_b(v_convert_f32_to_bf16_all_b(pair)).v1;
}

static inline float64 positive_reciprocal(float64 value) {
    float64 estimate = as_float64((int64)0x7ef311c3 - as_int64(value));
    estimate = estimate * (2.0f - value * estimate);
    estimate = estimate * (2.0f - value * estimate);
    estimate = estimate * (2.0f - value * estimate);
    return v_f32_sel_eq_f32_b(value, as_float64((int64)0x7f800000), 0.0f, estimate);
}

static inline float64 row_max(float64 value) {
    value = v_f32_max_b(value, v_f32_mov_dual_group_all_b(value, 0xffffffff, 1, 0, 3, 2, MkWrA(3, 3, 3, 3), 0));
    value = v_f32_max_b(value, v_f32_mov_dual_group_all_b(value, 0xffffffff, 2, 3, 0, 1, MkWrA(3, 3, 3, 3), 0));
    value = v_f32_max_b(value, v_f32_mov_group_b(value, 0xffffffff, 63, 0));
    float64 result = 0;
    #pragma loop_unroll(8)
    for (int lane = 0; lane < 8; ++lane) {
        const uchar256 broadcast = 0x80 | lane;
        result = v_f32_max_b(result, v_f32_shuffle_b(value, broadcast, 0, value));
    }
    return result;
}

void main(tensor raw, tensor sa, tensor swb, tensor residual, tensor norm_weight,
          tensor quantized, tensor scales, tensor residual_out,
          float epsilon, float inverse_width, float inverse_range) {
    const int5 start = get_index_space_offset();
    const int5 end = start + get_index_space_size();
    const int width = get_dim_size(raw, 0);
    const int tiles = width / 128;  // H=5120 -> 40 (multiple of 4)
    bfloat128 cached[64];

    for (int row = start[1]; row < end[1]; ++row) {
        int5 sc = {0, row, 0, 0, 0};
        const float64 a = v_broadcast_element_f32(v_f32_ld_tnsr_b(sc, sa), 0);
        float128 sq0 = {0}, sq1 = {0}, sq2 = {0}, sq3 = {0};
        bfloat128 m0 = 0, m1 = 0, m2 = 0, m3 = 0;

        for (int tile = 0; tile < tiles; tile += 4) {
            const int c0 = tile * 128, c1 = c0 + 128, c2 = c0 + 256, c3 = c0 + 384;
            int5 ic0 = {c0, row, 0, 0, 0}, ic1 = {c1, row, 0, 0, 0}, ic2 = {c2, row, 0, 0, 0}, ic3 = {c3, row, 0, 0, 0};
            int5 wc0a = {c0, 0, 0, 0, 0}, wc0b = {c0 + 64, 0, 0, 0, 0};
            int5 wc1a = {c1, 0, 0, 0, 0}, wc1b = {c1 + 64, 0, 0, 0, 0};
            int5 wc2a = {c2, 0, 0, 0, 0}, wc2b = {c2 + 64, 0, 0, 0, 0};
            int5 wc3a = {c3, 0, 0, 0, 0}, wc3b = {c3 + 64, 0, 0, 0, 0};

            const float64_pair_t r0 = v_convert_bf16_to_f32_all_b(v_bf16_ld_tnsr_b(ic0, raw));
            const float64_pair_t r1 = v_convert_bf16_to_f32_all_b(v_bf16_ld_tnsr_b(ic1, raw));
            const float64_pair_t r2 = v_convert_bf16_to_f32_all_b(v_bf16_ld_tnsr_b(ic2, raw));
            const float64_pair_t r3 = v_convert_bf16_to_f32_all_b(v_bf16_ld_tnsr_b(ic3, raw));

            const float64 k0a = v_f32_ld_tnsr_b(wc0a, swb) * a, k0b = v_f32_ld_tnsr_b(wc0b, swb) * a;
            const float64 k1a = v_f32_ld_tnsr_b(wc1a, swb) * a, k1b = v_f32_ld_tnsr_b(wc1b, swb) * a;
            const float64 k2a = v_f32_ld_tnsr_b(wc2a, swb) * a, k2b = v_f32_ld_tnsr_b(wc2b, swb) * a;
            const float64 k3a = v_f32_ld_tnsr_b(wc3a, swb) * a, k3b = v_f32_ld_tnsr_b(wc3b, swb) * a;

            float64_pair_t d0 = {r0.v1 * k0a, r0.v2 * k0b};
            float64_pair_t d1 = {r1.v1 * k1a, r1.v2 * k1b};
            float64_pair_t d2 = {r2.v1 * k2a, r2.v2 * k2b};
            float64_pair_t d3 = {r3.v1 * k3a, r3.v2 * k3b};

            const bfloat128 b0 = v_convert_f32_to_bf16_all_b(d0);
            const bfloat128 b1 = v_convert_f32_to_bf16_all_b(d1);
            const bfloat128 b2 = v_convert_f32_to_bf16_all_b(d2);
            const bfloat128 b3 = v_convert_f32_to_bf16_all_b(d3);

            const bfloat128 s0 = b0 + v_bf16_ld_tnsr_b(ic0, residual);
            const bfloat128 s1 = b1 + v_bf16_ld_tnsr_b(ic1, residual);
            const bfloat128 s2 = b2 + v_bf16_ld_tnsr_b(ic2, residual);
            const bfloat128 s3 = b3 + v_bf16_ld_tnsr_b(ic3, residual);

            v_bf16_st_tnsr(ic0, residual_out, s0);
            v_bf16_st_tnsr(ic1, residual_out, s1);
            v_bf16_st_tnsr(ic2, residual_out, s2);
            v_bf16_st_tnsr(ic3, residual_out, s3);

            sq0 = v_bf16_mac_acc32_b(s0, s0, sq0, (e_no_negation) << 1);
            sq1 = v_bf16_mac_acc32_b(s1, s1, sq1, (e_no_negation) << 1);
            sq2 = v_bf16_mac_acc32_b(s2, s2, sq2, (e_no_negation) << 1);
            sq3 = v_bf16_mac_acc32_b(s3, s3, sq3, (e_no_negation) << 1);

            const bfloat128 w0 = s0 * v_bf16_ld_tnsr_b(wc0a, norm_weight);
            const bfloat128 w1 = s1 * v_bf16_ld_tnsr_b(wc1a, norm_weight);
            const bfloat128 w2 = s2 * v_bf16_ld_tnsr_b(wc2a, norm_weight);
            const bfloat128 w3 = s3 * v_bf16_ld_tnsr_b(wc3a, norm_weight);

            cached[tile]     = w0;
            cached[tile + 1] = w1;
            cached[tile + 2] = w2;
            cached[tile + 3] = w3;

            m0 = v_bf16_max_b(m0, v_bf16_abs_b(w0));
            m1 = v_bf16_max_b(m1, v_bf16_abs_b(w1));
            m2 = v_bf16_max_b(m2, v_bf16_abs_b(w2));
            m3 = v_bf16_max_b(m3, v_bf16_abs_b(w3));
        }

        const float64 sum_all = ((sq0.v1 + sq1.v1) + (sq2.v1 + sq3.v1)) +
                                ((sq0.v2 + sq1.v2) + (sq2.v2 + sq3.v2));
        const float64 rrms = positive_rsqrt(row_sum(sum_all) * inverse_width + epsilon);
        const float64_pair_t mf = v_convert_bf16_to_f32_all_b(v_bf16_max_b(v_bf16_max_b(m0, m1), v_bf16_max_b(m2, m3)));
        const float64 maximum = bf16_round(row_max(v_f32_max_b(mf.v1, mf.v2)) * rrms);
        const float64 scale = bf16_round(bf16_round(maximum + 1.0e-8f) * inverse_range);
        const float64 inverse = bf16_round(positive_reciprocal(scale));
        float64_pair_t ip = {inverse, inverse};
        const bfloat128 inv_bf = v_convert_f32_to_bf16_all_b(ip);
        v_f32_st_tnsr(sc, scales, scale);

        for (int tile = 0; tile < tiles; tile += 4) {
            float64_pair_t v0 = v_convert_bf16_to_f32_all_b(cached[tile]);
            float64_pair_t v1 = v_convert_bf16_to_f32_all_b(cached[tile + 1]);
            float64_pair_t v2 = v_convert_bf16_to_f32_all_b(cached[tile + 2]);
            float64_pair_t v3 = v_convert_bf16_to_f32_all_b(cached[tile + 3]);
            v0.v1 *= rrms; v0.v2 *= rrms;
            v1.v1 *= rrms; v1.v2 *= rrms;
            v2.v1 *= rrms; v2.v2 *= rrms;
            v3.v1 *= rrms; v3.v2 *= rrms;
            const bfloat128 s0 = v_convert_f32_to_bf16_all_b(v0) * inv_bf;
            const bfloat128 s1 = v_convert_f32_to_bf16_all_b(v1) * inv_bf;
            const bfloat128 s2 = v_convert_f32_to_bf16_all_b(v2) * inv_bf;
            const bfloat128 s3 = v_convert_f32_to_bf16_all_b(v3) * inv_bf;
            const minifloat256 sp0 = v_convert_bf16_to_f8_b(s0, 0, SW_CLIP_FP, (minifloat256)0);
            const minifloat256 sp1 = v_convert_bf16_to_f8_b(s1, 0, SW_CLIP_FP, (minifloat256)0);
            const minifloat256 sp2 = v_convert_bf16_to_f8_b(s2, 0, SW_CLIP_FP, (minifloat256)0);
            const minifloat256 sp3 = v_convert_bf16_to_f8_b(s3, 0, SW_CLIP_FP, (minifloat256)0);
            minifloat256 p0 = v_f8_pack_b(sp0, SW_GROUP_0 | SW_STRIDE_2, (minifloat256)0);
            minifloat256 p1 = v_f8_pack_b(sp1, SW_GROUP_0 | SW_STRIDE_2, (minifloat256)0);
            minifloat256 p2 = v_f8_pack_b(sp2, SW_GROUP_0 | SW_STRIDE_2, (minifloat256)0);
            minifloat256 p3 = v_f8_pack_b(sp3, SW_GROUP_0 | SW_STRIDE_2, (minifloat256)0);
            p0 = v_f8_pack_b(sp0, SW_GROUP_1 | SW_STRIDE_2, p0);
            p1 = v_f8_pack_b(sp1, SW_GROUP_1 | SW_STRIDE_2, p1);
            p2 = v_f8_pack_b(sp2, SW_GROUP_1 | SW_STRIDE_2, p2);
            p3 = v_f8_pack_b(sp3, SW_GROUP_1 | SW_STRIDE_2, p3);
            minifloat256 q0 = v_f8_mov_dual_group_pack_b(p0, SW_PACK21, (minifloat256)0);
            const minifloat256 q1 = v_f8_mov_dual_group_pack_b(p1, SW_PACK21, (minifloat256)0);
            minifloat256 q2 = v_f8_mov_dual_group_pack_b(p2, SW_PACK21, (minifloat256)0);
            const minifloat256 q3 = v_f8_mov_dual_group_pack_b(p3, SW_PACK21, (minifloat256)0);
            q0 = v_f8_mov_dual_group_b(q1, 0xffffffff, 0, 2, MkWr(1, 1), q0);
            q2 = v_f8_mov_dual_group_b(q3, 0xffffffff, 0, 2, MkWr(1, 1), q2);
            q0 = v_f8_mov_dual_group_b(q1, 0xffffffff, 1, 3, MkWr(1, 1), q0);
            q2 = v_f8_mov_dual_group_b(q3, 0xffffffff, 1, 3, MkWr(1, 1), q2);
            int5 o0 = {tile * 128, row, 0, 0, 0};
            int5 o2 = {(tile + 2) * 128, row, 0, 0, 0};
            v_f8_st_tnsr(o0, quantized, q0);
            v_f8_st_tnsr(o2, quantized, q2);
        }
    }
}
