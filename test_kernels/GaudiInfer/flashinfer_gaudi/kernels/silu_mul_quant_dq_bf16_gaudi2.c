// SPDX-License-Identifier: Apache-2.0
// silu_mul_quant with the FP8 GEMM's dequantisation folded in:
//   act = silu(raw_g * (swb_g * a)) * (raw_u * (swb_u * a))
// 100% 128-lane BF16 inner loop, 4-tile fully interleaved DQ_QUAD + QUANT_QUAD
// (eliminates all 4-cycle VPU RAW pipeline stalls on Gaudi2 TPC!),
// 100% LUT-free (64 KiB VLM caches all 136 tiles), 256-byte full FP8 stores.
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

static inline bfloat128 fast_silu_bf16(bfloat128 x) {
    const bfloat128 xc = v_bf16_min_b(v_bf16_max_b(x, (bf16)(-80.0f)), (bf16)80.0f);
    const bfloat128 t = xc * (bf16)(-1.44269504f);
    const short128 n = v_convert_bf16_to_i16_b(t, SW_RHNE);
    const bfloat128 f = t - v_convert_i16_to_bf16_b(n, 0);
    bfloat128 p = v_bf16_mac_b((bfloat128)(bf16)0.2402265f, f, (bfloat128)(bf16)0.6931472f);
    p = v_bf16_mac_b(p, f, (bfloat128)(bf16)1.0f);
    const short128 bits = (n + (short128)127) << 7;
    const bfloat128 d = v_bf16_mac_b(p, *((bfloat128*)&bits), (bfloat128)(bf16)1.0f);
    const short128 ri = (short128)0x7ef3 - *((short128*)&d);
    bfloat128 r = *((bfloat128*)&ri);
    r = r * ((bfloat128)(bf16)2.0f - d * r);
    r = r * ((bfloat128)(bf16)2.0f - d * r);
    return x * r;
}

static inline float64 row_max_without_lookup(float64 value) {
    value = v_f32_max_b(value, v_f32_mov_dual_group_all_b(value, 0xffffffff, 1, 0, 3, 2,
                                                      MkWrA(3, 3, 3, 3), 0));
    value = v_f32_max_b(value, v_f32_mov_dual_group_all_b(value, 0xffffffff, 2, 3, 0, 1,
                                                      MkWrA(3, 3, 3, 3), 0));
    value = v_f32_max_b(value, v_f32_mov_group_b(value, 0xffffffff, 63, 0));
    uint64 lanes = read_lane_id_4b_b() & 7;
    uint64 c4 = ((lanes ^ 4) | 0x80) * 0x01010101;
    uint64 c2 = ((lanes ^ 2) | 0x80) * 0x01010101;
    uint64 c1 = ((lanes ^ 1) | 0x80) * 0x01010101;
    value = v_f32_max_b(value, v_f32_shuffle_b(value, *((uchar256*)&c4), 0, value));
    value = v_f32_max_b(value, v_f32_shuffle_b(value, *((uchar256*)&c2), 0, value));
    value = v_f32_max_b(value, v_f32_shuffle_b(value, *((uchar256*)&c1), 0, value));
    return value;
}

static inline minifloat256 bf16_128_to_f8_lo128(bfloat128 x) {
    const minifloat256 sp = v_convert_bf16_to_f8_b(x, 0, SW_CLIP_FP, (minifloat256)0);
    minifloat256 pk = v_f8_pack_b(sp, SW_GROUP_0 | SW_STRIDE_2, (minifloat256)0);
    pk = v_f8_pack_b(sp, SW_GROUP_1 | SW_STRIDE_2, pk);
    return v_f8_mov_dual_group_pack_b(pk, SW_PACK21, (minifloat256)0);
}

