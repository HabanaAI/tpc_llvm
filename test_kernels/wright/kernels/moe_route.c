// SPDX-License-Identifier: AGPL-3.0-only
// Copyright Aaron Botsis
// MoE router: the K largest of E = 512 expert logits per token (ties: lowest index), routing
// weights = softmax over the chosen K (== softmax over all E, top-K, renormalized).
// Padding tokens (t >= meta[1], a prefill chunk's bucket tail) get ids -1 and weights 0.
// tensors: 0 lg (>=E,T) f32 (router GEMM output; rows >= E are ignored)  1 meta i32 [pos0, n_real]
//        | 2 ids (64,T) i32 (lanes 0..K-1: global expert ids, rest 0)  2 wts (64,T) f32 (rest 0)
// params: K (<= 64). index space: [T]
#include "common.h"

#define NV 8            // E / 64
#define GONE -3.0e38f

void main(tensor lg, tensor meta, tensor ids, tensor wts, int K)
{
    const int5 off = get_index_space_offset();
    const int5 end = off + get_index_space_size();
    const float64 lanef = v_convert_i32_to_f32_b((int64)read_lane_id_4b_b(), SW_RHNE, 0, 1, 0);
    DECL_IDX(IDX)
    scalar_cache_reset();
    const int n_real = meta_i32(meta, 1);
    for (int t = off[0]; t < end[0]; t++) {
        float64 v[NV];
        #pragma unroll
        for (int j = 0; j < NV; j++) {
            int5 c = {64 * j, t, 0, 0, 0};
            v[j] = v_f32_ld_tnsr_b(c, lg);
        }
        float64 selv = 0.0f, seli = 0.0f, top = 0.0f;
        for (int k = 0; k < K; k++) {
            float64 m = v_f32_max_b(v_f32_max_b(v_f32_max_b(v[0], v[1]), v_f32_max_b(v[2], v[3])),
                                    v_f32_max_b(v_f32_max_b(v[4], v[5]), v_f32_max_b(v[6], v[7])));
            float64 mx, nmi;
            NL_REDUCE_MAX(m, IDX, mx);
            float64 cand = GONE;
            #pragma unroll
            for (int j = 0; j < NV; j++)
                cand = v_f32_max_b(cand, v_f32_sel_eq_f32_b(v[j], mx, -(lanef + (float)(64 * j)), GONE));
            NL_REDUCE_MAX(cand, IDX, nmi);                // lowest index of the maximum
            const float64 idx = -nmi;
            #pragma unroll
            for (int j = 0; j < NV; j++)
                v[j] = v_f32_sel_eq_f32_b(lanef + (float)(64 * j), idx, GONE, v[j]);
            const float kf = (float)k;
            selv = v_f32_sel_eq_f32_b(lanef, kf, mx, selv);
            seli = v_f32_sel_eq_f32_b(lanef, kf, idx, seli);
            top = k == 0 ? mx : top;
        }
        const float64 e = v_f32_sel_less_f32_b(lanef, (float)K, nl_exp(selv - top), 0.0f);
        float64 sum;
        NL_REDUCE_ADD(e, IDX, sum);
        int5 oc = {0, t, 0, 0, 0};
        const float real = t < n_real ? 1.0f : 0.0f;
        v_f32_st_tnsr(oc, wts, e * nl_recip(sum) * real);
        v_i32_st_tnsr(oc, ids, v_convert_f32_to_i32_b(real == 1.0f ? seli : (float64)-1.0f, SW_RHNE, 0, 1, 0));
    }
}
