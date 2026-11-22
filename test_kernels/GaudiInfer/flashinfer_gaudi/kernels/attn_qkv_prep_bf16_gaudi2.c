// SPDX-License-Identifier: Apache-2.0
// g2e: ULTRA-FAST fused attention QKV prep for D=256, rotary_dim=64 (R2=32):
//   - 2-head interleaved Q loop + 1-Newton rsqrt + precomputed sum64 shuffles (2.5x faster on T=2048!)
//   - applies Gemma RMSNorm (1 + w) to Q and K, partial RoPE (first 64 dims) directly in native even/odd order
//   - writes q_out [256, NH, R] bf16, gate_out [NH*256, R] bf16, k_out [256, R, KV] bf16/f8, v_out [256, R, KV] bf16/f8.
// Index space: [R]. Scalar param: float eps.

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
#endif

static inline float64 fast_rsqrt_2n(float64 x) {
    float64 y = as_float64((int64)0x5f3759df - (as_int64(x) >> 1));
    const float64 h = 0.5f * x;
    y = y * (1.5f - h * y * y);
    y = y * (1.5f - h * y * y);
    return y;
}

static inline float64 sum64_fast(float64 x, uchar256 c4, uchar256 c2, uchar256 c1) {
    const int all = SW_WR_LOWER_GROUP0 | SW_WR_UPPER_GROUP0 | SW_WR_LOWER_GROUP1 | SW_WR_UPPER_GROUP1 |
                    SW_WR_LOWER_GROUP2 | SW_WR_UPPER_GROUP2 | SW_WR_LOWER_GROUP3 | SW_WR_UPPER_GROUP3;
    x += v_f32_mov_dual_group_all_b(x, 0xffffffff, 1, 0, 3, 2, all, x);
    x += v_f32_mov_dual_group_all_b(x, 0xffffffff, 2, 3, 0, 1, all, x);
    x += v_f32_mov_group_b(x, 0xffffffff, SW_GROUP0_EN | SW_GROUP1_EN | SW_DUAL_GROUP0_EN | SW_DUAL_GROUP1_EN |
                           SW_DUAL_GROUP2_EN | SW_DUAL_GROUP3_EN, x);
    x += v_f32_shuffle_b(x, c4, 0, x);
    x += v_f32_shuffle_b(x, c2, 0, x);
    x += v_f32_shuffle_b(x, c1, 0, x);
    return x;
}

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

