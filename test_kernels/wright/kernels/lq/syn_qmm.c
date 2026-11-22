// SPDX-License-Identifier: AGPL-3.0-only
// Copyright Aaron Botsis
// Quantized weights in the planar layout, one type per build (-DQT_<type>):
//  -DDEQ: dequantize to bf16 W^T [M (fastest), K, nmat]; item = (row block, superblock)
//  else : GEMV out[:, col] = W_e x_col: bf16 codes x bf16 x, f32 accumulation;
//         item = (RB row blocks, column, K split): nks > 1 writes partial sums out[ks][col][M] (summed
//         by syn_ksum). has_ids: column col = t * nu + s uses matrix ids[s, t] and
//         x[:, s (or 0), t]; else matrix 0 and x column col at offx + col * sx1.
// Planar superblock (128 rows): [f16 param planes x 256 B][byte param planes x 128 B][code planes];
// a code chunk (4 k-steps) converts to two bf16 vectors per byte half, one k-step each,
// row i in lane 2 i and row 64 + i in lane 2 i + 1.
// -DQT_FP8: fp8_143 128-row blocks [128 x f32 scale][K/2 x 256 B, same lanes], no superblocks.
// W is 2-D [bytes per matrix, matrices]: coordinate 1 = matrix,
// held in WY[0] while a matrix is read; offsets are within the matrix.
// tensors: GEMV 0 W  1 x  2 ids | 3 out;  DEQ 0 W | 1 out
#include "syn_common.h"

__local__ int WY[1];

#if defined(QT_Q4_0)
#define SB 32
#define G 32
#define NF16 1
#define NBP 0
#define P4 1
#elif defined(QT_Q4_1)
#define SB 32
#define G 32
#define NF16 2
#define NBP 0
#define P4 1
#elif defined(QT_Q5_0)
#define SB 32
#define G 32
#define NF16 1
#define NBP 0
#define P4 1
#define P1 1
#elif defined(QT_Q5_1)
#define SB 32
#define G 32
#define NF16 2
#define NBP 0
#define P4 1
#define P1 1
#elif defined(QT_Q2_K)
#define SB 256
#define G 16
#define NF16 2
#define NBP 16
#define P2 1
#elif defined(QT_Q3_K)
#define SB 256
#define G 16
#define NF16 1
#define NBP 12
#define P2 1
#define P1 1
#elif defined(QT_Q4_K)
#define SB 256
#define G 32
#define NF16 2
#define NBP 12
#define P4 1
#elif defined(QT_Q5_K)
#define SB 256
#define G 32
#define NF16 2
#define NBP 12
#define P4 1
#define P1 1
#elif defined(QT_Q6_K)
#define SB 256
#define G 16
#define NF16 1
#define NBP 16
#define P4 1
#define P2 1
#elif defined(QT_IQ4_NL)
#define SB 32
#define G 32
#define NF16 1
#define NBP 0
#define P4 1
#define LUT_IQ4 1
#elif defined(QT_IQ4_XS)
#define SB 256
#define G 32
#define NF16 1
#define NBP 6
#define P4 1
#define LUT_IQ4 1
#elif defined(QT_MXFP4)
#define SB 32
#define G 32
#define NF16 0
#define NBP 1
#define P4 1
#define LUT_FP4 1
#elif defined(QT_NVFP4)
#define SB 64
#define G 16
#define NF16 0
#define NBP 4
#define P4 1
#define LUT_FP4 1
#elif defined(QT_TQ2_0)
#define SB 256
#define G 256
#define NF16 1
#define NBP 0
#define P2 1
#elif defined(QT_Q1_0)
#define SB 128
#define G 128
#define NF16 1
#define NBP 0
#define P1 1
#elif defined(QT_Q2_0)
#define SB 64
#define G 64
#define NF16 1
#define NBP 0
#define P2 1
#elif defined(QT_Q8_0)
#define SB 32
#define G 32
#define NF16 1
#define NBP 0
#define P8 1
#elif defined(QT_FP8)
#else
#error "no QT_ type"
#endif

#if !defined(QT_FP8)
#ifndef P4
#define P4 0
#endif
#ifndef P2
#define P2 0
#endif
#ifndef P1
#define P1 0
#endif
#ifndef P8
#define P8 0   // 8-bit signed codes, 2 k-steps per 256 B (q8_0)
#endif
#define NG (SB / G)
#define KU (G > 32 ? G : 32)   // K split unit of the GEMVs: whole groups and 32-step x groups
#define PBYTES (NF16 * 256 + NBP * 128)
#define SBB (PBYTES + P4 * (SB / 4) * 256 + P2 * (SB / 8) * 256 + P1 * (SB / 16) * 256 + P8 * (SB / 2) * 256)
#define S2 (P4 ? 4 : 0)
#define S1 (P4 ? 4 : (P2 ? 2 : 0))

__local__ float64 S[2 * NG];
__local__ float64 MN[2 * NG];

static inline float128 ld_f16p(tensor W, int p)
{
    int5 c = {p, WY[0], 0, 0, 0};
    return f16_to_f32(v_f16_ld_tnsr_b(c, W));
}

static inline uchar256 ld_u8(tensor W, int p)
{
    int5 c = {p, WY[0], 0, 0, 0};
    return v_u8_ld_tnsr_b(c, W);
}

static inline float256 u2f(uchar256 u)
{
    return convert_uchar256_to_float256(u, SW_LINEAR);
}

#define BP(b) ld_u8(W, pb + NF16 * 256 + 128 * (b))
#define SET_SM(g, s0, s1, m0, m1) { S[2 * (g)] = (s0); S[2 * (g) + 1] = (s1); MN[2 * (g)] = (m0); MN[2 * (g) + 1] = (m1); }

