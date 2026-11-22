// SPDX-License-Identifier: Apache-2.0
// g2e direct MME+SRAM kernel: 4-way VPU-interleaved 1-pass per-head RMSNorm(core_f32) * z_silu + per-row FP8 quant.
// 100% LUT-free -> 64 KiB VLM budget -> caches all 48 heads x 128 F32 (24 KiB) in VLM!
// Zero 4-cycle VPU RAW stalls across 4-head interleaved SUM64_4, RSQRT4, and F32_TO_F8_QUAD!
// Tensors (dim 0 fastest):
//   0 core    F32  [128, 48, R]   gdn_mtp_wyod F32 output in SRAM
//   1 z_silu  BF16 [6144, R]      silu(dequant(z)) BF16 in SRAM
//   2 norm_w  BF16 [128]          gdn_norm weight in HBM
//   3 output  F8   [6144, R]      quantized FP8 E4M3 codes for MME(out) in SRAM
//   4 scales  F32  [1, R]         row FP8 scales for MME(out) in SRAM
// Scalar param (SRF 0): float eps.
// Index space: dim 1 = row [0..R) (split across 24 TPCs).

static inline float64 reciprocal_without_lookup(float64 value) {
    float64 estimate = as_float64((int64)0x7ef311c3 - as_int64(value));
    estimate = estimate * (2.0f - value * estimate);
    estimate = estimate * (2.0f - value * estimate);
    estimate = estimate * (2.0f - value * estimate);
    return v_f32_sel_eq_f32_b(value, as_float64((int64)0x7f800000), 0.0f, estimate);
}

static inline float64 max64(float64 x, uchar256 sh4, uchar256 sh2, uchar256 sh1) {
    const int all = SW_WR_LOWER_GROUP0 | SW_WR_UPPER_GROUP0 | SW_WR_LOWER_GROUP1 | SW_WR_UPPER_GROUP1 |
                    SW_WR_LOWER_GROUP2 | SW_WR_UPPER_GROUP2 | SW_WR_LOWER_GROUP3 | SW_WR_UPPER_GROUP3;
    const int grp = SW_GROUP0_EN | SW_GROUP1_EN | SW_DUAL_GROUP0_EN | SW_DUAL_GROUP1_EN |
                    SW_DUAL_GROUP2_EN | SW_DUAL_GROUP3_EN;
    x = v_f32_max_b(x, v_f32_mov_dual_group_all_b(x, 0xffffffff, 1, 0, 3, 2, all, x));
    x = v_f32_max_b(x, v_f32_mov_dual_group_all_b(x, 0xffffffff, 2, 3, 0, 1, all, x));
    x = v_f32_max_b(x, v_f32_mov_group_b(x, 0xffffffff, grp, x));
    x = v_f32_max_b(x, v_f32_shuffle_b(x, sh4, 0, x));
    x = v_f32_max_b(x, v_f32_shuffle_b(x, sh2, 0, x));
    x = v_f32_max_b(x, v_f32_shuffle_b(x, sh1, 0, x));
    return x;
}

static float64_pair_t bf16_to_f32_linear(bfloat128 input)
{
    bfloat128_pair_t unpacked;
    unpacked.v1 = v_bf16_unpack_b(
        input, ((e_group_0) << 8) | ((e_every_second_element) << 9) | ((e_lower_half_group) << 10), unpacked.v1);
    unpacked.v2 = v_bf16_unpack_b(
        input, ((e_group_1) << 8) | ((e_every_second_element) << 9) | ((e_lower_half_group) << 10), unpacked.v2);
    const bfloat128 first_groups = unpacked.v1;
    unpacked.v1 = v_bf16_mov_dual_group_b(unpacked.v2, 0xFFFFFFFF, 0, 1, MkWr(1, 1), unpacked.v1);
    unpacked.v1 = v_bf16_mov_dual_group_b(first_groups, 0xFFFFFFFF, 1, 2, MkWr(1, 1), unpacked.v1);
    unpacked.v1 = v_bf16_mov_dual_group_b(unpacked.v2, 0xFFFFFFFF, 1, 3, MkWr(1, 1), unpacked.v1);
    unpacked.v2 = v_bf16_mov_dual_group_b(first_groups, 0xFFFFFFFF, 2, 0, MkWr(1, 1), unpacked.v2);
    unpacked.v2 = v_bf16_mov_dual_group_b(unpacked.v2, 0xFFFFFFFF, 2, 1, MkWr(1, 1), unpacked.v2);
    unpacked.v2 = v_bf16_mov_dual_group_b(first_groups, 0xFFFFFFFF, 3, 2, MkWr(1, 1), unpacked.v2);
    const float64_pair_t first = v_convert_bf16_to_f32_all_b(unpacked.v1);
    const float64_pair_t second = v_convert_bf16_to_f32_all_b(unpacked.v2);
    float64_pair_t result;
    result.v1 = first.v1;
    result.v2 = second.v1;
    return result;
}

