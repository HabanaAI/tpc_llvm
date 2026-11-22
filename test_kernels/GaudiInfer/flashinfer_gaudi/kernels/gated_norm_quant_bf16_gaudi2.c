// SPDX-License-Identifier: Apache-2.0
// g2e: GDN output gate, stage 1 (index space [HV / 8 head groups, T rows]: 96 points at T = 16):
//   normed = rms_norm(core[h]) * w   (per value head, DV = 128 = one BF16 vector; BF16 like rms_norm_ex_fwd_bf16)
//   gated  = normed * silu(z[h])     (silu rounded to BF16 like silu_fwd_bf16, product rounded to BF16)
//   pmax[:, g, t] = max |gated| over the group's heads (all 64 lanes) (stage 2, row_quant_bf16_gaudi2, makes the row FP8 codes)
// Replaces rms_norm + silu + (concat) + the multiply in silu_mul_quant. LUT-free: exp is a base-2 cubic, 1/x and
// 1/sqrt(x) are two Newton steps from bit tricks (relative error ~2e-4, far below BF16 rounding).
// Tensors (dim 0 fastest): core [128, HV, T] bf16, z [128, HV, T] bf16, w [128] bf16, gated [128*HV, T] bf16,
// pmax [64, HV/8, T] f32. Scalar: eps.

#define HEADS_PER_GROUP 8

static inline float64 reciprocal_without_lookup(float64 value) {  // positive normal values
    float64 estimate = as_float64((int64)0x7ef311c3 - as_int64(value));
    estimate = estimate * (2.0f - value * estimate);
    estimate = estimate * (2.0f - value * estimate);
    return estimate;
}

static inline float64 rsqrt_without_lookup(float64 x) {  // positive normal values
    float64 y = as_float64((int64)0x5f3759df - (as_int64(x) >> 1));
    const float64 h = 0.5f * x;
    y = y * (1.5f - h * y * y);
    y = y * (1.5f - h * y * y);
    return y;
}

static inline float64 sum64(float64 x) {
    const int all = SW_WR_LOWER_GROUP0 | SW_WR_UPPER_GROUP0 | SW_WR_LOWER_GROUP1 | SW_WR_UPPER_GROUP1 |
                    SW_WR_LOWER_GROUP2 | SW_WR_UPPER_GROUP2 | SW_WR_LOWER_GROUP3 | SW_WR_UPPER_GROUP3;
    x += v_f32_mov_dual_group_all_b(x, 0xffffffff, 1, 0, 3, 2, all, x);
    x += v_f32_mov_dual_group_all_b(x, 0xffffffff, 2, 3, 0, 1, all, x);
    x += v_f32_mov_group_b(x, 0xffffffff, SW_GROUP0_EN | SW_GROUP1_EN | SW_DUAL_GROUP0_EN | SW_DUAL_GROUP1_EN |
                           SW_DUAL_GROUP2_EN | SW_DUAL_GROUP3_EN, x);
    uint64 lanes = read_lane_id_4b_b() & 7;
    uint64 c4 = ((lanes ^ 4) | 0x80) * 0x01010101;
    uint64 c2 = ((lanes ^ 2) | 0x80) * 0x01010101;
    uint64 c1 = ((lanes ^ 1) | 0x80) * 0x01010101;
    x += v_f32_shuffle_b(x, *((uchar256*)&c4), 0, x);
    x += v_f32_shuffle_b(x, *((uchar256*)&c2), 0, x);
    x += v_f32_shuffle_b(x, *((uchar256*)&c1), 0, x);
    return x;
}

