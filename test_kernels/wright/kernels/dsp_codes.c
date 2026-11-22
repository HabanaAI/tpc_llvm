// SPDX-License-Identifier: AGPL-3.0-only
// Copyright Aaron Botsis
// DSpine code bridge, the codes: column t of the read projection scaled to norm sqrt(C), except the anchor
// columns (t % block == 0), whose code is the anchor token's exact code.
//   codes[:, t] = sqrt(C) * proj[:, t] / (|proj[:, t]| + eps)       (t % block != 0)
//               = sqrt(C) * codebook[:, anchors[t / block]]          (t % block == 0)
// in sigma order (the operand of the packed GEMM).
// tensors: 0 proj (C,T) f32  1 codebook (C,V) bf16 (natural)  2 anchors (B) i32 | 3 codes (C,T) bf16
// params: eps, block. index space: [T]. C multiple of 128.
#include "common.h"
void main(tensor proj, tensor codebook, tensor anchors, tensor codes, float eps, int block)
{
    const int5 off = get_index_space_offset();
    const int5 end = off + get_index_space_size();
    const int C = get_dim_size(proj, 0);
    const float64 sqrt_c = (float64)(float)C * nl_rsqrt((float64)(float)C);
    DECL_IDX(IDX)
    scalar_cache_reset();
    for (int t = off[0]; t < end[0]; t++) {
        int5 c = {0, t, 0, 0, 0};
        if (t % block == 0) {
            int5 ac = {t / block, 0, 0, 0, 0};
            const int anchor = s_i32_ld_g(gen_addr(ac, anchors));
            int5 e = {0, anchor, 0, 0, 0};
            for (int i = 0; i < C; i += 128) {
                e[0] = i;
                const float128 v = convert_bfloat128_to_float128(v_bf16_ld_tnsr_b(e, codebook), SW_LINEAR);
                c[0] = i;
                v_bf16_st_tnsr(c, codes, pack_sigma(v.v1 * sqrt_c, v.v2 * sqrt_c));
            }
            continue;
        }
        float64 acc = 0.0f;
        for (int i = 0; i < C; i += 64) {
            c[0] = i;
            const float64 v = v_f32_ld_tnsr_b(c, proj);
            acc = v_f32_mac_b(v, v, acc);
        }
        float64 ss;
        NL_REDUCE_ADD(acc, IDX, ss);
        ss = v_f32_max_b(ss, 1.1754943508222875e-38f);
        const float64 f = sqrt_c * nl_recip(ss * nl_rsqrt(ss) + eps);
        for (int i = 0; i < C; i += 128) {
            c[0] = i;
            const float64 a = v_f32_ld_tnsr_b(c, proj);
            c[0] = i + 64;
            const float64 b = v_f32_ld_tnsr_b(c, proj);
            c[0] = i;
            v_bf16_st_tnsr(c, codes, pack_sigma(a * f, b * f));
        }
    }
}
