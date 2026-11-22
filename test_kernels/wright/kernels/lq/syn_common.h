// SPDX-License-Identifier: AGPL-3.0-only
// Copyright Aaron Botsis
// Shared TPC-C helpers for the lq kernels (Gaudi2).
//
// Conventions:
//  - every kernel operand is a flat 1-D tensor; views are described by scalar params
//    (element offset + element strides of dims 1..3, dim 0 is contiguous)
//  - scalar param 0 is the number of work members (the index space size)
#pragma once

#define WR_ALL MkWrA(0b11, 0b11, 0b11, 0b11)
// replicate 16-lane dual group dg of a into all four dual groups
static inline float64 undef_f64() { float64 u_; return u_; }
// uniform shuffle
#define SHUF(a, i) v_f32_shuffle_b((a), (i), 0, undef_f64())
#define BCAST_DG(a, dg) v_f32_mov_dual_group_all_b(a, 0xFFFFFFFF, dg, dg, dg, dg, WR_ALL, undef_f64())
// shuffle control (uniform across lanes): element e (0..15) of each dual group
#define SHUF_IDX(e) ((unsigned char)(0x80 | (((e) >> 3) << 5) | ((e) & 7)))
// swap the two 32-lane halves of a vector (dual groups 0,1 <-> 2,3)
#define SWAP_HALVES(a) v_f32_mov_dual_group_all_b(a, 0xFFFFFFFF, 2, 3, 0, 1, WR_ALL, undef_f64())
#define DECL_IDX(IDX) uchar256 IDX[16]; _Pragma("unroll") for (int e_ = 0; e_ < 16; e_++) IDX[e_] = SHUF_IDX(e_);

// lane 0 of a broadcast to all lanes
#define BCAST0(a, IDX) ({ const float64 b_ = BCAST_DG((a), 0); SHUF(b_, IDX[0]); })

