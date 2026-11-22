// SPDX-License-Identifier: Apache-2.0
// g2e Direct MME+SRAM FlashDecoding fused tile step (SM(k) + ACC(k-2) [+ Finalize & Transpose]):
//   Executes on 24 TPC cores (384 rows = 4 KV heads x 96 rows -> 16 rows/TPC, 100% balanced)
//   in parallel with MME streaming PV(k-1) + QK(k+1) from HBM!
//   100% LUT-free, 8 KiB VLM score cache (covers up to TILE_K = 4096 with ZERO SRAM re-reads!),
//   4-way VPU-interleaved EXP128_HYBRID_QUAD + ST256_BF_QUAD (0 4-cycle RAW stalls!) + 10-op linear BF16 pack.
// Tensors (dim 0 fastest):
//   0 s_in      BF16 [TILE_K, 384]   scores tile from MME QK(k) in SRAM (TILE_K <= 4096, multiple of 512)
//   1 p_out     BF16/F8 [TILE_K, 384] unnormalized exp probs tile for MME PV(k) in SRAM
//   2 m_sm      F32  [1, 384]        running row max for slot b_sm in SRAM
//   3 l_sm      F32  [1, 384]        running row sum for slot b_sm in SRAM
//   4 r_sm_out  F32  [1, 384]        rescale factor exp(scale*(m_old - m_new)) for tile k in SRAM
//   5 pv_in     F32  [256, 384]      partial PV output from MME PV(k-2) in SRAM
//   6 o_acc     F32  [256, 384]      running F32 output accumulator for slot b_acc in SRAM
//   7 r_acc_in  F32  [1, 384]        rescale factor for tile k-2 in SRAM
//   8 l_acc     F32  [1, 384]        finalized row sum for slot b_acc in SRAM (used when is_last_acc)
//   9 att_out   BF16 [6144, 16]      final transposed BF16 attention output for slot b_acc (HBM or SRAM)
// Scalar SRFs:
//   0 float kl           scale * log2(e) = (1/sqrt(256)) * log2(e)
//   1 int   col_start_sm first cache column of tile k
//   2 int   slot_sm      slot index in last_tab
//   3 int   flags_sm     bit 0: do_sm, bit 1: is_first_sm
//   4 int   flags_acc    bit 0: do_acc, bit 1: is_first_acc, bit 2: is_last_acc
// Index space: dim 1 = row [0..384).

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

static inline float64 exp_biased(float64 x, float kl, float64 nb) {
    const float64 t = v_f32_mac_b(x, kl, nb);
    const int64 n = v_convert_f32_to_i32_b(t, SW_RHNE);
    const float64 f = t - v_convert_i32_to_f32_b(n, 0);
    float64 p = 0.23842697f;
    p = v_f32_mac_b(p, f, 0.7034453f);
    p = v_f32_mac_b(p, f, 1.00044307f);
    const int64 bits = n << 23;
    return p * *((float64*)&bits);
}

static inline float64 rcp3_f32(float64 v) {
    float64 est = as_float64((int64)0x7ef311c3 - as_int64(v));
    est = est * (2.0f - v * est);
    est = est * (2.0f - v * est);
    est = est * (2.0f - v * est);
    return est;
}

static inline bfloat128 f32_to_bf16_lo64(float64 x) {
    const bfloat128 sp = v_convert_f32_to_bf16_b(x, 0, SW_RHNE, (bfloat128)0);
    bfloat128 pk = v_bf16_pack_b(sp, SW_GROUP_0 | SW_STRIDE_2, (bfloat128)0);
    pk = v_bf16_pack_b(sp, SW_GROUP_1 | SW_STRIDE_2, pk);
    return v_bf16_mov_dual_group_pack_b(pk, SW_PACK21, (bfloat128)0);
}

static inline bfloat128 f32_pair_to_bf16_linear_fast(float64 lo, float64 hi) {
    bfloat128 p0 = f32_to_bf16_lo64(lo);
    const bfloat128 p1 = f32_to_bf16_lo64(hi);
    p0 = v_bf16_mov_dual_group_b(p1, 0xffffffff, 0, 2, MkWr(1, 1), p0);
    p0 = v_bf16_mov_dual_group_b(p1, 0xffffffff, 1, 3, MkWr(1, 1), p0);
    return p0;
}

