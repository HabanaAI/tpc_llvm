// SPDX-License-Identifier: AGPL-3.0-only
// Copyright Aaron Botsis
// Q5_K planar -> bf16 for the paged MoE batch_gemm (the paged interface of syn_qdq.c -DPAGED): syn_qdqp_q4k.c with the
// fifth code bit. Q5_K's planar superblock is Q4_K's (same params, same P4 plane) followed by the P1 plane: 16
// groups of 256 B, group m holding bit 4 of k = 16 m + 2 t + o at bit t, in the P4 byte lanes. P4 chunk c (k =
// 4 c .. 4 c + 3) takes its low nibbles' bit from bit 2 (c & 3) and its high nibbles' from bit 2 (c & 3) + 1 of
// P1 group c / 4: two P1 vectors per group of 8 chunks, merged as u8 before the f32 convert.
// tensors: 0 W [bytes per matrix, matrices] i8  1 pe [P] i32  2 dep | 3 out [M, K, P] bf16
// Rows start rb0 128-row blocks into each matrix. sbk: unused here.
#include "syn_common.h"

#define SB 256
#define PBYTES (2 * 256 + 12 * 128)
#define P1OFF (PBYTES + 64 * 256)
#define SBB (P1OFF + 16 * 256)
#define PK (SW_PACK | SW_PCK_32_TO_16)

__local__ float64 PS[8][4];   // per group: S rows 0..63, 64..127, MN rows 0..63, 64..127

// S = d * scale, MN = -dmin * min of every group of the superblock at pb into PS
// 8 -> 32 unpacked scale planes: dword i = (plane p row i, row 64 + i, plane p + 1 row i, row 64 + i)
#define SB_PARAMS(pb, mat)                                                                                      \
    {                                                                                                           \
        int5 cq = {(pb), (mat), 0, 0, 0};                                                                       \
        const float128 d = convert_half128_to_float128(v_f16_ld_tnsr_b(cq, W, SW_UNPACK | SW_UNPCK_16_TO_32), 0); \
        cq[0] = (pb) + 256;                                                                                     \
        const ushort128 dmb = v_u16_ld_tnsr_b(cq, W, SW_UNPACK | SW_UNPCK_16_TO_32) ^ (unsigned short) 0x8000;  \
        const float128 dm = convert_half128_to_float128(as_half128(dmb), 0);                                    \
        uchar256 P[6];                                                                                          \
        _Pragma("unroll")                                                                                       \
        for (int p = 0; p < 6; p++) {                                                                           \
            cq[0] = (pb) + 512 + 256 * p;                                                                       \
            P[p] = v_u8_ld_tnsr_b(cq, W, SW_UNPACK | SW_UNPCK_8_TO_32);                                         \
        }                                                                                                       \
        const uchar512 n89 = v_convert_u4_to_u8_all_b(P[4], 0, (uchar512){0}, 1, 0);                            \
        const uchar512 nab = v_convert_u4_to_u8_all_b(P[5], 0, (uchar512){0}, 1, 0);                            \
        uchar256 v6[8];                                                                                         \
        v6[0] = P[0] & (unsigned char) 63;                                                                      \
        v6[1] = P[1] & (unsigned char) 63;                                                                      \
        v6[2] = n89.v1 | ((P[0] >> 2) & (unsigned char) 0x30);                                                  \
        v6[3] = nab.v1 | ((P[1] >> 2) & (unsigned char) 0x30);                                                  \
        v6[4] = P[2] & (unsigned char) 63;                                                                      \
        v6[5] = P[3] & (unsigned char) 63;                                                                      \
        v6[6] = n89.v2 | ((P[2] >> 2) & (unsigned char) 0x30);                                                  \
        v6[7] = nab.v2 | ((P[3] >> 2) & (unsigned char) 0x30);                                                  \
        _Pragma("unroll")                                                                                       \
        for (int q = 0; q < 4; q++) {                                                                           \
            const float256 s = v_convert_u8_to_f32_all_b(v6[q], 0, (float256){0}, 1, 0);                        \
            const float256 n = v_convert_u8_to_f32_all_b(v6[4 + q], 0, (float256){0}, 1, 0);                    \
            PS[2 * q][0] = s.v1 * d.v1;                                                                         \
            PS[2 * q][1] = s.v2 * d.v2;                                                                         \
            PS[2 * q][2] = n.v1 * dm.v1;                                                                        \
            PS[2 * q][3] = n.v2 * dm.v2;                                                                        \
            PS[2 * q + 1][0] = s.v3 * d.v1;                                                                     \
            PS[2 * q + 1][1] = s.v4 * d.v2;                                                                     \
            PS[2 * q + 1][2] = n.v3 * dm.v1;                                                                    \
            PS[2 * q + 1][3] = n.v4 * dm.v2;                                                                    \
        }                                                                                                       \
    }

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
#define STEP(sb, rb, p, mat, pb, SB_, RB_, P_, MAT_, PB_)                                                       \
    {                                                                                                           \
        const bool w1_ = (sb) + 1 == sbe;                                                                       \
        const bool w2_ = w1_ && (rb) + 1 == rbe;                                                                \
        SB_ = w1_ ? sbb : (sb) + 1;                                                                             \
        RB_ = w1_ ? (w2_ ? rbb : (rb) + 1) : (rb);                                                              \
        P_ = w2_ ? next_page(pe, (p), pend) : (p);                                                              \
        MAT_ = w2_ ? (P_ < pend ? ldpe(pe, P_) : 0) : (mat);                                                    \
        PB_ = ((RB_ + rb0) * nsb + SB_) * SBB;                                                                        \
    }