// scale / offset of group g of the superblock at byte offset pb (w = S * code + MN), as
// {S rows 0..63, S rows 64..127, MN rows 0..63, MN rows 64..127}
static inline float256 qt_gp(tensor W, int pb, int g)
{
    float256 r;
#if defined(QT_Q4_0) || defined(QT_Q5_0) || defined(QT_IQ4_NL) || defined(QT_TQ2_0) || defined(QT_Q2_0) || defined(QT_Q1_0) || defined(QT_Q8_0)
    const float128 d = ld_f16p(W, pb);
#if defined(QT_Q4_0)
    const float off = -8.0f, sm = 1.0f;
#elif defined(QT_Q5_0)
    const float off = -16.0f, sm = 1.0f;
#elif defined(QT_IQ4_NL) || defined(QT_Q8_0)
    const float off = 0.0f, sm = 1.0f;
#elif defined(QT_Q1_0)
    const float off = -1.0f, sm = 2.0f;
#else
    const float off = -1.0f, sm = 1.0f;
#endif
    r.v1 = d.v1 * sm;
    r.v2 = d.v2 * sm;
    r.v3 = d.v1 * off;
    r.v4 = d.v2 * off;
#elif defined(QT_Q4_1) || defined(QT_Q5_1)
    const float128 d = ld_f16p(W, pb), m = ld_f16p(W, pb + 256);
    r.v1 = d.v1; r.v2 = d.v2; r.v3 = m.v1; r.v4 = m.v2;
#elif defined(QT_MXFP4)
    // 2^(e - 128) (e >= 2; smaller e are denormal scales, flushed)
    const float256 e = u2f(BP(0));
    const int64 b1 = v_convert_f32_to_i32_b(e.v1, SW_RHNE, 0, 1, 0), b2 = v_convert_f32_to_i32_b(e.v2, SW_RHNE, 0, 1, 0);
    r.v1 = v_f32_sel_less_f32_b(e.v1, 2.0f, 0.0f, as_float64(v_i32_shl_b(b1 - 1, (int64) 23)));
    r.v2 = v_f32_sel_less_f32_b(e.v2, 2.0f, 0.0f, as_float64(v_i32_shl_b(b2 - 1, (int64) 23)));
    r.v3 = 0.0f; r.v4 = 0.0f;
#elif defined(QT_NVFP4)
    // unsigned e4m3 (bias 7) * 0.5; 0 and 0x7F are 0
    const uchar256 u = BP(g);
    const float256 ex = u2f(u >> 3), mn = u2f(u & (unsigned char) 7), raw = u2f(u);
    float64 s[2];
    #pragma unroll
    for (int h = 0; h < 2; h++) {
        const float64 e_ = h ? ex.v2 : ex.v1, m_ = h ? mn.v2 : mn.v1, r_ = h ? raw.v2 : raw.v1;
        const int64 ei = v_convert_f32_to_i32_b(e_, SW_RHNE, 0, 1, 0);
        const float64 p2 = as_float64(v_i32_shl_b(ei + 119, (int64) 23));   // 2^(e - 8)
        float64 v = v_f32_sel_eq_f32_b(e_, 0.0f, m_ * (1.0f / 1024.0f), (1.0f + m_ * 0.125f) * p2);
        s[h] = v_f32_sel_eq_f32_b(r_, 127.0f, 0.0f, v);
    }
    r.v1 = s[0]; r.v2 = s[1]; r.v3 = 0.0f; r.v4 = 0.0f;
#elif defined(QT_Q2_K)
    const float128 d = ld_f16p(W, pb), dm = ld_f16p(W, pb + 256);
    const uchar256 u = BP(g);
    const float256 sc = u2f(u & (unsigned char) 15), mm = u2f(u >> 4);
    r.v1 = d.v1 * sc.v1; r.v2 = d.v2 * sc.v2; r.v3 = -dm.v1 * mm.v1; r.v4 = -dm.v2 * mm.v2;
#elif defined(QT_Q3_K)
    const float128 d = ld_f16p(W, pb);
    const int b = g & 3, q = g >> 2;
    const uchar256 lo = BP((q & 1) ? 4 + b : b), hi = BP(8 + b);
    const uchar256 l4 = (q & 2) ? (lo >> 4) : (lo & (unsigned char) 15);
    const uchar256 h2 = (hi >> (2 * q)) & (unsigned char) 3;
    const float256 sc = u2f(l4 | (h2 << 4));
    r.v1 = d.v1 * (sc.v1 - 32.0f); r.v2 = d.v2 * (sc.v2 - 32.0f);
    r.v3 = -4.0f * r.v1; r.v4 = -4.0f * r.v2;
#elif defined(QT_Q4_K) || defined(QT_Q5_K)
    const float128 d = ld_f16p(W, pb), dm = ld_f16p(W, pb + 256);
    uchar256 sc, mm;
    if (g < 4) {
        sc = BP(g) & (unsigned char) 63;
        mm = BP(g + 4) & (unsigned char) 63;
    } else {
        const uchar256 a = BP(g + 4), b = BP(g - 4), c = BP(g);
        sc = (a & (unsigned char) 15) | ((b >> 6) << 4);
        mm = (a >> 4) | ((c >> 6) << 4);
    }
    const float256 fs = u2f(sc), fm = u2f(mm);
    r.v1 = d.v1 * fs.v1; r.v2 = d.v2 * fs.v2; r.v3 = -dm.v1 * fm.v1; r.v4 = -dm.v2 * fm.v2;
#elif defined(QT_Q6_K)
    const float128 d = ld_f16p(W, pb);
    int5 c = {pb + 256 + 128 * g, WY[0], 0, 0, 0};
    const float256 sc = convert_char256_to_float256(v_i8_ld_tnsr_b(c, W), SW_LINEAR);
    r.v1 = d.v1 * sc.v1; r.v2 = d.v2 * sc.v2; r.v3 = -32.0f * r.v1; r.v4 = -32.0f * r.v2;
#elif defined(QT_IQ4_XS)
    const float128 d = ld_f16p(W, pb);
    const uchar256 l = (BP(2 + (g >> 1)) >> (4 * (g & 1))) & (unsigned char) 15;
    const uchar256 h = (BP(g >> 2) >> (2 * (g & 3))) & (unsigned char) 3;
    const float256 ls = u2f(l | (h << 4));
    r.v1 = d.v1 * (ls.v1 - 32.0f); r.v2 = d.v2 * (ls.v2 - 32.0f); r.v3 = 0.0f; r.v4 = 0.0f;
#endif
    return r;
}

// every group of the superblock at pb into S / MN
static inline void qt_params(tensor W, int pb)
{
    for (int g = 0; g < NG; g++) {
        const float256 r = qt_gp(W, pb, g);
        SET_SM(g, r.v1, r.v2, r.v3, r.v4)
    }
}

#if defined(LUT_IQ4) || defined(LUT_FP4)
static inline char256 lut16(uchar256 q)
{
#if defined(LUT_IQ4)
    const int T[16] = {-127, -104, -83, -65, -49, -35, -22, -10, 1, 13, 25, 38, 53, 69, 89, 113};
#else
    const int T[16] = {0, 1, 2, 3, 4, 6, 8, 12, 0, -1, -2, -3, -4, -6, -8, -12};
#endif
    char256 v = (char) T[0];
    #pragma unroll
    for (int i = 1; i < 16; i++) {
        v = v_i8_sel_eq_u8_b(q, (unsigned char) i, (char) T[i], v);
    }
    return v;
}
// native lane order (no permutation): .v1 = even bytes, .v2 = odd bytes
#define CVT(u) convert_char256_to_bfloat256(lut16(u), 0)
#elif P8
// signed codes
#define CVT(u) convert_char256_to_bfloat256((char256) (u), 0)
#else
#define CVT(u) convert_uchar256_to_bfloat256(u, 0)
#endif