static inline bfloat128 exp128_hybrid(bfloat128 xb, bfloat128 lo, float kl23, float64 nb23) {
    const float128 x = v_convert_bf16_to_f32_all_b(v_bf16_max_b(xb, lo));
    const int64 i1 = v_convert_f32_to_i32_b(v_f32_mac_b(x.v1, kl23, nb23), SW_RHNE);
    const int64 i2 = v_convert_f32_to_i32_b(v_f32_mac_b(x.v2, kl23, nb23), SW_RHNE);
    float128 ylin;
    ylin.v1 = *((float64*)&i1);
    ylin.v2 = *((float64*)&i2);
    const bfloat128 blin = v_convert_f32_to_bf16_all_b(ylin);
    const short128 bu = *((short128*)&blin);
    const short128 f_int = bu & (short128)127;
    const short128 exp_sh = bu & (short128)(~127);
    const bfloat128 f_bf = v_convert_i16_to_bf16_b(f_int, 0);
    bfloat128 p = (bfloat128)1.9788742e-5f;
    p = v_bf16_mac_b(p, f_bf, (bfloat128)0.005218506f);
    p = v_bf16_mac_b(p, f_bf, (bfloat128)1.0f);
    const short128 pu = *((short128*)&p);
    const short128 yu = (pu - (short128)0x3f80) + exp_sh;
    return *((bfloat128*)&yu);
}

