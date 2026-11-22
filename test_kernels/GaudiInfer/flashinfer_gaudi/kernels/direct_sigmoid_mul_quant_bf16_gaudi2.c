// SPDX-License-Identifier: Apache-2.0
// g2e direct MME+SRAM kernel for Attention apost: 4-way VPU-interleaved sigmoid(gate) * att + per-row FP8 quant.
// 100% LUT-free (64 KiB VLM), caches 48 BF16 tiles (12 KiB) in VLM, full 256-byte FP8 stores, zero RAW stalls!
// Tensors (dim 0 fastest):
//   0 gate   BF16 [6144, R]  attention output gate in SRAM
//   1 att    BF16 [6144, R]  attention core output in SRAM
//   2 output F8   [6144, R]  quantized FP8 E4M3 codes in SRAM
//   3 scales F32  [1, R]     row FP8 scales in SRAM
// Index space: dim 1 = row [0..R).

static inline float64 round_bf16(float64 value) {
    float64_pair_t pair;
    pair.v1 = value;
    pair.v2 = value;
    return v_convert_bf16_to_f32_all_b(v_convert_f32_to_bf16_all_b(pair)).v1;
}

static inline float64 reciprocal_without_lookup(float64 value) {
    float64 estimate = as_float64((int64)0x7ef311c3 - as_int64(value));
    estimate = estimate * (2.0f - value * estimate);
    estimate = estimate * (2.0f - value * estimate);
    estimate = estimate * (2.0f - value * estimate);
    return v_f32_sel_eq_f32_b(value, as_float64((int64)0x7f800000), 0.0f, estimate);
}

