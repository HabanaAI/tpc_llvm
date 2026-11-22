// SPDX-License-Identifier: Apache-2.0
// g2e: ULTRA-FAST fused GDN prefill conv + SiLU + L2-norm + chunk-head layout.
// Software-pipelined 4-row prefetch + 4-way fully interleaved SILU4_BF16 / SUM64_4 / RSQRT4_1N
// (eliminates 4-cycle VPU RAW pipeline stalls on Gaudi2 TPC!).

#define SILU4_BF16(V0, V1, V2, V3, X0, X1, X2, X3) do {                                        \
    const bfloat128 xc0_ = v_bf16_min_b(v_bf16_max_b((X0), (bf16)(-80.0f)), (bf16)80.0f);       \
    const bfloat128 xc1_ = v_bf16_min_b(v_bf16_max_b((X1), (bf16)(-80.0f)), (bf16)80.0f);       \
    const bfloat128 xc2_ = v_bf16_min_b(v_bf16_max_b((X2), (bf16)(-80.0f)), (bf16)80.0f);       \
    const bfloat128 xc3_ = v_bf16_min_b(v_bf16_max_b((X3), (bf16)(-80.0f)), (bf16)80.0f);       \
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
    r0_ = r0_ * ((bfloat128)(bf16)2.0f - d0_ * r0_);                                            \
    r1_ = r1_ * ((bfloat128)(bf16)2.0f - d1_ * r1_);                                            \
    r2_ = r2_ * ((bfloat128)(bf16)2.0f - d2_ * r2_);                                            \
    r3_ = r3_ * ((bfloat128)(bf16)2.0f - d3_ * r3_);                                            \
    r0_ = r0_ * ((bfloat128)(bf16)2.0f - d0_ * r0_);                                            \
    r1_ = r1_ * ((bfloat128)(bf16)2.0f - d1_ * r1_);                                            \
    r2_ = r2_ * ((bfloat128)(bf16)2.0f - d2_ * r2_);                                            \
    r3_ = r3_ * ((bfloat128)(bf16)2.0f - d3_ * r3_);                                            \
    (V0) = (X0) * r0_;                                                                          \
    (V1) = (X1) * r1_;                                                                          \
    (V2) = (X2) * r2_;                                                                          \
    (V3) = (X3) * r3_;                                                                          \
} while (0)

