// SPDX-License-Identifier: AGPL-3.0-only
// Copyright Aaron Botsis
// Planar quantized weights and fp8 blocks -> bf16, one type per build (-DQT_<type>):
//  default : syn_qmm.c -DDEQ interface, out[r, k, mat] at mat * smat + k * sk + r; item = (row block, superblock)
//  -DPAGED : paged MoE interface, out[r, k, p] = W_{pe[p]}[r, k] as [M, K, P]; index space [row blocks, superblocks,
//            pages]; pe[p] < 0: unused page, not written
// A group is 8 code chunks (32 k-steps). Per chunk the code planes are merged as u8 and converted to f32 (one k-step of
// rows 0..63 / 64..127 per vector), y = MN + S * q per k-step, stored as bf16 in natural row order.
// FP4 codes are rebuilt as fp8_143 bytes (sign, e, m at bits 7, 4..3, 2), exactly 2^-6 x the E2M1 value (S x 128).
// -DQT_Q8_0: q8_0 (8-bit signed code plane) in its own simple loop (see below).
// tensors: 0 W [bytes per matrix, matrices] i8  (PAGED: 1 pe [P] i32)  dep | out bf16
#define QT_LIB
#include "syn_qmm.c"

#define PK (SW_PACK | SW_PCK_32_TO_16)

#if defined(PAGED)
#define KADV(c, n) c[1] += (n)
#else
#define KADV(c, n) c[0] += (n) * sk
#endif

#if defined(QT_FP8)

// fp8_143 128-row blocks [128 x f32 scale][K/2 x 256 B]: a 256 B vector holds k-steps 2 m, 2 m + 1: (row i, 2 m), (i, 2 m + 1),
// (64 + i, 2 m), (64 + i, 2 m + 1); item = (row block, 64 k-steps), 16 k-steps per loop body
#define FP8_BODY(cw, co, sz)                                                                                        \
    {                                                                                                               \
        float256 f[8];                                                                                              \
        _Pragma("unroll")                                                                                           \
        for (int u = 0; u < 8; u++) {                                                                               \
            f[u] = v_convert_f8_to_f32_all_b(v_f8_ld_tnsr_b(cw, W), 0, (float256){0}, 1, 0);                        \
            cw[0] += 256;                                                                                           \
        }                                                                                                           \
        _Pragma("unroll")                                                                                           \
        for (int u = 0; u < 8; u++) {                                                                               \
            const float128 y0 = v_f32_mul_x2_b(s, f[u].v1, f[u].v3, 0, (float128){0}, 1, 0);                        \
            const float128 y1 = v_f32_mul_x2_b(s, f[u].v2, f[u].v4, 0, (float128){0}, 1, 0);                        \
            v_bf16_st_tnsr_partial(co, out, v_convert_f32_to_bf16_all_b(y0, SW_RHNE, (bfloat128){0}, 1, 0), sz, 0, PK, 1, 0); \
            KADV(co, 1);                                                                                            \
            v_bf16_st_tnsr_partial(co, out, v_convert_f32_to_bf16_all_b(y1, SW_RHNE, (bfloat128){0}, 1, 0), sz, 0, PK, 1, 0); \
            KADV(co, 1);                                                                                            \
        }                                                                                                           \
    }

#if defined(PAGED)
#error "no paged fp8"
#endif

void main(tensor W, tensor dep, tensor out, int nmem, int K, int M, int nrb, int nmat, int sk, int smat)
{
    const int5 ioff = get_index_space_offset();
    const int5 iend = ioff + get_index_space_size();
    const int nkb = (K + 63) >> 6;
    const int nitems = nmat * nrb * nkb;
    for (int m = ioff[0]; m < iend[0]; m++) {
        int i0, i1;
        SPLIT_RANGE(nitems, nmem, m, i0, i1)
        for (int it = i0; it < i1; it++) {
            const int kb = it % nkb, blk = it / nkb;
            const int rb = blk % nrb, mat = blk / nrb;
            const int base = rb * (512 + K * 128);
            const int rem = M - rb * 128;
            const int sz = (rem >= 128 ? 128 : rem) - 1;
            int5 c0 = {base, mat, 0, 0, 0};
            float128 s;
            s.v1 = v_f32_ld_tnsr_b(c0, W);
            c0[0] += 256;
            s.v2 = v_f32_ld_tnsr_b(c0, W);
            int5 cw = {base + 512 + 128 * 64 * kb, mat, 0, 0, 0};
            int5 co = {mat * smat + 64 * kb * sk + rb * 128, 0, 0, 0, 0};
            const int kend = 64 * kb + 64 < K ? 64 * kb + 64 : K;
            for (int k = 64 * kb; k < kend; k += 16) {
                FP8_BODY(cw, co, sz)
            }
        }
    }
}