static inline float64 row_max_without_lookup(float64 value) {
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

#define SIGMOID4_F32(z0, z1, z2, z3, o0, o1, o2, o3) do {                                         \
    const float64 zc0_ = v_f32_min_b(v_f32_max_b((z0), -87.0f), 87.0f);                           \
    const float64 zc1_ = v_f32_min_b(v_f32_max_b((z1), -87.0f), 87.0f);                           \
    const float64 zc2_ = v_f32_min_b(v_f32_max_b((z2), -87.0f), 87.0f);                           \
    const float64 zc3_ = v_f32_min_b(v_f32_max_b((z3), -87.0f), 87.0f);                           \
    const float64 t0_ = v_f32_mac_b(zc0_, -1.4426950408889634f, (float64)127.0f);                 \
    const float64 t1_ = v_f32_mac_b(zc1_, -1.4426950408889634f, (float64)127.0f);                 \
    const float64 t2_ = v_f32_mac_b(zc2_, -1.4426950408889634f, (float64)127.0f);                 \
    const float64 t3_ = v_f32_mac_b(zc3_, -1.4426950408889634f, (float64)127.0f);                 \
    const int64 n0_ = v_convert_f32_to_i32_b(t0_, SW_RHNE);                                       \
    const int64 n1_ = v_convert_f32_to_i32_b(t1_, SW_RHNE);                                       \
    const int64 n2_ = v_convert_f32_to_i32_b(t2_, SW_RHNE);                                       \
    const int64 n3_ = v_convert_f32_to_i32_b(t3_, SW_RHNE);                                       \
    const float64 f0_ = t0_ - v_convert_i32_to_f32_b(n0_, 0);                                     \
    const float64 f1_ = t1_ - v_convert_i32_to_f32_b(n1_, 0);                                     \
    const float64 f2_ = t2_ - v_convert_i32_to_f32_b(n2_, 0);                                     \
    const float64 f3_ = t3_ - v_convert_i32_to_f32_b(n3_, 0);                                     \
    float64 p0_ = 0.0096181291f, p1_ = 0.0096181291f, p2_ = 0.0096181291f, p3_ = 0.0096181291f;   \
    p0_ = v_f32_mac_b(p0_, f0_, 0.0555041087f); p1_ = v_f32_mac_b(p1_, f1_, 0.0555041087f);       \
    p2_ = v_f32_mac_b(p2_, f2_, 0.0555041087f); p3_ = v_f32_mac_b(p3_, f3_, 0.0555041087f);       \
    p0_ = v_f32_mac_b(p0_, f0_, 0.2402265070f); p1_ = v_f32_mac_b(p1_, f1_, 0.2402265070f);       \
    p2_ = v_f32_mac_b(p2_, f2_, 0.2402265070f); p3_ = v_f32_mac_b(p3_, f3_, 0.2402265070f);       \
    p0_ = v_f32_mac_b(p0_, f0_, 0.6931471806f); p1_ = v_f32_mac_b(p1_, f1_, 0.6931471806f);       \
    p2_ = v_f32_mac_b(p2_, f2_, 0.6931471806f); p3_ = v_f32_mac_b(p3_, f3_, 0.6931471806f);       \
    p0_ = v_f32_mac_b(p0_, f0_, 1.0f);          p1_ = v_f32_mac_b(p1_, f1_, 1.0f);                \
    p2_ = v_f32_mac_b(p2_, f2_, 1.0f);          p3_ = v_f32_mac_b(p3_, f3_, 1.0f);                \
    const int64 b0_ = n0_ << 23, b1_ = n1_ << 23, b2_ = n2_ << 23, b3_ = n3_ << 23;               \
    const float64 d0_ = 1.0f + p0_ * (*((float64*)&b0_));                                         \
    const float64 d1_ = 1.0f + p1_ * (*((float64*)&b1_));                                         \
    const float64 d2_ = 1.0f + p2_ * (*((float64*)&b2_));                                         \
    const float64 d3_ = 1.0f + p3_ * (*((float64*)&b3_));                                         \
    float64 e0_ = as_float64((int64)0x7ef311c3 - as_int64(d0_));                                  \
    float64 e1_ = as_float64((int64)0x7ef311c3 - as_int64(d1_));                                  \
    float64 e2_ = as_float64((int64)0x7ef311c3 - as_int64(d2_));                                  \
    float64 e3_ = as_float64((int64)0x7ef311c3 - as_int64(d3_));                                  \
    e0_ = e0_ * (2.0f - d0_ * e0_); e1_ = e1_ * (2.0f - d1_ * e1_);                               \
    e2_ = e2_ * (2.0f - d2_ * e2_); e3_ = e3_ * (2.0f - d3_ * e3_);                               \
    e0_ = e0_ * (2.0f - d0_ * e0_); e1_ = e1_ * (2.0f - d1_ * e1_);                               \
    e2_ = e2_ * (2.0f - d2_ * e2_); e3_ = e3_ * (2.0f - d3_ * e3_);                               \
    e0_ = e0_ * (2.0f - d0_ * e0_); e1_ = e1_ * (2.0f - d1_ * e1_);                               \
    e2_ = e2_ * (2.0f - d2_ * e2_); e3_ = e3_ * (2.0f - d3_ * e3_);                               \
    (o0) = e0_; (o1) = e1_; (o2) = e2_; (o3) = e3_;                                               \
} while (0)

