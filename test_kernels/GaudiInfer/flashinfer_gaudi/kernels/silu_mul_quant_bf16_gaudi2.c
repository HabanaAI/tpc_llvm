// SPDX-License-Identifier: Apache-2.0
// One index point owns a row. Intermediate products stay in VLM (64 KiB budget, 100% LUT-free).
// Supports 3 modes via input/silu dim 0 sizes (zero extra params):
//   1) silu_dim0 == 2 * width: fused LUT-free silu(input[0..F-1]) * input[F..2F-1] + quant (eliminates silu_fwd_bf16!)
//   2) input_dim0 == width   : direct silu[0..F-1] * input[0..F-1] + quant (eliminates concat({a, b}, 1) in mul_quant!)
//   3) input_dim0 == 2*width, silu_dim0 == width: legacy silu[0..F-1] * input[F..2F-1] + quant
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

static inline float64 fast_silu(float64 x) {
    const float64 xc = v_f32_min_b(v_f32_max_b(x, -87.0f), 87.0f);
    const float64 t = v_f32_mac_b(xc, -1.4426950408889634f, (float64)127.0f);
    const int64 n = v_convert_f32_to_i32_b(t, SW_RHNE);
    const float64 f = t - v_convert_i32_to_f32_b(n, 0);
    float64 p = 0.0096181291f;
    p = v_f32_mac_b(p, f, 0.0555041087f);
    p = v_f32_mac_b(p, f, 0.2402265070f);
    p = v_f32_mac_b(p, f, 0.6931471806f);
    p = v_f32_mac_b(p, f, 1.0f);
    const int64 bits = n << 23;
    const float64 d = 1.0f + p * as_float64(bits);
    float64 r = as_float64((int64)0x7ef311c3 - as_int64(d));
    r = r * (2.0f - d * r);
    r = r * (2.0f - d * r);
    r = r * (2.0f - d * r);
    return x * r;
}

static inline float64 row_max_without_lookup(float64 value) {
    value = v_f32_max_b(value, v_f32_mov_dual_group_all_b(value, 0xffffffff, 1, 0, 3, 2,
                                                      MkWrA(3, 3, 3, 3), 0));
    value = v_f32_max_b(value, v_f32_mov_dual_group_all_b(value, 0xffffffff, 2, 3, 0, 1,
                                                      MkWrA(3, 3, 3, 3), 0));
    value = v_f32_max_b(value, v_f32_mov_group_b(value, 0xffffffff, 63, 0));
    float64 result = 0;
    #pragma loop_unroll(8)
    for (int lane = 0; lane < 8; ++lane) {
        const uchar256 broadcast = 0x80 | lane;
        result = v_f32_max_b(result, v_f32_shuffle_b(value, broadcast, 0, value));
    }
    return result;
}

static inline bfloat128 multiply_pre(tensor input, tensor silu, int up_offset, int row, int tile) {
    int5 coord = {tile * 128, row, 0, 0, 0};
    const bfloat128 activated_gate = v_bf16_ld_tnsr_b(coord, silu);
    coord[0] += up_offset;
    const bfloat128 up = v_bf16_ld_tnsr_b(coord, input);
    return activated_gate * up;
}

static inline bfloat128 multiply_silu(tensor input, int width, int row, int tile) {
    int5 coord = {tile * 128, row, 0, 0, 0};
    float64_pair_t g = v_convert_bf16_to_f32_all_b(v_bf16_ld_tnsr_b(coord, input));
    coord[0] += width;
    const bfloat128 up = v_bf16_ld_tnsr_b(coord, input);
    g.v1 = fast_silu(g.v1);
    g.v2 = fast_silu(g.v2);
    return v_convert_f32_to_bf16_all_b(g) * up;
}

static inline minifloat256 bf16_128_to_f8_lo128(bfloat128 x) {
    const minifloat256 sp = v_convert_bf16_to_f8_b(x, 0, SW_CLIP_FP, (minifloat256)0);
    minifloat256 pk = v_f8_pack_b(sp, SW_GROUP_0 | SW_STRIDE_2, (minifloat256)0);
    pk = v_f8_pack_b(sp, SW_GROUP_1 | SW_STRIDE_2, pk);
    return v_f8_mov_dual_group_pack_b(pk, SW_PACK21, (minifloat256)0);
}