#elif defined(QT_Q8_0)

// q8_0 planar: per 128-row block and 32-step superblock [128 x f16 d][16 x 256 B signed codes]; a code vector holds
// k-steps 2 m, 2 m + 1: (row i, 2 m), (i, 2 m + 1), (64 + i, 2 m), (64 + i, 2 m + 1), times d, as bf16.
// One item = (row block, superblock).
static inline void q8_sb(tensor W, tensor out, int pb, int mat, int5 co, int sz, int sk)
{
    int5 cw = {pb, mat, 0, 0, 0};
    float128 s;
    {
        const float128 d = convert_half128_to_float128(v_f16_ld_tnsr_b(cw, W, SW_UNPACK | SW_UNPCK_16_TO_32), 0);
        s.v1 = d.v1;
        s.v2 = d.v2;
    }
    cw[0] += 256;
    #pragma unroll 2
    for (int h = 0; h < 2; h++) {
        char256 q[8];
        #pragma unroll
        for (int u = 0; u < 8; u++) {
            q[u] = v_i8_ld_tnsr_b(cw, W);
            cw[0] += 256;
        }
        #pragma unroll
        for (int u = 0; u < 8; u++) {
            const float256 f = v_convert_i8_to_f32_all_b(q[u], 0, (float256){0}, 1, 0);
            const float128 y0 = v_f32_mul_x2_b(s, f.v1, f.v3, 0, (float128){0}, 1, 0);
            const float128 y1 = v_f32_mul_x2_b(s, f.v2, f.v4, 0, (float128){0}, 1, 0);
            v_bf16_st_tnsr_partial(co, out, v_convert_f32_to_bf16_all_b(y0, SW_RHNE, (bfloat128){0}, 1, 0), sz, 0, PK, 1, 0);
            KADV(co, 1);
            v_bf16_st_tnsr_partial(co, out, v_convert_f32_to_bf16_all_b(y1, SW_RHNE, (bfloat128){0}, 1, 0), sz, 0, PK, 1, 0);
            KADV(co, 1);
        }
    }
}

#if defined(PAGED)
static inline int ldpe8(tensor pe, int p)
{
    int5 c = {p, 0, 0, 0, 0};
    return s_i32_ld_g((__global int *) gen_addr(c, pe));
}

void main(tensor W, tensor pe, tensor dep, tensor out, int nmem, int K, int M)
{
    const int5 ioff = get_index_space_offset();
    const int5 iend = ioff + get_index_space_size();
    const int nsb = K / SB;
    const int sk = 1;   // (KADV moves coordinate 1: k)
    scalar_cache_reset();
    for (int p = ioff[2]; p < iend[2]; p++) {
        const int mat = ldpe8(pe, p);
        if (mat < 0) {
            continue;
        }
        for (int rb = ioff[0]; rb < iend[0]; rb++) {
            const int rem = M - rb * 128;
            const int sz = (rem >= 128 ? 128 : rem) - 1;
            for (int sb = ioff[1]; sb < iend[1]; sb++) {
                int5 co = {rb * 128, sb * SB, p, 0, 0};
                q8_sb(W, out, (rb * nsb + sb) * SBB, mat, co, sz, sk);
            }
        }
    }
}
#else
void main(tensor W, tensor dep, tensor out, int nmem, int K, int M, int nrb, int nmat, int sk, int smat)
{
    const int5 ioff = get_index_space_offset();
    const int5 iend = ioff + get_index_space_size();
    const int nsb = K / SB;
    const int nitems = nmat * nrb * nsb;
    for (int m = ioff[0]; m < iend[0]; m++) {
        int i0, i1;
        SPLIT_RANGE(nitems, nmem, m, i0, i1)
        for (int it = i0; it < i1; it++) {
            const int sb = it % nsb, rb = (it / nsb) % nrb, mat = it / (nsb * nrb);
            const int rem = M - rb * 128;
            const int sz = (rem >= 128 ? 128 : rem) - 1;
            int5 co = {mat * smat + sb * SB * sk + rb * 128, 0, 0, 0, 0};
            q8_sb(W, out, (rb * nsb + sb) * SBB, mat, co, sz, sk);
        }
    }
}
#endif // PAGED

