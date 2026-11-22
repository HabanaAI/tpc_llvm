// SPDX-License-Identifier: Apache-2.0
// g2e: masked, unnormalised softmax of FP16 attention scores -> BF16 probabilities (128-lane form):
//   row r (query t = r % T) sees columns c <= last[t];  m = max of the visible scores;
//   p[r, c] = 2^(round(128 * scale * log2(e) * (s[r, c] - m)) / 128) as BF16 bits (0 past the visible columns);
//   sum[r] = sum of the BF16 p (FP32, the values the PV product consumes). The caller divides PV by sum.
// After the H3 attention kernels (gaudi2-mme-programming/direct/attention/kernels/exp_f16_logq.c, F16_AFFINE):
// y = (s - m) * 128 log2(e) scale in FP16, q = int16(y) is 128 * the base-2 logarithm on a 1/128 grid; its low
// 7 bits k select the BF16 mantissa. H3 reads round(128 (2^(k/128) - 1)) from a custom LUT slot; Synapse kernels
// have no free slot, so a cubic in k evaluated with FP16 MACs reproduces that table exactly (all 128 entries).
// bits = q + 16256 + corr(k) is the BF16 pattern of p (exponent 127 + floor(q / 128)). Every step works on 128
// lanes: ~14 VPU instructions per 128 scores against ~22 for the FP32 polynomial kernel. Error: the 1/128 log grid
// (+-0.27%), below BF16's half ulp; FP16 scores carry 3 more mantissa bits than BF16 ones.
// Tensors (dim 0 fastest): s [S, R] f16, last [T] i32 (host constants), p [S, R] or [C, R, S/C] bf16,
// sum [1, R] f32. S % 128 == 0, C % 512 == 0. Scalar: softmax scale (> 0).

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

static inline int5 p_coord(int col, int r, int C) {
    int5 c = {col % C, r, col / C, 0, 0};
    return c;
}
#define PC(col) p_coord((col), r, C)

// BF16 bits of p for one 128-score vector
#define PROB(x, out) do {                                                                  \
        short128 q_ = v_convert_f16_to_i16_b(v_f16_mul_b(v_f16_sub_b((x), mh), kh), SW_RHNE); \
        q_ = v_i16_max_b(q_, (short128)-16128);                                            \
        const half128 k_ = v_convert_i16_to_f16_b(q_ & (short128)127, SW_RHNE);            \
        half128 c_ = (half128)4.827976226806641e-06f;                                      \
        c_ = v_f16_mac_b(c_, k_, (half128)0.0017518997192382812f);                          \
        c_ = v_f16_mac_b(c_, k_, (half128)-0.30322265625f);                                 \
        c_ = v_f16_mac_b(c_, k_, (half128)-0.022216796875f);                                \
        short128 b_ = q_ + v_convert_f16_to_i16_b(c_, SW_RHNE) + (short128)16256;          \
        (out) = *(bfloat128*)&b_;                                                          \
    } while (0)
#define ACC(acc, y) ((acc) = v_bf16_mac_acc32_b((y), (bfloat128)1.0f, (acc), 0, 1, 0))

