// SPDX-License-Identifier: AGPL-3.0-only
// Copyright Aaron Botsis
// Attention prologue for one token per member (NG = number of KV groups, compile-time):
//   q_h = rope(rmsnorm_256(q_h) * qn_w)  -> q_g bf16 sigma order, (256, hpg*T), column = h%hpg + hpg*t
//   k_g = rope(rmsnorm_256(k_g) * kn_w)  -> kn_g row t           (chunk positions, natural order)
//   v_g                                   -> vn_g row sigma_inv(t)
// RoPE (NeoX, first 64 dims): out = x*cosv + swap_halves(x)*sinv,
//   cs row = [cos(32) cos(32) | -sin(32) sin(32)] for the token's position.
// tensors: 0 p (512 H + 512 NG, T) f32  [q0 g0 .. q_{H-1} g_{H-1} | k_0.. | v_0..] x 256, H = NG*hpg
//          1 qk_norm (512) f32 [q_norm | k_norm]  2 cs (128,T) f32  3 meta i32
//        | q_0..q_{NG-1} (256, hpg*T) bf16, kn_0.. (256,T) bf16, vn_0.. (256,T) bf16
// FP8 builds (-DFP8): p is an FP8-GEMM output with per-token scale ps (1,T) (input 1), and the
// node is split: -DPART_Q writes only q_*, -DPART_KV only kn/vn.
// params: eps, hpg. index space: [T]
#include "common.h"

#ifndef NG
#define NG 2
#endif

static inline void norm_rope(float64 x[4], const float64 w[4], float64 cosv, float64 sinv, float eps)
{
    DECL_IDX(IDX)
    float64 acc = x[0] * x[0];
    acc = v_f32_mac_b(x[1], x[1], acc);
    acc = v_f32_mac_b(x[2], x[2], acc);
    acc = v_f32_mac_b(x[3], x[3], acc);
    float64 ss;
    NL_REDUCE_ADD(acc, IDX, ss);
    const float64 r = nl_rsqrt(ss * (1.0f / 256.0f) + eps);
    x[0] = x[0] * r * w[0];
    x[1] = x[1] * r * w[1];
    x[2] = x[2] * r * w[2];
    x[3] = x[3] * r * w[3];
    const float64 sw = SWAP_HALVES(x[0]);
    x[0] = v_f32_mac_b(sw, sinv, x[0] * cosv);
}

static inline void load_head(tensor p, int ch, int t, float64 x[4], float64 s)
{
    int5 c = {ch, t, 0, 0, 0};
    x[0] = v_f32_ld_tnsr_b(c, p) * s; c[0] += 64;
    x[1] = v_f32_ld_tnsr_b(c, p) * s; c[0] += 64;
    x[2] = v_f32_ld_tnsr_b(c, p) * s; c[0] += 64;
    x[3] = v_f32_ld_tnsr_b(c, p) * s;
}

static inline void store_head(tensor dst, int5 c, const float64 x[4])
{
    c[0] = 0;
    v_bf16_st_tnsr(c, dst, pack_sigma(x[0], x[1]));
    c[0] = 128;
    v_bf16_st_tnsr(c, dst, pack_sigma(x[2], x[3]));
}

// pick the group's tensor with a switch
#if NG == 1
#define GROUP_STORE(g, a0, a1, a2, a3, c, x) { store_head(a0, c, x); }
#elif NG == 2
#define GROUP_STORE(g, a0, a1, a2, a3, c, x) \
    { if ((g) == 0) store_head(a0, c, x); else store_head(a1, c, x); }
#elif NG == 4
#define GROUP_STORE(g, a0, a1, a2, a3, c, x) \
    { switch (g) { case 0: store_head(a0, c, x); break; case 1: store_head(a1, c, x); break; \
                   case 2: store_head(a2, c, x); break; default: store_head(a3, c, x); } }
#else
#error "NG must be 1, 2 or 4"
#endif

#ifdef FP8
#define PS_ARG tensor ps,
#else
#define PS_ARG
#endif
#ifndef SDPA
#if defined(FP8) && defined(PART_Q)
  #if NG == 1