// group G (8 code chunks) of the current item
#define GROUP(G)                                                                                                \
    {                                                                                                           \
        const int g = (G);                                                                                      \
        _Pragma("unroll")                                                                                       \
        for (int j = 0; j < 4; j++) {                                                                           \
            v_bf16_st_tnsr_partial(cp, out, zp[j], szp, 0, PK, pend_st, 0);                                     \
            cp[1] += 1;                                                                                         \
        }                                                                                                       \
        _Pragma("unroll")                                                                                       \
        for (int c = 0; c < 8; c++) {                                                                           \
            uchar512 nb;                                                                                        \
            if (c == 0) {                                                                                       \
                nb = nb0;                                                                                       \
            } else if (c == 1) {                                                                                \
                nb = nb1;                                                                                       \
            } else {                                                                                            \
                nb = v_convert_u4_to_u8_all_b(v_u8_ld_tnsr_b(cw, W), 0, (uchar512){0}, 1, 0);                   \
                cw[0] += 256;                                                                                   \
            }                                                                                                   \
            const uchar256 hv = c < 4 ? h0 : h1;                                                                \
            const int sl = 4 - 2 * (c & 3);                                                                     \
            const uchar256 lo = (sl > 0 ? hv << (unsigned char) sl : hv >> (unsigned char) (-sl)) & (unsigned char) 16; \
            const uchar256 hi = (sl - 1 > 0 ? hv << (unsigned char) (sl - 1) : hv >> (unsigned char) (1 - sl)) & (unsigned char) 16; \
            const float256 qa = v_convert_u8_to_f32_all_b(nb.v1 | lo, 0, (float256){0}, 1, 0);                  \
            const float256 qb = v_convert_u8_to_f32_all_b(nb.v2 | hi, 0, (float256){0}, 1, 0);                  \
            const float64 q[4][2] = {{qa.v1, qa.v3}, {qa.v2, qa.v4}, {qb.v1, qb.v3}, {qb.v2, qb.v4}};           \
            _Pragma("unroll")                                                                                   \
            for (int j = 0; j < 4; j++) {                                                                       \
                const float128 y = v_f32_madd_x2_b(s, q[j][0], mn, q[j][1], 0, (float128){0}, 1, 0);            \
                const bfloat128 z = v_convert_f32_to_bf16_all_b(y, SW_RHNE, (bfloat128){0}, 1, 0);              \
                if (c < 7) {                                                                                    \
                    v_bf16_st_tnsr_partial(co, out, z, sz, 0, PK, 1, 0);                                        \
                    co[1] += 1;                                                                                 \
                } else {                                                                                        \
                    zp[j] = z;                                                                                  \
                }                                                                                               \
            }                                                                                                   \
        }                                                                                                       \
        cp = co;                                                                                                \
        co[1] += 4;                                                                                             \
        szp = sz;                                                                                               \
        pend_st = true;                                                                                         \
        int5 cl = cw;                                                                                           \
        if (g == 7) {                                                                                           \
            cl[0] = pb1 + PBYTES;                                                                               \
            cl[1] = mat1;                                                                                       \
        }                                                                                                       \
        nb0 = v_convert_u4_to_u8_all_b(v_u8_ld_tnsr_b(cl, W), 0, (uchar512){0}, 1, 0);                          \
        cl[0] += 256;                                                                                           \
        nb1 = v_convert_u4_to_u8_all_b(v_u8_ld_tnsr_b(cl, W), 0, (uchar512){0}, 1, 0);                          \
        int5 ch = cl;                                                                                           \
        ch[0] = g == 7 ? pb1 + P1OFF : pb + P1OFF + 512 * (g + 1);                                              \
        h0 = v_u8_ld_tnsr_b(ch, W);                                                                             \
        ch[0] += 256;                                                                                           \
        h1 = v_u8_ld_tnsr_b(ch, W);                                                                             \
        cw[0] += 512;                                                                                           \
        const int ng = g < 7 ? g + 1 : 0;                                                                       \
        s.v1 = PS[ng][0];                                                                                       \
        s.v2 = PS[ng][1];                                                                                       \
        mn.v1 = PS[ng][2];                                                                                      \
        mn.v2 = PS[ng][3];                                                                                      \
    }

