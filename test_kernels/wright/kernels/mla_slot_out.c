// SPDX-License-Identifier: AGPL-3.0-only
// Copyright Aaron Botsis
// MLA slot attention output (Mistral-Large-4), for steps whose attention is one slot launch in
// which every query column is in at most one slot (decode: one dense slot per row of at most
// SLOT_KEYS keys, or one sparse slot per column): out = O / l per column, as an output tensor.
// Columns no slot covers, and columns without visible keys, are 0.
// SINK (ds_slot_out.c): each head h = c % hpg has a logit sink[h] in the softmax denominator, with no
// value: out = O / (l + exp(sink[h] - mx)). Tensor 4 is then sink (hpg) f32 and out is tensor 5; param hpg.
// ML (ds_slot_ml.c): also each column's max and sum, mcol / lcol (W, G) f32 (outputs 5, 6; -1e30 and 0 for a
// column no slot covers), for ds_attn_comb.
// tensors: 0 O (>= D, M, S) f32 (rows past D ignored)  1 mx (Mp, S) f32 (SINK and ML builds only)
//          2 l (Mp, S) f32  3 slots (8, S) i32 | 4 out (D, W, G) f32
// index space: [ceil(W / 64), G]; PER_COLUMN builds (ds_slot_ml1.c): one member per column, [W, G]
// DIRECT (ds_slot_mld.c): slot s covers columns M s .. M s + M - 1 (a sparse slot per column of M heads), so a
// column finds its slot without a scan of the slots.
// CPM (ds_slot_ml8.c, with DIRECT): CPM columns per member, index space [ceil(W / CPM), G]; each member stores
// only its own lanes of mcol / lcol.
#include "common.h"

// lane k of v broadcast to all lanes
#define GET_LANE(v, k, out)                                                                  \
    { const float lo_ = (float)(k) - 0.5f, hi_ = (float)(k) + 0.5f;                          \
      float64 t_ = v_f32_sel_less_f32_b(lanef, lo_, 0.0f, v_f32_sel_less_f32_b(lanef, hi_, v, 0.0f)); \
      NL_REDUCE_ADD(t_, IDX, out) }

#ifdef SINK
void main(tensor O, tensor mx, tensor l, tensor slots, tensor sink, tensor out, int hpg)
#elif defined(ML)
void main(tensor O, tensor mx, tensor l, tensor slots, tensor out, tensor mcol, tensor lcol)
#else
void main(tensor O, tensor mx, tensor l, tensor slots, tensor out)
#endif
{
    const int5 off = get_index_space_offset();
    const int5 end = off + get_index_space_size();
    const int M = get_dim_size(O, 1), S = get_dim_size(O, 2), W = get_dim_size(out, 1);
    const int D = get_dim_size(out, 0);
    const float64 lanef = v_convert_i32_to_f32_b((int64)read_lane_id_4b_b(), SW_RHNE, 0, 1, 0);
    DECL_IDX(IDX)
    scalar_cache_reset();
    for (int g = off[1]; g < end[1]; g++) {
        for (int cb = off[0]; cb < end[0]; cb++) {
#ifdef PER_COLUMN
            const int c0 = cb, c1 = cb + 1;               // one column per member
#elif defined(CPM)
            const int c0 = cb * CPM, c1 = c0 + CPM < W ? c0 + CPM : W;
#else
            const int c0 = cb * 64, c1 = c0 + 64 < W ? c0 + 64 : W;
#endif
#ifdef ML
            float64 mv = -1.0e30f, lv = 0.0f;
#endif
#ifdef DIRECT
            for (int c = c0; c < c1; c++) {
                const int s = c / M, m = c - s * M;
                int5 fc = {0, s, 0, 0, 0};
                if (s >= S || s_i32_ld_g(gen_addr(fc, slots)) != g) {
                    int5 oc = {0, c, g, 0, 0};
                    for (int d = 0; d < D; d += 64) {
                        oc[0] = d;
                        v_f32_st_tnsr(oc, out, (float64)0.0f);
                    }
                    continue;
                }
                {
#else
            for (int c = c0; c < c1; c++) {
                int5 oc = {0, c, g, 0, 0};
                for (int d = 0; d < D; d += 64) {
                    oc[0] = d;
                    v_f32_st_tnsr(oc, out, (float64)0.0f);
                }
            }
            for (int s = 0; s < S; s++) {
                int5 fc = {0, s, 0, 0, 0};
                if (s_i32_ld_g(gen_addr(fc, slots)) != g)
                    continue;
                fc[0] = 1;
                const int q0 = s_i32_ld_g(gen_addr(fc, slots));
                const int lo = c0 > q0 ? c0 : q0, hi = c1 < q0 + M ? c1 : q0 + M;
                for (int c = lo; c < hi; c++) {
                    const int m = c - q0;
#endif
                    int5 sc = {m & ~63, s, 0, 0, 0};
                    float64 ls;
                    GET_LANE(v_f32_ld_tnsr_b(sc, l), m & 63, ls)
#ifdef SINK
                    float64 ms;
                    GET_LANE(v_f32_ld_tnsr_b(sc, mx), m & 63, ms)
                    int5 hc = {c % hpg, 0, 0, 0, 0};
                    const float sk = s_f32_ld_g(gen_addr(hc, sink));
                    const float64 den = ls + nl_exp((float64)sk - ms);
                    const float64 inv = v_f32_sel_less_f32_b(ls, 1.0e-30f, 0.0f, nl_recip(den));
#else
                    const float64 inv = v_f32_sel_less_f32_b(ls, 1.0e-30f, 0.0f, nl_recip(v_f32_max_b(ls, 1.0e-30f)));
#endif
#ifdef ML
                    {
                        float64 ms;
                        GET_LANE(v_f32_ld_tnsr_b(sc, mx), m & 63, ms)
                        const float lo_ = (float)(c - c0) - 0.5f, hi_ = (float)(c - c0) + 0.5f;
                        const float64 in_ = v_f32_sel_less_f32_b(lanef, lo_, 0.0f, v_f32_sel_less_f32_b(lanef, hi_, 1.0f, 0.0f));
                        mv = v_f32_sel_grt_f32_b(in_, 0.5f, ms, mv);
                        lv = v_f32_sel_grt_f32_b(in_, 0.5f, ls, lv);
                    }
#endif
                    int5 oc = {0, c, g, 0, 0}, pc = {0, m, s, 0, 0};
                    for (int d = 0; d < D; d += 64) {
                        oc[0] = d;
                        pc[0] = d;
                        v_f32_st_tnsr(oc, out, v_f32_ld_tnsr_b(pc, O) * inv);
                    }
                }
            }
#ifdef ML
            int5 mc = {c0, g, 0, 0, 0};
#ifdef PER_COLUMN
            v_f32_st_tnsr_partial(mc, mcol, mv, 0, 0);
            v_f32_st_tnsr_partial(mc, lcol, lv, 0, 0);
#elif defined(CPM)
            v_f32_st_tnsr_partial(mc, mcol, mv, CPM - 1, 0);   // lanes 0 .. CPM - 1: this member's columns
            v_f32_st_tnsr_partial(mc, lcol, lv, CPM - 1, 0);
#else
            v_f32_st_tnsr(mc, mcol, mv);
            v_f32_st_tnsr(mc, lcol, lv);
#endif
#endif
        }
    }
}
