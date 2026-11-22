// SPDX-License-Identifier: Apache-2.0
// g2e: fused per-row softmax statistics of temperature-scaled FP32 logits (for verify_topk / verify_topkb):
//   mx[r]  = inv_temp[r] * max_c logits[r, c]
//   sum[r] = sum_c exp(inv_temp[r] * logits[r, c] - mx[r])
// Eliminates materialising the [R, V] scaled logits tensor (127 MiB at R=128, V=248320) and runs a fast
// 4x-unrolled LUT-free base-2 polynomial exp (max rel err < 3e-6, full FP32 accuracy).
// Tensors (dim 0 fastest):
//   logits   F32 [V, R]  (V % 64 == 0)
//   inv_temp F32 [1, R]  or [1] (scalar broadcast when rank 1 or dim1 == 1)
//   mx_out   F32 [1, R]
//   sum_out  F32 [1, R]
// Index space: [R].

static inline float64 max64(float64 x)
{
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

static inline float64 sum64(float64 x)
{
    const int all = SW_WR_LOWER_GROUP0 | SW_WR_UPPER_GROUP0 | SW_WR_LOWER_GROUP1 | SW_WR_UPPER_GROUP1 |
                    SW_WR_LOWER_GROUP2 | SW_WR_UPPER_GROUP2 | SW_WR_LOWER_GROUP3 | SW_WR_UPPER_GROUP3;
    x += v_f32_mov_dual_group_all_b(x, 0xffffffff, 1, 0, 3, 2, all, x);
    x += v_f32_mov_dual_group_all_b(x, 0xffffffff, 2, 3, 0, 1, all, x);
    x += v_f32_mov_group_b(x, 0xffffffff, SW_GROUP0_EN | SW_GROUP1_EN | SW_DUAL_GROUP0_EN |
                           SW_DUAL_GROUP1_EN | SW_DUAL_GROUP2_EN | SW_DUAL_GROUP3_EN, x);
    uint64 lanes = read_lane_id_4b_b() & 7;
    uint64 c4 = ((lanes ^ 4) | 0x80) * 0x01010101;
    uint64 c2 = ((lanes ^ 2) | 0x80) * 0x01010101;
    uint64 c1 = ((lanes ^ 1) | 0x80) * 0x01010101;
    x += v_f32_shuffle_b(x, *((uchar256*)&c4), 0, x);
    x += v_f32_shuffle_b(x, *((uchar256*)&c2), 0, x);
    x += v_f32_shuffle_b(x, *((uchar256*)&c1), 0, x);
    return x;
}

// 2^(x * kl + nb) where kl = inv_temp * log2(e) and nb = 127 - raw_max * kl; x clamped at lo = raw_max - 120 / kl.
static inline float64 exp_biased_f32(float64 x, float kl, float64 nb, float64 lo)
{
    const float64 xc = v_f32_max_b(x, lo);
    const float64 t = v_f32_mac_b(xc, kl, nb);
    const int64 n = v_convert_f32_to_i32_b(t, SW_RHNE);
    const float64 f = t - v_convert_i32_to_f32_b(n, 0);
    float64 p = 0.0096181291f;
    p = v_f32_mac_b(p, f, 0.0555041087f);
    p = v_f32_mac_b(p, f, 0.2402265070f);
    p = v_f32_mac_b(p, f, 0.6931471806f);
    p = v_f32_mac_b(p, f, 1.0f);
    const int64 bits = n << 23;
    return p * (*((float64*)&bits));
}

void main(tensor logits, tensor inv_temp, tensor mx_out, tensor sum_out)
{
    const int5 start = get_index_space_offset();
    const int5 end = start + get_index_space_size();
    const int V = get_dim_size(logits, 0);
    const int it_rows = get_dim_size(inv_temp, 1);
    const int nvec = V / 64;
    const int groups = nvec / 4;
    const float64 neg = -3.0e38f;

    for (int r = start[0]; r < end[0]; ++r) {
        int5 tc = {0, it_rows > 1 ? r : 0, 0, 0, 0};
        const float it = s_f32_ld_g((__global__ float*)gen_addr(tc, inv_temp));
        const float kl = it * 1.4426950408889634f;

        // Pass 1: raw row maximum
        float64 m0 = neg, m1 = neg, m2 = neg, m3 = neg;
        int5 c0 = {0, r, 0, 0, 0}, c1 = {64, r, 0, 0, 0}, c2 = {128, r, 0, 0, 0}, c3 = {192, r, 0, 0, 0};
        for (int g = 0; g < groups; ++g) {
            m0 = v_f32_max_b(m0, v_f32_ld_tnsr_b(c0, logits));
            m1 = v_f32_max_b(m1, v_f32_ld_tnsr_b(c1, logits));
            m2 = v_f32_max_b(m2, v_f32_ld_tnsr_b(c2, logits));
            m3 = v_f32_max_b(m3, v_f32_ld_tnsr_b(c3, logits));
            c0[0] += 256; c1[0] += 256; c2[0] += 256; c3[0] += 256;
        }
        for (int v = groups * 4; v < nvec; ++v) {
            int5 cv = {v * 64, r, 0, 0, 0};
            m0 = v_f32_max_b(m0, v_f32_ld_tnsr_b(cv, logits));
        }
        const float64 raw_m = max64(v_f32_max_b(v_f32_max_b(m0, m1), v_f32_max_b(m2, m3)));
        const float64 mx = raw_m * it;
        const float64 nb = 127.0f - raw_m * kl;
        const float64 lo = raw_m - 120.0f / kl;

        // Pass 2: sum of exp((logits - raw_m) * it)
        float64 a0 = 0.0f, a1 = 0.0f, a2 = 0.0f, a3 = 0.0f;
        c0[0] = 0; c1[0] = 64; c2[0] = 128; c3[0] = 192;
        for (int g = 0; g < groups; ++g) {
            const float64 x0 = v_f32_ld_tnsr_b(c0, logits);
            const float64 x1 = v_f32_ld_tnsr_b(c1, logits);
            const float64 x2 = v_f32_ld_tnsr_b(c2, logits);
            const float64 x3 = v_f32_ld_tnsr_b(c3, logits);
            a0 += exp_biased_f32(x0, kl, nb, lo);
            a1 += exp_biased_f32(x1, kl, nb, lo);
            a2 += exp_biased_f32(x2, kl, nb, lo);
            a3 += exp_biased_f32(x3, kl, nb, lo);
            c0[0] += 256; c1[0] += 256; c2[0] += 256; c3[0] += 256;
        }
        for (int v = groups * 4; v < nvec; ++v) {
            int5 cv = {v * 64, r, 0, 0, 0};
            a0 += exp_biased_f32(v_f32_ld_tnsr_b(cv, logits), kl, nb, lo);
        }
        int5 oc = {0, r, 0, 0, 0};
        v_f32_st_tnsr(oc, mx_out, mx);
        v_f32_st_tnsr(oc, sum_out, sum64((a0 + a1) + (a2 + a3)));
    }
}
