// SPDX-License-Identifier: AGPL-3.0-only
// Copyright Aaron Botsis
// DSA top-k threshold per query column (Mistral-Large-4): over the column's visible keys
// 0 .. vis[c] - 1 of the score scratch (ix_slot_acc), the KS-th largest score as a monotone
// u32 key (bisection; exact ties may keep a few more). Columns that see at
// most KS keys keep every key (threshold 1: every float maps to a key >= 1). A key is kept when
// mkey(score) >= thr; mla_slot_pk applies it.
// tensors: 0 score (K, T) f32  1 vis (T) i32 | 2 thr (64, T) i32 (every lane holds the column's
//          threshold)
// params: KS. index space: [T]
// A column that sees at most NV * 64 keys reads its scores once, as keys in VLM (0 past vis[c]: every mid is
// >= 1, so they never count), and the passes count from there; one that sees more re-reads the scratch each pass.
#include "common.h"

#define NV 320  // 20480 keys

__local__ uint64 KEYS[NV];

static inline uint64 mkey(float64 v)
{
    const uint64 b = as_uint64(v);
    return as_uint64(v_f32_sel_less_f32_b(v, 0.0f, as_float64(~b), as_float64(b | (uint64)0x80000000u)));
}

void main(tensor score, tensor vis, tensor thr, int KS)
{
    const int5 off = get_index_space_offset();
    const int5 end = off + get_index_space_size();
    const float64 lanef = v_convert_i32_to_f32_b((int64)read_lane_id_4b_b(), SW_RHNE, 0, 1, 0);
    const uint64 one = as_uint64((float64)1.0f);
    DECL_IDX(IDX)
    scalar_cache_reset();
    for (int c = off[0]; c < end[0]; c++) {
        int5 vc = {c, 0, 0, 0, 0};
        const int lim = s_i32_ld_g(gen_addr(vc, vis));
        const float limf = (float)lim;
        uint64 lo = 1;
        if (lim > KS && lim <= NV * 64) {
            // vectors in VLM: whole groups of eight (the count chains)
            const int nv = ((lim + 511) >> 9) << 3;
            #pragma unroll 4
            for (int j = 0; j < nv; j++) {
                int5 sc = {64 * j, c, 0, 0, 0};
                const uint64 k = mkey(v_f32_ld_tnsr_b(sc, score));
                KEYS[j] = as_uint64(v_f32_sel_less_f32_b(lanef + (float)(64 * j), limf, as_float64(k), as_float64((uint64)0)));
            }
            uint64 hi = (uint64)0xFFFFFFFFu;
            for (int it = 0; it < 32; it++) {
                const uint64 d = hi - lo;
                const uint64 mid = lo + v_u32_shr_b(d, (int64)1) + (d & (uint64)1);
                uint64 n[8] = {0, 0, 0, 0, 0, 0, 0, 0};  // integer counts (exact), eight chains
                for (int j = 0; j < nv; j += 8) {
                    #pragma unroll
                    for (int u = 0; u < 8; u++)
                        n[u] += v_u32_sel_less_u32_b(KEYS[j + u], mid, (uint64)0, (uint64)1);
                }
                const uint64 all = ((n[0] + n[1]) + (n[2] + n[3])) + ((n[4] + n[5]) + (n[6] + n[7]));
                float64 tot;
                NL_REDUCE_ADD(v_convert_i32_to_f32_b(as_int64(all), SW_RHNE, 0, 1, 0), IDX, tot);
                const float64 ok = v_f32_sel_less_f32_b(tot, (float)KS, 0.0f, 1.0f);  // >= KS at or above mid
                lo = as_uint64(v_f32_sel_eq_f32_b(ok, 1.0f, as_float64(mid), as_float64(lo)));
                hi = as_uint64(v_f32_sel_eq_f32_b(ok, 1.0f, as_float64(hi), as_float64(mid - (uint64)1)));
            }
        } else if (lim > KS) {  // more than NV * 64 keys: ix_thr's passes over the scratch
            uint64 hi = (uint64)0xFFFFFFFFu;
            for (int it = 0; it < 32; it++) {
                const uint64 d = hi - lo;
                const uint64 mid = lo + v_u32_shr_b(d, (int64)1) + (d & (uint64)1);
                float64 c0 = 0.0f, c1 = 0.0f, c2 = 0.0f, c3 = 0.0f;
                for (int j = 0; j < lim; j += 256) {
                    int5 sc = {j, c, 0, 0, 0};
                    const uint64 k0 = mkey(v_f32_ld_tnsr_b(sc, score));
                    sc[0] = j + 64;
                    const uint64 k1 = mkey(v_f32_ld_tnsr_b(sc, score));
                    sc[0] = j + 128;
                    const uint64 k2 = mkey(v_f32_ld_tnsr_b(sc, score));
                    sc[0] = j + 192;
                    const uint64 k3 = mkey(v_f32_ld_tnsr_b(sc, score));
                    const float64 r0 = lanef + (float)j;
                    c0 += v_f32_sel_less_f32_b(r0, limf, as_float64(v_u32_sel_less_u32_b(k0, mid, (uint64)0, one)), 0.0f);
                    c1 += v_f32_sel_less_f32_b(r0 + 64.0f, limf, as_float64(v_u32_sel_less_u32_b(k1, mid, (uint64)0, one)), 0.0f);
                    c2 += v_f32_sel_less_f32_b(r0 + 128.0f, limf, as_float64(v_u32_sel_less_u32_b(k2, mid, (uint64)0, one)), 0.0f);
                    c3 += v_f32_sel_less_f32_b(r0 + 192.0f, limf, as_float64(v_u32_sel_less_u32_b(k3, mid, (uint64)0, one)), 0.0f);
                }
                float64 tot;
                NL_REDUCE_ADD((c0 + c1) + (c2 + c3), IDX, tot);
                const float64 ok = v_f32_sel_less_f32_b(tot, (float)KS, 0.0f, 1.0f);
                lo = as_uint64(v_f32_sel_eq_f32_b(ok, 1.0f, as_float64(mid), as_float64(lo)));
                hi = as_uint64(v_f32_sel_eq_f32_b(ok, 1.0f, as_float64(hi), as_float64(mid - (uint64)1)));
            }
        }
        int5 oc = {0, c, 0, 0, 0};
        v_i32_st_tnsr(oc, thr, __builtin_bit_cast(int64, lo));
    }
}