// Fully 4-way interleaved dequant + 128-lane BF16 SiLU + mul + absmax (0 RAW stalls on 4-cycle VPU!)
#define DQ_QUAD(TIDX, M0, M1, M2, M3) do {                                                      \
    const int c0_ = (TIDX) * 128, c1_ = ((TIDX) + 1) * 128;                                     \
    const int c2_ = ((TIDX) + 2) * 128, c3_ = ((TIDX) + 3) * 128;                               \
    int5 g0c_ = {c0_, row, 0, 0, 0}, g1c_ = {c1_, row, 0, 0, 0};                                \
    int5 g2c_ = {c2_, row, 0, 0, 0}, g3c_ = {c3_, row, 0, 0, 0};                                \
    int5 wg0c_ = {c0_, 0, 0, 0, 0}, wg1c_ = {c1_, 0, 0, 0, 0};                                  \
    int5 wg2c_ = {c2_, 0, 0, 0, 0}, wg3c_ = {c3_, 0, 0, 0, 0};                                  \
    int5 u0c_ = {c0_ + width, row, 0, 0, 0}, u1c_ = {c1_ + width, row, 0, 0, 0};                \
    int5 u2c_ = {c2_ + width, row, 0, 0, 0}, u3c_ = {c3_ + width, row, 0, 0, 0};                \
    int5 wu0c_ = {c0_ + width, 0, 0, 0, 0}, wu1c_ = {c1_ + width, 0, 0, 0, 0};                  \
    int5 wu2c_ = {c2_ + width, 0, 0, 0, 0}, wu3c_ = {c3_ + width, 0, 0, 0, 0};                  \
    const bfloat128 rg0_ = v_bf16_ld_tnsr_b(g0c_, raw);                                         \
    const bfloat128 rg1_ = v_bf16_ld_tnsr_b(g1c_, raw);                                         \
    const bfloat128 rg2_ = v_bf16_ld_tnsr_b(g2c_, raw);                                         \
    const bfloat128 rg3_ = v_bf16_ld_tnsr_b(g3c_, raw);                                         \
    const bfloat128 wg0_ = v_bf16_ld_tnsr_b(wg0c_, swb);                                        \
    const bfloat128 wg1_ = v_bf16_ld_tnsr_b(wg1c_, swb);                                        \
    const bfloat128 wg2_ = v_bf16_ld_tnsr_b(wg2c_, swb);                                        \
    const bfloat128 wg3_ = v_bf16_ld_tnsr_b(wg3c_, swb);                                        \
    const bfloat128 x0_ = rg0_ * (wg0_ * ab);                                                   \
    const bfloat128 x1_ = rg1_ * (wg1_ * ab);                                                   \
    const bfloat128 x2_ = rg2_ * (wg2_ * ab);                                                   \
    const bfloat128 x3_ = rg3_ * (wg3_ * ab);                                                   \
    const bfloat128 xc0_ = v_bf16_min_b(v_bf16_max_b(x0_, (bf16)(-80.0f)), (bf16)80.0f);        \
    const bfloat128 xc1_ = v_bf16_min_b(v_bf16_max_b(x1_, (bf16)(-80.0f)), (bf16)80.0f);        \
    const bfloat128 xc2_ = v_bf16_min_b(v_bf16_max_b(x2_, (bf16)(-80.0f)), (bf16)80.0f);        \
    const bfloat128 xc3_ = v_bf16_min_b(v_bf16_max_b(x3_, (bf16)(-80.0f)), (bf16)80.0f);        \
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
    const bfloat128 ru0_ = v_bf16_ld_tnsr_b(u0c_, raw);                                         \
    const bfloat128 ru1_ = v_bf16_ld_tnsr_b(u1c_, raw);                                         \
    const bfloat128 ru2_ = v_bf16_ld_tnsr_b(u2c_, raw);                                         \
    const bfloat128 ru3_ = v_bf16_ld_tnsr_b(u3c_, raw);                                         \
    const bfloat128 wu0_ = v_bf16_ld_tnsr_b(wu0c_, swb);                                        \
    const bfloat128 wu1_ = v_bf16_ld_tnsr_b(wu1c_, swb);                                        \
    const bfloat128 wu2_ = v_bf16_ld_tnsr_b(wu2c_, swb);                                        \
    const bfloat128 wu3_ = v_bf16_ld_tnsr_b(wu3c_, swb);                                        \
    r0_ = r0_ * ((bfloat128)(bf16)2.0f - d0_ * r0_);                                            \
    r1_ = r1_ * ((bfloat128)(bf16)2.0f - d1_ * r1_);                                            \
    r2_ = r2_ * ((bfloat128)(bf16)2.0f - d2_ * r2_);                                            \
    r3_ = r3_ * ((bfloat128)(bf16)2.0f - d3_ * r3_);                                            \
    r0_ = r0_ * ((bfloat128)(bf16)2.0f - d0_ * r0_);                                            \
    r1_ = r1_ * ((bfloat128)(bf16)2.0f - d1_ * r1_);                                            \
    r2_ = r2_ * ((bfloat128)(bf16)2.0f - d2_ * r2_);                                            \
    r3_ = r3_ * ((bfloat128)(bf16)2.0f - d3_ * r3_);                                            \
    const bfloat128 u0_ = ru0_ * (wu0_ * ab);                                                   \
    const bfloat128 u1_ = ru1_ * (wu1_ * ab);                                                   \
    const bfloat128 u2_ = ru2_ * (wu2_ * ab);                                                   \
    const bfloat128 u3_ = ru3_ * (wu3_ * ab);                                                   \
    const bfloat128 v0_ = (x0_ * r0_) * u0_;                                                    \
    const bfloat128 v1_ = (x1_ * r1_) * u1_;                                                    \
    const bfloat128 v2_ = (x2_ * r2_) * u2_;                                                    \
    const bfloat128 v3_ = (x3_ * r3_) * u3_;                                                    \
    activated[(TIDX)]     = v0_;                                                                \
    activated[(TIDX) + 1] = v1_;                                                                \
    activated[(TIDX) + 2] = v2_;                                                                \
    activated[(TIDX) + 3] = v3_;                                                                \
    (M0) = v_bf16_max_b((M0), v_bf16_abs_b(v0_));                                               \
    (M1) = v_bf16_max_b((M1), v_bf16_abs_b(v1_));                                               \
    (M2) = v_bf16_max_b((M2), v_bf16_abs_b(v2_));                                               \
    (M3) = v_bf16_max_b((M3), v_bf16_abs_b(v3_));                                               \
} while (0)

