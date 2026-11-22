// SPDX-License-Identifier: AGPL-3.0-only
// Copyright Aaron Botsis
// DSA kept-key counts per 64-key block (Mistral-Large-4), for ix_list: over column c's visible
// keys (vis[c]) of the score scratch, the keys whose mkey(score) is above the column's
// threshold (ix_thr) and the ties (equal to it), packed as strict + 65536 * ties, and per group
// of 64 blocks their sums.
// tensors: 0 score (K, T) f32  1 vis (T) i32  2 thr (64, T) i32 | 3 cnt (K / 64, T) i32
//          4 gsum (K / 4096, T) i32
// index space: [K / 4096, T] (a member: 64 blocks of a column, one output vector)
#include "common.h"

static inline uint64 mkey(float64 v)
{
    const uint64 b = as_uint64(v);
    return as_uint64(v_f32_sel_less_f32_b(v, 0.0f, as_float64(~b), as_float64(b | (uint64)0x80000000u)));
}

void main(tensor score, tensor vis, tensor thr, tensor cnt, tensor gsum)
{
    const int5 off = get_index_space_offset();
    const int5 end = off + get_index_space_size();
    const float64 lanef = v_convert_i32_to_f32_b((int64)read_lane_id_4b_b(), SW_RHNE, 0, 1, 0);
    const int64 lane = (int64)read_lane_id_4b_b();
    DECL_IDX(IDX)
    scalar_cache_reset();
    for (int c = off[1]; c < end[1]; c++) {
        int5 vc = {c, 0, 0, 0, 0};
        const int lim = s_i32_ld_g(gen_addr(vc, vis));
        const float limf = (float)lim;
        int5 tc = {0, c, 0, 0, 0};
        const uint64 th = v_u32_ld_tnsr_b(tc, thr);
        for (int q = off[0]; q < end[0]; q++) {
            int64 acc = 0;
            for (int i = 0; i < 64; i++) {
                const int j = (q * 64 + i) * 64;  // first key of the block
                if (j >= lim)
                    break;
                int5 sc = {j, c, 0, 0, 0};
                const uint64 k = mkey(v_f32_ld_tnsr_b(sc, score));
                const float64 live = v_f32_sel_less_f32_b(lanef + (float)j, limf, 1.0f, 0.0f);
                const float64 lt = as_float64(v_u32_sel_less_u32_b(k, th, as_uint64((float64)1.0f), (uint64)0));
                const float64 eq = as_float64(v_u32_sel_eq_u32_b(k, th, as_uint64((float64)1.0f), (uint64)0));
                const float64 packed = live * ((1.0f - lt - eq) + 65536.0f * eq);
                float64 tot;
                NL_REDUCE_ADD(packed, IDX, tot);
                acc = v_i32_sel_eq_i32_b(lane, (int64)i, v_convert_f32_to_i32_b(tot, SW_RHNE, 0, 1, 0), acc);
            }
            int5 oc = {q * 64, c, 0, 0, 0};
            v_i32_st_tnsr(oc, cnt, acc);
            int64 sum;
            NL_REDUCE_I32(acc, IDX, sum, v_i32_add_b);
            oc[0] = q;
            v_i32_st_tnsr_partial(oc, gsum, sum, 0, 0);
        }
    }
}
