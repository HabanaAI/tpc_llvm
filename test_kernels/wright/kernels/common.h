// SPDX-License-Identifier: AGPL-3.0-only
// Copyright Aaron Botsis
// Shared TPC-C helpers for Resin Gaudi kernels (Gaudi2).
#pragma once

#define WR_ALL MkWrA(0b11, 0b11, 0b11, 0b11)
// replicate 16-lane dual group dg of a into all four dual groups
#define BCAST_DG(a, dg) v_f32_mov_dual_group_all_b(a, 0xFFFFFFFF, dg, dg, dg, dg, WR_ALL, a)
// shuffle control (uniform across lanes): element e (0..15) of each dual group
#define SHUF_IDX(e) ((unsigned char)(0x80 | (((e) >> 3) << 5) | ((e) & 7)))
// every lane <- lane l (0..63) of v
static inline float64 lane_bcast(float64 v, int l)
{
    const float64 b = v_f32_shuffle_b(v, (uchar256)SHUF_IDX(l & 15), 0, v);
    const int dg = l >> 4;
    return dg == 0 ? BCAST_DG(b, 0) : dg == 1 ? BCAST_DG(b, 1) : dg == 2 ? BCAST_DG(b, 2) : BCAST_DG(b, 3);
}
// element (t, k) of a (T, K) f32 tensor in every lane: a vector read of 64 tokens and a broadcast
#define TOKEN_COEF(tensor_, t, k) lane_bcast(v_f32_ld_tnsr_b((int5){(t) & ~63, (k), 0, 0, 0}, tensor_), (t) & 63)
// swap the two 32-lane halves of a vector (dual groups 0,1 <-> 2,3)
#define SWAP_HALVES(a) v_f32_mov_dual_group_all_b(a, 0xFFFFFFFF, 2, 3, 0, 1, WR_ALL, a)

// f32 pair -> bf16 in "sigma order": out[2i] = a[i], out[2i+1] = b[i].
// With a = x[0..63], b = x[64..127] the stored block is x[sigma(s)],
// sigma(s) = s/2 (s even), 64 + s/2 (s odd).
static inline bfloat128 pack_sigma(float64 a, float64 b)
{
    float128 t;
    t.v1 = a;
    t.v2 = b;
    return v_convert_f32_to_bf16_all_b(t, SW_RHNE);
}

// ---- math.
static inline float64 nl_exp(float64 x)
{
    x = v_f32_max_b(v_f32_min_b(x, 88.0f), -87.0f);
    const float64 t = x * 1.4426950408889634f;                 // x / ln2
    const int64 n = v_convert_f32_to_i32_b(t, SW_RD, 0, 1, 0);    // floor
    const float64 nf = v_convert_i32_to_f32_b(n, SW_RHNE, 0, 1, 0);
    const float64 r = v_f32_mac_b(nf, -0.6931471805599453f, x); // x - n ln2, in [0, ln2)
    float64 pl = 1.0f / 5040.0f;
    pl = v_f32_mac_b(pl, r, 1.0f / 720.0f);
    pl = v_f32_mac_b(pl, r, 1.0f / 120.0f);
    pl = v_f32_mac_b(pl, r, 1.0f / 24.0f);
    pl = v_f32_mac_b(pl, r, 1.0f / 6.0f);
    pl = v_f32_mac_b(pl, r, 0.5f);
    pl = v_f32_mac_b(pl, r, 1.0f);
    pl = v_f32_mac_b(pl, r, 1.0f);
    const int64 e = v_i32_shl_b(n + 127, (int64)23);
    return pl * as_float64(e);
}

static inline float64 nl_recip(float64 x)       // x > 0 or x < 0, not 0
{
    float64 y = as_float64((int64)0x7EF311C3 - as_int64(x));
    y = y * (2.0f - x * y);
    y = y * (2.0f - x * y);
    y = y * (2.0f - x * y);
    return y;
}

