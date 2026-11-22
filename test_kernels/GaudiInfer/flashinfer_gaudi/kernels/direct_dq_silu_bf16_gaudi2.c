// SPDX-License-Identifier: Apache-2.0
// g2e direct MME+SRAM kernel: 4-way VPU-interleaved in-place dequant + fast LUT-free SiLU on gate [F, R] in SRAM.
// 3-way row split (grid = 3*R along dim 0): 48 items at B=1 (exact 2 items/TPC on all 24 TPCs!), 384 items at B=8!
// Tensors (dim 0 fastest):
//   0 gate   BF16 [F, R]  in-place input/output in SRAM
//   1 sa     F32  [1, R]  row activation scale in SRAM
//   2 swb    F32  [F]     pre-interleaved weight column scales [even 64, odd 64] per 128 cols
// Index space: dim 0 = item [0..3*R) -> row = item / 3, part = item % 3.

#define SILU4_F32(z0, z1, z2, z3, o0, o1, o2, o3) do {                                            \
    const float64 zc0_ = v_f32_min_b(v_f32_max_b((z0), -87.0f), 87.0f);                           \
    const float64 zc1_ = v_f32_min_b(v_f32_max_b((z1), -87.0f), 87.0f);                           \
    const float64 zc2_ = v_f32_min_b(v_f32_max_b((z2), -87.0f), 87.0f);                           \
    const float64 zc3_ = v_f32_min_b(v_f32_max_b((z3), -87.0f), 87.0f);                           \
    const float64 t0_ = v_f32_mac_b(zc0_, -1.4426950408889634f, (float64)127.0f);                 \
    const float64 t1_ = v_f32_mac_b(zc1_, -1.4426950408889634f, (float64)127.0f);                 \
    const float64 t2_ = v_f32_mac_b(zc2_, -1.4426950408889634f, (float64)127.0f);                 \
    const float64 t3_ = v_f32_mac_b(zc3_, -1.4426950408889634f, (float64)127.0f);                 \
    const int64 n0_ = v_convert_f32_to_i32_b(t0_, SW_RHNE);                                       \
    const int64 n1_ = v_convert_f32_to_i32_b(t1_, SW_RHNE);                                       \
    const int64 n2_ = v_convert_f32_to_i32_b(t2_, SW_RHNE);                                       \
    const int64 n3_ = v_convert_f32_to_i32_b(t3_, SW_RHNE);                                       \
    const float64 f0_ = t0_ - v_convert_i32_to_f32_b(n0_, 0);                                     \
    const float64 f1_ = t1_ - v_convert_i32_to_f32_b(n1_, 0);                                     \
    const float64 f2_ = t2_ - v_convert_i32_to_f32_b(n2_, 0);                                     \
    const float64 f3_ = t3_ - v_convert_i32_to_f32_b(n3_, 0);                                     \
    float64 p0_ = 0.0096181291f, p1_ = 0.0096181291f, p2_ = 0.0096181291f, p3_ = 0.0096181291f;   \
    p0_ = v_f32_mac_b(p0_, f0_, 0.0555041087f); p1_ = v_f32_mac_b(p1_, f1_, 0.0555041087f);       \
    p2_ = v_f32_mac_b(p2_, f2_, 0.0555041087f); p3_ = v_f32_mac_b(p3_, f3_, 0.0555041087f);       \
    p0_ = v_f32_mac_b(p0_, f0_, 0.2402265070f); p1_ = v_f32_mac_b(p1_, f1_, 0.2402265070f);       \
    p2_ = v_f32_mac_b(p2_, f2_, 0.2402265070f); p3_ = v_f32_mac_b(p3_, f3_, 0.2402265070f);       \
    p0_ = v_f32_mac_b(p0_, f0_, 0.6931471806f); p1_ = v_f32_mac_b(p1_, f1_, 0.6931471806f);       \
    p2_ = v_f32_mac_b(p2_, f2_, 0.6931471806f); p3_ = v_f32_mac_b(p3_, f3_, 0.6931471806f);       \
    p0_ = v_f32_mac_b(p0_, f0_, 1.0f);          p1_ = v_f32_mac_b(p1_, f1_, 1.0f);                \
    p2_ = v_f32_mac_b(p2_, f2_, 1.0f);          p3_ = v_f32_mac_b(p3_, f3_, 1.0f);                \
    const int64 b0_ = n0_ << 23, b1_ = n1_ << 23, b2_ = n2_ << 23, b3_ = n3_ << 23;               \
    const float64 d0_ = 1.0f + p0_ * (*((float64*)&b0_));                                         \
    const float64 d1_ = 1.0f + p1_ * (*((float64*)&b1_));                                         \
    const float64 d2_ = 1.0f + p2_ * (*((float64*)&b2_));                                         \
    const float64 d3_ = 1.0f + p3_ * (*((float64*)&b3_));                                         \
    float64 e0_ = as_float64((int64)0x7ef311c3 - as_int64(d0_));                                  \
    float64 e1_ = as_float64((int64)0x7ef311c3 - as_int64(d1_));                                  \
    float64 e2_ = as_float64((int64)0x7ef311c3 - as_int64(d2_));                                  \
    float64 e3_ = as_float64((int64)0x7ef311c3 - as_int64(d3_));                                  \
    e0_ = e0_ * (2.0f - d0_ * e0_); e1_ = e1_ * (2.0f - d1_ * e1_);                               \
    e2_ = e2_ * (2.0f - d2_ * e2_); e3_ = e3_ * (2.0f - d3_ * e3_);                               \
    e0_ = e0_ * (2.0f - d0_ * e0_); e1_ = e1_ * (2.0f - d1_ * e1_);                               \
    e2_ = e2_ * (2.0f - d2_ * e2_); e3_ = e3_ * (2.0f - d3_ * e3_);                               \
    e0_ = e0_ * (2.0f - d0_ * e0_); e1_ = e1_ * (2.0f - d1_ * e1_);                               \
    e2_ = e2_ * (2.0f - d2_ * e2_); e3_ = e3_ * (2.0f - d3_ * e3_);                               \
    (o0) = (z0) * e0_; (o1) = (z1) * e1_; (o2) = (z2) * e2_; (o3) = (z3) * e3_;                   \
} while (0)