#else // planar types

#define NGR (SB / 32)                                 // groups per superblock
#define P2OFF (PBYTES + P4 * (SB / 4) * 256)
#define P1OFF (P2OFF + P2 * (SB / 8) * 256)
#define SIDX(g) (G == 16 ? 2 * (g) : (g) * 32 / G)    // scale group of group g's first chunk

__local__ float64 PS[NG][4];   // per scale group: S rows 0..63, 64..127, MN rows 0..63, 64..127

// field of w bits at bit b of u moved to bit s (constants)
#define FLD(u, b, w, s)                                                                                             \
    (((s) == 0 && (b) + (w) == 8) ? (uchar256) ((u) >> (unsigned char) (b)) :                                       \
     ((b) == 0 && (s) + (w) == 8) ? (uchar256) ((u) << (unsigned char) (s)) :                                       \
     (uchar256) ((((s) > (b) ? (u) << (unsigned char) ((s) - (b)) : (s) < (b) ? (u) >> (unsigned char) ((b) - (s)) : (u))) & \
                 (unsigned char) (((1 << (w)) - 1) << (s))))

// the superblock's parameters at pb: f16 plane f as f32 rows 0..63 / 64..127; byte planes p, p + 1
// as dword i = (plane p row i, row 64 + i, plane p + 1 row i, row 64 + i)
#define LD_F16P(f) convert_half128_to_float128(v_f16_ld_tnsr_b(cpar(pb + 256 * (f), mat), W, SW_UNPACK | SW_UNPCK_16_TO_32), 0)
#define LD_BP2(p) v_u8_ld_tnsr_b(cpar(pb + NF16 * 256 + 128 * (p), mat), W, SW_UNPACK | SW_UNPCK_8_TO_32)
#define U2F(x) v_convert_u8_to_f32_all_b(x, 0, (float256){0}, 1, 0)
#define PUT(q, a, b, c, d) { PS[q][0] = (a); PS[q][1] = (b); PS[q][2] = (c); PS[q][3] = (d); }

static inline int5 cpar(int p, int mat)
{
    int5 c = {p, mat, 0, 0, 0};
    return c;
}

