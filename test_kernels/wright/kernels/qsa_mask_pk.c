// SPDX-License-Identifier: AGPL-3.0-only
// Copyright Aaron Botsis
// QSA mask for a prefill chunk with past context, from the precomputed selection keys (qsa_keys_pf):
// keys are [past bucket (L) | chunk (T)]; query t (position p = pos0 + t) sees past key k < pos0 and
// chunk key u <= t, restricted to the top KS keys among its complete blocks (keys < nk) plus its
// incomplete tail. Threshold by a 32-step binary search (as qsa_select); the count loop runs 8
// independent accumulators over the keys (non-candidates are 0, below every probe that matters).
// tensors: 0 K (L + T, T) i32  1 meta i32 [pos0, n] | 2 mask (L + T, T) f32 (0 / -30000)
// params: L, KS. index space: [T]
#include "common.h"

#define NEG (-30000.0f)

void main(tensor K, tensor meta, tensor mask, int L, int KS)
{
    const int5 off = get_index_space_offset();
    const int5 end = off + get_index_space_size();
    const int T = get_dim_size(mask, 1), LK = get_dim_size(mask, 0);
    scalar_cache_reset();
    const int pos0 = meta_i32(meta, 0), n = meta_i32(meta, 1);
    const float64 lanef = v_convert_i32_to_f32_b((int64)read_lane_id_4b_b(), SW_RHNE, 0, 1, 0);
    DECL_IDX(IDX)
    const uint64 one = as_uint64((float64)1.0f);
    for (int t = off[0]; t < end[0]; t++) {
        const int p = pos0 + t;
        const int nk = 4 * ((p + 1) >> 2);
        const float nkf = (float)nk;
        const int sel = t < n && nk > KS;
        const int nk64 = (nk + 63) & ~63;
        uint64 lo = 0, hi = (uint64)0xFFFFFFFFu;
        const float ksf = (float)KS;
        if (sel) {
            for (int it = 0; it < 32; it++) {
                const uint64 d = hi - lo;
                const uint64 mid = lo + v_u32_shr_b(d, (int64)1) + (d & (uint64)1);
                float64 c[8] = {0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f};
                int k0 = 0;
                for (; k0 + 512 <= nk64; k0 += 512) {
                    #pragma unroll
                    for (int u = 0; u < 8; u++) {
                        int5 kc = {k0 + 64 * u, t, 0, 0, 0};
                        c[u] += as_float64(v_u32_sel_less_u32_b(as_uint64(v_i32_ld_tnsr_b(kc, K)), mid, (uint64)0, one));
                    }
                }
                for (; k0 < nk64; k0 += 64) {
                    int5 kc = {k0, t, 0, 0, 0};
                    c[0] += as_float64(v_u32_sel_less_u32_b(as_uint64(v_i32_ld_tnsr_b(kc, K)), mid, (uint64)0, one));
                }
                float64 tot;
                NL_REDUCE_ADD(((c[0] + c[1]) + (c[2] + c[3])) + ((c[4] + c[5]) + (c[6] + c[7])), IDX, tot);
                const float64 ok = v_f32_sel_less_f32_b(tot, ksf, 0.0f, 1.0f);
                lo = as_uint64(v_f32_sel_eq_f32_b(ok, 1.0f, as_float64(mid), as_float64(lo)));
                hi = as_uint64(v_f32_sel_eq_f32_b(ok, 1.0f, as_float64(hi), as_float64(mid - (uint64)1)));
            }
        }
        // mask column t: past keys [0, L) then chunk keys [L, L + T)
        for (int kk = 0; kk < LK; kk += 64) {
            const int past = kk < L;
            const int k0 = past ? kk : pos0 + (kk - L);           // key position of lane 0
            const float64 kp = lanef + (float)k0;
            const float lim = past ? (float)pos0 : (float)(p + 1);
            float64 m = v_f32_sel_less_f32_b(kp, lim, 0.0f, NEG);
            if (sel && k0 < nk) {
                int5 kc = {k0, t, 0, 0, 0};
                const uint64 key = as_uint64(v_i32_ld_tnsr_b(kc, K));
                const float64 drop = v_f32_sel_less_f32_b(kp, nkf,
                    as_float64(v_u32_sel_less_u32_b(key, lo, as_uint64((float64)1.0f), as_uint64((float64)0.0f))), 0.0f);
                m = v_f32_sel_eq_f32_b(drop, 1.0f, (float64)NEG, m);
            }
            int5 mc = {kk, t, 0, 0, 0};
            v_f32_st_tnsr(mc, mask, m);
        }
    }
}
