// SPDX-License-Identifier: Apache-2.0
// g2e direct MME+SRAM kernel: fused dequant(qkv) + st_seq stash + causal width-4 conv + SiLU -> X_sram, V_sram, st_x.
// Runs in parallel with MME(z) + MME(ba) on all 24 TPC cores!
// Flat 1D index space [0 .. 80 * B) (640 work items across 24 TPCs = 26-27 items/core, 98.8% load balanced!).
#define QK 2048
#define TOKENS 16

static float64_pair_t bf16_to_f32_linear(bfloat128 input)
{
    bfloat128_pair_t unpacked;
    unpacked.v1 = v_bf16_unpack_b(
        input, ((e_group_0) << 8) | ((e_every_second_element) << 9) | ((e_lower_half_group) << 10), unpacked.v1);
    unpacked.v2 = v_bf16_unpack_b(
        input, ((e_group_1) << 8) | ((e_every_second_element) << 9) | ((e_lower_half_group) << 10), unpacked.v2);
    const bfloat128 first_groups = unpacked.v1;
    unpacked.v1 = v_bf16_mov_dual_group_b(unpacked.v2, 0xFFFFFFFF, 0, 1, MkWr(1, 1), unpacked.v1);
    unpacked.v1 = v_bf16_mov_dual_group_b(first_groups, 0xFFFFFFFF, 1, 2, MkWr(1, 1), unpacked.v1);
    unpacked.v1 = v_bf16_mov_dual_group_b(unpacked.v2, 0xFFFFFFFF, 1, 3, MkWr(1, 1), unpacked.v1);
    unpacked.v2 = v_bf16_mov_dual_group_b(first_groups, 0xFFFFFFFF, 2, 0, MkWr(1, 1), unpacked.v2);
    unpacked.v2 = v_bf16_mov_dual_group_b(unpacked.v2, 0xFFFFFFFF, 2, 1, MkWr(1, 1), unpacked.v2);
    unpacked.v2 = v_bf16_mov_dual_group_b(first_groups, 0xFFFFFFFF, 3, 2, MkWr(1, 1), unpacked.v2);
    const float64_pair_t first = v_convert_bf16_to_f32_all_b(unpacked.v1);
    const float64_pair_t second = v_convert_bf16_to_f32_all_b(unpacked.v2);
    float64_pair_t result;
    result.v1 = first.v1;
    result.v2 = second.v1;
    return result;
}

static inline float64 fast_silu_f32(float64 x) {
    const float64 xc = v_f32_min_b(v_f32_max_b(x, -87.0f), 87.0f);
    const float64 t = v_f32_mac_b(xc, -1.4426950408889634f, (float64)127.0f);
    const int64 n = v_convert_f32_to_i32_b(t, SW_RHNE);
    const float64 f = t - v_convert_i32_to_f32_b(n, 0);
    float64 p = 0.0096181291f;
    p = v_f32_mac_b(p, f, 0.0555041087f);
    p = v_f32_mac_b(p, f, 0.2402265070f);
    p = v_f32_mac_b(p, f, 0.6931471806f);
    p = v_f32_mac_b(p, f, 1.0f);
    const int64 bits = n << 23;
    const float64 d = 1.0f + p * as_float64(bits);
    float64 r = as_float64((int64)0x7ef311c3 - as_int64(d));
    r = r * (2.0f - d * r);
    r = r * (2.0f - d * r);
    r = r * (2.0f - d * r);
    return x * r;
}

#define LOAD_CONV_IN(B_IDX, ROW_K, OUT_V) do {     \
    int5 hc_ = {c, (ROW_K), 0, 0, 0};              \
    if ((B_IDX) == 0) (OUT_V) = v_bf16_ld_tnsr_b(hc_, ci0); \
    else if ((B_IDX) == 1) (OUT_V) = v_bf16_ld_tnsr_b(hc_, ci1); \
    else if ((B_IDX) == 2) (OUT_V) = v_bf16_ld_tnsr_b(hc_, ci2); \
    else if ((B_IDX) == 3) (OUT_V) = v_bf16_ld_tnsr_b(hc_, ci3); \
    else if ((B_IDX) == 4) (OUT_V) = v_bf16_ld_tnsr_b(hc_, ci4); \
    else if ((B_IDX) == 5) (OUT_V) = v_bf16_ld_tnsr_b(hc_, ci5); \
    else if ((B_IDX) == 6) (OUT_V) = v_bf16_ld_tnsr_b(hc_, ci6); \
    else (OUT_V) = v_bf16_ld_tnsr_b(hc_, ci7);     \
} while (0)

