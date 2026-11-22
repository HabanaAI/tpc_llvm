// SPDX-License-Identifier: Apache-2.0
// g2e: ULTRA-FAST single-pass GDN prefill output gate + row FP8 quantization, reading chunk-major out_c DIRECTLY!
// 4-head fully interleaved SUM64_4 + RSQRT4 + SILU4_BF16 + QUANT_QUAD (zero 4-cycle VPU RAW stalls!).
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

static inline float64 max64(float64 x, uchar256 c4, uchar256 c2, uchar256 c1) {
    const int all = SW_WR_LOWER_GROUP0 | SW_WR_UPPER_GROUP0 | SW_WR_LOWER_GROUP1 | SW_WR_UPPER_GROUP1 |
                    SW_WR_LOWER_GROUP2 | SW_WR_UPPER_GROUP2 | SW_WR_LOWER_GROUP3 | SW_WR_UPPER_GROUP3;
    x = v_f32_max_b(x, v_f32_mov_dual_group_all_b(x, 0xffffffff, 1, 0, 3, 2, all, x));
    x = v_f32_max_b(x, v_f32_mov_dual_group_all_b(x, 0xffffffff, 2, 3, 0, 1, all, x));
    x = v_f32_max_b(x, v_f32_mov_group_b(x, 0xffffffff, SW_GROUP0_EN | SW_GROUP1_EN | SW_DUAL_GROUP0_EN |
                                         SW_DUAL_GROUP1_EN | SW_DUAL_GROUP2_EN | SW_DUAL_GROUP3_EN, x));
    x = v_f32_max_b(x, v_f32_shuffle_b(x, c4, 0, x));
    x = v_f32_max_b(x, v_f32_shuffle_b(x, c2, 0, x));
    x = v_f32_max_b(x, v_f32_shuffle_b(x, c1, 0, x));
    return x;
}