static inline float64 nl_rsqrt(float64 x)       // x > 0
{
    float64 y = as_float64((int64)0x5F3759DF - v_i32_shr_b(as_int64(x), (int64)1));
    const float64 hx = x * 0.5f;
    y = y * (1.5f - hx * y * y);
    y = y * (1.5f - hx * y * y);
    y = y * (1.5f - hx * y * y);
    return y;
}

// Sum of all 64 lanes, broadcast to every lane. The macros declare s_, r0_ .. r3_:
// an argument of that name would be read uninitialized. IDX: uniform shuffle controls
// SHUF_IDX(0..15) held by the caller.
#define NL_REDUCE_(x, IDX, out, OP)                                                         \
    {                                                                                     \
        float64 s_ = OP((x), v_f32_mov_dual_group_all_b((x), 0xFFFFFFFF, 1, 0, 3, 2, WR_ALL, (x))); \
        s_ = OP(s_, v_f32_mov_dual_group_all_b(s_, 0xFFFFFFFF, 2, 3, 0, 1, WR_ALL, s_));     \
        float64 r0_ = v_f32_shuffle_b(s_, IDX[0], 0, s_), r1_ = v_f32_shuffle_b(s_, IDX[1], 0, s_); \
        float64 r2_ = v_f32_shuffle_b(s_, IDX[2], 0, s_), r3_ = v_f32_shuffle_b(s_, IDX[3], 0, s_); \
        _Pragma("unroll")                                                                 \
        for (int e_ = 4; e_ < 16; e_ += 4) {                                              \
            r0_ = OP(r0_, v_f32_shuffle_b(s_, IDX[e_], 0, s_));                            \
            r1_ = OP(r1_, v_f32_shuffle_b(s_, IDX[e_ + 1], 0, s_));                        \
            r2_ = OP(r2_, v_f32_shuffle_b(s_, IDX[e_ + 2], 0, s_));                        \
            r3_ = OP(r3_, v_f32_shuffle_b(s_, IDX[e_ + 3], 0, s_));                        \
        }                                                                                 \
        out = OP(OP(r0_, r1_), OP(r2_, r3_));                                             \
    }
// NL_REDUCE_ for i32 lanes (OP: v_i32_max_b, v_i32_min_b, ...)
#define NL_REDUCE_I32(x, IDX, out, OP)                                                      \
    {                                                                                     \
        int64 s_ = OP((x), v_i32_mov_dual_group_all_b((x), 0xFFFFFFFF, 1, 0, 3, 2, WR_ALL, (x))); \
        s_ = OP(s_, v_i32_mov_dual_group_all_b(s_, 0xFFFFFFFF, 2, 3, 0, 1, WR_ALL, s_));     \
        int64 r0_ = v_i32_shuffle_b(s_, IDX[0], 0, s_), r1_ = v_i32_shuffle_b(s_, IDX[1], 0, s_); \
        int64 r2_ = v_i32_shuffle_b(s_, IDX[2], 0, s_), r3_ = v_i32_shuffle_b(s_, IDX[3], 0, s_); \
        _Pragma("unroll")                                                                 \
        for (int e_ = 4; e_ < 16; e_ += 4) {                                              \
            r0_ = OP(r0_, v_i32_shuffle_b(s_, IDX[e_], 0, s_));                            \
            r1_ = OP(r1_, v_i32_shuffle_b(s_, IDX[e_ + 1], 0, s_));                        \
            r2_ = OP(r2_, v_i32_shuffle_b(s_, IDX[e_ + 2], 0, s_));                        \
            r3_ = OP(r3_, v_i32_shuffle_b(s_, IDX[e_ + 3], 0, s_));                        \
        }                                                                                 \
        out = OP(OP(r0_, r1_), OP(r2_, r3_));                                             \
    }