#define SUM64_4(a0, a1, a2, a3) do {                                                                 \
    (a0) += v_f32_mov_dual_group_all_b((a0), 0xffffffff, 1, 0, 3, 2, all_dg, (a0));                  \
    (a1) += v_f32_mov_dual_group_all_b((a1), 0xffffffff, 1, 0, 3, 2, all_dg, (a1));                  \
    (a2) += v_f32_mov_dual_group_all_b((a2), 0xffffffff, 1, 0, 3, 2, all_dg, (a2));                  \
    (a3) += v_f32_mov_dual_group_all_b((a3), 0xffffffff, 1, 0, 3, 2, all_dg, (a3));                  \
    (a0) += v_f32_mov_dual_group_all_b((a0), 0xffffffff, 2, 3, 0, 1, all_dg, (a0));                  \
    (a1) += v_f32_mov_dual_group_all_b((a1), 0xffffffff, 2, 3, 0, 1, all_dg, (a1));                  \
    (a2) += v_f32_mov_dual_group_all_b((a2), 0xffffffff, 2, 3, 0, 1, all_dg, (a2));                  \
    (a3) += v_f32_mov_dual_group_all_b((a3), 0xffffffff, 2, 3, 0, 1, all_dg, (a3));                  \
    (a0) += v_f32_mov_group_b((a0), 0xffffffff, swap_grp, (a0));                                     \
    (a1) += v_f32_mov_group_b((a1), 0xffffffff, swap_grp, (a1));                                     \
    (a2) += v_f32_mov_group_b((a2), 0xffffffff, swap_grp, (a2));                                     \
    (a3) += v_f32_mov_group_b((a3), 0xffffffff, swap_grp, (a3));                                     \
    (a0) += v_f32_shuffle_b((a0), sh4, 0, (a0));                                                     \
    (a1) += v_f32_shuffle_b((a1), sh4, 0, (a1));                                                     \
    (a2) += v_f32_shuffle_b((a2), sh4, 0, (a2));                                                     \
    (a3) += v_f32_shuffle_b((a3), sh4, 0, (a3));                                                     \
    (a0) += v_f32_shuffle_b((a0), sh2, 0, (a0));                                                     \
    (a1) += v_f32_shuffle_b((a1), sh2, 0, (a1));                                                     \
    (a2) += v_f32_shuffle_b((a2), sh2, 0, (a2));                                                     \
    (a3) += v_f32_shuffle_b((a3), sh2, 0, (a3));                                                     \
    (a0) += v_f32_shuffle_b((a0), sh1, 0, (a0));                                                     \
    (a1) += v_f32_shuffle_b((a1), sh1, 0, (a1));                                                     \
    (a2) += v_f32_shuffle_b((a2), sh1, 0, (a2));                                                     \
    (a3) += v_f32_shuffle_b((a3), sh1, 0, (a3));                                                     \
} while (0)

#define RSQRT4(x0, x1, x2, x3, y0, y1, y2, y3) do {                                                  \
    float64 h0_ = 0.5f * (x0), h1_ = 0.5f * (x1), h2_ = 0.5f * (x2), h3_ = 0.5f * (x3);              \
    (y0) = as_float64((int64)0x5f3759df - (as_int64(x0) >> 1));                                      \
    (y1) = as_float64((int64)0x5f3759df - (as_int64(x1) >> 1));                                      \
    (y2) = as_float64((int64)0x5f3759df - (as_int64(x2) >> 1));                                      \
    (y3) = as_float64((int64)0x5f3759df - (as_int64(x3) >> 1));                                      \
    float64 t0_ = (y0) * (y0), t1_ = (y1) * (y1), t2_ = (y2) * (y2), t3_ = (y3) * (y3);              \
    t0_ = 1.5f - h0_ * t0_; t1_ = 1.5f - h1_ * t1_; t2_ = 1.5f - h2_ * t2_; t3_ = 1.5f - h3_ * t3_;  \
    (y0) *= t0_; (y1) *= t1_; (y2) *= t2_; (y3) *= t3_;                                              \
    t0_ = (y0) * (y0); t1_ = (y1) * (y1); t2_ = (y2) * (y2); t3_ = (y3) * (y3);                      \
    t0_ = 1.5f - h0_ * t0_; t1_ = 1.5f - h1_ * t1_; t2_ = 1.5f - h2_ * t2_; t3_ = 1.5f - h3_ * t3_;  \
    (y0) *= t0_; (y1) *= t1_; (y2) *= t2_; (y3) *= t3_;                                              \
} while (0)

