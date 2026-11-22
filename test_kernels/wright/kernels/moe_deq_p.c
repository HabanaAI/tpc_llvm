// SPDX-License-Identifier: AGPL-3.0-only
// Copyright Aaron Botsis
// Prefill MoE: dequantize each allocated page's expert (block FP8 -> bf16 MME operand, see moe_deq.c)
// into the page's weight slot: out[p] = W[pe[p]] (unused pages, pe = -1, are skipped).
// tensors: 0 W (256, 128, NOB, NIB, E) fp8  1 S (64, 2, NOB, NIB, E) f32  2 pe (P,) i32 | 3 out (M, K, P) bf16
// params: a, d (as moe_deq). index space: [NOB, NIB, P]
#include "common.h"

void main(tensor W, tensor S, tensor pe, tensor out, int a, int d)
{
    const int5 off = get_index_space_offset();
    const int5 end = off + get_index_space_size();
    scalar_cache_reset();
    for (int p = off[2]; p < end[2]; p++) {
        int5 pc = {p, 0, 0, 0, 0};
        const int e = s_i32_ld_g(gen_addr(pc, pe));
        if (e < 0)
            continue;
        for (int ib = off[1]; ib < end[1]; ib++)
            for (int ob = off[0]; ob < end[0]; ob++) {
                int5 sc = {0, 0, ob, ib, e};
                const float64 s0 = v_f32_ld_tnsr_b(sc, S);
                sc[1] = 1;
                const float64 s1 = v_f32_ld_tnsr_b(sc, S);
                int5 wc = {0, 0, ob, ib, e};
                // 4 rows per step, loads first
                for (int i = 0; i < 128; i += 4) {
                    float256 f[4];
                    #pragma unroll
                    for (int u = 0; u < 4; u++) {
                        wc[1] = i + u;
                        f[u] = convert_minifloat256_to_float256(v_f8_ld_tnsr_b(wc, W), 0);
                    }
                    #pragma unroll
                    for (int u = 0; u < 4; u++) {
                        int5 oc = {a * ob, sigma_inv(128 * ib + i + u), p, 0, 0};
                        v_bf16_st_tnsr(oc, out, pack_sigma(f[u].v1 * s0, f[u].v2 * s0));
                        oc[0] = a * ob + d;
                        v_bf16_st_tnsr(oc, out, pack_sigma(f[u].v3 * s1, f[u].v4 * s1));
                    }
                }
            }
    }
}
