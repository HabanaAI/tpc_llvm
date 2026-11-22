// SPDX-License-Identifier: AGPL-3.0-only
// Copyright Aaron Botsis
// QSA block selection for the T query tokens of a decode launch (T = 1, or 1 + drafts <= 5 for speculative steps),
// key layout: every key carries its block's score. Query t sits at position p = pos0 + t; its complete
// blocks cover keys [0, nk), nk = 4 floor((p+1)/4); key i scores sum_h relu(s[i, h + NH t]). The block
// b0 = pos0 / 4 may be completed by the launch's own tokens: when it is complete for query t it is
// scored with the freshly computed block key (kbs col 0) instead of the cache; likewise block b0 + 1 (kbs col 4,
// all of its keys the launch's own: T = 5 with pos0 % 4 = 3). The top KS keys (= top
// KS/4 blocks) are kept: threshold by a 32-step binary search over a monotone uint32 encoding of the
// scores, all in the vector domain; zero scores are offset by -(block + 1) * 1e-30 so ties at zero keep
// the lowest blocks. Keys in [nk, p) (incomplete tail) are always kept.
// The per-key selection keys come from qsa_keys (all TPCs); this kernel runs the search per query.
// tensors: 0 K (Lc, T) i32 selection keys (qsa_keys)  1 q (128, NH T) f32  2 kbs (128, >=1) f32
//          3 meta i32 [pos0, ..] | 4 km (Lc, T) f32 key bias (0 / -1e30) for the cached keys
//          5 knb (Tn, R) f32 bias of the launch's own keys j per score row r (query r / hpg)
// params: KS (keys kept), hpg. index space: [T]
#include "common.h"

#define NEG (-1.0e30f)

static inline uint64 mkey(float64 v)             // monotone: a < b  <=>  mkey(a) < mkey(b) (unsigned)
{
    const uint64 b = as_uint64(v);
    const uint64 pos = b | (uint64)0x80000000u;
    return as_uint64(v_f32_sel_less_f32_b(v, 0.0f, as_float64(~b), as_float64(pos)));
}