static inline float64 max64(float64 x) {
    const int all = SW_WR_LOWER_GROUP0 | SW_WR_UPPER_GROUP0 | SW_WR_LOWER_GROUP1 | SW_WR_UPPER_GROUP1 |
                    SW_WR_LOWER_GROUP2 | SW_WR_UPPER_GROUP2 | SW_WR_LOWER_GROUP3 | SW_WR_UPPER_GROUP3;
    x = v_f32_max_b(x, v_f32_mov_dual_group_all_b(x, 0xffffffff, 1, 0, 3, 2, all, x));
    x = v_f32_max_b(x, v_f32_mov_dual_group_all_b(x, 0xffffffff, 2, 3, 0, 1, all, x));
    x = v_f32_max_b(x, v_f32_mov_group_b(x, 0xffffffff, SW_GROUP0_EN | SW_GROUP1_EN | SW_DUAL_GROUP0_EN |
                                         SW_DUAL_GROUP1_EN | SW_DUAL_GROUP2_EN | SW_DUAL_GROUP3_EN, x));
    uint64 lanes = read_lane_id_4b_b() & 7;
    uint64 c4 = ((lanes ^ 4) | 0x80) * 0x01010101;
    uint64 c2 = ((lanes ^ 2) | 0x80) * 0x01010101;
    uint64 c1 = ((lanes ^ 1) | 0x80) * 0x01010101;
    x = v_f32_max_b(x, v_f32_shuffle_b(x, *((uchar256*)&c4), 0, x));
    x = v_f32_max_b(x, v_f32_shuffle_b(x, *((uchar256*)&c2), 0, x));
    x = v_f32_max_b(x, v_f32_shuffle_b(x, *((uchar256*)&c1), 0, x));
    return x;
}

// silu(z) = z / (1 + 2^(-z log2 e)); z clamped to [-87, 87] for the exponent (silu ~ z resp. ~ 0 beyond)
static inline float64 silu(float64 z) {
    const float64 zc = v_f32_min_b(v_f32_max_b(z, -87.0f), 87.0f);
    const float64 t = v_f32_mac_b(zc, -1.4426950408889634f, (float64)127.0f);  // 2^(-z log2 e) with the bias
    const int64 n = v_convert_f32_to_i32_b(t, SW_RHNE);
    const float64 f = t - v_convert_i32_to_f32_b(n, 0);
    float64 p = 0.05517604947090149f;
    p = v_f32_mac_b(p, f, 0.24261149764060974f);
    p = v_f32_mac_b(p, f, 0.6932601928710938f);
    p = v_f32_mac_b(p, f, 0.9999280571937561f);
    const int64 bits = n << 23;
    const float64 e = p * *((float64*)&bits);
    return z * reciprocal_without_lookup(1.0f + e);
}

void main(tensor core, tensor z, tensor w, tensor gated, tensor pmax, float eps) {
    const int5 start = get_index_space_offset();
    const int5 end = start + get_index_space_size();
    int5 wc = {0, 0, 0, 0, 0};
    const float128 wf = v_convert_bf16_to_f32_all_b(v_bf16_ld_tnsr_b(wc, w));
    for (int row = start[1]; row < end[1]; ++row)
        for (int grp = start[0]; grp < end[0]; ++grp) {
            bfloat128 mb = 0;
            #pragma loop_unroll(4)
            for (int i = 0; i < HEADS_PER_GROUP; ++i) {
                const int h = grp * HEADS_PER_GROUP + i;
                int5 c = {0, h, row, 0, 0};
                const float128 x = v_convert_bf16_to_f32_all_b(v_bf16_ld_tnsr_b(c, core));
                const float128 zf = v_convert_bf16_to_f32_all_b(v_bf16_ld_tnsr_b(c, z));
                const float64 ss = sum64(v_f32_mac_b(x.v1, x.v1, x.v2 * x.v2));
                const float64 inv = rsqrt_without_lookup(ss * (1.0f / 128.0f) + eps);
                float128 nm, sl;
                nm.v1 = x.v1 * inv * wf.v1;
                nm.v2 = x.v2 * inv * wf.v2;
                sl.v1 = silu(zf.v1);
                sl.v2 = silu(zf.v2);
                const bfloat128 g = v_convert_f32_to_bf16_all_b(nm) * v_convert_f32_to_bf16_all_b(sl);
                int5 oc = {h * 128, row, 0, 0, 0};
                v_bf16_st_tnsr(oc, gated, g);
                mb = v_bf16_max_b(mb, v_bf16_abs_b(g));
            }
            const float128 gf = v_convert_bf16_to_f32_all_b(mb);
            const float64 maximum = v_f32_max_b(gf.v1, gf.v2);
            int5 pc = {0, grp, row, 0, 0};  // the group maximum in all 64 lanes
            v_f32_st_tnsr(pc, pmax, max64(maximum));
        }
}