#define SILU4_BF16(V0, V1, V2, V3, X0, X1, X2, X3) do {                                        \
    const bfloat128 xc0_ = v_bf16_min_b(v_bf16_max_b((X0), (bf16)(-80.0f)), (bf16)80.0f);       \
    const bfloat128 xc1_ = v_bf16_min_b(v_bf16_max_b((X1), (bf16)(-80.0f)), (bf16)80.0f);       \
    const bfloat128 xc2_ = v_bf16_min_b(v_bf16_max_b((X2), (bf16)(-80.0f)), (bf16)80.0f);       \
    const bfloat128 xc3_ = v_bf16_min_b(v_bf16_max_b((X3), (bf16)(-80.0f)), (bf16)80.0f);       \
    const bfloat128 t0_ = xc0_ * (bf16)(-1.44269504f);                                          \
    const bfloat128 t1_ = xc1_ * (bf16)(-1.44269504f);                                          \
    const bfloat128 t2_ = xc2_ * (bf16)(-1.44269504f);                                          \
    const bfloat128 t3_ = xc3_ * (bf16)(-1.44269504f);                                          \
    const short128 n0_ = v_convert_bf16_to_i16_b(t0_, SW_RHNE);                                 \
    const short128 n1_ = v_convert_bf16_to_i16_b(t1_, SW_RHNE);                                 \
    const short128 n2_ = v_convert_bf16_to_i16_b(t2_, SW_RHNE);                                 \
    const short128 n3_ = v_convert_bf16_to_i16_b(t3_, SW_RHNE);                                 \
    const bfloat128 f0_ = t0_ - v_convert_i16_to_bf16_b(n0_, 0);                                \
    const bfloat128 f1_ = t1_ - v_convert_i16_to_bf16_b(n1_, 0);                                \
    const bfloat128 f2_ = t2_ - v_convert_i16_to_bf16_b(n2_, 0);                                \
    const bfloat128 f3_ = t3_ - v_convert_i16_to_bf16_b(n3_, 0);                                \
    bfloat128 p0_ = v_bf16_mac_b((bfloat128)(bf16)0.2402265f, f0_, (bfloat128)(bf16)0.6931472f);\
    bfloat128 p1_ = v_bf16_mac_b((bfloat128)(bf16)0.2402265f, f1_, (bfloat128)(bf16)0.6931472f);\
    bfloat128 p2_ = v_bf16_mac_b((bfloat128)(bf16)0.2402265f, f2_, (bfloat128)(bf16)0.6931472f);\
    bfloat128 p3_ = v_bf16_mac_b((bfloat128)(bf16)0.2402265f, f3_, (bfloat128)(bf16)0.6931472f);\
    p0_ = v_bf16_mac_b(p0_, f0_, (bfloat128)(bf16)1.0f);                                        \
    p1_ = v_bf16_mac_b(p1_, f1_, (bfloat128)(bf16)1.0f);                                        \
    p2_ = v_bf16_mac_b(p2_, f2_, (bfloat128)(bf16)1.0f);                                        \
    p3_ = v_bf16_mac_b(p3_, f3_, (bfloat128)(bf16)1.0f);                                        \
    const short128 b0_ = (n0_ + (short128)127) << 7;                                            \
    const short128 b1_ = (n1_ + (short128)127) << 7;                                            \
    const short128 b2_ = (n2_ + (short128)127) << 7;                                            \
    const short128 b3_ = (n3_ + (short128)127) << 7;                                            \
    const bfloat128 d0_ = v_bf16_mac_b(p0_, *((bfloat128*)&b0_), (bfloat128)(bf16)1.0f);        \
    const bfloat128 d1_ = v_bf16_mac_b(p1_, *((bfloat128*)&b1_), (bfloat128)(bf16)1.0f);        \
    const bfloat128 d2_ = v_bf16_mac_b(p2_, *((bfloat128*)&b2_), (bfloat128)(bf16)1.0f);        \
    const bfloat128 d3_ = v_bf16_mac_b(p3_, *((bfloat128*)&b3_), (bfloat128)(bf16)1.0f);        \
    const short128 ri0_ = (short128)0x7ef3 - *((short128*)&d0_);                                \
    const short128 ri1_ = (short128)0x7ef3 - *((short128*)&d1_);                                \
    const short128 ri2_ = (short128)0x7ef3 - *((short128*)&d2_);                                \
    const short128 ri3_ = (short128)0x7ef3 - *((short128*)&d3_);                                \
    bfloat128 r0_ = *((bfloat128*)&ri0_), r1_ = *((bfloat128*)&ri1_);                           \
    bfloat128 r2_ = *((bfloat128*)&ri2_), r3_ = *((bfloat128*)&ri3_);                           \
    r0_ = r0_ * ((bfloat128)(bf16)2.0f - d0_ * r0_);                                            \
    r1_ = r1_ * ((bfloat128)(bf16)2.0f - d1_ * r1_);                                            \
    r2_ = r2_ * ((bfloat128)(bf16)2.0f - d2_ * r2_);                                            \
    r3_ = r3_ * ((bfloat128)(bf16)2.0f - d3_ * r3_);                                            \
    r0_ = r0_ * ((bfloat128)(bf16)2.0f - d0_ * r0_);                                            \
    r1_ = r1_ * ((bfloat128)(bf16)2.0f - d1_ * r1_);                                            \
    r2_ = r2_ * ((bfloat128)(bf16)2.0f - d2_ * r2_);                                            \
    r3_ = r3_ * ((bfloat128)(bf16)2.0f - d3_ * r3_);                                            \
    (V0) = (X0) * r0_;                                                                          \
    (V1) = (X1) * r1_;                                                                          \
    (V2) = (X2) * r2_;                                                                          \
    (V3) = (X3) * r3_;                                                                          \
} while (0)