// codes of chunk c of the superblock at pb: FL.v1 / FL.v2 = k 4 c, 4 c + 1; FH.v1 / FH.v2 = 4 c + 2, 4 c + 3
#if P4
#define QT_C4(pb, c, lo, hi, p)                                                    \
    {                                                                              \
        const uchar256 u_ = ld_u8(W, p + 256 * (c));                               \
        lo = u_ & (unsigned char) 15;                                              \
        hi = u_ >> 4;                                                              \
        p += (SB / 4) * 256;                                                       \
    }
#else
#define QT_C4(pb, c, lo, hi, p)
#endif
#if P2
#define QT_C2(pb, c, lo, hi, p)                                                    \
    {                                                                              \
        const uchar256 u_ = ld_u8(W, p + 256 * ((c) >> 1));                        \
        const int sh_ = ((c) & 1) * 4;                                             \
        lo = lo | (((u_ >> sh_) & (unsigned char) 3) << S2);                       \
        hi = hi | (((u_ >> (sh_ + 2)) & (unsigned char) 3) << S2);                 \
        p += (SB / 8) * 256;                                                       \
    }
#else
#define QT_C2(pb, c, lo, hi, p)
#endif
#if P1
#define QT_C1(pb, c, lo, hi, p)                                                    \
    {                                                                              \
        const uchar256 u_ = ld_u8(W, p + 256 * ((c) >> 2));                        \
        const int sh_ = ((c) & 3) * 2;                                             \
        lo = lo | (((u_ >> sh_) & (unsigned char) 1) << S1);                       \
        hi = hi | (((u_ >> (sh_ + 1)) & (unsigned char) 1) << S1);                 \
    }
#else
#define QT_C1(pb, c, lo, hi, p)
#endif
#if P8
// the two 2-step groups of chunk c (bytes as signed codes)
#define QT_C8(pb, c, lo, hi, p)                                                    \
    {                                                                              \
        lo = ld_u8(W, p + 512 * (c));                                              \
        hi = ld_u8(W, p + 512 * (c) + 256);                                        \
    }
#else
#define QT_C8(pb, c, lo, hi, p)
#endif
#define QT_CODES(pb, c, FL, FH)                                                    \
    bfloat256 FL, FH;                                                              \
    {                                                                              \
        int p_ = (pb) + PBYTES;                                                    \
        uchar256 lo_ = 0, hi_ = 0;                                                 \
        QT_C4(pb, c, lo_, hi_, p_)                                                 \
        QT_C2(pb, c, lo_, hi_, p_)                                                 \
        QT_C1(pb, c, lo_, hi_, p_)                                                 \
        QT_C8(pb, c, lo_, hi_, p_)                                                 \
        FL = CVT(lo_);                                                             \
        FH = CVT(hi_);                                                             \
    }
#endif // !QT_FP8

#if defined(QT_LIB)

// type definitions and helpers only (included by syn_qdq.c)

#elif defined(I8)

// int8 GEMV: x from syn_xq8 (X8: alternating int8 x of each k-pair, or XQ: 8 steps per 256 B; XG per 32 steps: dx,
// dx * sum xq, and the two 16-step halves' sums), int8 codes.
// y = sum over groups of S * dx * (integer group sum) + MN * xsum.
// tensors: 0 W  1 X8  2 XG  3 ids | 4 out
#if P2 || P1
#define RB8 2
#else
#define RB8 4
#endif
#define GF (G < 32 ? G : 32)   // finalize period: the weight group or the 32-step x group
#if P1 || defined(QT_Q4_0) || defined(QT_Q4_1)
#define CPI 4                  // chunks per inner loop iteration
#else
#define CPI 2
#endif
#define P4SZ (P4 * (SB / 4) * 256)
#define P2SZ (P2 * (SB / 8) * 256)
#if defined(QT_Q4_K) || defined(QT_Q5_K)
#define SBP 1   // scales decoded once per superblock
#else
#define SBP 0
#endif
#if defined(QT_MXFP4) || defined(QT_NVFP4) || defined(QT_IQ4_NL) || defined(QT_IQ4_XS) || defined(QT_Q8_0)
#define HAS_MN 0
#else
#define HAS_MN 1
#endif

// 16-bit accumulators where |code| * 127 * GF / 2 < 2^15, else 32-bit
#if P8 || (P4 && (P2 || P1))
typedef int256 acc_t;
typedef int128 gs_t;
#define ACC_ZERO(a) { (a).v1 = 0; (a).v2 = 0; (a).v3 = 0; (a).v4 = 0; }
#define MAC2(a, b, c, d, acc) v_i8_mac_x2_b(a, b, c, d, acc)
#define GSUM(s, a) { (s).v1 = (a).v1 + (a).v2; (s).v2 = (a).v3 + (a).v4; }
#define GS_ZERO(s) { (s).v1 = 0; (s).v2 = 0; }
#define GCVT(s) v_convert_i32_to_f32_x2_b(s, SW_RHNE)
#else
typedef short256 acc_t;
typedef short128 gs_t;
#define ACC_ZERO(a) { (a).v1 = 0; (a).v2 = 0; }
#define MAC2(a, b, c, d, acc) v_i8_mac_x2_acc16_b(a, b, c, d, acc)
#define GSUM(s, a) { (s) = (a).v1 + (a).v2; }
#define GS_ZERO(s) { (s) = 0; }
#define GCVT(s) convert_short128_to_float128(s, 0)
#endif

// XG of the column at xgc: per 32 steps 4 uniform vectors (256 floats): dx, xsum, xsum of the 16-step halves
#define XG_DX(k) (xgc + ((k) >> 5) * 256)
#define XG_XS(k) (xgc + ((k) >> 5) * 256 + (GF == 32 ? 64 : 128 + 64 * (((k) >> 4) & 1)))

#if SBP
__local__ float64 PG[2][RB8][NG][2];   // per superblock parity, row block and group: S * dx (rows 0..63, 64..127)
__local__ float64 XGL[NG][2];          // the superblock's dx, xsum per group
__local__ float64 MY[RB8][2];          // the MN terms
#else
__local__ float64 PG[2][RB8][2];       // per group parity and row block: S * dx
#if G > GF
__local__ float64 PW[RB8][4];          // the weight group's S, MN (it spans several x groups)
#endif
#endif
__local__ float64 YA[RB8][2];          // the f32 sums

static inline int5 cw8(int p)
{
    int5 c = {p, WY[0], 0, 0, 0};
    return c;
}

