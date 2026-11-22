// SPDX-License-Identifier: Apache-2.0
// g2e S4-A: Single-pass VLM-staged online softmax stats + topk2 stage-1 chunk max fusion (chunk = 128).
// Eliminates BOTH the separate reduce("max", lg) HBM pass AND the 2nd HBM pass of row_softmax_stats (3x -> 1x HBM reads!).
// Shards every row into 3 aligned partitions (3 * R items -> 48 at R=16, 384 at R=128 = 100% 24-TPC saturation, zero tail wave).
// 100% LUT-free (uses 16 KiB VLM for 4096-float superblock staging), 4-way VPU-interleaved base-2 polynomial exp.
// Tensors (dim 0 fastest):
//   0 logits   F32 [V, R]      (V % 128 == 0, nc = V / 128 >= 384)
//   1 inv_temp F32 [1, R] or [1]
//   2 pmx      F32 [64, R, 3]  partial mx = raw_m * inv_temp (256B-isolated per (r, p))
//   3 psum     F32 [64, R, 3]  partial sum of exp((x - raw_m) * inv_temp)
//   4 cmax     F32 [nc, R]     per-128-column chunk max for Ops::topk2
// Index space: [3 * R] where r = idx / 3, p = idx % 3.

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

static inline float64 exp_shifted_f32(float64 x, float64 m, float kl, float neg_span120)
{
    const float64 d = v_f32_max_b(x - m, neg_span120);
    const float64 t = v_f32_mac_b(d, kl, 127.0f);
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

// 4-way VPU-interleaved exp_shifted_f32 accumulation: subtracts m BEFORE multiplying by kl (zero cancellation!).
#define EXP4_ACC(x0, x1, x2, x3, m, kl, neg_span, a0, a1, a2, a3) do {             \
    const float64 _d0 = v_f32_max_b((x0) - (m), (neg_span));                       \
    const float64 _d1 = v_f32_max_b((x1) - (m), (neg_span));                       \
    const float64 _d2 = v_f32_max_b((x2) - (m), (neg_span));                       \
    const float64 _d3 = v_f32_max_b((x3) - (m), (neg_span));                       \
    const float64 _t0 = v_f32_mac_b(_d0, (kl), 127.0f);                            \
    const float64 _t1 = v_f32_mac_b(_d1, (kl), 127.0f);                            \
    const float64 _t2 = v_f32_mac_b(_d2, (kl), 127.0f);                            \
    const float64 _t3 = v_f32_mac_b(_d3, (kl), 127.0f);                            \
    const int64 _n0 = v_convert_f32_to_i32_b(_t0, SW_RHNE);                        \
    const int64 _n1 = v_convert_f32_to_i32_b(_t1, SW_RHNE);                        \
    const int64 _n2 = v_convert_f32_to_i32_b(_t2, SW_RHNE);                        \
    const int64 _n3 = v_convert_f32_to_i32_b(_t3, SW_RHNE);                        \
    const float64 _f0 = _t0 - v_convert_i32_to_f32_b(_n0, 0);                      \
    const float64 _f1 = _t1 - v_convert_i32_to_f32_b(_n1, 0);                      \
    const float64 _f2 = _t2 - v_convert_i32_to_f32_b(_n2, 0);                      \
    const float64 _f3 = _t3 - v_convert_i32_to_f32_b(_n3, 0);                      \
    float64 _p0 = 0.0096181291f, _p1 = 0.0096181291f;                              \
    float64 _p2 = 0.0096181291f, _p3 = 0.0096181291f;                              \
    _p0 = v_f32_mac_b(_p0, _f0, 0.0555041087f);                                    \
    _p1 = v_f32_mac_b(_p1, _f1, 0.0555041087f);                                    \
    _p2 = v_f32_mac_b(_p2, _f2, 0.0555041087f);                                    \
    _p3 = v_f32_mac_b(_p3, _f3, 0.0555041087f);                                    \
    _p0 = v_f32_mac_b(_p0, _f0, 0.2402265070f);                                    \
    _p1 = v_f32_mac_b(_p1, _f1, 0.2402265070f);                                    \
    _p2 = v_f32_mac_b(_p2, _f2, 0.2402265070f);                                    \
    _p3 = v_f32_mac_b(_p3, _f3, 0.2402265070f);                                    \
    _p0 = v_f32_mac_b(_p0, _f0, 0.6931471806f);                                    \
    _p1 = v_f32_mac_b(_p1, _f1, 0.6931471806f);                                    \
    _p2 = v_f32_mac_b(_p2, _f2, 0.6931471806f);                                    \
    _p3 = v_f32_mac_b(_p3, _f3, 0.6931471806f);                                    \
    _p0 = v_f32_mac_b(_p0, _f0, 1.0f);                                             \
    _p1 = v_f32_mac_b(_p1, _f1, 1.0f);                                             \
    _p2 = v_f32_mac_b(_p2, _f2, 1.0f);                                             \
    _p3 = v_f32_mac_b(_p3, _f3, 1.0f);                                             \
    const int64 _b0 = _n0 << 23, _b1 = _n1 << 23;                                  \
    const int64 _b2 = _n2 << 23, _b3 = _n3 << 23;                                  \
    (a0) = v_f32_mac_b(_p0, *((float64*)&_b0), (a0));                              \
    (a1) = v_f32_mac_b(_p1, *((float64*)&_b1), (a1));                              \
    (a2) = v_f32_mac_b(_p2, *((float64*)&_b2), (a2));                              \
    (a3) = v_f32_mac_b(_p3, *((float64*)&_b3), (a3));                              \
} while (0)

__local__ float64 vbuf[64];

