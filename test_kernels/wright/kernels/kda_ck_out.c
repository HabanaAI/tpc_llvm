// SPDX-License-Identifier: AGPL-3.0-only
// Copyright Aaron Botsis
// Chunked KDA, stage 5, per (head h, chunk c): o = QS + PD (inter-chunk q e^G S + intra-chunk P (U - W S)),
// then the gated RMSNorm of the head: out = o / rms(o) * w * sigmoid(gate), bf16 in sigma order (the
// o_proj GEMM operand), row h 128 + i of token c 64 + t.
// tensors: 0 QS (128, 64, nh, NC) f32  1 PD  2 gate (C, T) f32  3 w (128) f32 | 4 out (C, T) bf16
// params: eps. index space: [nh, NC] (from QS)
#include "common.h"

#define CH 64

void main(tensor QS, tensor PD, tensor gate, tensor w, tensor out, float eps)
{
    const int5 off = get_index_space_offset();
    const int5 end = off + get_index_space_size();
    DECL_IDX(IDX)
    int5 wc = {0, 0, 0, 0, 0};
    const float64 wa = v_f32_ld_tnsr_b(wc, w);
    wc[0] = 64;
    const float64 wb = v_f32_ld_tnsr_b(wc, w);
    for (int h = off[0]; h < end[0]; h++)
        for (int c = off[1]; c < end[1]; c++)
            for (int t = 0; t < CH; t++) {
                int5 ic = {0, t, h, c, 0};
                const float64 oa = v_f32_ld_tnsr_b(ic, QS) + v_f32_ld_tnsr_b(ic, PD);
                ic[0] = 64;
                const float64 ob = v_f32_ld_tnsr_b(ic, QS) + v_f32_ld_tnsr_b(ic, PD);
                const int p = c * CH + t;
                int5 gc = {128 * h, p, 0, 0, 0};
                const float64 ga = v_f32_ld_tnsr_b(gc, gate);
                gc[0] += 64;
                const float64 gb = v_f32_ld_tnsr_b(gc, gate);
                float64 ss;
                NL_REDUCE_ADD(v_f32_mac_b(oa, oa, ob * ob), IDX, ss);
                const float64 r = nl_rsqrt(ss * (1.0f / 128.0f) + eps);
                int5 oc = {128 * h, p, 0, 0, 0};
                v_bf16_st_tnsr(oc, out, pack_sigma(oa * r * wa * nl_sigm(ga), ob * r * wb * nl_sigm(gb)));
            }
}