// 4-way fully interleaved Hybrid2 exp128 (eliminates all 4-cycle VPU RAW stalls, bitwise identical to exp128_hybrid!)
#define EXP128_HYBRID_QUAD(Y0, Y1, Y2, Y3, X0, X1, X2, X3) do {                                   \
    const bfloat128 c0_ = v_bf16_max_b((X0), lo);                                                 \
    const bfloat128 c1_ = v_bf16_max_b((X1), lo);                                                 \
    const bfloat128 c2_ = v_bf16_max_b((X2), lo);                                                 \
    const bfloat128 c3_ = v_bf16_max_b((X3), lo);                                                 \
    const float128 fx0_ = v_convert_bf16_to_f32_all_b(c0_);                                       \
    const float128 fx1_ = v_convert_bf16_to_f32_all_b(c1_);                                       \
    const float128 fx2_ = v_convert_bf16_to_f32_all_b(c2_);                                       \
    const float128 fx3_ = v_convert_bf16_to_f32_all_b(c3_);                                       \
    const float64 m0a_ = v_f32_mac_b(fx0_.v1, kl23, nb23);                                        \
    const float64 m0b_ = v_f32_mac_b(fx0_.v2, kl23, nb23);                                        \
    const float64 m1a_ = v_f32_mac_b(fx1_.v1, kl23, nb23);                                        \
    const float64 m1b_ = v_f32_mac_b(fx1_.v2, kl23, nb23);                                        \
    const float64 m2a_ = v_f32_mac_b(fx2_.v1, kl23, nb23);                                        \
    const float64 m2b_ = v_f32_mac_b(fx2_.v2, kl23, nb23);                                        \
    const float64 m3a_ = v_f32_mac_b(fx3_.v1, kl23, nb23);                                        \
    const float64 m3b_ = v_f32_mac_b(fx3_.v2, kl23, nb23);                                        \
    const int64 i0a_ = v_convert_f32_to_i32_b(m0a_, SW_RHNE);                                     \
    const int64 i0b_ = v_convert_f32_to_i32_b(m0b_, SW_RHNE);                                     \
    const int64 i1a_ = v_convert_f32_to_i32_b(m1a_, SW_RHNE);                                     \
    const int64 i1b_ = v_convert_f32_to_i32_b(m1b_, SW_RHNE);                                     \
    const int64 i2a_ = v_convert_f32_to_i32_b(m2a_, SW_RHNE);                                     \
    const int64 i2b_ = v_convert_f32_to_i32_b(m2b_, SW_RHNE);                                     \
    const int64 i3a_ = v_convert_f32_to_i32_b(m3a_, SW_RHNE);                                     \
    const int64 i3b_ = v_convert_f32_to_i32_b(m3b_, SW_RHNE);                                     \
    float128 yl0_, yl1_, yl2_, yl3_;                                                              \
    yl0_.v1 = *((float64*)&i0a_); yl0_.v2 = *((float64*)&i0b_);                                   \
    yl1_.v1 = *((float64*)&i1a_); yl1_.v2 = *((float64*)&i1b_);                                   \
    yl2_.v1 = *((float64*)&i2a_); yl2_.v2 = *((float64*)&i2b_);                                   \
    yl3_.v1 = *((float64*)&i3a_); yl3_.v2 = *((float64*)&i3b_);                                   \
    const bfloat128 bl0_ = v_convert_f32_to_bf16_all_b(yl0_);                                     \
    const bfloat128 bl1_ = v_convert_f32_to_bf16_all_b(yl1_);                                     \
    const bfloat128 bl2_ = v_convert_f32_to_bf16_all_b(yl2_);                                     \
    const bfloat128 bl3_ = v_convert_f32_to_bf16_all_b(yl3_);                                     \
    const short128 bu0_ = *((short128*)&bl0_), bu1_ = *((short128*)&bl1_);                        \
    const short128 bu2_ = *((short128*)&bl2_), bu3_ = *((short128*)&bl3_);                        \
    const short128 fi0_ = bu0_ & (short128)127, fi1_ = bu1_ & (short128)127;                      \
    const short128 fi2_ = bu2_ & (short128)127, fi3_ = bu3_ & (short128)127;                      \
    const short128 es0_ = bu0_ & (short128)(~127), es1_ = bu1_ & (short128)(~127);                \
    const short128 es2_ = bu2_ & (short128)(~127), es3_ = bu3_ & (short128)(~127);                \
    const bfloat128 fb0_ = v_convert_i16_to_bf16_b(fi0_, 0);                                      \
    const bfloat128 fb1_ = v_convert_i16_to_bf16_b(fi1_, 0);                                      \
    const bfloat128 fb2_ = v_convert_i16_to_bf16_b(fi2_, 0);                                      \
    const bfloat128 fb3_ = v_convert_i16_to_bf16_b(fi3_, 0);                                      \
    bfloat128 p0_ = v_bf16_mac_b((bfloat128)1.9788742e-5f, fb0_, (bfloat128)0.005218506f);        \
    bfloat128 p1_ = v_bf16_mac_b((bfloat128)1.9788742e-5f, fb1_, (bfloat128)0.005218506f);        \
    bfloat128 p2_ = v_bf16_mac_b((bfloat128)1.9788742e-5f, fb2_, (bfloat128)0.005218506f);        \
    bfloat128 p3_ = v_bf16_mac_b((bfloat128)1.9788742e-5f, fb3_, (bfloat128)0.005218506f);        \
    p0_ = v_bf16_mac_b(p0_, fb0_, (bfloat128)1.0f);                                               \
    p1_ = v_bf16_mac_b(p1_, fb1_, (bfloat128)1.0f);                                               \
    p2_ = v_bf16_mac_b(p2_, fb2_, (bfloat128)1.0f);                                               \
    p3_ = v_bf16_mac_b(p3_, fb3_, (bfloat128)1.0f);                                               \
    const short128 yu0_ = (*((short128*)&p0_) - (short128)0x3f80) + es0_;                         \
    const short128 yu1_ = (*((short128*)&p1_) - (short128)0x3f80) + es1_;                         \
    const short128 yu2_ = (*((short128*)&p2_) - (short128)0x3f80) + es2_;                         \
    const short128 yu3_ = (*((short128*)&p3_) - (short128)0x3f80) + es3_;                         \
    (Y0) = *((bfloat128*)&yu0_); (Y1) = *((bfloat128*)&yu1_);                                     \
    (Y2) = *((bfloat128*)&yu2_); (Y3) = *((bfloat128*)&yu3_);                                     \
} while (0)