void main(tensor Kt, tensor q, tensor kbs, tensor meta, tensor km, tensor knb, int KS, int hpg)
{
    const int5 off = get_index_space_offset();
    const int5 end = off + get_index_space_size();
    const int Lc = get_dim_size(Kt, 0), T = get_dim_size(km, 1);
    const int NH = get_dim_size(q, 1) / T, Tn = get_dim_size(knb, 0), R = get_dim_size(knb, 1);
    scalar_cache_reset();
    const int pos0 = meta_i32(meta, 0);
    const int b0 = pos0 >> 2;
    const float64 lanef = v_convert_i32_to_f32_b((int64)read_lane_id_4b_b(), SW_RHNE, 0, 1, 0);
    DECL_IDX(IDX)
    int5 kc = {0, 0, 0, 0, 0};
    const float64 kb0 = v_f32_ld_tnsr_b(kc, kbs);
    kc[0] = 64;
    const float64 kb1 = v_f32_ld_tnsr_b(kc, kbs);
    const int has2 = get_dim_size(kbs, 1) > 4;
    kc[1] = 4;
    const float64 kb2 = has2 ? v_f32_ld_tnsr_b(kc, kbs) : (float64)0.0f;
    kc[0] = 0;
    const float64 kb3 = has2 ? v_f32_ld_tnsr_b(kc, kbs) : (float64)0.0f;
    for (int t = off[0]; t < end[0]; t++) {
    const int p = pos0 + t;
    const int nk = 4 * ((p + 1) >> 2);
    const int fresh = nk > 4 * b0;                   // block b0 complete for this query
    const int fresh2 = has2 && nk >= 4 * b0 + 8;     // block b0 + 1 too (own keys only)
    const int lim = nk < pos0 ? nk : pos0;           // cached keys that are block candidates: [0, lim)
    // fresh block score: sum_h relu(q_h . kb)
    float64 fs = 0.0f, fs2 = 0.0f;
    for (int h = 0; h < NH; h++) {
        int5 qc = {0, h + NH * t, 0, 0, 0};
        const float64 q0 = v_f32_ld_tnsr_b(qc, q);
        qc[0] = 64;
        const float64 q1 = v_f32_ld_tnsr_b(qc, q);
        float64 d, d2;
        NL_REDUCE_ADD(v_f32_mac_b(q0, kb0, q1 * kb1), IDX, d);
        fs += v_f32_max_b(d, 0.0f);
        if (fresh2) {
            NL_REDUCE_ADD(v_f32_mac_b(q0, kb2, q1 * kb3), IDX, d2);
            fs2 += v_f32_max_b(d2, 0.0f);
        }
    }
    fs = v_f32_sel_eq_f32_b(fs, 0.0f, (float64)(-(float)(b0 + 1) * 1.0e-30f), fs);
    fs2 = v_f32_sel_eq_f32_b(fs2, 0.0f, (float64)(-(float)(b0 + 2) * 1.0e-30f), fs2);
    const uint64 fkey = mkey(fs), fkey2 = mkey(fs2);
    const float fstart = (float)(4 * b0), limf = (float)lim;
#define SCORE(i0, key, valid)                                                                      \
    {                                                                                              \
        int5 kc_ = {i0, t, 0, 0, 0};                                                               \
        key = as_uint64(v_i32_ld_tnsr_b(kc_, Kt));                                                 \
        valid = v_f32_sel_less_f32_b(lanef + (float)(i0), limf, 1.0f, 0.0f);                       \
    }
    // the fresh block's keys among the launch's own tokens (positions pos0 .. nk-1) count as candidates
    const int e1 = nk < 4 * b0 + 4 ? nk : 4 * b0 + 4;   // own keys of block b0: [pos0, e1)
    const float nfresh = fresh ? (float)(e1 - pos0) : 0.0f, nfresh2 = fresh2 ? 4.0f : 0.0f;
    uint64 lo = 0, hi = (uint64)0xFFFFFFFFu;
    const float ksf = (float)KS;
    for (int it = 0; it < 32; it++) {
        const uint64 d = hi - lo;
        const uint64 mid = lo + v_u32_shr_b(d, (int64)1) + (d & (uint64)1);
        // non-candidate keys are 0 (qsa_keys) and mid >= 1 whenever it matters, so no validity mask;
        // 4 independent accumulators
        const uint64 one = as_uint64((float64)1.0f);
        float64 c0 = 0.0f, c1 = 0.0f, c2 = 0.0f, c3 = 0.0f;
        int5 k0 = {0, t, 0, 0, 0}, k1 = {64, t, 0, 0, 0}, k2 = {128, t, 0, 0, 0}, k3 = {192, t, 0, 0, 0};
        int i0 = 0;
        for (; i0 + 512 <= lim; i0 += 512) {
            k0[0] = i0; k1[0] = i0 + 64; k2[0] = i0 + 128; k3[0] = i0 + 192;
            c0 += as_float64(v_u32_sel_less_u32_b(as_uint64(v_i32_ld_tnsr_b(k0, Kt)), mid, (uint64)0, one));
            c1 += as_float64(v_u32_sel_less_u32_b(as_uint64(v_i32_ld_tnsr_b(k1, Kt)), mid, (uint64)0, one));
            c2 += as_float64(v_u32_sel_less_u32_b(as_uint64(v_i32_ld_tnsr_b(k2, Kt)), mid, (uint64)0, one));
            c3 += as_float64(v_u32_sel_less_u32_b(as_uint64(v_i32_ld_tnsr_b(k3, Kt)), mid, (uint64)0, one));
            k0[0] = i0 + 256; k1[0] = i0 + 320; k2[0] = i0 + 384; k3[0] = i0 + 448;
            c0 += as_float64(v_u32_sel_less_u32_b(as_uint64(v_i32_ld_tnsr_b(k0, Kt)), mid, (uint64)0, one));
            c1 += as_float64(v_u32_sel_less_u32_b(as_uint64(v_i32_ld_tnsr_b(k1, Kt)), mid, (uint64)0, one));
            c2 += as_float64(v_u32_sel_less_u32_b(as_uint64(v_i32_ld_tnsr_b(k2, Kt)), mid, (uint64)0, one));
            c3 += as_float64(v_u32_sel_less_u32_b(as_uint64(v_i32_ld_tnsr_b(k3, Kt)), mid, (uint64)0, one));
        }
        for (; i0 + 256 <= lim; i0 += 256) {
            k0[0] = i0; k1[0] = i0 + 64; k2[0] = i0 + 128; k3[0] = i0 + 192;
            c0 += as_float64(v_u32_sel_less_u32_b(as_uint64(v_i32_ld_tnsr_b(k0, Kt)), mid, (uint64)0, one));
            c1 += as_float64(v_u32_sel_less_u32_b(as_uint64(v_i32_ld_tnsr_b(k1, Kt)), mid, (uint64)0, one));
            c2 += as_float64(v_u32_sel_less_u32_b(as_uint64(v_i32_ld_tnsr_b(k2, Kt)), mid, (uint64)0, one));
            c3 += as_float64(v_u32_sel_less_u32_b(as_uint64(v_i32_ld_tnsr_b(k3, Kt)), mid, (uint64)0, one));
        }
        for (; i0 < lim; i0 += 64) {
            uint64 key; float64 valid;
            SCORE(i0, key, valid)
            c0 += valid * as_float64(v_u32_sel_less_u32_b(key, mid, (uint64)0, one));
        }
        float64 tot;
        NL_REDUCE_ADD((c0 + c1) + (c2 + c3), IDX, tot);
        tot += as_float64(v_u32_sel_less_u32_b(fkey, mid, as_uint64((float64)0.0f), as_uint64((float64)nfresh)));
        tot += as_float64(v_u32_sel_less_u32_b(fkey2, mid, as_uint64((float64)0.0f), as_uint64((float64)nfresh2)));
        const float64 ok = v_f32_sel_less_f32_b(tot, ksf, 0.0f, 1.0f);           // 1: count >= KS
        lo = as_uint64(v_f32_sel_eq_f32_b(ok, 1.0f, as_float64(mid), as_float64(lo)));
        hi = as_uint64(v_f32_sel_eq_f32_b(ok, 1.0f, as_float64(hi), as_float64(mid - (uint64)1)));
    }
    // cached keys: candidates below the threshold are dropped; the tail [nk, p) and beyond stay 0
    for (int i0 = 0; i0 < Lc; i0 += 64) {
        uint64 key; float64 valid;
        SCORE(i0, key, valid)
        const float64 keep = as_float64(v_u32_sel_less_u32_b(key, lo, as_uint64((float64)0.0f), as_uint64((float64)1.0f)));
        int5 oc = {i0, t, 0, 0, 0};
        v_f32_st_tnsr(oc, km, v_f32_sel_eq_f32_b(valid * (1.0f - keep), 1.0f, (float64)NEG, (float64)0.0f));
    }
    // the launch's own keys j (positions pos0 + j) for this query's score rows: dropped when they belong
    // to the fresh block (j < nk - pos0) and that block is not kept
    const float64 fdrop = as_float64(v_u32_sel_less_u32_b(fkey, lo, as_uint64((float64)NEG), as_uint64((float64)0.0f)));
    const float64 fdrop2 = as_float64(v_u32_sel_less_u32_b(fkey2, lo, as_uint64((float64)NEG), as_uint64((float64)0.0f)));
    float64 nb = v_f32_sel_less_f32_b(lanef, fresh ? (float)(e1 - pos0) : 0.0f, fdrop, (float64)0.0f);
    if (fresh2)                                       // own keys [e1, e1 + 4): block b0 + 1
        nb = v_f32_sel_geq_f32_b(lanef, (float)(e1 - pos0), v_f32_sel_less_f32_b(lanef, (float)(e1 - pos0 + 4), fdrop2, nb), nb);
    for (int r = hpg * t; r < hpg * (t + 1) && r < R; r++)
        for (int j0 = 0; j0 < Tn; j0 += 64) {
            int5 rc = {j0, r, 0, 0, 0};
            v_f32_st_tnsr(rc, knb, j0 == 0 ? nb : (float64)0.0f);
        }
    }
}
