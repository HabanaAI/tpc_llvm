// SPDX-License-Identifier: AGPL-3.0-only
// Copyright Aaron Botsis
// ds_ix_slot_acc8 for prefill slots (DeepSeek-V4.1) over bf16 head scores S (each head's product rounded to bf16,
// as the reference's bf16 einsum), with each member taking 1024 keys (16 vectors) of 8 of the slot's query tokens.
// A token's 32 head weights (w * scale * sq) are read once per member, then every vector of S is read once.
//   score = sk * sum_h w[h, col] * scale * sq[h + NH t, s] * relu(S[i, h + NH t])   if key0 + i < min((p + 1) / r, klen)
//   score = -inf                                                                    otherwise
// slots (8, S) i32 per slot: [group, qcol0, qpos0, key0, klen, qtok0, ratio, 0] (group -1: padding)
// tensors: 0 S (C, NH * Q, S) bf16  1 w (NH, T) f32  2 slots  3 score (K, T) f32  4 sq (NH Q, S) f32  5 sk (C, S) f32
//        | 6 sink (64, 1) i32
// params: scale. index space: [C / 1024, Q / 8, S]; member (0, 0, 0) writes the sink. NH = 32.
#include "common.h"

#define NINF (-3.0e38f)
#define NH 32
#define TQ 8
#define NB 16

void main(tensor S, tensor w, tensor slots, tensor score, tensor sq, tensor sk, tensor sink, float scale)
{
    const int5 off = get_index_space_offset();
    const int5 end = off + get_index_space_size();
    const float64 lanef = v_convert_i32_to_f32_b((int64)read_lane_id_4b_b(), SW_RHNE, 0, 1, 0);
    scalar_cache_reset();
    for (int s = off[2]; s < end[2]; s++) {
        int5 fc = {0, s, 0, 0, 0};
        if (s_i32_ld_g(gen_addr(fc, slots)) < 0)
            continue;
        fc[0] = 2;
        const int qpos0 = s_i32_ld_g(gen_addr(fc, slots));
        fc[0] = 3;
        const int key0 = s_i32_ld_g(gen_addr(fc, slots));
        fc[0] = 4;
        const int klen = s_i32_ld_g(gen_addr(fc, slots));
        fc[0] = 5;
        const int tok0 = s_i32_ld_g(gen_addr(fc, slots));
        fc[0] = 6;
        int ratio = s_i32_ld_g(gen_addr(fc, slots));
        ratio = ratio > 0 ? ratio : 1;
        for (int g = off[0]; g < end[0]; g++) {
            for (int tq = off[1]; tq < end[1]; tq++)
                for (int t = TQ * tq; t < TQ * tq + TQ; t++) {
                    const int col = tok0 + t;
                    const int seen = (qpos0 + t + 1) / ratio;
                    const int lim = seen < klen ? seen : klen;
                    float wt[NH];
                    #pragma unroll
                    for (int h = 0; h < NH; h++) {
                        int5 wc = {h, col, 0, 0, 0};
                        int5 qc = {h + NH * t, s, 0, 0, 0};
                        wt[h] = s_f32_ld_g(gen_addr(wc, w)) * scale * s_f32_ld_g(gen_addr(qc, sq));
                    }
                    #pragma unroll 1
                    for (int j = 0; j < NB; j += 2) {  // two vectors of keys per bf16 read
                        const int b = NB * g + j;
                        float64 a0 = 0.0f, a1 = 0.0f, c0 = 0.0f, c1 = 0.0f;  // even and odd heads, keys b | b + 1
                        int5 sc = {64 * b, NH * t, s, 0, 0};
                        #pragma unroll
                        for (int h = 0; h < NH; h += 2) {
                            sc[1] = h + NH * t;
                            float128 x = convert_bfloat128_to_float128(v_bf16_ld_tnsr_b(sc, S), SW_LINEAR);
                            a0 = v_f32_mac_b(v_f32_max_b(x.v1, 0.0f), wt[h], a0);
                            c0 = v_f32_mac_b(v_f32_max_b(x.v2, 0.0f), wt[h], c0);
                            sc[1] = h + 1 + NH * t;
                            x = convert_bfloat128_to_float128(v_bf16_ld_tnsr_b(sc, S), SW_LINEAR);
                            a1 = v_f32_mac_b(v_f32_max_b(x.v1, 0.0f), wt[h + 1], a1);
                            c1 = v_f32_mac_b(v_f32_max_b(x.v2, 0.0f), wt[h + 1], c1);
                        }
                        #pragma unroll
                        for (int u = 0; u < 2; u++) {
                            int5 kc = {64 * (b + u), s, 0, 0, 0};
                            const float64 acc = (u ? c0 + c1 : a0 + a1) * v_f32_ld_tnsr_b(kc, sk);
                            const float64 key = lanef + (float)(key0 + 64 * (b + u));
                            int5 oc = {key0 + 64 * (b + u), col, 0, 0, 0};
                            v_f32_st_tnsr(oc, score, v_f32_sel_less_f32_b(key, (float)lim, acc, NINF));
                        }
                    }
                }
        }
    }
    if (off[0] == 0 && off[1] == 0 && off[2] == 0) {
        int5 zc = {0, 0, 0, 0, 0};
        v_i32_st_tnsr(zc, sink, (int64)1);
    }
}
