// SPDX-License-Identifier: Apache-2.0
// g2e: input preparation of the speculative-verification GDN core (WY form), one launch per layer:
//   y[t, c] = silu(sum_k w[c, k] * seq[t + k, c])        (width-4 causal conv, FP32 math, taps in order 0..3)
// stored the way the WY core and its MME products want it. Channels [0, QK) are q, [QK, 2 QK) k, the rest v
// (QK = 16 heads * 128):
//   X[h, t, d]     = k[t, h * 128 + d]                    X FP32 [16, 2T, 128] (rows 0..T-1: k, T..2T-1: q)
//   X[h, T + t, d] = q[t, h * 128 + d]
//   V[t, c - 2 QK] = v[t, c]                              V FP32 [T, C - 2 QK]
// Tensors (dim 0 fastest): seq [C, T + 3] bf16 (conv history rows, then the T new rows), w [C, 4] bf16,
// X [128, 2T, 16] f32, V [C - 2 QK, T] f32. Index space [C / 128].
#define QK 2048

// bf16x128 -> two fp32x64 in linear channel order [0, 64) and [64, 128) (as gdn_packed_decode)
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

void main(tensor seq, tensor w, tensor X, tensor V)
{
    const int5 start = get_index_space_offset();
    const int5 end = start + get_index_space_size();
    const int tokens = get_dim_size(seq, 1) - 3;
    for (int tile = start[0]; tile < end[0]; ++tile) {
        const int c = tile * 128;
        int5 wc = {c, 0, 0, 0, 0};
        const float64_pair_t w0 = bf16_to_f32_linear(v_bf16_ld_tnsr_b(wc, w));
        wc[1] = 1;
        const float64_pair_t w1 = bf16_to_f32_linear(v_bf16_ld_tnsr_b(wc, w));
        wc[1] = 2;
        const float64_pair_t w2 = bf16_to_f32_linear(v_bf16_ld_tnsr_b(wc, w));
        wc[1] = 3;
        const float64_pair_t w3 = bf16_to_f32_linear(v_bf16_ld_tnsr_b(wc, w));
#ifdef BATCHED
        for (int b = start[1]; b < end[1]; ++b) {
        int5 sc = {c, 0, b, 0, 0};
#else
        const int b = 0;
        int5 sc = {c, 0, 0, 0, 0};
#endif
        float64_pair_t s0 = bf16_to_f32_linear(v_bf16_ld_tnsr_b(sc, seq));
        sc[1] = 1;
        float64_pair_t s1 = bf16_to_f32_linear(v_bf16_ld_tnsr_b(sc, seq));
        sc[1] = 2;
        float64_pair_t s2 = bf16_to_f32_linear(v_bf16_ld_tnsr_b(sc, seq));
        sc[1] = 3;
        bfloat128 next = v_bf16_ld_tnsr_b(sc, seq);
        for (int t = 0; t < tokens; ++t) {
            const bfloat128 xv = next;
            sc[1] = t + 4 < tokens + 3 ? t + 4 : t + 3;
            next = v_bf16_ld_tnsr_b(sc, seq);
            const float64_pair_t s3 = bf16_to_f32_linear(xv);
            float64_pair_t o;
            o.v1 = s0.v1 * w0.v1;
            o.v1 = o.v1 + s1.v1 * w1.v1;
            o.v1 = o.v1 + s2.v1 * w2.v1;
            o.v1 = o.v1 + s3.v1 * w3.v1;
            o.v2 = s0.v2 * w0.v2;
            o.v2 = o.v2 + s1.v2 * w1.v2;
            o.v2 = o.v2 + s2.v2 * w2.v2;
            o.v2 = o.v2 + s3.v2 * w3.v2;
            o.v1 = o.v1 * v_sigmoid_f32(o.v1);
            o.v2 = o.v2 * v_sigmoid_f32(o.v2);
            if (c < 2 * QK) {
                const int is_k = c >= QK;
                const int head = (c - (is_k ? QK : 0)) / 128;
                int5 xc = {0, is_k ? t : tokens + t, head, b, 0};
                v_f32_st_tnsr(xc, X, o.v1);
                xc[0] = 64;
                v_f32_st_tnsr(xc, X, o.v2);
            } else {
                int5 vc = {c - 2 * QK, t, b, 0, 0};
                v_f32_st_tnsr(vc, V, o.v1);
                vc[0] += 64;
                v_f32_st_tnsr(vc, V, o.v2);
            }
            s0 = s1; s1 = s2; s2 = s3;
        }
#ifdef BATCHED
        }
#endif
    }
}
