// SPDX-License-Identifier: AGPL-3.0-only
// Copyright Aaron Botsis
// MoE expert gate/up projection + SwiGLU for the routed (token, slot) pairs whose expert is local.
// Expert weights are block FP8 (128 x 128 blocks, the checkpoint's own scales), stored input-major
// so one 256-byte load holds 256 outputs of one input (no cross-lane reductions):
//   W[e][i][b] = tau-packed [gate rows 128b .. 128b+127 | up rows 128b .. 128b+127] at input i
//   (unpacked: v1, v2 = gate outputs, v3, v4 = up outputs, natural order).
//   S[e][ib][b][g][lane] = block scale of (output block b, input block ib) for g = gate / up,
//   replicated over the 64 lanes (one vector load, no lane extraction).
// h[128 b + j] of slot k = silu(gate) * up; members of slots whose expert is not local write nothing
// (moe_down skips those slots).
// tensors: 0 x (H,T) f32 (natural)  1 W (256, 128, NB, H/128, E) fp8  2 S (64, 2, NB, H/128, E) f32
//          3 ids (64,T) i32 (global expert ids in lanes 0..K-1)
//        | 4 h (128, NB, K, T) f32
// params: lo (first local expert), K. index space: [NB, K, T]
#include "common.h"

#ifdef LIM
void main(tensor x, tensor W, tensor S, tensor ids, tensor h, int lo, int K, float lim)
#else
void main(tensor x, tensor W, tensor S, tensor ids, tensor h, int lo, int K)
#endif
{
    const int5 off = get_index_space_offset();
    const int5 end = off + get_index_space_size();
    const int H = get_dim_size(x, 0);
    const int E = get_dim_size(W, 4);
    DECL_IDX(IDX)
    scalar_cache_reset();
    const int T = get_dim_size(x, 1);
    for (int ti = off[2]; ti < end[2]; ti++) {
        const int t = tok_spread(ti, T);
        for (int k = off[1]; k < end[1]; k++) {
            int5 ic = {k, t, 0, 0, 0};
            const int e = s_i32_ld_g(gen_addr(ic, ids)) - lo;
            if (e < 0 || e >= E)
                continue;
            for (int b = off[0]; b < end[0]; b++) {
                float64 a0 = 0.0f, a1 = 0.0f, a2 = 0.0f, a3 = 0.0f;
                for (int ib = 0; ib < H; ib += 128) {
                    float64 p0 = 0.0f, p1 = 0.0f, p2 = 0.0f, p3 = 0.0f;
                    for (int i0 = ib; i0 < ib + 128; i0 += 64) {
                        int5 xc = {i0, t, 0, 0, 0};
                        const float64 xv = v_f32_ld_tnsr_b(xc, x);
                        int5 wc = {0, 0, b, i0 >> 7, e};
#define DG_STEP(DG)                                                                          \
                        {                                                                    \
                            const float64 xd = BCAST_DG(xv, DG);                             \
                            _Pragma("unroll")                                                \
                            for (int l = 0; l < 16; l++) {                                   \
                                const float64 xb = v_f32_shuffle_b(xd, IDX[l], 0, xd);       \
                                wc[1] = (i0 & 127) + 16 * (DG) + l;                                  \
                                const float256 f = convert_minifloat256_to_float256(v_f8_ld_tnsr_b(wc, W), 0); \
                                p0 = v_f32_mac_b(f.v1, xb, p0);                              \
                                p1 = v_f32_mac_b(f.v2, xb, p1);                              \
                                p2 = v_f32_mac_b(f.v3, xb, p2);                              \
                                p3 = v_f32_mac_b(f.v4, xb, p3);                              \
                            }                                                                \
                        }
                        DG_STEP(0) DG_STEP(1) DG_STEP(2) DG_STEP(3)
#undef DG_STEP
                    }
                    int5 sc = {0, 0, b, ib >> 7, e};
                    const float64 sg = v_f32_ld_tnsr_b(sc, S);
                    sc[1] = 1;
                    const float64 su = v_f32_ld_tnsr_b(sc, S);
                    a0 = v_f32_mac_b(p0, sg, a0);
                    a1 = v_f32_mac_b(p1, sg, a1);
                    a2 = v_f32_mac_b(p2, su, a2);
                    a3 = v_f32_mac_b(p3, su, a3);
                }
                int5 oc = {0, b, k, t, 0};
                v_f32_st_tnsr(oc, h, nl_silus(CLAMP_G(a0)) * CLAMP_U(a2));
                oc[0] = 64;
                v_f32_st_tnsr(oc, h, nl_silus(CLAMP_G(a1)) * CLAMP_U(a3));
            }
        }
    }
}
