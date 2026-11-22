// SPDX-License-Identifier: Apache-2.0
// g2e direct MME+SRAM kernel: 4-way VPU-interleaved silu_gate * dequant(up) + per-row FP8 quant.
// LUT-free -> 64 KiB VLM budget -> caches all 136 BF16 tiles in VLM, zero 4-cycle VPU RAW stalls!
// Tensors (dim 0 fastest):
//   0 sgate  BF16 [F, R]  silu(dequant(gate)) in SRAM (already computed in parallel with MME(up))
//   1 up     BF16 [F, R]  unscaled up MME output in SRAM
//   2 sa     F32  [1, R]  row activation scale in SRAM
//   3 swb    F32  [F]     pre-interleaved up weight column scales [even 64, odd 64] per 128 cols
//   4 output F8   [F, R]  quantized FP8 E4M3 codes in SRAM
//   5 scales F32  [1, R]  row FP8 scales in SRAM
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

static inline minifloat256 bf16_128_to_f8_lo128(bfloat128 x) {
    const minifloat256 sp = v_convert_bf16_to_f8_b(x, 0, SW_CLIP_FP, (minifloat256)0);
    minifloat256 pk = v_f8_pack_b(sp, SW_GROUP_0 | SW_STRIDE_2, (minifloat256)0);
    pk = v_f8_pack_b(sp, SW_GROUP_1 | SW_STRIDE_2, pk);
    return v_f8_mov_dual_group_pack_b(pk, SW_PACK21, (minifloat256)0);
}

