// SPDX-License-Identifier: Apache-2.0
// g2e Direct MME+SRAM Attention apre QKV prep (D=256, NH=24, KV=4, G=6, T=16, B=8, R=128):
//   384 = 128 x 3 balanced slices across 24 TPCs (16 slices/TPC, zero tail wave!),
//   caches cos/sin + scale per row r across consecutive chunks, zero redundant bf16 round-trips!
// Index space: dim 1 = idx in [0..384) where r = idx / 3, chunk = idx % 3. Scalar SRF 0: float eps.
#include "norm_math_gaudi2.h"

#ifdef KV_FP8
static inline minifloat256 f32_pair_to_f8(float64 v1, float64 v2) {
    minifloat256 packed = 0;
    packed = v_convert_f32_to_f8_b(v1, 0, SW_CLIP_FP, packed);
    packed = v_convert_f32_to_f8_b(v2, 2, SW_CLIP_FP, packed);
    const minifloat256 sparse = packed;
    packed = v_f8_pack_b(sparse, SW_GROUP_0 | SW_STRIDE_2, (minifloat256)0);
    packed = v_f8_pack_b(sparse, SW_GROUP_1 | SW_STRIDE_2, packed);
    return v_f8_mov_dual_group_pack_b(packed, SW_PACK21, (minifloat256)0);
}
static inline void st_256_f8(int5 c0, tensor dst, float128 p0, float128 p1) {
    const minifloat256 q0 = f32_pair_to_f8(p0.v1, p0.v2);
    const minifloat256 q1 = f32_pair_to_f8(p1.v1, p1.v2);
    minifloat256 pk = v_f8_mov_dual_group_b(q1, 0xffffffff, 0, 2, MkWr(1, 1), q0);
    pk = v_f8_mov_dual_group_b(q1, 0xffffffff, 1, 3, MkWr(1, 1), pk);
    v_f8_st_tnsr(c0, dst, pk);
}
#endif

static inline float64_pair_t split_rope_32(float64 raw, float64 bg, uint64 l8, uchar256 shuf_eo, uchar256 shuf_x4) {
    const int swap_g = SW_GROUP0_EN | SW_GROUP1_EN | SW_DUAL_GROUP0_EN | SW_DUAL_GROUP1_EN |
                       SW_DUAL_GROUP2_EN | SW_DUAL_GROUP3_EN;
    const float64 t = v_f32_shuffle_b(raw, shuf_eo, 0, raw);
    const float64 t_sw = v_f32_mov_group_b(t, 0xffffffff, swap_g, t);
    const float64 t_sw4 = v_f32_shuffle_b(t_sw, shuf_x4, 0, t_sw);
    const float64 ev = v_f32_sel_less_u32_b(l8, 4, t, t_sw4);
    const float64 od = v_f32_sel_less_u32_b(l8, 4, t_sw4, t);
    const float64 ev_g1 = v_f32_mov_group_b(ev, 0xffffffff, swap_g, ev);
    float64 re = v_f32_mov_dual_group_all_b(ev, 0xffffffff, 0, 0, 0, 0,
                                            SW_WR_LOWER_GROUP0 | SW_WR_LOWER_GROUP1, bg);
    re = v_f32_mov_dual_group_all_b(ev_g1, 0xffffffff, 1, 1, 0, 0,
                                    SW_WR_UPPER_GROUP0 | SW_WR_UPPER_GROUP1, re);
    const float64 od_g0 = v_f32_mov_group_b(od, 0xffffffff, swap_g, od);
    float64 ro = v_f32_mov_dual_group_all_b(od_g0, 0xffffffff, 0, 0, 0, 0,
                                            SW_WR_LOWER_GROUP0 | SW_WR_LOWER_GROUP1, bg);
    ro = v_f32_mov_dual_group_all_b(od, 0xffffffff, 1, 1, 0, 0,
                                    SW_WR_UPPER_GROUP0 | SW_WR_UPPER_GROUP1, ro);
    float64_pair_t res;
    res.v1 = re;
    res.v2 = ro;
    return res;
}

static inline float128 load_dq_128_fast(tensor qkv_raw, tensor swb, int col, int r, float64 a) {
    int5 ic = {col, r, 0, 0, 0};
    int5 wc0 = {col, 0, 0, 0, 0}, wc1 = {col + 64, 0, 0, 0, 0};
    float128 f = v_convert_bf16_to_f32_all_b(v_bf16_ld_tnsr_b(ic, qkv_raw));
    f.v1 *= (v_f32_ld_tnsr_b(wc0, swb) * a);
    f.v2 *= (v_f32_ld_tnsr_b(wc1, swb) * a);
    return f;
}

