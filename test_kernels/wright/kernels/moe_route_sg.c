// SPDX-License-Identifier: AGPL-3.0-only
// Copyright Aaron Botsis
// MoE router, sigmoid scoring (GLM-5.3-Flash, noaux_tc with one group): s = sigmoid(logit); the K
// largest of s + bias are chosen (ties: lowest index), their weights are s normalized over the K and
// scaled: w_k = scale * s_k / (sum s + 1e-20). Padding tokens (t >= meta[1]) get ids -1, weights 0.
// tensors: 0 lg (>=E,T) f32 (router GEMM output)  1 bias (E) f32  2 meta i32 [pos0, n_real]
//        | 3 ids (64,T) i32 (lanes 0..K-1: global expert ids)  4 wts (64,T) f32
// -DSSP: s = sqrt(softplus(logit)), up to 384 experts (NV 6); otherwise the same.
// params: K (<= 64), E (<= 64 NV), scale. index space: [T]
#include "common.h"

#ifdef SSP
#define NV 6            // 384 lanes >= E = 384
static inline float64 score_fn(float64 x)
{
    const float64 y = nl_softplus(x);
    return y * nl_rsqrt(v_f32_max_b(y, 1.0e-30f));
}
#else
#define NV 5            // 320 lanes >= E = 288
#define score_fn nl_sigm
#endif
#define GONE -3.0e38f

__local__ float64 SG[NV];

void main(tensor lg, tensor bias, tensor meta, tensor ids, tensor wts, int K, int E, float scale)
{
    const int5 off = get_index_space_offset();
    const int5 end = off + get_index_space_size();
    const float64 lanef = v_convert_i32_to_f32_b((int64)read_lane_id_4b_b(), SW_RHNE, 0, 1, 0);
    DECL_IDX(IDX)
    scalar_cache_reset();
    const int n_real = meta_i32(meta, 1);
    float64 bv[NV];
    #pragma unroll
    for (int j = 0; j < NV; j++) {
        int5 c = {64 * j, 0, 0, 0, 0};
        bv[j] = v_f32_ld_tnsr_b(c, bias);             // lanes >= E: read past the end, masked below
    }
    const float Ef = (float)E;
    for (int t = off[0]; t < end[0]; t++) {
        float64 sg[NV], v[NV];
        #pragma unroll 1
        for (int j = 0; j < NV; j++) {
            int5 c = {64 * j, t, 0, 0, 0};
            SG[j] = score_fn(v_f32_ld_tnsr_b(c, lg));
        }
        #pragma unroll
        for (int j = 0; j < NV; j++) {
            sg[j] = SG[j];
            v[j] = v_f32_sel_less_f32_b(lanef + (float)(64 * j), Ef, sg[j] + bv[j], (float64)GONE);
        }
        float64 selw = 0.0f, seli = 0.0f;
        for (int k = 0; k < K; k++) {
            float64 m = v[0];
            #pragma unroll
            for (int j = 1; j < NV; j++) m = v_f32_max_b(m, v[j]);
            float64 mx, nmi;
            NL_REDUCE_MAX(m, IDX, mx);
            float64 cand = GONE, w = GONE;
            #pragma unroll
            for (int j = 0; j < NV; j++) {
                const float64 hit = v_f32_sel_eq_f32_b(v[j], mx, 1.0f, 0.0f);
                cand = v_f32_max_b(cand, v_f32_sel_eq_f32_b(hit, 1.0f, -(lanef + (float)(64 * j)), (float64)GONE));
            }
            NL_REDUCE_MAX(cand, IDX, nmi);                // lowest index of the maximum
            const float64 idx = -nmi;
            #pragma unroll
            for (int j = 0; j < NV; j++) {
                const float64 at = v_f32_sel_eq_f32_b(lanef + (float)(64 * j), idx, 1.0f, 0.0f);
                w = v_f32_max_b(w, v_f32_sel_eq_f32_b(at, 1.0f, sg[j], (float64)GONE));
                v[j] = v_f32_sel_eq_f32_b(at, 1.0f, (float64)GONE, v[j]);
            }
            float64 wk;                                   // the chosen expert's sigmoid score
            NL_REDUCE_MAX(w, IDX, wk);
            const float kf = (float)k;
            selw = v_f32_sel_eq_f32_b(lanef, kf, wk, selw);
            seli = v_f32_sel_eq_f32_b(lanef, kf, idx, seli);
        }
        float64 sum;
        NL_REDUCE_ADD(v_f32_sel_less_f32_b(lanef, (float)K, selw, 0.0f), IDX, sum);
        const float real = t < n_real ? 1.0f : 0.0f;
        int5 oc = {0, t, 0, 0, 0};
        const float64 wn = selw * nl_recip(sum + 1e-20f) * (scale * real);
        v_f32_st_tnsr(oc, wts, v_f32_sel_less_f32_b(lanef, (float)K, wn, 0.0f));
        const float64 io = v_f32_sel_less_f32_b(lanef, (float)K, seli, 0.0f);
        v_i32_st_tnsr(oc, ids, v_convert_f32_to_i32_b(real == 1.0f ? io : (float64)-1.0f, SW_RHNE, 0, 1, 0));
    }
}
