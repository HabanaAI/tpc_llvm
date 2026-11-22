// SPDX-License-Identifier: Apache-2.0
// DSpine fused QKV prep (100% LUT-free, zero VLM arrays, 100% 24-TPC saturated even at B=1!):
// Replaces ~34 Synapse nodes per layer x 5 layers = ~170 nodes:
//   split(qkv) + q_norm + rope_full(q) + transpose + scale -> qsc_all [128, G*16, B*KV] BF16
//   k_norm + rope_full(k) + transpose                      -> kb_all  [128, 16,   B*KV] BF16
//   transpose(v)                                           -> vb_all  [128, 16,   B*KV] BF16
// Tensors (dim 0 fastest):
//   0 qkv      BF16 [(NH+2*KV)*128, BT]  packed QKV projection output (NH=32, KV=8, D=128 -> 6144)
//   1 q_norm_w BF16 [128]
//   2 k_norm_w BF16 [128]
//   3 cos_in   F32  [64, BT]
//   4 sin_in   F32  [64, BT]
//   5 qsc_out  BF16 [128, G*16, B*KV]
//   6 kb_out   BF16 [128, 16,   B*KV]
//   7 vb_out   BF16 [128, 16,   B*KV]
// Scalar params (SRF 0..1): float eps, float q_scale (1/sqrt(128)).
// Index space: dim 0 = item [0..BT*KV) -> row = item / KV, h = item % KV.

#include "norm_math_gaudi2.h"

static inline float64_pair_t bf16_to_f32_linear(bfloat128 input) {
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

static inline float64 bf16_round_f32(float64 x) {
    float64_pair_t p = {x, x};
    return v_convert_bf16_to_f32_all_b(v_convert_f32_to_bf16_all_b(p)).v1;
}

static inline bfloat128 norm_rope_head(bfloat128 raw, float64 w1, float64 w2,
                                       float64 cv, float64 sv, float eps, float scale, int do_scale) {
    float128 sq = {0};
    sq = v_bf16_mac_acc32_b(raw, raw, sq, (e_no_negation) << 1);
    const float64 rrms = positive_rsqrt(row_sum(sq.v1 + sq.v2) * (1.0f / 128.0f) + eps);
    const float64_pair_t xf = bf16_to_f32_linear(raw);
    // Match BF16 boundary of rms_norm_ex_fwd_bf16 before NeoX RoPE
    const float64 n1 = bf16_round_f32((xf.v1 * rrms) * w1);
    const float64 n2 = bf16_round_f32((xf.v2 * rrms) * w2);
    float64 o1 = n1 * cv - n2 * sv;
    float64 o2 = n2 * cv + n1 * sv;
    if (do_scale) {
        o1 = bf16_round_f32(o1) * scale;
        o2 = bf16_round_f32(o2) * scale;
    }
    return f32_pair_to_bf16_linear_fast(o1, o2);
}

void main(tensor qkv, tensor q_norm_w, tensor k_norm_w, tensor cos_in, tensor sin_in,
          tensor qsc_out, tensor kb_out, tensor vb_out, float eps, float q_scale) {
    const int5 start = get_index_space_offset();
    const int5 end = start + get_index_space_size();
    const int BT = get_dim_size(qkv, 1);
    const int B = BT >> 4;
    const int BKV = get_dim_size(kb_out, 2);
    const int KV = BKV / B;
    const int G = get_dim_size(qsc_out, 1) >> 4;
    const int NH = KV * G;

    int5 wc = {0, 0, 0, 0, 0};
    const float64_pair_t qw = bf16_to_f32_linear(v_bf16_ld_tnsr_b(wc, q_norm_w));
    const float64_pair_t kw = bf16_to_f32_linear(v_bf16_ld_tnsr_b(wc, k_norm_w));

    for (int item = start[0]; item < end[0]; ++item) {
        const int row = item / KV;
        const int h = item - row * KV;
        const int b = row >> 4;
        const int t = row & 15;
        const int bkv = b * KV + h;

        int5 cc = {0, row, 0, 0, 0};
        const float64 cv = v_f32_ld_tnsr_b(cc, cos_in);
        const float64 sv = v_f32_ld_tnsr_b(cc, sin_in);

        // G query heads for this KV head -> qsc_out[0..127, g*16 + t, bkv]
        for (int g = 0; g < G; ++g) {
            int5 qc = {(h * G + g) * 128, row, 0, 0, 0};
            const bfloat128 qr = v_bf16_ld_tnsr_b(qc, qkv);
            const bfloat128 qo = norm_rope_head(qr, qw.v1, qw.v2, cv, sv, eps, q_scale, 1);
            int5 qoc = {0, g * 16 + t, bkv, 0, 0};
            v_bf16_st_tnsr(qoc, qsc_out, qo);
        }

        // K head -> kb_out[0..127, t, bkv]
        int5 kc = {(NH + h) * 128, row, 0, 0, 0};
        const bfloat128 kr = v_bf16_ld_tnsr_b(kc, qkv);
        const bfloat128 ko = norm_rope_head(kr, kw.v1, kw.v2, cv, sv, eps, 1.0f, 0);
        int5 koc = {0, t, bkv, 0, 0};
        v_bf16_st_tnsr(koc, kb_out, ko);

        // V head -> vb_out[0..127, t, bkv]
        int5 vc = {(NH + KV + h) * 128, row, 0, 0, 0};
        v_bf16_st_tnsr(koc, vb_out, v_bf16_ld_tnsr_b(vc, qkv));
    }
}
