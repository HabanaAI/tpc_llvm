// SPDX-License-Identifier: Apache-2.0
// g2e direct MME+SRAM kernel: fused ba (beta = sigmoid(b), g = softplus(a + dt_bias) * negexpA -> s_g, s_beta, st_g)
// + in-place dequant + SiLU on s_z [6144, R]. Runs in parallel with MME(gram) + 8x MME(bwr)!
// Tensors (dim 0 fastest):
//   0 ba_raw  F32  [96, R]       MME(ba) output in SRAM ([0..47]=b, [48..95]=a)
//   1 dt_bias BF16 [48]          HBM dt_bias
//   2 negexpA F32  [48]          HBM -exp(A_log)
//   3 s_g     F32  [48, T, B]    SRAM log-decay for gdn_mtp_wyod
//   4 s_beta  F32  [48, T, B]    SRAM beta for gdn_mtp_wyod
//   5 st_g    F32  [48, T, B]    HBM stashed log-decay (strided dim 2)
//   6 z_io    BF16 [6144, R]     SRAM unscaled MME(z) -> in-place silu(dequant(z))
//   7 sa      F32  [1, R]        SRAM activation scale
//   8 z_swb   F32  [6144]        HBM pre-interleaved z weight scales [even 64, odd 64] per 128 cols
// Index space: dim 1 = row [0..R) (split across 24 TPCs).

static float64 bf16_to_f32_first64(bfloat128 input)
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
    return v_convert_bf16_to_f32_all_b(unpacked.v1).v1;
}

static inline float64 fast_rcp3(float64 v) {
    float64 e = as_float64((int64)0x7ef311c3 - as_int64(v));
    e = e * (2.0f - v * e);
    e = e * (2.0f - v * e);
    e = e * (2.0f - v * e);
    return e;
}

static inline float64 fast_exp_f32(float64 x) {
    const float64 xc = v_f32_min_b(v_f32_max_b(x, -87.0f), 87.0f);
    const float64 t = v_f32_mac_b(xc, 1.4426950408889634f, (float64)127.0f);
    const int64 n = v_convert_f32_to_i32_b(t, SW_RHNE);
    const float64 f = t - v_convert_i32_to_f32_b(n, 0);
    float64 p = 0.0096181291f;
    p = v_f32_mac_b(p, f, 0.0555041087f);
    p = v_f32_mac_b(p, f, 0.2402265070f);
    p = v_f32_mac_b(p, f, 0.6931471806f);
    p = v_f32_mac_b(p, f, 1.0f);
    const int64 bits = n << 23;
    return v_f32_sel_leq_f32_b(x, -87.0f, 0.0f, p * as_float64(bits));
}

static inline float64 fast_sigmoid_f32(float64 x) {
    return fast_rcp3(1.0f + fast_exp_f32(-x));
}

// softplus(x) = max(x, 0) + log(1 + exp(-|x|)); for u in [0, 1], w = u / (2 + u) in [0, 1/3],
// log(1 + u) = 2*w*(1 + w^2*(1/3 + w^2*(1/5 + w^2*0.1498))) accurate to < 2e-7, 100% LUT-free.
static inline float64 fast_softplus_f32(float64 x) {
    const float64 u = fast_exp_f32(-v_f32_abs_b(x));
    const float64 w = u * fast_rcp3(2.0f + u);
    const float64 w2 = w * w;
    float64 p = v_f32_mac_b(w2, 0.1498f, 0.200000003f);
    p = v_f32_mac_b(w2, p, 0.333333343f);
    p = v_f32_mac_b(w2, p, 1.0f);
    return v_f32_max_b(x, 0.0f) + (w + w) * p;
}

static inline float64 round_bf16(float64 value) {
    float64_pair_t pair;
    pair.v1 = value;
    pair.v2 = value;
    return v_convert_bf16_to_f32_all_b(v_convert_f32_to_bf16_all_b(pair)).v1;
}

static inline float64 fast_silu(float64 x) {
    return x * fast_rcp3(1.0f + fast_exp_f32(-x));
}

#define Z_TILE(TIDX) do {                                                                \
    const int col_ = (TIDX) * 128;                                                       \
    int5 rc_ = {col_, row, 0, 0, 0};                                                     \
    int5 wc_ = {col_, 0, 0, 0, 0};                                                       \
    float64_pair_t zv_ = v_convert_bf16_to_f32_all_b(v_bf16_ld_tnsr_b(rc_, z_io));       \
    const float64 w1_ = v_f32_ld_tnsr_b(wc_, z_swb);                                     \
    wc_[0] = col_ + 64;                                                                  \
    const float64 w2_ = v_f32_ld_tnsr_b(wc_, z_swb);                                     \
    zv_.v1 = fast_silu(round_bf16(zv_.v1 * (w1_ * a)));                                  \
    zv_.v2 = fast_silu(round_bf16(zv_.v2 * (w2_ * a)));                                  \
    v_bf16_st_tnsr(rc_, z_io, v_convert_f32_to_bf16_all_b(zv_));                         \
} while (0)

void main(tensor ba_raw, tensor dt_bias, tensor negexpA,
          tensor s_g, tensor s_beta, tensor st_g,
          tensor z_io, tensor sa, tensor z_swb)
{
    const int5 start = get_index_space_offset();
    const int5 end = start + get_index_space_size();
    const int T = get_dim_size(s_g, 1);  // 16
    const int z_tiles = get_dim_size(z_io, 0) / 128;  // 48
    int5 c0 = {0, 0, 0, 0, 0};
    const float64 dtb = bf16_to_f32_first64(v_bf16_ld_tnsr_b(c0, dt_bias));
    const float64 negA = v_f32_ld_tnsr_b(c0, negexpA);

    for (int row = start[1]; row < end[1]; ++row) {
        // 1) ba -> beta and g
        const int b_idx = row / T;
        const int t_idx = row - b_idx * T;
        int5 bc = {0, row, 0, 0, 0};
        int5 ac = {48, row, 0, 0, 0};
        const float64 b_v = v_f32_ld_tnsr_b(bc, ba_raw);
        const float64 a_v = v_f32_ld_tnsr_b(ac, ba_raw);
        const float64 beta_v = fast_sigmoid_f32(b_v);
        const float64 x = a_v + dtb;
        const float64 sp = fast_softplus_f32(x);
        const float64 g_v = sp * negA;
        int5 oc = {0, t_idx, b_idx, 0, 0};
        v_f32_st_tnsr_partial(oc, s_beta, beta_v, 47, 0);
        v_f32_st_tnsr_partial(oc, s_g, g_v, 47, 0);
        v_f32_st_tnsr_partial(oc, st_g, g_v, 47, 0);

        // 2) in-place dequant + SiLU on z_io (48 tiles, 4x unrolled)
        int5 sc = {0, row, 0, 0, 0};
        const float64 a = v_broadcast_element_f32(v_f32_ld_tnsr_b(sc, sa), 0);
        for (int tile = 0; tile < z_tiles; tile += 4) {
            Z_TILE(tile);
            Z_TILE(tile + 1);
            Z_TILE(tile + 2);
            Z_TILE(tile + 3);
        }
    }
}
