// SPDX-License-Identifier: AGPL-3.0-only
// Copyright Aaron Botsis
// int8 activation for the quantized int8 GEMVs (syn_qmm -DI8): x [K, N] f32 (column view: element
// offset + column stride, dim 0 contiguous), K a multiple of 32, per 32-step group g:
//   dx = max |x| / 127, xq = round(x / dx)
//   X8[n][p] (256 B): the k-pair p = (2p, 2p + 1) as alternating bytes xq[2p], xq[2p + 1]
//   -DXQ (compact): X8[n][j] (256 B): steps 8 j .. 8 j + 7, xq[8 j + (l & 7)] in byte l
//   XG[n][k / 32]: four uniform 64-lane f32 vectors (1 KB): dx, dx * sum of the 32 xq, dx * sum of each 16 xq
// tensors: 0 x | 1 X8  2 XG
#include "syn_common.h"

void main(tensor x, tensor dep, tensor X8, tensor XG, int nmem, int K, int N, int offx, int sx1)
{
    const int5 ioff = get_index_space_offset();
    const int5 iend = ioff + get_index_space_size();
    DECL_IDX(IDX)
    const int64 parw = LANE_I32() & (int64) 1;
    const int nb = (K + 63) >> 6;   // the last block may hold one 32-step group
    const int nitems = N * nb;
    for (int m = ioff[0]; m < iend[0]; m++) {
        int i0, i1;
        SPLIT_RANGE(nitems, nmem, m, i0, i1)
        for (int it = i0; it < i1; it++) {
            const int n = it / nb, b = it - n * nb;
            const int k0 = b << 6;
            const float64 xv = ld_f32(x, offx + n * sx1 + k0);
            // max |x| of each 32-lane half -> quantized x (rounded floats) and per-16 sums
            const float64 ax = v_f32_abs_b(xv);
            const float64 t = v_f32_max_b(ax, v_f32_mov_dual_group_all_b(ax, 0xFFFFFFFF, 1, 0, 3, 2, WR_ALL, undef_f64()));
            float64 m0 = SHUF(t, IDX[0]), m1 = SHUF(t, IDX[1]), m2 = SHUF(t, IDX[2]), m3 = SHUF(t, IDX[3]);
            #pragma unroll
            for (int q = 4; q < 16; q += 4) {
                m0 = v_f32_max_b(m0, SHUF(t, IDX[q]));
                m1 = v_f32_max_b(m1, SHUF(t, IDX[q + 1]));
                m2 = v_f32_max_b(m2, SHUF(t, IDX[q + 2]));
                m3 = v_f32_max_b(m3, SHUF(t, IDX[q + 3]));
            }
            const float64 mx = v_f32_max_b(v_f32_max_b(m0, m1), v_f32_max_b(m2, m3));
            const float64 dx = mx * (1.0f / 127.0f);
            const float64 id = v_f32_sel_grt_f32_b(mx, 0.0f, 127.0f * nl_recip(v_f32_max_b(mx, 1e-30f)), 0.0f);
            const float64 xq = v_convert_i32_to_f32_b(v_convert_f32_to_i32_b(xv * id, SW_RHNE, 0, 1, 0), SW_RHNE, 0, 1, 0);
            float64 s0 = SHUF(xq, IDX[0]), s1 = SHUF(xq, IDX[1]), s2 = SHUF(xq, IDX[2]), s3 = SHUF(xq, IDX[3]);
            #pragma unroll
            for (int q = 4; q < 16; q += 4) {
                s0 += SHUF(xq, IDX[q]);
                s1 += SHUF(xq, IDX[q + 1]);
                s2 += SHUF(xq, IDX[q + 2]);
                s3 += SHUF(xq, IDX[q + 3]);
            }
            const float64 s = (s0 + s1) + (s2 + s3);   // integers below 2^24: exact in any order
            const float64 xs = s * dx;     // each lane: dx * the sum of its 16 xq
            // per 32 steps: dx, the 32-step sum, the 16-step sums
            #pragma unroll
            for (int h = 0; h < 2; h++) {
                const bool ok = k0 + 32 * h < K;
                float64 bdx, x16a, x16b;   // dx, 16-step sums of this half
                if (h == 0) { bdx = BCAST_DG(dx, 0); x16a = BCAST_DG(xs, 0); x16b = BCAST_DG(xs, 1); }
                else { bdx = BCAST_DG(dx, 2); x16a = BCAST_DG(xs, 2); x16b = BCAST_DG(xs, 3); }
                bdx = SHUF(bdx, IDX[0]);
                x16a = SHUF(x16a, IDX[0]);
                x16b = SHUF(x16b, IDX[0]);
                int5 cg = {(n * (K >> 5) + (k0 >> 5) + h) * 256, 0, 0, 0, 0};
                v_f32_st_tnsr(cg, XG, bdx, 0, ok);
                cg[0] += 64;
                v_f32_st_tnsr(cg, XG, x16a + x16b, 0, ok);
                cg[0] += 64;
                v_f32_st_tnsr(cg, XG, x16a, 0, ok);
                cg[0] += 64;
                v_f32_st_tnsr(cg, XG, x16b, 0, ok);
            }
            float256 g4;
            g4.v1 = xq; g4.v2 = xq; g4.v3 = xq; g4.v4 = xq;
            const char256 rep = convert_float256_to_char256(g4, SW_RHNE | SW_LINEAR);   // byte l: xq[l & 63]
#if defined(XQ)
            // XQ vector j: 8 steps, x[8 j + (l & 7)] in byte l. As words: x[8 j .. 8 j + 3] even, x[8 j + 4 ..] odd
            const float64 rw = as_float64(rep);   // word e of each dual group: xq[4 e .. 4 e + 3]
            #pragma unroll
            for (int j = 0; j < 8; j++) {
                const int64 v = v_i32_sel_eq_i32_b(parw, 0, as_int64(SHUF(rw, IDX[2 * j])), as_int64(SHUF(rw, IDX[2 * j + 1])));
                int5 cx = {(n * (K >> 3) + (k0 >> 3) + j) * 256, 0, 0, 0, 0};
                v_i32_st_tnsr(cx, X8, v, 0, k0 + 8 * j < K);
            }
#else
            // k-pair p: short p of each dual group (xq[2 p], xq[2 p + 1]) in every short lane (uniform 16-bit shuffle:
            // element p & 15 of group p >> 4)
            const short128 rs = as_short128(rep);
            int5 cx = {(n * (K >> 1) + (k0 >> 1)) * 256, 0, 0, 0, 0};
            #pragma unroll 1
            for (int g = 0; g < 2; g++) {
                uchar256 sel = (unsigned char) (0x80 | (g << 5));
                const bool ok = k0 + 32 * g < K;
                #pragma unroll 4
                for (int p = 0; p < 16; p++) {
                    v_i16_st_tnsr(cx, X8, v_i16_shuffle_b(rs, sel, 0, rs), 0, ok);
                    cx[0] += 256;
                    sel = sel + (unsigned char) 1;
                }
            }
#endif
        }
    }
}
