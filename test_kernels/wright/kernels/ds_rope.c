// SPDX-License-Identifier: AGPL-3.0-only
// Copyright Aaron Botsis
// DeepSeek-V4.1 RoPE (adjacent pairs (2i, 2i+1)) on the last 64 rows of each column, after
//   mode & 1: an RMSNorm of the column, v = x * rsqrt(mean x^2 + eps) * nw
//   mode & 2: a per-token scale, v = x * ts[t]
// and written as f32, or (mode & 4) as bf16 in natural order. Column c belongs to token c / hpt. cs (128, T): each
// token's position, [cos_(l>>1) | (l odd ? 1 : -1) sin_(l>>1)].
// tensors: 0 x (D, N) f32  1 cs (128, T) f32  2 ts (T) f32  3 nw (D) f32 (any tensor when unused) | 4 y (D, N)
// params: hpt, mode, eps.  index space: [N].  D a multiple of 128, at most 512.
#include "common.h"

#define MAXV 8

void main(tensor x, tensor cs, tensor ts, tensor nw, tensor y, int hpt, int mode, float eps)
{
    const int5 off = get_index_space_offset();
    const int5 end = off + get_index_space_size();
    const int D = get_dim_size(x, 0), nv = D / 64;
    const int64 lane = (int64)read_lane_id_4b_b();
    const int64 e = (lane ^ 1) & 15;                                   // pair partner within the dual group
    const int64 ctl = (0x80 | ((e >> 3) << 5) | (e & 7)) * 0x01010101;
    const uchar256 SW = __builtin_bit_cast(uchar256, ctl);
    DECL_IDX(IDX)
    scalar_cache_reset();
    float64 w[MAXV];
    #pragma unroll
    for (int j = 0; j < MAXV; j++) {
        int5 wc = {64 * j, 0, 0, 0, 0};
        w[j] = (mode & 1) && j < nv ? v_f32_ld_tnsr_b(wc, nw) : (float64)1.0f;
    }
    for (int c = off[0]; c < end[0]; c++) {
        const int t = c / hpt;
        float64 v[MAXV];
        #pragma unroll
        for (int j = 0; j < MAXV; j++) {
            int5 xc = {64 * j, c, 0, 0, 0};
            v[j] = j < nv ? v_f32_ld_tnsr_b(xc, x) : (float64)0.0f;
        }
        float64 sc = 1.0f;
        if (mode & 1) {
            float64 ss = 0.0f;
            #pragma unroll
            for (int j = 0; j < MAXV; j++)
                ss = v_f32_mac_b(v[j], v[j], ss);
            NL_REDUCE_ADD(ss, IDX, ss);
            sc = nl_rsqrt(ss * (1.0f / (float)D) + eps);
        }
        if (mode & 2) {
            int5 tc = {t, 0, 0, 0, 0};
            sc = sc * s_f32_ld_g(gen_addr(tc, ts));
        }
        #pragma unroll
        for (int j = 0; j < MAXV; j++)
            v[j] = v[j] * sc * w[j];
        int5 pc = {0, t, 0, 0, 0};
        const float64 co = v_f32_ld_tnsr_b(pc, cs);
        pc[0] = 64;
        const float64 si = v_f32_ld_tnsr_b(pc, cs);
        #pragma unroll
        for (int j = 0; j < MAXV; j++)
            if (j == nv - 1)
                v[j] = v_f32_mac_b(v_f32_shuffle_b(v[j], SW, 0, v[j]), si, v[j] * co);
        if (mode & 4) {
            #pragma unroll
            for (int j = 0; j < MAXV; j += 2)
                if (j < nv) {
                    float128 p;
                    p.v1 = v[j];
                    p.v2 = v[j + 1];
                    int5 oc = {64 * j, c, 0, 0, 0};
                    v_bf16_st_tnsr(oc, y, v_convert_f32_to_bf16_all_b(p, SW_RHNE | SW_LINEAR));
                }
        } else {
            #pragma unroll
            for (int j = 0; j < MAXV; j++)
                if (j < nv) {
                    int5 oc = {64 * j, c, 0, 0, 0};
                    v_f32_st_tnsr(oc, y, v[j]);
                }
        }
    }
}