#if SBP
// Q4_K / Q5_K: the scales of every group of the superblock at pb, once per superblock.
// S * dx per group into PG, the MN terms (MN * xsum, zero outside the item's K range) into MY.
#define LD_BP2_8(p) v_u8_ld_tnsr_b(cw8(pb + NF16 * 256 + 128 * (p)), W, SW_UNPACK | SW_UNPCK_8_TO_32)
#define U2F_8(x) v_convert_u8_to_f32_all_b(x, 0, (float256){0}, 1, 0)
static inline void i8_params(tensor W, int pb, int b, int par)
{
    const float128 d = convert_half128_to_float128(v_f16_ld_tnsr_b(cw8(pb), W, SW_UNPACK | SW_UNPCK_16_TO_32), 0);
    const ushort128 dmb = v_u16_ld_tnsr_b(cw8(pb + 256), W, SW_UNPACK | SW_UNPCK_16_TO_32) ^ (unsigned short) 0x8000;
    const float128 dm = convert_half128_to_float128(as_half128(dmb), 0);   // -dmin
    uchar256 P[6];
    #pragma unroll
    for (int p = 0; p < 6; p++) {
        P[p] = LD_BP2_8(2 * p);
    }
    const uchar512 n89 = v_convert_u4_to_u8_all_b(P[4], 0, (uchar512){0}, 1, 0);
    const uchar512 nab = v_convert_u4_to_u8_all_b(P[5], 0, (uchar512){0}, 1, 0);
    uchar256 v6[8];
    v6[0] = P[0] & (unsigned char) 63;
    v6[1] = P[1] & (unsigned char) 63;
    v6[2] = n89.v1 | ((P[0] >> 2) & (unsigned char) 0x30);
    v6[3] = nab.v1 | ((P[1] >> 2) & (unsigned char) 0x30);
    v6[4] = P[2] & (unsigned char) 63;
    v6[5] = P[3] & (unsigned char) 63;
    v6[6] = n89.v2 | ((P[2] >> 2) & (unsigned char) 0x30);
    v6[7] = nab.v2 | ((P[3] >> 2) & (unsigned char) 0x30);
    float128 t;
    t.v1 = 0.0f; t.v2 = 0.0f;
    #pragma unroll
    for (int q = 0; q < 4; q++) {
        const float256 sc = U2F_8(v6[q]), mn = U2F_8(v6[4 + q]);
        #pragma unroll
        for (int h = 0; h < 2; h++) {
            const int g = 2 * q + h;
            float128 s_, m_;
            s_.v1 = h ? sc.v3 : sc.v1; s_.v2 = h ? sc.v4 : sc.v2;
            m_.v1 = h ? mn.v3 : mn.v1; m_.v2 = h ? mn.v4 : mn.v2;
            s_ = v_f32_mul_x2_b(v_f32_mul_x2_b(s_, d.v1, d.v2), XGL[g][0], XGL[g][0]);
            PG[par][b][g][0] = s_.v1; PG[par][b][g][1] = s_.v2;
            t = v_f32_mac_x2_b(m_, XGL[g][1], XGL[g][1], t);
        }
    }
    float128 y;
    y.v1 = MY[b][0]; y.v2 = MY[b][1];
    y = v_f32_mac_x2_b(t, dm.v1, dm.v2, y);
    MY[b][0] = y.v1; MY[b][1] = y.v2;
}
#else
// per group: S, MN of group g of the superblock at pb ({S rows 0..63, 64..127, MN rows 0..63, 64..127})
static inline float256 i8_gp(tensor W, int pb, int g)
{
#if defined(QT_MXFP4)
    // 2^(e - 127) from the exponent bits (e = 0: 0); the caller halves dx
    const int64 e = as_int64(v_u8_ld_tnsr_b(cw8(pb), W, SW_UNPACK | SW_UNPCK_8_TO_32));   // e[i] | e[64 + i] << 8 | ..
    float256 r;
    r.v1 = as_float64(v_i32_shl_b(e, (int64) 23) & (int64) 0x7F800000);
    r.v2 = as_float64(v_i32_shl_b(e, (int64) 15) & (int64) 0x7F800000);
    return r;
#else
    return qt_gp(W, pb, g);
#endif
}
#endif

static inline char256 undef_c256() { char256 u_; return u_; }

#if defined(LUT_IQ4) || defined(LUT_FP4)
// 16-entry code table, repeated every 16 bytes
static inline char256 lut_tab()
{
#if defined(LUT_IQ4)
    const int T[16] = {-127, -104, -83, -65, -49, -35, -22, -10, 1, 13, 25, 38, 53, 69, 89, 113};
#else
    const int T[16] = {0, 1, 2, 3, 4, 6, 8, 12, 0, -1, -2, -3, -4, -6, -8, -12};
#endif
    const uchar256 l = read_lane_id_1b_b() & (unsigned char) 15;
    char256 v = (char) T[0];
    #pragma unroll
    for (int i = 1; i < 16; i++) {
        v = v_i8_sel_eq_u8_b(l, (unsigned char) i, (char) T[i], v);
    }
    return v;
}
#define I8C(u) v_i8_shuffle_b(TAB, (u) | (unsigned char) 0x80, 0, undef_c256())
#else
#define I8C(u) ((char256) (u))
#endif

// x: X8 (each k-pair as 256 B), or the compact XQ (8 steps per 256 B, every 8 bytes the same).
#if defined(QT_Q4_0) || defined(QT_Q4_1) || defined(QT_Q8_0)
#define XQ 1
#define MAC_(lo, hi, acc) MAC2(lo, xlo, hi, xhi, acc)
#else
#define XQ 0
#define MAC_(lo, hi, acc) MAC2(lo, xlo, xhi, hi, acc)
#endif
// one k-chunk (4 steps) of row block b: codes into the MAC with the chunk's x pair
#if P8
#define CHUNK(b, cc)                                                                                                   \
    {                                                                                                                  \
        const char256 lo_ = v_i8_ld_tnsr_b(c4[b], W);                                                                  \
        c4[b][0] += 256;                                                                                               \
        const char256 hi_ = v_i8_ld_tnsr_b(c4[b], W);                                                                  \
        c4[b][0] += 256;                                                                                               \
        acc[b] = MAC_(lo_, hi_, acc[b]);                                                                               \
    }
#else
#define CHUNK(b, cc)                                                                                                   \
    {                                                                                                                  \
        CHUNK_P4(b)                                                                                                    \
        CHUNK_P2(b, cc)                                                                                                \
        CHUNK_P1(b, cc)                                                                                                \
    }