#ifdef OUT_FP8
static inline minifloat256 f32_pair_to_f8(float64 v1, float64 v2) {
    minifloat256 packed = 0;
    packed = v_convert_f32_to_f8_b(v1, 0, SW_CLIP_FP, packed);
    packed = v_convert_f32_to_f8_b(v2, 2, SW_CLIP_FP, packed);
    const minifloat256 sparse = packed;
    packed = v_f8_pack_b(sparse, SW_GROUP_0 | SW_STRIDE_2, (minifloat256)0);
    packed = v_f8_pack_b(sparse, SW_GROUP_1 | SW_STRIDE_2, packed);
    return v_f8_mov_dual_group_pack_b(packed, SW_PACK21, (minifloat256)0);
}
static inline minifloat256 bf16_128_to_f8_lo128(bfloat128 x) {
    const minifloat256 sp = v_convert_bf16_to_f8_b(x, 0, SW_CLIP_FP, (minifloat256)0);
    minifloat256 pk = v_f8_pack_b(sp, SW_GROUP_0 | SW_STRIDE_2, (minifloat256)0);
    pk = v_f8_pack_b(sp, SW_GROUP_1 | SW_STRIDE_2, pk);
    return v_f8_mov_dual_group_pack_b(pk, SW_PACK21, (minifloat256)0);
}
#define ST128_BF(coord, yb) v_f8_st_tnsr_partial((coord), p_out, bf16_128_to_f8_lo128(yb), 127, 0)
#define ST256_BF_QUAD(c0, c1, c2, c3, y0, y1, y2, y3) do {                                        \
    (void)(c1); (void)(c3);                                                                       \
    const minifloat256 sp0_ = v_convert_bf16_to_f8_b((y0), 0, SW_CLIP_FP, (minifloat256)0);       \
    const minifloat256 sp1_ = v_convert_bf16_to_f8_b((y1), 0, SW_CLIP_FP, (minifloat256)0);       \
    const minifloat256 sp2_ = v_convert_bf16_to_f8_b((y2), 0, SW_CLIP_FP, (minifloat256)0);       \
    const minifloat256 sp3_ = v_convert_bf16_to_f8_b((y3), 0, SW_CLIP_FP, (minifloat256)0);       \
    minifloat256 q0_ = v_f8_pack_b(sp0_, SW_GROUP_0 | SW_STRIDE_2, (minifloat256)0);              \
    minifloat256 q1_ = v_f8_pack_b(sp1_, SW_GROUP_0 | SW_STRIDE_2, (minifloat256)0);              \
    minifloat256 q2_ = v_f8_pack_b(sp2_, SW_GROUP_0 | SW_STRIDE_2, (minifloat256)0);              \
    minifloat256 q3_ = v_f8_pack_b(sp3_, SW_GROUP_0 | SW_STRIDE_2, (minifloat256)0);              \
    q0_ = v_f8_pack_b(sp0_, SW_GROUP_1 | SW_STRIDE_2, q0_);                                       \
    q1_ = v_f8_pack_b(sp1_, SW_GROUP_1 | SW_STRIDE_2, q1_);                                       \
    q2_ = v_f8_pack_b(sp2_, SW_GROUP_1 | SW_STRIDE_2, q2_);                                       \
    q3_ = v_f8_pack_b(sp3_, SW_GROUP_1 | SW_STRIDE_2, q3_);                                       \
    q0_ = v_f8_mov_dual_group_pack_b(q0_, SW_PACK21, (minifloat256)0);                            \
    q1_ = v_f8_mov_dual_group_pack_b(q1_, SW_PACK21, (minifloat256)0);                            \
    q2_ = v_f8_mov_dual_group_pack_b(q2_, SW_PACK21, (minifloat256)0);                            \
    q3_ = v_f8_mov_dual_group_pack_b(q3_, SW_PACK21, (minifloat256)0);                            \
    q0_ = v_f8_mov_dual_group_b(q1_, 0xffffffff, 0, 2, MkWr(1, 1), q0_);                          \
    q2_ = v_f8_mov_dual_group_b(q3_, 0xffffffff, 0, 2, MkWr(1, 1), q2_);                          \
    q0_ = v_f8_mov_dual_group_b(q1_, 0xffffffff, 1, 3, MkWr(1, 1), q0_);                          \
    q2_ = v_f8_mov_dual_group_b(q3_, 0xffffffff, 1, 3, MkWr(1, 1), q2_);                          \
    v_f8_st_tnsr((c0), p_out, q0_);                                                               \
    v_f8_st_tnsr((c2), p_out, q2_);                                                               \
} while (0)
#else
#define ST128_BF(coord, yb) v_bf16_st_tnsr((coord), p_out, (yb))
#define ST256_BF_QUAD(c0, c1, c2, c3, y0, y1, y2, y3) do {                                        \
    v_bf16_st_tnsr((c0), p_out, (y0));                                                            \
    v_bf16_st_tnsr((c1), p_out, (y1));                                                            \
    v_bf16_st_tnsr((c2), p_out, (y2));                                                            \
    v_bf16_st_tnsr((c3), p_out, (y3));                                                            \
} while (0)
#endif

