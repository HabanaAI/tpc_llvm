// SPDX-License-Identifier: Apache-2.0
// g2e direct MME+SRAM Span-0 entry kernel: fused embed gather + L0 RMSNorm + per-row FP8 quant -> SRAM.
// Eliminates Synapse embed_recipe(R) launch + pre-Direct stream_.synchronize()!
// 100% LUT-free (64 KiB VLM), 4-way VPU-interleaved, full 256B FP8 stores.
// Tensors (dim 0 fastest):
//   0 ids          I32  [R]      token ids in HBM (ids_ + 0)
//   1 embed        BF16 [H, V]   embedding table in HBM
//   2 norm_weight  BF16 [H]      L0 in_norm1p in HBM
//   3 quantized    F8   [H, R]   FP8 E4M3 codes in SRAM (s_x8)
//   4 scales       F32  [1, R]   row FP8 scales in SRAM (s_xs)
//   5 residual_out BF16 [H, R]   gathered embeddings in SRAM (s_res0)
//   6 x_norm       BF16 [H, R]   RMSNorm output BF16 in SRAM (s_xnorm)
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

void main(tensor ids, tensor embed, tensor norm_weight,
          tensor quantized, tensor scales, tensor residual_out, tensor x_norm,
          float epsilon, float inverse_width, float inverse_range) {
    const int5 start = get_index_space_offset();
    const int5 end = start + get_index_space_size();
    const int width = get_dim_size(embed, 0);
    const int tiles = width / 128;  // H=5120 -> 40 (multiple of 4)
    bfloat128 cached[64];

    for (int row = start[1]; row < end[1]; ++row) {
        int5 idc = {row, 0, 0, 0, 0};
        const int tok = s_i32_ld_g((__global__ int*)gen_addr(idc, ids));

        float128 sq0 = {0}, sq1 = {0}, sq2 = {0}, sq3 = {0};
        bfloat128 m0 = 0, m1 = 0, m2 = 0, m3 = 0;

        for (int tile = 0; tile < tiles; tile += 4) {
            const int c0 = tile * 128, c1 = c0 + 128, c2 = c0 + 256, c3 = c0 + 384;
            int5 ec0 = {c0, tok, 0, 0, 0}, ec1 = {c1, tok, 0, 0, 0}, ec2 = {c2, tok, 0, 0, 0}, ec3 = {c3, tok, 0, 0, 0};
            int5 oc0 = {c0, row, 0, 0, 0}, oc1 = {c1, row, 0, 0, 0}, oc2 = {c2, row, 0, 0, 0}, oc3 = {c3, row, 0, 0, 0};
            int5 wc0 = {c0, 0,   0, 0, 0}, wc1 = {c1, 0,   0, 0, 0}, wc2 = {c2, 0,   0, 0, 0}, wc3 = {c3, 0,   0, 0, 0};

            const bfloat128 s0 = v_bf16_ld_tnsr_b(ec0, embed);
            const bfloat128 s1 = v_bf16_ld_tnsr_b(ec1, embed);
            const bfloat128 s2 = v_bf16_ld_tnsr_b(ec2, embed);
            const bfloat128 s3 = v_bf16_ld_tnsr_b(ec3, embed);

            v_bf16_st_tnsr(oc0, residual_out, s0);
            v_bf16_st_tnsr(oc1, residual_out, s1);
            v_bf16_st_tnsr(oc2, residual_out, s2);
            v_bf16_st_tnsr(oc3, residual_out, s3);

            sq0 = v_bf16_mac_acc32_b(s0, s0, sq0, (e_no_negation) << 1);
            sq1 = v_bf16_mac_acc32_b(s1, s1, sq1, (e_no_negation) << 1);
            sq2 = v_bf16_mac_acc32_b(s2, s2, sq2, (e_no_negation) << 1);
            sq3 = v_bf16_mac_acc32_b(s3, s3, sq3, (e_no_negation) << 1);

            const bfloat128 w0 = s0 * v_bf16_ld_tnsr_b(wc0, norm_weight);
            const bfloat128 w1 = s1 * v_bf16_ld_tnsr_b(wc1, norm_weight);
            const bfloat128 w2 = s2 * v_bf16_ld_tnsr_b(wc2, norm_weight);
            const bfloat128 w3 = s3 * v_bf16_ld_tnsr_b(wc3, norm_weight);

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
        float64_pair_t rp = {rrms, rrms}, ip = {inverse, inverse};
        const bfloat128 rrms_bf = v_convert_f32_to_bf16_all_b(rp);
        const bfloat128 inv_bf = v_convert_f32_to_bf16_all_b(ip);
        int5 sc = {0, row, 0, 0, 0};
        v_f32_st_tnsr(sc, scales, scale);

        for (int tile = 0; tile < tiles; tile += 4) {
            const bfloat128 xn0 = cached[tile]     * rrms_bf;
            const bfloat128 xn1 = cached[tile + 1] * rrms_bf;
            const bfloat128 xn2 = cached[tile + 2] * rrms_bf;
            const bfloat128 xn3 = cached[tile + 3] * rrms_bf;
            int5 c0 = {tile * 128,       row, 0, 0, 0};
            int5 c1 = {(tile + 1) * 128, row, 0, 0, 0};
            int5 c2 = {(tile + 2) * 128, row, 0, 0, 0};
            int5 c3 = {(tile + 3) * 128, row, 0, 0, 0};
            v_bf16_st_tnsr(c0, x_norm, xn0);
            v_bf16_st_tnsr(c1, x_norm, xn1);
            v_bf16_st_tnsr(c2, x_norm, xn2);
            v_bf16_st_tnsr(c3, x_norm, xn3);
            const minifloat256 sp0 = v_convert_bf16_to_f8_b(xn0 * inv_bf, 0, SW_CLIP_FP, (minifloat256)0);
            const minifloat256 sp1 = v_convert_bf16_to_f8_b(xn1 * inv_bf, 0, SW_CLIP_FP, (minifloat256)0);
            const minifloat256 sp2 = v_convert_bf16_to_f8_b(xn2 * inv_bf, 0, SW_CLIP_FP, (minifloat256)0);
            const minifloat256 sp3 = v_convert_bf16_to_f8_b(xn3 * inv_bf, 0, SW_CLIP_FP, (minifloat256)0);
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
            v_f8_st_tnsr(c0, quantized, q0);
            v_f8_st_tnsr(c2, quantized, q2);
        }
    }
}