#endif
#if P4
// the 4-bit plane: (u & 15, u >> 4)
#define CHUNK_P4(b)                                                                                                    \
    {                                                                                                                  \
        const uchar512 n_ = v_convert_u4_to_u8_all_b(v_u8_ld_tnsr_b(c4[b], W), 0, (uchar512){0}, 1, 0);                \
        c4[b][0] += 256;                                                                                               \
        acc[b] = MAC_(I8C(n_.v1), I8C(n_.v2), acc[b]);                                                                 \
    }
#else
#define CHUNK_P4(b)
#endif
// the 2- and 1-bit planes, at bits S2 / S1
#if P2
#define CHUNK_P2(b, cc)                                                                                                \
    {                                                                                                                  \
        if (((cc) & 1) == 0) {                                                                                         \
            u2[b] = v_u8_ld_tnsr_b(c2[b], W);                                                                          \
            c2[b][0] += 256;                                                                                           \
        }                                                                                                              \
        const int sh_ = ((cc) & 1) * 4;                                                                                \
        acc[b] = MAC_((char256) BITS2(u2[b], sh_), (char256) BITS2(u2[b], sh_ + 2), acc[b]);                           \
    }
// bits sh, sh + 1 of u at bits S2, S2 + 1
#define BITS2(u, sh) ((sh) == S2 ? ((u) & (unsigned char) (3 << S2)) :                                                 \
                      (sh) < S2 ? (((u) << (S2 - (sh))) & (unsigned char) (3 << S2)) :                                 \
                                  (((u) >> ((sh) - S2)) & (unsigned char) (3 << S2)))
#else
#define CHUNK_P2(b, cc)
#endif
#if P1
#define CHUNK_P1(b, cc)                                                                                                \
    {                                                                                                                  \
        if (((cc) & 3) == 0) {                                                                                         \
            u1[b] = v_u8_ld_tnsr_b(c1[b], W);                                                                          \
            c1[b][0] += 256;                                                                                           \
        }                                                                                                              \
        const int sh_ = ((cc) & 3) * 2;                                                                                \
        acc[b] = MAC_((char256) BITS1(u1[b], sh_), (char256) BITS1(u1[b], sh_ + 1), acc[b]);                           \
    }
#define BITS1(u, sh) ((sh) == S1 ? ((u) & (unsigned char) (1 << S1)) :                                                 \
                      (sh) < S1 ? (((u) << (S1 - (sh))) & (unsigned char) (1 << S1)) :                                 \
                                  (((u) >> ((sh) - S1)) & (unsigned char) (1 << S1)))
#else
#define CHUNK_P1(b, cc)
#endif