#define NL_ADD_(a, b) ((a) + (b))
#define NL_REDUCE_ADD(x, IDX, out) NL_REDUCE_(x, IDX, out, NL_ADD_)
#define NL_REDUCE_MAX(x, IDX, out) NL_REDUCE_(x, IDX, out, v_f32_max_b)

// Four independent 64-lane sums at once (tokens processed in groups of 4): out_k = sum of x_k,
// broadcast to all lanes.
#define NL_REDUCE4_ADD(x0, x1, x2, x3, IDX, o0, o1, o2, o3)                                  \
    {                                                                                     \
        float64 f_[4] = {(x0), (x1), (x2), (x3)};                                         \
        _Pragma("unroll")                                                                 \
        for (int k_ = 0; k_ < 4; k_++) {                                                  \
            f_[k_] = f_[k_] + v_f32_mov_dual_group_all_b(f_[k_], 0xFFFFFFFF, 1, 0, 3, 2, WR_ALL, f_[k_]); \
            f_[k_] = f_[k_] + v_f32_mov_dual_group_all_b(f_[k_], 0xFFFFFFFF, 2, 3, 0, 1, WR_ALL, f_[k_]); \
        }                                                                                 \
        /* dual group k <- token k's 16 partial sums (all dual groups hold them) */       \
        float64 p_ = v_f32_mov_dual_group_all_b(f_[1], 0xFFFFFFFF, 0, 1, 1, 1, MkWrA(0b00, 0b11, 0b00, 0b00), f_[0]); \
        p_ = v_f32_mov_dual_group_all_b(f_[2], 0xFFFFFFFF, 2, 2, 2, 2, MkWrA(0b00, 0b00, 0b11, 0b00), p_); \
        p_ = v_f32_mov_dual_group_all_b(f_[3], 0xFFFFFFFF, 3, 3, 3, 3, MkWrA(0b00, 0b00, 0b00, 0b11), p_); \
        float64 r0_ = v_f32_shuffle_b(p_, IDX[0], 0, p_), r1_ = v_f32_shuffle_b(p_, IDX[1], 0, p_); \
        float64 r2_ = v_f32_shuffle_b(p_, IDX[2], 0, p_), r3_ = v_f32_shuffle_b(p_, IDX[3], 0, p_); \
        _Pragma("unroll")                                                                 \
        for (int e_ = 4; e_ < 16; e_ += 4) {                                              \
            r0_ = r0_ + v_f32_shuffle_b(p_, IDX[e_], 0, p_);                                \
            r1_ = r1_ + v_f32_shuffle_b(p_, IDX[e_ + 1], 0, p_);                            \
            r2_ = r2_ + v_f32_shuffle_b(p_, IDX[e_ + 2], 0, p_);                            \
            r3_ = r3_ + v_f32_shuffle_b(p_, IDX[e_ + 3], 0, p_);                            \
        }                                                                                 \
        const float64 t_ = (r0_ + r1_) + (r2_ + r3_);   /* dual group k = sum of token k */ \
        o0 = BCAST_DG(t_, 0); o1 = BCAST_DG(t_, 1); o2 = BCAST_DG(t_, 2); o3 = BCAST_DG(t_, 3); \
    }

// silu with a 2-step Newton reciprocal (~2e-4 relative; activations only)
static inline float64 nl_silu2(float64 x)
{
    const float64 d = 1.0f + nl_exp(-x);
    float64 y = as_float64((int64)0x7EF311C3 - as_int64(d));
    y = y * (2.0f - d * y);
    y = y * (2.0f - d * y);
    return x * y;
}

