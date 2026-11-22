// SPDX-License-Identifier: Apache-2.0
// g2e: GDN output gate, stage 2 (index space [T rows]): per-row FP8 quantisation of the gated values with the
// row maximum assembled from stage 1's group maxima (gated_norm_quant_bf16_gaudi2).
// 4x unrolled with BF16 direct scale + 256-byte full FP8 stores.
// Tensors (dim 0 fastest): gated [W, T] bf16 (W % 128 == 0), pmax [64, NG, T] f32, codes [W, T] f8, scale [1, T] f32.

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

static inline minifloat256 bf16_128_to_f8_lo128(bfloat128 x) {
    const minifloat256 sp = v_convert_bf16_to_f8_b(x, 0, SW_CLIP_FP, (minifloat256)0);
    minifloat256 pk = v_f8_pack_b(sp, SW_GROUP_0 | SW_STRIDE_2, (minifloat256)0);
    pk = v_f8_pack_b(sp, SW_GROUP_1 | SW_STRIDE_2, pk);
    return v_f8_mov_dual_group_pack_b(pk, SW_PACK21, (minifloat256)0);
}

#define RQ_PAIR(TIDX) do {                                                          \
    int5 c0_ = {(TIDX) * 128, row, 0, 0, 0};                                        \
    int5 c1_ = {((TIDX) + 1) * 128, row, 0, 0, 0};                                  \
    minifloat256 pk0_ = bf16_128_to_f8_lo128(v_bf16_ld_tnsr_b(c0_, gated) * inv_bf16); \
    const minifloat256 pk1_ = bf16_128_to_f8_lo128(v_bf16_ld_tnsr_b(c1_, gated) * inv_bf16); \
    pk0_ = v_f8_mov_dual_group_b(pk1_, 0xffffffff, 0, 2, MkWr(1, 1), pk0_);         \
    pk0_ = v_f8_mov_dual_group_b(pk1_, 0xffffffff, 1, 3, MkWr(1, 1), pk0_);         \
    v_f8_st_tnsr(c0_, codes, pk0_);                                                 \
} while (0)

void main(tensor gated, tensor pmax, tensor codes, tensor scales) {
    const int5 start = get_index_space_offset();
    const int5 end = start + get_index_space_size();
    const int tiles = get_dim_size(gated, 0) / 128;
    const int bulk = tiles & ~3;
    const int pairs = tiles & ~1;
    const int NG = get_dim_size(pmax, 1);
    for (int row = start[0]; row < end[0]; ++row) {
        float64 maximum = 0;
        for (int g = 0; g < NG; ++g) {
            int5 pc = {0, g, row, 0, 0};
            maximum = v_f32_max_b(maximum, v_f32_ld_tnsr_b(pc, pmax));
        }
        const float64 scale = round_bf16(round_bf16(maximum + 1.0e-8f) * (float)(bf16)(1.0f / 240.0f));
        const float64 inverse = round_bf16(reciprocal_without_lookup(scale));
        float64_pair_t inv_pair;
        inv_pair.v1 = inverse;
        inv_pair.v2 = inverse;
        const bfloat128 inv_bf16 = v_convert_f32_to_bf16_all_b(inv_pair);
        int5 scale_coord = {0, row, 0, 0, 0};
        v_f32_st_tnsr(scale_coord, scales, scale);
        for (int tile = 0; tile < bulk; tile += 4) {
            RQ_PAIR(tile);
            RQ_PAIR(tile + 2);
        }
        for (int tile = bulk; tile < pairs; tile += 2) {
            RQ_PAIR(tile);
        }
        for (int tile = pairs; tile < tiles; ++tile) {
            int5 coord = {tile * 128, row, 0, 0, 0};
            const minifloat256 pk = bf16_128_to_f8_lo128(v_bf16_ld_tnsr_b(coord, gated) * inv_bf16);
            v_f8_st_tnsr_partial(coord, codes, pk, 127, 0);
        }
    }
}