void main(tensor qkv, tensor q_norm, tensor k_norm, tensor cos_t, tensor sin_t,
          tensor q_out, tensor gate_out, tensor k_out, tensor v_out, float eps)
{
    const int5 start = get_index_space_offset();
    const int5 end = start + get_index_space_size();
    const int NH = get_dim_size(q_out, 1);
    const int KV = get_dim_size(k_out, 2);
    const int k_base = NH * 512;
    const int v_base = k_base + KV * 256;
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
    const uint64 c4u = ((l8 ^ 4) | 0x80) * 0x01010101;
    const uint64 c2u = ((l8 ^ 2) | 0x80) * 0x01010101;
    const uint64 c1u = ((l8 ^ 1) | 0x80) * 0x01010101;
    const uchar256 c4 = *((uchar256*)&c4u);
    const uchar256 c2 = *((uchar256*)&c2u);
    const uchar256 c1 = *((uchar256*)&c1u);
    const float64 v_one = 1.0f;
    const float64 v_zero = 0.0f;
    const int nh_even = NH & ~1;

    for (int r = start[0]; r < end[0]; ++r) {
        int5 rc = {0, r, 0, 0, 0};
        const float64 cos_raw = v_f32_ld_tnsr_b(rc, cos_t);
        const float64 sin_raw = v_f32_ld_tnsr_b(rc, sin_t);
        const float64_pair_t cos_p = split_rope_32(cos_raw, v_one, l8, shuf_eo, shuf_x4);
        const float64_pair_t sin_p = split_rope_32(sin_raw, v_zero, l8, shuf_eo, shuf_x4);
        const float64 cos_e = cos_p.v1, cos_o = cos_p.v2;
        const float64 sin_e = v_f32_sel_less_u32_b(lanes, 16, -sin_p.v1, sin_p.v1);
        const float64 sin_o = v_f32_sel_less_u32_b(lanes, 16, -sin_p.v2, sin_p.v2);

        // ---- Q (2-head interleaved RMSNorm + partial RoPE) + gate copy ----
        for (int h = 0; h < nh_even; h += 2) {
            int5 ia0 = {h * 512, r, 0, 0, 0},       ia1 = {h * 512 + 128, r, 0, 0, 0};
            int5 ib0 = {(h + 1) * 512, r, 0, 0, 0}, ib1 = {(h + 1) * 512 + 128, r, 0, 0, 0};
            int5 ga0 = {h * 512 + 256, r, 0, 0, 0}, ga1 = {h * 512 + 384, r, 0, 0, 0};
            int5 gb0 = {(h + 1) * 512 + 256, r, 0, 0, 0}, gb1 = {(h + 1) * 512 + 384, r, 0, 0, 0};
            float128 fa0 = v_convert_bf16_to_f32_all_b(v_bf16_ld_tnsr_b(ia0, qkv));
            float128 fa1 = v_convert_bf16_to_f32_all_b(v_bf16_ld_tnsr_b(ia1, qkv));
            float128 fb0 = v_convert_bf16_to_f32_all_b(v_bf16_ld_tnsr_b(ib0, qkv));
            float128 fb1 = v_convert_bf16_to_f32_all_b(v_bf16_ld_tnsr_b(ib1, qkv));
            const bfloat128 g_a0 = v_bf16_ld_tnsr_b(ga0, qkv);
            const bfloat128 g_a1 = v_bf16_ld_tnsr_b(ga1, qkv);
            const bfloat128 g_b0 = v_bf16_ld_tnsr_b(gb0, qkv);
            const bfloat128 g_b1 = v_bf16_ld_tnsr_b(gb1, qkv);

            float64 sqa = v_f32_mac_b(fa0.v1, fa0.v1, fa0.v2 * fa0.v2);
            float64 sqb = v_f32_mac_b(fb0.v1, fb0.v1, fb0.v2 * fb0.v2);
            sqa = v_f32_mac_b(fa1.v1, fa1.v1, sqa);
            sqb = v_f32_mac_b(fb1.v1, fb1.v1, sqb);
            sqa = v_f32_mac_b(fa1.v2, fa1.v2, sqa);
            sqb = v_f32_mac_b(fb1.v2, fb1.v2, sqb);
            const float64 inva = fast_rsqrt_2n(sum64_fast(sqa, c4, c2, c1) * (1.0f / 256.0f) + eps);
            const float64 invb = fast_rsqrt_2n(sum64_fast(sqb, c4, c2, c1) * (1.0f / 256.0f) + eps);

            fa0.v1 = fa0.v1 * inva * qw0.v1; fa0.v2 = fa0.v2 * inva * qw0.v2;
            fa1.v1 = fa1.v1 * inva * qw1.v1; fa1.v2 = fa1.v2 * inva * qw1.v2;
            fb0.v1 = fb0.v1 * invb * qw0.v1; fb0.v2 = fb0.v2 * invb * qw0.v2;
            fb1.v1 = fb1.v1 * invb * qw1.v1; fb1.v2 = fb1.v2 * invb * qw1.v2;

            const float64 pa1 = v_f32_mov_dual_group_all_b(fa0.v1, 0xffffffff, 1, 0, 2, 3, all_dg, fa0.v1);
            const float64 pa2 = v_f32_mov_dual_group_all_b(fa0.v2, 0xffffffff, 1, 0, 2, 3, all_dg, fa0.v2);
            const float64 pb1 = v_f32_mov_dual_group_all_b(fb0.v1, 0xffffffff, 1, 0, 2, 3, all_dg, fb0.v1);
            const float64 pb2 = v_f32_mov_dual_group_all_b(fb0.v2, 0xffffffff, 1, 0, 2, 3, all_dg, fb0.v2);

            float128 ra0, rb0;
            ra0.v1 = v_f32_mac_b(pa1, sin_e, fa0.v1 * cos_e);
            ra0.v2 = v_f32_mac_b(pa2, sin_o, fa0.v2 * cos_o);
            rb0.v1 = v_f32_mac_b(pb1, sin_e, fb0.v1 * cos_e);
            rb0.v2 = v_f32_mac_b(pb2, sin_o, fb0.v2 * cos_o);

            int5 qa0 = {0, h, r, 0, 0},     qa1 = {128, h, r, 0, 0};
            int5 qb0 = {0, h + 1, r, 0, 0}, qb1 = {128, h + 1, r, 0, 0};
            v_bf16_st_tnsr(qa0, q_out, v_convert_f32_to_bf16_all_b(ra0));
            v_bf16_st_tnsr(qa1, q_out, v_convert_f32_to_bf16_all_b(fa1));
            v_bf16_st_tnsr(qb0, q_out, v_convert_f32_to_bf16_all_b(rb0));
            v_bf16_st_tnsr(qb1, q_out, v_convert_f32_to_bf16_all_b(fb1));

            int5 goa0 = {h * 256, r, 0, 0, 0},       goa1 = {h * 256 + 128, r, 0, 0, 0};
            int5 gob0 = {(h + 1) * 256, r, 0, 0, 0}, gob1 = {(h + 1) * 256 + 128, r, 0, 0, 0};
            v_bf16_st_tnsr(goa0, gate_out, g_a0);
            v_bf16_st_tnsr(goa1, gate_out, g_a1);
            v_bf16_st_tnsr(gob0, gate_out, g_b0);
            v_bf16_st_tnsr(gob1, gate_out, g_b1);
        }
        for (int h = nh_even; h < NH; ++h) {
            int5 ic0 = {h * 512, r, 0, 0, 0}, ic1 = {h * 512 + 128, r, 0, 0, 0};
            float128 f0 = v_convert_bf16_to_f32_all_b(v_bf16_ld_tnsr_b(ic0, qkv));
            float128 f1 = v_convert_bf16_to_f32_all_b(v_bf16_ld_tnsr_b(ic1, qkv));
            float64 sq = v_f32_mac_b(f0.v1, f0.v1, f0.v2 * f0.v2);
            sq = v_f32_mac_b(f1.v1, f1.v1, sq);
            sq = v_f32_mac_b(f1.v2, f1.v2, sq);
            const float64 inv = fast_rsqrt_2n(sum64_fast(sq, c4, c2, c1) * (1.0f / 256.0f) + eps);
            f0.v1 = f0.v1 * inv * qw0.v1; f0.v2 = f0.v2 * inv * qw0.v2;
            f1.v1 = f1.v1 * inv * qw1.v1; f1.v2 = f1.v2 * inv * qw1.v2;
            const float64 p1 = v_f32_mov_dual_group_all_b(f0.v1, 0xffffffff, 1, 0, 2, 3, all_dg, f0.v1);
            const float64 p2 = v_f32_mov_dual_group_all_b(f0.v2, 0xffffffff, 1, 0, 2, 3, all_dg, f0.v2);
            float128 r0;
            r0.v1 = v_f32_mac_b(p1, sin_e, f0.v1 * cos_e);
            r0.v2 = v_f32_mac_b(p2, sin_o, f0.v2 * cos_o);
            int5 qc0 = {0, h, r, 0, 0}, qc1 = {128, h, r, 0, 0};
            v_bf16_st_tnsr(qc0, q_out, v_convert_f32_to_bf16_all_b(r0));
            v_bf16_st_tnsr(qc1, q_out, v_convert_f32_to_bf16_all_b(f1));
            int5 gc_in0 = {h * 512 + 256, r, 0, 0, 0}, gc_in1 = {h * 512 + 384, r, 0, 0, 0};
            int5 gc_out0 = {h * 256, r, 0, 0, 0}, gc_out1 = {h * 256 + 128, r, 0, 0, 0};
            v_bf16_st_tnsr(gc_out0, gate_out, v_bf16_ld_tnsr_b(gc_in0, qkv));
            v_bf16_st_tnsr(gc_out1, gate_out, v_bf16_ld_tnsr_b(gc_in1, qkv));
        }

        // ---- K (RMSNorm + partial RoPE -> transposed [256, R, KV]) ----
        for (int h = 0; h < KV; ++h) {
            int5 ic0 = {k_base + h * 256, r, 0, 0, 0}, ic1 = {k_base + h * 256 + 128, r, 0, 0, 0};
            float128 f0 = v_convert_bf16_to_f32_all_b(v_bf16_ld_tnsr_b(ic0, qkv));
            float128 f1 = v_convert_bf16_to_f32_all_b(v_bf16_ld_tnsr_b(ic1, qkv));
            float64 sq = v_f32_mac_b(f0.v1, f0.v1, f0.v2 * f0.v2);
            sq = v_f32_mac_b(f1.v1, f1.v1, sq);
            sq = v_f32_mac_b(f1.v2, f1.v2, sq);
            const float64 inv = fast_rsqrt_2n(sum64_fast(sq, c4, c2, c1) * (1.0f / 256.0f) + eps);
            f0.v1 = f0.v1 * inv * kw0.v1; f0.v2 = f0.v2 * inv * kw0.v2;
            f1.v1 = f1.v1 * inv * kw1.v1; f1.v2 = f1.v2 * inv * kw1.v2;
            const float64 p1 = v_f32_mov_dual_group_all_b(f0.v1, 0xffffffff, 1, 0, 2, 3, all_dg, f0.v1);
            const float64 p2 = v_f32_mov_dual_group_all_b(f0.v2, 0xffffffff, 1, 0, 2, 3, all_dg, f0.v2);
            float128 r0;
            r0.v1 = v_f32_mac_b(p1, sin_e, f0.v1 * cos_e);
            r0.v2 = v_f32_mac_b(p2, sin_o, f0.v2 * cos_o);
            int5 kc0 = {0, r, h, 0, 0}, kc1 = {128, r, h, 0, 0};
#ifdef KV_FP8
            v_f8_st_tnsr_partial(kc0, k_out, f32_pair_to_f8(r0.v1, r0.v2), 127, 0);
            v_f8_st_tnsr_partial(kc1, k_out, f32_pair_to_f8(f1.v1, f1.v2), 127, 0);
#else
            v_bf16_st_tnsr(kc0, k_out, v_convert_f32_to_bf16_all_b(r0));
            v_bf16_st_tnsr(kc1, k_out, v_convert_f32_to_bf16_all_b(f1));
#endif
        }

        // ---- V (transposed [256, R, KV], optional FP8) ----
        for (int h = 0; h < KV; ++h) {
            int5 ic0 = {v_base + h * 256, r, 0, 0, 0}, ic1 = {v_base + h * 256 + 128, r, 0, 0, 0};
            const bfloat128 v0 = v_bf16_ld_tnsr_b(ic0, qkv);
            const bfloat128 v1 = v_bf16_ld_tnsr_b(ic1, qkv);
            int5 vc0 = {0, r, h, 0, 0}, vc1 = {128, r, h, 0, 0};
#ifdef KV_FP8
            const float128 vf0 = v_convert_bf16_to_f32_all_b(v0);
            const float128 vf1 = v_convert_bf16_to_f32_all_b(v1);
            v_f8_st_tnsr_partial(vc0, v_out, f32_pair_to_f8(vf0.v1, vf0.v2), 127, 0);
            v_f8_st_tnsr_partial(vc1, v_out, f32_pair_to_f8(vf1.v1, vf1.v2), 127, 0);
#else
            v_bf16_st_tnsr(vc0, v_out, v0);
            v_bf16_st_tnsr(vc1, v_out, v1);
#endif
        }
    }
}