// natural log for x > 0: x = m * 2^e, m in [sqrt(.5), sqrt(2)); log m via atanh series
static inline float64 nl_log(float64 x)
{
    int64 xi = as_int64(x);
    int64 e = v_i32_shr_b(xi, (int64)23) - 127;
    float64 m = as_float64((xi & 0x007FFFFF) | 0x3F800000);        // [1, 2)
    const float64 big = v_f32_sel_grt_f32_b(m, 1.41421356f, 1.0f, 0.0f);
    m = v_f32_sel_grt_f32_b(m, 1.41421356f, m * 0.5f, m);
    const float64 ef = v_convert_i32_to_f32_b(e, SW_RHNE, 0, 1, 0) + big;
    const float64 z = (m - 1.0f) * nl_recip(m + 1.0f);
    const float64 z2 = z * z;
    float64 pl = 1.0f / 9.0f;
    pl = v_f32_mac_b(pl, z2, 1.0f / 7.0f);
    pl = v_f32_mac_b(pl, z2, 1.0f / 5.0f);
    pl = v_f32_mac_b(pl, z2, 1.0f / 3.0f);
    pl = v_f32_mac_b(pl, z2, 1.0f);
    return v_f32_mac_b(ef, 0.6931471805599453f, 2.0f * z * pl);
}

static inline float64 nl_softplus(float64 x)      // log(1 + e^x), threshold 20
{
    return v_f32_sel_grt_f32_b(x, 20.0f, x, nl_log(1.0f + nl_exp(x)));
}

// shuffle controls for NL_REDUCE_*: declare once per kernel
#define DECL_IDX(IDX) uchar256 IDX[16]; _Pragma("unroll") for (int e_ = 0; e_ < 16; e_++) IDX[e_] = SHUF_IDX(e_);

static inline float64 nl_sigmoid(float64 x) { return nl_recip(1.0f + nl_exp(-x)); }
static inline float64 nl_silu(float64 x) { return x * nl_sigmoid(x); }
// overflow-free sigmoid for any finite x: exp(-|x|) in (0, 1], the reciprocal's argument in (1, 2]
static inline float64 nl_sigm(float64 x)
{
    const float64 s = nl_recip(1.0f + nl_exp(-v_f32_abs_b(x)));
    return v_f32_sel_less_f32_b(x, 0.0f, 1.0f - s, s);
}
static inline float64 nl_silus(float64 x) { return x * nl_sigm(x); }

// exp based sigmoid
static inline float64 sigmoid_exp(float64 x) { return v_reciprocal_f32(1.0f + v_exp_f32(-x)); }
// 4 x f32 (x[0..63], x[64..127], x[128..191], x[192..255]) -> fp8 e4m3 in
// "tau order": out[4i + j] = x[64 j + i].
static inline minifloat256 pack_tau(float64 a, float64 b, float64 c, float64 d)
{
    float256 t;
    t.v1 = a;
    t.v2 = b;
    t.v3 = c;
    t.v4 = d;
    return convert_float256_to_minifloat256(t, SW_RHNE);
}

// 4 x f32 (x[0..63], .., x[192..255]) -> fp8 e4m3 in natural order (out[i] = x[i]).
static inline minifloat256 pack_lin(float64 a, float64 b, float64 c, float64 d)
{
    float256 t;
    t.v1 = a;
    t.v2 = b;
    t.v3 = c;
    t.v4 = d;
    return convert_float256_to_minifloat256(t, SW_RHNE | SW_LINEAR);
}

// 2 x f32 (x[0..63], x[64..127]) -> 128 fp8 e4m3 bytes in "pair order": out[2i + j] = x[64 j + i] (lanes
// 0..127; store with v_f8_st_tnsr_partial(.., 127, 0)).
static inline minifloat256 pack_nat128(float64 a, float64 b)
{
    minifloat256 p = 0;
    p = v_convert_f32_to_f8_b(a, 0, SW_RHNE | SW_CLIP_FP, p);
    p = v_convert_f32_to_f8_b(b, 2, SW_RHNE | SW_CLIP_FP, p);
    const minifloat256 sp = p;
    p = v_f8_pack_b(sp, SW_GROUP_0 | SW_STRIDE_2, (minifloat256)0);
    p = v_f8_pack_b(sp, SW_GROUP_1 | SW_STRIDE_2, p);
    return v_f8_mov_dual_group_pack_b(p, SW_PACK21, (minifloat256)0);
}