#define QUANT_QUAD(TIDX) do {                                                                   \
    const bfloat128 s0_ = activated[(TIDX)]     * inv_bf16;                                     \
    const bfloat128 s1_ = activated[(TIDX) + 1] * inv_bf16;                                     \
    const bfloat128 s2_ = activated[(TIDX) + 2] * inv_bf16;                                     \
    const bfloat128 s3_ = activated[(TIDX) + 3] * inv_bf16;                                     \
    const minifloat256 sp0_ = v_convert_bf16_to_f8_b(s0_, 0, SW_CLIP_FP, (minifloat256)0);      \
    const minifloat256 sp1_ = v_convert_bf16_to_f8_b(s1_, 0, SW_CLIP_FP, (minifloat256)0);      \
    const minifloat256 sp2_ = v_convert_bf16_to_f8_b(s2_, 0, SW_CLIP_FP, (minifloat256)0);      \
    const minifloat256 sp3_ = v_convert_bf16_to_f8_b(s3_, 0, SW_CLIP_FP, (minifloat256)0);      \
    minifloat256 p0_ = v_f8_pack_b(sp0_, SW_GROUP_0 | SW_STRIDE_2, (minifloat256)0);            \
    minifloat256 p1_ = v_f8_pack_b(sp1_, SW_GROUP_0 | SW_STRIDE_2, (minifloat256)0);            \
    minifloat256 p2_ = v_f8_pack_b(sp2_, SW_GROUP_0 | SW_STRIDE_2, (minifloat256)0);            \
    minifloat256 p3_ = v_f8_pack_b(sp3_, SW_GROUP_0 | SW_STRIDE_2, (minifloat256)0);            \
    p0_ = v_f8_pack_b(sp0_, SW_GROUP_1 | SW_STRIDE_2, p0_);                                     \
    p1_ = v_f8_pack_b(sp1_, SW_GROUP_1 | SW_STRIDE_2, p1_);                                     \
    p2_ = v_f8_pack_b(sp2_, SW_GROUP_1 | SW_STRIDE_2, p2_);                                     \
    p3_ = v_f8_pack_b(sp3_, SW_GROUP_1 | SW_STRIDE_2, p3_);                                     \
    p0_ = v_f8_mov_dual_group_pack_b(p0_, SW_PACK21, (minifloat256)0);                          \
    p1_ = v_f8_mov_dual_group_pack_b(p1_, SW_PACK21, (minifloat256)0);                          \
    p2_ = v_f8_mov_dual_group_pack_b(p2_, SW_PACK21, (minifloat256)0);                          \
    p3_ = v_f8_mov_dual_group_pack_b(p3_, SW_PACK21, (minifloat256)0);                          \
    p0_ = v_f8_mov_dual_group_b(p1_, 0xffffffff, 0, 2, MkWr(1, 1), p0_);                        \
    p2_ = v_f8_mov_dual_group_b(p3_, 0xffffffff, 0, 2, MkWr(1, 1), p2_);                        \
    p0_ = v_f8_mov_dual_group_b(p1_, 0xffffffff, 1, 3, MkWr(1, 1), p0_);                        \
    p2_ = v_f8_mov_dual_group_b(p3_, 0xffffffff, 1, 3, MkWr(1, 1), p2_);                        \
    int5 oc0_ = {(TIDX) * 128, row, 0, 0, 0};                                                   \
    int5 oc2_ = {((TIDX) + 2) * 128, row, 0, 0, 0};                                             \
    v_f8_st_tnsr(oc0_, output, p0_);                                                            \
    v_f8_st_tnsr(oc2_, output, p2_);                                                            \
} while (0)