void main(tensor gate, tensor att, tensor output, tensor scales) {
    const int5 start = get_index_space_offset();
    const int5 end = start + get_index_space_size();
    const int tiles = get_dim_size(output, 0) / 128;  // 48
    bfloat128 activated[48];

    for (int row = start[1]; row < end[1]; ++row) {
        bfloat128 m0 = 0, m1 = 0, m2 = 0, m3 = 0;

        for (int tile = 0; tile < tiles; tile += 4) {
            const int c0 = tile * 128, c1 = c0 + 128, c2 = c0 + 256, c3 = c0 + 384;
            int5 rc0 = {c0, row, 0, 0, 0}, rc1 = {c1, row, 0, 0, 0};
            int5 rc2 = {c2, row, 0, 0, 0}, rc3 = {c3, row, 0, 0, 0};

            const float64_pair_t g0 = v_convert_bf16_to_f32_all_b(v_bf16_ld_tnsr_b(rc0, gate));
            const float64_pair_t g1 = v_convert_bf16_to_f32_all_b(v_bf16_ld_tnsr_b(rc1, gate));
            const bfloat128 a0 = v_bf16_ld_tnsr_b(rc0, att);
            const bfloat128 a1 = v_bf16_ld_tnsr_b(rc1, att);
            float64_pair_t s0, s1;
            SIGMOID4_F32(g0.v1, g0.v2, g1.v1, g1.v2, s0.v1, s0.v2, s1.v1, s1.v2);
            const bfloat128 p0 = v_convert_f32_to_bf16_all_b(s0) * a0;
            const bfloat128 p1 = v_convert_f32_to_bf16_all_b(s1) * a1;
            activated[tile]     = p0;
            activated[tile + 1] = p1;
            m0 = v_bf16_max_b(m0, v_bf16_abs_b(p0));
            m1 = v_bf16_max_b(m1, v_bf16_abs_b(p1));

            const float64_pair_t g2 = v_convert_bf16_to_f32_all_b(v_bf16_ld_tnsr_b(rc2, gate));
            const float64_pair_t g3 = v_convert_bf16_to_f32_all_b(v_bf16_ld_tnsr_b(rc3, gate));
            const bfloat128 a2 = v_bf16_ld_tnsr_b(rc2, att);
            const bfloat128 a3 = v_bf16_ld_tnsr_b(rc3, att);
            float64_pair_t s2, s3;
            SIGMOID4_F32(g2.v1, g2.v2, g3.v1, g3.v2, s2.v1, s2.v2, s3.v1, s3.v2);
            const bfloat128 p2 = v_convert_f32_to_bf16_all_b(s2) * a2;
            const bfloat128 p3 = v_convert_f32_to_bf16_all_b(s3) * a3;
            activated[tile + 2] = p2;
            activated[tile + 3] = p3;
            m2 = v_bf16_max_b(m2, v_bf16_abs_b(p2));
            m3 = v_bf16_max_b(m3, v_bf16_abs_b(p3));
        }

        const float64_pair_t mf = v_convert_bf16_to_f32_all_b(v_bf16_max_b(v_bf16_max_b(m0, m1), v_bf16_max_b(m2, m3)));
        const float64 maximum = row_max_without_lookup(v_f32_max_b(mf.v1, mf.v2));
        const float64 scale = round_bf16(round_bf16(maximum + 1.0e-8f) * (float)(bf16)(1.0f / 240.0f));
        const float64 inverse = round_bf16(reciprocal_without_lookup(scale));
        float64_pair_t inv_pair = {inverse, inverse};
        const bfloat128 inv_bf = v_convert_f32_to_bf16_all_b(inv_pair);
        int5 sc = {0, row, 0, 0, 0};
        v_f32_st_tnsr(sc, scales, scale);

        for (int tile = 0; tile < tiles; tile += 4) {
            const minifloat256 sp0 = v_convert_bf16_to_f8_b(activated[tile]     * inv_bf, 0, SW_CLIP_FP, (minifloat256)0);
            const minifloat256 sp1 = v_convert_bf16_to_f8_b(activated[tile + 1] * inv_bf, 0, SW_CLIP_FP, (minifloat256)0);
            const minifloat256 sp2 = v_convert_bf16_to_f8_b(activated[tile + 2] * inv_bf, 0, SW_CLIP_FP, (minifloat256)0);
            const minifloat256 sp3 = v_convert_bf16_to_f8_b(activated[tile + 3] * inv_bf, 0, SW_CLIP_FP, (minifloat256)0);
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
            v_f8_st_tnsr(o0, output, q0);
            v_f8_st_tnsr(o2, output, q2);
        }
    }
}