void main(tensor gate, tensor sa, tensor swb) {
    const int5 start = get_index_space_offset();
    const int5 end = start + get_index_space_size();
    const int tiles = get_dim_size(gate, 0) / 128;  // 17408 / 128 = 136 (46 + 46 + 44, all even!)
    const int t1 = ((tiles + 2) / 3 + 1) & ~1;      // 46
    const int t2 = t1 * 2;                          // 92

    for (int item = start[0]; item < end[0]; ++item) {
        const int row = item / 3;
        const int part = item - row * 3;
        const int tb = (part == 0) ? 0 : (part == 1 ? t1 : t2);
        const int te = (part == 0) ? t1 : (part == 1 ? t2 : tiles);
        int5 sc = {0, row, 0, 0, 0};
        const float64 a = v_broadcast_element_f32(v_f32_ld_tnsr_b(sc, sa), 0);
        for (int tile = tb; tile < te; tile += 2) {
            const int c0 = tile * 128, c1 = c0 + 128;
            int5 rc0 = {c0, row, 0, 0, 0}, rc1 = {c1, row, 0, 0, 0};
            int5 w0a = {c0, 0, 0, 0, 0}, w0b = {c0 + 64, 0, 0, 0, 0};
            int5 w1a = {c1, 0, 0, 0, 0}, w1b = {c1 + 64, 0, 0, 0, 0};
            const float64_pair_t g0 = v_convert_bf16_to_f32_all_b(v_bf16_ld_tnsr_b(rc0, gate));
            const float64_pair_t g1 = v_convert_bf16_to_f32_all_b(v_bf16_ld_tnsr_b(rc1, gate));
            const float64 k0a = v_f32_ld_tnsr_b(w0a, swb) * a, k0b = v_f32_ld_tnsr_b(w0b, swb) * a;
            const float64 k1a = v_f32_ld_tnsr_b(w1a, swb) * a, k1b = v_f32_ld_tnsr_b(w1b, swb) * a;
            float64_pair_t d0 = {g0.v1 * k0a, g0.v2 * k0b};
            float64_pair_t d1 = {g1.v1 * k1a, g1.v2 * k1b};
            const float64_pair_t r0 = v_convert_bf16_to_f32_all_b(v_convert_f32_to_bf16_all_b(d0));
            const float64_pair_t r1 = v_convert_bf16_to_f32_all_b(v_convert_f32_to_bf16_all_b(d1));
            float64_pair_t o0, o1;
            SILU4_F32(r0.v1, r0.v2, r1.v1, r1.v2, o0.v1, o0.v2, o1.v1, o1.v2);
            v_bf16_st_tnsr(rc0, gate, v_convert_f32_to_bf16_all_b(o0));
            v_bf16_st_tnsr(rc1, gate, v_convert_f32_to_bf16_all_b(o1));
        }
    }
}