// items: (K split, x group, RB row blocks). An x group is a column, or (with one x row for all slots) a
// token: then its slots' row blocks are one list (an item may span two experts).
void main(tensor W, tensor X8, tensor XG, tensor ids, tensor dep, tensor out,
          int nmem, int K, int M, int nrb, int ncol, int nu, int bcast, int has_ids,
          int nx1, int offi, int si0, int si1, int nks, float inv_nbx, float inv_nxg, float inv_nrb, float inv_nu)
{
    const int5 ioff = get_index_space_offset();
    const int5 iend = ioff + get_index_space_size();
    if (has_ids) {
        scalar_cache_reset();
    }
#if defined(LUT_IQ4) || defined(LUT_FP4)
    const char256 TAB = lut_tab();
#endif
#if XQ
    // byte l of the expansion of k-pair q of an XQ vector: x[2 q + (l & 1)]
    uchar256 XS[4];
    #pragma unroll
    for (int q = 0; q < 4; q++) {
        XS[q] = (read_lane_id_1b_b() & (unsigned char) 1) | (unsigned char) (0x80 | 2 * q);
    }
#endif
    const int nsbk = K / SB;                // superblocks per row
    const int nkc = K / KU;                 // split units
    const int kper = (nkc + nks - 1) / nks;
    const int flat = has_ids && bcast;
    const int R = flat ? nu * nrb : nrb;    // row blocks per x group
    const int nbx = (R + RB8 - 1) / RB8;
    const int nxg = flat ? ncol / nu : ncol;
    const int nitems = nks * nxg * nbx;
    const int bstride = nsbk * SBB;
    for (int m = ioff[0]; m < iend[0]; m++) {
        int i0, i1;
        SPLIT_RANGE(nitems, nmem, m, i0, i1)
        for (int it = i0; it < i1; it++) {
            const int r1 = FDIV(it, inv_nbx), bg = it - r1 * nbx;
            const int ks = FDIV(r1, inv_nxg), xg = r1 - ks * nxg;
            int kc0 = ks * kper, kc1 = kc0 + kper;
            if (kc1 > nkc) kc1 = nkc;
            const int ka = kc0 * KU, kb = kc1 * KU;
            // the x group's token / slot, x column
            const int t = flat ? xg : (has_ids ? FDIV(xg, inv_nu) : 0);
            const int s0 = flat ? 0 : xg - t * nu;
            const int xcol = !has_ids ? xg : ((flat || bcast ? 0 : s0) + t * nx1);
            // row blocks: matrix, superblock offset, output offset, rows left (past the end: the last row block again,
            // not stored)
            int eb[RB8], pbb[RB8], pob[RB8], remb[RB8];
            #pragma unroll
            for (int b = 0; b < RB8; b++) {
                const int f = bg * RB8 + b < R ? bg * RB8 + b : R - 1;
                const int s = flat ? FDIV(f, inv_nrb) : s0;
                const int rb = f - (flat ? s * nrb : 0);
                int5 ci = {offi + s * si0 + t * si1, 0, 0, 0, 0};
                eb[b] = s_i32_ld_g((__global int *) gen_addr(ci, ids), 0, 0, has_ids);
                pbb[b] = (rb * nsbk + ka / SB) * SBB;
                pob[b] = (ks * ncol + (flat ? t * nu + s : xg)) * M + rb * 128;
                remb[b] = bg * RB8 + b < R ? M - rb * 128 : 0;
            }
            const int xgc = xcol * (K >> 5) * 256;
            acc_t acc[RB8];
            gs_t gsp[RB8];
            #pragma unroll
            for (int b = 0; b < RB8; b++) {
                ACC_ZERO(acc[b])
                GS_ZERO(gsp[b])
                YA[b][0] = 0.0f; YA[b][1] = 0.0f;
#if SBP
                MY[b][0] = 0.0f; MY[b][1] = 0.0f;
#endif
            }
            // offsets kept incrementally (position in the superblock, x position)
            int ksb = ka % SB;
            int5 cx = {(XQ ? xcol * (K >> 3) + (ka >> 3) : xcol * (K >> 1) + (ka >> 1)) * 256, 0, 0, 0, 0};
            int par = 0, gpp = ksb / GF, parp = 0;   // the previous group's params: PG[parp][b][gpp]
            // code plane coordinates per row block, kept incrementally (at a superblock's end: + SBB - plane size)
            int5 c4[RB8];
#if P2
            int5 c2[RB8];
#endif
#if P1
            int5 c1[RB8];
#endif
            #pragma unroll
            for (int b = 0; b < RB8; b++) {
                const int p = pbb[b] + PBYTES;
                c4[b][0] = p + (P8 ? 512 : 256) * (ksb / 4); c4[b][1] = eb[b]; c4[b][2] = 0; c4[b][3] = 0; c4[b][4] = 0;
#if P2
                c2[b] = c4[b]; c2[b][0] = p + P4SZ + 256 * (ksb / 8);
#endif
#if P1
                c1[b] = c4[b]; c1[b][0] = p + P4SZ + P2SZ + 256 * (ksb / 16);
#endif
            }
            for (int k0 = ka; k0 < kb; k0 += GF) {
#if SBP
                if (ksb == 0 || k0 == ka) {
                    // a new superblock (or the item's first, entered mid-way): all its groups' params
                    par ^= 1;
                    // the superblock's groups' dx, xsum (0 outside the item's K range: no MN term)
                    const int ks0 = k0 - ksb;
                    #pragma unroll
                    for (int g = 0; g < NG; g++) {
                        int5 cg = {XG_DX(ks0 + G * g), 0, 0, 0, 0};
                        XGL[g][0] = v_f32_ld_tnsr_b(cg, XG);
                        cg[0] = XG_XS(ks0 + G * g);
                        XGL[g][1] = v_f32_ld_tnsr_b(cg, XG, 0, 0.0f, ks0 + G * g >= k0 && ks0 + G * g < kb);
                    }
                    #pragma unroll 1
                    for (int b = 0; b < RB8; b++) {
                        WY[0] = eb[b];
                        i8_params(W, pbb[b], b, par);
                    }
                    if (k0 == ka) {
                        parp = par;
                    }
                }
                const int gi = ksb / G;
#else
                // per-group params (one or two loads): S * dx to PG[par], MN * xsum to y
                par ^= 1;
                const int gi = 0;
                int5 cg = {XG_DX(k0), 0, 0, 0, 0};
                float64 dx = v_f32_ld_tnsr_b(cg, XG);
#if defined(QT_MXFP4)
                dx = dx * 0.5f;
#endif
                cg[0] = XG_XS(k0);
                const float64 xs = v_f32_ld_tnsr_b(cg, XG);
                #pragma unroll
                for (int b = 0; b < RB8; b++) {
                    WY[0] = eb[b];
#if G > GF
                    if ((ksb % G) == 0) {
                        const float256 r = i8_gp(W, pbb[b], ksb / G);
                        PW[b][0] = r.v1; PW[b][1] = r.v2; PW[b][2] = r.v3; PW[b][3] = r.v4;
                    }
                    float256 r;
                    r.v1 = PW[b][0]; r.v2 = PW[b][1]; r.v3 = PW[b][2]; r.v4 = PW[b][3];
#else
                    const float256 r = i8_gp(W, pbb[b], ksb / G);
#endif
                    float128 sp, mn;
                    sp.v1 = r.v1; sp.v2 = r.v2; mn.v1 = r.v3; mn.v2 = r.v4;
                    const float128 sd = v_f32_mul_x2_b(sp, dx, dx);
                    PG[par][b][0] = sd.v1; PG[par][b][1] = sd.v2;
#if HAS_MN
                    float128 y;
                    y.v1 = YA[b][0]; y.v2 = YA[b][1];
                    y = v_f32_mac_x2_b(mn, xs, xs, y);
                    YA[b][0] = y.v1; YA[b][1] = y.v2;
#endif
                }
                if (k0 == ka) {
                    parp = par;
                }
#endif
#if P2
                uchar256 u2[RB8];
#endif
#if P1
                uchar256 u1[RB8];
#endif
                #pragma unroll 1
                for (int c0 = 0; c0 < GF / 4; c0 += CPI) {
                    char256 xv;
                    #pragma unroll
                    for (int cc = 0; cc < CPI; cc++) {
#if XQ
                        if ((cc & 1) == 0) {
                            xv = v_i8_ld_tnsr_b(cx, X8);
                            cx[0] += 256;
                        }
                        const char256 xlo = v_i8_shuffle_b(xv, XS[2 * (cc & 1)], 0, undef_c256());
                        const char256 xhi = v_i8_shuffle_b(xv, XS[2 * (cc & 1) + 1], 0, undef_c256());
#else
                        const char256 xlo = v_i8_ld_tnsr_b(cx, X8);
                        cx[0] += 256;
                        const char256 xhi = v_i8_ld_tnsr_b(cx, X8);
                        cx[0] += 256;
#endif
                        #pragma unroll
                        for (int b = 0; b < RB8; b++) {
                            CHUNK(b, cc)
                        }
                    }
                }
                // the previous group's sums (zero before the first group)
                #pragma unroll
                for (int b = 0; b < RB8; b++) {
                    const float128 r = GCVT(gsp[b]);
                    float128 sp, y;
                    y.v1 = YA[b][0]; y.v2 = YA[b][1];
#if SBP
                    sp.v1 = PG[parp][b][gpp][0]; sp.v2 = PG[parp][b][gpp][1];
#else
                    sp.v1 = PG[parp][b][0]; sp.v2 = PG[parp][b][1];
#endif
                    y = v_f32_mac_x2_b(sp, r.v1, r.v2, y);
                    YA[b][0] = y.v1; YA[b][1] = y.v2;
                    GSUM(gsp[b], acc[b])
                    ACC_ZERO(acc[b])
                }
                gpp = gi;
                parp = par;
                // the next superblock
                ksb += GF;
                const int wrap = ksb == SB;
                ksb = wrap ? 0 : ksb;
                #pragma unroll
                for (int b = 0; b < RB8; b++) {
                    pbb[b] += wrap ? SBB : 0;
                    c4[b][0] += wrap ? SBB - (P8 ? P8 * (SB / 2) * 256 : P4SZ) : 0;
#if P2
                    c2[b][0] += wrap ? SBB - P2SZ : 0;
#endif
#if P1
                    c1[b][0] += wrap ? SBB - P1 * (SB / 16) * 256 : 0;
#endif
                }
            }
            #pragma unroll
            for (int b = 0; b < RB8; b++) {
                const float128 r = GCVT(gsp[b]);
                float128 sp, y;
                y.v1 = YA[b][0]; y.v2 = YA[b][1];
#if SBP
                sp.v1 = PG[parp][b][gpp][0]; sp.v2 = PG[parp][b][gpp][1];
                y = v_f32_mac_x2_b(sp, r.v1, r.v2, y);
                y.v1 += MY[b][0];
                y.v2 += MY[b][1];
#else
                sp.v1 = PG[parp][b][0]; sp.v2 = PG[parp][b][1];
                y = v_f32_mac_x2_b(sp, r.v1, r.v2, y);
#endif
                // rows 0..63, 64..127 (partial stores, none past M)
                const int rem = remb[b];
                int5 co = {pob[b], 0, 0, 0, 0};
                v_f32_st_tnsr_partial(co, out, y.v1, (rem < 64 ? rem : 64) - 1, 0, 0, rem > 0);
                co[0] += 64;
                v_f32_st_tnsr_partial(co, out, y.v2, (rem < 128 ? rem : 128) - 65, 0, 0, rem > 64);
            }
        }
    }
}

