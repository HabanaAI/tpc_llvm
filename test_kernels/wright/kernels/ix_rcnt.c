// SPDX-License-Identifier: AGPL-3.0-only
// Copyright Aaron Botsis
// DSA top-k threshold by radix, the counting half of a pass (DeepSeek-V4.1; ix_rsel completes it): for the
// keys t_i = lo + i 2^sh (i = 1 .. 15; lo from state), how many of a member's visible entries of the column
// have mkey(score) >= t_i. The M members of a column split its visible entries in whole vectors. Columns that
// see at most KS entries are not counted.
// tensors: 0 score (K, T) f32  1 vis (T) i32  2 state (64, T) i32 (lo in every lane)
//        | 3 part (64, M, T) i32 (lane i: the count for t_i; lane 0: 0)
// params: sh, KS. index space: [M, T]
#include "common.h"

static inline uint64 mkey(float64 v)
{
    const uint64 b = as_uint64(v);
    return as_uint64(v_f32_sel_less_f32_b(v, 0.0f, as_float64(~b), as_float64(b | (uint64)0x80000000u)));
}

void main(tensor score, tensor vis, tensor state, tensor part, int sh, int KS)
{
    const int5 off = get_index_space_offset();
    const int5 end = off + get_index_space_size();
    const int M = get_dim_size(part, 1);
    const int64 lane = (int64)read_lane_id_4b_b();
    const float64 lanef = v_convert_i32_to_f32_b(lane, SW_RHNE, 0, 1, 0);
    DECL_IDX(IDX)
    scalar_cache_reset();
    for (int c = off[1]; c < end[1]; c++) {
        int5 vc = {c, 0, 0, 0, 0};
        const int lim = s_i32_ld_g(gen_addr(vc, vis));
        int5 tc = {0, c, 0, 0, 0};
        const uint64 lo = v_u32_ld_tnsr_b(tc, state);
        const float limf = (float)lim;
        const int nv = (lim + 63) / 64;
        for (int m = off[0]; m < end[0]; m++) {
            int64 out = 0;
            if (lim > KS) {
                int64 acc[15];
                uint64 t[15];
                #pragma unroll
                for (int i = 0; i < 15; i++) {
                    acc[i] = 0;
                    t[i] = lo + (uint64)((unsigned)(i + 1) << sh);
                }
                const int v0 = m * nv / M, v1 = (m + 1) * nv / M;
                for (int v = v0; v < v1; v++) {
                    int5 sc = {64 * v, c, 0, 0, 0};
                    uint64 k = mkey(v_f32_ld_tnsr_b(sc, score));
                    k = as_uint64(v_f32_sel_less_f32_b(lanef + (float)(64 * v), limf, as_float64(k), as_float64((uint64)0)));
                    #pragma unroll
                    for (int i = 0; i < 15; i++)
                        acc[i] += as_int64(v_u32_sel_less_u32_b(k, t[i], (uint64)0, (uint64)1));
                }
                #pragma unroll
                for (int i = 0; i < 15; i++) {
                    int64 s;
                    NL_REDUCE_I32(acc[i], IDX, s, v_i32_add_b);
                    out = v_i32_sel_eq_i32_b(lane, (int64)(i + 1), s, out);
                }
            }
            int5 oc = {0, m, c, 0, 0};
            v_i32_st_tnsr(oc, part, out);
        }
    }
}