void main(tensor qkv_raw, tensor sa, tensor swb, tensor q_norm, tensor k_norm,
          tensor rope_pos, tensor cos_tab, tensor sin_tab,
          tensor q_out, tensor gate_out, tensor k_sram, tensor v_sram, float eps)
{
    const int5 start = get_index_space_offset();
    const int5 end = start + get_index_space_size();
    const int all_dg = SW_WR_LOWER_GROUP0 | SW_WR_UPPER_GROUP0 | SW_WR_LOWER_GROUP1 | SW_WR_UPPER_GROUP1 |
                       SW_WR_LOWER_GROUP2 | SW_WR_UPPER_GROUP2 | SW_WR_LOWER_GROUP3 | SW_WR_UPPER_GROUP3;

    int5 wc0 = {0, 0, 0, 0, 0}, wc1 = {128, 0, 0, 0, 0};
    float128 qw0 = v_convert_bf16_to_f32_all_b(v_bf16_ld_tnsr_b(wc0, q_norm));
    float128 qw1 = v_convert_bf16_to_f32_all_b(v_bf16_ld_tnsr_b(wc1, q_norm));
    qw0.v1 += 1.0f; qw0.v2 += 1.0f;
    qw1.v1 += 1.0f; qw1.v2 += 1.0f;
    float128 kw0 = v_convert_bf16_to_f32_all_b(v_bf16_ld_tnsr_b(wc0, k_norm));
    float128 kw1 = v_convert_bf16_to_f32_all_b(v_bf16_ld_tnsr_b(wc1, k_norm));
    kw0.v1 += 1.0f; kw0.v2 += 1.0f;
    kw1.v1 += 1.0f; kw1.v2 += 1.0f;

    const uint64 lanes = read_lane_id_4b_b();
    const uint64 l8 = lanes & 7;
    const uint64 g1_bit = (lanes & 8) << 2;
    const uint64 eo_idx = v_u32_sel_less_u32_b(l8, 4, l8 << 1, ((l8 - 4) << 1) | 1);
    const uint64 shuf_eo_u = (eo_idx | g1_bit | 0x80) * 0x01010101;
    const uint64 shuf_x4_u = ((l8 ^ 4) | g1_bit | 0x80) * 0x01010101;
    const uchar256 shuf_eo = *((uchar256*)&shuf_eo_u);
    const uchar256 shuf_x4 = *((uchar256*)&shuf_x4_u);
    const float64 v_one = 1.0f;
    const float64 v_zero = 0.0f;

    int prev_r = -1;
    float64 cos_e = 1.0f, cos_o = 1.0f, sin_e = 0.0f, sin_o = 0.0f, a = 1.0f;

    for (int idx = start[1]; idx < end[1]; ++idx) {
        const int r = idx / 3;
        const int c = idx - r * 3;
        const int b = r >> 4;
        const int t = r & 15;
        if (r != prev_r) {
            int5 pc = {r, 0, 0, 0, 0};
            const int pos = s_i32_ld_g((__global__ int*)gen_addr(pc, rope_pos));
            int5 rc = {0, pos, 0, 0, 0};
            const float64 cos_raw = v_f32_ld_tnsr_b(rc, cos_tab);
            const float64 sin_raw = v_f32_ld_tnsr_b(rc, sin_tab);
            const float64_pair_t cos_p = split_rope_32(cos_raw, v_one, l8, shuf_eo, shuf_x4);
            const float64_pair_t sin_p = split_rope_32(sin_raw, v_zero, l8, shuf_eo, shuf_x4);
            cos_e = cos_p.v1;
            cos_o = cos_p.v2;
            sin_e = v_f32_sel_less_u32_b(lanes, 16, -sin_p.v1, sin_p.v1);
            sin_o = v_f32_sel_less_u32_b(lanes, 16, -sin_p.v2, sin_p.v2);
            int5 sc = {0, r, 0, 0, 0};
            a = v_broadcast_element_f32(v_f32_ld_tnsr_b(sc, sa), 0);
            prev_r = r;
        }

        const int nh0 = (c == 0) ? 0 : ((c == 1) ? 9 : 18);
        const int nh1 = (c == 0) ? 9 : ((c == 1) ? 18 : 24);
        for (int nh = nh0; nh < nh1; ++nh) {
            float128 f0 = load_dq_128_fast(qkv_raw, swb, nh * 512, r, a);
            float128 f1 = load_dq_128_fast(qkv_raw, swb, nh * 512 + 128, r, a);
            float64 sq = v_f32_mac_b(f0.v1, f0.v1, f0.v2 * f0.v2);
            sq = v_f32_mac_b(f1.v1, f1.v1, sq);
            sq = v_f32_mac_b(f1.v2, f1.v2, sq);
            const float64 inv = positive_rsqrt(row_sum(sq) * (1.0f / 256.0f) + eps);
            f0.v1 = f0.v1 * inv * qw0.v1;
            f0.v2 = f0.v2 * inv * qw0.v2;
            f1.v1 = f1.v1 * inv * qw1.v1;
            f1.v2 = f1.v2 * inv * qw1.v2;
            const float64 p1 = v_f32_mov_dual_group_all_b(f0.v1, 0xffffffff, 1, 0, 2, 3, all_dg, f0.v1);
            const float64 p2 = v_f32_mov_dual_group_all_b(f0.v2, 0xffffffff, 1, 0, 2, 3, all_dg, f0.v2);
            float128 r0;
            r0.v1 = v_f32_mac_b(p1, sin_e, f0.v1 * cos_e);
            r0.v2 = v_f32_mac_b(p2, sin_o, f0.v2 * cos_o);

            const int h_kv = nh / 6;
            const int g = nh - h_kv * 6;
            const int rem = g * 16 + t;
            int5 qc0 = {0, rem, h_kv, b, 0};
#ifdef KV_FP8
            st_256_f8(qc0, q_out, r0, f1);
#else
            int5 qc1 = {128, rem, h_kv, b, 0};
            v_bf16_st_tnsr(qc0, q_out, v_convert_f32_to_bf16_all_b(r0));
            v_bf16_st_tnsr(qc1, q_out, v_convert_f32_to_bf16_all_b(f1));
#endif

            const int gc = nh * 512 + 256;
            float128 g0 = load_dq_128_fast(qkv_raw, swb, gc, r, a);
            float128 g1 = load_dq_128_fast(qkv_raw, swb, gc + 128, r, a);
            int5 go0 = {nh * 256, r, 0, 0, 0}, go1 = {nh * 256 + 128, r, 0, 0, 0};
            v_bf16_st_tnsr(go0, gate_out, v_convert_f32_to_bf16_all_b(g0));
            v_bf16_st_tnsr(go1, gate_out, v_convert_f32_to_bf16_all_b(g1));
        }

        if (c == 2) {
            for (int h = 0; h < 4; ++h) {
                const int kb = 12288 + h * 256;
                float128 f0 = load_dq_128_fast(qkv_raw, swb, kb, r, a);
                float128 f1 = load_dq_128_fast(qkv_raw, swb, kb + 128, r, a);
                float64 sq = v_f32_mac_b(f0.v1, f0.v1, f0.v2 * f0.v2);
                sq = v_f32_mac_b(f1.v1, f1.v1, sq);
                sq = v_f32_mac_b(f1.v2, f1.v2, sq);
                const float64 inv = positive_rsqrt(row_sum(sq) * (1.0f / 256.0f) + eps);
                f0.v1 = f0.v1 * inv * kw0.v1;
                f0.v2 = f0.v2 * inv * kw0.v2;
                f1.v1 = f1.v1 * inv * kw1.v1;
                f1.v2 = f1.v2 * inv * kw1.v2;
                const float64 p1 = v_f32_mov_dual_group_all_b(f0.v1, 0xffffffff, 1, 0, 2, 3, all_dg, f0.v1);
                const float64 p2 = v_f32_mov_dual_group_all_b(f0.v2, 0xffffffff, 1, 0, 2, 3, all_dg, f0.v2);
                float128 r0;
                r0.v1 = v_f32_mac_b(p1, sin_e, f0.v1 * cos_e);
                r0.v2 = v_f32_mac_b(p2, sin_o, f0.v2 * cos_o);
                int5 kc0 = {0, t, h, b, 0};
#ifdef KV_FP8
                st_256_f8(kc0, k_sram, r0, f1);
#else
                int5 kc1 = {128, t, h, b, 0};
                v_bf16_st_tnsr(kc0, k_sram, v_convert_f32_to_bf16_all_b(r0));
                v_bf16_st_tnsr(kc1, k_sram, v_convert_f32_to_bf16_all_b(f1));
#endif
            }
            for (int h = 0; h < 4; ++h) {
                const int vb = 13312 + h * 256;
                float128 v0 = load_dq_128_fast(qkv_raw, swb, vb, r, a);
                float128 v1 = load_dq_128_fast(qkv_raw, swb, vb + 128, r, a);
                int5 vc0 = {0, t, h, b, 0};
#ifdef KV_FP8
                st_256_f8(vc0, v_sram, v0, v1);
#else
                int5 vc1 = {128, t, h, b, 0};
                v_bf16_st_tnsr(vc0, v_sram, v_convert_f32_to_bf16_all_b(v0));
                v_bf16_st_tnsr(vc1, v_sram, v_convert_f32_to_bf16_all_b(v1));
#endif
            }
        }
    }
}