void main(tensor logits, tensor inv_temp, tensor pmx, tensor psum, tensor cmax)
{
    const int5 start = get_index_space_offset();
    const int5 end = start + get_index_space_size();
    const int V = get_dim_size(logits, 0);
    const int nc = V / 128;
    const int c_per = ((nc / 3) / 64) * 64;
    const int it_rows = get_dim_size(inv_temp, 1);
    const uint64 lanes = read_lane_id_4b_b();
    const float64 neg = -3.0e38f;

    for (int idx = start[0]; idx < end[0]; ++idx) {
        const int r = idx / 3;
        const int p = idx - r * 3;
        const int c_beg = p * c_per;
        const int c_end = (p == 2) ? nc : (c_beg + c_per);

        int5 tc = {0, it_rows > 1 ? r : 0, 0, 0, 0};
        const float it = s_f32_ld_g((__global__ float*)gen_addr(tc, inv_temp));
        const float kl = it * 1.4426950408889634f;
        const float neg_span120 = -120.0f / kl;

        float64 cur_m = neg;
        float64 a0 = 0.0f, a1 = 0.0f, a2 = 0.0f, a3 = 0.0f;
        float64 cm_pack = 0.0f;
        int pack_lane = 0;
        int pack_base = c_beg;

        int c = c_beg;
        while (c < c_end) {
            const int rem = c_end - c;
            const int nch = rem >= 32 ? 32 : rem;
            const int pairs = nch >> 1;

            // Phase 1 (single HBM read): load <= 32 chunks (64 vectors = 16 KiB) into VLM, compute chunk maxes + superblock max
            float64 sb_max0 = neg, sb_max1 = neg;
            int col = c * 128;
            int bi = 0;
            for (int k = 0; k < pairs; ++k) {
                int5 c0 = {col,       r, 0, 0, 0};
                int5 c1 = {col + 64,  r, 0, 0, 0};
                int5 c2 = {col + 128, r, 0, 0, 0};
                int5 c3 = {col + 192, r, 0, 0, 0};
                const float64 x0 = v_f32_ld_tnsr_b(c0, logits);
                const float64 x1 = v_f32_ld_tnsr_b(c1, logits);
                const float64 x2 = v_f32_ld_tnsr_b(c2, logits);
                const float64 x3 = v_f32_ld_tnsr_b(c3, logits);
                vbuf[bi]     = x0;
                vbuf[bi + 1] = x1;
                vbuf[bi + 2] = x2;
                vbuf[bi + 3] = x3;
                const float64 m01 = v_f32_max_b(x0, x1);
                const float64 m23 = v_f32_max_b(x2, x3);
                sb_max0 = v_f32_max_b(sb_max0, m01);
                sb_max1 = v_f32_max_b(sb_max1, m23);
                const float64 cm0 = max64(m01);
                const float64 cm1 = max64(m23);
                cm_pack = v_f32_mov_vb(cm0, 0, cm_pack, v_u32_cmp_eq_b(lanes, (uint64)pack_lane));
                cm_pack = v_f32_mov_vb(cm1, 0, cm_pack, v_u32_cmp_eq_b(lanes, (uint64)(pack_lane + 1)));
                pack_lane += 2;
                col += 256;
                bi += 4;
            }
            if (nch & 1) {
                int5 c0 = {col,      r, 0, 0, 0};
                int5 c1 = {col + 64, r, 0, 0, 0};
                const float64 x0 = v_f32_ld_tnsr_b(c0, logits);
                const float64 x1 = v_f32_ld_tnsr_b(c1, logits);
                vbuf[bi]     = x0;
                vbuf[bi + 1] = x1;
                const float64 m01 = v_f32_max_b(x0, x1);
                sb_max0 = v_f32_max_b(sb_max0, m01);
                const float64 cm0 = max64(m01);
                cm_pack = v_f32_mov_vb(cm0, 0, cm_pack, v_u32_cmp_eq_b(lanes, (uint64)pack_lane));
                pack_lane += 1;
                bi += 2;
            }
            if (pack_lane == 64) {
                int5 mc = {pack_base, r, 0, 0, 0};
                v_f32_st_tnsr(mc, cmax, cm_pack);
                pack_base += 64;
                pack_lane = 0;
            }

            // Online max update + accumulator rescale (once per 4096 floats)
            const float64 smax = max64(v_f32_max_b(sb_max0, sb_max1));
            const float64 m_new = v_f32_max_b(cur_m, smax);
            const float64 sc_raw = exp_shifted_f32(cur_m, m_new, kl, neg_span120);
            const float64 scale = v_f32_sel_leq_f32_b(cur_m - m_new, neg_span120, 0.0f, sc_raw);
            a0 *= scale;
            a1 *= scale;
            a2 *= scale;
            a3 *= scale;
            cur_m = m_new;

            // Phase 2 (zero HBM reads!): stream VLM through 4-way interleaved exp
            for (int vi = 0; vi < pairs * 4; vi += 4) {
                EXP4_ACC(vbuf[vi], vbuf[vi + 1], vbuf[vi + 2], vbuf[vi + 3], m_new, kl, neg_span120, a0, a1, a2, a3);
            }
            if (nch & 1) {
                a0 += exp_shifted_f32(vbuf[pairs * 4], m_new, kl, neg_span120);
                a1 += exp_shifted_f32(vbuf[pairs * 4 + 1], m_new, kl, neg_span120);
            }
            c += nch;
        }
        if (pack_lane > 0) {
            int5 mc = {pack_base, r, 0, 0, 0};
            v_f32_st_tnsr(mc, cmax, cm_pack);
        }

        int5 oc = {0, r, p, 0, 0};
        v_f32_st_tnsr(oc, pmx, cur_m * it);
        v_f32_st_tnsr(oc, psum, sum64((a0 + a1) + (a2 + a3)));
    }
}