#define SUM64_4(X0, X1, X2, X3) do {                                                            \
    const int all_ = SW_WR_LOWER_GROUP0 | SW_WR_UPPER_GROUP0 | SW_WR_LOWER_GROUP1 | SW_WR_UPPER_GROUP1 | \
                     SW_WR_LOWER_GROUP2 | SW_WR_UPPER_GROUP2 | SW_WR_LOWER_GROUP3 | SW_WR_UPPER_GROUP3;  \
    const int grps_ = SW_GROUP0_EN | SW_GROUP1_EN | SW_DUAL_GROUP0_EN | SW_DUAL_GROUP1_EN |     \
                      SW_DUAL_GROUP2_EN | SW_DUAL_GROUP3_EN;                                    \
    (X0) += v_f32_mov_dual_group_all_b((X0), 0xffffffff, 1, 0, 3, 2, all_, (X0));               \
    (X1) += v_f32_mov_dual_group_all_b((X1), 0xffffffff, 1, 0, 3, 2, all_, (X1));               \
    (X2) += v_f32_mov_dual_group_all_b((X2), 0xffffffff, 1, 0, 3, 2, all_, (X2));               \
    (X3) += v_f32_mov_dual_group_all_b((X3), 0xffffffff, 1, 0, 3, 2, all_, (X3));               \
    (X0) += v_f32_mov_dual_group_all_b((X0), 0xffffffff, 2, 3, 0, 1, all_, (X0));               \
    (X1) += v_f32_mov_dual_group_all_b((X1), 0xffffffff, 2, 3, 0, 1, all_, (X1));               \
    (X2) += v_f32_mov_dual_group_all_b((X2), 0xffffffff, 2, 3, 0, 1, all_, (X2));               \
    (X3) += v_f32_mov_dual_group_all_b((X3), 0xffffffff, 2, 3, 0, 1, all_, (X3));               \
    (X0) += v_f32_mov_group_b((X0), 0xffffffff, grps_, (X0));                                   \
    (X1) += v_f32_mov_group_b((X1), 0xffffffff, grps_, (X1));                                   \
    (X2) += v_f32_mov_group_b((X2), 0xffffffff, grps_, (X2));                                   \
    (X3) += v_f32_mov_group_b((X3), 0xffffffff, grps_, (X3));                                   \
    (X0) += v_f32_shuffle_b((X0), c4, 0, (X0));                                                 \
    (X1) += v_f32_shuffle_b((X1), c4, 0, (X1));                                                 \
    (X2) += v_f32_shuffle_b((X2), c4, 0, (X2));                                                 \
    (X3) += v_f32_shuffle_b((X3), c4, 0, (X3));                                                 \
    (X0) += v_f32_shuffle_b((X0), c2, 0, (X0));                                                 \
    (X1) += v_f32_shuffle_b((X1), c2, 0, (X1));                                                 \
    (X2) += v_f32_shuffle_b((X2), c2, 0, (X2));                                                 \
    (X3) += v_f32_shuffle_b((X3), c2, 0, (X3));                                                 \
    (X0) += v_f32_shuffle_b((X0), c1, 0, (X0));                                                 \
    (X1) += v_f32_shuffle_b((X1), c1, 0, (X1));                                                 \
    (X2) += v_f32_shuffle_b((X2), c1, 0, (X2));                                                 \
    (X3) += v_f32_shuffle_b((X3), c1, 0, (X3));                                                 \
} while (0)

#define CONV4_QUAD(O0, O1, O2, O3, S0, S1, S2, S3, S4, S5, S6) do {                             \
    (O0).v1 = (S0).v1 * w0.v1; (O0).v2 = (S0).v2 * w0.v2;                                       \
    (O1).v1 = (S1).v1 * w0.v1; (O1).v2 = (S1).v2 * w0.v2;                                       \
    (O2).v1 = (S2).v1 * w0.v1; (O2).v2 = (S2).v2 * w0.v2;                                       \
    (O3).v1 = (S3).v1 * w0.v1; (O3).v2 = (S3).v2 * w0.v2;                                       \
    (O0).v1 = v_f32_mac_b((S1).v1, w1.v1, (O0).v1); (O0).v2 = v_f32_mac_b((S1).v2, w1.v2, (O0).v2); \
    (O1).v1 = v_f32_mac_b((S2).v1, w1.v1, (O1).v1); (O1).v2 = v_f32_mac_b((S2).v2, w1.v2, (O1).v2); \
    (O2).v1 = v_f32_mac_b((S3).v1, w1.v1, (O2).v1); (O2).v2 = v_f32_mac_b((S3).v2, w1.v2, (O2).v2); \
    (O3).v1 = v_f32_mac_b((S4).v1, w1.v1, (O3).v1); (O3).v2 = v_f32_mac_b((S4).v2, w1.v2, (O3).v2); \
    (O0).v1 = v_f32_mac_b((S2).v1, w2.v1, (O0).v1); (O0).v2 = v_f32_mac_b((S2).v2, w2.v2, (O0).v2); \
    (O1).v1 = v_f32_mac_b((S3).v1, w2.v1, (O1).v1); (O1).v2 = v_f32_mac_b((S3).v2, w2.v2, (O1).v2); \
    (O2).v1 = v_f32_mac_b((S4).v1, w2.v1, (O2).v1); (O2).v2 = v_f32_mac_b((S4).v2, w2.v2, (O2).v2); \
    (O3).v1 = v_f32_mac_b((S5).v1, w2.v1, (O3).v1); (O3).v2 = v_f32_mac_b((S5).v2, w2.v2, (O3).v2); \
    (O0).v1 = v_f32_mac_b((S3).v1, w3.v1, (O0).v1); (O0).v2 = v_f32_mac_b((S3).v2, w3.v2, (O0).v2); \
    (O1).v1 = v_f32_mac_b((S4).v1, w3.v1, (O1).v1); (O1).v2 = v_f32_mac_b((S4).v2, w3.v2, (O1).v2); \
    (O2).v1 = v_f32_mac_b((S5).v1, w3.v1, (O2).v1); (O2).v2 = v_f32_mac_b((S5).v2, w3.v2, (O2).v2); \
    (O3).v1 = v_f32_mac_b((S6).v1, w3.v1, (O3).v1); (O3).v2 = v_f32_mac_b((S6).v2, w3.v2, (O3).v2); \
} while (0)