#define SUM64_4(X0, X1, X2, X3) do {                                                            \
    const int all_ = SW_WR_LOWER_GROUP0 | SW_WR_UPPER_GROUP0 | SW_WR_LOWER_GROUP1 | SW_WR_UPPER_GROUP1 | \
                     SW_WR_LOWER_GROUP2 | SW_WR_UPPER_GROUP2 | SW_WR_LOWER_GROUP3 | SW_WR_UPPER_GROUP3;  \
    const int grps_ = SW_GROUP0_EN | SW_GROUP1_EN | SW_DUAL_GROUP0_EN | SW_DUAL_GROUP1_EN |     \
                      SW_DUAL_GROUP2_EN | SW_DUAL_GROUP3_EN;                                    \
    (X0) += v_f32_mov_dual_group_all_b((X0), 0xffffffff, 1, 0, 3, 2, all_, (X0));               \
    (X1) += v_f32_mov_dual_group_all_b((X1), 0xffffffff, 1, 0, 3, 2, all_, (X1));               \
    (X2) += v_f32_mov_dual_group_all_b((X2), 0xffffffff, 1, 0, 3, 2, all_, (X2));               \
    (X3) += v_f32_mov_dual_group_all_b((X3), 0xffffffff, 1, 0, 3, 2, all_, (X3));               \
    (X0) += v_f32_mov_dual_group_all_b((X0), 0xffffffff, 2, 3, 0, 1, all_, (X0));               \
    (X1) += v_f32_mov_dual_group_all_b((X1), 0xffffffff, 2, 3, 0, 1, all_, (X1));               \
    (X2) += v_f32_mov_dual_group_all_b((X2), 0xffffffff, 2, 3, 0, 1, all_, (X2));               \
    (X3) += v_f32_mov_dual_group_all_b((X3), 0xffffffff, 2, 3, 0, 1, all_, (X3));               \
    (X0) += v_f32_mov_group_b((X0), 0xffffffff, grps_, (X0));                                   \
    (X1) += v_f32_mov_group_b((X1), 0xffffffff, grps_, (X1));                                   \
    (X2) += v_f32_mov_group_b((X2), 0xffffffff, grps_, (X2));                                   \
    (X3) += v_f32_mov_group_b((X3), 0xffffffff, grps_, (X3));                                   \
    (X0) += v_f32_shuffle_b((X0), c4, 0, (X0));                                                 \
    (X1) += v_f32_shuffle_b((X1), c4, 0, (X1));                                                 \
    (X2) += v_f32_shuffle_b((X2), c4, 0, (X2));                                                 \
    (X3) += v_f32_shuffle_b((X3), c4, 0, (X3));                                                 \
    (X0) += v_f32_shuffle_b((X0), c2, 0, (X0));                                                 \
    (X1) += v_f32_shuffle_b((X1), c2, 0, (X1));                                                 \
    (X2) += v_f32_shuffle_b((X2), c2, 0, (X2));                                                 \
    (X3) += v_f32_shuffle_b((X3), c2, 0, (X3));                                                 \
    (X0) += v_f32_shuffle_b((X0), c1, 0, (X0));                                                 \
    (X1) += v_f32_shuffle_b((X1), c1, 0, (X1));                                                 \
    (X2) += v_f32_shuffle_b((X2), c1, 0, (X2));                                                 \
    (X3) += v_f32_shuffle_b((X3), c1, 0, (X3));                                                 \
} while (0)