void main(tensor s_in, tensor p_out, tensor m_sm, tensor l_sm, tensor r_sm_out,
          tensor pv_in, tensor o_acc, tensor r_acc_in, tensor l_acc, tensor att_out,
          tensor last_tab,
          float kl, int col_start_sm, int slot_sm, int flags_sm, int flags_acc)
{
    const int5 start = get_index_space_offset();
    const int5 end = start + get_index_space_size();
    const int tile_k = get_dim_size(s_in, 0);
    const int do_sm = flags_sm & 1;
    const int is_first_sm = (flags_sm >> 1) & 1;
    const int do_acc = flags_acc & 1;
    const int is_first_acc = (flags_acc >> 1) & 1;
    const int is_last_acc = (flags_acc >> 2) & 1;

    int pos_sm = 0;
    if (do_sm) {
        int5 lc = {0, slot_sm, 0, 0, 0};
        pos_sm = s_i32_ld_g((__global__ int*)gen_addr(lc, last_tab));
    }

    const bfloat128 neg = -3.0e38f;
#ifdef OUT_FP8
    const minifloat256 zero_f8 = 0;
#else
    const bfloat128 zero_bf = 0;
#endif
    bfloat128 s_vlm[32];  // 8 KiB VLM cache (full TILE_K <= 4096!)

    for (int row = start[1]; row < end[1]; ++row) {
        int5 rc = {0, row, 0, 0, 0};

        // ---- Part A: Online Softmax on S_in -> P_out (tile k) ----
        if (do_sm) {
            const int h_sm = row / 96;
            const int t_sm = (row - h_sm * 96) & 15;
            int visible = pos_sm + t_sm + 1 - col_start_sm;
            if (visible < 0) visible = 0;
            if (visible > tile_k) visible = tile_k;

            if (visible == 0) {
                for (int col = 0; col < tile_k; col += 256) {
                    int5 pc = {col, row, 0, 0, 0};
#ifdef OUT_FP8
                    v_f8_st_tnsr(pc, p_out, zero_f8);
#else
                    v_bf16_st_tnsr(pc, p_out, zero_bf);
                    pc[0] = col + 128;
                    v_bf16_st_tnsr(pc, p_out, zero_bf);
#endif
                }
                if (is_first_sm) {
                    v_f32_st_tnsr(rc, m_sm, (float64)(-1.0e30f));
                    v_f32_st_tnsr(rc, l_sm, (float64)0.0f);
                    v_f32_st_tnsr(rc, r_sm_out, (float64)0.0f);
                } else {
                    v_f32_st_tnsr(rc, r_sm_out, (float64)1.0f);
                }
            } else {
                const int full = visible >> 7;
                const int rest = visible & 127;
                const int groups = full >> 2;

                // Pass 1: Load into VLM + BF16 128-lane row max
                bfloat128 m0 = neg, m1 = neg, m2 = neg, m3 = neg;
                for (int g = 0; g < groups; ++g) {
                    int5 c0 = {g * 512, row, 0, 0, 0}, c1 = {g * 512 + 128, row, 0, 0, 0};
                    int5 c2 = {g * 512 + 256, row, 0, 0, 0}, c3 = {g * 512 + 384, row, 0, 0, 0};
                    const bfloat128 x0 = v_bf16_ld_tnsr_b(c0, s_in);
                    const bfloat128 x1 = v_bf16_ld_tnsr_b(c1, s_in);
                    const bfloat128 x2 = v_bf16_ld_tnsr_b(c2, s_in);
                    const bfloat128 x3 = v_bf16_ld_tnsr_b(c3, s_in);
                    s_vlm[g * 4 + 0] = x0;
                    s_vlm[g * 4 + 1] = x1;
                    s_vlm[g * 4 + 2] = x2;
                    s_vlm[g * 4 + 3] = x3;
                    m0 = v_bf16_max_b(m0, x0);
                    m1 = v_bf16_max_b(m1, x1);
                    m2 = v_bf16_max_b(m2, x2);
                    m3 = v_bf16_max_b(m3, x3);
                }
                for (int v = groups * 4; v < full; ++v) {
                    int5 cv = {v * 128, row, 0, 0, 0};
                    const bfloat128 xv = v_bf16_ld_tnsr_b(cv, s_in);
                    s_vlm[v] = xv;
                    m0 = v_bf16_max_b(m0, xv);
                }
                bfloat128 edge = neg;
                if (rest) {
                    int5 ce = {full * 128, row, 0, 0, 0};
                    edge = v_bf16_sel_less_u16_b(read_lane_id_2b_b(), (unsigned short)rest,
                                                 v_bf16_ld_tnsr_b(ce, s_in), neg);
                    m1 = v_bf16_max_b(m1, edge);
                }
                const float128 mf = v_convert_bf16_to_f32_all_b(v_bf16_max_b(v_bf16_max_b(m0, m1), v_bf16_max_b(m2, m3)));
                const float64 m_tile = max64(v_f32_max_b(mf.v1, mf.v2));

                float64 m_new = m_tile;
                float64 l_prev = 0.0f;
                float64 rescale = 0.0f;
                if (!is_first_sm) {
                    const float64 m_old = v_broadcast_element_f32(v_f32_ld_tnsr_b(rc, m_sm), 0);
                    const float64 l_old = v_broadcast_element_f32(v_f32_ld_tnsr_b(rc, l_sm), 0);
                    m_new = v_f32_max_b(m_old, m_tile);
                    const float64 diff = m_old - m_new;
                    const float64 t_sc = diff * kl;
                    const float64 d_cl = v_f32_max_b(diff, -120.0f / kl);
                    rescale = v_f32_sel_less_f32_b(t_sc, -120.0f, (float64)0.0f, exp_biased(d_cl, kl, (float64)127.0f));
                    l_prev = l_old * rescale;
                }
                v_f32_st_tnsr(rc, m_sm, m_new);
                v_f32_st_tnsr(rc, r_sm_out, rescale);

#ifdef OUT_FP8
                const float64 nb = 134.0f - m_new * kl;
#else
                const float64 nb = 127.0f - m_new * kl;
#endif
                float128 lof;
                lof.v1 = m_new - 120.0f / kl;
                lof.v2 = lof.v1;
                const bfloat128 lo = v_convert_f32_to_bf16_all_b(lof);

                // Pass 2: 4-way interleaved Hybrid2 exp128 from VLM -> P_out + pairwise FP32 sum
                const float kl23 = kl * 8388608.0f;
                const float64 nb23 = nb * 8388608.0f;
                float64 a0 = 0, a1 = 0;
                for (int g = 0; g < groups; ++g) {
                    bfloat128 y0, y1, y2, y3;
                    EXP128_HYBRID_QUAD(y0, y1, y2, y3,
                                       s_vlm[g * 4 + 0], s_vlm[g * 4 + 1],
                                       s_vlm[g * 4 + 2], s_vlm[g * 4 + 3]);
                    const float128 s4 = v_convert_bf16_to_f32_all_b((y0 + y1) + (y2 + y3));
                    a0 += s4.v1;
                    a1 += s4.v2;
                    int5 p0 = {g * 512, row, 0, 0, 0}, p1 = {g * 512 + 128, row, 0, 0, 0};
                    int5 p2 = {g * 512 + 256, row, 0, 0, 0}, p3 = {g * 512 + 384, row, 0, 0, 0};
                    ST256_BF_QUAD(p0, p1, p2, p3, y0, y1, y2, y3);
                }
                for (int v = groups * 4; v < full; ++v) {
                    const bfloat128 yv = exp128_hybrid(s_vlm[v], lo, kl23, nb23);
                    const float128 sv = v_convert_bf16_to_f32_all_b(yv);
                    a0 += sv.v1;
                    a1 += sv.v2;
                    int5 pv = {v * 128, row, 0, 0, 0};
                    ST128_BF(pv, yv);
                }
                int v_done = full;
                if (rest) {
                    const float128 x_ = v_convert_bf16_to_f32_all_b(v_bf16_max_b(edge, lo));
                    float128 y_;
                    y_.v1 = exp_biased(x_.v1, kl, nb);
                    y_.v2 = exp_biased(x_.v2, kl, nb);
                    const uint64 lanes = read_lane_id_4b_b();
                    y_.v1 = v_f32_sel_less_u32_b(lanes * 2, (unsigned)rest, y_.v1, 0.0f);
                    y_.v2 = v_f32_sel_less_u32_b(lanes * 2 + 1, (unsigned)rest, y_.v2, 0.0f);
                    a0 += y_.v1;
                    a1 += y_.v2;
                    int5 pe = {full * 128, row, 0, 0, 0};
#ifdef OUT_FP8
                    v_f8_st_tnsr_partial(pe, p_out, f32_pair_to_f8(y_.v1, y_.v2), 127, 0);
#else
                    bfloat128 ob = v_convert_f32_to_bf16_all_b(y_);
                    ob = v_bf16_sel_less_u16_b(read_lane_id_2b_b(), (unsigned short)rest, ob, zero_bf);
                    v_bf16_st_tnsr(pe, p_out, ob);
#endif
                    v_done = full + 1;
                }
                const int total_vecs = tile_k >> 7;
                for (int v = v_done; v < total_vecs; ++v) {
                    int5 pz = {v * 128, row, 0, 0, 0};
#ifdef OUT_FP8
                    v_f8_st_tnsr_partial(pz, p_out, zero_f8, 127, 0);
#else
                    v_bf16_st_tnsr(pz, p_out, zero_bf);
#endif
                }
                const float64 l_new = l_prev + sum64(a0 + a1);
                v_f32_st_tnsr(rc, l_sm, l_new);
            }
        }

        // ---- Part B: Accumulate PV(k-2) into O_acc [+ Finalize & Transpose if last tile of slot] ----
        if (do_acc) {
            int5 d0 = {0, row, 0, 0, 0}, d1 = {64, row, 0, 0, 0};
            int5 d2 = {128, row, 0, 0, 0}, d3 = {192, row, 0, 0, 0};
            float64 o0 = v_f32_ld_tnsr_b(d0, pv_in);
            float64 o1 = v_f32_ld_tnsr_b(d1, pv_in);
            float64 o2 = v_f32_ld_tnsr_b(d2, pv_in);
            float64 o3 = v_f32_ld_tnsr_b(d3, pv_in);
            if (!is_first_acc) {
                const float64 r = v_broadcast_element_f32(v_f32_ld_tnsr_b(rc, r_acc_in), 0);
                o0 = v_f32_mac_b(v_f32_ld_tnsr_b(d0, o_acc), r, o0);
                o1 = v_f32_mac_b(v_f32_ld_tnsr_b(d1, o_acc), r, o1);
                o2 = v_f32_mac_b(v_f32_ld_tnsr_b(d2, o_acc), r, o2);
                o3 = v_f32_mac_b(v_f32_ld_tnsr_b(d3, o_acc), r, o3);
            }
            if (is_last_acc) {
                const float64 l_val = v_f32_max_b(v_broadcast_element_f32(v_f32_ld_tnsr_b(rc, l_acc), 0), 1.0e-30f);
                const float64 inv_l = rcp3_f32(l_val);
                o0 *= inv_l;
                o1 *= inv_l;
                o2 *= inv_l;
                o3 *= inv_l;
                const bfloat128 b0 = f32_pair_to_bf16_linear_fast(o0, o1);
                const bfloat128 b1 = f32_pair_to_bf16_linear_fast(o2, o3);
                const int h = row / 96;
                const int rem = row - h * 96;
                const int g = rem >> 4;
                const int t = rem & 15;
                const int nh = h * 6 + g;
                int5 ac0 = {nh * 256, t, 0, 0, 0}, ac1 = {nh * 256 + 128, t, 0, 0, 0};
                v_bf16_st_tnsr(ac0, att_out, b0);
                v_bf16_st_tnsr(ac1, att_out, b1);
            } else {
                v_f32_st_tnsr(d0, o_acc, o0);
                v_f32_st_tnsr(d1, o_acc, o1);
                v_f32_st_tnsr(d2, o_acc, o2);
                v_f32_st_tnsr(d3, o_acc, o3);
            }
        }
    }
}
