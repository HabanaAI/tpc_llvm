// SPDX-License-Identifier: AGPL-3.0-only
// Copyright Aaron Botsis
// Attention epilogue: out = bf16( (o_new + o_past) * sigmoid(gate) ), sigma order, heads
// concatenated. Head h lives in KV group g = h/hpg: o*_g (256, hpg*T), column = h%hpg + hpg*t,
// d in sigma order. Gate rows in p are in sigma order.
// tensors: on_0..on_{NG-1} [op_0..op_{NG-1}] p (512 H + 512 NG, T) f32 | out (256 H, T) bf16
// FP8 build (-DFP8): p has a per-token scale ps (1,T) (input after p); the gated result is
// emitted as fp8 (tau order) + per-token scale for the o_proj FP8 GEMM:
//   ... p ps | out8 (256 H, T) fp8  os (1,T) f32
// params: hpg. index space: [T]
#include "common.h"

#ifndef NG
#define NG 2
#endif

#define add_o(o, oc, a, b) \
    { int5 c_ = oc; a = a + v_f32_ld_tnsr_b(c_, o); c_[0] += 64; b = b + v_f32_ld_tnsr_b(c_, o); }

#if NG == 1
#define GROUP_ADD(g, t0, t1, t2, t3, oc, a, b) { add_o(t0, oc, a, b) }
#elif NG == 2
#define GROUP_ADD(g, t0, t1, t2, t3, oc, a, b) \
    { if ((g) == 0) add_o(t0, oc, a, b) else add_o(t1, oc, a, b) }
#elif NG == 4
#define GROUP_ADD(g, t0, t1, t2, t3, oc, a, b) \
    { switch (g) { case 0: add_o(t0, oc, a, b) break; case 1: add_o(t1, oc, a, b) break; \
                   case 2: add_o(t2, oc, a, b) break; default: add_o(t3, oc, a, b) } }
#else
#error "NG must be 1, 2 or 4"
#endif

#ifndef SDPA
#ifdef FP8
#define P_ARGS tensor p, tensor ps, tensor out, tensor os
__local__ float64 GB[128];        // gated head outputs of one token (<= 8192 channels)
#else
#define P_ARGS tensor p, tensor out
#endif
#if NG == 1
  #ifdef NO_PAST
void main(tensor on0, P_ARGS, int hpg)
  #else
void main(tensor on0, tensor op0, P_ARGS, int hpg)
  #endif
#define on1 on0
#define on2 on0
#define on3 on0
#define op1 op0
#define op2 op0
#define op3 op0
#elif NG == 2
  #ifdef NO_PAST
void main(tensor on0, tensor on1, P_ARGS, int hpg)
  #else
void main(tensor on0, tensor on1, tensor op0, tensor op1, P_ARGS, int hpg)
  #endif
#define on2 on0
#define on3 on0
#define op2 op0
#define op3 op0
#else
  #ifdef NO_PAST
void main(tensor on0, tensor on1, tensor on2, tensor on3, P_ARGS, int hpg)
  #else
void main(tensor on0, tensor on1, tensor on2, tensor on3, tensor op0, tensor op1, tensor op2, tensor op3,
          P_ARGS, int hpg)
  #endif