// 2 x f32 (x[0..63], x[64..127]) -> bf16 in natural order (out[i] = x[i])
static inline bfloat128 cvt_bf16_lin(float64 a, float64 b)
{
    float128 t;
    t.v1 = a;
    t.v2 = b;
    return convert_float128_to_bfloat128(t, SW_RHNE | SW_LINEAR);
}

static inline float64 silu_f32(float64 x) { return x * v_sigmoid_f32(x); }
static inline float64 silu_exp(float64 x) { return x * sigmoid_exp(x); }

// cache reset
static inline void scalar_cache_reset() { cache_invalidate(SW_D | SW_RST_D_PREF); }

// per-token scalar of a (1, T) f32 tensor, broadcast to all lanes (IDX0 = SHUF_IDX(0) vector)
#define TOKEN_SCALE(s_t, t, IDX0) ({                                        \
    int5 c_ = {0, (t), 0, 0, 0};                                            \
    const float64 v_ = BCAST_DG(v_f32_ld_tnsr_b(c_, s_t), 0);               \
    v_f32_shuffle_b(v_, IDX0, 0, v_); })

// store a per-token scalar (all lanes equal) into a (1, T) f32 tensor
#define STORE_TOKEN_SCALE(s_t, t, v) { int5 c_ = {0, (t), 0, 0, 0}; v_f32_st_tnsr(c_, s_t, v); }

#define FP8_MAX 240.0f

// The per-token FP8 scale for a largest magnitude amax, and its reciprocal: amax / 240, or with POW2 the
// smallest power of two s with amax <= 240 s (its reciprocal is then exact).
#ifdef POW2
static inline float64 token_scale(float64 amax)
{
    const float64 a = v_f32_max_b(amax, 1e-30f);
    float64 r = as_float64((as_int64(a * (1.0f / FP8_MAX)) + 0x007FFFFF) & 0x7F800000);
    r = v_f32_sel_grt_f32_b(a, r * FP8_MAX, r * 2.0f, r);
    r = v_f32_sel_leq_f32_b(a, r * (0.5f * FP8_MAX), r * 0.5f, r);
    return r;
}
static inline float64 token_inv(float64 s) { return as_float64(0x7F000000 - as_int64(s)); }
#else
#define token_scale(amax) (v_f32_max_b((amax), 1e-30f) * (1.0f / FP8_MAX))
#define token_inv(s) nl_recip(s)
#endif

// SwiGLU input clamps of the *_lim kernel builds (GLM-5.3-Flash: gate <= lim, |up| <= lim; `lim` is the
// kernel's last parameter); identity otherwise
#ifdef LIM
#define CLAMP_G(x) v_f32_min_b((x), lim)
#define CLAMP_U(x) v_f32_max_b(v_f32_min_b((x), lim), -lim)
#else
#define CLAMP_G(x) (x)
#define CLAMP_U(x) (x)
#endif

// token of index-space position i over a power-of-2 bucket T: bit-reversed
static inline int tok_spread(int i, int T)
{
    if (T < 64 || (T & (T - 1)) != 0)
        return i;
    int r = 0;
    for (int b = 1; b < T; b <<= 1) {
        r = (r << 1) | (i & 1);
        i >>= 1;
    }
    return r;
}

static inline int meta_i32(tensor meta, int idx)
{
    int5 c = {idx, 0, 0, 0, 0};
    return s_i32_ld_g(gen_addr(c, meta));
}

// inverse of sigma inside a 128 block: natural index p -> stored slot
static inline int sigma_inv(int p)
{
    const int b = p & ~127, r = p & 127;
    return b + (r < 64 ? 2 * r : 2 * (r - 64) + 1);
}