void main(tensor s, tensor last, tensor p, tensor sums, float scale)
{
    const int5 start = get_index_space_offset();
    const int5 end = start + get_index_space_size();
    const int S = get_dim_size(s, 0);
    const int T = get_dim_size(last, 0);
    const bfloat128 zero = 0;
    const half128 neg = (half128)-65504.0f;
    const int chunked = get_dim_size(p, 2) > 1 || get_dim_size(p, 0) != S;
    const int C = chunked ? get_dim_size(p, 0) : S;
    float128 kf;
    kf.v1 = 128.0f * 1.4426950408889634f * scale;
    kf.v2 = kf.v1;
    const half128 kh = v_convert_f32_to_f16_all_b(kf);
    for (int r = start[0]; r < end[0]; ++r) {
        int5 lc = {r % T, 0, 0, 0, 0};
        int visible = s_i32_ld_g((__global__ int*)gen_addr(lc, last)) + 1;
        visible = visible < S ? visible : S;
        const int full = visible / 128;
        const int rest = visible - full * 128;
        const int groups = full / 4;
        // ---- pass 1: row maximum (FP16, 128 lanes per instruction)
        half128 m0 = neg, m1 = neg, m2 = neg, m3 = neg;
        int5 c = {0, r, 0, 0, 0};
        for (int g = 0; g < groups; ++g) {
            int5 c0 = {g * 512, r, 0, 0, 0}, c1 = {g * 512 + 128, r, 0, 0, 0};
            int5 c2 = {g * 512 + 256, r, 0, 0, 0}, c3 = {g * 512 + 384, r, 0, 0, 0};
            m0 = v_f16_max_b(m0, v_f16_ld_tnsr_b(c0, s));
            m1 = v_f16_max_b(m1, v_f16_ld_tnsr_b(c1, s));
            m2 = v_f16_max_b(m2, v_f16_ld_tnsr_b(c2, s));
            m3 = v_f16_max_b(m3, v_f16_ld_tnsr_b(c3, s));
        }
        for (int v = groups * 4; v < full; ++v) {
            c[0] = v * 128;
            m0 = v_f16_max_b(m0, v_f16_ld_tnsr_b(c, s));
        }
        half128 edge = neg;
        if (rest) {  // boundary vector: lanes >= rest masked (bit-level select)
            c[0] = full * 128;
            const half128 x = v_f16_ld_tnsr_b(c, s);
            const short128 sel = v_i16_sel_less_u16_b(read_lane_id_2b_b(), (unsigned short)rest, *(short128*)&x,
                                                      (short128)-1025);  // 0xfbff: FP16 -65504
            edge = *(half128*)&sel;
            m1 = v_f16_max_b(m1, edge);
        }
        const float128 mf = v_convert_f16_to_f32_all_b(v_f16_max_b(v_f16_max_b(m0, m1), v_f16_max_b(m2, m3)));
        float128 mm;
        mm.v1 = max64(v_f32_max_b(mf.v1, mf.v2));
        mm.v2 = mm.v1;
        const half128 mh = v_convert_f32_to_f16_all_b(mm);  // exact: a maximum of FP16 values
        // ---- pass 2: probabilities and their sum
        float128 a0, a1, a2, a3;
        a0.v1 = 0; a0.v2 = 0;
        a1 = a0; a2 = a0; a3 = a0;
        for (int g = 0; g < groups; ++g) {
            int5 c0 = {g * 512, r, 0, 0, 0}, c1 = {g * 512 + 128, r, 0, 0, 0};
            int5 c2 = {g * 512 + 256, r, 0, 0, 0}, c3 = {g * 512 + 384, r, 0, 0, 0};
            const half128 x0 = v_f16_ld_tnsr_b(c0, s);
            const half128 x1 = v_f16_ld_tnsr_b(c1, s);
            const half128 x2 = v_f16_ld_tnsr_b(c2, s);
            const half128 x3 = v_f16_ld_tnsr_b(c3, s);
            bfloat128 o0, o1, o2, o3;
            PROB(x0, o0);
            PROB(x1, o1);
            PROB(x2, o2);
            PROB(x3, o3);
            ACC(a0, o0);
            ACC(a1, o1);
            ACC(a2, o2);
            ACC(a3, o3);
            v_bf16_st_tnsr(PC(g * 512), p, o0);
            v_bf16_st_tnsr(PC(g * 512 + 128), p, o1);
            v_bf16_st_tnsr(PC(g * 512 + 256), p, o2);
            v_bf16_st_tnsr(PC(g * 512 + 384), p, o3);
        }
        for (int v = groups * 4; v < full; ++v) {
            c[0] = v * 128;
            const half128 x = v_f16_ld_tnsr_b(c, s);
            bfloat128 o;
            PROB(x, o);
            ACC(a0, o);
            v_bf16_st_tnsr(PC(v * 128), p, o);
        }
        int next = full;
        if (rest) {
            bfloat128 o;
            PROB(edge, o);
            o = v_bf16_sel_less_u16_b(read_lane_id_2b_b(), (unsigned short)rest, o, zero);
            ACC(a1, o);
            v_bf16_st_tnsr(PC(full * 128), p, o);
            next = full + 1;
        }
        for (int v = next; v < S / 128; ++v) {
            v_bf16_st_tnsr(PC(v * 128), p, zero);
        }
        const float64 a = ((a0.v1 + a1.v1) + (a2.v1 + a3.v1)) + ((a0.v2 + a1.v2) + (a2.v2 + a3.v2));
        int5 sc = {0, r, 0, 0, 0};
        v_f32_st_tnsr(sc, sums, sum64(a));
    }
}
