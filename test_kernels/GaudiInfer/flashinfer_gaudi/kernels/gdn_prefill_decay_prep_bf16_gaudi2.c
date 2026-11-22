// SPDX-License-Identifier: Apache-2.0
// g2e: ULTRA-FAST fused GDN prefill decay + mask + initial Newton/Doubling matrix prep for C_chunk == 64.
// Uses BF16 packed 256B loads of kk/qk ([128, 32, BK]), 4-way interleaved FP32 exp2 (exact decay, zero VPU stalls!),
// and computes a/p/l8/x0 directly in 128-lane BF16 (1 f32->bf16 pack per 2 rows instead of 4!).
//
// Inputs (TPC dim 0 fastest):
//   kk   [128, 32, N * HK] bf16  (reshaped from [N * HK, 64, 64] bf16: 2 rows per 256B vector!)
//   qk   [128, 32, N * HK] bf16
//   cum  [64, N * HV]      f32
//   bt   [64, N * HV]      f32
// Outputs:
//   a_out  [128, 32, N * HV] bf16
//   l8_out [128, 32, N * HV] bf16
//   x0_out [128, 32, N * HV] bf16
//   p_out  [128, 32, N * HV] bf16
//   ws_out [64, N * HV]      bf16
//   gm_out [64, N * HV]      bf16
//   tl_out [64, N * HV]      bf16
//   bt_out [64, N * HV]      bf16
//   gl_out [1, 1, N * HV]    f32
// Index space: [N * HV].

