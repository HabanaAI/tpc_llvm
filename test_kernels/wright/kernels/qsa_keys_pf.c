// SPDX-License-Identifier: AGPL-3.0-only
// Copyright Aaron Botsis
// QSA prefill selection keys (all TPCs), computed once for qsa_mask_pk's 32 search passes: for query t
// (position p = pos0 + t) and key position k < nk (nk = 4 floor((p+1)/4), its complete blocks), the
// monotone uint32 encoding of sum_h relu(score) with zero scores offset by -(block + 1) 1e-30 (ties keep
// the lowest blocks); 0 for k >= nk and for padding queries (t >= n). Scores as in qsa_mask_pf.c: key
// position k reads row k of s if k < kb0 = 4 (pos0 / 4), else row L + k - kb0 (the chunk's block keys).
// tensors: 0 s (L + 4 NBc, NH T) f32  1 meta i32 [pos0, n] | 2 K (L + T, T) i32
// params: L. index space: [(L + T) / 1024, T]
#include "common.h"

#define CHK 1024

static inline uint64 mkey(float64 v)
{
    const uint64 b = as_uint64(v);
    return as_uint64(v_f32_sel_less_f32_b(v, 0.0f, as_float64(~b), as_float64(b | (uint64)0x80000000u)));
}

void main(tensor s, tensor meta, tensor K, int L)
{
    const int5 off = get_index_space_offset();
    const int5 end = off + get_index_space_size();
    const int T = get_dim_size(K, 1), LK = get_dim_size(K, 0);
    const int NH = get_dim_size(s, 1) / T;
    scalar_cache_reset();
    const int pos0 = meta_i32(meta, 0), n = meta_i32(meta, 1);
    const int kb0 = 4 * (pos0 >> 2);
    const float64 lanef = v_convert_i32_to_f32_b((int64)read_lane_id_4b_b(), SW_RHNE, 0, 1, 0);
    for (int t = off[1]; t < end[1]; t++) {
        const int p = pos0 + t;
        const int nk = t < n ? 4 * ((p + 1) >> 2) : 0;
        const float nkf = (float)nk;
        for (int ch = off[0]; ch < end[0]; ch++)
            for (int k0 = ch * CHK; k0 < ch * CHK + CHK && k0 < LK; k0 += 64) {
                uint64 key = 0;
                if (k0 < nk) {
                    float64 sa = 0.0f, sb = 0.0f;
                    for (int h = 0; h < NH; h++) {
                        int5 ca = {k0, h + NH * t, 0, 0, 0}, cb = {L + k0 - kb0, h + NH * t, 0, 0, 0};
                        if (k0 < kb0) sa += v_f32_max_b(v_f32_ld_tnsr_b(ca, s), 0.0f);
                        if (k0 + 64 > kb0) sb += v_f32_max_b(v_f32_ld_tnsr_b(cb, s), 0.0f);
                    }
                    const float64 kp = lanef + (float)k0;
                    float64 sc = v_f32_sel_less_f32_b(kp, (float)kb0, sa, sb);
                    const float64 blk = v_convert_i32_to_f32_b(v_i32_shr_b(v_convert_f32_to_i32_b(kp, SW_RZ, 0, 1, 0), (int64)2), SW_RHNE, 0, 1, 0);
                    sc = v_f32_sel_eq_f32_b(sc, 0.0f, (blk + 1.0f) * -1.0e-30f, sc);
                    key = as_uint64(v_f32_sel_less_f32_b(kp, nkf, as_float64(mkey(sc)), as_float64((uint64)0)));
                }
                int5 oc = {k0, t, 0, 0, 0};
                v_i32_st_tnsr(oc, K, as_int64(key));
            }
    }
}
