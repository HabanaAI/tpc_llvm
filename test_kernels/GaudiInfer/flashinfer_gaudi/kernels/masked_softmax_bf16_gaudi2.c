// SPDX-License-Identifier: Apache-2.0
// g2e: masked, unnormalised softmax of attention scores, one index point per row (VLIW-friendly form):
//   row r (query t = r % T) sees columns c <= last[t];  m = max of the visible scores;
//   p[r, c] = exp(scale * (s[r, c] - m)) (BF16, 0 past the visible columns);  sum[r] = sum of p (FP32 before
//   rounding to BF16). The caller divides PV by sum.
// Steady loops have no masking and no branches: four independent 128-column vectors per iteration (four
// accumulators), the boundary vector is masked separately, columns past it are zero-filled. exp is a LUT-free
// base-2 quadratic (relative error 1.7e-3, below BF16's half ulp 3.9e-3) with the row bias folded into the
// exponent field (after the H3 attention kernels, gaudi2-mme-programming/direct/attention/kernels/exp_sum.c).
// The kernel is VPU-bound (one vector instruction per cycle), so instructions per element are what counts: the
// row maximum and the underflow clamp work on 128 BF16 lanes at once, the sum adds the unrounded FP32 values.
// Tensors (dim 0 fastest): s [S, R] bf16, last [T] i32 (host constants), p [S, R] bf16, sum [1, R] f32.
// S % 128 == 0. p may also be [C, R, S/C] (C % 512 == 0): columns in chunks, ready for a batched PV GEMM.

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

// 2^t for t = x * kl + nb (kl = scale * log2(e), nb = 127 - m * kl: the IEEE bias folded in); x is clamped (in
// BF16, by the caller) so that t >= ~6 keeps the exponent field valid. Relative-minimax quadratic for 2^f,
// f in [-0.5, 0.5].
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
#define OUT_VEC minifloat256
// exp of one 128-column BF16 vector (clamped from below at lo) -> FP8 E4M3 x128 out; FP32 values (x128) added to acc
#define EXP128(xb, out, acc) do {                                                  \
        const float128 x_ = v_convert_bf16_to_f32_all_b(v_bf16_max_b((xb), lo));  \
        float128 y_;                                                               \
        y_.v1 = exp_biased(x_.v1, kl, nb); y_.v2 = exp_biased(x_.v2, kl, nb);      \
        (out) = f32_pair_to_f8(y_.v1, y_.v2);                                      \
        (acc) += y_.v1 + y_.v2;                                                    \
    } while (0)
#define ST128(coord, val) v_f8_st_tnsr_partial((coord), p, (val), 127, 0)
#define ST256_PAIR(c_lo, c_hi, v_lo, v_hi) do {                                    \
        (void)(c_hi);                                                              \
        minifloat256 pk_ = v_f8_mov_dual_group_b((v_hi), 0xffffffff, 0, 2, MkWr(1, 1), (v_lo)); \
        pk_ = v_f8_mov_dual_group_b((v_hi), 0xffffffff, 1, 3, MkWr(1, 1), pk_);    \
        v_f8_st_tnsr((c_lo), p, pk_);                                              \
    } while (0)
#else
#define OUT_VEC bfloat128
// exp of one 128-column BF16 vector (clamped from below at lo) -> BF16 out; the FP32 values added to acc
#define EXP128(xb, out, acc) do {                                                  \
        const float128 x_ = v_convert_bf16_to_f32_all_b(v_bf16_max_b((xb), lo));  \
        float128 y_;                                                               \
        y_.v1 = exp_biased(x_.v1, kl, nb); y_.v2 = exp_biased(x_.v2, kl, nb);      \
        (out) = v_convert_f32_to_bf16_all_b(y_);                                   \
        (acc) += y_.v1 + y_.v2;                                                    \
    } while (0)
#define ST128(coord, val) v_bf16_st_tnsr((coord), p, (val))
#define ST256_PAIR(c_lo, c_hi, v_lo, v_hi) do {                                    \
        ST128((c_lo), (v_lo));                                                     \
        ST128((c_hi), (v_hi));                                                     \
    } while (0)
#endif

#define VLM_GROUPS 56
#define VLM_VECS (VLM_GROUPS * 4)