static inline float64 exp_vec_f32(float64 x) {
    const float64 xc = v_f32_max_b(x, -87.0f);
    const float64 t = xc * 1.4426950408889634f;
    const int64 n = v_convert_f32_to_i32_b(t, SW_RHNE);
    const float64 f = t - v_convert_i32_to_f32_b(n, 0);
    float64 p = 0.0096181291f;
    p = v_f32_mac_b(p, f, 0.0555041087f);
    p = v_f32_mac_b(p, f, 0.2402265070f);
    p = v_f32_mac_b(p, f, 0.6931471806f);
    p = v_f32_mac_b(p, f, 1.0f);
    const int64 bits = (n + 127) << 23;
    const float64 r = p * (*((float64*)&bits));
    return v_f32_sel_leq_f32_b(x, -87.0f, 0.0f, r);
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

static inline bfloat128 bcast_f32_pair_bf16(float64 b0, float64 b1, ushort128 hi_bit) {
    float64_pair_t p0, p1;
    p0.v1 = b0; p0.v2 = b0;
    p1.v1 = b1; p1.v2 = b1;
    return v_bf16_sel_eq_u16_b(hi_bit, (unsigned short)0,
                               v_convert_f32_to_bf16_all_b(p0),
                               v_convert_f32_to_bf16_all_b(p1));
}

void main(tensor kk, tensor qk, tensor cum, tensor bt,
          tensor a_out, tensor l8_out, tensor x0_out, tensor p_out,
          tensor ws_out, tensor gm_out, tensor tl_out, tensor bt_out, tensor gl_out)
{
    const int5 start = get_index_space_offset();
    const int5 end = start + get_index_space_size();
    const int BV = get_dim_size(cum, 1);
    const int BK = get_dim_size(kk, 2);
    const int REP = BV / BK;

    const uint64 lanes = read_lane_id_4b_b();
    const ushort128 lanes128 = read_lane_id_2b_b();
    const ushort128 u128 = lanes128 & (ushort128)63;
    const ushort128 hi_bit = lanes128 >> 6;
    const ushort128 u_blk8 = u128 >> 3;
    const bfloat128 v_zero_b = (bfloat128)(bf16)0.0f;
    const bfloat128 v_one_b  = (bfloat128)(bf16)1.0f;
    const float64 v_zero_f = 0.0f;

    for (int bv = start[0]; bv < end[0]; ++bv) {
        const int bk = bv / REP;

        int5 v1c = {0, bv, 0, 0, 0};
        const float64 G_vec = v_f32_ld_tnsr_b(v1c, cum);
        const float64 B_vec = v_f32_ld_tnsr_b(v1c, bt);
        const float64 G_last = v_broadcast_element_f32(G_vec, 63);

        const float64 gamma_v = exp_vec_f32(G_vec);
        const float64 tail_v = exp_vec_f32(G_last - G_vec);
        const float64 ws_v = B_vec * gamma_v;
        const float64 gl_v = exp_vec_f32(G_last);

        v_bf16_st_tnsr_partial(v1c, ws_out, f32_to_bf16_lo64(ws_v), 63, 0);
        v_bf16_st_tnsr_partial(v1c, gm_out, f32_to_bf16_lo64(gamma_v), 63, 0);
        v_bf16_st_tnsr_partial(v1c, tl_out, f32_to_bf16_lo64(tail_v), 63, 0);
        v_bf16_st_tnsr_partial(v1c, bt_out, f32_to_bf16_lo64(B_vec), 63, 0);
        int5 glc = {0, 0, bv, 0, 0};
        v_f32_st_tnsr(glc, gl_out, gl_v);

        // Process 4 rows (2 pairs) per iteration: 4-way interleaved FP32 exp2 (exact + 0 VPU RAW stalls!)
        for (int t = 0; t < 64; t += 4) {
            const int pr0 = t >> 1;
            const int pr1 = pr0 + 1;
            int5 kc0 = {0, pr0, bk, 0, 0};
            int5 kc1 = {0, pr1, bk, 0, 0};
            const bfloat128 kk_p0 = v_bf16_ld_tnsr_b(kc0, kk);
            const bfloat128 qk_p0 = v_bf16_ld_tnsr_b(kc0, qk);
            const bfloat128 kk_p1 = v_bf16_ld_tnsr_b(kc1, kk);
            const bfloat128 qk_p1 = v_bf16_ld_tnsr_b(kc1, qk);

            const float64 Gt0 = v_broadcast_element_f32(G_vec, t);
            const float64 Gt1 = v_broadcast_element_f32(G_vec, t + 1);
            const float64 Gt2 = v_broadcast_element_f32(G_vec, t + 2);
            const float64 Gt3 = v_broadcast_element_f32(G_vec, t + 3);

            const float64 arg0 = v_f32_sel_leq_u32_b(lanes, (uint64)t,       Gt0 - G_vec, -87.0f);
            const float64 arg1 = v_f32_sel_leq_u32_b(lanes, (uint64)(t + 1), Gt1 - G_vec, -87.0f);
            const float64 arg2 = v_f32_sel_leq_u32_b(lanes, (uint64)(t + 2), Gt2 - G_vec, -87.0f);
            const float64 arg3 = v_f32_sel_leq_u32_b(lanes, (uint64)(t + 3), Gt3 - G_vec, -87.0f);

            const float64 xc0 = v_f32_max_b(arg0, -87.0f);
            const float64 xc1 = v_f32_max_b(arg1, -87.0f);
            const float64 xc2 = v_f32_max_b(arg2, -87.0f);
            const float64 xc3 = v_f32_max_b(arg3, -87.0f);

            const float64 et0 = v_f32_mac_b(xc0, 1.4426950408889634f, (float64)127.0f);
            const float64 et1 = v_f32_mac_b(xc1, 1.4426950408889634f, (float64)127.0f);
            const float64 et2 = v_f32_mac_b(xc2, 1.4426950408889634f, (float64)127.0f);
            const float64 et3 = v_f32_mac_b(xc3, 1.4426950408889634f, (float64)127.0f);

            const int64 en0 = v_convert_f32_to_i32_b(et0, SW_RHNE);
            const int64 en1 = v_convert_f32_to_i32_b(et1, SW_RHNE);
            const int64 en2 = v_convert_f32_to_i32_b(et2, SW_RHNE);
            const int64 en3 = v_convert_f32_to_i32_b(et3, SW_RHNE);

            const float64 ef0 = et0 - v_convert_i32_to_f32_b(en0, 0);
            const float64 ef1 = et1 - v_convert_i32_to_f32_b(en1, 0);
            const float64 ef2 = et2 - v_convert_i32_to_f32_b(en2, 0);
            const float64 ef3 = et3 - v_convert_i32_to_f32_b(en3, 0);

            float64 ep0 = v_f32_mac_b((float64)0.2402265070f, ef0, (float64)0.6931471806f);
            float64 ep1 = v_f32_mac_b((float64)0.2402265070f, ef1, (float64)0.6931471806f);
            float64 ep2 = v_f32_mac_b((float64)0.2402265070f, ef2, (float64)0.6931471806f);
            float64 ep3 = v_f32_mac_b((float64)0.2402265070f, ef3, (float64)0.6931471806f);

            ep0 = v_f32_mac_b(ep0, ef0, (float64)1.0f);
            ep1 = v_f32_mac_b(ep1, ef1, (float64)1.0f);
            ep2 = v_f32_mac_b(ep2, ef2, (float64)1.0f);
            ep3 = v_f32_mac_b(ep3, ef3, (float64)1.0f);

            const int64 eb0 = en0 << 23;
            const int64 eb1 = en1 << 23;
            const int64 eb2 = en2 << 23;
            const int64 eb3 = en3 << 23;

            const float64 d0 = v_f32_sel_leq_f32_b(arg0, -87.0f, v_zero_f, ep0 * (*((float64*)&eb0)));
            const float64 d1 = v_f32_sel_leq_f32_b(arg1, -87.0f, v_zero_f, ep1 * (*((float64*)&eb1)));
            const float64 d2 = v_f32_sel_leq_f32_b(arg2, -87.0f, v_zero_f, ep2 * (*((float64*)&eb2)));
            const float64 d3 = v_f32_sel_leq_f32_b(arg3, -87.0f, v_zero_f, ep3 * (*((float64*)&eb3)));

            const bfloat128 d_p0 = f32_pair_to_bf16_linear_fast(d0, d1);
            const bfloat128 d_p1 = f32_pair_to_bf16_linear_fast(d2, d3);

            const bfloat128 Bt_p0 = bcast_f32_pair_bf16(v_broadcast_element_f32(B_vec, t),
                                                        v_broadcast_element_f32(B_vec, t + 1), hi_bit);
            const bfloat128 Bt_p1 = bcast_f32_pair_bf16(v_broadcast_element_f32(B_vec, t + 2),
                                                        v_broadcast_element_f32(B_vec, t + 3), hi_bit);

            const ushort128 trow0 = (ushort128)t + hi_bit;
            const ushort128 trow1 = (ushort128)(t + 2) + hi_bit;
            const unsigned short tb = (unsigned short)(t >> 3);

            const bfloat128 a_p0 = v_bf16_sel_less_u16_b(u128, trow0, (Bt_p0 * d_p0) * kk_p0, v_zero_b);
            const bfloat128 a_p1 = v_bf16_sel_less_u16_b(u128, trow1, (Bt_p1 * d_p1) * kk_p1, v_zero_b);
            const bfloat128 p_p0 = d_p0 * qk_p0;
            const bfloat128 p_p1 = d_p1 * qk_p1;

            const bfloat128 a8_p0 = v_bf16_sel_eq_u16_b(u_blk8, tb, a_p0, v_zero_b);
            const bfloat128 a8_p1 = v_bf16_sel_eq_u16_b(u_blk8, tb, a_p1, v_zero_b);
            const bfloat128 eye_p0 = v_bf16_sel_eq_u16_b(u128, trow0, v_one_b, v_zero_b);
            const bfloat128 eye_p1 = v_bf16_sel_eq_u16_b(u128, trow1, v_one_b, v_zero_b);

            int5 oc0 = {0, pr0, bv, 0, 0};
            int5 oc1 = {0, pr1, bv, 0, 0};
            v_bf16_st_tnsr(oc0, a_out,  a_p0);
            v_bf16_st_tnsr(oc1, a_out,  a_p1);
            v_bf16_st_tnsr(oc0, l8_out, a8_p0 + eye_p0);
            v_bf16_st_tnsr(oc1, l8_out, a8_p1 + eye_p1);
            v_bf16_st_tnsr(oc0, x0_out, eye_p0 - a8_p0);
            v_bf16_st_tnsr(oc1, x0_out, eye_p1 - a8_p1);
            v_bf16_st_tnsr(oc0, p_out,  p_p0);
            v_bf16_st_tnsr(oc1, p_out,  p_p1);
        }
    }
}
