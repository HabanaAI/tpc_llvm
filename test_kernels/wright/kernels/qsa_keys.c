// SPDX-License-Identifier: AGPL-3.0-only
// Copyright Aaron Botsis
// QSA selection pre-pass (all TPCs): each cached key's monotone uint32 selection key for each query t,
// computed once for qsa_select's 32 search passes: relu-sum of the heads' scores, zero-score tie
// offset -(block+1) 1e-30, the fresh block (b0 = pos0/4, when complete for the query) from the freshly
// computed block key, 0 for keys that are not candidates (>= min(nk, pos0)). See qsa_select.c.
// tensors: 0 s (Lc, NH T) f32  1 q (128, NH T) f32  2 kbs (128, >=1) f32  3 meta i32 | 4 K (Lc, T) i32
// index space: [Lc / 1024, T]
#include "common.h"

#define CHK 1024

static inline uint64 mkey(float64 v)
{
    const uint64 b = as_uint64(v);
    return as_uint64(v_f32_sel_less_f32_b(v, 0.0f, as_float64(~b), as_float64(b | (uint64)0x80000000u)));
}

void main(tensor s, tensor q, tensor kbs, tensor meta, tensor K)
{
    const int5 off = get_index_space_offset();
    const int5 end = off + get_index_space_size();
    const int Lc = get_dim_size(K, 0), T = get_dim_size(K, 1), NH = get_dim_size(s, 1) / T;
    scalar_cache_reset();
    const int pos0 = meta_i32(meta, 0);
    const int b0 = pos0 >> 2;
    const float64 lanef = v_convert_i32_to_f32_b((int64)read_lane_id_4b_b(), SW_RHNE, 0, 1, 0);
    DECL_IDX(IDX)
    int5 kc = {0, 0, 0, 0, 0};
    const float64 kb0 = v_f32_ld_tnsr_b(kc, kbs);
    kc[0] = 64;
    const float64 kb1 = v_f32_ld_tnsr_b(kc, kbs);
    for (int t = off[1]; t < end[1]; t++) {
        const int p = pos0 + t;
        const int nk = 4 * ((p + 1) >> 2);
        const int fresh = nk > 4 * b0;
        const int lim = nk < pos0 ? nk : pos0;
        float64 fs = 0.0f;
        for (int h = 0; h < NH; h++) {
            int5 qc = {0, h + NH * t, 0, 0, 0};
            const float64 q0 = v_f32_ld_tnsr_b(qc, q);
            qc[0] = 64;
            const float64 q1 = v_f32_ld_tnsr_b(qc, q);
            float64 d;
            NL_REDUCE_ADD(v_f32_mac_b(q0, kb0, q1 * kb1), IDX, d);
            fs += v_f32_max_b(d, 0.0f);
        }
        fs = v_f32_sel_eq_f32_b(fs, 0.0f, (float64)(-(float)(b0 + 1) * 1.0e-30f), fs);
        const uint64 fkey = mkey(fs);
        const float fstart = fresh ? (float)(4 * b0) : 3.0e38f, limf = (float)lim;
        for (int ch = off[0]; ch < end[0]; ch++) {
            for (int i0 = ch * CHK; i0 < ch * CHK + CHK && i0 < Lc; i0 += 64) {
                float64 sc = 0.0f;
                for (int h = 0; h < NH; h++) {
                    int5 c = {i0, h + NH * t, 0, 0, 0};
                    sc += v_f32_max_b(v_f32_ld_tnsr_b(c, s), 0.0f);
                }
                const float64 li = lanef + (float)i0;
                const float64 blk = v_convert_i32_to_f32_b(v_i32_shr_b(v_convert_f32_to_i32_b(li, SW_RZ, 0, 1, 0), (int64)2), SW_RHNE, 0, 1, 0);
                sc = v_f32_sel_eq_f32_b(sc, 0.0f, (blk + 1.0f) * -1.0e-30f, sc);
                uint64 key = mkey(sc);
                key = as_uint64(v_f32_sel_less_f32_b(li, fstart, as_float64(key), as_float64(fkey)));
                key = as_uint64(v_f32_sel_less_f32_b(li, limf, as_float64(key), as_float64((uint64)0)));
                int5 oc = {i0, t, 0, 0, 0};
                v_i32_st_tnsr(oc, K, as_int64(key));
            }
        }
    }
}