void main(tensor qkv_raw, tensor sa, tensor swb, tensor conv_w,
          tensor X_sram, tensor V_sram, tensor st_x, tensor st_seq,
          tensor ci0, tensor ci1, tensor ci2, tensor ci3,
          tensor ci4, tensor ci5, tensor ci6, tensor ci7)
{
    const int5 start = get_index_space_offset();
    const int5 end = start + get_index_space_size();
    const int B = get_dim_size(X_sram, 3);
    int prev_tile = -1;
    float64 sw1 = 0, sw2 = 0;
    float64_pair_t w0, w1, w2, w3;

    for (int idx = start[0]; idx < end[0]; ++idx) {
        const int tile = (B == 8) ? (idx >> 3) : idx;
        const int b = (B == 8) ? (idx & 7) : 0;
        const int c = tile * 128;
        if (tile != prev_tile) {
            prev_tile = tile;
            int5 wc = {c, 0, 0, 0, 0};
            sw1 = v_f32_ld_tnsr_b(wc, swb);
            wc[0] = c + 64;
            sw2 = v_f32_ld_tnsr_b(wc, swb);
            wc[0] = c;
            w0 = bf16_to_f32_linear(v_bf16_ld_tnsr_b(wc, conv_w));
            wc[1] = 1;
            w1 = bf16_to_f32_linear(v_bf16_ld_tnsr_b(wc, conv_w));
            wc[1] = 2;
            w2 = bf16_to_f32_linear(v_bf16_ld_tnsr_b(wc, conv_w));
            wc[1] = 3;
            w3 = bf16_to_f32_linear(v_bf16_ld_tnsr_b(wc, conv_w));
        }

        bfloat128 h0, h1, h2;
        LOAD_CONV_IN(b, 0, h0);
        LOAD_CONV_IN(b, 1, h1);
        LOAD_CONV_IN(b, 2, h2);
        int5 sqc = {c, 0, b, 0, 0};
        v_bf16_st_tnsr(sqc, st_seq, h0);
        sqc[1] = 1;
        v_bf16_st_tnsr(sqc, st_seq, h1);
        sqc[1] = 2;
        v_bf16_st_tnsr(sqc, st_seq, h2);

        float64_pair_t s0 = bf16_to_f32_linear(h0);
        float64_pair_t s1 = bf16_to_f32_linear(h1);
        float64_pair_t s2 = bf16_to_f32_linear(h2);

        const int base_row = b * TOKENS;
        int5 rc = {c, base_row, 0, 0, 0};
        int5 ac = {0, base_row, 0, 0, 0};
        bfloat128 next_raw = v_bf16_ld_tnsr_b(rc, qkv_raw);
        float64 next_a = v_broadcast_element_f32(v_f32_ld_tnsr_b(ac, sa), 0);

        const int is_qk = (c < 2 * QK);
        const int is_k = (c >= QK);
        const int head = (c - (is_k ? QK : 0)) >> 7;
        const int row_base = is_k ? 0 : TOKENS;
        const int v_col = c - 2 * QK;

        #pragma loop_unroll(2)
        for (int t = 0; t < TOKENS; ++t) {
            const bfloat128 raw_now = next_raw;
            const float64 a_now = next_a;
            const int tn = t + 1 < TOKENS ? t + 1 : t;
            rc[1] = base_row + tn;
            ac[1] = base_row + tn;
            next_raw = v_bf16_ld_tnsr_b(rc, qkv_raw);
            next_a = v_broadcast_element_f32(v_f32_ld_tnsr_b(ac, sa), 0);

            float64_pair_t dq = v_convert_bf16_to_f32_all_b(raw_now);
            dq.v1 = dq.v1 * (sw1 * a_now);
            dq.v2 = dq.v2 * (sw2 * a_now);
            const bfloat128 dq_bf = v_convert_f32_to_bf16_all_b(dq);
            sqc[1] = t + 3;
            v_bf16_st_tnsr(sqc, st_seq, dq_bf);

            const float64_pair_t s3 = bf16_to_f32_linear(dq_bf);
            float64_pair_t o;
            o.v1 = s0.v1 * w0.v1 + s1.v1 * w1.v1 + s2.v1 * w2.v1 + s3.v1 * w3.v1;
            o.v2 = s0.v2 * w0.v2 + s1.v2 * w1.v2 + s2.v2 * w2.v2 + s3.v2 * w3.v2;
            o.v1 = fast_silu_f32(o.v1);
            o.v2 = fast_silu_f32(o.v2);
            if (is_qk) {
                int5 xc = {0, row_base + t, head, b, 0};
                v_f32_st_tnsr(xc, X_sram, o.v1);
                v_f32_st_tnsr(xc, st_x, o.v1);
                xc[0] = 64;
                v_f32_st_tnsr(xc, X_sram, o.v2);
                v_f32_st_tnsr(xc, st_x, o.v2);
            } else {
                int5 vc = {v_col, t, b, 0, 0};
                v_f32_st_tnsr(vc, V_sram, o.v1);
                vc[0] = v_col + 64;
                v_f32_st_tnsr(vc, V_sram, o.v2);
            }
            s0 = s1; s1 = s2; s2 = s3;
        }
    }
}
