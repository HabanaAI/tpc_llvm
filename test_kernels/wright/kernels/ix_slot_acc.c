// SPDX-License-Identifier: AGPL-3.0-only
// Copyright Aaron Botsis
// DSA indexer scores of one slot launch (Mistral-Large-4), into the step's score scratch: for
// slot s (a query block's tokens against one 2048-key chunk), token t of the block (column
// qtok0 + t) and key key0 + i,
//   score = sum_h w[h, col] * relu(scale * S[i, h + NH t])     if the key is visible to t
//                                                              (key0 + i <= qpos0 + t, < klen)
//   score = -inf                                               otherwise.
// slots (8, S) i32 per slot: [group, qcol0, qpos0, key0, klen, qtok0, 0, 0] (group -1: padding)
// RATIO (ds_ix_slot_acc.c): keys are compressed entries of r tokens each (slot field 6, 0 or 1: one token); key i
// is visible to token position p when key0 + i < (p + 1) / r.
// tensors: 0 S (C, NH * Q, S) f32  1 w (NH, T) f32  2 slots  3 score (K, T) f32
//        | 4 sink (64, 1) i32
// params: scale. index space: [C / 64, S]; member (0, 0) writes the sink.
#include "common.h"

#define NINF (-3.0e38f)

#ifdef SCALED
void main(tensor S, tensor w, tensor slots, tensor score, tensor sq, tensor sk, tensor sink, float scale)
#else
void main(tensor S, tensor w, tensor slots, tensor score, tensor sink, float scale)
#endif
{
    const int5 off = get_index_space_offset();
    const int5 end = off + get_index_space_size();
    const int NH = get_dim_size(w, 0);
    const int Q = get_dim_size(S, 1) / NH;
    const float64 lanef = v_convert_i32_to_f32_b((int64)read_lane_id_4b_b(), SW_RHNE, 0, 1, 0);
    scalar_cache_reset();
    for (int s = off[1]; s < end[1]; s++) {
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
#ifdef RATIO
        fc[0] = 6;
        int ratio = s_i32_ld_g(gen_addr(fc, slots));
        ratio = ratio > 0 ? ratio : 1;
#endif
        for (int b = off[0]; b < end[0]; b++) {
            const float64 key = lanef + (float)(key0 + 64 * b);
            for (int t = 0; t < Q; t++) {
                const int col = tok0 + t;
#ifdef RATIO
                const int seen = (qpos0 + t + 1) / ratio;  // compressed keys whose groups are complete
                int lim = seen < klen ? seen : klen;
#else
                int lim = qpos0 + t + 1 < klen ? qpos0 + t + 1 : klen;  // visible keys < lim
#endif
                float64 a0 = 0.0f, a1 = 0.0f;
                for (int h = 0; h < NH; h += 2) {  // (scale > 0: it moves past the relu)
                    int5 wc = {h, col, 0, 0, 0};
                    float w0 = s_f32_ld_g(gen_addr(wc, w)) * scale;
                    wc[0] = h + 1;
                    float w1 = s_f32_ld_g(gen_addr(wc, w)) * scale;
#ifdef SCALED
                    int5 qc = {h + NH * t, s, 0, 0, 0};
                    w0 *= s_f32_ld_g(gen_addr(qc, sq));
                    qc[0]++;
                    w1 *= s_f32_ld_g(gen_addr(qc, sq));
#endif
                    int5 sc = {64 * b, h + NH * t, s, 0, 0};
                    a0 = v_f32_mac_b(v_f32_max_b(v_f32_ld_tnsr_b(sc, S), 0.0f), w0, a0);
                    sc[1] = h + 1 + NH * t;
                    a1 = v_f32_mac_b(v_f32_max_b(v_f32_ld_tnsr_b(sc, S), 0.0f), w1, a1);
                }
                int5 oc = {key0 + 64 * b, col, 0, 0, 0};
#ifdef SCALED
                int5 kc = {64 * b, s, 0, 0, 0};
                const float64 acc = (a0 + a1) * v_f32_ld_tnsr_b(kc, sk);
#else
                const float64 acc = a0 + a1;
#endif
                v_f32_st_tnsr(oc, score, v_f32_sel_less_f32_b(key, (float)lim, acc, NINF));
            }
        }
    }
    if (off[0] == 0 && off[1] == 0) {
        int5 zc = {0, 0, 0, 0, 0};
        v_i32_st_tnsr(zc, sink, (int64)1);
    }
}
