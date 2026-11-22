// SPDX-License-Identifier: AGPL-3.0-only
// Copyright Aaron Botsis
// GLM DSA, a prefill chunk's selection as an additive per-key mask for mla_slot_pm: per query column, a key is
// kept (0) when its pool is visible (below vis[c]) and kept (mkey(score) >= thr[c]: ix_thr at KS = 512 pools), or
// it is in the column's tail (positions 4 vis[c] .. klen[c] - 1, which no complete pool covers); else -3e38.
// Only the 256-key groups below the chunk's end pos0 + n are written, for every column.
// The rest of the mask keeps what it held.
// tensors: 0 score (Kp, T) f32  1 vis (T) i32 (complete pools)  2 klen (T) i32 (keys seen: position + 1)
//          3 thr (64, T) i32  4 meta [pos0, n] | 5 mask (4 Kp, T) f32
// index space: [4 Kp / 256, T]: one member reads 64 pools and writes 4 vectors of 64 keys.
#include "common.h"

#define NINF (-3.0e38f)

static inline uint64 mkey(float64 v)
{
    const uint64 b = as_uint64(v);
    return as_uint64(v_f32_sel_less_f32_b(v, 0.0f, as_float64(~b), as_float64(b | (uint64)0x80000000u)));
}

void main(tensor score, tensor vis, tensor klen, tensor thr, tensor meta, tensor mask)
{
    const int5 off = get_index_space_offset();
    const int5 end = off + get_index_space_size();
    const float64 lanef = v_convert_i32_to_f32_b((int64)read_lane_id_4b_b(), SW_RHNE, 0, 1, 0);
    const int64 lane2 = v_i32_shr_b((int64)read_lane_id_4b_b(), 2);
    DECL_IDX(IDX)
    scalar_cache_reset();
    const int groups = (meta_i32(meta, 0) + meta_i32(meta, 1) + 255) >> 8;
    const int bend = end[0] < groups ? end[0] : groups;
    if (off[0] >= bend)
        return;
    for (int c = off[1]; c < end[1]; c++) {
        int5 vc = {c, 0, 0, 0, 0};
        const int nvis = s_i32_ld_g(gen_addr(vc, vis));
        const float visf = (float)nvis;
        const float tail0 = (float)(4 * nvis);
        const float limf = (float)s_i32_ld_g(gen_addr(vc, klen));
        int5 tc = {0, c, 0, 0, 0};
        const uint64 th = __builtin_bit_cast(uint64, v_i32_ld_tnsr_b(tc, thr));
        for (int b = off[0]; b < bend; b++) {
            int5 pc = {64 * b, c, 0, 0, 0};
            const float64 pool = lanef + (float)(64 * b);
            const uint64 kept = v_u32_sel_less_u32_b(mkey(v_f32_ld_tnsr_b(pc, score)), th, as_uint64((float64)NINF), as_uint64((float64)0.0f));
            const float64 m = v_f32_sel_less_f32_b(pool, visf, as_float64(kept), NINF);
            #pragma unroll
            for (int dg = 0; dg < 4; dg++) {
                const float64 sd = dg == 0 ? BCAST_DG(m, 0) : dg == 1 ? BCAST_DG(m, 1) : dg == 2 ? BCAST_DG(m, 2) : BCAST_DG(m, 3);
                float64 r = NINF;
                #pragma unroll
                for (int e = 0; e < 16; e++)
                    r = as_float64(v_i32_sel_eq_i32_b(lane2, (int64)e, as_int64(v_f32_shuffle_b(sd, IDX[e], 0, sd)), as_int64(r)));
                const int k0 = 256 * b + 64 * dg;
                const float64 key = lanef + (float)k0;
                // the tail: tail0 <= key < klen
                const float64 intail = v_f32_sel_less_f32_b(key, limf, v_f32_sel_less_f32_b(key, tail0, r, 0.0f), r);
                int5 oc = {k0, c, 0, 0, 0};
                v_f32_st_tnsr(oc, mask, intail);
            }
        }
    }
}