// math
static inline float64 nl_exp(float64 x)
{
    x = v_f32_max_b(v_f32_min_b(x, 88.0f), -87.0f);
    const float64 t = x * 1.4426950408889634f;
    // round to nearest: |r| <= ln2/2 keeps the degree-7 polynomial at ~5e-9 relative error
    const int64 n = v_convert_f32_to_i32_b(t, SW_RHNE, 0, 1, 0);
    const float64 nf = v_convert_i32_to_f32_b(n, SW_RHNE, 0, 1, 0);
    const float64 r = v_f32_mac_b(nf, -0.6931471805599453f, x);
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

static inline float64 nl_recip(float64 x)
{
    float64 y = as_float64((int64)0x7EF311C3 - as_int64(x));
    y = y * (2.0f - x * y);
    y = y * (2.0f - x * y);
    y = y * (2.0f - x * y);
    return y;
}

// two Newton steps: ~5e-6 relative error
static inline float64 nl_rsqrt2(float64 x)
{
    float64 y = as_float64((int64)0x5F3759DF - v_i32_shr_b(as_int64(x), (int64)1));
    const float64 hx = x * 0.5f;
    y = y * (1.5f - hx * y * y);
    y = y * (1.5f - hx * y * y);
    return y;
}

static inline float64 nl_rsqrt(float64 x)
{
    float64 y = as_float64((int64)0x5F3759DF - v_i32_shr_b(as_int64(x), (int64)1));
    const float64 hx = x * 0.5f;
    y = y * (1.5f - hx * y * y);
    y = y * (1.5f - hx * y * y);
    y = y * (1.5f - hx * y * y);
    return y;
}

static inline float64 nl_log(float64 x)
{
    int64 xi = as_int64(x);
    int64 e = v_i32_shr_b(xi, (int64)23) - 127;
    float64 m = as_float64((xi & 0x007FFFFF) | 0x3F800000);
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

// overflow-free sigmoid; x < 0: e / (1 + e) (no cancellation)
static inline float64 nl_sigm(float64 x)
{
    const float64 e = nl_exp(-v_f32_abs_b(x));
    const float64 s = nl_recip(1.0f + e);
    return v_f32_sel_less_f32_b(x, 0.0f, e * s, s);
}
static inline float64 nl_silu(float64 x) { return x * nl_sigm(x); }
// correctly rounded (one fused residual step)
static inline float64 nl_sigm_acc(float64 x)
{
    const float64 e = nl_exp(-v_f32_abs_b(x));
    const float64 d = 1.0f + e;
    float64 s = nl_recip(d);
    s = v_f32_mac_b(s, v_f32_mac_b(d, -s, (float64) 1.0f), s);
    return v_f32_sel_less_f32_b(x, 0.0f, e * s, s);
}
static inline float64 nl_silu_acc(float64 x) { return x * nl_sigm_acc(x); }
// nl_sigm_acc variant: e = exp(-|x|) (|x| clamped at 87) by a degree-6 minimax
// polynomial in the reduced argument (2e-9 relative), 1 / (1 + e) on [1, 2] from a quadratic start (1e-2)
// and two Newton steps
static inline float64 nl_sigm_f(float64 x)
{
    const float64 ax = v_f32_min_b(v_f32_abs_b(x), 87.0f);
    const int64 n = v_convert_f32_to_i32_b(ax * 1.4426950408889634f, SW_RHNE, 0, 1, 0);
    const float64 nf = v_convert_i32_to_f32_b(n, SW_RHNE, 0, 1, 0);
    // exp(-ax) = 2^-n exp(-q), q = ax - n ln2
    const float64 q = v_f32_mac_b(nf, 0.6931471805599453f, ax, SW_NEG, 1, 0);
    float64 pl = 0.001383684459142387f;
    pl = v_f32_mac_b(pl, q, -0.008374815806746483f);
    pl = v_f32_mac_b(pl, q, 0.04166822507977486f);
    pl = v_f32_mac_b(pl, q, -0.16666419804096222f);
    pl = v_f32_mac_b(pl, q, 0.49999991059303284f);
    pl = v_f32_mac_b(pl, q, -1.0f);
    pl = v_f32_mac_b(pl, q, 1.0f);
    const float64 e = pl * as_float64(v_i32_shl_b((int64) 127 - n, (int64) 23));
    const float64 d = 1.0f + e;
    float64 y = v_f32_mac_b(v_f32_mac_b(d, 0.3232311010360718f, (float64) -1.4545421600341797f), d, (float64) 2.1212100982666016f);
    y = v_f32_mac_b(y, v_f32_mac_b(d, y, (float64) 1.0f, SW_NEG, 1, 0), y);
    y = v_f32_mac_b(y, v_f32_mac_b(d, y, (float64) 1.0f, SW_NEG, 1, 0), y);
    return v_f32_sel_less_f32_b(x, 0.0f, e * y, y);
}
// gelu (tanh form) = x * sigmoid(2 sqrt(2 / pi) x (1 + 0.044715 x^2))
static inline float64 nl_gelu_f(float64 x)
{
    return x * nl_sigm_f(x * 1.5957691216057308f * v_f32_mac_b(x * x, 0.044715f, (float64) 1.0f));
}
// softplus: log1p(exp(x)), identity above 20
static inline float64 nl_softplus(float64 x)
{
    return v_f32_sel_grt_f32_b(x, 20.0f, x, nl_log(1.0f + nl_exp(x)));
}
static inline float64 nl_tanh(float64 x)
{
    const float64 ax = v_f32_min_b(v_f32_abs_b(x), 15.0f);
    const float64 e = nl_exp(-2.0f * ax);
    const float64 t = (1.0f - e) * nl_recip(1.0f + e);
    return v_f32_sel_less_f32_b(x, 0.0f, -t, t);
}

// sum / max of all 64 lanes, broadcast to every lane
#define NL_REDUCE_(x, IDX, out, OP)                                                         \
    {                                                                                     \
        float64 s_ = OP((x), v_f32_mov_dual_group_all_b((x), 0xFFFFFFFF, 1, 0, 3, 2, WR_ALL, undef_f64())); \
        s_ = OP(s_, v_f32_mov_dual_group_all_b(s_, 0xFFFFFFFF, 2, 3, 0, 1, WR_ALL, undef_f64()));     \
        float64 r0_ = SHUF(s_, IDX[0]), r1_ = SHUF(s_, IDX[1]); \
        float64 r2_ = SHUF(s_, IDX[2]), r3_ = SHUF(s_, IDX[3]); \
        _Pragma("unroll")                                                                 \
        for (int e_ = 4; e_ < 16; e_ += 4) {                                              \
            r0_ = OP(r0_, SHUF(s_, IDX[e_]));                            \
            r1_ = OP(r1_, SHUF(s_, IDX[e_ + 1]));                        \
            r2_ = OP(r2_, SHUF(s_, IDX[e_ + 2]));                        \
            r3_ = OP(r3_, SHUF(s_, IDX[e_ + 3]));                        \
        }                                                                                 \
        out = OP(OP(r0_, r1_), OP(r2_, r3_));                                             \
    }
#define NL_ADD_(a, b) ((a) + (b))
#define NL_REDUCE_ADD(x, IDX, out) NL_REDUCE_(x, IDX, out, NL_ADD_)
#define NL_REDUCE_MAX(x, IDX, out) NL_REDUCE_(x, IDX, out, v_f32_max_b)

// four independent 64-lane sums / maxima at once
#define NL_REDUCE4_(x0, x1, x2, x3, IDX, o0, o1, o2, o3, OP)                                  \
    {                                                                                     \
        float64 f_[4] = {(x0), (x1), (x2), (x3)};                                         \
        _Pragma("unroll")                                                                 \
        for (int k_ = 0; k_ < 4; k_++) {                                                  \
            f_[k_] = OP(f_[k_], v_f32_mov_dual_group_all_b(f_[k_], 0xFFFFFFFF, 1, 0, 3, 2, WR_ALL, undef_f64())); \
            f_[k_] = OP(f_[k_], v_f32_mov_dual_group_all_b(f_[k_], 0xFFFFFFFF, 2, 3, 0, 1, WR_ALL, undef_f64())); \
        }                                                                                 \
        float64 p_ = v_f32_mov_dual_group_all_b(f_[1], 0xFFFFFFFF, 0, 1, 1, 1, MkWrA(0b00, 0b11, 0b00, 0b00), f_[0]); \
        p_ = v_f32_mov_dual_group_all_b(f_[2], 0xFFFFFFFF, 2, 2, 2, 2, MkWrA(0b00, 0b00, 0b11, 0b00), p_); \
        p_ = v_f32_mov_dual_group_all_b(f_[3], 0xFFFFFFFF, 3, 3, 3, 3, MkWrA(0b00, 0b00, 0b00, 0b11), p_); \
        float64 r0_ = SHUF(p_, IDX[0]), r1_ = SHUF(p_, IDX[1]); \
        float64 r2_ = SHUF(p_, IDX[2]), r3_ = SHUF(p_, IDX[3]); \
        _Pragma("unroll")                                                                 \
        for (int e_ = 4; e_ < 16; e_ += 4) {                                              \
            r0_ = OP(r0_, SHUF(p_, IDX[e_]));                                \
            r1_ = OP(r1_, SHUF(p_, IDX[e_ + 1]));                            \
            r2_ = OP(r2_, SHUF(p_, IDX[e_ + 2]));                            \
            r3_ = OP(r3_, SHUF(p_, IDX[e_ + 3]));                            \
        }                                                                                 \
        const float64 t_ = OP(OP(r0_, r1_), OP(r2_, r3_));                                     \
        o0 = BCAST_DG(t_, 0); o1 = BCAST_DG(t_, 1); o2 = BCAST_DG(t_, 2); o3 = BCAST_DG(t_, 3); \
    }
#define NL_REDUCE4_ADD(x0, x1, x2, x3, IDX, o0, o1, o2, o3) NL_REDUCE4_(x0, x1, x2, x3, IDX, o0, o1, o2, o3, NL_ADD_)
#define NL_REDUCE4_MAX(x0, x1, x2, x3, IDX, o0, o1, o2, o3) NL_REDUCE4_(x0, x1, x2, x3, IDX, o0, o1, o2, o3, v_f32_max_b)

// cache reset
static inline void scalar_cache_reset() { cache_invalidate(SW_D | SW_RST_D_PREF); }

// lane index as f32 / i32
#define LANE_I32() ((int64) read_lane_id_4b_b())
#define LANE_F32() v_convert_i32_to_f32_b((int64) read_lane_id_4b_b(), SW_RHNE, 0, 1, 0)

// store n lanes (1..64) of a f32 vector at flat coordinate c0
#define ST_F32(t, c0, v, n)                                                      \
    {                                                                            \
        int5 c_ = {(c0), 0, 0, 0, 0};                                            \
        if ((n) >= 64) v_f32_st_tnsr(c_, t, (v));                                \
        else v_f32_st_tnsr_partial(c_, t, (v), (n) - 1, 0);                      \
    }

static inline float64 ld_f32(tensor t, int c0)
{
    int5 c = {c0, 0, 0, 0, 0};
    return v_f32_ld_tnsr_b(c, t);
}

// f32 <-> bf16 / f16 in natural lane order. a vector of 128 bf16 holds elements c0..c0+127
static inline bfloat128 cvt_bf16(float64 a, float64 b)
{
    float128 t;
    t.v1 = a;
    t.v2 = b;
    return convert_float128_to_bfloat128(t, SW_RHNE | SW_LINEAR);
}
static inline half128 cvt_f16(float64 a, float64 b)
{
    float128 t;
    t.v1 = a;
    t.v2 = b;
    return convert_float128_to_half128(t, SW_RHNE | SW_LINEAR);
}
static inline float128 bf16_to_f32(bfloat128 x) { return convert_bfloat128_to_float128(x, SW_LINEAR); }
static inline float128 f16_to_f32(half128 x) { return convert_half128_to_float128(x, SW_LINEAR); }

// split [0, n) into nm parts; part m = [a, b)
// member m's item range for a host-computed chunk = ceil(n / nmem)
#define SPLIT_CHUNK(n, chunk, m, a, b)                    \
    {                                                     \
        a = (m) * (chunk);                                \
        b = a + (chunk);                                  \
        if (a > (n)) a = (n);                             \
        if (b > (n)) b = (n);                             \
    }

// a / b for small non-negative ints, inv = 1.0f / b from the host (exact below 2^22)
#define FDIV(a, inv) s_convert_f32_to_i32(((float) (a) + 0.5f) * (inv), SW_RD)

#define SPLIT_RANGE(n, nm, m, a, b)                       \
    {                                                     \
        const int per_ = ((n) + (nm) - 1) / (nm);         \
        a = (m) * per_;                                   \
        b = a + per_;                                     \
        if (a > (n)) a = (n);                             \
        if (b > (n)) b = (n);                             \
    }

// store n (1..64) results to the kernel's output: f32, or bf16 / f16 when built with -DOUT_BF16 / -DOUT_F16
#if defined(OUT_BF16)
#define ST_OUT(t, c0, v, n)                                                                  \
    {                                                                                        \
        int5 c_ = {(c0), 0, 0, 0, 0};                                                        \
        v_bf16_st_tnsr_partial(c_, t, cvt_bf16((v), (v)), ((n) >= 64 ? 64 : (n)) - 1, 0);    \
    }
#elif defined(OUT_F16)
#define ST_OUT(t, c0, v, n)                                                                  \
    {                                                                                        \
        int5 c_ = {(c0), 0, 0, 0, 0};                                                        \
        v_f16_st_tnsr_partial(c_, t, cvt_f16((v), (v)), ((n) >= 64 ? 64 : (n)) - 1, 0);      \
    }
#else
#define ST_OUT(t, c0, v, n) ST_F32(t, c0, v, n)
#endif

// fp8_143 codes of rows for an fp8 GEMM with a scale per row (token). FP8_SCALE: s_ = max(mx, 1e-30) / 240 (mx: the row's
// max |y| in every lane), inv_ = 1 / s_; p2: s_ = the power of two >= that, inv_ exact.
// FP8_ROW: q[c0 + e] = fp8(y[e] inv_) for the row y (XV[0 .. ne0 / 64),
// ne0 % 256 == 0)
#define FP8_SCALE(mx, p2, s_, inv_)                                                              \
    {                                                                                            \
        s_ = v_f32_max_b((mx), 1e-30f) * (1.0f / 240.0f);                                       \
        if (p2) {                                                                                \
            s_ = as_float64((as_int64(s_) + 0x007FFFFF) & (int) 0xFF800000);                     \
            inv_ = as_float64((int64) 0x7F000000 - as_int64(s_));                                \
        } else {                                                                                 \
            inv_ = nl_recip(s_);                                                                 \
        }                                                                                        \
    }
#define FP8_ROW(XV, ne0, inv_, q, c0)                                                            \
    {                                                                                            \
        _Pragma("unroll 1")                                                                      \
        for (int e_ = 0; e_ < (ne0); e_ += 256) {                                                \
            float256 t_;                                                                         \
            t_.v1 = v_f32_min_b(v_f32_max_b(XV[e_ >> 6] * (inv_), -240.0f), 240.0f);             \
            t_.v2 = v_f32_min_b(v_f32_max_b(XV[(e_ >> 6) + 1] * (inv_), -240.0f), 240.0f);       \
            t_.v3 = v_f32_min_b(v_f32_max_b(XV[(e_ >> 6) + 2] * (inv_), -240.0f), 240.0f);       \
            t_.v4 = v_f32_min_b(v_f32_max_b(XV[(e_ >> 6) + 3] * (inv_), -240.0f), 240.0f);       \
            int5 c_ = {(c0) + e_, 0, 0, 0, 0};                                                   \
            v_f8_st_tnsr(c_, q, convert_float256_to_minifloat256(t_, SW_RHNE | SW_LINEAR));      \
        }                                                                                        \
    }