#elif defined(DEQ)

// out[r, k, mat] at mat * smat + k * sk + r (sk = M, smat = M K: [M, K, nmat]; sk = M nmat, smat = M:
// [M, nmat, K], all matrices as one M nmat x K operand)
void main(tensor W, tensor dep, tensor out, int nmem, int K, int M, int nrb, int nmat, int sk, int smat)
{
    const int5 ioff = get_index_space_offset();
    const int5 iend = ioff + get_index_space_size();
    const bfloat128 one = (bf16) 1.0f;
#if defined(QT_FP8)
    // item = (row block, 64 k-steps)
    const int nkb = (K + 63) >> 6;
    const int nitems = nmat * nrb * nkb;
    for (int m = ioff[0]; m < iend[0]; m++) {
        int i0, i1;
        SPLIT_RANGE(nitems, nmem, m, i0, i1)
        for (int it = i0; it < i1; it++) {
            const int kb = it % nkb, blk = it / nkb;
            const int rb = blk % nrb, mat = blk / nrb;
            const int base = rb * (512 + K * 128);
            int5 c0 = {base, mat, 0, 0, 0};
            const float64 s0 = v_f32_ld_tnsr_b(c0, W);
            c0[0] += 256;
            const float64 s1 = v_f32_ld_tnsr_b(c0, W);
            const int rem = M - rb * 128;
            const int kend = kb * 64 + 64 < K ? kb * 64 + 64 : K;
            for (int k = kb * 64; k < kend; k += 2) {
                int5 cw = {base + 512 + 128 * k, mat, 0, 0, 0};
                const bfloat256 w = convert_minifloat256_to_bfloat256(v_f8_ld_tnsr_b(cw, W), 0);
                #pragma unroll
                for (int j = 0; j < 2; j++) {
                    float128 q;
                    q.v1 = 0.0f;
                    q.v2 = 0.0f;
                    q = v_bf16_mac_acc32_b(j == 0 ? w.v1 : w.v2, one, q);
                    const bfloat128 y = cvt_bf16(q.v1 * s0, q.v2 * s1);
                    int5 co = {mat * smat + (k + j) * sk + rb * 128, 0, 0, 0, 0};
                    if (rem >= 128) v_bf16_st_tnsr(co, out, y); else v_bf16_st_tnsr_partial(co, out, y, rem - 1, 0);
                }
            }
        }
    }
#else
    const int nsb = K / SB;
    const int nitems = nmat * nrb * nsb;
    for (int m = ioff[0]; m < iend[0]; m++) {
        int i0, i1;
        SPLIT_RANGE(nitems, nmem, m, i0, i1)
        for (int it = i0; it < i1; it++) {
            const int sb = it % nsb, blk = it / nsb;
            const int rb = blk % nrb, mat = blk / nrb;
            const int pb = (rb * nsb + sb) * SBB;
            WY[0] = mat;
            qt_params(W, pb);
            const int rem = M - rb * 128;
            for (int c = 0; c < SB / 4; c++) {
                QT_CODES(pb, c, fl, fh)
                const int g = (4 * c) / G;
                const float64 sl = S[2 * g], sh = S[2 * g + 1], ml = MN[2 * g], mh = MN[2 * g + 1];
                const int kg = sb * SB + 4 * c;
                const int po = mat * smat + kg * sk + rb * 128;
                #pragma unroll
                for (int j = 0; j < 4; j++) {
                    const bfloat128 v = j == 0 ? fl.v1 : j == 1 ? fl.v2 : j == 2 ? fh.v1 : fh.v2;
                    float128 q;
                    q.v1 = 0.0f;
                    q.v2 = 0.0f;
                    q = v_bf16_mac_acc32_b(v, one, q);    // rows 0..63 / 64..127
                    const bfloat128 y = cvt_bf16(v_f32_mac_b(q.v1, sl, ml), v_f32_mac_b(q.v2, sh, mh));
                    int5 co = {po + j * sk, 0, 0, 0, 0};
                    if (rem >= 128) v_bf16_st_tnsr(co, out, y); else v_bf16_st_tnsr_partial(co, out, y, rem - 1, 0);
                }
            }
        }
    }
#endif
}

#else // GEMV

// uniform bf16 vector of x lane l (0..15) of the broadcast dual group bd (uniform: native order)
static inline bfloat128 xb_(float64 u)
{
    float128 t;
    t.v1 = u;
    t.v2 = u;
    return convert_float128_to_bfloat128(t, SW_RHNE);
}
#define XB(bd, l) xb_(SHUF(bd, IDX[l]))

// row blocks per item: each x broadcast feeds RB blocks (2 for multi-plane codes)
#if !defined(QT_FP8) && (P2 || P1)
#define RB 2
#else
#define RB 4
#endif
__local__ float64 PV[4 * RB];

