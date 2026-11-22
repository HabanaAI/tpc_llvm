// SPDX-License-Identifier: AGPL-3.0-only
// Copyright Aaron Botsis
// Prefill MoE: routing ids of the overflow pairs only (local, real token, bm == -1); everything else
// becomes -1 (not local for any rank), so moe_up / moe_down process exactly the overflow.
// tensors: 0 ids (64,T) i32  1 bm (64,T) i32  2 meta i32 | 3 ido (64,T) i32
// params: lo, E (local experts), K. index space: [T]
#include "common.h"

void main(tensor ids, tensor bm, tensor meta, tensor ido, int lo, int E, int K)
{
    const int5 off = get_index_space_offset();
    const int5 end = off + get_index_space_size();
    scalar_cache_reset();
    const int n_real = meta_i32(meta, 1);
    const int64 lane = (int64)read_lane_id_4b_b();
    for (int t = off[0]; t < end[0]; t++) {
        int5 c = {0, t, 0, 0, 0};
        const int64 v = v_i32_ld_tnsr_b(c, ids) - lo;
        const int64 b = v_i32_ld_tnsr_b(c, bm);
        // local: 0 <= v < E and lane < K; overflow: b == -1
        int64 ok = v_i32_sel_less_i32_b(v, (int64)0, (int64)0, (int64)1);
        ok = v_i32_sel_less_i32_b(v, (int64)E, ok, (int64)0);
        ok = v_i32_sel_less_i32_b(lane, (int64)K, ok, (int64)0);
        ok = v_i32_sel_eq_i32_b(b, (int64)-1, ok, (int64)0);
        ok = t < n_real ? ok : (int64)0;
        v_i32_st_tnsr(c, ido, v_i32_sel_eq_i32_b(ok, (int64)1, v + lo, (int64)-1));
    }
}