void main(tensor out_c, tensor z, tensor w, tensor codes, tensor scales, float eps)
{
    const int5 start = get_index_space_offset();
    const int5 end = start + get_index_space_size();
    const int C_chunk = get_dim_size(out_c, 1);
    const int HV = get_dim_size(z, 0) / 128;
    int5 wc = {0, 0, 0, 0, 0};
    const float128 wf = v_convert_bf16_to_f32_all_b(v_bf16_ld_tnsr_b(wc, w));

    const uint64 l8 = read_lane_id_4b_b() & 7;
    const uint64 c4u = ((l8 ^ 4) | 0x80) * 0x01010101;
    const uint64 c2u = ((l8 ^ 2) | 0x80) * 0x01010101;
    const uint64 c1u = ((l8 ^ 1) | 0x80) * 0x01010101;
    const uchar256 c4 = *((uchar256*)&c4u);
    const uchar256 c2 = *((uchar256*)&c2u);
    const uchar256 c1 = *((uchar256*)&c1u);

    bfloat128 buf[64];

    for (int row = start[0]; row < end[0]; ++row) {
        const int ci = row / C_chunk;
        const int r = row - ci * C_chunk;
        const int hb = ci * HV;
        bfloat128 m0 = 0, m1 = 0, m2 = 0, m3 = 0;

        for (int h = 0; h < HV; h += 4) {
            int5 c0 = {0, r, hb + h, 0, 0},     c1c = {0, r, hb + h + 1, 0, 0};
            int5 c2c = {0, r, hb + h + 2, 0, 0}, c3c = {0, r, hb + h + 3, 0, 0};
            int5 z0 = {h * 128, row, 0, 0, 0},       z1 = {(h + 1) * 128, row, 0, 0, 0};
            int5 z2 = {(h + 2) * 128, row, 0, 0, 0}, z3 = {(h + 3) * 128, row, 0, 0, 0};
            const float128 x0 = v_convert_bf16_to_f32_all_b(v_bf16_ld_tnsr_b(c0, out_c));
            const float128 x1 = v_convert_bf16_to_f32_all_b(v_bf16_ld_tnsr_b(c1c, out_c));
            const float128 x2 = v_convert_bf16_to_f32_all_b(v_bf16_ld_tnsr_b(c2c, out_c));
            const float128 x3 = v_convert_bf16_to_f32_all_b(v_bf16_ld_tnsr_b(c3c, out_c));
            const bfloat128 zb0 = v_bf16_ld_tnsr_b(z0, z);
            const bfloat128 zb1 = v_bf16_ld_tnsr_b(z1, z);
            const bfloat128 zb2 = v_bf16_ld_tnsr_b(z2, z);
            const bfloat128 zb3 = v_bf16_ld_tnsr_b(z3, z);

            float64 s0 = v_f32_mac_b(x0.v1, x0.v1, x0.v2 * x0.v2);
            float64 s1 = v_f32_mac_b(x1.v1, x1.v1, x1.v2 * x1.v2);
            float64 s2 = v_f32_mac_b(x2.v1, x2.v1, x2.v2 * x2.v2);
            float64 s3 = v_f32_mac_b(x3.v1, x3.v1, x3.v2 * x3.v2);
            SUM64_4(s0, s1, s2, s3);
            s0 = v_f32_mac_b(s0, 1.0f / 128.0f, eps);
            s1 = v_f32_mac_b(s1, 1.0f / 128.0f, eps);
            s2 = v_f32_mac_b(s2, 1.0f / 128.0f, eps);
            s3 = v_f32_mac_b(s3, 1.0f / 128.0f, eps);
            float64 inv0 = as_float64((int64)0x5f3759df - (as_int64(s0) >> 1));
            float64 inv1 = as_float64((int64)0x5f3759df - (as_int64(s1) >> 1));
            float64 inv2 = as_float64((int64)0x5f3759df - (as_int64(s2) >> 1));
            float64 inv3 = as_float64((int64)0x5f3759df - (as_int64(s3) >> 1));
            const float64 h0 = 0.5f * s0, h1 = 0.5f * s1, h2 = 0.5f * s2, h3 = 0.5f * s3;
            inv0 = inv0 * (1.5f - h0 * inv0 * inv0);
            inv1 = inv1 * (1.5f - h1 * inv1 * inv1);
            inv2 = inv2 * (1.5f - h2 * inv2 * inv2);
            inv3 = inv3 * (1.5f - h3 * inv3 * inv3);

            float128 nm0, nm1, nm2, nm3;
            nm0.v1 = x0.v1 * inv0 * wf.v1; nm0.v2 = x0.v2 * inv0 * wf.v2;
            nm1.v1 = x1.v1 * inv1 * wf.v1; nm1.v2 = x1.v2 * inv1 * wf.v2;
            nm2.v1 = x2.v1 * inv2 * wf.v1; nm2.v2 = x2.v2 * inv2 * wf.v2;
            nm3.v1 = x3.v1 * inv3 * wf.v1; nm3.v2 = x3.v2 * inv3 * wf.v2;

            bfloat128 sz0, sz1, sz2, sz3;
            SILU4_BF16(sz0, sz1, sz2, sz3, zb0, zb1, zb2, zb3);
            const bfloat128 g0 = v_convert_f32_to_bf16_all_b(nm0) * sz0;
            const bfloat128 g1 = v_convert_f32_to_bf16_all_b(nm1) * sz1;
            const bfloat128 g2 = v_convert_f32_to_bf16_all_b(nm2) * sz2;
            const bfloat128 g3 = v_convert_f32_to_bf16_all_b(nm3) * sz3;
            buf[h] = g0; buf[h + 1] = g1; buf[h + 2] = g2; buf[h + 3] = g3;
            m0 = v_bf16_max_b(m0, v_bf16_abs_b(g0));
            m1 = v_bf16_max_b(m1, v_bf16_abs_b(g1));
            m2 = v_bf16_max_b(m2, v_bf16_abs_b(g2));
            m3 = v_bf16_max_b(m3, v_bf16_abs_b(g3));
        }

        const float64_pair_t mf = v_convert_bf16_to_f32_all_b(v_bf16_max_b(v_bf16_max_b(m0, m1), v_bf16_max_b(m2, m3)));
        const float64 maximum = max64(v_f32_max_b(mf.v1, mf.v2), c4, c2, c1);
        const float64 scale = round_bf16(round_bf16(maximum + 1.0e-8f) * (float)(bf16)(1.0f / 240.0f));
        const float64 inverse = round_bf16(reciprocal_without_lookup(scale));
        float64_pair_t inv_pair;
        inv_pair.v1 = inverse;
        inv_pair.v2 = inverse;
        const bfloat128 inv_bf16 = v_convert_f32_to_bf16_all_b(inv_pair);
        int5 sc = {0, row, 0, 0, 0};
        v_f32_st_tnsr(sc, scales, scale);

        for (int h = 0; h < HV; h += 4) {
            const bfloat128 s0_ = buf[h]     * inv_bf16;
            const bfloat128 s1_ = buf[h + 1] * inv_bf16;
            const bfloat128 s2_ = buf[h + 2] * inv_bf16;
            const bfloat128 s3_ = buf[h + 3] * inv_bf16;
            const minifloat256 sp0_ = v_convert_bf16_to_f8_b(s0_, 0, SW_CLIP_FP, (minifloat256)0);
            const minifloat256 sp1_ = v_convert_bf16_to_f8_b(s1_, 0, SW_CLIP_FP, (minifloat256)0);
            const minifloat256 sp2_ = v_convert_bf16_to_f8_b(s2_, 0, SW_CLIP_FP, (minifloat256)0);
            const minifloat256 sp3_ = v_convert_bf16_to_f8_b(s3_, 0, SW_CLIP_FP, (minifloat256)0);
            minifloat256 p0_ = v_f8_pack_b(sp0_, SW_GROUP_0 | SW_STRIDE_2, (minifloat256)0);
            minifloat256 p1_ = v_f8_pack_b(sp1_, SW_GROUP_0 | SW_STRIDE_2, (minifloat256)0);
            minifloat256 p2_ = v_f8_pack_b(sp2_, SW_GROUP_0 | SW_STRIDE_2, (minifloat256)0);
            minifloat256 p3_ = v_f8_pack_b(sp3_, SW_GROUP_0 | SW_STRIDE_2, (minifloat256)0);
            p0_ = v_f8_pack_b(sp0_, SW_GROUP_1 | SW_STRIDE_2, p0_);
            p1_ = v_f8_pack_b(sp1_, SW_GROUP_1 | SW_STRIDE_2, p1_);
            p2_ = v_f8_pack_b(sp2_, SW_GROUP_1 | SW_STRIDE_2, p2_);
            p3_ = v_f8_pack_b(sp3_, SW_GROUP_1 | SW_STRIDE_2, p3_);
            p0_ = v_f8_mov_dual_group_pack_b(p0_, SW_PACK21, (minifloat256)0);
            p1_ = v_f8_mov_dual_group_pack_b(p1_, SW_PACK21, (minifloat256)0);
            p2_ = v_f8_mov_dual_group_pack_b(p2_, SW_PACK21, (minifloat256)0);
            p3_ = v_f8_mov_dual_group_pack_b(p3_, SW_PACK21, (minifloat256)0);
            p0_ = v_f8_mov_dual_group_b(p1_, 0xffffffff, 0, 2, MkWr(1, 1), p0_);
            p2_ = v_f8_mov_dual_group_b(p3_, 0xffffffff, 0, 2, MkWr(1, 1), p2_);
            p0_ = v_f8_mov_dual_group_b(p1_, 0xffffffff, 1, 3, MkWr(1, 1), p0_);
            p2_ = v_f8_mov_dual_group_b(p3_, 0xffffffff, 1, 3, MkWr(1, 1), p2_);
            int5 oc0 = {h * 128, row, 0, 0, 0};
            int5 oc2 = {(h + 2) * 128, row, 0, 0, 0};
            v_f8_st_tnsr(oc0, codes, p0_);
            v_f8_st_tnsr(oc2, codes, p2_);
        }
    }
}