void main(tensor W, tensor pe, tensor dep, tensor out, int nmem, int K, int M, int sbk, int rb0)
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
    int pb = ((rb + rb0) * nsb + sb) * SBB;
    SB_PARAMS(pb, mat)
    float128 s, mn;
    s.v1 = PS[0][0];
    s.v2 = PS[0][1];
    mn.v1 = PS[0][2];
    mn.v2 = PS[0][3];
    int5 cn = {pb + PBYTES, mat, 0, 0, 0};
    uchar512 nb0 = v_convert_u4_to_u8_all_b(v_u8_ld_tnsr_b(cn, W), 0, (uchar512){0}, 1, 0);
    cn[0] += 256;
    uchar512 nb1 = v_convert_u4_to_u8_all_b(v_u8_ld_tnsr_b(cn, W), 0, (uchar512){0}, 1, 0);
    cn[0] = pb + P1OFF;
    uchar256 h0 = v_u8_ld_tnsr_b(cn, W);
    cn[0] += 256;
    uchar256 h1 = v_u8_ld_tnsr_b(cn, W);
    bfloat128 zp[4];
    zp[0] = 0;
    zp[1] = 0;
    zp[2] = 0;
    zp[3] = 0;
    int5 cp = {0, 0, 0, 0, 0};
    int szp = 0;
    bool pend_st = false;
    // the next item's index state
    int sb1, rb1, p1, mat1, pb1;
    STEP(sb, rb, p, mat, pb, sb1, rb1, p1, mat1, pb1)
    int sb2 = 0, rb2 = 0, p2 = 0, mat2 = 0, pb2 = 0;
    for (int it = 0; it < nitems; it++) {
        const int rem = M - rb * 128;
        const int sz = (rem >= 128 ? 128 : rem) - 1;
        int5 co = {rb * 128, sb * SB, p, 0, 0};
        int5 cw = {pb + PBYTES + 512, mat, 0, 0, 0};
        for (int gi = 0; gi < 7; gi++) {
            GROUP(gi)
        }
        // compute the next item's scales
        for (int r = 0; r < (it + 1 < nitems); r++) {
            STEP(sb1, rb1, p1, mat1, pb1, sb2, rb2, p2, mat2, pb2)
            SB_PARAMS(pb1, mat1)
        }
        GROUP(7)
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
    #pragma unroll
    for (int j = 0; j < 4; j++) {
        v_bf16_st_tnsr_partial(cp, out, zp[j], szp, 0, PK, pend_st, 0);
        cp[1] += 1;
    }
}