#define MUL_PRE_STEP(TIDX, MAXACC) do {                                            \
    const bfloat128 v_ = multiply_pre(input, silu, up_offset, row, (TIDX));        \
    activated[(TIDX)] = v_;                                                        \
    (MAXACC) = v_bf16_max_b((MAXACC), v_bf16_abs_b(v_));                           \
} while (0)

#define MUL_SILU_STEP(TIDX, MAXACC) do {                                           \
    const bfloat128 v_ = multiply_silu(input, width, row, (TIDX));                 \
    activated[(TIDX)] = v_;                                                        \
    (MAXACC) = v_bf16_max_b((MAXACC), v_bf16_abs_b(v_));                           \
} while (0)

#define QUANT_PAIR(TIDX) do {                                                      \
    minifloat256 pk0_ = bf16_128_to_f8_lo128(activated[(TIDX)] * inv_bf16);        \
    const minifloat256 pk1_ = bf16_128_to_f8_lo128(activated[(TIDX) + 1] * inv_bf16); \
    pk0_ = v_f8_mov_dual_group_b(pk1_, 0xffffffff, 0, 2, MkWr(1, 1), pk0_);        \
    pk0_ = v_f8_mov_dual_group_b(pk1_, 0xffffffff, 1, 3, MkWr(1, 1), pk0_);        \
    int5 oc_ = {(TIDX) * 128, row, 0, 0, 0};                                       \
    v_f8_st_tnsr(oc_, output, pk0_);                                               \
} while (0)

void main(tensor input, tensor silu, tensor output, tensor scales) {
    const int5 start = get_index_space_offset();
    const int5 end = start + get_index_space_size();
    const int width = get_dim_size(output, 0);
    const int up_offset = get_dim_size(input, 0) - width;
    const int do_silu = (get_dim_size(silu, 0) == 2 * width);
    const int tiles = width / 128;
    const int bulk = tiles & ~3;
    const int pairs = tiles & ~1;
    bfloat128 activated[136];
    for (int row = start[0]; row < end[0]; ++row) {
        bfloat128 m0 = 0, m1 = 0, m2 = 0, m3 = 0;
        if (do_silu) {
            for (int tile = 0; tile < bulk; tile += 4) {
                MUL_SILU_STEP(tile, m0);
                MUL_SILU_STEP(tile + 1, m1);
                MUL_SILU_STEP(tile + 2, m2);
                MUL_SILU_STEP(tile + 3, m3);
            }
            for (int tile = bulk; tile < tiles; ++tile) {
                MUL_SILU_STEP(tile, m0);
            }
        } else {
            for (int tile = 0; tile < bulk; tile += 4) {
                MUL_PRE_STEP(tile, m0);
                MUL_PRE_STEP(tile + 1, m1);
                MUL_PRE_STEP(tile + 2, m2);
                MUL_PRE_STEP(tile + 3, m3);
            }
            for (int tile = bulk; tile < tiles; ++tile) {
                MUL_PRE_STEP(tile, m0);
            }
        }
        const float64_pair_t mf = v_convert_bf16_to_f32_all_b(v_bf16_max_b(v_bf16_max_b(m0, m1), v_bf16_max_b(m2, m3)));
        const float64 maximum = row_max_without_lookup(v_f32_max_b(mf.v1, mf.v2));
        const float64 scale = round_bf16(round_bf16(maximum + 1.0e-8f) * (float)(bf16)(1.0f / 240.0f));
        const float64 inverse = round_bf16(reciprocal_without_lookup(scale));
        float64_pair_t inv_pair;
        inv_pair.v1 = inverse;
        inv_pair.v2 = inverse;
        const bfloat128 inv_bf16 = v_convert_f32_to_bf16_all_b(inv_pair);
        int5 scale_coord = {0, row, 0, 0, 0};
        v_f32_st_tnsr(scale_coord, scales, scale);
        for (int tile = 0; tile < bulk; tile += 4) {
            QUANT_PAIR(tile);
            QUANT_PAIR(tile + 2);
        }
        for (int tile = bulk; tile < pairs; tile += 2) {
            QUANT_PAIR(tile);
        }
        for (int tile = pairs; tile < tiles; ++tile) {
            const minifloat256 pk = bf16_128_to_f8_lo128(activated[tile] * inv_bf16);
            int5 coord = {tile * 128, row, 0, 0, 0};
            v_f8_st_tnsr_partial(coord, output, pk, 127, 0);
        }
    }
}