void main(tensor p, PS_ARG tensor qkn, tensor cs, tensor meta, tensor q0, float eps, int hpg)
#define q1 q0
#define q2 q0
#define q3 q0
  #elif NG == 2
void main(tensor p, PS_ARG tensor qkn, tensor cs, tensor meta, tensor q0, tensor q1, float eps, int hpg)
#define q2 q0
#define q3 q0
  #else
void main(tensor p, PS_ARG tensor qkn, tensor cs, tensor meta, tensor q0, tensor q1, tensor q2, tensor q3,
          float eps, int hpg)
  #endif
#elif defined(FP8) && defined(PART_KV)
  #if NG == 1
void main(tensor p, PS_ARG tensor qkn, tensor cs, tensor meta, tensor kn0, tensor vn0, float eps, int hpg)
#define kn1 kn0
#define kn2 kn0
#define kn3 kn0
#define vn1 vn0
#define vn2 vn0
#define vn3 vn0
  #elif NG == 2
void main(tensor p, PS_ARG tensor qkn, tensor cs, tensor meta, tensor kn0, tensor kn1, tensor vn0, tensor vn1,
          float eps, int hpg)
#define kn2 kn0
#define kn3 kn0
#define vn2 vn0
#define vn3 vn0
  #else
void main(tensor p, PS_ARG tensor qkn, tensor cs, tensor meta, tensor kn0, tensor kn1, tensor kn2, tensor kn3,
          tensor vn0, tensor vn1, tensor vn2, tensor vn3, float eps, int hpg)
  #endif
#elif NG == 1
void main(tensor p, tensor qkn, tensor cs, tensor meta, tensor q0, tensor kn0, tensor vn0, float eps, int hpg)
#define q1 q0
#define q2 q0
#define q3 q0
#define kn1 kn0
#define kn2 kn0
#define kn3 kn0
#define vn1 vn0
#define vn2 vn0
#define vn3 vn0
#elif NG == 2
void main(tensor p, tensor qkn, tensor cs, tensor meta,
          tensor q0, tensor q1, tensor kn0, tensor kn1, tensor vn0, tensor vn1, float eps, int hpg)
#define q2 q0
#define q3 q0
#define kn2 kn0
#define kn3 kn0
#define vn2 vn0
#define vn3 vn0
#else
void main(tensor p, tensor qkn, tensor cs, tensor meta,
          tensor q0, tensor q1, tensor q2, tensor q3, tensor kn0, tensor kn1, tensor kn2, tensor kn3,
          tensor vn0, tensor vn1, tensor vn2, tensor vn3, float eps, int hpg)
#endif
{
    const int5 off = get_index_space_offset();
    const int5 end = off + get_index_space_size();
    const int nh = NG * hpg;
    float64 qw[4], kw[4];
    int5 wc = {0, 0, 0, 0, 0};
    #pragma unroll
    for (int i = 0; i < 4; i++) {
        wc[0] = i * 64;
        qw[i] = v_f32_ld_tnsr_b(wc, qkn);
        wc[0] = 256 + i * 64;
        kw[i] = v_f32_ld_tnsr_b(wc, qkn);
    }
    for (int t = off[0]; t < end[0]; t++) {
        int5 cc = {0, t, 0, 0, 0};
        const float64 cosv = v_f32_ld_tnsr_b(cc, cs);
        cc[0] = 64;
        const float64 sinv = v_f32_ld_tnsr_b(cc, cs);
        float64 x[4];
#ifdef FP8
        DECL_IDX(IDX)
        const float64 sc = TOKEN_SCALE(ps, t, IDX[0]);
#else
        const float64 sc = 1.0f;
#endif
#if !defined(FP8) || defined(PART_Q)
        int g = 0, hi = 0;                 // group / index within group of head h
        for (int h = 0; h < nh; h++) {
            load_head(p, 512 * h, t, x, sc);
            norm_rope(x, qw, cosv, sinv, eps);
            int5 oc = {0, hi + hpg * t, 0, 0, 0};
            GROUP_STORE(g, q0, q1, q2, q3, oc, x)
            if (++hi == hpg) { hi = 0; g++; }
        }
#endif
#if !defined(FP8) || defined(PART_KV)
        const int vslot = sigma_inv(t);
        for (int g = 0; g < NG; g++) {
            load_head(p, 512 * nh + 256 * g, t, x, sc);
            norm_rope(x, kw, cosv, sinv, eps);
            int5 kcrd = {0, t, 0, 0, 0};
            GROUP_STORE(g, kn0, kn1, kn2, kn3, kcrd, x)
            load_head(p, 512 * nh + 256 * NG + 256 * g, t, x, sc);
            int5 vcrd = {0, vslot, 0, 0, 0};
            GROUP_STORE(g, vn0, vn1, vn2, vn3, vcrd, x)
        }
#endif
    }
#if !defined(FP8) || defined(PART_KV)
    // V rows may hold 128 key slots (sigma packing): the slots of keys >= T are zeroed
    const int Tk = get_dim_size(kn0, 1), Tv = get_dim_size(vn0, 1);
    if (off[0] == 0 && Tv > Tk) {
        float64 z[4] = {0.0f, 0.0f, 0.0f, 0.0f};
        for (int t = Tk; t < Tv; t++) {
            int5 vcrd = {0, sigma_inv(t), 0, 0, 0};
            for (int g = 0; g < NG; g++)
                GROUP_STORE(g, vn0, vn1, vn2, vn3, vcrd, z)
        }
    }
#endif
}
#endif  // !SDPA

