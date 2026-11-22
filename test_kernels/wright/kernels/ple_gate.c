// SPDX-License-Identifier: AGPL-3.0-only
// Copyright Aaron Botsis
// PLE (hashed n-gram per-layer embedding) gate, per token (Qwen3.8-Flash-Next, Qwen4ExpTextPLELayer):
//   key = kv[0 .. NS*H), value = kv[NS*H .. NS*H + H)     (key_proj / value_proj GEMM output)
//   g_s = <rmsnorm(key_s) * nk_s, rmsnorm(x_s) * nq_s> / sqrt(H);  g_s = sign(g) sqrt(max(|g|, 1e-6))
//   gv_s = sigmoid(g_s) * value;  gvn_s = rmsnorm(gv_s) * nc_s        (per stream s)
// tensors: 0 x (NS*H,T) f32 (residual)  1 kv ((NS+1)*H,T) f32  2 nk  3 nq  4 nc (NS*H) f32
//        | 5 gv (NS*H,T) f32  6 gvn (NS*H,T) f32
// params: eps. index space: [T]
#include "common.h"

#define NS 4

void main(tensor x, tensor kv, tensor nk, tensor nq, tensor nc, tensor gv, tensor gvn, float eps)
{
    const int5 off = get_index_space_offset();
    const int5 end = off + get_index_space_size();
    const int H = get_dim_size(x, 0) / NS;
    const float inv_h = 1.0f / (float)H;
    const float64 inv_sq = nl_rsqrt((float64)(float)H);
    DECL_IDX(IDX)
    for (int t = off[0]; t < end[0]; t++) {
        float64 ak[NS], ax[NS], vv = 0.0f;
        #pragma unroll
        for (int s = 0; s < NS; s++) {
            ak[s] = 0.0f; ax[s] = 0.0f;
            for (int i = 0; i < H; i += 64) {
                int5 c = {s * H + i, t, 0, 0, 0};
                const float64 k = v_f32_ld_tnsr_b(c, kv), xv = v_f32_ld_tnsr_b(c, x);
                ak[s] = v_f32_mac_b(k, k, ak[s]);
                ax[s] = v_f32_mac_b(xv, xv, ax[s]);
            }
        }
        for (int i = 0; i < H; i += 64) {
            int5 c = {NS * H + i, t, 0, 0, 0};
            const float64 v = v_f32_ld_tnsr_b(c, kv);
            vv = v_f32_mac_b(v, v, vv);
        }
        float64 sk[NS], sx[NS], sv;
        NL_REDUCE4_ADD(ak[0], ak[1], ak[2], ak[3], IDX, sk[0], sk[1], sk[2], sk[3]);
        NL_REDUCE4_ADD(ax[0], ax[1], ax[2], ax[3], IDX, sx[0], sx[1], sx[2], sx[3]);
        NL_REDUCE_ADD(vv, IDX, sv);
        float64 ad[NS];
        #pragma unroll
        for (int s = 0; s < NS; s++) {
            ad[s] = 0.0f;
            for (int i = 0; i < H; i += 64) {
                int5 c = {s * H + i, t, 0, 0, 0}, wc = {s * H + i, 0, 0, 0, 0};
                ad[s] = v_f32_mac_b(v_f32_ld_tnsr_b(c, kv) * v_f32_ld_tnsr_b(wc, nk),
                                    v_f32_ld_tnsr_b(c, x) * v_f32_ld_tnsr_b(wc, nq), ad[s]);
            }
        }
        float64 dt[NS];
        NL_REDUCE4_ADD(ad[0], ad[1], ad[2], ad[3], IDX, dt[0], dt[1], dt[2], dt[3]);
        const float64 msv = sv * inv_h;                            // mean(value^2)
        #pragma unroll
        for (int s = 0; s < NS; s++) {
            const float64 g = dt[s] * nl_rsqrt(sk[s] * inv_h + eps) * nl_rsqrt(sx[s] * inv_h + eps) * inv_sq;
            const float64 ag = v_f32_max_b(v_f32_abs_b(g), 1e-6f);
            const float64 sq = ag * nl_rsqrt(ag);                  // sqrt(|g|)
            const float64 sg = nl_sigm(v_f32_sel_less_f32_b(g, 0.0f, -sq, sq));
            const float64 rn = nl_rsqrt(sg * sg * msv + eps) * sg; // norm factor of gv_s = sg * value
            for (int i = 0; i < H; i += 64) {
                int5 c = {NS * H + i, t, 0, 0, 0}, oc = {s * H + i, t, 0, 0, 0}, wc = {s * H + i, 0, 0, 0, 0};
                const float64 v = v_f32_ld_tnsr_b(c, kv);
                v_f32_st_tnsr(oc, gv, v * sg);
                v_f32_st_tnsr(oc, gvn, v * rn * v_f32_ld_tnsr_b(wc, nc));
            }
        }
    }
}