#ifdef WITH_MAXES
void main(tensor s, tensor last, tensor p, tensor sums, tensor maxes, float scale)
#else
void main(tensor s, tensor last, tensor p, tensor sums, float scale)
#endif
{
    const float kl = scale * 1.4426950408889634f;  // softmax scale (> 0) folded into the exponent
    const int5 start = get_index_space_offset();
    const int5 end = start + get_index_space_size();
    const int S = get_dim_size(s, 0);
    const int T = get_dim_size(last, 0);
    const bfloat128 zero = 0;
#ifdef OUT_FP8
    const minifloat256 zero_f8 = 0;
#endif
    const bfloat128 neg = -3.0e38f;
    const int chunked = get_dim_size(p, 2) > 1 || get_dim_size(p, 0) != S;
    const int C = chunked ? get_dim_size(p, 0) : S;
    bfloat128 s_vlm[VLM_VECS];  // 56 KiB on-core VLM cache (eliminates Pass 2 HBM score reads up to 28.6K tokens!)
    for (int r = start[0]; r < end[0]; ++r) {
        int5 lc = {r % T, 0, 0, 0, 0};
        int visible = s_i32_ld_g((__global__ int*)gen_addr(lc, last)) + 1;
        visible = visible < S ? visible : S;
        const int full = visible / 128;         // whole visible vectors
        const int rest = visible - full * 128;  // visible lanes of the boundary vector
        const int groups = full / 4;
        const int vlm_groups = groups < VLM_GROUPS ? groups : VLM_GROUPS;
        // ---- pass 1: row maximum (BF16, 128 lanes per instruction) + VLM caching
        bfloat128 m0 = neg, m1 = neg, m2 = neg, m3 = neg;
        int5 c = {0, r, 0, 0, 0};
        for (int g = 0; g < vlm_groups; ++g) {
            int5 c0 = {g * 512, r, 0, 0, 0}, c1 = {g * 512 + 128, r, 0, 0, 0};
            int5 c2 = {g * 512 + 256, r, 0, 0, 0}, c3 = {g * 512 + 384, r, 0, 0, 0};
            const bfloat128 x0 = v_bf16_ld_tnsr_b(c0, s);
            const bfloat128 x1 = v_bf16_ld_tnsr_b(c1, s);
            const bfloat128 x2 = v_bf16_ld_tnsr_b(c2, s);
            const bfloat128 x3 = v_bf16_ld_tnsr_b(c3, s);
            s_vlm[g * 4 + 0] = x0;
            s_vlm[g * 4 + 1] = x1;
            s_vlm[g * 4 + 2] = x2;
            s_vlm[g * 4 + 3] = x3;
            m0 = v_bf16_max_b(m0, x0);
            m1 = v_bf16_max_b(m1, x1);
            m2 = v_bf16_max_b(m2, x2);
            m3 = v_bf16_max_b(m3, x3);
        }
        for (int g = vlm_groups; g < groups; ++g) {
            int5 c0 = {g * 512, r, 0, 0, 0}, c1 = {g * 512 + 128, r, 0, 0, 0};
            int5 c2 = {g * 512 + 256, r, 0, 0, 0}, c3 = {g * 512 + 384, r, 0, 0, 0};
            m0 = v_bf16_max_b(m0, v_bf16_ld_tnsr_b(c0, s));
            m1 = v_bf16_max_b(m1, v_bf16_ld_tnsr_b(c1, s));
            m2 = v_bf16_max_b(m2, v_bf16_ld_tnsr_b(c2, s));
            m3 = v_bf16_max_b(m3, v_bf16_ld_tnsr_b(c3, s));
        }
        for (int v = groups * 4; v < full; ++v) {
            c[0] = v * 128;
            const bfloat128 xv = v_bf16_ld_tnsr_b(c, s);
            if (v < VLM_VECS) s_vlm[v] = xv;
            m0 = v_bf16_max_b(m0, xv);
        }
        bfloat128 edge = neg;
        if (rest) {  // boundary vector: lanes >= rest masked
            c[0] = full * 128;
            edge = v_bf16_sel_less_u16_b(read_lane_id_2b_b(), (unsigned short)rest, v_bf16_ld_tnsr_b(c, s), neg);
            m1 = v_bf16_max_b(m1, edge);
        }
        const float128 mf = v_convert_bf16_to_f32_all_b(v_bf16_max_b(v_bf16_max_b(m0, m1), v_bf16_max_b(m2, m3)));
        const float64 m = max64(v_f32_max_b(mf.v1, mf.v2));
#ifdef OUT_FP8
        const float64 nb = 134.0f - m * kl;  // +7 in base-2 exponent folds x128 into FP8 probabilities & sums
#else
        const float64 nb = 127.0f - m * kl;
#endif
        // underflow clamp: scores below m - 120 / kl give 2^(t >= ~7) ~ 0 instead of a broken exponent field
        float128 lof;
        lof.v1 = m - 120.0f / kl;
        lof.v2 = lof.v1;
        const bfloat128 lo = v_convert_f32_to_bf16_all_b(lof);
        // ---- pass 2: exponentials and their sum (reads from VLM for first 28.6K columns!)
        float64 a0 = 0, a1 = 0, a2 = 0, a3 = 0;
        int p_col = 0, p_ch = 0;
        for (int g = 0; g < vlm_groups; ++g) {
            const bfloat128 x0 = s_vlm[g * 4 + 0];
            const bfloat128 x1 = s_vlm[g * 4 + 1];
            const bfloat128 x2 = s_vlm[g * 4 + 2];
            const bfloat128 x3 = s_vlm[g * 4 + 3];
            OUT_VEC o0, o1, o2, o3;
            EXP128(x0, o0, a0);
            EXP128(x1, o1, a1);
            EXP128(x2, o2, a2);
            EXP128(x3, o3, a3);
            int5 p0 = {p_col, r, p_ch, 0, 0}, p1 = {p_col + 128, r, p_ch, 0, 0};
            int5 p2 = {p_col + 256, r, p_ch, 0, 0}, p3 = {p_col + 384, r, p_ch, 0, 0};
            ST256_PAIR(p0, p1, o0, o1);
            ST256_PAIR(p2, p3, o2, o3);
            p_col += 512;
            if (p_col == C) { p_col = 0; ++p_ch; }
        }
        for (int g = vlm_groups; g < groups; ++g) {
            int5 c0 = {g * 512, r, 0, 0, 0}, c1 = {g * 512 + 128, r, 0, 0, 0};
            int5 c2 = {g * 512 + 256, r, 0, 0, 0}, c3 = {g * 512 + 384, r, 0, 0, 0};
            const bfloat128 x0 = v_bf16_ld_tnsr_b(c0, s);
            const bfloat128 x1 = v_bf16_ld_tnsr_b(c1, s);
            const bfloat128 x2 = v_bf16_ld_tnsr_b(c2, s);
            const bfloat128 x3 = v_bf16_ld_tnsr_b(c3, s);
            OUT_VEC o0, o1, o2, o3;
            EXP128(x0, o0, a0);
            EXP128(x1, o1, a1);
            EXP128(x2, o2, a2);
            EXP128(x3, o3, a3);
            int5 p0 = {p_col, r, p_ch, 0, 0}, p1 = {p_col + 128, r, p_ch, 0, 0};
            int5 p2 = {p_col + 256, r, p_ch, 0, 0}, p3 = {p_col + 384, r, p_ch, 0, 0};
            ST256_PAIR(p0, p1, o0, o1);
            ST256_PAIR(p2, p3, o2, o3);
            p_col += 512;
            if (p_col == C) { p_col = 0; ++p_ch; }
        }
        for (int v = groups * 4; v < full; ++v) {
            c[0] = v * 128;
            const bfloat128 x = v < VLM_VECS ? s_vlm[v] : v_bf16_ld_tnsr_b(c, s);
            OUT_VEC o;
            EXP128(x, o, a0);
            int5 pc = {p_col, r, p_ch, 0, 0};
            ST128(pc, o);
            p_col += 128;
            if (p_col == C) { p_col = 0; ++p_ch; }
        }
        int next = full;
        if (rest) {  // masked lanes were clamped to lo: their ~2^-120 terms vanish in the sum
            int5 pc = {p_col, r, p_ch, 0, 0};
#ifdef OUT_FP8
            const float128 x_ = v_convert_bf16_to_f32_all_b(v_bf16_max_b(edge, lo));
            float128 y_;
            y_.v1 = exp_biased(x_.v1, kl, nb); y_.v2 = exp_biased(x_.v2, kl, nb);
            a1 += y_.v1 + y_.v2;
            bfloat128 ob = v_convert_f32_to_bf16_all_b(y_);
            ob = v_bf16_sel_less_u16_b(read_lane_id_2b_b(), (unsigned short)rest, ob, zero);
            const float128 ym = v_convert_bf16_to_f32_all_b(ob);
            ST128(pc, f32_pair_to_f8(ym.v1, ym.v2));
#else
            bfloat128 o;
            EXP128(edge, o, a1);
            o = v_bf16_sel_less_u16_b(read_lane_id_2b_b(), (unsigned short)rest, o, zero);
            ST128(pc, o);
#endif
            p_col += 128;
            if (p_col == C) { p_col = 0; ++p_ch; }
            next = full + 1;
        }
        for (int v = next; v < S / 128; ++v) {  // past the visible columns
            int5 pc = {p_col, r, p_ch, 0, 0};
#ifdef OUT_FP8
            ST128(pc, zero_f8);
#else
            ST128(pc, zero);
#endif
            p_col += 128;
            if (p_col == C) { p_col = 0; ++p_ch; }
        }
        int5 sc = {0, r, 0, 0, 0};
        v_f32_st_tnsr(sc, sums, sum64((a0 + a1) + (a2 + a3)));
#ifdef WITH_MAXES
        v_f32_st_tnsr(sc, maxes, m);
#endif
    }
}