void main(tensor core, tensor z_silu, tensor norm_w, tensor output, tensor scales, float eps) {
    const int5 start = get_index_space_offset();
    const int5 end = start + get_index_space_size();
    const int all_dg = SW_WR_LOWER_GROUP0 | SW_WR_UPPER_GROUP0 | SW_WR_LOWER_GROUP1 | SW_WR_UPPER_GROUP1 |
                       SW_WR_LOWER_GROUP2 | SW_WR_UPPER_GROUP2 | SW_WR_LOWER_GROUP3 | SW_WR_UPPER_GROUP3;
    const int swap_grp = SW_GROUP0_EN | SW_GROUP1_EN | SW_DUAL_GROUP0_EN | SW_DUAL_GROUP1_EN |
                         SW_DUAL_GROUP2_EN | SW_DUAL_GROUP3_EN;
    const uint64 lanes = read_lane_id_4b_b() & 7;
    const uint64 c4 = ((lanes ^ 4) | 0x80) * 0x01010101;
    const uint64 c2 = ((lanes ^ 2) | 0x80) * 0x01010101;
    const uint64 c1 = ((lanes ^ 1) | 0x80) * 0x01010101;
    const uchar256 sh4 = *((uchar256*)&c4);
    const uchar256 sh2 = *((uchar256*)&c2);
    const uchar256 sh1 = *((uchar256*)&c1);

    int5 wc = {0, 0, 0, 0, 0};
    const float64_pair_t wf = bf16_to_f32_linear(v_bf16_ld_tnsr_b(wc, norm_w));
    float64 cached[96];  // 48 heads x 2 float64 = 24 KiB in VLM

    for (int row = start[1]; row < end[1]; ++row) {
        float64 m0 = 0, m1 = 0, m2 = 0, m3 = 0;
        for (int h = 0; h < 48; h += 4) {
            int5 cc00 = {0, h,     row, 0, 0}, cc01 = {64, h,     row, 0, 0};
            int5 cc10 = {0, h + 1, row, 0, 0}, cc11 = {64, h + 1, row, 0, 0};
            int5 cc20 = {0, h + 2, row, 0, 0}, cc21 = {64, h + 2, row, 0, 0};
            int5 cc30 = {0, h + 3, row, 0, 0}, cc31 = {64, h + 3, row, 0, 0};
            const float64 a0 = v_f32_ld_tnsr_b(cc00, core), b0 = v_f32_ld_tnsr_b(cc01, core);
            const float64 a1 = v_f32_ld_tnsr_b(cc10, core), b1 = v_f32_ld_tnsr_b(cc11, core);
            const float64 a2 = v_f32_ld_tnsr_b(cc20, core), b2 = v_f32_ld_tnsr_b(cc21, core);
            const float64 a3 = v_f32_ld_tnsr_b(cc30, core), b3 = v_f32_ld_tnsr_b(cc31, core);

            float64 s0 = b0 * b0, s1 = b1 * b1, s2 = b2 * b2, s3 = b3 * b3;
            s0 = v_f32_mac_b(a0, a0, s0);
            s1 = v_f32_mac_b(a1, a1, s1);
            s2 = v_f32_mac_b(a2, a2, s2);
            s3 = v_f32_mac_b(a3, a3, s3);

            SUM64_4(s0, s1, s2, s3);
            const float64 v0 = s0 * (1.0f / 128.0f) + eps;
            const float64 v1 = s1 * (1.0f / 128.0f) + eps;
            const float64 v2 = s2 * (1.0f / 128.0f) + eps;
            const float64 v3 = s3 * (1.0f / 128.0f) + eps;
            float64 inv0, inv1, inv2, inv3;
            RSQRT4(v0, v1, v2, v3, inv0, inv1, inv2, inv3);

            int5 zc0 = {h * 128,       row, 0, 0, 0};
            int5 zc1 = {(h + 1) * 128, row, 0, 0, 0};
            int5 zc2 = {(h + 2) * 128, row, 0, 0, 0};
            int5 zc3 = {(h + 3) * 128, row, 0, 0, 0};
            const float64_pair_t z0 = bf16_to_f32_linear(v_bf16_ld_tnsr_b(zc0, z_silu));
            const float64_pair_t z1 = bf16_to_f32_linear(v_bf16_ld_tnsr_b(zc1, z_silu));
            const float64_pair_t z2 = bf16_to_f32_linear(v_bf16_ld_tnsr_b(zc2, z_silu));
            const float64_pair_t z3 = bf16_to_f32_linear(v_bf16_ld_tnsr_b(zc3, z_silu));

            const float64 g0a = (a0 * inv0 * wf.v1) * z0.v1;
            const float64 g0b = (b0 * inv0 * wf.v2) * z0.v2;
            const float64 g1a = (a1 * inv1 * wf.v1) * z1.v1;
            const float64 g1b = (b1 * inv1 * wf.v2) * z1.v2;
            const float64 g2a = (a2 * inv2 * wf.v1) * z2.v1;
            const float64 g2b = (b2 * inv2 * wf.v2) * z2.v2;
            const float64 g3a = (a3 * inv3 * wf.v1) * z3.v1;
            const float64 g3b = (b3 * inv3 * wf.v2) * z3.v2;

            cached[2 * h + 0] = g0a; cached[2 * h + 1] = g0b;
            cached[2 * h + 2] = g1a; cached[2 * h + 3] = g1b;
            cached[2 * h + 4] = g2a; cached[2 * h + 5] = g2b;
            cached[2 * h + 6] = g3a; cached[2 * h + 7] = g3b;

            m0 = v_f32_max_b(m0, v_f32_max_b(v_f32_abs_b(g0a), v_f32_abs_b(g0b)));
            m1 = v_f32_max_b(m1, v_f32_max_b(v_f32_abs_b(g1a), v_f32_abs_b(g1b)));
            m2 = v_f32_max_b(m2, v_f32_max_b(v_f32_abs_b(g2a), v_f32_abs_b(g2b)));
            m3 = v_f32_max_b(m3, v_f32_max_b(v_f32_abs_b(g3a), v_f32_abs_b(g3b)));
        }
        const float64 maximum = max64(v_f32_max_b(v_f32_max_b(m0, m1), v_f32_max_b(m2, m3)), sh4, sh2, sh1);
        const float64 scale = (maximum + 1.0e-8f) * (1.0f / 240.0f);
        const float64 inv = reciprocal_without_lookup(scale);
        int5 sc = {0, row, 0, 0, 0};
        v_f32_st_tnsr(sc, scales, scale);

        for (int g = 0; g < 24; ++g) {
            const float64 x0 = cached[4 * g + 0] * inv;
            const float64 x1 = cached[4 * g + 1] * inv;
            const float64 x2 = cached[4 * g + 2] * inv;
            const float64 x3 = cached[4 * g + 3] * inv;
            const minifloat256 sp0 = v_convert_f32_to_f8_b(x0, 0, SW_CLIP_FP, (minifloat256)0);
            const minifloat256 sp1 = v_convert_f32_to_f8_b(x1, 0, SW_CLIP_FP, (minifloat256)0);
            const minifloat256 sp2 = v_convert_f32_to_f8_b(x2, 0, SW_CLIP_FP, (minifloat256)0);
            const minifloat256 sp3 = v_convert_f32_to_f8_b(x3, 0, SW_CLIP_FP, (minifloat256)0);
            minifloat256 p0 = v_f8_pack_b(sp0, SW_GROUP_0 | SW_STRIDE_4, (minifloat256)0);
            minifloat256 p1 = v_f8_pack_b(sp1, SW_GROUP_0 | SW_STRIDE_4, (minifloat256)0);
            minifloat256 p2 = v_f8_pack_b(sp2, SW_GROUP_0 | SW_STRIDE_4, (minifloat256)0);
            minifloat256 p3 = v_f8_pack_b(sp3, SW_GROUP_0 | SW_STRIDE_4, (minifloat256)0);
            p0 = v_f8_pack_b(sp0, SW_GROUP_1 | SW_STRIDE_4, p0);
            p1 = v_f8_pack_b(sp1, SW_GROUP_1 | SW_STRIDE_4, p1);
            p2 = v_f8_pack_b(sp2, SW_GROUP_1 | SW_STRIDE_4, p2);
            p3 = v_f8_pack_b(sp3, SW_GROUP_1 | SW_STRIDE_4, p3);
            minifloat256 v0 = v_f8_mov_dual_group_pack_b(p0, SW_PACK41, (minifloat256)0);
            const minifloat256 v1 = v_f8_mov_dual_group_pack_b(p1, SW_PACK41, (minifloat256)0);
            const minifloat256 v2 = v_f8_mov_dual_group_pack_b(p2, SW_PACK41, (minifloat256)0);
            const minifloat256 v3 = v_f8_mov_dual_group_pack_b(p3, SW_PACK41, (minifloat256)0);
            v0 = v_f8_mov_dual_group_b(v1, 0xffffffff, 0, 1, MkWr(1, 1), v0);
            v0 = v_f8_mov_dual_group_b(v2, 0xffffffff, 0, 2, MkWr(1, 1), v0);
            v0 = v_f8_mov_dual_group_b(v3, 0xffffffff, 0, 3, MkWr(1, 1), v0);
            int5 oc = {g * 256, row, 0, 0, 0};
            v_f8_st_tnsr(oc, output, v0);
        }
    }
}