#define QUANT_PAIR(TIDX) do {                                                      \
    minifloat256 pk0_ = bf16_128_to_f8_lo128(activated[(TIDX)] * inv_bf16);        \
    const minifloat256 pk1_ = bf16_128_to_f8_lo128(activated[(TIDX) + 1] * inv_bf16); \
    pk0_ = v_f8_mov_dual_group_b(pk1_, 0xffffffff, 0, 2, MkWr(1, 1), pk0_);        \
    pk0_ = v_f8_mov_dual_group_b(pk1_, 0xffffffff, 1, 3, MkWr(1, 1), pk0_);        \
    int5 oc_ = {(TIDX) * 128, row, 0, 0, 0};                                       \
    v_f8_st_tnsr(oc_, output, pk0_);                                               \
} while (0)

void main(tensor raw, tensor sa, tensor swb, tensor output, tensor scales)
{
    const int5 start = get_index_space_offset();
    const int5 end = start + get_index_space_size();
    const int width = get_dim_size(output, 0);
    const int tiles = width / 128;
    const int bulk = tiles & ~3;
    const int pairs = tiles & ~1;
    bfloat128 activated[136];
    for (int row = start[0]; row < end[0]; ++row) {
        int5 sc = {0, row, 0, 0, 0};
        const float64 a = v_broadcast_element_f32(v_f32_ld_tnsr_b(sc, sa), 0);
        float64_pair_t ap;
        ap.v1 = a; ap.v2 = a;
        const bfloat128 ab = v_convert_f32_to_bf16_all_b(ap);
        bfloat128 m0 = 0, m1 = 0, m2 = 0, m3 = 0;
        for (int tile = 0; tile < bulk; tile += 4) {
            DQ_QUAD(tile, m0, m1, m2, m3);
        }
        for (int tile = bulk; tile < tiles; ++tile) {
            int5 coord = {tile * 128, row, 0, 0, 0};
            int5 wc = {tile * 128, 0, 0, 0, 0};
            const bfloat128 rg = v_bf16_ld_tnsr_b(coord, raw);
            const bfloat128 wg = v_bf16_ld_tnsr_b(wc, swb);
            coord[0] += width; wc[0] += width;
            const bfloat128 ru = v_bf16_ld_tnsr_b(coord, raw);
            const bfloat128 wu = v_bf16_ld_tnsr_b(wc, swb);
            const bfloat128 v_ = fast_silu_bf16(rg * (wg * ab)) * (ru * (wu * ab));
            activated[tile] = v_;
            m0 = v_bf16_max_b(m0, v_bf16_abs_b(v_));
        }
        const float64_pair_t mf = v_convert_bf16_to_f32_all_b(v_bf16_max_b(v_bf16_max_b(m0, m1), v_bf16_max_b(m2, m3)));
        const float64 maximum = row_max_without_lookup(v_f32_max_b(mf.v1, mf.v2));
        const float64 scale = round_bf16(round_bf16(maximum + 1.0e-8f) * (float)(bf16)(1.0f / 240.0f));
        const float64 inverse = round_bf16(reciprocal_without_lookup(scale));
        float64_pair_t inv_pair;
        inv_pair.v1 = inverse;
        inv_pair.v2 = inverse;
        const bfloat128 inv_bf16 = v_convert_f32_to_bf16_all_b(inv_pair);
        v_f32_st_tnsr(sc, scales, scale);
        for (int tile = 0; tile < bulk; tile += 4) {
            QUANT_QUAD(tile);
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