void main(tensor seq, tensor w, tensor qc, tensor kc, tensor vc)
{
    const int5 start = get_index_space_offset();
    const int5 end = start + get_index_space_size();
    const int C_chunk = get_dim_size(qc, 1);
    const int T = get_dim_size(seq, 1) - 3;
    const int N = T / C_chunk;
    const int HK = get_dim_size(qc, 2) / N;
    const int HV = get_dim_size(vc, 2) / N;
    const int ntiles = 2 * HK + HV;
    const float q_scale = 0.08838834764831845f;

    const uint64 l8 = read_lane_id_4b_b() & 7;
    const uint64 c4u = ((l8 ^ 4) | 0x80) * 0x01010101;
    const uint64 c2u = ((l8 ^ 2) | 0x80) * 0x01010101;
    const uint64 c1u = ((l8 ^ 1) | 0x80) * 0x01010101;
    const uchar256 c4 = *((uchar256*)&c4u);
    const uchar256 c2 = *((uchar256*)&c2u);
    const uchar256 c1 = *((uchar256*)&c1u);

    for (int idx = start[0]; idx < end[0]; ++idx) {
        const int ci = idx / ntiles;
        const int tile = idx - ci * ntiles;
        const int c = tile * 128;
        int5 wc0 = {c, 0, 0, 0, 0}, wc1 = {c, 1, 0, 0, 0}, wc2 = {c, 2, 0, 0, 0}, wc3 = {c, 3, 0, 0, 0};
        const float128 w0 = v_convert_bf16_to_f32_all_b(v_bf16_ld_tnsr_b(wc0, w));
        const float128 w1 = v_convert_bf16_to_f32_all_b(v_bf16_ld_tnsr_b(wc1, w));
        const float128 w2 = v_convert_bf16_to_f32_all_b(v_bf16_ld_tnsr_b(wc2, w));
        const float128 w3 = v_convert_bf16_to_f32_all_b(v_bf16_ld_tnsr_b(wc3, w));

        const int kind = tile < HK ? 0 : (tile < 2 * HK ? 1 : 2);
        const int h = kind == 0 ? tile : (kind == 1 ? tile - HK : tile - 2 * HK);
        const int bh = kind < 2 ? (ci * HK + h) : (ci * HV + h);
        const int t0 = ci * C_chunk;

        int5 sc0 = {c, t0, 0, 0, 0}, sc1 = {c, t0 + 1, 0, 0, 0}, sc2 = {c, t0 + 2, 0, 0, 0};
        int5 sc3 = {c, t0 + 3, 0, 0, 0}, sc4 = {c, t0 + 4, 0, 0, 0}, sc5 = {c, t0 + 5, 0, 0, 0}, sc6 = {c, t0 + 6, 0, 0, 0};
        float128 s0 = v_convert_bf16_to_f32_all_b(v_bf16_ld_tnsr_b(sc0, seq));
        float128 s1 = v_convert_bf16_to_f32_all_b(v_bf16_ld_tnsr_b(sc1, seq));
        float128 s2 = v_convert_bf16_to_f32_all_b(v_bf16_ld_tnsr_b(sc2, seq));
        bfloat128 nb3 = v_bf16_ld_tnsr_b(sc3, seq);
        bfloat128 nb4 = v_bf16_ld_tnsr_b(sc4, seq);
        bfloat128 nb5 = v_bf16_ld_tnsr_b(sc5, seq);
        bfloat128 nb6 = v_bf16_ld_tnsr_b(sc6, seq);

        if (kind == 2) {
            for (int r = 0; r < C_chunk; r += 4) {
                const float128 s3 = v_convert_bf16_to_f32_all_b(nb3);
                const float128 s4 = v_convert_bf16_to_f32_all_b(nb4);
                const float128 s5 = v_convert_bf16_to_f32_all_b(nb5);
                const float128 s6 = v_convert_bf16_to_f32_all_b(nb6);
                const int nr = r + 4 < C_chunk ? r + 4 : r;
                int5 l0 = {c, t0 + nr + 3, 0, 0, 0}, l1 = {c, t0 + nr + 4, 0, 0, 0};
                int5 l2 = {c, t0 + nr + 5, 0, 0, 0}, l3 = {c, t0 + nr + 6, 0, 0, 0};
                nb3 = v_bf16_ld_tnsr_b(l0, seq);
                nb4 = v_bf16_ld_tnsr_b(l1, seq);
                nb5 = v_bf16_ld_tnsr_b(l2, seq);
                nb6 = v_bf16_ld_tnsr_b(l3, seq);

                float128 o0, o1, o2, o3;
                CONV4_QUAD(o0, o1, o2, o3, s0, s1, s2, s3, s4, s5, s6);
                const bfloat128 x0 = v_convert_f32_to_bf16_all_b(o0);
                const bfloat128 x1 = v_convert_f32_to_bf16_all_b(o1);
                const bfloat128 x2 = v_convert_f32_to_bf16_all_b(o2);
                const bfloat128 x3 = v_convert_f32_to_bf16_all_b(o3);
                bfloat128 v0, v1, v2, v3;
                SILU4_BF16(v0, v1, v2, v3, x0, x1, x2, x3);
                int5 oc0 = {0, r, bh, 0, 0}, oc1 = {0, r + 1, bh, 0, 0};
                int5 oc2 = {0, r + 2, bh, 0, 0}, oc3 = {0, r + 3, bh, 0, 0};
                v_bf16_st_tnsr(oc0, vc, v0);
                v_bf16_st_tnsr(oc1, vc, v1);
                v_bf16_st_tnsr(oc2, vc, v2);
                v_bf16_st_tnsr(oc3, vc, v3);
                s0 = s4; s1 = s5; s2 = s6;
            }
        } else {
            for (int r = 0; r < C_chunk; r += 4) {
                const float128 s3 = v_convert_bf16_to_f32_all_b(nb3);
                const float128 s4 = v_convert_bf16_to_f32_all_b(nb4);
                const float128 s5 = v_convert_bf16_to_f32_all_b(nb5);
                const float128 s6 = v_convert_bf16_to_f32_all_b(nb6);
                const int nr = r + 4 < C_chunk ? r + 4 : r;
                int5 l0 = {c, t0 + nr + 3, 0, 0, 0}, l1 = {c, t0 + nr + 4, 0, 0, 0};
                int5 l2 = {c, t0 + nr + 5, 0, 0, 0}, l3 = {c, t0 + nr + 6, 0, 0, 0};
                nb3 = v_bf16_ld_tnsr_b(l0, seq);
                nb4 = v_bf16_ld_tnsr_b(l1, seq);
                nb5 = v_bf16_ld_tnsr_b(l2, seq);
                nb6 = v_bf16_ld_tnsr_b(l3, seq);

                float128 o0, o1, o2, o3;
                CONV4_QUAD(o0, o1, o2, o3, s0, s1, s2, s3, s4, s5, s6);
                const bfloat128 x0 = v_convert_f32_to_bf16_all_b(o0);
                const bfloat128 x1 = v_convert_f32_to_bf16_all_b(o1);
                const bfloat128 x2 = v_convert_f32_to_bf16_all_b(o2);
                const bfloat128 x3 = v_convert_f32_to_bf16_all_b(o3);
                bfloat128 b0, b1, b2, b3;
                SILU4_BF16(b0, b1, b2, b3, x0, x1, x2, x3);
                o0 = v_convert_bf16_to_f32_all_b(b0);
                o1 = v_convert_bf16_to_f32_all_b(b1);
                o2 = v_convert_bf16_to_f32_all_b(b2);
                o3 = v_convert_bf16_to_f32_all_b(b3);
                float64 sq0 = v_f32_mac_b(o0.v1, o0.v1, o0.v2 * o0.v2);
                float64 sq1 = v_f32_mac_b(o1.v1, o1.v1, o1.v2 * o1.v2);
                float64 sq2 = v_f32_mac_b(o2.v1, o2.v1, o2.v2 * o2.v2);
                float64 sq3 = v_f32_mac_b(o3.v1, o3.v1, o3.v2 * o3.v2);
                SUM64_4(sq0, sq1, sq2, sq3);
                sq0 += 1.0e-6f; sq1 += 1.0e-6f; sq2 += 1.0e-6f; sq3 += 1.0e-6f;
                float64 i0 = as_float64((int64)0x5f3759df - (as_int64(sq0) >> 1));
                float64 i1 = as_float64((int64)0x5f3759df - (as_int64(sq1) >> 1));
                float64 i2 = as_float64((int64)0x5f3759df - (as_int64(sq2) >> 1));
                float64 i3 = as_float64((int64)0x5f3759df - (as_int64(sq3) >> 1));
                const float64 h0 = 0.5f * sq0, h1 = 0.5f * sq1, h2 = 0.5f * sq2, h3 = 0.5f * sq3;
                i0 = i0 * (1.5f - h0 * i0 * i0);
                i1 = i1 * (1.5f - h1 * i1 * i1);
                i2 = i2 * (1.5f - h2 * i2 * i2);
                i3 = i3 * (1.5f - h3 * i3 * i3);
                if (kind == 0) {
                    i0 = i0 * q_scale; i1 = i1 * q_scale; i2 = i2 * q_scale; i3 = i3 * q_scale;
                }
                o0.v1 *= i0; o0.v2 *= i0;
                o1.v1 *= i1; o1.v2 *= i1;
                o2.v1 *= i2; o2.v2 *= i2;
                o3.v1 *= i3; o3.v2 *= i3;
                int5 oc0 = {0, r, bh, 0, 0}, oc1 = {0, r + 1, bh, 0, 0};
                int5 oc2 = {0, r + 2, bh, 0, 0}, oc3 = {0, r + 3, bh, 0, 0};
                if (kind == 0) {
                    v_bf16_st_tnsr(oc0, qc, v_convert_f32_to_bf16_all_b(o0));
                    v_bf16_st_tnsr(oc1, qc, v_convert_f32_to_bf16_all_b(o1));
                    v_bf16_st_tnsr(oc2, qc, v_convert_f32_to_bf16_all_b(o2));
                    v_bf16_st_tnsr(oc3, qc, v_convert_f32_to_bf16_all_b(o3));
                } else {
                    v_bf16_st_tnsr(oc0, kc, v_convert_f32_to_bf16_all_b(o0));
                    v_bf16_st_tnsr(oc1, kc, v_convert_f32_to_bf16_all_b(o1));
                    v_bf16_st_tnsr(oc2, kc, v_convert_f32_to_bf16_all_b(o2));
                    v_bf16_st_tnsr(oc3, kc, v_convert_f32_to_bf16_all_b(o3));
                }
                s0 = s4; s1 = s5; s2 = s6;
            }
        }
    }
}