#endif
{
    const int5 off = get_index_space_offset();
    const int5 end = off + get_index_space_size();
    const int nh = NG * hpg;
#ifdef FP8
    DECL_IDX(IDX)
#endif
    for (int t = off[0]; t < end[0]; t++) {
#ifdef FP8
        const float64 ps_t = TOKEN_SCALE(ps, t, IDX[0]);
        float64 amax = 0.0f;
#endif
        int g = 0, hi = 0;                 // group / index within group of head h
        for (int h = 0; h < nh; h++) {
            for (int blk = 0; blk < 2; blk++) {
                int5 oc = {blk * 128, hi + hpg * t, 0, 0, 0};
                float64 a = 0.0f, b = 0.0f;
                GROUP_ADD(g, on0, on1, on2, on3, oc, a, b)
#ifndef NO_PAST
                GROUP_ADD(g, op0, op1, op2, op3, oc, a, b)
#endif
                int5 gc = {512 * h + 256 + blk * 128, t, 0, 0, 0};
#ifdef FP8
                const float64 ga = v_f32_ld_tnsr_b(gc, p) * ps_t;
                gc[0] += 64;
                const float64 gb = v_f32_ld_tnsr_b(gc, p) * ps_t;
                const int vi = (256 * h + blk * 128) >> 6;
                GB[vi] = a * nl_sigmoid(ga);
                GB[vi + 1] = b * nl_sigmoid(gb);
                amax = v_f32_max_b(amax, v_f32_max_b(v_f32_abs_b(GB[vi]), v_f32_abs_b(GB[vi + 1])));
#else
                const float64 ga = v_f32_ld_tnsr_b(gc, p);
                gc[0] += 64;
                const float64 gb = v_f32_ld_tnsr_b(gc, p);
                int5 dc = {256 * h + blk * 128, t, 0, 0, 0};
                v_bf16_st_tnsr(dc, out, pack_sigma(a * nl_sigmoid(ga), b * nl_sigmoid(gb)));
#endif
            }
            if (++hi == hpg) { hi = 0; g++; }
        }
#ifdef FP8
        NL_REDUCE_MAX(amax, IDX, amax);
        const float64 sc = v_f32_max_b(amax, 1e-30f) * (1.0f / FP8_MAX);
        const float64 inv = nl_recip(sc);
        for (int v = 0; v < nh * 4; v += 4) {
            int5 dc = {v * 64, t, 0, 0, 0};
            v_f8_st_tnsr(dc, out, pack_tau(GB[v] * inv, GB[v + 1] * inv, GB[v + 2] * inv, GB[v + 3] * inv));
        }
        STORE_TOKEN_SCALE(os, t, sc)
#endif
    }
}
#endif  // !SDPA

#ifdef SDPA
// SDPA build: the attention output is stacked, o4 (256, T, hpg, G) f32 (d in sigma order); no past part.
// tensors: o4 p [ps] | out (bf16)  or, FP8: o4 p ps | out8 os
#ifdef FP8
__local__ float64 GB[128];
void main(tensor o4, tensor p, tensor ps, tensor out, tensor os, int hpg)
#else
void main(tensor o4, tensor p, tensor out, int hpg)
#endif
{
    const int5 off = get_index_space_offset();
    const int5 end = off + get_index_space_size();
    const int nh = get_dim_size(o4, 3) * hpg;
#ifdef FP8
    DECL_IDX(IDX)
#endif
    for (int t = off[0]; t < end[0]; t++) {
#ifdef FP8
        const float64 ps_t = TOKEN_SCALE(ps, t, IDX[0]);
        float64 amax = 0.0f;
#endif
        int g = 0, hi = 0;
        for (int h = 0; h < nh; h++) {
            for (int blk = 0; blk < 2; blk++) {
                int5 oc = {blk * 128, t, hi, g, 0};
                const float64 a = v_f32_ld_tnsr_b(oc, o4);
                oc[0] += 64;
                const float64 b = v_f32_ld_tnsr_b(oc, o4);
                int5 gc = {512 * h + 256 + blk * 128, t, 0, 0, 0};
#ifdef FP8
                const float64 ga = v_f32_ld_tnsr_b(gc, p) * ps_t;
                gc[0] += 64;
                const float64 gb = v_f32_ld_tnsr_b(gc, p) * ps_t;
                const int vi = (256 * h + blk * 128) >> 6;
                GB[vi] = a * nl_sigmoid(ga);
                GB[vi + 1] = b * nl_sigmoid(gb);
                amax = v_f32_max_b(amax, v_f32_max_b(v_f32_abs_b(GB[vi]), v_f32_abs_b(GB[vi + 1])));
#else
                const float64 ga = v_f32_ld_tnsr_b(gc, p);
                gc[0] += 64;
                const float64 gb = v_f32_ld_tnsr_b(gc, p);
                int5 dc = {256 * h + blk * 128, t, 0, 0, 0};
                v_bf16_st_tnsr(dc, out, pack_sigma(a * nl_sigmoid(ga), b * nl_sigmoid(gb)));
#endif
            }
            if (++hi == hpg) { hi = 0; g++; }
        }
#ifdef FP8
        NL_REDUCE_MAX(amax, IDX, amax);
        const float64 sc = v_f32_max_b(amax, 1e-30f) * (1.0f / FP8_MAX);
        const float64 inv = nl_recip(sc);
        for (int v = 0; v < nh * 4; v += 4) {
            int5 dc = {v * 64, t, 0, 0, 0};
            v_f8_st_tnsr(dc, out, pack_tau(GB[v] * inv, GB[v + 1] * inv, GB[v + 2] * inv, GB[v + 3] * inv));
        }
        STORE_TOKEN_SCALE(os, t, sc)
#endif
    }
}
#endif  // SDPA