#ifdef SDPA
// SDPA build (-DSDPA, prefill chunk without past): stacked outputs:
//   q4 (256, T, hpg, G) bf16, kn4 (256, T, 1, G) bf16 (natural rows),
//   vn (256, T, G) bf16 rows at sigma_inv(t), vs4 (256, T, 1, G) bf16 natural rows.
// tensors: p [ps] qkn cs meta | q4 kn4 vn vs4
#ifdef FP8
void main(tensor p, tensor ps, tensor qkn, tensor cs, tensor meta, tensor q4, tensor kn4, tensor vn, tensor vs4,
          float eps, int hpg)
#else
void main(tensor p, tensor qkn, tensor cs, tensor meta, tensor q4, tensor kn4, tensor vn, tensor vs4,
          float eps, int hpg)
#endif
{
    const int5 off = get_index_space_offset();
    const int5 end = off + get_index_space_size();
    const int G = get_dim_size(q4, 3);
    const int nh = G * hpg;
    float64 qw[4], kw[4];
    int5 wc = {0, 0, 0, 0, 0};
    #pragma unroll
    for (int i = 0; i < 4; i++) {
        wc[0] = i * 64;
        qw[i] = v_f32_ld_tnsr_b(wc, qkn);
        wc[0] = 256 + i * 64;
        kw[i] = v_f32_ld_tnsr_b(wc, qkn);
    }
#ifdef FP8
    DECL_IDX(IDX)
#endif
    for (int t = off[0]; t < end[0]; t++) {
        int5 cc = {0, t, 0, 0, 0};
        const float64 cosv = v_f32_ld_tnsr_b(cc, cs);
        cc[0] = 64;
        const float64 sinv = v_f32_ld_tnsr_b(cc, cs);
#ifdef FP8
        const float64 sc = TOKEN_SCALE(ps, t, IDX[0]);
#else
        const float64 sc = 1.0f;
#endif
        float64 x[4];
        int g = 0, hi = 0;
        for (int h = 0; h < nh; h++) {
            load_head(p, 512 * h, t, x, sc);
            norm_rope(x, qw, cosv, sinv, eps);
            int5 oc = {0, t, hi, g, 0};
            store_head(q4, oc, x);
            if (++hi == hpg) { hi = 0; g++; }
        }
        const int vslot = sigma_inv(t);
        for (int gg = 0; gg < G; gg++) {
            load_head(p, 512 * nh + 256 * gg, t, x, sc);
            norm_rope(x, kw, cosv, sinv, eps);
            int5 kc = {0, t, 0, gg, 0};
            store_head(kn4, kc, x);
            load_head(p, 512 * nh + 256 * G + 256 * gg, t, x, sc);
            int5 vc = {0, vslot, gg, 0, 0};
            store_head(vn, vc, x);
            int5 sc4 = {0, t, 0, gg, 0};
            store_head(vs4, sc4, x);
        }
    }
}
#endif  // SDPA
