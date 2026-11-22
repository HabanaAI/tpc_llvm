// SPDX-License-Identifier: Apache-2.0
// DSpine fused Code Bridge Stage C (100% LUT-free, 4-way VPU-interleaved):
//   if (row & 15) == 0: message[row] = 0
//   else:               message[row, r] = bf16(sigmoid(q[row, r] + s[row-1, r] + gate_b[r]) * v[row-1, r])
// Tensors (dim 0 fastest):
//   0 packed  BF16 [3*R, BT]   [q | s | v] concatenated along dim 0
//   1 gate_b  BF16 [R]         bridge.gate.bias
//   2 message BF16 [R, BT]
// Index space: dim 0 = row [0..BT).

#define SIGMOID4_F32(z0, z1, z2, z3, o0, o1, o2, o3) do {                                         \
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
    (o0) = e0_; (o1) = e1_; (o2) = e2_; (o3) = e3_;                                               \
} while (0)

void main(tensor packed, tensor gate_b, tensor message) {
    const int5 start = get_index_space_offset();
    const int5 end = start + get_index_space_size();
    const int R = get_dim_size(message, 0);
    const int tiles = R / 128;  // R=512 -> 4 (multiple of 2)
    const bfloat128 zero_bf = 0;

    for (int row = start[0]; row < end[0]; ++row) {
        if ((row & 15) == 0) {
            for (int tile = 0; tile < tiles; ++tile) {
                int5 oc = {tile * 128, row, 0, 0, 0};
                v_bf16_st_tnsr(oc, message, zero_bf);
            }
            continue;
        }

        const int prev = row - 1;
        for (int tile = 0; tile < tiles; tile += 2) {
            const int c0 = tile * 128, c1 = c0 + 128;
            int5 qc0 = {c0,         row,  0, 0, 0}, qc1 = {c1,         row,  0, 0, 0};
            int5 sc0 = {R + c0,     prev, 0, 0, 0}, sc1 = {R + c1,     prev, 0, 0, 0};
            int5 vc0 = {2 * R + c0, prev, 0, 0, 0}, vc1 = {2 * R + c1, prev, 0, 0, 0};
            int5 bc0 = {c0,         0,    0, 0, 0}, bc1 = {c1,         0,    0, 0, 0};

            const float64_pair_t q0 = v_convert_bf16_to_f32_all_b(v_bf16_ld_tnsr_b(qc0, packed));
            const float64_pair_t q1 = v_convert_bf16_to_f32_all_b(v_bf16_ld_tnsr_b(qc1, packed));
            const float64_pair_t s0 = v_convert_bf16_to_f32_all_b(v_bf16_ld_tnsr_b(sc0, packed));
            const float64_pair_t s1 = v_convert_bf16_to_f32_all_b(v_bf16_ld_tnsr_b(sc1, packed));
            const float64_pair_t v0 = v_convert_bf16_to_f32_all_b(v_bf16_ld_tnsr_b(vc0, packed));
            const float64_pair_t v1 = v_convert_bf16_to_f32_all_b(v_bf16_ld_tnsr_b(vc1, packed));
            const float64_pair_t b0 = v_convert_bf16_to_f32_all_b(v_bf16_ld_tnsr_b(bc0, gate_b));
            const float64_pair_t b1 = v_convert_bf16_to_f32_all_b(v_bf16_ld_tnsr_b(bc1, gate_b));

            const float64 z0a = (q0.v1 + s0.v1) + b0.v1;
            const float64 z0b = (q0.v2 + s0.v2) + b0.v2;
            const float64 z1a = (q1.v1 + s1.v1) + b1.v1;
            const float64 z1b = (q1.v2 + s1.v2) + b1.v2;

            float64 g0a, g0b, g1a, g1b;
            SIGMOID4_F32(z0a, z0b, z1a, z1b, g0a, g0b, g1a, g1b);

            float64_pair_t o0 = {g0a * v0.v1, g0b * v0.v2};
            float64_pair_t o1 = {g1a * v1.v1, g1b * v1.v2};
            v_bf16_st_tnsr(qc0, message, v_convert_f32_to_bf16_all_b(o0));
            v_bf16_st_tnsr(qc1, message, v_convert_f32_to_bf16_all_b(o1));
        }
    }
}