// S and MN of every scale group of the superblock at pb of matrix mat into PS (as qt_gp in syn_qmm.c)
static inline void sb_params(tensor W, int pb, int mat)
{
#if defined(QT_Q4_0) || defined(QT_Q5_0) || defined(QT_TQ2_0) || defined(QT_Q2_0) || defined(QT_Q1_0)
#if defined(QT_Q4_0)
    const float off = -8.0f, sm = 1.0f;
#elif defined(QT_Q5_0)
    const float off = -16.0f, sm = 1.0f;
#elif defined(QT_Q1_0)
    const float off = -1.0f, sm = 2.0f;
#else
    const float off = -1.0f, sm = 1.0f;
#endif
    const float128 d = LD_F16P(0);
    PUT(0, d.v1 * sm, d.v2 * sm, d.v1 * off, d.v2 * off)
#elif defined(QT_Q4_1) || defined(QT_Q5_1)
    const float128 d = LD_F16P(0), m = LD_F16P(1);
    PUT(0, d.v1, d.v2, m.v1, m.v2)
#elif defined(QT_Q2_K)
    const float128 d = LD_F16P(0);
    const ushort128 dmb = v_u16_ld_tnsr_b(cpar(pb + 256, mat), W, SW_UNPACK | SW_UNPCK_16_TO_32) ^ (unsigned short) 0x8000;
    const float128 dm = convert_half128_to_float128(as_half128(dmb), 0);   // -dmin
    #pragma unroll
    for (int p = 0; p < 16; p += 2) {
        const uchar256 u = LD_BP2(p);
        const float256 sc = U2F(u & (unsigned char) 15), mm = U2F(u >> (unsigned char) 4);
        PUT(p, d.v1 * sc.v1, d.v2 * sc.v2, dm.v1 * mm.v1, dm.v2 * mm.v2)
        PUT(p + 1, d.v1 * sc.v3, d.v2 * sc.v4, dm.v1 * mm.v3, dm.v2 * mm.v4)
    }
#elif defined(QT_Q3_K)
    // group 4 q + b: low 4 bits from plane b (q = 0, 2) or 4 + b (q = 1, 3), low (q < 2) or high nibble; bits 4-5
    // from plane 8 + b at bit 2 q
    const float128 d = LD_F16P(0);
    #pragma unroll 1
    for (int q = 0; q < 4; q++) {
        #pragma unroll
        for (int b = 0; b < 4; b += 2) {
            const uchar256 lo = LD_BP2((q & 1) * 4 + b), hi = LD_BP2(8 + b);
            const uchar256 l4 = (q & 2) ? (uchar256) (lo >> (unsigned char) 4) : (uchar256) (lo & (unsigned char) 15);
            const uchar256 h2 = (hi >> (unsigned char) (2 * q)) & (unsigned char) 3;
            const float256 sc = U2F(l4 | (h2 << (unsigned char) 4));
            const float64 s0 = d.v1 * (sc.v1 - 32.0f), s1 = d.v2 * (sc.v2 - 32.0f);
            const float64 s2 = d.v1 * (sc.v3 - 32.0f), s3 = d.v2 * (sc.v4 - 32.0f);
            PUT(4 * q + b, s0, s1, -4.0f * s0, -4.0f * s1)
            PUT(4 * q + b + 1, s2, s3, -4.0f * s2, -4.0f * s3)
        }
    }
#elif defined(QT_Q4_K) || defined(QT_Q5_K)
    // 8 -> 32 unpacked scale planes: 6-bit scales and mins of all 8 groups
    const float128 d = LD_F16P(0);
    const ushort128 dmb = v_u16_ld_tnsr_b(cpar(pb + 256, mat), W, SW_UNPACK | SW_UNPCK_16_TO_32) ^ (unsigned short) 0x8000;
    const float128 dm = convert_half128_to_float128(as_half128(dmb), 0);
    uchar256 P[6];
    #pragma unroll
    for (int p = 0; p < 6; p++) {
        P[p] = LD_BP2(2 * p);
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
    #pragma unroll
    for (int q = 0; q < 4; q++) {
        const float256 s = U2F(v6[q]), n = U2F(v6[4 + q]);
        PUT(2 * q, s.v1 * d.v1, s.v2 * d.v2, n.v1 * dm.v1, n.v2 * dm.v2)
        PUT(2 * q + 1, s.v3 * d.v1, s.v4 * d.v2, n.v3 * dm.v1, n.v4 * dm.v2)
    }
#elif defined(QT_Q6_K)
    const float128 d = LD_F16P(0);
    #pragma unroll 2
    for (int p = 0; p < 16; p += 2) {
        const char256 u = v_i8_ld_tnsr_b(cpar(pb + 256 + 128 * p, mat), W, SW_UNPACK | SW_UNPCK_8_TO_32);
        const float256 sc = v_convert_i8_to_f32_all_b(u, 0, (float256){0}, 1, 0);
        const float64 s0 = d.v1 * sc.v1, s1 = d.v2 * sc.v2, s2 = d.v1 * sc.v3, s3 = d.v2 * sc.v4;
        PUT(p, s0, s1, -32.0f * s0, -32.0f * s1)
        PUT(p + 1, s2, s3, -32.0f * s2, -32.0f * s3)
    }
#elif defined(QT_MXFP4)
    // 2^(e - 128) x 128 (e >= 2; smaller e are denormal scales, flushed; e > 248 clamped)
    const float256 e = U2F(LD_BP2(0));
    #pragma unroll
    for (int h = 0; h < 2; h++) {
        const float64 eh = h ? e.v2 : e.v1;
        const int64 b = v_i32_min_b(v_convert_f32_to_i32_b(eh, SW_RHNE, 0, 1, 0), (int64) 248);
        PS[0][h] = v_f32_sel_less_f32_b(eh, 2.0f, 0.0f, as_float64(v_i32_shl_b(b + 6, (int64) 23)));
    }
    PS[0][2] = 0.0f;
    PS[0][3] = 0.0f;
#elif defined(QT_NVFP4)
    // unsigned e4m3 (bias 7) x 0.5 x 128; 0 and 0x7F are 0
    #pragma unroll
    for (int p = 0; p < 4; p += 2) {
        const uchar256 u = LD_BP2(p);
        const float256 ex = U2F(u >> (unsigned char) 3), mn = U2F(u & (unsigned char) 7), raw = U2F(u);
        #pragma unroll
        for (int h = 0; h < 4; h++) {
            const float64 e_ = h == 0 ? ex.v1 : h == 1 ? ex.v2 : h == 2 ? ex.v3 : ex.v4;
            const float64 m_ = h == 0 ? mn.v1 : h == 1 ? mn.v2 : h == 2 ? mn.v3 : mn.v4;
            const float64 r_ = h == 0 ? raw.v1 : h == 1 ? raw.v2 : h == 2 ? raw.v3 : raw.v4;
            const int64 ei = v_convert_f32_to_i32_b(e_, SW_RHNE, 0, 1, 0);
            const float64 p2 = as_float64(v_i32_shl_b(ei + 126, (int64) 23));   // 2^(e - 1)
            const float64 v = v_f32_sel_eq_f32_b(e_, 0.0f, m_ * 0.125f, (1.0f + m_ * 0.125f) * p2);
            PS[p + h / 2][h & 1] = v_f32_sel_eq_f32_b(r_, 127.0f, 0.0f, v);
        }
        PS[p][2] = 0.0f;
        PS[p][3] = 0.0f;
        PS[p + 1][2] = 0.0f;
        PS[p + 1][3] = 0.0f;
    }
#else
#error "no parameters for this type"
#endif
}

#define SB_PARAMS(pb, mat) sb_params(W, (pb), (mat));

#if NG * (NF16 + NBP) <= 8
#define NEXT_PARAMS(cond, STEP_ARGS, pb1, mat1) { STEP STEP_ARGS SB_PARAMS(pb1, mat1) }
#else
#define NEXT_PARAMS(cond, STEP_ARGS, pb1, mat1)                                                                     \
    for (int r = 0; r < (cond); r++) {                                                                              \
        STEP STEP_ARGS                                                                                              \
        SB_PARAMS(pb1, mat1)                                                                                        \
    }
#endif

// P4 chunk at c as two byte vectors ready for the f32 convert (k-steps 4 c + o, 4 c + 2 + o)
#if defined(LUT_FP4)
static inline uchar512 ld4(tensor W, int5 c)
{
    const uchar256 b = v_u8_ld_tnsr_b(c, W);
    uchar512 r;
    r.v1 = ((b << (unsigned char) 4) & (unsigned char) 0x80) | ((b << (unsigned char) 2) & (unsigned char) 0x1C);
    r.v2 = (b & (unsigned char) 0x80) | ((b >> (unsigned char) 2) & (unsigned char) 0x1C);
    return r;
}
#define CV8(x) v_convert_f8_to_f32_all_b(__builtin_astype((x), minifloat256), 0, (float256){0}, 1, 0)
#elif defined(LUT_IQ4)
#error "IQ4 types use syn_qmm.c -DDEQ"
#else
#define ld4(W, c) v_convert_u4_to_u8_all_b(v_u8_ld_tnsr_b(c, W), 0, (uchar512){0}, 1, 0)
#define CV8(x) v_convert_u8_to_f32_all_b(x, 0, (float256){0}, 1, 0)
#endif

// the first code vectors of the group whose planes start at p4 / p2 / p1 (byte offsets)
#define PRELOAD(cl, p4, p2, p1)                                                                                     \
    {                                                                                                               \
        if (P4) {                                                                                                   \
            cl[0] = (p4);                                                                                           \
            nb0 = ld4(W, cl);                                                                                       \
            cl[0] += 256;                                                                                           \
            nb1 = ld4(W, cl);                                                                                       \
        }                                                                                                           \
        if (P2) {                                                                                                   \
            cl[0] = (p2);                                                                                           \
            u20 = v_u8_ld_tnsr_b(cl, W);                                                                            \
        }                                                                                                           \
        if (P1) {                                                                                                   \
            cl[0] = (p1);                                                                                           \
            h0 = v_u8_ld_tnsr_b(cl, W);                                                                             \
            cl[0] += 256;                                                                                           \
            h1 = v_u8_ld_tnsr_b(cl, W);                                                                             \
        }                                                                                                           \
    }

// the scales of group g (the second pair: chunks 4..7 of a 16-step scale group)
#define LD_SCALES(g)                                                                                                \
    {                                                                                                               \
        const int si_ = SIDX(g);                                                                                    \
        s.v1 = PS[si_][0];                                                                                          \
        s.v2 = PS[si_][1];                                                                                          \
        mn.v1 = PS[si_][2];                                                                                         \
        mn.v2 = PS[si_][3];                                                                                         \
        if (G == 16) {                                                                                              \
            s2.v1 = PS[si_ + 1][0];                                                                                 \
            s2.v2 = PS[si_ + 1][1];                                                                                 \
            mn2.v1 = PS[si_ + 1][2];                                                                                \
            mn2.v2 = PS[si_ + 1][3];                                                                                \
        }                                                                                                           \
    }

// group G of the current item; plast4 / plast2 / plast1: the next item's planes (used by the last group)
#define GROUP(G_, plast4, plast2, plast1, matn)                                                                     \
    {                                                                                                               \
        const int g = (G_);                                                                                         \
        _Pragma("unroll")                                                                                           \
        for (int j = 0; j < 4; j++) {                                                                               \
            v_bf16_st_tnsr_partial(cp, out, zp[j], szp, 0, PK, pend_st, 0);                                         \
            KADV(cp, 1);                                                                                            \
        }                                                                                                           \
        uchar256 u2 = 0;                                                                                            \
        _Pragma("unroll")                                                                                           \
        for (int c = 0; c < 8; c++) {                                                                               \
            uchar256 lo = 0, hi = 0;                                                                                \
            if (P4) {                                                                                               \
                uchar512 nb;                                                                                        \
                if (c == 0) {                                                                                       \
                    nb = nb0;                                                                                       \
                } else if (c == 1) {                                                                                \
                    nb = nb1;                                                                                       \
                } else {                                                                                            \
                    nb = ld4(W, cw);                                                                                \
                    cw[0] += 256;                                                                                   \
                }                                                                                                   \
                lo = nb.v1;                                                                                         \
                hi = nb.v2;                                                                                         \
            }                                                                                                       \
            if (P2) {                                                                                               \
                if (c == 0) {                                                                                       \
                    u2 = u20;                                                                                       \
                } else if ((c & 1) == 0) {                                                                          \
                    u2 = v_u8_ld_tnsr_b(c2, W);                                                                     \
                    c2[0] += 256;                                                                                   \
                }                                                                                                   \
                const int b_ = 4 * (c & 1);                                                                         \
                lo = lo | FLD(u2, b_, 2, S2);                                                                       \
                hi = hi | FLD(u2, b_ + 2, 2, S2);                                                                   \
            }                                                                                                       \
            if (P1) {                                                                                               \
                const uchar256 hv = c < 4 ? h0 : h1;                                                                \
                const int b_ = 2 * (c & 3);                                                                         \
                lo = lo | FLD(hv, b_, 1, S1);                                                                       \
                hi = hi | FLD(hv, b_ + 1, 1, S1);                                                                   \
            }                                                                                                       \
            const float256 qa = CV8(lo);                                                                            \
            const float256 qb = CV8(hi);                                                                            \
            const float64 q[4][2] = {{qa.v1, qa.v3}, {qa.v2, qa.v4}, {qb.v1, qb.v3}, {qb.v2, qb.v4}};               \
            _Pragma("unroll")                                                                                       \
            for (int j = 0; j < 4; j++) {                                                                           \
                const float128 y = G == 16 && c >= 4 ? v_f32_madd_x2_b(s2, q[j][0], mn2, q[j][1], 0, (float128){0}, 1, 0) : \
                                                       v_f32_madd_x2_b(s, q[j][0], mn, q[j][1], 0, (float128){0}, 1, 0); \
                const bfloat128 z = v_convert_f32_to_bf16_all_b(y, SW_RHNE, (bfloat128){0}, 1, 0);                  \
                if (c < 7) {                                                                                        \
                    v_bf16_st_tnsr_partial(co, out, z, sz, 0, PK, 1, 0);                                            \
                    KADV(co, 1);                                                                                    \
                } else {                                                                                            \
                    zp[j] = z;                                                                                      \
                }                                                                                                   \
            }                                                                                                       \
        }                                                                                                           \
        cp = co;                                                                                                    \
        KADV(co, 4);                                                                                                \
        szp = sz;                                                                                                   \
        pend_st = true;                                                                                             \
        int5 cl = cw;                                                                                               \
        if (g == NGR - 1) {                                                                                         \
            cl[1] = (matn);                                                                                         \
            PRELOAD(cl, plast4, plast2, plast1)                                                                     \
        } else {                                                                                                    \
            PRELOAD(cl, pb + PBYTES + 2048 * (g + 1), pb + P2OFF + 1024 * (g + 1), pb + P1OFF + 512 * (g + 1))      \
            cw[0] += 512;                                                                                           \
            c2[0] += 256;                                                                                           \
        }                                                                                                           \
        LD_SCALES(g < NGR - 1 ? g + 1 : 0)                                                                          \
    }

#define ITEM_DECLS                                                                                                  \
    float128 s, mn, s2, mn2;                                                                                        \
    uchar512 nb0, nb1;                                                                                              \
    uchar256 u20, h0, h1;                                                                                           \
    bfloat128 zp[4];                                                                                                \
    zp[0] = 0;                                                                                                      \
    zp[1] = 0;                                                                                                      \
    zp[2] = 0;                                                                                                      \
    zp[3] = 0;                                                                                                      \
    int5 cp = {0, 0, 0, 0, 0};                                                                                      \
    int szp = 0;                                                                                                    \
    bool pend_st = false;

#define FLUSH                                                                                                       \
    _Pragma("unroll")                                                                                               \
    for (int j = 0; j < 4; j++) {                                                                                   \
        v_bf16_st_tnsr_partial(cp, out, zp[j], szp, 0, PK, pend_st, 0);                                             \
        KADV(cp, 1);                                                                                                \
    }

#if defined(PAGED)

static inline int ldpe(tensor pe, int p)
{
    int5 c = {p, 0, 0, 0, 0};
    return s_i32_ld_g((__global int *) gen_addr(c, pe));
}

// the next used page after p (pend: none)
static inline int next_page(tensor pe, int p, int pend)
{
    p++;
    while (p < pend && ldpe(pe, p) < 0) {
        p++;
    }
    return p;
}

// index state of the item after (sb, rb, p, mat, pb): superblock, row block, page, its matrix, planar offset
#define STEP(sb, rb, p, mat, pb, SB_, RB_, P_, MAT_, PB_)                                                           \
    {                                                                                                               \
        const bool w1_ = (sb) + 1 == sbe;                                                                           \
        const bool w2_ = w1_ && (rb) + 1 == rbe;                                                                    \
        SB_ = w1_ ? sbb : (sb) + 1;                                                                                 \
        RB_ = w1_ ? (w2_ ? rbb : (rb) + 1) : (rb);                                                                  \
        P_ = w2_ ? next_page(pe, (p), pend) : (p);                                                                  \
        MAT_ = w2_ ? (P_ < pend ? ldpe(pe, P_) : 0) : (mat);                                                        \
        PB_ = (RB_ * nsb + SB_) * SBB;                                                                              \
    }

void main(tensor W, tensor pe, tensor dep, tensor out, int nmem, int K, int M)
{
    const int5 ioff = get_index_space_offset();
    const int5 iend = ioff + get_index_space_size();
    const int nsb = K / SB;
    const int rbb = ioff[0], rbe = iend[0], sbb = ioff[1], sbe = iend[1], pend = iend[2];
    scalar_cache_reset();
    int npg = 0;
    for (int q = ioff[2]; q < pend; q++) {
        npg += ldpe(pe, q) >= 0;
    }
    const int nitems = npg * (rbe - rbb) * (sbe - sbb);
    if (nitems == 0) {
        return;
    }
    int sb = sbb, rb = rbb, p = next_page(pe, ioff[2] - 1, pend);
    int mat = ldpe(pe, p);
    int pb = (rb * nsb + sb) * SBB;
    ITEM_DECLS
    SB_PARAMS(pb, mat)
    LD_SCALES(0)
    int5 cn = {0, mat, 0, 0, 0};
    PRELOAD(cn, pb + PBYTES, pb + P2OFF, pb + P1OFF)
    int sb1, rb1, p1, mat1, pb1;
    STEP(sb, rb, p, mat, pb, sb1, rb1, p1, mat1, pb1)
    int sb2 = 0, rb2 = 0, p2 = 0, mat2 = 0, pb2 = 0;
    for (int it = 0; it < nitems; it++) {
        const int rem = M - rb * 128;
        const int sz = (rem >= 128 ? 128 : rem) - 1;
        int5 co = {rb * 128, sb * SB, p, 0, 0};
        int5 cw = {pb + PBYTES + 512, mat, 0, 0, 0};
        int5 c2 = {pb + P2OFF + 256, mat, 0, 0, 0};
        for (int gi = 0; gi < NGR - 1; gi++) {
            GROUP(gi, 0, 0, 0, 0)
        }
        // compute the next item's scales
        NEXT_PARAMS(it + 1 < nitems, (sb1, rb1, p1, mat1, pb1, sb2, rb2, p2, mat2, pb2), pb1, mat1)
        GROUP(NGR - 1, pb1 + PBYTES, pb1 + P2OFF, pb1 + P1OFF, mat1)
        sb = sb1;
        rb = rb1;
        p = p1;
        mat = mat1;
        pb = pb1;
        sb1 = sb2;
        rb1 = rb2;
        p1 = p2;
        mat1 = mat2;
        pb1 = pb2;
    }
    FLUSH
}

#else

// index state of the item after (sb, rem, mat, pb, mb, ob): superblock, rows left in the row block, matrix,
// planar offset, matrix output base, output base
#define STEP(sb, rem, mat, pb, mb, ob, SB_, REM_, MAT_, PB_, MB_, OB_)                                              \
    {                                                                                                               \
        const bool w1_ = (sb) + 1 == nsb;                                                                           \
        const int r1_ = w1_ ? (rem) - 128 : (rem);                                                                  \
        const bool w2_ = w1_ && r1_ <= 0;                                                                           \
        SB_ = w1_ ? 0 : (sb) + 1;                                                                                   \
        REM_ = w2_ ? M : r1_;                                                                                       \
        MAT_ = w2_ ? (mat) + 1 : (mat);                                                                             \
        PB_ = w2_ ? 0 : (pb) + SBB;                                                                                 \
        MB_ = w2_ ? (mb) + smat : (mb);                                                                             \
        OB_ = w2_ ? MB_ : (ob) + (w1_ ? drb : dsb);                                                                 \
    }

void main(tensor W, tensor dep, tensor out, int nmem, int K, int M, int nrb, int nmat, int sk, int smat)
{
    const int5 ioff = get_index_space_offset();
    const int5 iend = ioff + get_index_space_size();
    const int nsb = K / SB;
    const int nitems = nmat * nrb * nsb;
    const int dsb = SB * sk, drb = 128 - (nsb - 1) * SB * sk;
    for (int m = ioff[0]; m < iend[0]; m++) {
        int i0, i1;
        SPLIT_RANGE(nitems, nmem, m, i0, i1)
        if (i0 >= i1) {
            continue;
        }
        int sb = i0 % nsb, rb = (i0 / nsb) % nrb, mat = i0 / (nsb * nrb);
        int pb = (rb * nsb + sb) * SBB;
        int mb = mat * smat, ob = mb + sb * SB * sk + rb * 128, rem = M - rb * 128;
        ITEM_DECLS
        SB_PARAMS(pb, mat)
        LD_SCALES(0)
        int5 cn = {0, mat, 0, 0, 0};
        PRELOAD(cn, pb + PBYTES, pb + P2OFF, pb + P1OFF)
        int sb1, rem1, mat1, pb1, mb1, ob1;
        STEP(sb, rem, mat, pb, mb, ob, sb1, rem1, mat1, pb1, mb1, ob1)
        int sb2 = 0, rem2 = 0, mat2 = 0, pb2 = 0, mb2 = 0, ob2 = 0;
        for (int it = i0; it < i1; it++) {
            const int sz = (rem >= 128 ? 128 : rem) - 1;
            int5 co = {ob, 0, 0, 0, 0};
            int5 cw = {pb + PBYTES + 512, mat, 0, 0, 0};
            int5 c2 = {pb + P2OFF + 256, mat, 0, 0, 0};
            for (int gi = 0; gi < NGR - 1; gi++) {
                GROUP(gi, 0, 0, 0, 0)
            }
            // compute the next item's scales
            NEXT_PARAMS(it + 1 < i1, (sb1, rem1, mat1, pb1, mb1, ob1, sb2, rem2, mat2, pb2, mb2, ob2), pb1, mat1)
            GROUP(NGR - 1, pb1 + PBYTES, pb1 + P2OFF, pb1 + P1OFF, mat1)
            sb = sb1;
            rem = rem1;
            mat = mat1;
            pb = pb1;
            mb = mb1;
            ob = ob1;
            sb1 = sb2;
            rem1 = rem2;
            mat1 = mat2;
            pb1 = pb2;
            mb1 = mb2;
            ob1 = ob2;
        }
        FLUSH
    }
}

#endif // PAGED
#endif // planar types