void main(tensor sgate, tensor up, tensor sa, tensor swb, tensor output, tensor scales) {
    const int5 start = get_index_space_offset();
    const int5 end = start + get_index_space_size();
    const int width = get_dim_size(output, 0);
    const int tiles = width / 128;
    const int bulk = tiles & ~3;
    bfloat128 activated[136];

    for (int row = start[1]; row < end[1]; ++row) {
        int5 sc = {0, row, 0, 0, 0};
        const float64 a = v_broadcast_element_f32(v_f32_ld_tnsr_b(sc, sa), 0);
        bfloat128 m0 = 0, m1 = 0, m2 = 0, m3 = 0;

        for (int tile = 0; tile < bulk; tile += 4) {
            const int c0 = tile * 128, c1 = c0 + 128, c2 = c0 + 256, c3 = c0 + 384;
            int5 rc0 = {c0, row, 0, 0, 0}, rc1 = {c1, row, 0, 0, 0}, rc2 = {c2, row, 0, 0, 0}, rc3 = {c3, row, 0, 0, 0};
            int5 wc0a = {c0, 0, 0, 0, 0}, wc0b = {c0 + 64, 0, 0, 0, 0};
            int5 wc1a = {c1, 0, 0, 0, 0}, wc1b = {c1 + 64, 0, 0, 0, 0};
            int5 wc2a = {c2, 0, 0, 0, 0}, wc2b = {c2 + 64, 0, 0, 0, 0};
            int5 wc3a = {c3, 0, 0, 0, 0}, wc3b = {c3 + 64, 0, 0, 0, 0};

            const float64_pair_t u0 = v_convert_bf16_to_f32_all_b(v_bf16_ld_tnsr_b(rc0, up));
            const float64_pair_t u1 = v_convert_bf16_to_f32_all_b(v_bf16_ld_tnsr_b(rc1, up));
            const float64_pair_t u2 = v_convert_bf16_to_f32_all_b(v_bf16_ld_tnsr_b(rc2, up));
            const float64_pair_t u3 = v_convert_bf16_to_f32_all_b(v_bf16_ld_tnsr_b(rc3, up));

            const float64 k0a = v_f32_ld_tnsr_b(wc0a, swb) * a, k0b = v_f32_ld_tnsr_b(wc0b, swb) * a;
            const float64 k1a = v_f32_ld_tnsr_b(wc1a, swb) * a, k1b = v_f32_ld_tnsr_b(wc1b, swb) * a;
            const float64 k2a = v_f32_ld_tnsr_b(wc2a, swb) * a, k2b = v_f32_ld_tnsr_b(wc2b, swb) * a;
            const float64 k3a = v_f32_ld_tnsr_b(wc3a, swb) * a, k3b = v_f32_ld_tnsr_b(wc3b, swb) * a;

            float64_pair_t d0 = {u0.v1 * k0a, u0.v2 * k0b};
            float64_pair_t d1 = {u1.v1 * k1a, u1.v2 * k1b};
            float64_pair_t d2 = {u2.v1 * k2a, u2.v2 * k2b};
            float64_pair_t d3 = {u3.v1 * k3a, u3.v2 * k3b};

            const bfloat128 g0 = v_bf16_ld_tnsr_b(rc0, sgate);
            const bfloat128 g1 = v_bf16_ld_tnsr_b(rc1, sgate);
            const bfloat128 g2 = v_bf16_ld_tnsr_b(rc2, sgate);
            const bfloat128 g3 = v_bf16_ld_tnsr_b(rc3, sgate);

            const bfloat128 p0 = g0 * v_convert_f32_to_bf16_all_b(d0);
            const bfloat128 p1 = g1 * v_convert_f32_to_bf16_all_b(d1);
            const bfloat128 p2 = g2 * v_convert_f32_to_bf16_all_b(d2);
            const bfloat128 p3 = g3 * v_convert_f32_to_bf16_all_b(d3);

            activated[tile]     = p0;
            activated[tile + 1] = p1;
            activated[tile + 2] = p2;
            activated[tile + 3] = p3;

            m0 = v_bf16_max_b(m0, v_bf16_abs_b(p0));
            m1 = v_bf16_max_b(m1, v_bf16_abs_b(p1));
            m2 = v_bf16_max_b(m2, v_bf16_abs_b(p2));
            m3 = v_bf16_max_b(m3, v_bf16_abs_b(p3));
        }
        for (int tile = bulk; tile < tiles; ++tile) {
            const int col = tile * 128;
            int5 rc = {col, row, 0, 0, 0}, wca = {col, 0, 0, 0, 0}, wcb = {col + 64, 0, 0, 0, 0};
            const float64_pair_t u = v_convert_bf16_to_f32_all_b(v_bf16_ld_tnsr_b(rc, up));
            float64_pair_t d = {u.v1 * (v_f32_ld_tnsr_b(wca, swb) * a), u.v2 * (v_f32_ld_tnsr_b(wcb, swb) * a)};
            const bfloat128 p = v_bf16_ld_tnsr_b(rc, sgate) * v_convert_f32_to_bf16_all_b(d);
            activated[tile] = p;
            m0 = v_bf16_max_b(m0, v_bf16_abs_b(p));
        }

        const float64_pair_t mf = v_convert_bf16_to_f32_all_b(v_bf16_max_b(v_bf16_max_b(m0, m1), v_bf16_max_b(m2, m3)));
        const float64 maximum = row_max_without_lookup(v_f32_max_b(mf.v1, mf.v2));
        const float64 scale = round_bf16(round_bf16(maximum + 1.0e-8f) * (float)(bf16)(1.0f / 240.0f));
        const float64 inverse = round_bf16(reciprocal_without_lookup(scale));
        float64_pair_t inv_pair = {inverse, inverse};
        const bfloat128 inv_bf = v_convert_f32_to_bf16_all_b(inv_pair);
        v_f32_st_tnsr(sc, scales, scale);

        for (int tile = 0; tile < bulk; tile += 4) {
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
        for (int tile = bulk; tile < tiles; ++tile) {
            const minifloat256 pk = bf16_128_to_f8_lo128(activated[tile] * inv_bf);
            int5 oc = {tile * 128, row, 0, 0, 0};
            v_f8_st_tnsr_partial(oc, output, pk, 127, 0);
        }
    }
}