void main(tensor W, tensor x, tensor ids, tensor dep, tensor out,
          int nmem, int K, int M, int nrb, int ncol, int nu, int bcast, int has_ids,
          int offx, int sx1, int sx2, int offi, int si0, int si1, int nks)
{
    const int5 ioff = get_index_space_offset();
    const int5 iend = ioff + get_index_space_size();
    DECL_IDX(IDX)
    if (has_ids) {
        scalar_cache_reset();
    }
#if defined(QT_FP8)
    const int nkc = K >> 6;                 // 64-step blocks
#else
    const int nsbk = K / SB;                // superblocks per row
    const int nkc = K / KU;                 // split units
#endif
    const int kper = (nkc + nks - 1) / nks;
    const int nbg = (nrb + RB - 1) / RB;
    // items: K split outermost, then column, then row-block group
    const int nitems = nks * ncol * nbg;
    for (int m = ioff[0]; m < iend[0]; m++) {
        int i0, i1;
        SPLIT_RANGE(nitems, nmem, m, i0, i1)
        for (int it = i0; it < i1; it++) {
            const int bg = it % nbg, rest = it / nbg;
            const int col = rest % ncol, ks = rest / ncol;
            const int rb0 = bg * RB;
            int kc0 = ks * kper, kc1 = kc0 + kper;
            if (kc1 > nkc) kc1 = nkc;
            int e = 0, px = offx + col * sx1;
            WY[0] = 0;
            if (has_ids) {
                const int s = col % nu, t = col / nu;
                int5 ci = {offi + s * si0 + t * si1, 0, 0, 0, 0};
                e = s_i32_ld_g((__global int *) gen_addr(ci, ids));
                WY[0] = e;
                px = offx + (bcast ? 0 : s * sx1) + t * sx2;
            }
            float128 acc[RB], y[RB];
            #pragma unroll
            for (int b = 0; b < RB; b++) {
                acc[b].v1 = 0.0f; acc[b].v2 = 0.0f;
                y[b] = acc[b];
            }
#if defined(QT_FP8)
            // fp8 blocks: [128 f32 scales][K/2 x 256 B]; 64 steps per x vector
            for (int kb = kc0 * 64; kb < kc1 * 64; kb += 64) {
                const float64 xv = ld_f32(x, px + kb);
                for (int dg = 0; dg < 4; dg++) {
                    float64 bd;
                    if (dg == 0) bd = BCAST_DG(xv, 0);
                    else if (dg == 1) bd = BCAST_DG(xv, 1);
                    else if (dg == 2) bd = BCAST_DG(xv, 2);
                    else bd = BCAST_DG(xv, 3);
                    const int kk = kb + 16 * dg;
                    #pragma unroll
                    for (int l = 0; l < 16; l += 2) {
                        const bfloat128 x0 = XB(bd, l), x1 = XB(bd, l + 1);
                        #pragma unroll
                        for (int b = 0; b < RB; b++) {
                            int5 cw = {((rb0 + b) * (512 + K * 128)) + 512 + 128 * (kk + l), e, 0, 0, 0};
                            const bfloat256 w = convert_minifloat256_to_bfloat256(v_f8_ld_tnsr_b(cw, W), 0);
                            acc[b] = v_bf16_mac_acc32_b(w.v1, x0, acc[b]);
                            acc[b] = v_bf16_mac_acc32_b(w.v2, x1, acc[b]);
                        }
                    }
                }
            }
            #pragma unroll
            for (int b = 0; b < RB; b++) {
                // per-row scales: the block's first 512 bytes as f32
                int5 c0 = {(rb0 + b) * (512 + K * 128), e, 0, 0, 0};
                y[b].v1 = acc[b].v1 * v_f32_ld_tnsr_b(c0, W);
                c0[0] += 256;
                y[b].v2 = acc[b].v2 * v_f32_ld_tnsr_b(c0, W);
            }
#else
            float64 xv = 0.0f, xs4 = 0.0f, xsum = 0.0f;
            for (int k16 = kc0 * KU; k16 < kc1 * KU; k16 += 16) {
                if ((k16 & 63) == 0 || k16 == kc0 * KU) {
                    xv = ld_f32(x, px + (k16 & ~63));
                    float64 r0 = SHUF(xv, IDX[0]), r1 = SHUF(xv, IDX[1]), r2 = SHUF(xv, IDX[2]), r3 = SHUF(xv, IDX[3]);
                    #pragma unroll
                    for (int q = 4; q < 16; q += 4) {
                        r0 += SHUF(xv, IDX[q]);
                        r1 += SHUF(xv, IDX[q + 1]);
                        r2 += SHUF(xv, IDX[q + 2]);
                        r3 += SHUF(xv, IDX[q + 3]);
                    }
                    xs4 = (r0 + r1) + (r2 + r3);   // each lane: the sum of its dual group
                }
                const int dg = (k16 >> 4) & 3;
                float64 bd, xs;
                if (dg == 0) { bd = BCAST_DG(xv, 0); xs = BCAST_DG(xs4, 0); }
                else if (dg == 1) { bd = BCAST_DG(xv, 1); xs = BCAST_DG(xs4, 1); }
                else if (dg == 2) { bd = BCAST_DG(xv, 2); xs = BCAST_DG(xs4, 2); }
                else { bd = BCAST_DG(xv, 3); xs = BCAST_DG(xs4, 3); }
                const int ksb = k16 % SB, sb = k16 / SB;
                const int pb0 = (rb0 * nsbk + sb) * SBB;
                if ((ksb % G) == 0) {
                    // the group's scale / offset per block
                    #pragma unroll
                    for (int b = 0; b < RB; b++) {
                        const float256 r = qt_gp(W, pb0 + b * nsbk * SBB, ksb / G);
                        PV[4 * b] = r.v1; PV[4 * b + 1] = r.v2; PV[4 * b + 2] = r.v3; PV[4 * b + 3] = r.v4;
                    }
                }
                #pragma unroll
                for (int cc = 0; cc < 4; cc++) {
                    const bfloat128 x0 = XB(bd, 4 * cc), x1 = XB(bd, 4 * cc + 1);
                    const bfloat128 x2 = XB(bd, 4 * cc + 2), x3 = XB(bd, 4 * cc + 3);
                    #pragma unroll
                    for (int b = 0; b < RB; b++) {
                        QT_CODES(pb0 + b * nsbk * SBB, ksb / 4 + cc, fl, fh)
                        acc[b] = v_bf16_mac_acc32_b(fl.v1, x0, acc[b]);
                        acc[b] = v_bf16_mac_acc32_b(fl.v2, x1, acc[b]);
                        acc[b] = v_bf16_mac_acc32_b(fh.v1, x2, acc[b]);
                        acc[b] = v_bf16_mac_acc32_b(fh.v2, x3, acc[b]);
                    }
                }
                xsum += xs;
                if (((ksb + 16) % G) == 0) {
                    #pragma unroll
                    for (int b = 0; b < RB; b++) {
                        y[b].v1 = v_f32_mac_b(PV[4 * b], acc[b].v1, v_f32_mac_b(PV[4 * b + 2], xsum, y[b].v1));
                        y[b].v2 = v_f32_mac_b(PV[4 * b + 1], acc[b].v2, v_f32_mac_b(PV[4 * b + 3], xsum, y[b].v2));
                        acc[b].v1 = 0.0f;
                        acc[b].v2 = 0.0f;
                    }
                    xsum = 0.0f;
                }
            }
#endif
            #pragma unroll
            for (int b = 0; b < RB; b++) {
                const int po = (ks * ncol + col) * M + (rb0 + b) * 128;
                const int rem = M - (rb0 + b) * 128;
                if (rem > 0) {
                    ST_F32(out, po, y[b].v1, rem);
                }
                if (rem > 64) {
                    ST_F32(out, po + 64, y[b].v2, rem - 64);
                }
            }
        }
    }
}

#endif
