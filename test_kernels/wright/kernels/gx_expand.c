// SPDX-License-Identifier: AGPL-3.0-only
// Copyright Aaron Botsis
// GLM DSA, decode: a column's kept pools (ix_list, at most 512) as key positions for the sparse attention gather:
// pool P -> 4P .. 4P + 3, then the tail 4 vis .. klen - 1 (always kept, including the token's own key: at most
// 3 + 1 keys past the last complete pool), and the slot record
//   slots[:, c] = [group (0, or -1 with no keys), H c, C, 0, count, c, 0, 0]
// (qcol0 = H c: query block 1; qpos0 = C: every listed key is visible; key0 = 0).
// tensors: 0 pools (KS, T) i32  1 pslots (8, T) i32 (ix_list's record: [4] = kept pools)  2 vis (T) i32
//          3 klen (T) i32 | 4 keys (C, T) i32  5 slots (8, T) i32
// params: H, C. index space: [T]
#include "common.h"

void main(tensor pools, tensor pslots, tensor vis, tensor klen, tensor keys, tensor slots, int H, int C)
{
    const int5 off = get_index_space_offset();
    const int5 end = off + get_index_space_size();
    const int64 lane = (int64)read_lane_id_4b_b();
    const int64 lane2 = v_i32_shr_b(lane, 2);
    const int64 low2 = lane & 3;
    uchar256 IDX4[16];
    #pragma unroll
    for (int e = 0; e < 16; e++) IDX4[e] = SHUF_IDX(e);
    scalar_cache_reset();
    for (int c = off[0]; c < end[0]; c++) {
        int5 pc = {4, c, 0, 0, 0};
        const int kept = s_i32_ld_g(gen_addr(pc, pslots));
        int5 vc = {c, 0, 0, 0, 0};
        const int nvis = s_i32_ld_g(gen_addr(vc, vis));
        const int lim = s_i32_ld_g(gen_addr(vc, klen));
        int n = 0;
        for (int q = 0; q < kept; q += 64) {
            int5 lc = {q, c, 0, 0, 0};
            const float64 ids = as_float64(v_i32_ld_tnsr_b(lc, pools));
            #pragma unroll
            for (int dg = 0; dg < 4; dg++) {
                const float64 sd = dg == 0 ? BCAST_DG(ids, 0) : dg == 1 ? BCAST_DG(ids, 1) : dg == 2 ? BCAST_DG(ids, 2) : BCAST_DG(ids, 3);
                int64 r = 0;
                #pragma unroll
                for (int e = 0; e < 16; e++)
                    r = v_i32_sel_eq_i32_b(lane2, (int64)e, as_int64(v_f32_shuffle_b(sd, IDX4[e], 0, sd)), r);
                int5 oc = {4 * q + 64 * dg, c, 0, 0, 0};
                v_i32_st_tnsr(oc, keys, v_i32_shl_b(r, 2) + low2);
            }
        }
        // The tail as vector stores after the pools: they replace the pool lanes past 4 kept.
        n = 4 * kept;
        const int room = C - n;
        const int tail = lim - 4 * nvis < room ? lim - 4 * nvis : room;
        for (int t = 0; t < tail; t += 64) {
            int5 oc = {n + t, c, 0, 0, 0};
            v_i32_st_tnsr(oc, keys, lane + (int64)(4 * nvis + t));
        }
        n += tail > 0 ? tail : 0;
        int64 rec = 0;
        rec = v_i32_sel_eq_i32_b(lane, (int64)0, (int64)(n > 0 ? 0 : -1), rec);
        rec = v_i32_sel_eq_i32_b(lane, (int64)1, (int64)(H * c), rec);
        rec = v_i32_sel_eq_i32_b(lane, (int64)2, (int64)C, rec);
        rec = v_i32_sel_eq_i32_b(lane, (int64)4, (int64)n, rec);
        rec = v_i32_sel_eq_i32_b(lane, (int64)5, (int64)c, rec);
        int5 rc = {0, c, 0, 0, 0};
        v_i32_st_tnsr_partial(rc, slots, rec, 7, 0);
    }
}
